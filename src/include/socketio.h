/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
**
MODULEID("$Id: socketio.h,v 1.3 1998/02/27 02:16:13 ayman Beta $")
**
*/

 int ProbeSockets (void);
 static int GetRegisteredSockets (int *);
 static int AddtoQueue (Socket *);
 static void AddtoSocketQueue (Socket *, char *, int);
 static void RemovefromSocketQueue (Socket *, int *);

 int ToSocket (int, const char *, ...) __attribute__ ((format (printf, 2, 3)));
 int WriteToSocket (int, const char *);

