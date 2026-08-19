================================================================================
Format Text Utility
(c) 2025 Steven-L. Fuchs
================================================================================

1 Syntax
................................................................................

  At the start of a line you can type a dot with the command you want to use.
  Commands which are unrecognised or have invalid arguments are ignored. The
  syntax is quite flexible allowing you to use multiple dots or to leave out
  any spaces, i.e. something like: 

    ...wr0250

  is completely allowed! 

1.1 Modes
................................................................................

  +--------------------------------------------+
  | TODO:                                      |
  | - '.ce XX', center lines to XX line length |
  | - '.in XX', indent lines with XX spaces    |
  +--------------------------------------------+

  1. Preformatted: '.pr'
  2. Wrap: '.wr XX XX'

  The default mode is the preformatted mode, which is what the interpreter
  starts with. 

1.2 Inserts
................................................................................

  +--------------------------------------+
  | TODO:                                |
  | - '.fl S', prints contents of file S |
  | - '.cl S', executes command S        |
  +--------------------------------------+

  1. Newline: '.'
  2. Spacing: '.sp X'
  3. Horizontal: '.hr XX C'

1.3 Escapes
................................................................................

  There are only two escape sequences: '\.' and '\\'. If you need to write a
  dot at the start of the line without it being a command, use a backslash to
  create an escape. If you you want to have a backslash followed by a dot,
  just like an escape sequence, you can use another backslash. Any other
  backslash at the beginning of the line is just treated like an normal
  character in the text. 

2 Encoding
................................................................................

  Characters must be between 32 - 126 for ASCII interpretation, i.e. any
  greater values will not be accepted and cause the program to stop. This
  program only uses UNIX LF characters for newlines and expects the input file
  to use at least a single LF character for the end of a line! 

