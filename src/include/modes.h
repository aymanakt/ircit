/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
**
MODULEID("$Id: modes.h,v 1.2 1998/02/20 14:58:44 ayman Beta $")
**
*/

 static void Usermodes (char *);

 static void Chanmodes (Channel *, char *, char *, char *, char *);
 static void Chanmode_o (Channel *, char *, char *, int);
 static void Chanmode_v (Channel *, char *, char *, int);
 static void Chanmode_b (Channel *, char *, char *, char *, int);

 void sMODE (char *, char *);

