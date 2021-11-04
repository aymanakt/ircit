/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
**
MODULEID("$Id: log.h,v 1.1 1998/02/17 13:18:48 ayman Beta $")
**
*/

 struct LastMsg
               {
                time_t when;
                char msg[LARGBUF],
                     nick[MAXNICKLEN],
                     host[MAXHOSTLEN];
                void *link;
               };
 typedef struct LastMsg LastMsg;


 struct PublicMsg 
                 {
                  time_t when;
                  char msg[LARGBUF];
                  void *link;
                 };
 typedef PublicMsg PublicMsg;


 struct Logs 
            {
             List *MsgLog,
                  *PubLog,
                  *DCCLog,
                  *AwayLog,
                  *TelnetLog;
            };
 typedef struct Logs Logs;
