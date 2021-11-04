/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
*/

#include <main.h>
#include <terminal.h>
#include <output.h>
#include <list.h>
#include <scroll.h>
#include <history.h>
#include <updates.h>
#include <prefs.h>
#include <people.h>
#include <sockets.h>
#include <serverio.h>
#include <events.h>
#include <dcc.h>

 MODULEID("$Id: ctcp.c,v 1.5 1998/03/26 09:58:01 ayman Beta $");

  static Index *
 RegisterDCCOffer (unsigned, Someone *, unsigned long, unsigned long, ...);
  static void
 ProcessCTCP_DCCCSEND (const char *, const char *, char *);
  static void
 ProcessCTCP_DCCCHAT (const char *, const char *host, char *);
  static void
 ProcessCTCP_DCC (const char *, const char *, char *);
  static void
 ProcessCTCP_ACTION (const char *, const char *, char *);
  static void
 ProcessCTCP_PING (const char *, const char *, char *);
  static void
 ProcessCTCP_FINGER (const char *, const char *, char *);
  static void
 ProcessCTCP_SOURCE (const char *, const char *, char *);


 #define CTCP_CHAR 0x1
 #define _B 0x2

 #define ANNOUNCE_CHAT_OFFER() \
  say ("%$% 6DCC CHAT request recieved from %s [%s] [%lu %d].\n", \
         CTCP, nick, host, ((DCCParams *)iptr->pindex)->ip, \
         ((DCCParams *)iptr->pindex)->port)

 #define ANNOUNCE_SEND_OFFER() \
  say ("%$% 6DCC SEND (6%s %lu) from %s [%lu %d].\n", \
        CTCP, file, size, nick, ip, port)

 #define MULTIBLE_CHAT_REQUESTS() \
  say ("%$% Multible 6DCC CHAT requests from %s!\n", \
         CTCPH, pptr->name); \
 nCTCPToServer (nick, "DCC REJECT CHAT chat")


  static Index * 
 RegisterDCCOffer (unsigned type, Someone *pptr, 
                   unsigned long ip, unsigned long port, ...)

 {
  const size_t sizeofDCCParams=sizeof(DCCParams);
  Index *iptr;
  DCCParams *dcc;

   iptr=AddtoSomeoneIndex (pptr); 
   
   iptr->when=time (NULL);
   iptr->protocol=PROTOCOL_DCC;

   dcc=FillDCCEntry (iptr);

   dcc->type|=type;
   dcc->flags|=AWAITINGCONNECTION;
   dcc->ip=ip;
   dcc->port=port;

    if (type&DCCSENDING)
     {
      va_list va;
      char *file;
      size_t size;

       va_start (va, port);
       file=va_arg (va, char *);
       size=va_arg (va, size_t);
       
       dcc->size=size;
       dcc->file=strdup (file);
     }

   return (Index *)iptr;

 }  /**/


  static __inline__ void
 ProcessCTCP_DCCCHAT (const char *nick, const char *host, char *rest)

 {
  unsigned long ip=0L;
  unsigned long port=0L;
  Index *iptr;
  Someone *pptr;

   splitw (&rest);

    if ((ip=atoul((splitw(&rest)))), ((!ip)||!(port=atoul(rest))))
     {
      say ("%$% Badly formatted 6DCC CHAT request from %s [%s]\n"
           "  %$192%$196%$16 IP: %lu, Port: %lu\n",
           CTCPH, nick, host, ip, port);

      return;
     }

    if (!(pptr=SomeonebyName(nick)))
     {
      pptr=AddtoPeople (nick);

      pptr->when=time (NULL);
     }
    else
     {
       if ((iptr=FindDCCIndex(DCC_CHAT, pptr)))
        {
         say ("%$% %s [%s] issued multible 6DCC CHAT requests" 
              " - Ignoring...\n", CTCPH, nick, host);

         nCTCPToServer (nick, "DCC REJECT CHAT chat");

         return;
        }
     }
    
   iptr=RegisterDCCOffer (DCCCHATTOUS, pptr, ip, port); 

   ANNOUNCE_CHAT_OFFER();

   UpdateSockets (M_CHATS, '+');

    if (set("AUTO_DCC_CHAT"))
     {
      int x=0;
      char p[MEDIUMBUF];
      #define H_TABS  0x17
      extern InputHistory *tab;

       say ("%$% Engaging AUTO-DCC-CHAT...\n", INFO);

        if ((x=AttemptDCCConnection_(pptr, (DCCParams *)iptr->pindex)))
         {
           if (x>0)
            {
             say ("%$% 6DCC CHAT^B^C connection to %s established.\n",
                  CTCP, pptr->name);

             sprintf (p, "/DMSG %s ", pptr->name);
             AddtoUserInputHistory (tab, p, 0);
            }
           else
            {
             say ("%$% Peer working on connection...", CTCP);
            }
         }
        else
         {
          say ("%$% 6DCC CHAT connection to %s failed.\n", 
               CTCPH, pptr->name);

          nCTCPToServer (nick, "DCC REJECT CHAT chat");

          RemoveIndexbyptr (pptr, iptr);
         }
     }

 }  /**/


  static __inline__ void
 ProcessCTCP_DCCSEND (const char *nick, const char *host, char *rest)

 {
  size_t size=0;
  char *file;
  unsigned long ip=0L,
                port=0L;
  Index *iptr;
  Someone *pptr;

    if (file=splitw(&rest), !rest)
     {
      badly_formatted:

      say ("%$% Badly formatted 6DCC SEND offer from %s [%s]\n"
           "  %$192%$196%$16 file: %s, rest: %s\n", CTCPH, 
           nick, host, file, rest?rest:"NULL");

      return;
     }

   if (ip=atoul(splitw(&rest)), ((!ip)||(!rest)))  goto badly_formatted;

   if (port=atoul(splitw(&rest)), ((!port)||(!rest)))  goto badly_formatted;

   if (!(size=atoul(splitw(&rest))))  goto badly_formatted;

   if (!(pptr=SomeonebyName(nick)))
    {
     pptr=AddtoPeople (nick);

     pptr->when=time (NULL);
    }
   else
    {
      if ((iptr=FindDCCIndex(DCC_GET, pptr, file)))
       {
        say ("%$% %s [%s] issued multible 6DCC SEND (6%s) "
             "requests - Ignoring...\n", CTCPH, nick, host, file);

        nCTCPToServer (nick, "DCC REJECT SEND %s", file);

        return;
       }
    }

   iptr=RegisterDCCOffer (DCCSENDING, pptr, ip, port, file, size);

   ANNOUNCE_SEND_OFFER();

   UpdateSockets (M_GETS, '+');

    if (set("AUTO_DCC_GET"))
     {
      int x=0;

       say ("%$% Engaging AUTO-DCC-GET (%s %lu)...\n", INFO, file, size);

        if (CheckFileBeforeGetting((DCCParams *)iptr->pindex))
         {
           if ((x=AttemptDCCConnection_(pptr, (DCCParams *)iptr->pindex)))
            {
              if (x>0)
               {
                say ("%$% 6DCC GET (6%s %lu) from %s"
                     " started.\n", CTCP, file, size, nick);

                return;
               }
              else
               {
                say ("%$% Peer working on connection...\n", CTCP);
               }
            }
           else
            {
             say ("%$% 6DCC GET (6%s %lu) from %s failed.",
                  CTCPH, file, size, nick);

             nCTCPToServer (nick, "DCC REJECT SEND %s", file);

             RemoveIndexbyptr (pptr, iptr);
            }
         }
        else
         {
          say ("%$% 6DCC GET (6%s %lu) from %s failed.",
               CTCPH, file, size, nick);

          nCTCPToServer (nick, "DCC REJECT SEND %s", file);

          RemoveIndexbyptr (pptr, iptr);
         }
     }

 }  /**/


  static __inline__ void 
 ProcessCTCP_DCC (const char *nick, const char *host, char *rest)

 {
  char *type;

    if ((!rest)||(type=splitw(&rest), !rest))
     {
      say ("%$% Badly formatted DCC request from %s [%s]\n"
           "  %$192%$196%$16 %s\n",
           CTCPH, nick, host, rest?rest:"NULL");

      return;
     }

    if (!strcmp(type, "CHAT"))
     {
      ProcessCTCP_DCCCHAT (nick, host, rest);
     }
    else
    if (!strcmp(type, "SEND"))
     {
      ProcessCTCP_DCCSEND (nick, host, rest);
     }

 }  /**/


  static __inline__ void
 ProcessCTCP_ACTION (const char *nick, const char *host, char *rest)

 {
   say ("* %s %s\n", nick, rest);

 }  /**/


  static __inline__ void
 ProcessCTCP_FINGER (const char *nick, const char *host, char *rest)

 {
   say ("%$% %s [%s] fingered you.\n", CTCP, nick, host);

    if (sets("FINGER_REPLAY"))
     {
      nCTCPToServer (nick, "FINGER %s", valueof("FINGER_REPLAY"));
     }

 }  /**/


  static __inline__ void
 ProcessCTCP_PING (const char *nick, const char *host, char *rest)

 {
   say ("%$% %s [%s] pinged you.\n", CTCP, nick, host);

   nCTCPToServer (nick, "PING %u", time(NULL));

    if (sets("PING_REPLAY"))
     {
      ToServer ("NOTICE %s :%s\n", nick, valueof("PING_REPLAY"));
     }

 }  /**/


  static __inline__ void
 ProcessCTCP_VERSION (const char *nick, const char *host, char *rest)

 {
  extern char *c_version;

   say ("%$% 6CTCP VERSION query from %s [%s].\n", CTCP, nick, host);

   nCTCPToServer (nick, "VERSION \02%s\02", c_version);

 }  /**/


  static __inline__ void
 ProcessCTCP_USERINFO (const char *nick, const char *host, char *rest)

 {
   say ("%$% 6CTCP USERINFO query from %s [%s].\n", CTCP,
          nick, host);

 }  /**/


  static __inline__ void
 ProcessCTCP_CLIENTINFO (const char *nick, const char *host, char *rest)

 {
  extern char *c_version;

   say ("%$% 6CTCP CLIENTINFO query from %s [%s].\n", CTCP, 
          nick, host);

   nCTCPToServer (nick, "CLIENTINFO This's %s! Currently supported CTCP tags:"
                        " DCC, ACTION, USERINFO, CLIENTINFO, PING, FINGER,"
                        " SOURCE, ERRMSG, VERSION, TIME, UTC, ECHO."
                        "  DONT'T perform further query on these tags.",
                        c_version);

 }  /**/


  static __inline__ void
 ProcessCTCP_ERRMSG (const char *nick, const char *host, char *rest)

 {
   say ("%$% 6CTCP ERRMSG from %s [%s] - %s\n", CTCP, nick, host);

 }  /**/


  static __inline__ void
 ProcessCTCP_SOURCE (const char *nick, const char *host, char *rest)

 {
   say ("%$% 6CTCP SOURCE query from %s [%s].\n", 
        CTCP, nick, host);

   nCTCPToServer (nick, "%s",
                  "ftp://sunsite.unc.edu/pub/Linux/system/network/chat/irc/"
                  "ircit-0.3.1.tgz\n For more info, contact ayman@POBoxes.com.");


 }  /**/


  static __inline__ void
 ProcessCTCP_TIME (const char *nick, const char *host, char *rest)

 {
  time_t now;

   say ("%$% 6CTCP TIME query from %s [%s].\n", CTCP, nick, host);

   nCTCPToServer (nick, "TIME %s", ctime(&now));

 }  /**/


  static __inline__ void
 ProcessCTCP_ECHO (const char *nick, const char *host, char *rest)

 {
   say ("%$% 6CTCP ECHO request from %s [%s].\n", CTCP, 
        nick, host);

   nCTCPToServer (nick, "ECHO %s", rest?rest:"");

 }  /**/


  static __inline__ void
 ProcessCTCP_UTC (const char *nick, const char *host, char *rest)

 {
   say ("%$% 6CTCP UTC query from %s [%s].\n", CTCP, nick, host);

 }  /**/


  static __inline__ void
 ProcessCTCP_SED (const char *nick, const char *host, char *rest)

 {
   say ("%$% 6CTCP SED query from %s [%s].\n", CTCP, nick, host);

   nCTCPToServer (nick, "ERRMSG SED not supported.");

 }  /**/


 /*
 ** PRIVMSG-embeded CTCP
 */
 void ParseCTCP (char *nick, char *host, char *msg)

 {
  char p[strlen(msg)+1];
  char *s=p;
  char *ctcp;

    for (++msg; ((*msg)&&(*msg!=CTCP_CHAR));)  *s++=*msg++;
        
   (*s=0, s=p, ctcp=splitw(&s));

   //log (1, "ctcp: `%s', msg: `%s'", ctcp, s);

   {
    register void *auxptr;

     if ((auxptr=EventRegistered(EVNT_USRCTCP)))
      {
       ExecHook (auxptr, "%s %s %s %s", nick, host, ctcp, s);

       return;
      }
   }

    if (!strcmp(ctcp, "DCC"))
       {
        ProcessCTCP_DCC (nick, FIXIT(host), s);
       }
    else
    if (!strcmp(ctcp, "ACTION"))
       {
        ProcessCTCP_ACTION (nick, FIXIT(host), s);
       }
    else
    if (!strcmp(ctcp, "FINGER"))
       {
        ProcessCTCP_FINGER (nick, FIXIT(host), s);
       }
    else
    if (!strcmp(ctcp, "PING"))
       {
        ProcessCTCP_PING (nick, FIXIT(host), s);
       }
    else
    if (!strcmp(ctcp, "VERSION"))
       {
        ProcessCTCP_VERSION (nick, FIXIT(host), s);
       }
    else
    if (!strcmp(ctcp, "USERINFO"))
       {
        ProcessCTCP_USERINFO (nick, FIXIT(host), s);
       }
    else
    if (!strcmp(ctcp, "CLIENTINFO"))
       {
        ProcessCTCP_CLIENTINFO (nick, FIXIT(host), s);
       }
    else
    if (!strcmp(ctcp, "SOURCE"))
       {
        ProcessCTCP_SOURCE (nick, FIXIT(host), s);
       }
    else
    if (!strcmp(ctcp, "ERRMSG"))
       {
        ProcessCTCP_ERRMSG (nick, FIXIT(host), s);
       }
    else
    if (!strcmp(ctcp, "ECHO"))
       {
        ProcessCTCP_ECHO (nick, FIXIT(host), s);
       }
    else
    if (!strcmp(ctcp, "TIME"))
       {
        ProcessCTCP_TIME (nick, FIXIT(host), s);
       }
    else
    if (!strcmp(ctcp, "UTC"))
       {
        ProcessCTCP_UTC (nick, FIXIT(host), s);
       }
    else
    if (!strcmp(ctcp, "SED"))
       {
        ProcessCTCP_SED (nick, FIXIT(host), s);
       }
    else
       {
        say ("%$% Unsupported 6CTCP %s from %s [%s].\n", CTCPH,
             ctcp, nick, host);

        nCTCPToServer (nick, "ERRMSG Unsupported CTCP %s", ctcp);
       }

 }  /**/


 /*
 ** NOTICE-embeded CTC. Normally CTCP replays..
 */
 void ParsenCTCP (char *nick, char *host, char *msg)

 {
  char p[strlen(msg)+1];
  char *s=p;
  char *ctcp;

    for (++msg; ((*msg)&&(*msg!=CTCP_CHAR)); )  *s++=*msg++;

   (*s=0, s=p, ctcp=splitw(&s));

    if (!strcmp(ctcp, "PING"))
     {
      int t;
      extern uping_replay;
      extern TimersTable *const tt_ptr;

       t=time(NULL)-uping_replay;

       say ("%$% 6CTCP PING replay from %s: %d second%s\n", CTCP,
            nick, t, t>1?"s.":".");
     }
    else
     {
      say ("%$% 6CTCP %s replay from %s: %s\n", CTCP,
           ctcp, nick, s?s:"Empty replay message.");
     }

 }  /**/

