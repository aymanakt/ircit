/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
**
MODULEID("$Id: ignore.h,v 1.1 1998/02/17 13:18:48 ayman Beta $")
**
*/

 struct Ignore
              {
               time_t when;
               char  host[MAXHOSTLEN];
               unsigned long flags;
               size_t hits;
               int    type,
                      duration;
               void  *link;
              };
 typedef struct Ignore Ignore;

#define TEMP 1
#define PERM 2

#define IRCIGNORE     1
#define TELNETIGNORE  2
#define SESSIONIGNORE 4

#define CHANNEL 1
#define PERSON  2

#define IG_MSG       1 
#define IG_PUB       2
#define IG_DCC       4
#define IG_CTCP      8
#define IG_JOIN      16
#define IG_KICK      32
#define IG_TOPIC     64
#define IG_MODES     128
#define IG_PART      256
#define IG_QUIT      512
#define IG_INVITE    1024
#define IG_NOTICE    2048
#define IG_NETSPLIT  4096
#define IG_TELNET    8192
#define IG_GLOBAL    16384

#define IG_ALL (IG_MSG|IG_CTCP|IG_INVITE|IG_NOTICE|IG_TELNET)

#define IS_IGNOTICE(x) ((x)->flags&IG_NOTICE)
#define IS_IGINVITE(x) ((x)->flags&IG_INVITE)
#define IS_IGMSG(x) ((x)->flags&IG_MSG)
#define IS_IGCTCP(x) ((x)->flags&IG_CTCP)
#define IS_IGTELNET(x) ((x)->flags&IG_TELNET)
#define IS_IGGLOBAL(x) ((x)->flags&IG_GLOBAL)
#define IS_IGALL(x) (((x)->flags^IG_ALL)==0)

 int ValidIgnoreType (const char *);
 void InitIgnoreList (void);
 static int ClearIgnoreList (List *, int, int, void *);
 void RemovefromIgnore (List *, char *, int);
 static int _AddtoIgnoreList (List *, Ignore *);
 int AddtoIgnore (List *, char *, int, int);
 char *MaskIgnoreEntry (const char *);
 int IsSheIgnored (char *, int);
 void uIGNORE (char *args);
 void ShowIgnore (List *, int);

