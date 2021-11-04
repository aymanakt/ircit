/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
*/

#include <main.h>
#include <terminal.h>
#include <prefs.h>
#include <misc.h>
#include <stack.h>
#include <list.h>
#include <scroll.h>
#include <servers.h>
#include <history.h>
#include <lastlog.h>
#include <chanrec.h>
#include <sockets.h>
#include <updates.h>
#include <notify.h>
#include <people.h>
#include <timer.h>
#include <useri.h>

#include <fcntl.h>
#include <sys/stat.h>

 MODULEID("$Id: useri.c,v 1.9 1998/04/06 06:13:44 ayman Beta $");


#include <msgbox.h>

 STD_IRC_COMMAND(uABOUT)

 {
   About (NULL, 0);

 }  /**/


#define error say ("%$% arguments invalid or incomplete.\n", MILD)
#define you_need_help say ("%$% You really need -h.\n", MILD)
#define YOU_NEED_HELP() \
        say ("%$% You really need -h.\n", MILD); \
        return

#define MISSING_ARGUMENTS() \
        say ("%$% arguments invalid or incomplete.\n", MILD); \
        return

#define HELP(x) ((*(x)=='h')||(*(x)=='H')||(*(x)=='?'))&&(!*((x)+1))
#define ISCHANNEL(x) ((*(x)=='#')||(*(x)=='&')||(*(x)=='+'))
#define IS_ADD(x) ((x)==1)

#define CTCP_CH 0x1

 char tok[10][80];

 int whois_issued=0;
 int away=0;

 time_t away_when=0L,
        uping_replay;

 extern struct UserData data;


 #define IS_AWAY_SET(x) (!strncasecmp((x), "-SET", 4))
 #define IS_AWAY_UNSET(x) (!strncasecmp((x), "-UNSET", 6))

 #define AWAY_REQUEST() (mode==1)

 #define DETERMINE_MODE(x) \
         if (IS_AWAY_SET((x))) \
            { \
              if (!*((x)+4)) \
                 { \
                  mode=1; \
                  *(x)=0; \
                 } \
              else \
                 { \
                   if (*((x)+4)==' ') \
                      { \
                       SPLIT_RMSPC(NULL, (x), ' '); \
                      } \
                        \
                  mode=1; \
                 } \
            } \
         else \
         if (IS_AWAY_UNSET((x))) \
            { \
              if (!*((x)+6)) \
                 { \
                  mode=0; \
                 } \
              else \
                 { \
                  mode=1; \
                 } \
            } \
         else \
            { \
             mode=1; \
            }

 STD_IRC_COMMAND(uAWAY)

 {
  int mode=0;

    if (!args)
     {
       if (!away)
        {
         say ("%$% You weren't away.\n", MILD);
        }
       else
        {
         ToServer ("AWAY\n");
        }

      return;
     }

    if (*args=='-')
     {
      DETERMINE_MODE(args);
     }
    else
     {
      mode=1;
     }

   if (AWAY_REQUEST())
    {
      if (away)
       {
        say ("%$% You already marked as being away.\n", MILD);
       }
      else
       {
         if (!*args)
          {
           ToServer ("AWAY :%s\n", valueof("AWAY_MSG"));
          }
         else
          {
           ToServer ("AWAY :%s\n", args);
          }
       }
    } 
   else
    {
      if (!away)
       {
        say ("%$% You weren't away.\n", MILD);
       }
      else
       {
        ToServer ("AWAY\n");
       }
    }

 }  /**/

 #undef DETERMINE_MODE


 STD_IRCIT_COMMAND(uCD)

 {
  int i;
  char s[XLARGBUF];

    if (!args)
     {
       if (getcwd(s, XLARGBUF))
        {
         say ("%$% Current working directory:\n  %$192%$196%$16 %s\n", 
              INFO, s);
        }
       else
        {
         say ("%$% Unable to read your working directory - %s.\n", MILD,
              strerror(errno));
        }

      return;
     }
    
    if (*args=='-')
     {
       if (!strncasecmp(args, "-SET", 4))
        {
         if (*(args+4)==' ') 
          { 
           SPLIT_RMSPC(NULL, args, ' '); 
          } 
        }
     }

    if (*args=='/')
     {
       if (chdir(args))
        {
         say ("%$% Unable to change directory - %s.\n", MILD, 
              strerror(errno));
        }
       else
        {
         if (getcwd(s, XLARGBUF))
          {
           say ("%$% Current directory is now: %s\n", INFO, s);
          }
        }             
     }

    else
    if (*args=='~')
     {
       if (ut_ptr->homedir)
        {
         sprintf (s, "%s%c%s", ut_ptr->homedir, '/', args+1);
             
          if (chdir(s))
           {
            say ("%$% Unable to change directory - %s.\n", MILD,
                 strerror(errno));            
           }
          else
           {
             if (getcwd(s, XLARGBUF))
              {
               say ("%$% Current directory is now: %s\n", INFO, s);

               return;
              }
           }
        }
       else
        {
         say ("%$% %s\n", MILD,
             "Unable to expand '~' to your $HOME, use absolute path instead!");
        }
     }

    else
     {
       if (getcwd(s, XLARGBUF))
        {
         strcat (s, "/");
         strcat (s, args);
             
          if (chdir(s))
           {
            say ("%$% Unable to change directory - %s.\n", MILD,
                 strerror(errno));
           }
          else
           {
             if (getcwd(s, XLARGBUF))
              {
               say ("%$% Current directory is now: %s\n", MILD, s);

               return;
              }
           } 
        }
       else
        {
         say ("%$% Unable to change directory - %s.\n", MILD, 
              strerror(errno));
        }
     }

 }  /**/


 STD_IRCIT_COMMAND(uCHANNELS)

 {
  int mode=0,
      sub=0;
  int x, y;
  register int j=0;
  char *ch=NULL;
  const Channel *chptr;
  extern ChannelsTable *const channels;

    if (!args)
     {
      ShowChannelsTable (channels, (1-1), channels->nEntries);

      return;
     }

   mode|=1;

   splitargs2 (args, ' ');

   {
    if (arethesame(tok[j], "-LST"))
     {
      j++;
     }
    else
    if (arethesame(tok[j], "-SCRL"))
     {
       if (!*tok[++j])
        {
         ScrollingChannels (NULL, 0);

         return;
        }

      mode&=~1;
      mode|=2;
     }
   }

    if (ISCHANNEL(tok[j]))
     {
      ch=tok[j];
      j++; 
     }
    else
     {
       if (*curchan())
        {
          if (arethesame(tok[j], "."))  j++;
                
         ch=curchan();
        }
       else
        {
         say ("%$% You are afloat.\n", MILD);

         return;
        }
     }

    if (!(chptr=ChannelbyName(ch)))
     {
      say ("%$% You are not on channel %s.\n", MILD, ch);

      return;
     }

    if ((!*tok[j])||(arethesame(tok[j], "-all"))) {sub=0; j++;}
    else
    if (arethesame(tok[j], "-OP")) {sub|=USER_CHANOP; j++;}
    else
    if (arethesame(tok[j], "-VOICE")) {sub|=USER_CHANVOICE; j++;}
    else
    if (arethesame(tok[j], "-IRCOP")) {sub|=USER_IRCOP; j++;}
    else
    if (arethesame(tok[j], "-SPLIT")) {sub=-1; j++;}
    else
    if (arethesame(tok[j], "-LOG")) {sub=-2; j++;}
    else  
       {sub=0;}
    
    if (*tok[j])
     {
       if (!(DetermineRange(tok[j], &x, &y)))
        {
         say ("%$% Conflic-of-Range detected.\n", MILD);

         return;
        }

       if (y>chptr->nEntries)
        {
         say ("%$% Conflict-of-Range detected.\n", MILD);

         return;
        }
     }
    else
     {
      x=1;
      y=chptr->nEntries;
     } 

   switch (mode&(1|2))
    {
     case 1:
          {
            if (sub>0)
             {
              ShowUsers (chptr, sub, (x-1), y);
             }
            else
             {
              switch (sub)
               {
                case -1:
                     ShowSplitUsers (chptr, x-1, y);
                     break;

                case -2:
                     ShowChLastLogs (chptr);
                     break;
               }  
            }

           break;
          }

     case 2:
          {
           ScrollingChanusers ((void *)chptr, 0);
           break;
          }
    }
        
 }  /**/


 STD_IRCIT_COMMAND(uSHOOT)

 {
  int i,
      j,
      k;
  register Socket *sptr;
  extern Socket *so_hash[];
  extern SocketsTable *const sockets;

    if (!args)
     {
      MISSING_ARGUMENTS();
     }

   i=splitargs2 (args, ',');

    for (j=0; j<=i; j++)
     {
       if (!*tok[j])  continue;

      k=atoi (tok[j]);

       if ((!k)||(k>MAXSOCKSCOUNT))
        {
         say ("%$% Invalid socket specified: %s.\n", MILD, tok[j]);
         continue;
        }

       if ((sptr=so_hash[k]), sptr==(Socket *)NULL)
        {
         say ("%$% Socket %d is not connected.\n", MILD, k);
         continue;
        }

       if (sptr->close_func)  (*sptr->close_func)(sptr, 3);
     }
 
 }  /**/ 


 STD_IRCIT_COMMAND(uCTCP)

 {
  int i,
      j;
  char n[LARGBUF],
       t[LARGBUF];

    if (!args)
     {
      MISSING_ARGUMENTS();
     }

   SPLIT_RMSPC(n, args, ' ');

    if (!*args)
     {
      MISSING_ARGUMENTS();
     }

   SPLIT_RMSPC(t, args, ' ');

   i=splitargs2 (n, ',');

    for (j=0; j<=i; j++)
     {
       if (!strcasecmp(t, "PING"))
        {
         uping_replay=time (NULL);
         mCTCPToServer (tok[j], "PING");
        }
       else
       if (!strcasecmp(t, "DCC"))
        {
         char p[LARGBUF];

          sprintf (p, "%s", args);

          uDCC (p);
        }
       else
        {
          if (*args)
           {
            mCTCPToServer (tok[j], "%s %s", strtoupper(t), args);
           }
          else
           {
            mCTCPToServer (tok[j], "%s", strtoupper(t));
           }
        }
     }

 }  /**/


 STD_IRCIT_COMMAND(uDESCRIBE)

 {
  int i,
      j;
  char *c;

    if (!args)
     {
      MISSING_ARGUMENTS();
     }

   c=splitw (&args);

    if (!args)
     {
      MISSING_ARGUMENTS();
     }

   i=splitargs2 (c, ',');

    for (j=0; j<=i; j++)
     {
       if (ISCHANNEL(tok[j]))
        {
          if (iscurchan(tok[j]))
           {
            say ("* %s %s\n", curnick(), args);
           }
          else
           {
            say ("* %s->%s %s\n", 
                 curnick(), tok[j], args);
           }
        }
       else
        {
         say ("* %s->%s %s\n", curnick(), tok[j], args);
        }

      mCTCPToServer (tok[j], "ACTION %s", args);
     }

 }  /**/


 STD_IRCIT_COMMAND(uEXIT)

 {
    if (!args)
     {
       if (isup())
        {
         ToServer ("QUIT :to be continued...\n");
        }

      exit (0);
     }

    if (isup())
     {
       if (*args)
        {
         ToServer ("QUIT :%s\n", args);
        }
       else
        {
         ToServer ("QUIT :IRCIT - Sophistication by design\n");
        }
     }   
 
   exit (0);

 }  /**/


 STD_IRCIT_COMMAND(uHIDE)

 {
  register int j=0;
  time_t when=0L;

   splitargs2 (args, ' ');

    if (arethesame(tok[j], "-T"))
     {
       if (!*tok[++j])
        {
         YOU_NEED_HELP();
        }

       if (!isdigits(tok[j]))
        {
         YOU_NEED_HELP();
        }

      when=atoul (tok[j]);
     }
    else
    if (*tok[j])
     {      
      YOU_NEED_HELP();
     }

    if (!when)
     {
      say ("%$% Performing detachment...\n", INFO);
      SurrenderControllingTerminal ();

      return;
     }
    else
    if (!AddmetoTimers(DOMEONCE, when, "Detach Session", 
                       SurrenderControllingTerminal))
     {
      say ("%$% Unable to schedual Session Detachment.", MILD);

      return;
     }

  say ("%$% Session schedualed to be detached in %lu seconds time.\n", INFO,
       when);

 }  /**/


 STD_IRC_COMMAND(uINVITE)

 {
  register int i,
               j;
  char *n,
       c[MAXCHANLEN+1];
  extern ChannelsTable *const channels;

    if (!args)
     {
      MISSING_ARGUMENTS();
     }

    if (channels->nEntries==0)
     {
      say ("%$% You are afloat.\n", MILD);

      return;
     }

   n=splitw (&args);
   TRIM(args);

    if ((!args)||( (args)&& (arethesame(args, "."))) )
     {
      strcpy (c, curchan());
     }
    else
     {
      if (strlen(args)>MAXCHANLEN)
       {
        say ("%$% Channel name too long.\n", MILD);

        return;
       }
      else
       {
         if (!ISCHANNEL(args))
          {
           strcpy (&c[1], args);
           c[0]='#';
          }
         else
          {
           strcpy (c, args);
          }
       }
     }

   i=splitargs2 (n, ',');

    for (j=0; j<=i; j++)
     {
      ToServer ("INVITE %s %s\n", tok[j], c);
     }

 }  /**/

 
 STD_IRCIT_COMMAND(uIRCIT)

 {
  int mode=0;
  register int j=0;
  char *s;
  unsigned long port=0L;
  register Server *vptr;
  extern SocketsTable *const sockets;
  extern List *servers;

    if (!args)
     {
      MISSING_ARGUMENTS();
     }

   mode|=1;

   splitargs2 (args, ' ');

    if (arethesame(tok[j], "-IRC"))
     {
      j++;
     }
    else
    if (arethesame(tok[j], "-OTHR"))
     {
      mode|=2; mode&=~1;
      j++;
     }
    else
    if (arethesame(tok[j], "-IRCIT"))
     {
      mode|=4; mode&=~1;
      j++;
     }

    if (!*tok[j])
     {
      MISSING_ARGUMENTS();
     }

   s=tok[j];  /* server */
   j++;

   *tok[j]?
    (port=atoul(tok[j])):
    (port=atoul(valueof("DEFAULT_SERVER_PORT")));
    
    if (isdigits(s))
     {
       if (!(vptr=_GetServerfromList(servers, NUMBER, atoi(s))))
        {
         return;
        }

       if ((vptr->port!=port)&&(*tok[j]))
        {
         vptr=AddtoServersList (servers, vptr->server, port);
        }
     }
    else
     {
       if (!(vptr=_GetServerfromList(servers, NAME, s, port)))
        {
         vptr=AddtoServersList (servers, s, port);
        }
     }

   switch (mode&(1|2|4))
    {
     case 1:
          IRCServerConnect (s=vptr->server, vptr->port);
          break;

     case 2:
          _ConnectToServer (s=vptr->server, vptr->port, SOCK_OTHER);
          break;

     case 4:  /* ircit - not supported yet */
          say ("%$% This option is not supported yet.\n", MILD);
          break;
    }

 }  /**/

 #undef DETERMINE_MODE
         

 #define IS_WILDCARD(x) (strchr((x), '*'))
 #define IS_EVERYTHING(x) (!strcmp((x), "-*.*"))

 #define ADD_NICK() \
         strcpy (&s[strlen(s)], uptr->nick); \
         s[strlen(s)]=',' 

 #define MASSKICK() \
         { \
          int i=0; \
          char s[]={[0 ... LARGBUF]=0}; \
                                        \
            if (chptr->nEntries==1) \
               { \
                say ("%$% Cannot do MASS-KICK on %s.\n", MILD, chptr->channel); \
                                                                             \
                return; \
               } \
                 \
           say ("%$% Launching MASS-KICK on channel %s...\n", INFO, c); \
           uptr=chptr->front; \
                              \
            for ( ; uptr!=NULL; uptr=uptr->next) \
                { \
                  if (isme(uptr->nick)) \
                     { \
                      continue; \
                     } \
                       \
                 i++; \
                  if (i%4==0) \
                     { \
                      i=1; \
                      ToServer ("KICK %s %s :%s\n", chptr->channel, s, \
                                *args?args:" "); \
                                                 \
                      memset (s, 0, LARGBUF); \
                      ADD_NICK(); \
                      continue; \
                     } \
                       \
                 ADD_NICK(); \
                } \
           if (*s) \
              { \
               ToServer ("KICK %s %s :%s\n", \
                        chptr->channel, s, *args?args:" "); \
              } \
         }

 #define PROCESS_WILDCARD() \
         { \
          char s[LARGBUF]; \
                           \
           uptr=chptr->front; \
                            \
            for ( ; uptr!=NULL; uptr=uptr->next) \
                { \
                  if (isme(uptr->nick)) \
                     { \
                      continue; \
                     } \
                       \
                 sprintf (s, "%s!%s@%s", uptr->nick, uptr->user, uptr->host); \
                   if (match(tok[j], s)) \
                      { \
                       ToServer ("KICK %s %s :%s\n", \
                                chptr->channel, uptr->nick, *args?args:" "); \
                      } \
                } \
         }
 
         
 STD_IRC_COMMAND(uKICK)

 {
  int i;
  register int j=0;
  char c[MAXCHANLEN],
       v[MAXNICKLEN];
  register Channel *chptr;
  register ChanUser *uptr;
  extern ChannelsTable *const channels;

    if (!args)
     {
      MISSING_ARGUMENTS();
     }

    if (NOCHANNELS(channels))
     {
      say ("%$% You are afloat.\n", MILD);

      return;
     }

   i=splitargs2 (args, ' ');

    if (ISCHANNEL(tok[j]))
     {
      mstrncpy (c, tok[j], MAXCHANLEN);
      splitw (&args), j++;
     }
    else
    if (arethesame(tok[j], "."))
     {
      mstrncpy (c, curchan(), MAXCHANLEN);
      splitw (&args), j++;
     }
    else
     {
      mstrncpy (c, curchan(), MAXCHANLEN);
     }

#if 0
    if ((chptr=ChannelbyName(c))==(Channel *)NULL)
     {
      say ("%$% You are not on channel %s.\n", MILD, c);

      return;
     }

    if (!ischanop(chptr))
     {
      say ("%$% You are not channel operator on channel %s.\n", MILD, c);

      return;
     }
#endif

    if (*tok[j])
     {
      char *aux;

        if (*tok[j+1])  splitw (&args);

       ToServer ("KICK %s %s :%s\n", c, tok[j], args?args:"");
     }
    else
     {
      MISSING_ARGUMENTS();
     }

#if 0
    if (*args=='-')
     {
      if (*curchan())
       {
        strcpy (c, curchan());
       }
     } 
    else
    if (ISCHANNEL(args))
     {
      SPLIT_RMSPC(c, args, ' ');

       if (*args==0)
        {
         MISSING_ARGUMENTS();
        }
     }
    else 
     {
       if (*curchan())
        {
         strcpy (c, curchan());
        }
     }

    if ((chptr=ChannelbyName(c))==(Channel *)NULL)
     {
      say ("%$% You are not on channel %s.\n", MILD, c);

      return;
     }

    if (!ischanop(chptr))
     {
      say ("%$% You are not channel operator on channel %s.\n", MILD, c);

      return;
     }

   SPLIT_RMSPC(v, args, ' ');
   wipespaces (args);

   i=splitargs2 (v, ',');

    for (j=0; j<=i; j++)
        {
          if (*tok[j]=='-')
             {
               if (IS_EVERYTHING(tok[j]))
                  {
                   MASSKICK();

                   return;
                  }
             }
          else
          if (IS_WILDCARD(tok[j]))
             {
              PROCESS_WILDCARD();
              continue;
             }
        
          if ((uptr=(ChanUser *)userbychptr(chptr, tok[j]))==(ChanUser *)NULL)
             {
              say ("%$% User %s not on channel %s.\n", MILD, tok[j], c);
              continue;
             }
 
         ToServer ("KICK %s %s :%s\n", chptr->channel, 
                   uptr->nick, *args?args:" ");     
        }   
#endif
 }  /**/

 #undef PROCESS_WILDCARD


 STD_IRC_COMMAND(uNOTICE)

 {
  char *r;

    if (!args)
     {
      say ("%$% arguments invalid or incomplete.\n", MILD);

      return;
     }

  r=splitw (&args);

   ToServer ("NOTICE %s :%s\n", r, args?args:" ");

   {
    register int i, j;

     i=splitargs2 (r, ',');

      for (j=0; j<=i; j++)
       {
        say ("<-%s-> %s\n", tok[j], args?args:" ");
       }  
   }

 }  /**/


 STD_IRCIT_COMMAND(uSET)

 {
  int disable=0;
  char *s,
       *aux;
  register PrefsTable *pptr;
  extern PrefsTable *preferences;

    if (!args)
     {
      ShowPreferences (preferences, 1);

      return;
     }

   s=splitw (&args);

    if (*s=='-')
     {
       if (args)
        {
         YOU_NEED_HELP();
        }

      s++; 
      disable=1;                 
     }

    if (!(pptr=FindEntry(s)))
     {
      say ("%$% I don't know how to handle %s.\n", MILD, s);

      return;
     }

    if ((!args)&&(!disable))
     {
       if (!pptr->entry.value)
        {
         say ("%$% %s currently unset.\n",INFO,pptr->entry.preference);
        }
       else
        {
         say ("%$% Current value of %s:\n  %$192%$196%$16 %s\n",
              INFO, pptr->entry.preference, pptr->entry.value);
        }

      return;
     }

   TRIM(args);

   {
     if (pptr->entry.level==BOOLEAN)
      {
        if (disable)  args="OFF";

        if ((!arethesame(args, "ON"))&&(!arethesame(args, "OFF")))
         {
          say ("%$% %s takes either ON or OFF.\n", MILD, 
               pptr->entry.preference);

          return;
         } 
      }
     else
     if ((pptr->entry.level==DIGITS)&&(!disable))
      {
        if (!isdigits(args))
         {
          say ("%$% %s takes only numeric values.\n", MILD, 
                pptr->entry.preference);

          return;
         }
      }

    aux=pptr->entry.value;

     if (args)
      {
        if (!(pptr->entry.value=strdup(args)))
         {
          say ("%$% Memory exhaustion.\n", HOT);

          exit (1);
         }
      } 
     else
      {
       pptr->entry.value=(char *)NULL;
      }
   }
   
    if (pptr->entry.func)
     {
      pptr->entry.func(&pptr->entry, aux);
     }

    if (aux)  free (aux);

    if (pptr->entry.value)
     {
      say ("%$% Value of %s set to:\n  %$192%$196%$16 %s\n", INFO,
           pptr->entry.preference, pptr->entry.value);
     }
    else
     {
      say ("%$% %s is Unset.\n", INFO, pptr->entry.preference);
     }

 }  /**/


#include <pwd.h>
#include <grp.h>

 #define LACK() \
         say ("%$% You lack privileges %$%\n", MILD, 1|A_BLINK); \
         return

 STD_IRCIT_COMMAND(uSETUID)

 {
  int mode=0;
  char *u;
  struct passwd *pwd;
  extern void uWHOAMI (char *);

    if (!args)
       {
        uWHOAMI (NULL);

        return;
       }

    if (*args=='-')
       {
         if (!strncasecmp(args, "-e", 2))
            {
              if ((!*(args+2))||(*(args+2)!=' ')) 
                 {
                  mode|=1;
                 }
              else
                 {
                  splitw (&args);
                  mode|=1;
                 }
            }
         else
         if (!strncasecmp(args, "-r", 2))
            {
              if ((!*(args+2))||(*(args+2)!=' ')) 
                 {
                  mode|=1;
                 }
              else
                 {
                  splitw (&args);
                  mode|=2;
                 }
            }
         else
         if (!strncasecmp(args, "-a", 2))
            {
              if ((!*(args+2))||(*(args+2)!=' ')) 
                 {
                  mode|=1;
                 }
              else
                 {
                  splitw (&args);
                  mode|=8;
                 }
            }
         else
         if (!strncasecmp(args, "-back", 5))
            {
              if (!*(args+5)) 
                 {
                  mode|=4;
                 }
              else
                 {
                  mode|=1;
                 }
            }
         else
         if (HELP(args+1))
            {
             say ("%$% Help on SETUID:\n%s\n%s\n%s\n", INFO,
                   "  SETUID [[-type] <name>]",
                   "  SETUID [[-type] <uid>]",
                   "  Supported types: E, R, A, BACK");

             return;
            }
         else
            {
             mode|=1;
            }
       }
    else
       {
        mode|=1;
       }

   u=args;

    if ((mode&4)==0)
       {
         if (isdigits(u)) 
            {
              if (!(pwd=getpwuid((uid_t)atoul(u))))
                 {
                  say ("%$% UID %s doesn't correspond to valid user.\n",MILD,u);

                  return;
                 }
            }
         else
            {
              if (!(pwd=getpwnam(u)))
                 {
                  say ("%$% User %s doesn't have access to this system.\n", MILD,                      u);

                  return; 
                 }
            }
       }

   switch (mode&(1|2|4|8))
          {
           case 1:
                if (seteuid(pwd->pw_uid)<0)
                   {
                    LACK();
                   }
               break;

           case 2:
                if (seteuid(pwd->pw_uid)<0)
                   {
                    LACK();
                   }

                if (setreuid(getuid(), geteuid())<0)
                    {
                     LACK();
                    }

               break;

           case 4:
                if (setuid(ut_ptr->euid)<0)
                    {
                     LACK();
                    }
               break;

           case 8:
                if (setuid(pwd->pw_uid)<0)
                    {
                     LACK();
                    }
               break;
          }

   say ("%$% Your new IDs :\n %$192%$196%$16"
        " UID : %d\n  %$192%$196%$16"
        " EUID: %d\n", INFO, geteuid(), getuid());

 }  /**/


 STD_IRC_COMMAND(uJOIN)

 {
  int i=0,
      j;
  char c[MAXCHANLEN+2]={0};
  register Channel *chptr;
  extern ChannelsTable *const channels;

    if (!args)
     {
       if (st_ptr->invited_to[0])
        {
         ToServer ("JOIN %s\n", st_ptr->invited_to);
         st_ptr->invited_to[0]=0;

         return;
        }
       else
        {
          if (channels->nEntries>0)
           {
            ShowChannelsTable (channels, (1-1), channels->nEntries);
           }
          else
           {
            YOU_NEED_HELP();
           }
        }

      return;
     }
   
   i=splitargs2 (args, ',');

    for (j=0; j<=i; j++)
        {
          if ((*tok[j]!='#')&&(*tok[j]!='&')&&(*tok[j]!='+'))
             {
               if (strlen(tok[j])>MAXCHANLEN)
                  {
                   say ("%$% %s: Channel name too long.\n", MILD, tok[j]);
                   continue;
                  }

              c[0]='#';
              strcpy (&c[1], tok[j]);
             }
          else
             {
               if (strlen(tok[j])>MAXCHANLEN+1)
                  {
                   say ("%$% Channel name too long.\n", MILD);
                   continue;
                  }

              strcpy (&c[0], tok[j]);
             }  

         if ((chptr=ChannelbyName(c))!=(Channel *)NULL)
            {
              if (iscurchan(c))
                 {
                  say ("%$% You currently talking to that channel.\n", MILD);
                 }
              else
                 {
                  say ("%$% You are now talking to channel %s.\n", INFO, c);
                  AdjustChannels (channels, c, 1);
                  UpdateUpper ();
                 }
            }
         else
            {
             ToServer ("JOIN %s\n", c);
            }
        }

 }  /**/


 void uCTRLF (void)

 {
    if (st_ptr->invited_to[0])
       {
        ToServer ("JOIN %s\n", st_ptr->invited_to);
        st_ptr->invited_to[0]=0;

        return;
       }
    else
       {
        say ("%$% No recent channel invitation.\n", MILD);

        return;
       }

 }  /**/


 void uCTRLX (void)

 {
  char *channel;

    if (!isup())
     {
      say ("%$% You are afloat.\n", MILD);

      return;
     }

    if (nchannels()<=1)
     {
      say ("%$% Meaningless....\n", INFO);
        
      return;
     }

    if ((channel=SwitchtoChannel(curchan()))!=(char *)NULL)
     {
      say ("%$% You're now talking to %s.\n", INFO, channel);
     }

 }  /**/
  
 
 #define LK_INTERFACE 1
 #define LK_ADDRESS   2

 #define IS_INTERFACE(x) (!strncasecmp((x), "-IF", 3))
 #define IS_ADDRESS(x) (!strncasecmp((x), "-ADDR", 5))
 
 #define LOADMODE(x, y) ((x)|=(y))
 
 #define DETERMINE_MODE(x) \
         if (IS_INTERFACE((x))) \
            { \
              if ((!*((x)+3))||(*((x)+3)!=' ')) \
                 { \
                  LOADMODE(mode, LK_INTERFACE); \
                 } \
              else \
                 { \
                  LOADMODE(mode, LK_INTERFACE); \
                  SPLIT_RMSPC(NULL, (x), ' '); \
                 } \
            } \
         else \
         if (IS_ADDRESS((x))) \
            { \
              if ((!*((x)+5))||(*((x)+5)!=' ')) \
                 { \
                  LOADMODE(mode, LK_INTERFACE); \
                 } \
              else \
                 { \
                  LOADMODE(mode, LK_ADDRESS); \
                  SPLIT_RMSPC(NULL, (x), ' '); \
                 } \
            } \
         else \
         if (HELP((x)+1)) \
            { \
             say ("%$% Help on LOOKUP:\n%s\n%s\n", INFO, \
                          "  LOOKUP [-type] <addr|if>", \
                          "  Supported types: IF, ADDR"); \
             return; \
            } \
         else \
            { \
             LOADMODE(mode, LK_INTERFACE); \
            }

 STD_IRCIT_COMMAND(uLOOKUP)

 {
#if 0
  int mode=0,
      sub=0;
  int x, y;
  register int j=0;

    if (!args)
       {
        YOU_NEED_HELP();
       }
#endif
//#ifdef 0
  int mode=0;

    if (!args)
       {
        YOU_NEED_HELP();
       }

    if (*args=='-')
       {
        DETERMINE_MODE(args);
       }
    else
       {
        LOADMODE(mode, LK_INTERFACE);
       }

 #undef IS_INTERFACE
 #undef IS_ADDRESS
 #define IS_INTERFACE() (mode&LK_INTERFACE)
 #define IS_ADDRESS() (mode&LK_ADDRESS)

    if (IS_INTERFACE())
       {
        LookupInterface (args);
       }
    else
    if (IS_ADDRESS())
       {
        nslookup (args);
       }
    else
       {
        say ( 
             "%$% Unable to lookup ``%s'': unclear whether address or interface.\n", MILD,
             args);
       }
//#endif
 }  /**/

 #undef DETERMINE_MODE


 STD_IRCIT_COMMAND(uLS)
 
 {
  char arg[LARGBUF];
  extern void uEXEC (char *);

    if (!args)
       {
        sprintf (arg, "ls");
       }
    else
       {
        sprintf (arg, "ls %s", args);
       }

   uEXEC (arg);

 }  /**/


 STD_IRCIT_COMMAND(uME)

 {
    if (!args)
     {
      say ("%$% arguments invalid or incomplete.\n", MILD);

      return;
     }

    if ((!curchan())||(!strlen(curchan())))
     {
      say ("%$% No target channel.\n", MILD);

      return;
     }

   {
    char s[strlen(args)+MAXCHANLEN+2];

     sprintf (s, "%s %s", curchan(), args);
     uDESCRIBE (s);
   }

 }  /**/

             
 STD_IRCIT_COMMAND(uPART)

 {
  extern ChannelsTable *const channels;

    if (ChannelsTableEmpty(channels))
     {
      say ("%$% You are afloat.\n", MILD);

      return;
     }

    if (!args) 
     {
      ToServer ("PART %s\n", curchan());

      return;
     } 

    if (*args=='-')
       {
        char *p=args+1;

          if (HELP(p))
             {
              say ("%$% Help on PART:\n%s\n%s\n", INFO,
                          "  PART [[#]channel[, channel, ...]]",
                          "  PART -*.*");
             }
          else
          if (arethesame(p, "*.*"))
             {
              int i;

               i=LeaveAllChannels ();

               say ("%$% Leaving as many as %d channel%s\n", INFO, i,
                     i>1?"s...":"...");
             }
          else
             {
              you_need_help;
             }
       }
    else
       {
        char c[MAXCHANLEN];
        int i,
            j;
        register Channel *chptr;

         i=splitargs2 (args, ',');

          for (j=0; j<=i; j++)
              {
                if ((*tok[j]!='#')&&(*tok[j]!='&')&&(*tok[j]!='+'))
                   {
                     if (strlen(tok[j])>MAXCHANLEN)
                        {
                         say ("%$% Channel name too long.\n", MILD);
                         continue;
                        }

                    c[0]='#';
                    strcpy (&c[1], tok[j]);
                   }
                else
                   {
                     if (strlen(tok[j])>MAXCHANLEN+1)
                        {
                         say ("%$% %s: Channel name too long.\n", MILD, tok[j]);
                         continue;
                        }

                    strcpy (&c[0], tok[j]);
                   }

                if ((chptr=ChannelbyName(c))==(Channel *)NULL)
                   {
                    say ("%$% You were not on channel %s.\n", MILD, c);
                    continue;
                   }
                else
                   {
                    ToServer ("PART %s\n", c);
                   }
              }
       }
        
 }  /**/


 STD_IRCIT_COMMAND(uQUOTE)

 {
    if (!args)
     {
      say ("%$% arguments invalid or incomplete.\n", MILD);

      return;
     }

   splitargs2 (args, ' ');

    if (*tok[0]=='-')
     {
      char *p=tok[0]+1;

        if (((*p=='h')||(*p=='H')||(*p=='?'))&&(!*(p+1)))
         {
          say ("%$% Help on QUOTE:\n%s\n", INFO,
               "  QUOTE <text>");
         }
        else
         {
          you_need_help;
         }

       return;
     }

   ToServer ("%s\n", args);

 }  /**/


 STD_IRCIT_COMMAND(uLOCALHOST)

 {
   FetchLocalhost ();
  
 }  /**/

   
 void uLUSERS (char *args)

 {
   ToServer ("LUSERS\n");

 }  /**/


 STD_IRCIT_COMMAND(uMSG)

 {
  int i,
      j;
  char *n,
       s[MEDIUMBUF];
  register Channel *chptr;
  extern InputHistory *tab;

    if (!args)
     {
      say ("%$% arguments invalid or incomplete.\n", MILD);

      return;
     }
   
   n=splitw (&args);
   TRIM(args);

    if (!args)  args=" ";

   i=splitargs2 (n, ',');

    for (j=0; j<=i; j++)
     {
       if (!*tok[j])  continue;

       if (ISCHANNEL(tok[j]))
        {
          if (strlen(tok[j]+1)>MAXCHANLEN)
           {
            say ("%$% %s: Channel name too long.\n", MILD, tok[j]);
            continue;
           }

          if ((chptr=ChannelbyName(tok[j]))==(Channel *)NULL)
           {
            say ("%$% You aren't on channel %s.\n", MILD, tok[j]);
            continue;
           } 

         say ("%$% msg sent to %s's %d members...\n", INFO, tok[j],
              PRIVMSGChanusesrs(chptr, args)); 
        }
       else
        {
         ToServer ("PRIVMSG %s :%s\n", tok[j], args);
              
         say ("<[%s]> %s\n", tok[j], args);
         sprintf (s, "/MSG %s ", tok[j]);
         AddtoUserInputHistory (tab, s, 0);

         continue;
        }
     }

 }   /**/


 STD_IRCIT_COMMAND(uPEOPLE)

 {
    if (!args)
     {
      ShowPeople ();
     }

 }  /**/


 STD_IRCIT_COMMAND(uDMSG)

 {
  char *n;
  register Someone *pptr;
  register Socket *sptr;
  register Index *iptr;
  register DCCParams *dcc;

    if (!args)
     {
      say ("%$% arguments invalid or incomplete.\n", MILD);

      return;
     }

   n=splitw (&args);
   TRIM(args);

    if (!args)  args=" ";

    if (!(pptr=SomeonebyName(n)))
     {
      say ("%$% You don't maintain DCC CHAT connection to %s.\n", CTCPH, n);

      return;
     }

    if (!(iptr=(Index *)FindDCCIndex(1, pptr)))
     {
      say ("%$% You don't maintain DCC CHAT connection to %s.\n", CTCPH, n);

      return;
     }

   (DCCParams *)dcc=INDEXTODCC(iptr);
        
    if (dcc->type&(DCCCHATTOUS|DCCCHATTOHIM))
     {
       if (dcc->flags&CONNECTED)
        {
         (Socket *)sptr=INDEXTOSOCKET(iptr);
         ToSocket (sptr->sock, "%s\n", args);
         say ("6<=%s=> %s\n", pptr->name, args);

         dcc->xfer+=strlen (args);
        }
       else
       if (dcc->flags&AWAITINGCONNECTION)
        {
         say ("%$% 6DCC CHAT connection with %s inprogress.\n"
              "  %$192%$196%$16 %s\n", CTCPH, n, PrintDCCState(iptr));
        }
     }
           
 }  /**/


 STD_IRCIT_COMMAND(uSMSG)

 {
  int socket;
  char *s;
  register Socket *sptr;
  extern Socket *so_hash[];

    if (!args)
     {
      say ("%$% arguments invalid or incomplete.\n", MILD);

      return;
     }

   s=splitw (&args);
   TRIM(args);

    if (!args)  args=" ";

    if (socket=atoi(s), !socket)
     {
      YOU_NEED_HELP();
     }

    if (socket>MAXSOCKSCOUNT)
     {
      YOU_NEED_HELP();
     }

    if (!(sptr=so_hash[socket]))
     {
      say ("%$% Invalid socket specified.\n", MILD);

      return;
     }
        
   ToSocket (socket, "%s\r\n", args);
   say ("<|%d|> %s\n", socket, args);
    
 }  /**/


 STD_IRCIT_COMMAND(uSTATUS)

 {
  int idx;

    if (!args)
     {
      say ("%$% Current status: %d\n", INFO, mt_ptr->c_status);

      return;
     }

   idx=atoi (args);

   {
     if ((idx>3)||(idx<0))
      {
       say ("%$% Invalid Status index specified.\n", MILD);

       return;
      }

    LoadUpperStatus (NULL, idx-1);
   }

 }  /**/


 STD_IRCIT_COMMAND(uTMSG)

 {
  int i,
      k;
  int j[]={[0 ... 19]=0};

   i=GetTelnetSockets (j);

    for (k=0; k<i; k++)
     {
      ToSocket (j[k], "%s\n", args);
      //say ("3<[%s]> %
     } 

 }  /**/


 STD_IRC_COMMAND(uNICK)

 {
    if (!args)
       {
        say ("%$% You currently known as %s.\n", INFO, curnick());

        return;
       } 

   splitargs2 (args, ' ');

    if (isme(tok[0]))
     {
      say ("%$% No, you cannot do this!\n", MILD);

      return;
     }

    if (strlen(tok[0])>MAXNICKLEN)
     {
      say ("%$% Nick too long.\n", MILD);

      return;
     }

   ToServer ("NICK %s\n", tok[0]);

 }  /**/


#include <termios.h>

 STD_IRCIT_COMMAND(uSCREENSHOT)

 {
  FILE *fp;
  int n,k,l=0;
  short r, g, b;
  char s[81];
 
    if (!(fp=fopen("dump.neg", "w")))
     {
      say ("%$% Unable to open negative-file for writing.\n", MILD);

      return;
     }

   fprintf (fp, "(U\n");

    for (n=0; n<curscr->_maxy+1; n++)
     {
      memset (s, 0, 80);

       for (k=0; k<80; k++)
        {
         s[k]=curscr->_line[n].text[k]&A_CHARTEXT;
        }
 
      s[k]=0;
 
      fprintf (fp, "%s\n", s);
     }

   fclose (fp);

    if (!(fp=fopen("dump.attr", "w")))
     {
      say ("%$% Unable to open attribute-file for writing\n", MILD);

      return;
     }

    for (n=0; n<curscr->_maxy+1; n++)
     {
      memset (s, 0, 80);

       for (k=0; k<=80; k++)
        {
         l=curscr->_line[n].text[k]&A_ATTRIBUTES;
         l|=curscr->_line[n].text[k]&curscr->_attrs;
         l|=curscr->_line[n].text[k]&curscr->_bkgd;

          if (l&A_ALTCHARSET)  l&=~A_ALTCHARSET;

           {
            color_content (PAIR_NUMBER(l&A_COLOR), &r, &g, &b);

            fprintf (fp, "%u %u %u %u %u %u %u \n",
                     PAIR_NUMBER(l&A_COLOR), 
                     l&A_BOLD?1:0,
                     l&A_REVERSE?1:0, 
                     l&A_BLINK?1:0,
                     r, g, b);
           }
        }
     }

   fclose (fp);


#if 0
  extern  void MoveWindow (int, int);
  int y, x;

   y=atoi(splitw(&args));
   x=atoi(splitw(&args));

   MoveWindow (y, x);
#endif 
 
 }  /**/


 STD_IRCIT_COMMAND(uTIMERS)

 {
    if (!args)
     {
      ShowTimerEntries ();
     }
    else
     {
      ScrollingTimers (NULL, 0);
     }

 }  /**/
 

 STD_IRCIT_COMMAND(uMASS)

 {
  char massop[]="+ooo";
  char massvoice[]="+vvv";
  char *type;
  char ch[MAXCHANLEN];
  int de=0;
  const Channel *chptr=NULL;
   
    if (!args)
     {
      MISSING_ARGUMENTS();
     }

    if (!*curchan())
     {
      say ("%$% You are afloat.\n", MILD);

      return;
     }

    if ((*args=='#')||(*args=='&')) //channels '+' dont have modes
     {
      strcpy (ch, splitw((char **)&args));
     }
    else
     {
      strcpy (ch, curchan());
     } 

    if (!args)  {YOU_NEED_HELP();}
      
   type=splitw (&args); 

    if (*type=='-')
     {
      de=1;
      type++;

       if (!*type)  {YOU_NEED_HELP();}
     }
    else
    if (*type=='+')
     {
      type++;
     }

    if (strlen(type)>1)  {YOU_NEED_HELP();}

    if (!(chptr=ChannelbyName(ch)))
     {
      say ("%$% You are not on channel %s.\n", MILD, ch);
      
      return;
     }

   switch (*type)
    {
     case 'o':
     case 'O':
           {
             if (de) massop[0]='-';

             if (args)
              {
               TRIM(args);
               MassOVWithMask (chptr, massop, args);
              }
             else
              {
               MassOV (chptr, massop);
              }
            break;
           }

     case 'v':
     case 'V':
           {
             if (de) massvoice[0]='-';

             if (args)
              {
               TRIM(args);
               MassOVWithMask (chptr, massvoice, args);
              }
             else
              {
               MassOV (chptr, massvoice);
              }
            break;
           }

     case 'b':
     case 'B':
           {
             if (args)  TRIM(args);
               
            MassBan (chptr, args?args:NULL);
            break;
           }

     case 'k':
     case 'K':
           {
             if (args)  TRIM(args);

            MassKick (chptr, args?args:NULL);
            break;
           }

     default:
          YOU_NEED_HELP();
    }

 }  /**/

 
 STD_IRC_COMMAND(uMODE)

 {
    if (!args)
     {
       if (*curchan())
        {
          if (strlen(cht_ptr->c_channelmode)>1)
           {
            say ("%$% Current channel modes for %s\n"
                 "  %$192%$196%$16 (%s)\n", INFO, 
                 curchan(), cht_ptr->c_channelmode);
           }
          else
           {
            say ("%$% No channel modes for %s defined.\n", 
                 INFO, curchan());
           }
        }
       else
        {
         say ("%$% arguments invalid or incomplete.\n", MILD);
        }

      return;
     }

   ToServer ("MODE %s\n", args); 

 }  /**/


 void uUMODE (char *args)

 {


 }  /**/


 STD_IRC_COMMAND(uWHO)

 {
  int i,
      op=0;
  register int j;
  extern ChannelsTable *const channels;

    if (!args)
       {
         if (NOCHANNELS(channels))
            {
             say ("%$% You are afloat.\n", MILD);
            }
         else
            {
             ShowUsers (cht_ptr->c_chptr, 0, 0, ((Channel *)cht_ptr->c_chptr)->nEntries);
            }

        return;
       }

    if (*args=='-')
       {
        char *p=args+1;

         if (HELP(p))
            {
             say ("Help on WHO:\n\%s\n", INFO,
                  "  WHO [channel[, channel, ...]]");

             return;
            }
         /*else
         if (!strncasecmp(p, "OP", 2))
            {
              if (!*(p+2))
                 {
                  MISSING_ARGUMENTS();
                 }
              else
              if (*(p+2)==' ')
                 {
                  SPLIT_RMSPC(NULL, args, ' ');
                  op=1;
                 }
            }*/
       }

   i=splitargs2 (args, ',');

    for (j=0; j<=i; j++)
        {    
          if (!ChannelbyName(tok[j]))
             {
              ToServer ("WHO %s %c\n", tok[j], op?'o':' ');
             }
          else
             {
              ShowChanUsers (tok[j]);
             }
        }

 }  /**/


 STD_IRCIT_COMMAND(uWHOAMI)

 {
    if (args)
     {
      say ("%$% Don't do this again.\n", MILD);
     }

   DisplayHeader (8, 7);

   say (" %$179%-9.9s%$179%-11.10s%$179%-4.4d%$179%-4.4d%$179%-4.4d%$179"
        "%-11.11s%$179%-15.15s%$179\n", ut_ptr->login, ut_ptr->realname, 
        ut_ptr->userid, geteuid(), ut_ptr->usergid, 
        ut_ptr->shell, ut_ptr->homedir);

   finish2(8, 7);

 }  /**/

   
 STD_IRC_COMMAND(uWHOIS)

 {
  int i=0;

    if (!args)
     {
      error;
      return;
     }
  
   i=splitargs2 (args, ' ');

    if (!*tok[1])
     {
      ToServer ("WHOIS %s\n", tok[0]);
     }
    else
     {
      ToServer ("WHOIS %s %s\n", tok[0], tok[1]);
     }

   whois_issued=1; 

 }  /**/ 


 STD_IRCIT_COMMAND(uWHOWAS)

 {
  int i=0;

    if (!args)
     {
      YOU_NEED_HELP();
     }

   i=splitargs2 (args, ' ');

    if (!*tok[1])
     {
      ToServer ("WHOWAS %s\n", tok[0]);

      return;
     }

    if (*tok[0]!='#'&&atoi(tok[1])&&!*tok[2])
     {
      ToServer ("WHOWAS %s %d\n");

      return;
     }

    if (*tok[0]!='#'&&atoi(tok[1])&&strchr(tok[2],'.')&&strlen(tok[2])>=3)
     {
      ToServer ("WHOWAS %s %d %s\n", tok[0], tok[1], tok[2]);

      return;
     }

   error;

 }  /**/
 

 #define MASS_UPDATE() \
          register Channel *chptr=channels->head; \
                                                  \
           say ("%$% Launching massive update...\n", INFO); \
                                                         \
            for ( ; chptr!=NULL; chptr=chptr->next) \
                { \
                 ResetChanUsers (chptr) ; \
                                          \
                 ToServer ("NAMES %s\nWHO %s\nMODE %s\n", \
                          chptr->channel, chptr->channel, chptr->channel); \
                } \
                  \
           return

 STD_IRCIT_COMMAND(uUPDATE)

 {
  int i;
  register int j;
  char c[MAXCHANLEN];
  register Channel *chptr;
  extern ChannelsTable *const channels; 
 
    if (!args)
     {
       if (ChannelsTableEmpty(channels))
        {
         say ("%$% You are afloat.\n", MILD);
        }
       else
        {
         say ("%$% Updating channel %s...\n", INFO, curchan());

         ResetChanUsers (curchptr());

         ToServer ("NAMES %s\nWHO %s\nMODE %s\n", 
                   curchan(), curchan(), curchan());
        }

      return;
     } 

    if (NOCHANNELS(channels))
     {
      say ("%$% You are afloat.\n", MILD);

      return;
     }

   {
    i=splitargs2 (args, ',');

     for (j=0; j<=i; j++)
      {
        if ((j==0)&&(!strcmp("-*.*", tok[j])))
         {
          MASS_UPDATE();
         }

        if ((*tok[j]!='#')&&(*tok[j]!='&')&&(*tok[j]!='+'))
         {
           if (strlen(tok[j])>MAXCHANLEN)
            {
             say ("%$% %s: Channel name too long.\n", MILD, tok[j]);
             continue;
            }

          c[0]='#';
          strcpy (&c[1], tok[j]);
         }
        else
         {
           if (strlen(tok[j])>MAXCHANLEN+1)
            {
             say ("%$% %s: Channel name too long.\n", MILD, tok[j]);
             continue;
            }

          strcpy (&c[0], tok[j]);
         }

        if ((chptr=ChannelbyName(c))!=(Channel *)NULL)
         {
          ResetChanUsers (chptr);
          ToServer ("NAMES %s\nWHO %s\nMODE %s\n", c, c, c);
         }
        else
         {
          say ("%$% Cannot update channel %s.\n", MILD, c);
         }
      }
   }

 }  /**/


 /*
 ** clear [-all|-mw|-lw|uw]
 */
 STD_IRCIT_COMMAND(uCLEAR)

 {
  int mode=0;

    if (!args)
     {
      mode|=2;
     }
    else
    if (*args=='-')
     {
       if ((arethesame(args, "-all"))||(arethesame(args, "-*.*")))
        {
         mode|=1;
        }
       else
       if (arethesame(args, "-mw"))
        {
         mode|=2;
        }
       else
       if (arethesame(args, "-uw"))
        {
         mode|=4;
        }
       else
       if (arethesame(args, "-lw"))
        {
         mode|=8;
        }
       else
        {
         YOU_NEED_HELP();
        }
     }
    else
     {
      YOU_NEED_HELP();
     }
  
   switch (mode&(1|2|4|8))
          {
           case 1:
                RedrawMainWin ();
                break;

           case 2:
                RedrawMainWin ();
                break;

           case 4:
                //RedrawUpperWin ();
                break;

           case 8:
                //RedrawLowerWin ();
                break;
          }

   doupdate ();

 }  /**/


 #define L_PER (((double)c_volume.l/(double)MAX_VOL)*100.0)
 #define R_PER (((double)c_volume.r/(double)MAX_VOL)*100.0)
 #define INDB ACS_BULLET|COLOR_PAIR(CYAN)|A_BOLD
 #define IND  ACS_BULLET|COLOR_PAIR(CYAN)

 #define SHOW_VOLUME() \
         { \
          register int i=0; \
          int l, \
              r; \
          char p[10], \
               s[10]; \
          chtype ll[]={[0 ... 10]=0}, \
                 rr[]={[0 ... 10]=0}; \
                         \
           sprintf (p, "%.0f", L_PER); \
           sprintf (s, "%.0f", R_PER); \
           l=atoi(p); \
           r=atoi(s); \
                      \
            do {ll[i++]=INDB;} while ((l-=10)>=10&&i<=10); while (i!=10) ll[i++]=IND; i=0; \
            do {rr[i++]=INDB;} while ((r-=10)>=10&&i<=10);  while (i!=10) rr[i++]=IND; i=0;\
           nYell ("%$% Current volume setting:\n  %$192%$196%$16 [R: %z (%d/%d)] [L: %z (%d/%d)] \n", INFO, rr, c_volume.r, MAX_VOL, ll, c_volume.l, MAX_VOL); \
         }

 #include <linux/soundcard.h>


 STD_IRCIT_COMMAND(uVOLUME)

 {
  int sndfd;
  unsigned char req;
  typedef struct __volume {unsigned char r, l;} __volume;
  __volume c_volume;
  #define MAX_VOL 95
  #define MIN_VOL 5
  
    if ((sndfd=open("/dev/mixer", O_RDWR, 0))<0) 
     {
      say ("%$% Unable to open mixer device: %s\n", MILD, strerror(errno));

      return;
     }

    if (!args)
     {
       if (ioctl(sndfd, MIXER_READ(SOUND_MIXER_VOLUME), &c_volume)<0) 
        {
         say ("%$% Unable to fetch volume settings: %s\n", 
              MILD, strerror(errno));

         return;
        }

      SHOW_VOLUME();  

      close (sndfd);      

      return;
     }

    if (((req=atoi(args))<MIN_VOL)||(req>MAX_VOL))
     {
      say ("%$% I don't know how to handle your settings.\n", MILD);

      close (sndfd);

      return;
     }

   c_volume.r=req;
   c_volume.l=req;

    if (ioctl(sndfd, MIXER_WRITE(SOUND_MIXER_VOLUME), &c_volume)<0) 
     {
      say ("%$% Unable to set volume: %s\n", MILD, strerror(errno));

      close (sndfd);

      return;
     } 

   SHOW_VOLUME();

   close (sndfd);

 }  /**/

