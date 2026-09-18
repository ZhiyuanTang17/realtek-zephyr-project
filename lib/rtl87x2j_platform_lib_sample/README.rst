.. zephyr:code-sample:: rtl87x2j_platform_library
   :name: RTL87X2J External Platform Library

   Build and link the RTL87X2J platform library with an external Makefile.

Overview
********

This is a runnable Zephyr application organized like
``samples/application_development/external_lib``. The application CMake build
exports the selected board's compiler flags and invokes the library's simple
Makefile through ``ExternalProject_Add``. The resulting static archive is
linked into ``app`` and the normal Zephyr ELF and binary files are produced.

The user runs only ``west build``; there is no need to invoke the library
Makefile directly.

Building
********

From the west workspace root:

.. code-block:: console

   west build -p always -b rtl87x2j_evb/rtl8762jth \
     realtek-zephyr-project/lib/rtl87x2j_platform_lib_sample \
     -- -DRTK_BASE=$HOME/bee5/realtek

The firmware outputs are under ``build/zephyr/``. Flash with the runner selected
for the board, for example:

.. code-block:: console

   west flash

Expected Output
***************

.. code-block:: console

   RTL87X2J external platform library sample running
