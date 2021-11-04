/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
**
MODULEID("$Id: flood.h,v 1.1 1998/02/17 13:18:48 ayman Beta $")
**
*/

#define CTCPFLOOD    1
#define CTCPACTFLOOD 2
#define DCCFLOOD     3
#define DEOPFLOOD    4
#define INVITEFLOOD  4
#define JOINFLOOD    6
#define KICKFLOOD    7
#define MSGFLOOD     8
#define NICKFLOOD    9
#define NOTICEFLOOD  10
#define PUBLICFLOOD  11
/*#define WALL_FLOOD 3
#define WALLOP_FLOOD 4*/
#define NROFFLOODS 11

 struct Flood {
               char   nick[MAXNICKLEN],
                      host[MAXHOSTLEN],
                      chan[MAXCHANLEN];
               int    type,
                      counter;
               time_t when;
              };

 void CheckFlood (int, char *, char *, char *);

