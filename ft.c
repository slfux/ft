/* Format Text (c) 2025 Steven-L. Fuchs (s.fx)
 * 
 * A messy but fast single-pass interpreter for formatting commands.
 *
 * NOTES: 
 * - chars must be 32 - 126 ASCII
 * - chars -1 = EOF, -2 = SOF
 * - line must end with LF
 * - max chars per line: 255
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_LENGTH 256
#define MAX_ARGS   4
#define SOF        -2

typedef enum MODE_ {
  MODE_PRE,
  MODE_WRAP,
} MODE;

typedef enum STATE_ {
  STATE_COLLECT,
  STATE_ESCAPE,
  STATE_COMMAND
} STATE;

typedef enum CSTATE_ {
  COM_NONE,
  COM_PRE,
  COM_WRAP,
  COM_SPACE,
  COM_HORIZONTAL,
  COM_TRAIL
} CSTATE;

void printIndent(int indent) {
  for (int i = 0; i < indent; i++)
    putchar(' ');
}

void printNewlines(int newlines) {
  for (int i = 0; i < newlines; i++)
    putchar('\n');
}

void printHorizontal(int length, char c) {
  for (int i = 0; i < length; i++)
    putchar(c);
  putchar('\n');
}

short xtoi(char c) {
  if (c <= 102 && c >= 97) c -= 32;
  if (c <= 70 && c >= 65) return c - 55;
  return c - 48;
}

int parse(char *file) {
  FILE         *fp;
  MODE          mode;
  STATE         state;
  CSTATE        cstate;
  short         words, chars, indent, breakl; // used for length calculation
  short         i, last, llen, argc;          // used for book keeping
  char          c, pc;
  bool          escapeSkip;
  char          lastw[MAX_LENGTH];
  char          line[MAX_LENGTH];
  unsigned char args[MAX_ARGS];

  fp = fopen(file, "rb");

  if (!fp) {
    perror("Could not open file");
    return EXIT_FAILURE;
  }

  // PREPARE

  memset(lastw, 0, MAX_LENGTH);
  memset(line, 0, MAX_LENGTH);
  memset(args, 0, MAX_ARGS);

  mode       = MODE_PRE;       // current collection mode
  state      = STATE_COLLECT;  // current state
  cstate     = COM_NONE;       // current state for command processing
  words      = 1;              // number of words, used for number of spaces
  chars      = 0;              // number of read graphical chars
  indent     = 0;              // number of spaces inserted on right
  breakl     = MAX_LENGTH - 1; // break limit, number of chars until newline
  last       = 0;              // last word length
  llen       = 0;              // line length
  argc       = 0;              // count of chars in args buffer
  c          = SOF;            // current char, set to start of file
  escapeSkip = false;          // skip reading to handle same char in collect

  // LOOP

  while (c != EOF) {

    if (!escapeSkip) {
      pc = c;
      c = fgetc(fp);
    } else
      escapeSkip = false;

    // if end of file reached
    if (c == EOF) {
      // was collecting and buffers not empty, dump line and word
      if (state == STATE_COLLECT && (llen || last)) {
        printIndent(indent);
        if (llen == 0)
          puts(lastw);
        else
          printf("%s %s\n", line, lastw);
      }
      break;
    }

    // null or negative values should not be in file
    if (c <= 0) {
      fprintf(stderr, "Invalid character: 0x%02x\n", (unsigned char)c);
      fclose(fp);
      return EXIT_FAILURE;
    }

    switch (state) { /*1*/
    
      case STATE_COLLECT:

        if (c == '\\' && (pc == '\n' || pc == SOF)) {
          state = STATE_ESCAPE;
          continue;
        }

        // if going into command mode dump all buffers        
        if (c == '.' && (pc == '\n' || pc == SOF)) {
          state = STATE_COMMAND;

          // output everything so far if line or last word is not empty
          if (llen || last) {
            printIndent(indent);
            if (llen == 0)
              puts(lastw);
            else
              printf("%s %s\n", line, lastw);
          }

          // reset everything
          memset(lastw, 0, MAX_LENGTH);
          memset(line, 0, MAX_LENGTH);
          words = 1;
          chars = 0;
          last  = 0;
          llen  = 0;

          // reset command state
          cstate = COM_NONE;
    
          continue;
        }

        // parse inputs according to current mode
        switch (mode) { /*2.1*/
        
          case MODE_PRE:
          
            // needs to use last word buffer because of escapes
            lastw[last] = c;
            // one character always set
            putchar(lastw[0]);
            // might be set due to escape
            if (lastw[1]) putchar(lastw[1]);
            
            lastw[0] = 0; // sometimes set to '\' by escape
            lastw[1] = 0; // if 0 set to '\', 1 has the char

            last  = 0;
            chars = 0; // is incremented by escape
            
            break; /*2.1*/
            
          case MODE_WRAP:
          
            // line not full
            if (breakl > chars + indent + words - 1) {

              // if graphical char, add to last word
              if (isgraph(c)) {
                lastw[last] = c;
                chars++;
                last++;
              // if not graphic char
              } else {
                // if last word not empty, copy word into line
                if (last > 0) {
                  if (llen > 0) {
                    line[llen] = ' ';
                    llen++;
                  }
                  for (i = 0; i < last; i++)
                    line[llen + i] = lastw[i];
                  llen += last;
                  last  = 0;
                  words++;
                  memset(lastw, 0, MAX_LENGTH);
                }
              }
            
            // line full
            } else {

              // word still not done
              if (isgraph(c)) {
                // word takes up entire line, print it and break it down
                if (last == breakl - indent) {
                  printIndent(indent);
                  puts(lastw);
                  memset(lastw, 0, MAX_LENGTH);

                  lastw[0] = c;
                  chars    = 1;
                  last     = 1;

                // print line and use word for next line
                } else {
                  lastw[last] = c;
                  last++;

                  printIndent(indent);
                  puts(line);
                  
                  memset(line, 0, MAX_LENGTH);

                  words = 1;
                  chars = last;
                  llen  = 0;
                }
              // if word is done, print line + word, reset everything
              } else {
                printIndent(indent);

                // word spans entire line, just print word
                if (last == breakl - indent)
                  puts(lastw);
                else
                  printf("%s %s\n", line, lastw);

                memset(lastw, 0, MAX_LENGTH);
                memset(line, 0, MAX_LENGTH);

                words = 1;
                chars = 0;
                last  = 0;
                llen  = 0;
              }
            
            }
            
            break; /*2.1*/
            
        }
        
        break; /*1*/

      case STATE_ESCAPE:

        /*
         * NOTE:
         * last is 0 here, because the word buffer got flushed because of the
         * previous newline before the escaping backslash!
         */

        // if just escaped period, then print it
        if (c == '.')
          lastw[0] = c;
        else {
          /* 
           * if current char is not a backslash already,
           * write backslash and skip reading so collector can process
           * current char which is already read!
           */
          if (c != '\\')
            lastw[0] = '\\';
          
          escapeSkip = true;
        }
        
        chars++;
        last++;
        state = STATE_COLLECT;
        
        break; /*1*/

      case STATE_COMMAND:

        // if empty/newline command, insert newline and just continue
        if (c == '\n' && !cstate) {
          putchar('\n');
          state = STATE_COLLECT;
          continue;
        }

        // continue reading until command
        if (pc == '.' && !cstate)
          continue;

        // if no state chosen, look for one or trail to end of line
        if (!cstate) {
          // clear buffer and counter before using them next round
          memset(args, 0, MAX_ARGS);
          argc = 0;
        
          if (pc == 'p' && c == 'r') {
            cstate = COM_PRE;
            continue;
          } else if (pc == 'w' && c == 'r') {
            cstate = COM_WRAP;
            continue;
          } else if (pc == 's' && c == 'p') {
            cstate = COM_SPACE;
            continue;
          } else if (pc == 'h' && c == 'r') {
            cstate = COM_HORIZONTAL;
            continue;
          } else { // no valid command
            cstate = COM_TRAIL;
            continue;
          }
        }

        switch (cstate) { /*2.2*/

          case COM_NONE:

            // should never occur here
            // just to shut up the compiler

            break; /*2.2*/

          case COM_PRE:

            // discard everything

            if (c == '\n') {
              mode = MODE_PRE;
              state = STATE_COLLECT;
            }
            
            break; /*2.2*/
        
          case COM_WRAP:

            // collect in args buffer until full, discard rest
            if (isgraph(c) && argc < MAX_ARGS) {
              args[argc] = c;
              argc++;
            }

            // at end of line try to parse and switch mode
            if (c == '\n') {

              // check for valid hex values as args
              if (
                isxdigit(args[0]) && isxdigit(args[1]) && 
                isxdigit(args[2]) && isxdigit(args[3])
              ) {
                indent = (xtoi(args[0]) << 4) | xtoi(args[1]);
                breakl = (xtoi(args[2]) << 4) | xtoi(args[3]);

                // prohibit 0 size formatting
                if (indent >= breakl) {
                  state = STATE_COLLECT;
                  continue;
                }
              // if args invalid just ignore and continue collecting
              } else {
                state = STATE_COLLECT;
                continue;
              }
            
              mode = MODE_WRAP;
              state = STATE_COLLECT;
            }

            break; /*2.2*/

          case COM_SPACE:

            if (isgraph(c) && argc < MAX_ARGS) {
              args[argc] = c;
              argc++;
            }

            if (c == '\n') {
              if (isxdigit(args[0]))
                printNewlines(xtoi(args[0]));

              state = STATE_COLLECT;
            }

            break; /*2.2*/

          case COM_HORIZONTAL:

            if (isgraph(c) && argc < MAX_ARGS) {
              args[argc] = c;
              argc++;
            }

            if (c == '\n') {
              if (isxdigit(args[0]) && isxdigit(args[1]) && isgraph(args[2]))
                printHorizontal(
                  (xtoi(args[0]) << 4) | xtoi(args[1]),
                  args[2]
                );
            
              state = STATE_COLLECT;
            }

            break; /*2.2*/

          case COM_TRAIL:

            // do nothing for unrecognised command

            if (c == '\n') {
              state = STATE_COLLECT;
            }

            break; /*2.2*/
        }
        
        break; /*1*/
    }
  }

  fclose(fp);

  return EXIT_SUCCESS;
}

void printHelp(char *exe) {
  printf(
    "Format Text (c) 2025 Steven-L. Fuchs (s.fx)\n\n"
    "Usage: %s [FILE|-h]\n\n"
    "Options:\n"
    "  -h  print help\n\n",
    exe
  );
}

int main(int argc, char **argv) {
  int   i;
  char *file;

  if (argc <= 1) {
    puts("Please give an input file");
    return EXIT_SUCCESS;
  }

  for (i = 1; i < argc; i++) {
    if (!strcmp("-h", argv[i])) {
      printHelp(argv[0]);
      return EXIT_SUCCESS;
    } else if (i == 1) {
      file = argv[i];
    } else break;
  }

  return parse(file);
}
