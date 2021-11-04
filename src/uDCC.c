/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
*/

#include <main.h>
#include <terminal.h>
#include <list.h>
#include <scroll.h>
#include <history.h>
#include <updates.h>
#include <people.h>
#include <sockets.h>
#include <dcc.h>
#include <uDCC.h>

 MODULEID("$Id: uDCC.c,v 1.5 1998/04/06 06:13:44 ayman Beta $");

 static void ProcessOfferedChat (Someone *, Index *, DCCParams *);
 static void uDCCCHAT (char *);
 static void ProcessOfferedSend (Someone *, Index *, DCCParams *);
 static void uDCCGET (char *);
 static void uDCCSEND (char *);

 static void DCCCLOSE_CHAT (char *);
 static void DCCCLOSE_GET (char *);
 static void DCCCLOSE_SEND (char *);
 static void uDCCCLOSE (char *);


 #define dccptr ((DCCParams *)iptr->pindex)
 #define _sptr ((Socket *)iptr->index)
 #define _pptr ((Someone *)iptr->sindex)

 static __inline__ void
  ProcessOfferedChat (Someone *pptr, Index *iptr, DCCParams *dcc)

 {
  int x=0;

    if ((x=AttemptDCCConnection_(pptr, dcc)))
     {
       if (x>0)
        {
         char s[MEDIUMBUF];
         extern InputHistory *tab;

          say ("%$% 6DCC CHAT connection to %s established.\n", 
               CTCP, pptr->name);

          sprintf (s, "/DMSG %s ", "/DMSG", pptr->name);
          AddtoUserInputHistory (tab, s, 0);
        }
       else
        {
         say ("%$% Peer is working on connection...\n", CTCP);
        }
     }
    else
     {
      say ("%$% 6DCC CHAT connection to %s failed.\n",
           CTCPH, pptr->name);

      RemoveIndexbyptr (pptr, iptr);
     }

 }  /**/


 static __inline__ void uDCCCHAT (char *args)

 {
  int i,
      j,
      x,
      esc=0;
  register char *aux;
  register Someone *pptr;
  register Index *iptr;
  extern char tok[][80];

    if (!args)
     {
       if (!(iptr=FindDCCIndex(DCC_CHAT, NULL)))
        {
         say ("%$% No pending 6CHAT offers found.\n", MILD);
        }
       else
        {
         ProcessOfferedChat ((Someone *)iptr->sindex, iptr, 
                             (DCCParams *)iptr->pindex);
        }

      return;
     }

   i=splitargs2 (args, ',');

    for (j=0; j<=i; j++)
     {
       if (!*tok[j])  continue;

      {
       if (*tok[j]=='\\')
        {
         if ((esc=ProcessEscaped(tok[j]+1)))
          {
           aux=tok[j]+1;
          }
         else
          {
           aux=tok[j];
          }
        }
       else
        {
         aux=tok[j];
        }
      }

       if ((!esc)&&(arethesame(aux, "*")))
        {
         say ("%$% This option isn't implemented yet.\n", MILD);

         return;
        }

       if (!(pptr=SomeonebyName(tok[j])))
        {
         pptr=AddtoPeople (tok[j]);

         pptr->when=time (NULL);

          if (InitDCCCHAT(pptr))
           {
            char s[LARGBUF];
            extern InputHistory *tab;

             sprintf (s, "/DMSG %s ", tok[j]);
             AddtoUserInputHistory (tab, s, 0);
           }
          else
           {
            RemovefromPeople (pptr->name);
           }
        }
       else 
        {
          if (!(iptr=FindDCCIndex(DCC_CHAT, pptr)))
           {
            char s[LARGBUF];
            extern InputHistory *tab;

              if (InitDCCCHAT(pptr))
               {
                sprintf (s, "/DMSG %s ", pptr->name);
                AddtoUserInputHistory (tab, s, 0);
               }
           }
          else 
           {
             if (IS_DCCWAITING(dccptr))
              {
                if (IS_CHATTOUS(dccptr))
                 {
                   if (!iptr->index) 
                    {
                     ProcessOfferedChat (pptr, iptr, dccptr);
                    }
                   else 
                    {
                     say ("%$% Peer is still processing this"
                          " connection.\n", CTCPH);
                    }
                 }
                else
                if (IS_CHATTOHIM(dccptr))
                 {
                  say ("%$% 6DCC CHAT connection with %s"
                       " in progress.\n  %$192%$196%$16 %s.\n", CTCPH, 
                       pptr->name, PrintDCCState(iptr));
                 }
              }
             else
             if (IS_DCCCONNECTED(dccptr))
              {
               say ("%$% You already have active 6DCC CHAT" 
                    " session with %s.\n  %$192%$196%$16 %s.\n", 
                    CTCPH, pptr->name, PrintDCCState(iptr));
              }
           }
        }
     }

 }  /**/


  static __inline__ void 
 ProcessOfferedSend (Someone *pptr, Index *iptr, DCCParams *dcc)

 {
  int x=0;

    if (IS_DCCCONNECTED(dcc))
     {
      say ("%$% 6DCC GET (6%s) from %s is already running.\n"
           "  %$192%$196%$16 %s.\n", 
           CTCPH, dcc->file, pptr->name, PrintDCCState(iptr));

      return;
     }
    else
    if ((IS_DCCWAITING(dccptr))&&(iptr->index))
     {
      say ("%$% Peer is still processing this connection.\n", CTCPH);

      return;
     }

    if (CheckFileBeforeGetting(dcc))
     {
       if ((x=AttemptDCCConnection_(pptr, dcc)))
        {
          if (x>0)
           {
            say ("%$% 6DCC GET (6%s %lu) from %s started.\n",
                 CTCP, dcc->file, dcc->size, pptr->name);
           }
          else
           {
            say ("%$% Peer working on connection...\n", CTCP);
           }
        }
       else
        {
         say ("%$% 6DCC GET (6%s %lu) from %s failed.\n",
              CTCPH, dcc->file, dcc->size, pptr->name);

         RemoveIndexbyptr (pptr, iptr);
        }
     }
    else
     {
      say ("%$% Unable to start 6DCC GET (6%s %lu) from %s.\n",
           CTCPH, dcc->file, dcc->size, pptr->name);

      RemoveIndexbyptr (pptr, iptr);
     }

 }  /**/

 
 static __inline__ STD_IRCIT_COMMAND(uDCCGET)

 {
  int i,
      j,
      x,
      esc=0;
  char *s,
       *aux;
  register Someone *pptr;
  register Index *iptr;
  extern char tok[][80];

    if (!args)
     {
       if (!(iptr=FindDCCIndex(DCC_GET, NULL)))
        {
         say ("%$% No pending 6SEND offers found.\n", CTCPH);
        }
       else
        {
         ProcessOfferedSend ((Someone *)iptr->sindex, iptr,
                             (DCCParams *)iptr->pindex);
        }

      return;
     }

   s=splitw (&args);

    if (!args)
     {
      {
       if (*s=='\\')
        {
         if ((esc=ProcessEscaped(s+1)))
          {
           aux=s+1;
          }
         else
          {
           aux=s;
          }
        }
       else
        {
         aux=s;
        }
      }

       if ((!esc)&&(arethesame(aux, "*")))
        {
         int n=0;

          say ("%$% Getting all pending files...\n", CTCP);

          n=GetAllPendingFiles ((Someone *)NULL);

          say ("%$% Processed %d pending file%s..\n", CTCP, n, n>1?"s.":".");
        }
       else
        {
          if (!(pptr=SomeonebyName(aux)))
           {
            say ("%$% Unable to find %s in the DCC list.\n", CTCPH, aux);
           }
          else 
           {
             if (!(iptr=FindDCCIndex(DCC_GET, pptr, NULL)))
              {
               say ("%$% No pending 6SEND offer from %s found.\n",
                    CTCPH, pptr->name);
              }
             else
              {
               ProcessOfferedSend (pptr, iptr, dccptr);
              }
           }
        }
     }
    else 
     {
      TRIM(args);

      i=splitargs2 (args, ',');

       for (j=0; j<=i; j++)
        {
          if (!*tok[j])  continue;

         {
          if (*tok[j]=='\\')
           {
            if ((esc=ProcessEscaped(tok[j]+1)))
             {
              aux=tok[j]+1;
             }
            else
             {
              aux=tok[j];
             }
           }
          else
           {
            aux=tok[j];
           }
         }

          if (!(pptr=SomeonebyName(s)))
           {
            say ("%$% Unable to find %s in the DCC list.\n", CTCPH, s);

            return;
           }

          if ((!esc)&&(arethesame(aux, "*")))
           {
            int n=0;

             n=GetAllPendingFiles (pptr);

             say ("%$% Processed %d pending file%sfrom %s...\n", CTCP, 
                  n, n>1?"s ":" ", pptr->name);

             return;
            }

          if (!(iptr=FindDCCIndex(DCC_GET, pptr, aux)))
           {
            say ("%$% No pending 6SEND (6%s) from %s found.\n",
                 CTCPH, aux, pptr->name);
           }
          else
           {
            ProcessOfferedSend (pptr, iptr, (DCCParams *)iptr->pindex);
           }
        }
     }

 }  /**/


 static __inline__ STD_IRCIT_COMMAND(uDCCSEND)

 {
  int i,
      j,
      new=0;
  char *s;
  register Someone *pptr;
  register Index *iptr;
  extern char tok[][80];

    if (!args)
     {
      say ("%$% arguments invalid or incomplete.\n", MILD);

      return;
     }

    if ((s=splitw(&args)), !args) 
     {
      say ("%$% arguments invalid or incomplete.\n", MILD);

      return;
     }

   TRIM(args);

   i=splitargss (args, ',');

    for (j=0; j<=(i-1); j++)
     {
       if (!*tok[j])  continue;

      pptr=SomeonebyName (s);

       if (pptr==(Someone *)NULL)
        {
         pptr=AddtoPeople (s);

         pptr->when=time (NULL);
         new=1;
        }

       if ((iptr=FindDCCIndex(DCC_SEND, pptr, tok[j])))
        {
          if (IS_DCCCONNECTED(dccptr))
           {
            say ("%$% 6DCC SEND (6%s) to %s already"
                 " running.\n", CTCPH, dccptr->file, pptr->name);
           }
          else
          if (IS_DCCWAITING(dccptr))
           {
            say ("%$% 6DCC SEND (6%s) to %s in progress.\n",
                 CTCPH, dccptr->file, pptr->name);
           }
        }
       else
        {
          if (!(InitDCCSEND(pptr, tok[j])))
           {
            if (new)  RemovefromPeople (pptr->name);
           }
        }
     }

 }  /**/


  static __inline__ STD_IRCIT_COMMAND(DCCCLOSE_CHAT)

 {
  int i,
      j;
  register Someone *pptr;
  register Index *iptr;
  extern char tok[][80];

   i=splitargss (args, ',');

    for (j=0; j<=(i-1); j++)
     {
       if (!*tok[j])  continue;

       if (!(pptr=SomeonebyName(tok[j])))
        {
         say ("%$% Unable to find %s in the DCC list.\n", CTCPH, tok[j]);
        }
       else
        {
          if (!(iptr=FindDCCIndex(DCC_CHAT, pptr)))
           {
            say ("%$% Unable to find 6DCC CHAT connection"
                 " with %s.", CTCPH, pptr->name);
           }
          else
           {
            CloseDCCConnection (DCC_CHAT, pptr, iptr);
           }
        }
     }

 }  /**/


 static __inline__ void DCCCLOSE_GET (char *args)

 {
  int i,
      j,
      esc=0;
  char *n,
       *aux;
  register Someone *pptr;
  register Index *iptr;
  extern char tok[][80];

   if ((n=splitw(&args)), !args)
    {
     say ("%$% arguments invalid or incomplete.\n", CTCPH);
      
     return;
    }

   TRIM(args);

   i=splitargs2 (args, ',');

    for (j=0; j<=i; j++)
     {
       if (!*tok[j])  continue;

       if (!(pptr=SomeonebyName(n)))
        {
         say ("%$% Unable to find %s in the DCC list.\n", CTCPH, n);
         continue;
        }

      {
       if (*tok[j]=='\\')
        {
         if ((esc=ProcessEscaped(tok[j]+1)))
          {
           aux=tok[j]+1;
          }
         else
          {
           aux=tok[j];
          }
        }
       else
        {
         aux=tok[j];
        }
      }

       if ((!esc)&&(arethesame(tok[0], "*")))
        {
         CloseDCCConnection (DCC_GET, pptr, NULL);

         return;
        }
       else
        {
          if (!(iptr=FindDCCIndex(DCC_GET, pptr, tok[j])))
           {
            say ("%$% Unable to find 6DCC SEND (6%s)"
                 " from %s.\n", CTCPH, tok[j], pptr->name);
           }
          else
           {
            CloseDCCConnection (DCC_GET, pptr, iptr);
           }
        }
     }

 }  /**/


 static __inline__ STD_IRCIT_COMMAND(DCCCLOSE_SEND)

 {
  int i,
      j,
      esc=0;
  char *n,
       *aux;
  register Someone *pptr;
  register Index *iptr;
  extern char tok[][80];

   if ((n=splitw(&args)), !args)
    {
     say ("%$% arguments invalid or incomplete.\n", CTCPH);

     return;
    }

   TRIM(args);

   i=splitargs2 (args, ',');

    for (j=0; j<=i; j++)
     {
       if (!*tok[j])  continue;

       if (!(pptr=SomeonebyName(n)))
        {
         say ("%$% Unable to find %s in the DCC list.\n", CTCPH, n);
         continue;
        }

      {
       if (*tok[j]=='\\')
        {
         if ((esc=ProcessEscaped(tok[j]+1)))
          {
           aux=tok[j]+1;
          }
         else
          {
           aux=tok[j];
          }
        }
       else
        {
         aux=tok[j];
        }
      }

       if ((!esc)&&(arethesame(tok[0], "*")))
        {
         CloseDCCConnection (DCC_SEND, pptr, NULL);

         return;
        }
       else
        {
          if (!(iptr=FindDCCIndex(DCC_SEND, pptr, tok[j])))
           {
            say ("%$% Unable to find 6DCC SEND (6%s)"
                 " to %s.\n", CTCPH, tok[j], pptr->name);
           }
          else
           {
            CloseDCCConnection (DCC_SEND, pptr, iptr);
           }
        }
     }

 }  /**/


 static __inline__ STD_IRCIT_COMMAND(uDCCCLOSE)

 {
  int mode=0;
  char *s,
       n[LARGBUF];
  register Socket *sptr;
  register Someone *pptr;
  register Index *iptr;
  DCCParams *dcc;

   if (!args)
    {
     say ("%$% arguments invalid or incomplete.\n", MILD);
      
     return;
    }

   if ((s=splitw(&args)), !args) 
    {
      if (arethesame(s, "*"))
       {
        say ("%$% Mass-Closure of DCC connections started...\n", CTCP);

        CloseDCCConnection (EVERYTHING, NULL);
       }
      else
       {
        say ("%$% arguments invalid or incomplete.\n", MILD);
       }

     return;
    }

  TRIM(args);

   if (arethesame(s, "CHAT"))      mode=DCC_CHAT;
   else if (arethesame(s, "GET"))  mode=DCC_GET;
   else if (arethesame(s, "SEND")) mode=DCC_SEND;
   else 
   if (!arethesame(s, "*"))
    {
     say ("%$% Unknown DCC type %s specified.\n", MILD, s);

     return;
    }

    if (arethesame(args, "*"))
     {
      say("%$% Mass-Closure of 6%s type DCC connections started...\n",
          CTCP, strtoupper(s));

      CloseDCCConnection (mode, NULL);

      return;
     }
    else
     {
      switch (mode)
       {
        case DCC_CHAT:
              {
               DCCCLOSE_CHAT (args);

               break;
              }

        case DCC_GET:
              {
               DCCCLOSE_GET (args);

               break;
              }
             
        case DCC_SEND:
              {
               DCCCLOSE_SEND (args);

               break;
              }

        default:
             say ("%$% Unknown DCC type specfied.\n", MILD);
       }
     }
      
 }  /**/


 STD_IRCIT_COMMAND(uDCC)

 {
  char *s;

    if (!args)
     {
      ShowDCCConnections ();

      return;
     }

    if (!set("ALLOW_DCC"))
     {
      say ("%$% DCC protocol is currently disabled.\n", MILD);

      return;
     }

   s=splitw (&args);

    if (s[0]=='-')
     {
       if (arethesame(s, "-SCRL"))
        {
         ScrollingDCCConnections (NULL, 0);
        }
       else
       if (arethesame(s, "-LST"))
        {
         ShowDCCConnections ();
        }
       else
        {
         say ("%$% You really need -h.\n", MILD);
        }
     }

    else
    if (!strcasecmp(s, "CHAT"))
     {
      uDCCCHAT ((args&&*args)?args:NULL);
     }
    else
    if (!strcasecmp(s, "GET"))
     {
      uDCCGET ((args&&*args)?args:NULL);
     }
    else
    if (!strcasecmp(s, "SEND"))
     {
      uDCCSEND ((args&&*args)?args:NULL);
     }
    else
    if (!strcasecmp(s, "CLOSE"))
     {
      uDCCCLOSE ((args&&*args)?args:NULL);
     }
    else
     {
      say ("%$% You really need -h.\n", MILD);
     }

 }   /**/


 STD_IRCIT_COMMAND(uCHAT)

 {
  char s[LARGBUF];

   sprintf (s, "%s %s", "CHAT", args?args:"");

   uDCC (s);

 }  /**/


 STD_IRCIT_COMMAND(uSEND)

 {
  char s[LARGBUF];

   sprintf (s, "%s %s", "SEND", args?args:"");

   uDCC (s);

 }  /**/


 STD_IRCIT_COMMAND(uGET)

 {
  char s[LARGBUF];

   sprintf (s, "%s %s", "GET", args?args:"");

   uDCC (s);

 }  /**/


 STD_IRCIT_COMMAND(uCLOSE)

 {
  char s[LARGBUF];

   sprintf (s, "%s %s", "CLOSE", args?args:"");

   uDCC (s);

 }  /**/

