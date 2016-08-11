#ifndef _SAM_CONFIG_H
#define _SAM_CONFIG_H

/* Where to put temporary files. */
#define TMPDIR "/tmp"

/* Is the target 64-bits?
 * 0 - build for 32-bit systems
 * 1 - build for 64-bit little-endian systems
 * 2 - build for 64-bit big-endian systems
 */
#define USE64BITS 1

/* Command key definitions. These should generally be ASCII control codes.
 * Define them to 0 to disable a key.
 */
#define LINEUP      0x05 /* Ctrl-E */
#define LINEDOWN    0x18 /* Ctrl-X */
#define CHARLEFT    0x13 /* Ctrl-S */
#define CHARRIGHT   0x04 /* Ctrl-D */
#define COMMANDKEY  0x0B /* Ctrl-K */
#define SCROLLKEY   0x80 /* Scroll down. */
#define UPKEY       0x81 /* Scroll up.   */
#define ESC         0x1B /* Escape */

/* The remote shell to use for remote connections. */
#define RXPATH "/usr/bin/ssh"

/* The system shell to use. Must be Bourne-compatible. */
#define SHPATH "/bin/sh"

#endif
