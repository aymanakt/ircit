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
#include <sockets.h>
#include <people.h>
#include <history.h>
#include <lastlog.h>
#include <chanrec.h>
#include <ignore.h>
#include <misc.h>
#include <notify.h>
#include <updates.h>
#include <prefs.h>
#include <events.h>

 MODULEID("$Id: serverr.c,v 1.8 1998/04/08 08:54:11 ayman Beta $");

#define CTCP_CHAR 0x1
#define PREFEX "~+-^="
#define ISCHANNEL(x) ((*(x)=='#')||(*(x)=='&')||(*(x)=='+'))

 int whois_busy,
     serror;
 extern int away;
 extern time_t away_when;
 extern int whois_issued;
 extern time_t aux;

 extern ChannelsTable *const channels;
 extern InputHistory *tab;
 extern chtype *f;
 extern chtype *chf;

 STD_IRC_SERVER(sPING)

 {
  char *r;
   rest?(r=(++rest)):(r="");

   //ToServer ("PONG %s\n", rest?++rest:"");
   ToServer ("PONG %s\n", r); 
   say ("sent 'PONG %s'\n", r);
 }  /**/


 STD_IRC_SERVER(sPONG)

 {
   st_ptr->c_serverlag=time (NULL);
   UpdateServerLag ();
/*
   st_ptr->c_serverlag=time(NULL)-tt_ptr->ping_replay;
   UpdateServerLag ();
   st_ptr->c_serverlag=0;
*/  
 }  /**/


 STD_IRC_SERVER(sERROR)

 {
   beep();
   say ("%$% %s\n", SRV_HOT, rest);

 }  /**/


 STD_IRC_SERVER(sINVITE)

 {
  char *n,
       *h,
       *v;

    if (n=splitn(&from), !from)  from="*@*";
    if (v=splitw(&rest), ((rest)&&(*rest==':')))  rest++;

    if ((mt_ptr->c_ignore&IG_INVITE)==0)
     {
      char s[MAXHOSTLEN];

       FIXIT(from);
       sprintf (s, "%s!%s", n, from);

        if (IsSheIgnored(s, IG_INVITE))
         {
          return;
         }
     }
    else
     {
      return;  /* global invite-ignore in place */
     }


   {
    chtype *chs;
    register *auxptr;

     chs=PreSay ("%$% %s [%s] invites you to %s.\n" 
                 "%$% Type /JOIN or press CTRL-F to join %s.\n",
                 SRV_INFO, n, from, rest,
                 INFO, rest);

#ifdef LAST__LOG
       if (isLogLevel(LOG_MISC))
        {
         AddtoLastLog (LOG_MISC, NULL, chs);
        }
#endif

      if ((auxptr=EventRegistered(EVNT_INVITE)))
       {
        ExecHook (auxptr, "%s %s %s", n, from, rest);
       }
      else
       {
        nYelll (chs);
       }
   }
   
    if (set("JOIN_ON_INVITE"))
     {
      say ("%$% Auto-Joining channel %s...\n", INFO, rest);
      ToServer ("JOIN %s\n", rest);

      return;
     }

   strcpy (st_ptr->invited_to,  rest);

 }  /**/  


 STD_IRC_SERVER(sMOTD)

 {
    if (set("SHOW_MOTD"))
     {
      splitw (&rest);

      say ("%$% %s\n", SRV_MOTD, *rest==':'?++rest:rest);
     }

   st_ptr->c_serverlag=time (NULL);

 }  /**/


 STD_IRC_SERVER(sPRIVMSG)

 {
  char *c,
       *n;
  chtype *chs;  
  register Channel *chptr;
  static const int ch_c[]={0, 4, 3, 4, 2, 6, 9, 0, 4, 3, 2, 6, 9, 0};
  extern dList *msglog;

    if (n=splitn(&from), (!from))  from="*@*";
    if (c=splitw(&rest), ((rest)&&(*rest==':')))  rest++;

    if (ISCHANNEL(c))
     {
       if ((chptr=ChannelbyName(c))==(Channel *)NULL)
        {
         say ("%$% Possible desync - can't find %c!\n", MILD, c);
         say ("[%s:%s] %s\n", n, c, rest);
             
         return;
        }

      if (*rest==CTCP_CHAR)
       {
        register char *p, 
                      *q;

         rest++; /* 'ACTION... */
          if (!(q=strrchr(rest, '')))
           {
            say ("%$% Badly formatted 6CTCP %s request from %s[%s].\n",
                 CTCPH, rest, n, from);

            return;
           }

         *q=0;
          if ((p=splitw(&rest)), (!rest))  rest="";

         {
          register void *auxptr;

            if ((auxptr=EventRegistered(EVNT_CHANCTCP)))
             {
              ExecHook (auxptr, "%s %s %s %s %s", n, from, c, p, rest);
             }
            else
             {
               if (!strcasecmp(p, "ACTION"))
                {
                 if (iscurchan(c))
                  {
                   say ("%$%*%$% %s %s\n", 
                        chf[chptr->id], chf[chptr->id+10], n, rest);
                  }
                 else
                  {
                   say ("%$%*%$% <%s:%s> %s\n", 
                        chf[chptr->id], chf[chptr->id+10], n, c, rest);
                  } 
                }
               else
                {
                say("%$% Unsupported 6CTCP %s sent to %s by %s[%s].\n",
                    CTCPH, p, c, n, from);
                }
             } 
         } 
       }
      else
       {
        register void *auxptr;

          if (iscurchan(c))
           {
            chs=PreSay ("%$%%s%$% %s\n",
                        chf[((Channel *)(cht_ptr->c_chptr))->id], n,
                        chf[((Channel *)(cht_ptr->c_chptr))->id+10], rest);
           }
          else
           {
            chs=PreSay ("%$%%s:%s%$% %s\n", chf[chptr->id], n, c,
                        chf[chptr->id+10], rest);
           }

#ifdef LAST__LOG
          if (IS_CHLOGLEVEL(chptr, LOG_PUB))
           {
            AddtoLastLog (LOG_PUB, chptr, chs);
           }
#endif

          if ((auxptr=EventRegistered(EVNT_CHANPRIVMSG)))
           {
            ExecHook (auxptr, "%s %s %s", n, from, rest);
           }
          else
           {
            nYelll (chs);
           }
       }
     }
    else 
     {
       mstrncpy (lastmsg_n(), n, MAXNICKLEN);
       mstrncpy (lastmsg_h(), from, MAXHOSTLEN);

        if (away)
         {
          UpdateHistoryTable (H_AWAYMSGS, 0, '+'); 
         }

        if (*rest==CTCP_CHAR)
         {
          ParseCTCP (n, from, rest);

          return;
         }

       FIXIT(from);
     
        if ((mt_ptr->c_ignore&IG_MSG)==0)
         {
          char s[MAXHOSTLEN];

           sprintf (s, "%s!%s", n, from);

            if (!IsSheIgnored(s, IG_MSG))
             {
              register void *auxptr;

               chs=PreSay ("[%s!%s] %s\n", n, from, rest);

               if ((auxptr=EventRegistered(EVNT_USRPRIVMSG)))
                {
                 ExecHook (auxptr, "%s %s %s", n, from, rest);
                }
               else
                {
                  if (IS_BEEPONMSG())  beep();

                 nYelll (chs);
                }
#ifdef LAST__LOG
               if (isLogLevel(LOG_MSG))
                {
                 AddtoLastLog (LOG_MSG, NULL, chs);
                }
#endif
             }
         }
      }

 }  /**/
   

 STD_IRC_SERVER(sJOIN)

 {
  char *n;
  register chtype *chs;
  register Channel *chptr;

    if ((n=splitn(&from)), (!from))  from="*@*";
   FIXIT(from);
    
    if (isme(n))  
     {
       if ((chptr=(Channel *)AddChannel(rest+1))!=(Channel *)NULL)
        {
         register void *auxptr;
         //say ("%$% You have joined %s.\n", f[chptr->id], chptr->channel);

         AdjustChannels (channels, NULL, 1);
         UpdateUpper ();

          if ((auxptr=EventRegistered(EVNT_CHANJOIN)))
           {
            ExecHook (auxptr, "%s", chptr->channel);
           }
          else
           {
            say ("%$% You have joined %s.\n", f[chptr->id], chptr->channel);
           }
        }
     }
    else
     {
      register ChanUser *uptr;

       chptr=ChannelbyName (rest+1);

        if (chptr==(Channel *)NULL)
         {
          say ("%$% Can't add user %s to non-existent channel %s!\n", MILD,
               n, rest+1);

          return;
         }

       uptr=(ChanUser *)userbychptr (chptr, n);

        if (uptr!=NULL)
         {
          char s[strlen(uptr->host)+strlen(uptr->user)+4];

           sprintf (s, "%s@%s", uptr->user, uptr->host);

            if ((arethesame(s, from))&&(uptr->split)) /* looks like him */
             {
              uptr->split=0L;

              chs=PreSay ("%$% %s returned from netsplit...\n", f[chptr->id],n);

              nYelll (chs);

#ifdef LAST__LOG
               if (IS_CHLOGLEVEL(chptr, LOG_JOIN))
                {
                 AddtoLastLog (LOG_JOIN, chptr, chs);
                }
#endif

              return;
             }
            else 
             {
              RemoveUserFromChannel(rest+1, n);

               if (iscurchan(rest+1))
                {
                 UpdateUpperBorder (CH_USERS, 0, 0, '-');
                }
             }
         }

        if ((uptr=(ChanUser *)AddChanuser(chptr, n))!=NULL) 
         {
          ToServer ("WHO %s\n", n);

          uptr->when=time (NULL);

           if (iscurchan(chptr->channel))
            {
             UpdateUpperBorder (CH_USERS, 0, 0, '+');
            }
          
          {
           register void *auxptr;

            chs=PreSay ("%$% %s[%s] has joined %s.\n",
                        f[chptr->id], n, from, chptr->channel);

#ifdef LAST__LOG
             if (IS_CHLOGLEVEL(chptr, LOG_JOIN))
              {
               AddtoLastLog (LOG_JOIN, chptr, chs);
              }
#endif
             if ((auxptr=EventRegistered(EVNT_CHANUSRJOIN)))
              {
               ExecHook (auxptr, "%s %s %s", n, from, chptr->channel);
              }
             else
              {
               nYelll (chs);
              }
          } 
         }
     }


 }  /**/


 STD_IRC_SERVER(sPART)

 {
  int cid;
  char *n,
       *c;
  register chtype *chs;
  register Channel *chptr;

    if ((n=splitn(&from)), (!from))  from="*@*";
   c=splitw (&rest);

    if (rest)
     {
      if (*rest==':')
       {
        rest++;
       }
     }

   cid=id (c);

    if (isme(n))
     {
      say ("%$% You have left %s.\n", f[cid+10], c);

      RemovefromChannelsTable (channels, c);
      AdjustChannels (channels, NULL, 1);
      UpdateUpper ();
     }
    else
     {
      if ((chptr=(Channel *)RemoveUserFromChannel(c, n)))
       {
         if (iscurchan(c))
          {
           UpdateUpperBorder (CH_USERS, 0, 0, '-');
          }

        {
         register void *auxptr;

          chs=PreSay ("%$% %s has left channel %s (%s)\n",
                      f[cid+10], n, c, rest?rest:"");

#ifdef LAST__LOG
           if (IS_CHLOGLEVEL(chptr, LOG_PART))
            {
             AddtoLastLog (LOG_PART, chptr, chs);
            }
#endif

           if ((auxptr=EventRegistered(EVNT_CHANUSRPART)))
            {
             ExecHook (auxptr, "%s %s %s", n, from, c);
            }
           else
            {
             nYelll (chs);
            }
        }
       }
      else
       {
        say ("%$% %s %s %s [%s] (wasn't in chan record!)\n",
             f[cid+10], n, "has left channel", c, rest?rest:" ");
       }
     }

 }  /**/


 STD_IRC_SERVER(sKICK)

 {
  int cid;
  char *n,
       *c,
       *v;
  register chtype *chs;
  register Channel *chptr;
   
   c=splitw (&rest);
    if ((v=splitw(&rest)), (!(++rest)))  rest="";
    if (n=splitn(&from), !from)  from="*@*";

    if (!(chptr=ChannelbyName(c)))
     {
      say ("%$% Possible desync - Unable to find internal representation"
           " of %s\n" "  %$192%$196%$16 %s kicked off %s by %s [%s]\n", MILD,
           c, v, c, n, from?from:"");
 
      return;
     }

   cid=chptr->id;

    if (isme(v))
     {
      {
       register void *auxptr;

         if ((auxptr=EventRegistered(EVNT_CHANKICK)))
          {
           ExecHook (auxptr, "%s %s %s %s", n, from, chptr->channel, rest);
          }
         else
          {
           beep();

            if ((rest)&&(*rest))
             {
              say ("%$% You have been kicked off %s by %s.\n" 
                   "  %$192%$196%$16 %s\n", 
                   f[chptr->id], c, n, rest);
             }
            else
             {
              say ("%$% You have been kicked off %s by %s.\n", 
                   f[chptr->id], c, n);
             }

            if (set("REJOIN_ON_KICK"))
             {
              ToServer ("JOIN %s\n", c);
             }
          }
      }

      RemovefromChannelsTable (channels, c);
      AdjustChannels (channels, NULL, 1);
      UpdateUpper (); 
     }
    else
     {
       if (RemoveUserFromChannel(c, v))
        {
          if (iscurchan(c))
           {
            UpdateUpperBorder (CH_USERS, 0, 0, '-');
           }
        }

      {
       register void *auxptr;

        chs=PreSay ("%$% %s has been kicked off %s by %s (%s)\n",
                    f[chptr->id], v, c, n, rest);

#ifdef LAST__LOG
         if (IS_CHLOGLEVEL(chptr, LOG_KICK))
          {
           AddtoLastLog (LOG_KICK, chptr, chs);
          }
#endif

         if ((auxptr=EventRegistered(EVNT_CHANUSRKICK)))
          {
           ExecHook (auxptr, "%s %s %s %s %s", n, from, v, chptr->channel,rest);
          }
         else
          {
           nYelll (chs); 
          }
      }
     }

 }  /**/


 STD_IRC_SERVER(sNICK)

 {
  char *n;
  register chtype *chs;

    if (n=splitn(&from), (!from))  from="*@*";

    if (isme(n))
     {
      chs=PreSay ("%$% Changed your nick from %s to %s.\n", 
                  SRV_INFO, curnick(),rest+1);
 
#ifdef LAST__LOG
       if (isLogLevel(LOG_MISC))
        {
         AddtoLastLog (LOG_MISC, NULL, chs);
        }
#endif

      nYelll (chs);
 
      UpdateChanuserNick (n, rest+1);
      UpdateServerTable (SERV_NICK, 0, rest+1, 0);

      ToServer ("USERHOST %s\n", rest+1);
     }
    else
     {
      UpdateChanuserNick (n, rest+1);

      {
       register void *auxptr;

         if ((auxptr=EventRegistered(EVNT_NICKCHANGE)))
          {
           ExecHook (auxptr, "%s %s %s", n, from, rest+1);
          }
         else
          {
           say ("%$% %s changed his nick to %s.\n", SRV_INFO, n, rest+1); 
          }
      }
     }

 }  /**/


 STD_IRC_SERVER(sNOTICE)

 {
  char *n="",
       *t="";
  chtype *chs;
  register Channel *chptr;

    if ((!from)||(!*from))
     {
      splitcolon (&rest);
      say ("%$% %s\n", SRV_MILD, rest?rest:"");

      return;
     }

    if (n=splitn(&from), (!from))  from="*@*";
    if (t=splitw(&rest), (!rest))  rest="";
    else  rest++;

    if (ISCHANNEL(t))
     {
       if (!(chptr=ChannelbyName(t)))
        {
         say ("%$% Possible de-sync - NOTICE recieved for channel %s:\n"
              " -^_%s:%s^_- %s\n",
              MILD, n, t, rest);

         return;
        }

       if (*rest==CTCP_CHAR)
        {
         char *aux;

           if (aux=strrchr(++rest, ''))  *aux=0;

          chs=PreSay ("%$% 6CTCP REPLAY %s sent to channel %s by %s[%s].\n",
               CTCPH, rest, t, n, from);
        }
       else
        {
         chs=PreSay ("-%s:%s- %s\n", n, t, rest);
        }

#ifdef LAST__LOG
       if (IS_CHLOGLEVEL(chptr, LOG_PUB))
        {
         AddtoLastLog (LOG_PUB, chptr, chs);
        }
#endif

      nYelll (chs);
     }
    else
     {
       if (*rest==CTCP_CHAR)
        {
         ParsenCTCP (n, from, rest);
        }
       else
        {
          if ((mt_ptr->c_ignore&IG_NOTICE)==0)
           {
            char s[MAXHOSTLEN];

             FIXIT(from);
             sprintf (s, "%s!%s", n, from);

              if (!IsSheIgnored(s, IG_NOTICE))
               {
                chs=PreSay ("-%s- %s\n", n, rest);
#ifdef LAST__LOG
                 if (isLogLevel(LOG_NOTC))
                  {
                   AddtoLastLog (LOG_NOTC, NULL, chs); 
                  }
#endif

                nYelll (chs);
               }
           }
        }
     }

 }  /**/
  

 STD_IRC_SERVER(sMISC)
 
 {
   splitw (&rest);

   say ("%$% %s\n", SRV_INFO, *rest==':'?++rest:rest);

 }  /**/


 STD_IRC_SERVER(sMISC_H)

 {
   splitw (&rest);

   say ("%$% %s\n", SRV_MILD, *rest==':'?++rest:rest);

 }  /**/


 STD_IRC_SERVER(sQUIT)

 {
  int found=0,
      split=0;
  char *n,
       *p;
  chtype *chs;
  register Channel *chptr;
  register ChanUser *uptr;

    if (n=splitn(&from), (!from))  from="*@*";
   FIXIT(from);

    for (p=rest+strlen(rest)-1; ((isspace(*p))&&(p>=rest)); p--); *(p+1)=0;

   rest++;

   p=strchr (rest, ' ');

    if ((p)&&(p==strrchr(rest, ' '))) 
     {
      char *x, 
           *y;

       *p=0;
       x=strchr (p+1, '.');
       y=strchr (rest, '.');

        if ((x)&&(y)&&(*(x+1)!=0)&&(x-1!=p)&&(y+1!=p)&&(y!=rest))
         {
          split=1;
         } 
        else
         {
          *p=' ';
         }
     }

    if (split)
     {
      chs=PreSay ("%$% NETSPLIT %s left IRC (%s %s)%~\n"
                  "  %$192%$196%$16 channel(s):", SRV_MILD, n, rest, p+1);
     }
    else
     {
      chs=PreSay ("%$% SIGN-OFF %s left IRC (%s)%~\n"
                  "  %$192%$196%$16 channel(s):", SRV_INFO, n, rest);
     }

   chptr=channels->head;

    while (chptr!=NULL)
     {
      uptr=(ChanUser *)userbychptr (chptr, n);

#ifdef LAST__LOG
       if (IS_CHLOGLEVEL(chptr, LOG_PART))
        {
         AddtoLastLog (LOG_PART, chptr, chs);
        }
#endif
       if (uptr!=NULL)
        {
         found++;

          if (split)
           {
            nYelll (chs);

            uptr->split=time (NULL);
            chptr->nsplit++;
           }
          else
           {
            nYelll (chs);

            RemoveChanUserbychptr (chptr, n);

             if (iscurchan(chptr->channel))
              {
               UpdateUpperBorder (CH_USERS, 0, 0, '-');
              }
           }

         say (" %s", chptr->channel);
        }

      chptr=chptr->next;
     }

    if (!found)
     {
      say (" No channel information available!\n");

      return;
     }

   say (".\n"); 

    if (split)
     {
      if (!istimeractive(7))
       {
        ActivateTimer (7, NULL);
       }
     }

 }  /**/
           

 STD_IRC_SERVER(sTOPIC)

 {
  char *n,
       *c;
  chtype *chs;
  register Channel *chptr;

    if (n=splitn(&from), (!from))  from="*@*";
    if (c=splitw(&rest), (!rest))  rest="";
   FIXIT(from);
  
    if ((chptr=ChannelbyName(c))!=NULL)
     {
      rest++;

       if (chptr->topic)
        {
         free (chptr->topic);
        }

      chptr->topic=strdup (rest);

      chs=PreSay("%$% New topic for %s by %s.\n  %$192%$196%$16 %$174%s%$175\n",
                 f[chptr->id], c, n, chptr->topic);

#ifdef LAST__LOG
       if (IS_CHLOGLEVEL(chptr, LOG_OTHR))
        {
         AddtoLastLog (LOG_PART, chptr, chs);
        }
#endif

      {
       register void *auxptr;

         if ((auxptr=EventRegistered(EVNT_TOPIC)))
          {
           ExecHook (auxptr, "%d %s %s %s", chptr->id, n, from, rest);
          }
         else
          {
           nYelll (chs);
          }
      }
     }

 }  /**/


 STD_IRC_SERVER(s001)

 {
  register char *n;

    if ((n=splitw(&rest)), (!rest))  rest="";

   say ("%$% %s\n", SRV_INFO, *rest==':'?++rest:rest);

   mt_ptr->welcomed=1;

   UpdateServerTable (SERV_NICK, 0, n, 0);
   /*DisplayServerTable (SERV_NICK);*/

   ActivateTimer (1, NULL);
   ActivateTimer (2, NULL);

   ToServer ("USERHOST %s\n", curnick());

 }  /**/


 #define NFLAGS(x) ((Notify *)(x)->whatever)->flags
 #define NNAME(x) ((Notify *)(x)->whatever)->name
 #define NTIME(x) ((Notify *)(x)->whatever)->when

 STD_IRC_SERVER(s303)

 {
  char t[20];
  register char *s;
  chtype *chs;
  register ListEntry *nptr;
  extern List *irc_notify;

   splitcolon (&rest);

    while (rest) 
     {
      s=splitw (&rest); 
      nptr=irc_notify->head;

       for ( ; nptr!=NULL; nptr=nptr->next)
        {
          if (arethesame(NNAME(nptr), s))
           {
             if ((NFLAGS(nptr)&1)==0) //was off
              {
               NTIME(nptr)=time(NULL);
               NFLAGS(nptr)|=(1|2|8);
               NFLAGS(nptr)&=~4;
               ToServer ("USERHOST %s\n", s);

                if (set("WHOIS_ON_NOTIFY"))
                 {
                  ToServer ("WHOIS %s\n", s);
                 }

                if (strcmp(NNAME(nptr), s))
                 {
                  strcpy (NNAME(nptr), s);
                 }
              }
             else
              {
               NFLAGS(nptr)|=4;
              }
           }
        }
     }

   nptr=irc_notify->head;

    for ( ; nptr!=NULL; nptr=nptr->next)
     {
       if ((NFLAGS(nptr)&1)&&!(NFLAGS(nptr)&2)&&!(NFLAGS(nptr)&4))
        {
         NTIME(nptr)=time (NULL);
         strftime (t, 15, "%I:%M %p", localtime(&NTIME(nptr)));
         chs=PreSay ("%$% IRC sign-off at %s for %s detected.\n", 
                     INFO, t, NNAME(nptr));

#ifdef LAST__LOG
       if (isLogLevel(LOG_MISC))
        {
         AddtoLastLog (LOG_MISC, NULL, chs);
        }
#endif
         nYelll (chs);
         NFLAGS(nptr)&=~1;
         UpdateHistoryTable (H_NOTIFY, 0, '-');
        }

      NFLAGS(nptr)&=~4;
     }

 }  /**/
 

 //:irc.stealth.net 322 Rohanoul #systeembeheerders 1 :Als je systeembe
 STD_IRC_SERVER(s322)

 {
  register char *c,
                *aux;

   splitw (&rest);
   c=splitw (&rest);
   aux=splitw (&rest);

    while ((rest)&&(*rest)&&(*rest++!=':'));

   say ("%$% %s [%d]\n  %$192%$196%$16 %s\n", SRV_INFO, c, atoi(aux), 
        *rest?rest:"");

 }  /**/


 STD_IRC_SERVER(s324)

 {
  char t[20],
       *c,
       m[100],
       buf[50];
  register Channel *chptr;

   splitw (&rest); 
   c=splitw (&rest);
   strcpy (m, rest?rest:"");

    if ((chptr=ChannelbyName(c)))
     {
      RecordChanMode (chptr, m);

       if (iscurchan(c))
        {
         AdjustChannels (channels, NULL, 1);
         UpdateUpper ();
        }
       else
        {
         ParseChanmode (c, buf);
         say ("%$% Updated  chanmode for %s (%s)\n", f[chptr->id], c, buf);
        } 
     }
    else
     {
      say ("%$% Channel mode for %s is %s\n", SRV_INFO, c, m);
     }

 }  /**/

 
 STD_IRC_SERVER(s314)

 {
  int i=0, *ii=&i;
  char *u,
       *n,
       *h;

   splitw (&rest);
   n=splitw (&rest);
   u=splitw (&rest);
   h=splitw (&rest);
   splitcolon (&rest);

   say (" %$218%$196,2( WHOWAS info for %s )%n%$196,&\n", 
        n, &i, &i);

   say (" %$179 Address:     %s@%s\n"
        " %$179 Real name:   %s\n", u, h, rest?rest:" ");

 }  /**/


 STD_IRC_SERVER(s311)

 {
  int i=0;
  char *u,
       *n,
       *h;

   splitw (&rest);
   n=splitw (&rest);
   u=splitw (&rest);
   h=splitw (&rest);
   splitcolon (&rest);

   say (" %$218%$196,2( WHOIS info for %s )%n%$196,&\n",
        n, &i, &i);
   say (" %$179 Address:     %s@%s\n" 
        " %$179 Real name:   %s\n", u, h, rest?rest:" ");

 }  /**/


 STD_IRC_SERVER(s319)

 {
   splitw (&rest);
   splitw (&rest);

   say (" %$179 On channels: %s\n", rest+1);

 }  /**/ 


 STD_IRC_SERVER(s313)

 {
   splitw (&rest);
   splitw (&rest);

   say (" %$179 %s\n", rest+1);

 }  /**/


 STD_IRC_SERVER(s312)

 {
  char *s;

   splitw (&rest);
   splitw (&rest);
   s=splitw (&rest);
   rest++;

   say (" %$179 Via server:  %s\n"
        " %$179 Server info: %s\n", s, rest?rest:" ");

 }  /**/


 STD_IRC_SERVER(s301)
 
 {
  char *n;

   splitw (&rest);
   n=splitw (&rest);

    if (whois_issued)
     {
      say (" %$179 Away:       %s\n", rest+1);
     }
    else
     {
      say ("%$% %s is away: %s\n", SRV_INFO, n, rest+1);
     }

 }  /**/


 #define NFORMAT "%$% IRC sign-on at %s for %s [%s] detected.\n"

 STD_IRC_SERVER(s302)

 {
  char t[20];
  char *n;
  chtype *chs;
  register ListEntry *nptr;
  extern List *irc_notify;

   splitcolon (&rest);

    if ((!rest)||(!*rest))
     {
      return;
     }
    else
     {
      n=tokenize (&rest, '=');
      rest++;

       if ((isme(n)))
        {
          if (strchr("~+-^", *rest))
           {
            rest++;
           }

         sprintf (mt_ptr->myuserhost, "%s!%s", curnick(), rest);
        }

       if (irc_notify->nEntries)
        {
         nptr=irc_notify->head;

          for ( ; nptr!=NULL; nptr=nptr->next)
           {
             if ((NFLAGS(nptr)&8)&&(arethesame(n, NNAME(nptr))))
              {
                if ((NFLAGS(nptr)&1)&&(NFLAGS(nptr)&2)&&!(NFLAGS(nptr)&4))
                  {
                   strftime (t, 15, "%I:%M %p", localtime(&NTIME(nptr)));

                   chs=PreSay (NFORMAT, IT_INFO, t, NNAME(nptr), rest);

#ifdef LAST__LOG
                    if (isLogLevel(LOG_MISC))
                     {
                      AddtoLastLog (LOG_MISC, NULL, chs);
                     }
#endif
                   nYelll (chs);

                   NFLAGS(nptr)&=~2;
                   NFLAGS(nptr)&=~8;
                         
                   UpdateHistoryTable (H_NOTIFY, 0, '+'); 

                   return;
                  }
              }
           }
        }

      say ("%$% %s happens to be %s\n", SRV_INFO, n, rest);
     } 

 }  /**/


 STD_IRC_SERVER(s317)

 {
  char *s;

   splitw (&rest);
   splitw (&rest);
   s=splitw (&rest);

    if ((s)&&(*s))
     {
      say (" %$179 Idle:        %s seconds\n\n", secstostr(atoi(s), 1));
     }
    else
     {
      say (" %$179 Idle:        \n\n");   
     }

 }  /**/


 STD_IRC_SERVER(s318)

 {
   whois_issued=0;

 }  /**/


 STD_IRC_SERVER(s329)

 {
  char *c;
  unsigned long t;
  register Channel *chptr;

   splitw (&rest);
   c=splitw (&rest);

   chptr=ChannelbyName (c);

    if (chptr!=(Channel *)NULL)
     {
      t=atoul (rest);

      say ("%$% Channel created %s", f[chptr->id+10], ctime((time_t *)&t));
     }

 }  /**/


 STD_IRC_SERVER(s332)

 {
  char *c;
  register Channel *chptr;

   splitw (&rest);
   c=splitw (&rest);

    if ((chptr=ChannelbyName(c))!=NULL)
     {
      rest++;

       if (chptr->topic)  free (chptr->topic);

      chptr->topic=strdup (rest);

      say ("%$% Topic for %s: %s\n", f[chptr->id+10], 
           chptr->channel, chptr->topic);
     }

 }  /**/


 STD_IRC_SERVER(s333)

 {
  char *c,
       *n;
  unsigned long t;
  register Channel *chptr;

   splitw (&rest);
   c=splitw (&rest);
   n=splitw (&rest);

   chptr=ChannelbyName (c);

    if (chptr!=(Channel *)NULL)
     {
      t=atoul (rest);

      say ("%$% Topic created by %s, %s", f[chptr->id+10], 
           n, ctime((time_t *)&t)); 
     }

 }  /**/


 STD_IRC_SERVER(s352)

 {
  char *c;
  register Channel *chptr;
  extern ChannelsTable *channels;

   splitw (&rest);
   c=splitw (&rest);

   chptr=ChannelbyName (c);

    if ((chptr!=NULL)&&(chptr->status&PROCESSINGWHOREPLAY))
     {
      ProcessUser (rest);
     }
    else
    if (chptr!=NULL)
     {
      ProcessUser (rest);
     }
    /*else
     {
      say ("%$% %s %s\n", SRV_INFO, c, rest);
     }*/

 }  /**/
 static char s_[600];


 STD_IRC_SERVER(s353)

 {
  char *c,
       *n;
  char *p;
  int i=0;
  register Channel *chptr;

   splitw (&rest);
   splitw (&rest);
   c=splitw (&rest);
   rest++;

    for (p=rest+strlen(rest)-1; ((isspace(*p))&&(p>=rest)); p--)
        ;
   *(p+1)=0;

   chptr=ChannelbyName (c);

    if ((chptr!=NULL)&&(chptr->status&PROCESSINGNAMESLIST))
     {
      register ChanUser *uptr;

       say ("%$% Users on %s:\n", f[chptr->id+10], c);

        do
         {
          i++;
          n=splitw (&rest);
           
           if ((*n=='@')||(*n=='+'))
            {
             uptr=(ChanUser *)AddChanuser (chptr, &n[1]);
             *n=='@'?
              ({uptr->usermodes|=USER_CHANOP; chptr->nchanop++; 1;}):
              ({uptr->usermodes|=USER_CHANVOICE; chptr->nchanvoice++; 1;});

              if ((!chptr->chanop)&&(isme(uptr->nick))&&
                  (uptr->usermodes&USER_CHANOP))
               {
                chptr->chanop=1;
                UpdateServerTable (SERV_UMODE, USER_CHANOP, NULL, '+');
               }

             sprintf (&s_[strlen(s_)], " %c%-10s", 
                      uptr->usermodes&USER_CHANOP?'@':'+', 
                      uptr->nick);
            }
           else
            {
             uptr=(ChanUser *)AddChanuser (chptr, &n[0]);
             sprintf (&s_[strlen(s_)], "  %-10s", uptr->nick);
            }

           if (i%6==0)
            {
             i=0;
             say ("%s\n", s_); memset (s_, 0, 600);//s_[0]=0;
            }
         }
        while (rest);
     }

    else
     {
      say ("%$% Users on %s:\n", SRV_INFO, c);

       do
        {
         i++;
         n=splitw(&rest);

          if ((*n=='@')||(*n=='+'))
           {
            sprintf (&s_[strlen(s_)], "  %c%s", *n=='@'?'@':'+', &n[1]);
           }
          else
           {
            sprintf (&s_[strlen(s_)], "   %s", &n[0]);
           }
     
          if (i%6==0)
           {
            i=0;
            say ("%s\n", s_); 
            memset (s_, 0, 600);
           }
        }
       while (rest);
     }

    if (s_[0])  say ("%s\n", s_); 

   memset (s_, 0, 600);

 }  /**/


 STD_IRC_SERVER(s366)

 {
  char *c;
  register Channel *chptr;

   splitw (&rest);
   c=splitw (&rest);

    chptr=ChannelbyName (c);

     if (chptr!=NULL)
      {
        if (chptr->status&PROCESSINGNAMESLIST)
         {
          chptr->status&=~PROCESSINGNAMESLIST;
            
          say ("%$% %d user%sand %d chanop%sin %s!\n", f[chptr->id+10], 
               chptr->nEntries,
               chptr->nEntries>1?"s ":" ", 
               chptr->nchanop, 
               chptr->nchanop>1?"s ":" ", chptr->channel);

           if (iscurchan(c))
            {
             AdjustChannels (channels, NULL, 1);
             UpdateUpper ();

              if (chptr->status&FRESHLYJOINED)
               {
                chptr->status&=~FRESHLYJOINED;

                /*EventRegistered (0x04, chptr->channel);*/
               }
            }
         }
      }
     else
      {
       say ("%$% End of NAMES list on channel %s.\n", SRV_INFO, c);
      }

 }  /**/


 STD_IRC_SERVER(s367)

 {
   splitw (&rest);

   say ("%$% %s\n", SRV_INFO, rest);

 }  /**/


 STD_IRC_SERVER(s368)

 {


 }  /**/


 STD_IRC_SERVER(s401)

 {
  char *s;
  extern People *const people;
  extern SocketsTable *const sockets;

   splitw (&rest);
   s=splitw (&rest);

   say ("%$% %s: %s.\n", SRV_MILD, s, rest+1);

    if ((*s!='#')||(*s!='&')||(*s!='+'))
       {
        register Someone *pptr;
        register Index *iptr;
        Socket *sptr;
        DCCParams *dcc;

         pptr=people->head;

          for ( ; pptr!=NULL; pptr=pptr->next)
              {
                if (!strcasecmp(pptr->name, s))
                   {
                    iptr=pptr->head;

                     for ( ; iptr!=NULL; iptr=iptr->next)
                         {
                           if (iptr->protocol==PROTOCOL_DCC)
                              {
                               (DCCParams *)dcc=INDEXTODCC(iptr);
                          
                                if (dcc->flags&AWAITINGCONNECTION)
                                   {
                                    if (dcc->type&DCCCHATTOHIM)
                                       {
                                        say ("%$% %s to %s.\n", INFO,
                                             "Auto-Closing DCC CHAT offer", s);
                                        (Socket *)sptr=INDEXTOSOCKET(iptr);
                                        close (sptr->sock);
                                        RemovefromSocketsTable (sockets, 
                                                                sptr->sock);
                                        RemoveIndexbyptr (pptr, iptr); 
                                     
                                        UpdateSockets (M_CHATS, '-');//mt_ptr->nChats--;

                                        return;
                                       }
                                    else
                                    if (dcc->type&DCCGETTING) 
                                       {
                                        say ("%$% %s (%s %d) to %s.\n", INFO,
                                             "Auto-closing DCC SEND ", 
                                              dcc->file, dcc->size, s);
                                        (Socket *)sptr=INDEXTOSOCKET(iptr);
                                        close (sptr->sock);
                                        RemovefromSocketsTable (sockets, 
                                                                sptr->sock);
                                        RemoveIndexbyptr (pptr, iptr);

                                        UpdateSockets (M_SENDS, '-');//mt_ptr->nSends--;

                                        return;
                                       }
                                   }
                              }
                         }
                   }
              }
       }

 }  /**/


 STD_IRC_SERVER(s402)

 {
  char *s;

  splitw (&rest);
  s=splitw (&rest);

  say ("%$% %s: %s.\n", SRV_MILD, s, rest+1);

 }  /**/
  

 STD_IRC_SERVER(s404)

 {
  char *c;

   splitw (&rest); 
   c=splitw (&rest);

   say ("%$% You don't have voice mode on channel %s.\n", SRV_MILD, c);

 }  /**/


 STD_IRC_SERVER(s221)

 {
  char *n;

   n=splitw (&rest);

    if (isme(n))
     {
      int i=0,
          mode=0;
      char *res=rest;

        while (*res++)
         {
           if (*res=='i')
            {
             mode|=USER_INVISIBLE;
            }
           else
           if (*res=='r')
            {
             mode|=USER_RESTRICTED;
            }
           else
           if (*res=='o')
            {
             mode|=USER_CHANOP;
            }
           else
           if (*res=='v')
            {
             mode|=USER_CHANVOICE;
            }
         }

      UpdateServerTable (SERV_UMODE, mode, 0, ' ');
     }
    else
     {
      say ("%$% %s%s%s%s%s\n", MILD,
           "ServerTable dosen't seem to be in sync, updating current ",
           "nick entry. your new nick: ", n,
           "was", curnick());
      UpdateServerTable (SERV_NICK, 0, n, ' '); 
     }

 }  /**/
   

 STD_IRC_SERVER(s305)

 {
  char *n;
  extern TimersTable *const tt_ptr;

    if (!away)
     {
      say ( 
      "%$% Server spitting junk at us: unaway replay, but I wasn't set as away.\n", MILD);

      return;
     }

   n=splitw (&rest);

    if (!strcasecmp(n, curnick()))
     {
      say ("%$% You are no longer marked as being away.\n", SRV_INFO);

      away_when=time(NULL)-away_when;
      say ("  you've been away for %s\n", secstostr(away_when, 1));
      away=0;
      away_when=0L;
      Away (0);
     }
    else
     {
      say ( 
        "%$% In unaway replay from server she called me %s instead of %s. Ignoring...\n", MILD, 
      n, curnick());
     }   

 }  /**/


 STD_IRC_SERVER(s306)

 {
  char *n;
  extern TimersTable *const tt_ptr;

   n=splitw (&rest);

    if (!strcasecmp(n, curnick()))
     {
      say ("%$% You have been marked as being away.\n", SRV_INFO);
      away=1;
      away_when=time (NULL);
      Away (1);
     }
    else
     {
      say ("%$% In away replay from server she called me %s instead of %s."
           " Ignoring...\n", MILD, n, curnick());
     }

 }  /**/


 STD_IRC_SERVER(s315)

 {
  char *c;
  register Channel *chptr;

   splitw (&rest);
   c=splitw (&rest);

   chptr=ChannelbyName (c);

    if ((chptr!=NULL)&&(chptr->status&PROCESSINGWHOREPLAY))
     {
      chptr->status&=~PROCESSINGWHOREPLAY;
     }
 
 }  /**/


 STD_IRC_SERVER(s341)

 {
  char *n;

   splitw (&rest);
   n=splitw (&rest); 

    say ("%$% Inviting %s to channel %s...\n", SRV_INFO, n, rest);

 }  /**/


 STD_IRC_SERVER(s381)

 {
  char *n="";

  n=splitw (&rest);

   if (arethesame(n, curnick()))
    {
     say ("%$% You are now IRC Operator!\n", SRV_INFO);

     lower_status_figures(0, 1, 0);
    }

 }  /**/


 STD_IRC_SERVER(s433)

 {
  char *s1=valueof("DEFAULT_NICK");
  char *s2=valueof("ALT_NICK");
  char *n;
  char *rn=NULL;
  char nick[MAXNICKLEN];

   splitw (&rest);
   n=splitw (&rest);

    if (ISCHANNEL(n))
     {
      say ("%$% Channel %s is temporarily unavailable.\n", SRV_MILD, n);

      return;
     }

   say ("%$% Nickname (%s) is already in use!\n", SRV_MILD, n);

   ToServer ("WHOIS %s\n", n);
   whois_issued=1;

    if (arethesame(n, st_ptr->c_nick))
     {
       if (s1)
        {
          if (arethesame(s1, n))
           {
            if (s2)
             {
              if (!arethesame(s2, n))
               {
                rn=s2; 
               } /* else make up one..*/
             }
           }
          else
           {
            rn=s1;
           }
        }
       else
       if (s2)
        {
         if (!arethesame(s2, n))
          {
           rn=s2; 
          } /* else make up one..*/
        }
     }
    else
     {
      ToServer ("NICK %s\n", st_ptr->c_nick);

      return;
     }
    
    if (!rn)  /* make up one */
     {
      register int i;
      char *betu="aZbYcXdWeVfUgThSiRPlOmNJKLMnLoKpAqBrCsDtEuFvGwHxIyJzQ";

        for (i=0; i<=3; i++)
         {
          *(st_ptr->c_nick+i)?nick[i]=*(st_ptr->c_nick+i):
                              *(betu+(rand()%(27-i)));
         }
     
        for ( ; i<=8; i++)
         {
          nick[i]=*(betu+(rand()%(27-i)));
         }

       rn=nick;
     }
   
   ToServer ("NICK %s\n", rn);
   /* mstrncpy (curnick(), rn, MAXNICKLEN); */

 }  /**/


 STD_IRC_SERVER(s442)

 {
  char *c;

   splitw (&rest);
   c=splitw (&rest);

   say ("%$% You are not on channel %s.\n", SRV_MILD, c);

 }  /**/


 STD_IRC_SERVER(s443)

 {
  char *n,
       *c;

   splitw (&rest);
   n=splitw (&rest);
   c=splitw (&rest);
   rest++;

   say ("%$% %s %s %s\n", SRV_INFO, n, rest, c);

 }  /**/


 STD_IRC_SERVER(s451)

 {
   say ("%$% You have not registered yet!\n", SRV_HOT);
   PostConnection ();

 }  /**/


 STD_IRC_SERVER(s473)

 {
  char *c;

   splitw (&rest);
   c=splitw (&rest);

   say ("%$% Channel %s is currently invite-only!\n", SRV_MILD, c);

 }  /**/


 STD_IRC_SERVER(s474)

 {
  char *c;

   splitw (&rest);
   c=splitw (&rest);

   say ("%$% You are banned from channel %s!\n", SRV_MILD, c);

 }  /**/


 STD_IRC_SERVER(s475)

 {
  char *c;

   splitw (&rest);
   c=splitw (&rest);

   say ("%$% Channel %s is key-protected!\n", SRV_MILD, c);

 }  /**/


 STD_IRC_SERVER(s477)

 {
  char *c;

   splitw (&rest);
   c=splitw (&rest);

   say ("%$% Channel modes aren't allowed in %s.\n", SRV_MILD, c);

 }  /**/

