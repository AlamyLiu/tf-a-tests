/*
 * Copyright (c) 2018, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <debug.h>
#include <drivers/io/io_driver.h>
#include <drivers/io/io_fip.h>
#include <errno.h>
#include <firmware_image_package.h>
#include <image_loader.h>
#include <io_storage.h>
#include <platform.h>
#include <platform_def.h>
#include <stdint.h>
#include <string.h>
#include <uuid.h>
#include <uuid_utils.h>


typedef struct {
	unsigned int file_pos;
	fip_toc_entry_t entry;
} file_state_t;

static file_state_t current_file = {0};
static uintptr_t backend_dev_handle;
static uintptr_t backend_image_spec;


/* Firmware Image Package driver functions */
static int fip_dev_open(const uintptr_t dev_spec, io_dev_info_t **dev_info);
static int fip_file_open(io_dev_info_t *dev_info, const uintptr_t spec,
			  io_entity_t *entity);
static int fip_file_len(io_entity_t *entity, size_t *length);
static int fip_file_read(io_entity_t *entity, uintptr_t buffer, size_t length,
			  size_t *length_read);
static int fip_file_close(io_entity_t *entity);
static int fip_dev_init(io_dev_info_t *dev_info, const uintptr_t init_params);
static int fip_dev_close(io_dev_info_t *dev_info);


/* TODO: We could check version numbers or do a package checksum? */
static inline int is_valid_header(fip_toc_header_t *header)
{
	if ((header->name == TOC_HEADER_NAME) && (header->serial_number != 0)) {
		return 1;
	} else {
		return 0;
	}
}


/* Identify the device type as a virtual driver */
io_type_t device_type_fip(void)
{
	return IO_TYPE_FIRMWARE_IMAGE_PACKAGE;
}


static const io_dev_connector_t fip_dev_connector = {
	.dev_open = fip_dev_open
};


static const io_dev_funcs_t fip_dev_funcs = {
	.type = device_type_fip,
	.open = fip_file_open,
	.seek = NULL,
	.size = fip_file_len,
	.read = fip_file_read,
	.write = NULL,
	.close = fip_file_close,
	.dev_init = fip_dev_init,
	.dev_close = fip_dev_close,
};


/* No state associated with this device so structure can be const */
static const io_dev_info_t fip_dev_info = {
	.funcs = &fip_dev_funcs,
	.info = (uintptr_t)NULL
};


/* Open a connection to the FIP device */
static int fip_dev_open(const uintptr_t dev_spec __attribute__((unused)),
			 io_dev_info_t **dev_info)
{
	assert(dev_info != NULL);
	*dev_info = (io_dev_info_t *)&fip_dev_info; /* cast away const */

	return IO_SUCCESS;
}


/* Do some basic package checks. */
static int fip_dev_init(io_dev_info_t *dev_info, const uintptr_t init_params)
{
	int result = IO_FAIL;
	unsigned int image_id = (unsigned int)init_params;
	uintptr_t backend_handle;
	fip_toc_header_t header;
	size_t bytes_read;

	/* Obtain a reference to the image by querying the platform layer */
	result = plat_get_image_source(image_id, &backend_dev_handle,
				       &backend_image_spec);
	if (result != IO_SUCCESS) {
		WARN("Failed to obtain reference to image id=%u (%i)\n",
			image_id, result);
		result = IO_FAIL;
		goto fip_dev_init_exit;
	}

	/* Attempt to access the FIP image */
	result = io_open(backend_dev_handle, backend_image_spec,
			 &backend_handle);
	if (result != IO_SUCCESS) {
		WARN("Failed to access image id=%u (%i)\n", image_id, result);
		result = IO_FAIL;
		goto fip_dev_init_exit;
	}

	result = io_read(backend_handle, (uintptr_t)&header, sizeof(header),
			&bytes_read);
	if (result == IO_SUCCESS) {
		if (!is_valid_header(&header)) {
			WARN("Firmware Image Package header check failed.\n");
			result = IO_FAIL;
		} else {
			VERBOSE("FIP header looks OK.\n");
		}
	}

	io_close(backend_handle);

 fip_dev_init_exit:
	return result;
}

/* Close a connection to the FIP device */
static int fip_dev_close(io_dev_info_t *dev_info)
{
	/* TODO: Consider tracking open files and cleaning them up here */

	/* Clear the backend. */
	backend_dev_handle = (uintptr_t)NULL;
	backend_image_spec = (uintptr_t)NULL;

	return IO_SUCCESS;
}


/* Open a file for access from package. */
static int fip_file_open(io_dev_info_t *dev_info, const uintptr_t spec,
			 io_entity_t *entity)
{
	int result = IO_FAIL;
	uintptr_t backend_handle;
	const io_uuid_spec_t *uuid_spec = (io_uuid_spec_t *)spec;
	size_t bytes_read;
	int found_file = 0;

	assert(uuid_spec != NULL);
	assert(entity != NULL);

	/* Can only have one file open at a time for the moment. We need to
	 * track state like file cursor position. We know the header lives at
	 * offset zero, so this entry should never be zero for an active file.
	 * When the system supports dynamic memory allocation we can allow more
	 * than one open file at a time if needed.
	 */
	if (current_file.entry.offset_address != 0) {
		WARN("fip_file_open : Only one open file at a time.\n");
		return IO_RESOURCES_EXHAUSTED;
	}

	/* Attempt to access the FIP image */
	result = io_open(backend_dev_handle, backend_image_spec,
			 &backend_handle);
	if (result != IO_SUCCESS) {
		WARN("Failed to open Firmware Image Package (%i)\n", result);
		result = IO_FAIL;
		goto fip_file_open_exit;
	}

	/* Seek past the FIP header into the Table of Contents */
	result = io_seek(backend_handle, IO_SEEK_SET, sizeof(fip_toc_header_t));
	if (result != IO_SUCCESS) {
		WARN("fip_file_open: failed to seek\n");
		result = IO_FAIL;
		goto fip_file_open_close;
	}

	found_file = 0;
	do {
		result = io_read(backend_handle,
				 (uintptr_t)&current_file.entry,
				 sizeof(current_file.entry),
				 &bytes_read);
		if (result == IO_SUCCESS) {
			if (uuid_equal(&current_file.entry.uuid,
					  &uuid_spec->uuid)) {
				found_file = 1;
				break;
			}
		} else {
			WARN("Failed to read FIP (%i)\n", result);
			goto fip_file_open_close;
		}
	} while (!is_uuid_null(&current_file.entry.uuid));

	if (found_file == 1) {
		/* All fine. Update entity info with file state and return. Set
		 * the file position to 0. The 'current_file.entry' holds the
		 * base and size of the file.
		 */
		current_file.file_pos = 0;
		entity->info = (uintptr_t)&current_file;
	} else {
		/* Did not find the file in the FIP. */
		current_file.entry.offset_address = 0;
		result = IO_FAIL;
	}

 fip_file_open_close:
	io_close(backend_handle);

 fip_file_open_exit:
	return result;
}


/* Return the size of a file in package */
static int fip_file_len(io_entity_t *entity, size_t *length)
{
	assert(entity != NULL);
	assert(length != NULL);

	*length =  ((file_state_t *)entity->info)->entry.size;

	return IO_SUCCESS;
}


/* Read data from a file in package */
static int fip_file_read(io_entity_t *entity, uintptr_t buffer, size_t length,
			  size_t *length_read)
{
	int result = IO_FAIL;
	file_state_t *fp;
	size_t file_offset;
	size_t bytes_read;
	uintptr_t backend_handle;

	assert(entity != NULL);
	assert(buffer != (uintptr_t)NULL);
	assert(length_read != NULL);
	assert(entity->info != (uintptr_t)NULL);

	/* Open the backend, attempt to access the blob image */
	result = io_open(backend_dev_handle, backend_image_spec,
			 &backend_handle);
	if (result != IO_SUCCESS) {
		WARN("Failed to open FIP (%i)\n", result);
		result = IO_FAIL;
		goto fip_file_read_exit;
	}

	fp = (file_state_t *)entity->info;

	/* Seek to the position in the FIP where the payload lives */
	file_offset = fp->entry.offset_address + fp->file_pos;
	result = io_seek(backend_handle, IO_SEEK_SET, file_offset);
	if (result != IO_SUCCESS) {
		WARN("fip_file_read: failed to seek\n");
		result = IO_FAIL;
		goto fip_file_read_close;
	}

	result = io_read(backend_handle, buffer, length, &bytes_read);
	if (result != IO_SUCCESS) {
		/* We cannot read our data. Fail. */
		WARN("Failed to read payload (%i)\n", result);
		result = IO_FAIL;
		goto fip_file_read_close;
	} else {
		/* Set caller length and new file position. */
		*length_read = bytes_read;
		fp->file_pos += bytes_read;
	}

/* Close the backend. */
 fip_file_read_close:
	io_close(backend_handle);

 fip_file_read_exit:
	return result;
}


/* Close a file in package */
static int fip_file_close(io_entity_t *entity)
{
	/* Clear our current file pointer.
	 * If we had malloc() we would free() here.
	 */
	if (current_file.entry.offset_address != 0) {
		memset(&current_file, 0, sizeof(current_file));
	}

	/* Clear the Entity info. */
	entity->info = 0;

	return IO_SUCCESS;
}

/* Exported functions */

/* Register the Firmware Image Package driver with the IO abstraction */
int register_io_dev_fip(const io_dev_connector_t **dev_con)
{
	int result = IO_FAIL;
	assert(dev_con != NULL);

	result = io_register_device(&fip_dev_info);
	if (result == IO_SUCCESS)
		*dev_con = &fip_dev_connector;

	return result;
}
