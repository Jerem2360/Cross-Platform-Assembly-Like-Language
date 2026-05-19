# Changelog

This file summarizes the user-visible changes brought by version 0.2.

## Highlights

- Updated the project branding in the README:
  - corrected the language name/acronym wording
  - added build instructions
  - improved the hello-world example documentation
- Added support for defining arrays in uninitialized data.
- Added custom integer parsing, along with a new parsing test.
- Improved float literal handling and float comparison support.
- Fixed several code generation and pointer/dangling reference issues in operand writing.
- Fixed invalid assembly generation in indexed symbol dereferencing.
- Tightened language rules around:
  - `if ... goto` targets
  - dereference bases and indexes involving floats
  - label/symbol usage as dereference indexes
- Improved register occupation cleanup behavior.

## Notable commit themes

- Parser and literal handling improvements
- Backend/code generation fixes for x86-64
- Documentation and build instructions updates
- Internal cleanup in register tracking and string coding

## User impact

These changes make the language/compiler more reliable and easier to use by:
- supporting more valid source patterns
- rejecting invalid constructs earlier
- generating safer assembly output
