/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
**
MODULEID("$Id: people.h,v 1.6 1998/04/08 08:54:34 ayman Beta $")
**
*/

 struct DCCParams
          {
           int    fd;
           int    flags,
                  type,
                  socket;
           char   *file,
                  *chksum;
           void   *index;       /* to 'Index *'   */
           unsigned long recv,  /* bytes recieved */
                         xfer;  /* bytes sent     */
           long   port;
           unsigned long size,
                         ack,
                         ip;
          };
 typedef struct DCCParams DCCParams;
               
 struct Index 
            {
             time_t        idle,
                           when;
             int           flag,
                           what,
                           type,
                           gflags,
                           protocol,
                           whatever;
             void         *index;  // to 'Socket *'
             void         *pindex; // protocol index (DCC, telnet, ...)
             void         *sindex; //pointer back to the owner (pptr)
             struct Index *next;
            };
 typedef struct Index Index;

 struct Someone 
              {
               int     glock,
                      nEntries;
               time_t when,
                      laston;
               int    id,
                      flags,
                      cflags,
                      gflags,
                      status;
               char  name[MAXNICKLEN];
               Index  *head,
                      *tail; 
               struct Someone *next;
              };
 typedef struct Someone Someone;

 struct People 
             {
              int     glock, 
                      nEntries;
              Someone *head;
              Someone *tail;
             };
 typedef struct People People;

#define INDEXTOSOMEONE(x) ((Someone *)(x)->sindex)
#define INDEXTOSOCKET(x) ((Socket *)(x)->index)
#define INDEXTODCC(x) ((DCCParams *)(x)->pindex)
#define INDEXTOPROTOCOL(x) ((x)->pindex) //generic

#define PROTOCOLTOINDEX(x) ((Index *)(x)->index)

#define NOMORE_INDICES(x) ((x)->nEntries==0)
#define GOT_INDICES(x) ((x)->nEntries>0)
#define GOT_PEOPLE(x) (people->nEntries>0)

#define IS_DCCCHAT(x) (((x)->type&DCCCHATTOUS)||((x)->type&DCCCHATTOHIM))
#define IS_CONNECTED(x) ((x)->flags&CONNECTED)
#define IS_NOTCONNECTED(x) ((x)->flags&AWAITINGCONNECTION)

 void InitPeople (void);
 Someone *AddtoPeople (const char *);
 int RemovefromPeople (char *);
 Index *AddtoSomeoneIndex (Someone *);
 int RemovefromSomeoneIndex (Someone *, int);
 int RemoveIndexbyptr (Someone *, Index *);
 int ResetDCCPeople (int, ...);
 Index *GetSomeoneIndex (Someone *, int);
 Someone *SomeonebyName (const char *);
 Someone *FindSomeone (char *, int *, void *);
 Someone *AddSomeone (char *, int , int);
 void FinishoffIndex (Index *);
 int isvalidname (char *);
 int GetID (void);
 void ShowPeople (void);
 boolean None (const People *);
 boolean IndexEmpty (const Someone *);

 //void ScrollingPeople (void);

