/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
**
MODULEID("$Id: servers.h,v 1.5 1998/04/08 08:54:34 ayman Beta $")
**
*/

  struct Server {
          time_t when,
                 left;
          int id,
              port;
          int flags;
          char nick[MAXNICKLEN],        
               server[MAXHOSTLEN];
          void *link;
         };
 typedef struct Server Server;

 void InitServersList (void);
 Server *AddtoServersList (List *, char *, int);
 void ClearServersList (List *, int, ...);
 Server *_GetServerfromList (List *, int, ...);
 int _ConnectToServer (char *, unsigned long, int);
 int IRCServerConnect (char *, unsigned long);
 void PostConnection (void);
 int HowmanyServers (void);
 int serverid (char *, unsigned long);
 void ShowServersList (const List *, int, int);
 void uSERVER (char *);

 void ScrollingServers (void *, int);

