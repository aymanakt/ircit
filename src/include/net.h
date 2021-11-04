/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
**
MODULEID("$Id: net.h,v 1.3 1998/04/08 08:54:34 ayman Beta $")
**
*/

 struct resolved
                {
                 char dns[MAXHOSTLEN];
                 char dotted[14];
                 unsigned long ip;
                };

 struct ResolvedAddress {
        char *dns,
             *dotted;
        unsigned flags;
        struct in_addr *inetaddr;
       };
 typedef struct ResolvedAddress ResolvedAddress;


 void InitIPC (void);
 void InitNet (void); 
 void FetchLocalhost (void);
 int InitTelnet (void);
 int LookupInterface (const char *);
 int PlaySound (int);
 void InitUDP (void) __attribute__ ((unused));
 int isdottedquad (const char *);
 int RequestSocket (void);
 int SetSocketFlags (int, int, int);
 int Reconnect (char *, int);
 int Connect (struct in_addr *, int);
 //struct in_addr *ResolveAddress (const char *);
 int ResolveAddress (const char *, ResolvedAddress *);
 void nslookup (char *);
 char *RawIPToDotted (unsigned long);
 char *HostToDotted (char *);
 char *DottedToHost (char *);
 char *h_strerror (int);

