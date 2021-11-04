/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
**
MODULEID("$Id: notify.h,v 1.2 1998/02/20 14:58:44 ayman Beta $")
**
*/

  struct Notify
              {
               time_t when;
               int flags,
                   xtra;
               char name[MAXNICKLEN];
               char *host;
               void *link;
              };
 typedef struct Notify Notify;


 void InitIRCNotify (void);
 void InitUTMPNotify (void);

 int inNotifyList (const List *, const char *);
 static int _AddtoNotifyList (List *, Notify *);
 void AddtoNotify (List *, char *, int);
 int ClearNotifyList (List *, int , void *);
 void RemovefromNotify (List *, char *);

 void ResetIRCNotify (void);
 
 void ircNotify (void);
 void utmpCheck (void);
 int utmpNotify (void);

 void ShowIRCNotify (List *, char *);
 void ShowUTMPNotify (void);

