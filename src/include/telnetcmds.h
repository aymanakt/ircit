/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
**
MODULEID("$Id: telnetcmds.h,v 1.2 1998/02/20 14:58:44 ayman Beta $")
**
*/

 void TU_HELP (int, char *);
 void TU_INFO (int, char *);
 void TU_MODE (int, char *);
 void TU_MSG (int, char *);
 void TU_QUIT (int, char);
 void TU_TIME (int, char *);
 void TU_WHOM (int, char *);

 struct TelnetCommand 
                     {
                      char *identifier;
                      char *alias;
                      void (*func) (int, char *);
                     };

 struct TelnetCommand TU_Functions []=
        {
         {"HELP",     "h",       (void (*)(int, char *)) TU_HELP    },  
         {"INFO",     "info",    (void (*)(int, char *)) TU_INFO    },
         {"MODE",     "md",      (void (*)(int, char *)) TU_MODE    },
         {"MSG",      "m",       (void (*)(int, char *)) TU_MSG     },
         {"QUIT",     "quit",    (void (*)(int, char *)) TU_QUIT    },
         {"TIME",     "t",       (void (*)(int, char *)) TU_TIME    },
         {"WHOM",     "w",       (void (*)(int, char *)) TU_WHOM    },
         {(char *)0,  (char *)0, (void (*)(int, char *)) 0          }
        };

