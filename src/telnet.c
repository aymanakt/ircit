/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
*/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <main.h>
#include <terminal.h>
#include <output.h>
#include <list.h>
#include <scroll.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <lastlog.h>
#include <sockets.h>
#include <updates.h>
#include <people.h>
#include <prefs.h>
#include <history.h>
#include <telnet.h>
#include <telnetcmds.h>

 MODULEID("$Id: telnet.c,v 1.6 1998/04/06 06:13:44 ayman Beta $");

 static int DetectedFlood (Someone *, Telnet *);
 static int DetectedLoginFlood (const char *);
 
 int nTelnetCmnds=((int)(sizeof(TU_Functions)/sizeof(struct TelnetCommand)));

 extern char buf_q[];

#define IG_TELNET 8192

#define HOSTIGNORED() \
  ToSocket (nsocket, " All connections originating from your host are currently ignorred!\r\n"); \
  close (nsocket); \
  say ("%$% Rejected 3Telnet connection from 03%s - Site listed as ignored.\n", TELH, host); \
  return

 int AnswerTelnetRequest (Socket *__sptr)
 
 {
  int i=0,
      nsocket,
      sin_size,
      opt=1;
  char *host;
  chtype *chs;
  Socket *sptr;
  time_t now;
  struct sockaddr_in hisaddr;
  struct hostent *hp;
  char *intros[]={
        " IRCIT 0.4 (c) Ayman Akt.",
        " Enter a login name (max 11 chars):"};
  extern char *c_version;
  extern SocketsTable *const sockets;

   sin_size=sizeof (struct sockaddr_in);
   nsocket=accept (mt_ptr->telnetsock, (struct sockaddr *)&hisaddr, &sin_size); 

    if ((nsocket<0)&&(errno!=EWOULDBLOCK))
     {
      say ("%$% Unable to accept telnet connection (accept()).\n", TELH);

      return 0;
     }
 
   host=(char *)inet_ntoa (hisaddr.sin_addr);
 
    if ((mt_ptr->c_ignore&IG_TELNET)==0)
     {
      char s[MAXHOSTLEN];

       sprintf (s, "*!*@%s", host);

        if (IsSheIgnored(s, IG_TELNET))
         {
          chs=PreSay ("%$% Rejected 3Telnet connection from 03%s"
                      " - Site listed as ignored.\n", TELH, host);
#ifdef LAST__LOG
           if (isLogLevel(LOG_TEL))
            {
             AddtoLastLog (LOG_TEL, NULL, chs);
            }
#endif

          nYelll (chs);
         }
     }
    else
     {
      HOSTIGNORED();
     }

    if (DetectedLoginFlood(host))  
     {
      shutdown (nsocket, 0); 

      return;
     }

   setsockopt (nsocket, SOL_SOCKET, SO_KEEPALIVE, (void *)&opt, sizeof(int));

    if (!(sptr=(Socket *)AddtoSocketsTable(sockets, nsocket)))
     {
      say ("%$% Unable to accept telnet connection: internal.\n", TELH);

      ToSocket (nsocket, "%s\r\n",
               "Sorry I'll have to close your connection, try sometime later!");

      return 0;
     }

   sptr->when=time (NULL);
   strcpy (sptr->haddress, host);
   sptr->type=SOCK_CONNECTEDTELNET;
   sptr->flag|=AWAITINGUSERNAME;
   sptr->read_func=TELNET_Read;
   sptr->close_func=TELNET_Close;
   sptr->hport=ntohs (hisaddr.sin_port);
   sptr->port=atoi (valueof("DEFAULT_TELNET_PORT"));

   Flash (3, 1);

   chs=PreSay ("%$% 3Telnet connection from 3%s.\n", 
               TEL, sptr->haddress);

#ifdef LAST__LOG
       if (isLogLevel(LOG_TEL))
        {
         AddtoLastLog (LOG_TEL, NULL, chs);
        }
#endif

   nYelll (chs);

   ToSocket (sptr->sock, "\r\n%s\r\n%s\n", intros[0], intros[1]);

    if (!RegisteredWithEvery30Seconds(CheckTelnetTimeout))
     {
      RegisterWithEvery30Seconds(CheckTelnetTimeout);
     }

   return nsocket;

 }  /**/

                   
 void TelnetLogin (Socket *ptr)

 {
  int already_in=1;
  char s[100];
  chtype *chs;
  register Someone *pptr;
  register Index *iptr;
  register Telnet *tptr;
  char *replays[]=
   {
    "I don't like your login name - Terminating your link, bye...",
    "Do not enter name longer than 11 chars next time you log in.",
    "This name is already in use, work out different one."
   };
  extern SocketsTable *const sockets;

    if (!isvalidname(buf_q))
     {
      ToSocket (ptr->sock, "%s\n", replays[0]);
      chs=PreSay ("%$% Denied access to 3Telnet user from 3%s:"
                  " ivalid name ``%s''.\n", TELH, ptr->haddress, buf_q);

#ifdef LAST__LOG
       if (isLogLevel(LOG_TEL))
        {
         AddtoLastLog (LOG_TEL, NULL, chs);
        }
#endif

      nYelll (chs);
        
      shutdown (ptr->sock, 0);
      ptr->flag|=FIRING_SQUAD;

      Flash (3, 0);

      return;
     } 
        
    if (strlen(buf_q)>11)
     {
      buf_q[11]=0;
      ToSocket (ptr->sock, "%s\n", replays[1]);
     }

   xmalloc(tptr, (sizeof(Telnet)));
   memset (tptr, 0, sizeof(Telnet)); 

   tptr->flags|=CONNECTED;
   tptr->type|=CONNECTEDTELNET;

    if (!(pptr=SomeonebyName(buf_q)))
     {
      pptr=AddtoPeople (buf_q);
  
      already_in=0;
      pptr->when=time (NULL);
     }

   iptr=AddtoSomeoneIndex (pptr);
  
   iptr->when=iptr->idle=time (NULL); 
   iptr->protocol=PROTOCOL_TELNET;

   ptr->flag&=~AWAITINGUSERNAME;
   ptr->flag|=CONNECTEDTELNET;

   (Telnet *)iptr->pindex=(Telnet *)tptr;
   (Index *)tptr->index=(Index *)iptr;

   (Socket *)iptr->index=(Socket *)ptr;
   (Index *)ptr->index=(Index *)iptr;

   UpdateSockets (ptr->type, '+');

   ToSocket (ptr->sock, "\r\nSzervusz %s! You in.\r\n", pptr->name);
   ToSocket (ptr->sock, "There %s %d user%son board.\r\n",
                         mt_ptr->nTelnets-1>1?"are":"is",
                         mt_ptr->nTelnets-1,
                         mt_ptr->nTelnets-1>1?"s ":" ");

   sprintf (s,"\r\n-*- %s from %s has logged in.", pptr->name, ptr->haddress);
   BroadcastMsg (s, ptr->sock);

   chs=PreSay ("%$% 3Telnet user 3%s [%s] logged in.\n", TEL,
               pptr->name, ptr->haddress);
#ifdef LAST__LOG
       if (isLogLevel(LOG_TEL))
        {
         AddtoLastLog (LOG_TEL, NULL, chs);
        }
#endif

   nYelll (chs);

 }  /**/


 void DisconnectTelnetuser (Socket *sptr, Index *iptr)

 {
  char s[MINIBUF];
  register Someone *pptr=NULL;
  register Telnet *telnet;
  extern SocketsTable *const sockets;

   telnet=INDEXTOPROTOCOL(iptr);

    if (sptr->flag&AWAITINGUSERNAME)
     {
      ToSocket (sptr->sock, "\r\n%s\r\n",
                "Sorry I'll have to close your connection.");
     }
    else
     {
      ToSocket (sptr->sock, "\r\n-*- %s\r\n", 
                "Your connection is about to be terminated...");

      pptr=INDEXTOSOMEONE(iptr);
     }

    close (sptr->sock);
    RemovefromSocketsTable (sockets, sptr->sock);

     if (pptr!=(Someone *)NULL)
      {
       say ("%$% Closed 3Telnet connection for %s.\n", 
            TELH, pptr->name);

       sprintf (s,"\r\n-*- User %s removed from the board.\r\n", pptr->name);
       BroadcastMsg (s, 0);

       RemoveIndexbyptr (pptr, iptr); 
      }
     else
      {
       say ("%$% Closed half-open Telnet connection.\n", TELH);
      }

   Flash (3, 0);

 }  /**/

 
 void ParseTelnet (Socket *sptr)

 {
  int i,
      j[]={[0 ... MAXSOCKSCOUNT]=0},
      k;
  char *buf;
  register Index *iptr;
  register Someone *pptr;
  Telnet *tptr;

   (Index *)iptr=SOCKETTOINDEX(sptr);
   (Someone *)pptr=(Someone *)iptr->sindex;
   tptr=INDEXTOTELNET(iptr);

    if (DetectedFlood(pptr, tptr))
     {
      return;
     }

   buf=buf_q;

    if (*buf=='.')
     {
      int i=0,
          id=0,
          len,
          found=0,
          counter=0;
      char cmnd[LARGBUF];

       buf++;

        if (IS_SHOWTELNETCMND())
         {
          say ("[3%s:cmnd] %s\n", pptr->name, buf);
         }

       otokenize (cmnd, buf, ' ');

        if ((len=strlen(cmnd))>20)
         {
          ToSocket (sptr->sock, "-!- Command too long: ``%s''\r\n", cmnd);

          return;
         }

       wipespaces (buf);

        while ((TU_Functions[i].identifier!=NULL)&&(!found))
         {
           if (!strncasecmp(TU_Functions[i].identifier, cmnd, len)||
               !strncasecmp(TU_Functions[i].alias, cmnd, len))
            {
             counter++;

              if (!strcasecmp(TU_Functions[i].identifier, cmnd)||
                  !strcasecmp(TU_Functions[i].alias, cmnd))
               {
                found=1;
                counter=1;
                id=i;
               }
              else
               {
                id=i;
               }
            }
          i++;
         }

        if ((id>=0)&&(counter==1))
         {
           if (*buf)
            {
             TU_Functions[id].func (sptr->sock, buf);
            }
           else
            {
             TU_Functions[id].func (sptr->sock, NULL);
            }

          return;
         }

        if (counter>1)
         {
          ToSocket (sptr->sock, "%d possible matches.\r\n", counter);

          return;
         }

        if ((!found)&&(counter==0))
         {
          ToSocket (sptr->sock, "-!- Unknown command ``%s''.\r\n", cmnd);

          return;
         }

       iptr->idle=time (NULL);

       buf_q[0]=0;

       return;
     }


    if (IS_SHOWTELNETMSG())
     {
      say ("[3%s:msg] %s\n", pptr->name, buf);
     }

   i=GetTelnetSockets (j);

    for (k=0; k<i; k++)
     {
       if (sptr->sock==j[k])
        {
         continue;
        }

      ToSocket (j[k], "<%s> %s\n", pptr->name, buf_q);
     }
  
   iptr->idle=time (NULL);

   buf_q[0]=0;

 }  /**/


// void BroadcastMsg (int leaveout, const char *msg, ...)
 void BroadcastMsg (char *msg, int leaveout)

 {
#if 0
  int i,
      k,
      j[]={[0 ... MAXSOCKSCOUNT]=0};
  char buffer [600];
  va_list va;

   va_start (va, msg);
   vsnprintf (buffer, 512, msg, va);
   va_end (va);

   i=GetTelnetSockets (j);

    if (i>0)
     {
       for (k=0; k<i; k++)
        {
          if (leaveout)
           {
             if (j[k]==leaveout)
              {
               continue;
              }
           }

         ToSocket (j[k], "%s\n", msg);
        }
     }
#endif

  int i,
      j[]={[0 ... MAXSOCKSCOUNT]=0},
      k;

   i=GetTelnetSockets (j);

    if (i>0)
     {
       for (k=0; k<i; k++)
        {
          if (leaveout)
           {
             if (j[k]==leaveout)
              {
               continue;
              }
           }

         ToSocket (j[k], "%s\n", msg);
        }
     }

 }  /**/


 Index *FindTelnetIndex (const Someone *pptr)

 {
  register Index *iptr;

    for (iptr=pptr->head; iptr!=NULL; iptr=iptr->next)
     {
       if (IS_TELNET(iptr))
        {
         return (Index *)iptr;
        }
     }

   return (Index *)NULL;

 }  /**/


 struct LoginFlood {
         int attempts;
         time_t when;
         char host[MAXHOSTLEN];
        };
 typedef struct LoginFlood LoginFlood;

 static int DetectedLoginFlood (const char *host)

 {
  time_t now=time (NULL);
  register ListEntry *eptr;
  static List tflood;
  #define fldptr ((LoginFlood *)eptr->whatever)

    for (eptr=tflood.head; eptr!=NULL; eptr=eptr->next)
        {
          if (arethesame(host, fldptr->host))
             {
              if (++fldptr->attempts>=2)
                 {
                  if (now-fldptr->when<=2)
                     {
                      char s[MAXHOSTLEN+6];
                      chtype *chs;

                       chs=PreSay ("LOGIN-FLOOD from  %s\n", host);
                       sprintf (s, "-TEL +%s", host);
                       uIGNORE (s);
#ifdef LAST__LOG
                        if (isLogLevel(LOG_TEL))
                         {
                          AddtoLastLog (LOG_TEL, NULL, chs);
                         }
#endif
                       nYelll (chs);
	
                       return 1;
                     }
                  else 
                     {
                      fldptr->when=now;
                      fldptr->attempts=1;
                     }
                 }

              return 0;
             }
        }

 #undef fldptr

    {
     LoginFlood *fldptr;

      xmalloc(fldptr, (sizeof(LoginFlood)));
      memset (fldptr, 0, sizeof(LoginFlood));

      strcpy (fldptr->host, host);
      fldptr->attempts++;
      fldptr->when=time (NULL);

      eptr=AddtoList (&tflood);
      eptr->whatever=(void *)fldptr;
     }

   return 0;

 }  /**/


 static int DetectedFlood (Someone *pptr, Telnet *tptr)

 {
  time_t now=time(NULL);
  chtype *chs;
  #define iptr ((Index *)tptr->index)
  #define sptr ((Socket *)iptr->index)

    if (tptr->idle!=now)
     {
      tptr->idle=now;
      tptr->msg_per_sec=0;
     }
    else
     {
      tptr->msg_per_sec++;

       if (tptr->msg_per_sec>=2)
        {
         chs=PreSay ("%$% 3Telnet flood detected from %s.\n", TELH,
                     pptr->name); 
#ifdef LAST__LOG
          if (isLogLevel(LOG_TEL))
           {
            AddtoLastLog (LOG_TEL, NULL, chs);
           }
#endif
         nYelll (chs);

         sptr->flag|=FIRING_SQUAD;

         return 1;
        }      
     } 

   return 0;

 }  /**/

 #undef sptr
 #undef tptr


 void CheckTelnetTimeout (void)

 {
  int i,
      k,
      j[]={[0 ... MAXSOCKSCOUNT]=0};
  char *timeout;
  unsigned t;

    if (!(timeout=valueof("TELNET_TIMEOUT")))  return;

    if (!(t=atoi(timeout)))  return;

    if ((i=GetTelnetSockets(j)))
     {
      time_t now=time(NULL);
      register Socket *sptr;
      extern Socket *so_hash[];

        for (k=0; k<i; k++)
         {
           if (sptr=so_hash[j[k]], (sptr)&&(sptr->flag&AWAITINGUSERNAME))
            {
              if (now-sptr->when>=t)
               {
                say ("%$% Auto-Closing timed out 3Telnet from "
                     "[3%s].\n", INFO, sptr->haddress);

                (*sptr->close_func)(sptr, 3);
               }
            }
         }
     }

 }  /**/

