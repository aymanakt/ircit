/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
**
"$Id: main.h,v 1.6 1998/04/08 08:54:34 ayman Beta $"
**
*/

#ifndef __GNUC__
# error This program will not compile without GNUC-compatible compiler.
#endif

#include <limits.h>
#include <sys/types.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include <str.h>

#define MODULEID(id) static const char rcs[]=id;

#define CTRL_B 0x2
#define _B 0x2
#define CTRL_C 0x3
#define CTRL_G 0x7
#define CTRL__ 0x1F
#define __ 0x1F
#define CTRL_V 0x16
#define _V 0x16
#define EVERYTHING 0xffff

#define INFO  0xFE|A_ALTCHARSET|COLOR_PAIR(CYAN)|WA_BOLD
#define MSG   0x2D|A_ALTCHARSET|COLOR_PAIR(GREEN)|A_BOLD
#define MILD  0xFE|A_ALTCHARSET|COLOR_PAIR(YELLOW)|WA_BOLD
#define HOT   0xFE|A_ALTCHARSET|COLOR_PAIR(RED)|WA_BOLD

#define LST_SIGNAL 6

#define ALL      1
#define NUMBER   2
#define NAME     3
#define WILDCARD 4

#define PROTOCOL_DCC    1
#define PROTOCOL_TELNET 2
#define PROTOCOL_IPC    3

#define MODE1 1
#define MODE2 2

#define MAXNICKLEN  32
#define MAXCHANLEN  150
#define MAXTOPIC    80
#define MAXHOSTLEN  165

#define MINIBUF     50
#define SMALLBUF    150
#define MEDIUMBUF   300
#define LARGBUF     600
#define XLARGBUF    1024
#define XXLARGBUF   2048

#define NOTCONNECTED 1
#define CONNECTED    2
#define TRYING       4

 typedef enum {FALSE, TRUE} boolean;

#define STD_IRCIT_COMMAND(x) void x (char *args)
#define STD_IRC_COMMAND(x) void x (char *args)
#define STD_TELNET_COMMAND(x) void x (int so, char *args)
#define STD_IRCIT_PREF(x) void x (Preference *pref, char *value)
#define STD_SCRIPT_COMMAND(x) x (struct fplArgument *arg)
#define STD_IRC_SERVER(x) void x (char *from, char *rest)

 #define xmalloc(x, y) \
         (x)=malloc ((y)); \
          if (!(x)) \
           { \
            nYell("%$% Unable to obtain additional memory - Bye...\n", HOT); \
            exit (1); \
           }

 #define xfree(x) free((x)); (x)=NULL

 #define say(x...) nYell (x)
 #define lsay(w, x, y, z...) lnYell (w, x, y, z) 
 #define swprintf(x...) Fwprintf (StatusWin, x)
 #define uswprintf(x...) Fwprintf (UpperStatusWin, x)

 extern char *empty_string;
 extern void log (int, ...);

 int main (int, char **);
 void InitStuffandThings (void) __attribute__ ((constructor));
 void CheckCommandLine (int, char **);
 void Help (void);

