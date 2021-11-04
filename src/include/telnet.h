/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
**
MODULEID("$Id: telnet.h,v 1.3 1998/04/08 08:54:34 ayman Beta $")
**
*/

 struct Telnet 
              {
               int   idle,
                     type,
                     flags,
                     status,
                     msg_per_sec;
               char *channnel;
               void *index;
              };
 typedef struct Telnet Telnet;

#define TELNETTOINDEX(x) ((Index *)(x)->index)
#define INDEXTOTELNET(x) ((Telnet *)(x)->pindex)
#define IS_TELNET(x) ((x)->protocol==PROTOCOL_TELNET)

#define IS_CONNECTED_TELNET(x) ((x)->flags&CONNECTED)
#define IS_T_INVISIBLE(x) ((x)->flags&1)

#define UTELNET_MSG  0x01  /* show what user types */
#define UTELNET_CMND 0x02  /* show user commands */

 int AnswerTelnetRequest (Socket *);
 void TelnetLogin (Socket *);
 Index *FindTelnetIndex (const Someone *);
 void DisconnectTelnetuser (Socket *, Index *);
 void CheckTimeout (int);
 void ParseDCC (Socket *);
 void ParseTelnet (Socket *);
 void BroadcastMsg (char *, int);
 void CheckTelnetTimeout (void);

