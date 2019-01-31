Trusted Firmware-A Tests - version 2.0
======================================

The Trusted Firmware-A Tests (TF-A-Tests) is a suite of baremetal tests to
exercise the `Trusted Firmware-A (TF-A)`_ features from the Normal World. It
enables strong TF-A functional testing without dependency on a Rich OS. It
mainly interacts with TF-A through its SMC interface.

It provides a basis for TF-A developers to validate their own platform ports and
add their own test cases.

The following TF-A features are currently tested to some extent (this list is
not exhaustive):

-  `SMC Calling Convention`_
-  `Power State Coordination Interface (PSCI)`_
-  `Software Delegated Exception Interface (SDEI)`_
-  `Performance Measurement Framework (PMF)`_
-  Communication and interaction with the `Test Secure Payload (TSP)`_
-  `Firmware update`_ (or recovery mode)
-  `EL3 payload`_ boot flow
-  `Secure partition`_ support

These tests are not a compliance test suite for the Arm interface standards used
in TF-A (such as PSCI).

They do not cover 100% of the TF-A code. The fact that all tests pass does not
mean that TF-A is free of bugs.

They are not reference code. They should not be considered as the official way
to test hardware/firmware features. Instead, they are provided as example code
to experiment with and improve on.

License
-------

The software is provided under a BSD-3-Clause `license`_. Contributions to this
project are accepted under the same license with developer sign-off as
described in the `Contributing Guidelines`_.

This project contains code from other projects as listed below. The original
license text is included in those source files.

-  The libc source code is derived from `FreeBSD`_ code, which uses various BSD
   licenses, including BSD-3-Clause and BSD-2-Clause.

-  The `LLVM compiler-rt`_ source code is disjunctively dual licensed
   (NCSA OR MIT). It is used by this project under the terms of the NCSA
   license (also known as the University of Illinois/NCSA Open Source License),
   which is a permissive license compatible with BSD-3-Clause. Any
   contributions to this code must be made under the terms of both licenses.

This release
------------

This is the first public release of the TF-A Tests code. It provides a starting
point for exercising some of the TF-A features on the Arm FVP and Juno
platforms, porting the tests to new platforms, enhancing existing tests or
implementing new ones.

Please note that this code is not mature yet and suffers from some stability
issues. Refer to the known issues in the `change log`_ for more details.


Platforms
`````````

Juno Arm Development Platform
'''''''''''''''''''''''''''''

The AArch64 build of this release has been tested on variants r0, r1 and r2 of
the `Juno Arm Development Platform`_. The AArch32 build has only been tested on
variant r0.

Armv8 Architecture Fixed Virtual Platforms
''''''''''''''''''''''''''''''''''''''''''

The AArch64 build has been tested on the following Armv8 Architecture Fixed
Virtual Platforms (`FVP`_):

-  ``FVP_Base_AEMv8A-AEMv8A``
-  ``FVP_Base_Cortex-A35x4``
-  ``FVP_Base_Cortex-A57x4-A53x4``
-  ``FVP_Base_RevC-2xAEMv8A``
-  ``Foundation_Platform``

The AArch32 build has been tested on the following `FVP`_\ s:

-  ``FVP_Base_Cortex-A32x4``
-  ``FVP_Base_RevC-2xAEMv8A``

NOTE: Unless otherwise stated, the model version is version 11.4, build 37.

System Guidance for Infrastructure Fixed Virtual Platforms
''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

The AArch64 build has been tested on the following SGI Fixed Virtual Platforms
(`FVP`_):
-  ``FVP_CSS_SGI-575``

NOTE: For ``FVP_CSS_SGI-575``, the model version is 11.4, build 40

Still to come
`````````````

-  More tests.
-  Support for new platforms.
-  Design improvements.
-  Stability improvements.
-  Enhance test framework to make it easier to implement tests.
-  Fixing known issues (see the `change log`_ for more details).


Getting started
---------------

Get the TF-A Tests source code from `trustedfirmware.org`_.

See the `User Guide`_ for instructions on how to install, build and use the TF-A
Tests.

See the `Design Guide`_ for information on how the TF-A Tests internally work.

See the `Porting Guide`_ for information about how to use this software on
another Armv8-A platform.

See the `Contributing Guidelines`_ for information on how to contribute to this
project.


IRC channel
-----------

Development discussion takes place on the same channel as for TF-A, i.e. the
``#trusted-firmware-a`` channel on the Freenode IRC network. This is not an
official support channel.  If you have an issue to raise, please use the issues
tracker <TODO: insert link>.

--------------

*Copyright (c) 2018, Arm Limited. All rights reserved.*

.. _Contributing Guidelines: contributing.rst
.. _license: license.rst
.. _change log: docs/change-log.rst
.. _Design Guide: docs/design.rst
.. _Porting Guide: docs/porting-guide.rst
.. _User Guide: docs/user-guide.rst

.. _FVP: https://developer.arm.com/products/system-design/fixed-virtual-platforms
.. _Juno Arm Development Platform: https://developer.arm.com/products/system-design/development-boards/juno-development-board

.. _FreeBSD: http://www.freebsd.org
.. _LLVM compiler-rt: https://compiler-rt.llvm.org/

.. _Power State Coordination Interface (PSCI): PSCI_
.. _PSCI: http://infocenter.arm.com/help/topic/com.arm.doc.den0022d/Power_State_Coordination_Interface_PDD_v1_1_DEN0022D.pdf
.. _Software Delegated Exception Interface (SDEI): SDEI_
.. _SDEI: http://infocenter.arm.com/help/topic/com.arm.doc.den0054a/ARM_DEN0054A_Software_Delegated_Exception_Interface.pdf
.. _SMC Calling Convention: http://infocenter.arm.com/help/topic/com.arm.doc.den0028b/ARM_DEN0028B_SMC_Calling_Convention.pdf

.. _trustedfirmware.org: https://git.trustedfirmware.org/TF-A/tf-a-tests.git

.. _Trusted Firmware-A (TF-A): TF-A_
.. _TF-A: https://www.github.com/ARM-software/arm-trusted-firmware
.. _Test Secure Payload (TSP): TSP_
.. _TSP: https://github.com/ARM-software/arm-trusted-firmware/tree/master/bl32/tsp
.. _Performance Measurement Framework (PMF): PMF_
.. _PMF: https://github.com/ARM-software/arm-trusted-firmware/blob/master/docs/firmware-design.rst#performance-measurement-framework
.. _Firmware update: https://github.com/ARM-software/arm-trusted-firmware/blob/master/docs/firmware-update.rst
.. _EL3 payload: https://github.com/ARM-software/arm-trusted-firmware/blob/master/docs/user-guide.rst#el3-payloads-alternative-boot-flow
.. _Secure partition: https://github.com/ARM-software/arm-trusted-firmware/blob/master/docs/secure-partition-manager-design.rst
