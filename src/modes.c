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
#include <updates.h>
#include <list.h>
#include <scroll.h>
#include <lastlog.h>
#include <chanrec.h>
#include <modes.h>

 MODULEID("$Id: modes.c,v 1.5 1998/04/06 06:13:44 ayman Beta $");


 #define ISCHANNEL(x) ((*(x)=='#')||(*(x)=='&')||(*(x)=='+'))

 static __inline__ void Usermodes (char *modes)

 {
  int add=0;
  register char *p;

    if (!(p=modes)) return;

    do {
        switch (*p)
               {
                case '+':
                     add=1;
                     break;

                case '-':
                     add=0;
                     break;

                case 'i':
                     add?(st_ptr->c_usermode|=USER_INVISIBLE):
                         (st_ptr->c_usermode&=~USER_INVISIBLE);
                     break;

                case 'o':
                     add==0?(lower_status_figures(0, 0, 0)):1;
                     break;

                case 'r':
                     add?(st_ptr->c_usermode|=USER_RESTRICTED):
                         (st_ptr->c_usermode&=~USER_RESTRICTED);
                     break;

                case 'w':
                     add?(st_ptr->c_usermode|=USER_WALLOP):
                         (st_ptr->c_usermode&=~USER_WALLOP);
                     break;
               }
       }
    while (*p++);

   say ("%$% Your usermode has been set to %s.\n", SRV_INFO, modes);

   DisplayServerTable (SERV_UMODE);

 }  /**/


   static __inline__ void 
  Chanmode_v (Channel *const chptr, char *nick, char *by, int how)

 {
  ChanUser *uptr;
  extern chtype *f;

   uptr=(ChanUser *)userbychptr (chptr, nick);

     if (uptr==(ChanUser *)NULL)
        {
         say (
              "%$% Chanvoice status change on %s to non-existent %s by %s!\n",
               f[chptr->id], chptr->channel, nick, by);
        }
     else
        {
          if (isme(nick))
             {
               if (how==1)
                  {
                    if (!(uptr->usermodes&USER_CHANVOICE))
                       {
                        say ("%$% %s on %s!\n", f[chptr->id],
                             "You now have voice on channel ", chptr->channel);

                        uptr->usermodes|=USER_CHANVOICE;
                        chptr->nchanvoice++;

                         if (iscurchan(chptr->channel))
                            {
                             UpdateServerTable (SERV_UMODE,
                                                USER_CHANVOICE,
                                                NULL,
                                                '+');
                            }
                       }
                  }
               else
                  {
                    if (uptr->usermodes&USER_CHANVOICE)
                       {
                        say ("%$% %s on %s!\n", f[chptr->id],
                            "You no longer have channel voice ",chptr->channel);

                        uptr->usermodes&=~USER_CHANVOICE;
                        chptr->nchanvoice--;

                         if (iscurchan(chptr->channel))
                            {
                             UpdateServerTable (SERV_UMODE, USER_CHANVOICE,
                                                NULL, '-');
                            }
                       }
                  }
             }
          else
             {
               if (how==1)
                  {
                    if (!(uptr->usermodes&USER_CHANVOICE))
                       {
                        say ("%$% %s %s on %s!\n", f[chptr->id], nick, 
                             "now has channel voice", chptr->channel);

                        uptr->usermodes|=USER_CHANVOICE;
                        chptr->nchanvoice++;
                       }
                  }
               else
                  {
                    if (uptr->usermodes&USER_CHANVOICE)
                       {
                        say ("%$% %s %s on %s!\n", f[chptr->id], nick,
                             "no longer has channel voice", chptr->channel);

                        uptr->usermodes&=~USER_CHANVOICE;
                        chptr->nchanvoice--;
                       }
                  }
             }
        }

 }  /**/


  static __inline__ void 
 Chanmode_o (Channel *const chptr, char *nick, char *by, int how)

 {
  ChanUser *uptr;
  extern chtype *f;

    uptr=(ChanUser *)userbychptr (chptr, nick);

     if (uptr==(ChanUser *)NULL)
        {
         say ( 
              "%$% Chanop status change on %s to non-existent %s by %s!\n",
               f[chptr->id], chptr->channel, nick, by);
        }
     else
        {
          if (isme(nick))
             {
               if (how==1)
                  {
                    if (!(uptr->usermodes&USER_CHANOP))
                       {
                        say ("%$% %s on %s!\n", f[chptr->id],
                             "You're now channel operator", chptr->channel);
                                                 
                        uptr->usermodes|=USER_CHANOP;
                        chptr->chanop=1;
                        chptr->nchanop++;

                         if (iscurchan(chptr->channel))
                            {
                             UpdateServerTable (SERV_UMODE,
                                                USER_CHANOP,                                                                    NULL,
                                                '+');
                            }
                       }
                  }
               else
                  {
                    if (uptr->usermodes&USER_CHANOP)
                       {
                        say ("%$% %s on %s!\n", f[chptr->id],
                             "You no longer channel operator", chptr->channel);

                        uptr->usermodes&=~USER_CHANOP;
                        chptr->chanop=0;
                        chptr->nchanop--;

                         if (iscurchan(chptr->channel))
                            {
                             UpdateServerTable (SERV_UMODE, USER_CHANOP,
                                                NULL, '-');
                            }
                       }
                  }
             }
          else
             {
               if (how==1)
                  {
                    if (!(uptr->usermodes&USER_CHANOP))
                       {
                        say ("%$% %s %s on %s!\n", f[chptr->id], nick, 
                             "is now channel operator", chptr->channel);

                        uptr->usermodes|=USER_CHANOP;
                        chptr->nchanop++;
                       }
                  }
               else
                  {
                    if (uptr->usermodes&USER_CHANOP)
                       {
                        say ("%$% %s %s on %s!\n", f[chptr->id], nick,
                             "is no longer channel operator", chptr->channel);

                        uptr->usermodes&=~USER_CHANOP;
                        chptr->nchanop--;
                       }
                  }
             }
        }

 }  /**/


  static __inline__ void 
 Chanmode_b (Channel *const chptr, char *mask, char *host, char *by, int how)

 {
  int flag=0;
  ChanUser *uptr;
  register Ban *bptr;
  extern chtype *f;

    if (how==1)
       {
         if (match(mask, me()))
            {
             say ("%$% BAN on you on %s by %s!\n", f[chptr->id],
                  chptr->channel, by);
 
             ToServer ("MODE %s -b %s\n", chptr->channel, mask);
             flag=1;
            }

        bptr=AddBan (chptr, mask);

         if (bptr!=(Ban *)NULL)
            {
             bptr->when=time (NULL);
             strcpy (bptr->who, by);
             bptr->flag=flag;
            }
         else
            {
             say ("%$% Unable to add ban to %s!\n", HOT, chptr->channel);
            }
       }
    else
       {
         if (RemoveBan(chptr, NAME, mask))
            {
             /* say ("ban removed!\n"); */
            }
       }


 }  /**/


  static __inline__ void 
 Chanmodes (Channel *const chptr, char *by, char *from, char *modes, char *v)

 {
  int add=0;
  register char *p;

    if (!(p=modes)) return;

    do {
        switch (*p)
               {
                case '+':
                     add=1;
                     break;

                case '-':
                     add=0;
                     break;

                case 'o':
                       Chanmode_o (chptr, splitw(&v), by, add);
                       break;

                case 'b':
                       Chanmode_b (chptr, splitw(&v), by, from, add);
                       break;

                case 'v':
                     Chanmode_v (chptr, splitw(&v), by, add);
                     break;
 
                case 'm':
                     add?(chptr->chanmode|=CHAN_MODERATE):
                         (chptr->chanmode&=~CHAN_MODERATE);
                     break;

                case 'i':
                     add?(chptr->chanmode|=CHAN_INVITE):
                         (chptr->chanmode&=~CHAN_INVITE);
                     break;

                case 't':
                     add?(chptr->chanmode|=CHAN_TOPIC):
                         (chptr->chanmode&=~CHAN_TOPIC);
                     break;

                case 's':
                     add?(chptr->chanmode|=CHAN_SECRET):
                         (chptr->chanmode&=~CHAN_SECRET);
                     break;

                case 'p':
                     add?(chptr->chanmode|=CHAN_PRIVATE):
                         (chptr->chanmode&=~CHAN_PRIVATE);
                     break;
                 
                case 'n':
                     add?(chptr->chanmode|=CHAN_NO_MSG):
                         (chptr->chanmode&=~CHAN_NO_MSG);
                     break;

                case 'a':
                     add?(chptr->chanmode|=CHAN_ANON):
                         (chptr->chanmode&=~CHAN_ANON);
                     break;

                case 'k':
                     {
                       if (add)
                          {
                           strcpy (chptr->key, splitw(&v));
                          }
                       else
                          {
                           chptr->key[0]=0;
                           splitw (&v);
                          }

                      break;
                     }

                case 'l':
                     {
                       if (add)
                          {
                           chptr->limit=strtoint(splitw(&v));
                          }
                       else
                          {
                           chptr->limit=0;
                          }

                     break;
                    }
               }
       }
    while (*p++);

 }  /**/


 void sMODE (char *from, char *rest)

 {
  char *n;
  chtype *chs;
  extern chtype *f;

    if (ISCHANNEL(rest))
     {
      register char *p;
      char *c,
           *m,
           b[LARGBUF]={0};
      register Channel *chptr;

        if (strchr(from, '!'))
         {
          n=splitn (&from);

          FIXIT(from);
         }
        else
         {
          n=from;
         }

       c=splitw (&rest);

        if ((chptr=ChannelbyName(c))==(Channel *)NULL)
         {
          say ("%$% Channel mode change on non-existent channel %s by %s!\n",
               MILD, c, n);

          return;
         }

       m=splitw (&rest);
       strcpy (b, rest);
       Chanmodes (chptr, n, from, m, b);
         
        for (p=rest+strlen(rest)-1; ((isspace(*p))&&(p>=rest)); p--)
            ;
       *(p+1)=0;

       chs=PreSay ("%$% Channel mode change detected on %s by %s:\n"
                   "  %$192%$196%$16 (%s %s)\n", 
                   f[chptr->id], chptr->channel, n, m, rest);

#ifdef LAST__LOG
        if (IS_CHLOGLEVEL(chptr, LOG_MODE))
         {
          AddtoLastLog (LOG_MODE, chptr, chs);
         }
#endif

       nYelll (chs);

       ParseChanmode (c, b);
       UpdateUpperBorder (CH_CHANMODE, 0, b, 0); 
     }
    else
     {
      n=splitw (&from);

      splitcolon (&rest);
      Usermodes (rest);
     }

 }  /**/

