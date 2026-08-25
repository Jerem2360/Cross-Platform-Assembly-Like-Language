# Changelog

This file summarizes the user-visible changes brought by version 0.3.


# Changes

- Update tests to work for linux x86-64
- grouped commonly used code from tests into lib.txt (!!build script not updated yet!!)
- Implement systemv calling conventions for x86-64
- Internal machinery for syscalls
- updated build script to lib.txt
- added support for linux targets
- created linux build scripts
- finished systemv varargs callconv
- ran tests on WSL & Windows
- [REFACTOR] added "0d" and "\d" prefix/escape sequence for base 10 numbers
- [REFACTOR] added combinations of r, w, x to provide access rights for a given section (e.g.: `section .data rw:`; `section .text rx:`)
- [REFACTOR] added support for array data with predefined value.


# TODO list

- global architecture refactor
- find more generic solution to call arguments being overridden by previous arg writes
- allow support for runtime float constants on targets with different architecture than compiling machine
- change the function statement to 'fun' instead of 'fn'


