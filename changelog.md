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


# TODO list

- global architecture refactor
- find more generic solution to call arguments being overridden by previous arg writes
- allow support for runtime float constants on targets with different architecture than compiling machine


