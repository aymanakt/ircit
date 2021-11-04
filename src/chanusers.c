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
#include <stack.h>
#include <scroll.h>
#include <lastlog.h>
#include <chanrec.h>
#include <misc.h>
#include <chanusers.h>

 MODULEID("$Id: chanusers.c,v 1.9 1998/04/09 14:45:39 ayman Beta $");

 extern ChannelsTable *const channels;


 __inline__ void HashChanuser (int i, Channel *chptr, ChanUser *uptr)

 {
  register ChanUser *aux;

   aux=chptr->Hash[i];
   chptr->Hash[i]=uptr;
   uptr->hnext=aux;

 }  /**/


  __inline__ void 
 UnhashChanuser (const char *nick, Channel *chptr, ChanUser *uptr)

 {
  int i;
  register ChanUser *aux;

   i=hashstr (nick);

    if (uptr==chptr->Hash[i])
     {
      if (uptr->hnext==NULL)
       {
        chptr->Hash[i]=(ChanUser *)NULL;
       }
      else
       {
        aux=uptr->hnext;
        chptr->Hash[i]=aux;
       }

      uptr->hnext=NULL;
     }
    else
     {
      int found=0;
      ChanUser *prev=NULL;

       aux=chptr->Hash[i];

        while ((aux!=NULL)&&(!found))
         {
           if (aux==uptr)
            {
             found=1;
            }
           else
            {
             prev=aux;
             aux=aux->hnext;
            }
         }

        if (aux->hnext==NULL)
         {
          prev->hnext=NULL;
         }
        else
         {
          prev->hnext=aux->hnext;
         }

       aux->hnext=NULL;
     }

 }  /**/


 ChanUser *userbychannel (const char *channel, const char *nick)

 {
  int i;
  register Channel *chptr;
  register ChanUser *uptr;

    if ((chptr=ChannelbyName(channel))!=NULL)
     {
      i=hashstr(nick);
      uptr=chptr->Hash[i];

       while (uptr)
        {
         if (arethesame(nick, uptr->nick))
          {
           return (ChanUser *)uptr;
          }
         else
          {
           uptr=uptr->hnext;
          }
        }
     }

   return (ChanUser *)NULL;

 }  /**/ 


 ChanUser *FindChanneluser (const char *nick)

 {
  int i;
  register Channel *chptr;
  register ChanUser *uptr;


   i=hashstr(nick);
   chptr=channels->head;

    for ( ; chptr!=NULL; chptr=chptr->next)
     {
      uptr=chptr->Hash[i];

       while (uptr)
        {
         if (arethesame(nick, uptr->nick))
          {
           return (ChanUser *)uptr;
          }
         else
          {
           uptr=uptr->hnext;
          }
        }
     }
   
   return (ChanUser *)NULL;

 }  /**/
  
 
 ChanUser *userbychptr (const Channel *chptr, const char *nick)

 {
  int i;
  register ChanUser *uptr;

   i=hashstr (nick);
   uptr=chptr->Hash[i];

    while (uptr)
     {
      if (!strcasecmp(nick, uptr->nick))
       {
        return (ChanUser *)uptr;
       }
      else
       {
        uptr=uptr->hnext;
       }
     }

   return (ChanUser *)NULL;

 }  /**/


 ChanUser *AddUserToChannel (const char *channel, const char *nick)

 {
  int i;
  register ChanUser *uptr;
  register Channel *chptr;

    if ((chptr=ChannelbyName(channel))==NULL)
     {
      return (ChanUser *)NULL;
     }

    if (userbychptr(chptr, nick))
     {
      return (ChanUser *)NULL;
     }

    if ((uptr=(ChanUser *)malloc(sizeof(ChanUser)))==NULL)
     {
      return (ChanUser *)NULL;
     }

   memset (uptr, 0, sizeof(ChanUser));
   mstrncpy (uptr->nick, nick, MAXNICKLEN);

    if (!UsersRecordEmpty(chptr))
     {
      chptr->rear->next=uptr;
      chptr->rear=uptr;
     }
    else
     {
      chptr->front=chptr->rear=uptr; 
     }

   uptr->chptr=chptr;
  
   i=hashstr (uptr->nick);
   HashChanuser (i, chptr, uptr);

   chptr->nEntries++;

    if (((chptr->glock>>0)&0x1)&&((chptr->glock>>1)&0x1))
     {
      RefreshListContents (chptr);
     }

   return (ChanUser *)uptr;

 }  /**/


 int AddUser (const char *channel, const char *nick, const char *host)

 {
  register ChanUser *uptr;

    if ((uptr=AddUserToChannel(channel, nick))!=NULL)
     {
      uptr->when=time (NULL);
      mstrncpy (uptr->host, host, MAXHOSTLEN);

      return 1;
     }

  return 0;

 }  /**/


 ChanUser *AddChanuser (Channel *chptr, const char *nick)

 {
  int i;
  const size_t sizeofChanUser=sizeof(ChanUser);
  register ChanUser *uptr;

   xmalloc(uptr, sizeofChanUser);
   memset (uptr, 0, sizeof(ChanUser));

   mstrncpy (uptr->nick, nick, MAXNICKLEN);

    if (!UsersRecordEmpty(chptr))
     {
      chptr->rear->next=uptr;
      chptr->rear=uptr;
     }
    else
     {
      chptr->front=chptr->rear=uptr;
     }

   i=hashstr ((const char *)uptr->nick);
   HashChanuser (i, chptr, uptr);

   uptr->chptr=chptr;

   chptr->nEntries++;

    if (((chptr->glock>>0)&0x1)&&((chptr->glock>>1)&0x1))
     {
      RefreshListContents (chptr);
     }

   return (ChanUser *)uptr;

 }  /**/


 void ProcessUser (char *rest)

 {
  char *u,
       *h,
       *s,
       *n,
       *f;
  register Channel *chptr;
  register ChanUser *uptr;
  extern ChannelsTable *channels;

    u=splitw (&rest);
    h=splitw (&rest);
    s=splitw (&rest);
    n=splitw (&rest);
    f=splitw (&rest);

     for (chptr=channels->head; chptr!=NULL; chptr=chptr->next) //
         {
          
    uptr=userbychptr (chptr, n);

     if (uptr==(ChanUser *)NULL)
        {
         continue;//uptr=(ChanUser *)AddChanuser (chptr, n);;  
        }

    mstrncpy (uptr->user, u, MAXNICKLEN);
    mstrncpy (uptr->host, h, MAXHOSTLEN);
    strcpy (uptr->server, s);
    strcpy (uptr->realname, &rest[3]);

    if (*f=='H')
       {
        uptr->usermodes|=USER_HERE;
       }
    else
    if (*f=='G')
       {
        uptr->usermodes|=USER_GONE;
       }

    if (strchr(f, '@'))
       {
        uptr->usermodes|=USER_CHANOP;
       }
    else
    if (strchr(f, '+'))
       {
        uptr->usermodes|=USER_CHANVOICE;
       }
  
    if (strchr(f, '*'))
       {
        uptr->usermodes|=USER_IRCOP;
       }
     }

 }  /**/


 Channel *RemoveUserFromChannel (const char *channel, const char *nick)

 {
  int found=0;
  register ChanUser *uptr, 
                    *prev=NULL;
  register Channel *chptr;

    if ((chptr=ChannelbyName(channel))!=NULL)
       {
        uptr=chptr->front;
        
         while ((uptr!=NULL)&&(!found))
               {
                 if (strcasecmp(uptr->nick, nick)==0)
                    {
                     found=1;
                    }
                 else
                    {
                     prev=uptr;
                     uptr=uptr->next;
                    }
               }

         if (!found)  return NULL;

         if (prev==NULL) 
            {
             chptr->front=uptr->next;
            }
         else
         if (uptr->next==NULL) 
            {
             chptr->rear=prev;
             chptr->rear->next=NULL;
            }
         else
            {
             prev->next=uptr->next; 
            }

        UnhashChanuser ((const char *)uptr->nick, chptr, uptr);
                   
        chptr->nEntries--;

         if (uptr->usermodes&USER_CHANOP)  chptr->nchanop--;
         if (uptr->usermodes&USER_CHANVOICE)  chptr->nchanvoice--;
         if (uptr->split)  chptr->nsplit--;

         if (((chptr->glock>>0)&0x1)&&((chptr->glock>>1)&0x1))
          {
           RefreshListContents (chptr);
          }

        free (uptr);

        return chptr;
       }

   return NULL;

 }  /**/


 int RemoveChanUserbychptr (Channel *chptr, const char *nick)

 {
  int found=0;
  register ChanUser *uptr=chptr->front,
                    *prev=NULL;

    while ((uptr!=NULL)&&(!found))
          {
            if (arethesame(uptr->nick, nick))
               {
                found=1;
               }
            else
               {
                prev=uptr;
                uptr=uptr->next;
               }
          }

    if (!found)  return 0;

    if (prev==NULL)
       {
        chptr->front=uptr->next;
       }
    else
    if (uptr->next==NULL)
       {
        chptr->rear=prev;
        chptr->rear->next=NULL;
       }
    else
       {
        prev->next=uptr->next;
       }

   UnhashChanuser ((const char *)uptr->nick, chptr, uptr);

    if (uptr->usermodes&USER_CHANOP)  chptr->nchanop--;
    if (uptr->usermodes&USER_CHANVOICE)  chptr->nchanvoice--;
    if (uptr->split)  chptr->nsplit--;

   chptr->nEntries--;

    if (((chptr->glock>>0)&0x1)&&((chptr->glock>>1)&0x1))
     {
      RefreshListContents (chptr);
     }

   free (uptr);

   return 1;

 }  /**/


 int ResetChanUsers (Channel *chptr)

 {
  register int i=0;
  register ChanUser *uptr,
                    *aux;

   uptr=chptr->front;

    while (uptr!=NULL)
     {
      /*i++;*/
      aux=uptr->next;
      free (uptr);
      uptr=aux;
     }

    for (i=0; i<=HASHSIZE; i++)
     {
      chptr->Hash[i]=(ChanUser *)NULL;
     }

   chptr->nEntries=chptr->nchanop=chptr->nsplit=chptr->nchanvoice=0;
   chptr->front=chptr->rear=NULL;
  
   chptr->status|=(PROCESSINGNAMESLIST|PROCESSINGWHOREPLAY);
                  
   return i;

 }  /**/


 void UpdateChanuserNick (const char *onick, const char *nnick)

 {
  int i,
      j;
  register Channel *chptr;
  register ChanUser *uptr;
  extern chtype *chf;

   j=hashstr (onick);
   chptr=channels->head;

    for ( ; chptr!=NULL; chptr=chptr->next)
     {
      uptr=chptr->Hash[j];

       while (uptr!=NULL)
        {
          if (!strcasecmp(onick, uptr->nick))
           {
#ifdef LAST__LOG
             if (IS_CHLOGLEVEL(chptr, LOG_OTHR))
              {
               AddtoLastLog (LOG_OTHR, chptr, 
                             (PreSay("%$% %s changed his nick to %s.\n",
                                     chf[chptr->id+10], onick, nnick)));
              }
#endif
            strcpy (uptr->nick, nnick);

            UnhashChanuser (onick, chptr, uptr);
            i=hashstr (nnick);
            HashChanuser (i, chptr, uptr);
            break;
           }
          else
           {
            uptr=uptr->hnext;
           }
        }
     }

 }  /**/


 void CheckSplitUsers (void)

 {
  int thr,
      nsplit=0;
  time_t now=time(NULL);
  register Channel *chptr;
  register ChanUser *uptr;
           ChanUser *aux; 
  extern ChannelsTable *channels;

     if (!(thr=atoi((char *)valueof("SPLIT_DURATION"))))  return;

    chptr=channels->head;

     for ( ; chptr!=NULL; chptr=chptr->next)
      {
        if (chptr->nsplit>0)
         {
          uptr=chptr->front;

           while (uptr!=NULL)
            {
             aux=uptr->next;

              if ((uptr->split)&&((now-uptr->split)>=thr))
               {
                RemoveChanUserbychptr (chptr, uptr->nick);
               }

             uptr=aux;
            }

          nsplit+=chptr->nsplit;
         }
      }

    if (nsplit<=0)
     {
      DeActivateTimer (7, NULL);
     }

    if ((Channel *)cht_ptr->c_chptr)
     UpdateUpperBorder (CH_NUSERS, ((Channel *)cht_ptr->c_chptr)->nEntries, 0, 0);
 
 }  /**/  

  
 int PRIVMSGChanusesrs (const Channel *chptr, const char *msg)

 {
  register ChanUser *uptr=chptr->front;

    for (; uptr!=NULL; uptr=uptr->next)
     {
      ToServer ("PRIVMSG %s :%s\n", uptr->nick, msg);
     }

  return chptr->nEntries;

 }  /**/


 #define SPLIT_ICON 0x15|A_ALTCHARSET|COLOR_PAIR(CYAN)|A_BLINK

 int ShowChanUsers (const ChanUser *uptr, const int i)

 {
  int ii=0,
      j=0;
  char s[10],
       h[]={[0 ... 4]=0};

    if (uptr->usermodes&USER_HERE)
       {
        h[j++]='H';
       }
    else
    if (uptr->usermodes&USER_GONE)
       {
        h[j++]='G';
       }
    else
       {
        h[j++]='?';
       }
     
    if (uptr->usermodes&USER_IRCOP)
       {
        h[j++]='*';
       }

    if (uptr->usermodes&USER_CHANOP)
       {
        h[j++]='@';
       }
    else
    if (uptr->usermodes&USER_CHANVOICE)
       {
        h[j++]='+';
       }

   h[j]=0;

    if (!uptr->when)
       {
        strcpy (s, "--:--");
       }
    else
       {
        strftime (s, 10, "%H:%M", localtime(&uptr->when));
       }

   say (" %$179%-3d%$179%-3s%$179%-9.9s  %$% %$179%-19.19s%$179%-15.15s"
        "%$179%-5.5s%$179\n", i, h, uptr->nick, uptr->split?SPLIT_ICON:' ', 
        uptr->host, uptr->server, s);

   return ii;

 }  /**/


 void ShowUsers (const Channel *ptr, int modes, int x, int y)

 {
  register int i=0;
  register ChanUser *uptr;

   uptr=ptr->front;

    while ((++i)<=x)
     {
      uptr=uptr->next;
     }
   i--;

   DisplayHeader (6, 6);

    for ( ; uptr!=NULL, ((++i)<=(y)); uptr=uptr->next)
     {
       if ((modes)&&((uptr->usermodes&modes)==0))  continue;

       ShowChanUsers (uptr, i);
     } 

   finish2 (6, 6);

 }  /**/


 void ShowSplitUsers (const Channel *chptr, int x, int y)

 {
  register int i=0;
  register ChanUser *uptr;

    if (!chptr->nsplit)
     {
      say ("%$% No split users in %s found.\n", INFO, chptr->channel);

      return;
     }

   uptr=chptr->front;

    while ((++i)<=x)
     {
      uptr=uptr->next;
     }
   i--;

   DisplayHeader (6, 6);

    for ( ; uptr!=NULL, ((++i)<=(y)); uptr=uptr->next)
     {
       if (uptr->split)
        {
         ShowChanUsers (uptr, i);
        }
     }

   finish2 (6, 6);

 }  /**/


 void ScrollingChanusersList (Channel *);
 void ScrollingChanusersInit (ScrollingList *);
 void ScrollingChanusersEntries (ScrollingList *);
 void ScrollingChanusersEnter (void *, int);


 void ScrollingChanusers (void *ptr, int idx)

 {
   ScrollingChanusersList ((Channel *)ptr);

 }   /**/


 static const char *chu_options[]={
              " Info ",
              " Op   ",
              " Kick ",
              " Ban  "
             };
 static List chuo_ent={
         0, 4, NULL, NULL
        };
 static header chuo_hdr={4, NULL, chu_options};

 static const char *chu_hdrs[]={"User", "Host"};
 static const int len[]={10, 20};
 static header chu_hdr={2, len, chu_hdrs};

 void ScrollingChanusersList (Channel *ptr)

 {
  register ScrollingList *auxscrptr=NULL;
  #define ALSO_INIT_OPTMENU 1	

    if (!(auxscrptr=CreateScrollingList(ALSO_INIT_OPTMENU, ptr)))
       {
        /* */
       }
    else
       {
        int *attrs;
        register ScrollingList *optmenu;

         xmalloc(attrs, (4*sizeof(int)));
         attrs[0]=1, attrs[1]=0, attrs[2]=0, attrs[3]=0;

         auxscrptr->startx=14;
         auxscrptr->starty=4;
         auxscrptr->width=50;
         auxscrptr->hight=10;
         auxscrptr->attrs=attrs;
         auxscrptr->nEntries=ptr->nEntries;
         (Channel *)auxscrptr->list=ptr;
         sprintf (auxscrptr->caption, " Users on %.9s ", ptr->channel);
         //ptr->glock=0;
         auxscrptr->lock=&ptr->glock;
         auxscrptr->h=&chu_hdr;
         auxscrptr->func=ScrollingChanusersEntries;
         auxscrptr->func_init=ScrollingChanusersInit;
         auxscrptr->enter_func=ScrollingChanusersEnter;

         optmenu=auxscrptr->opt_menu;
         optmenu->nEntries=4;
         optmenu->startx=50;
         optmenu->width=6;
         optmenu->hight=4;
         optmenu->h=&chuo_hdr;
         optmenu->list=&chuo_ent;

         LoadScrollingList (auxscrptr); 
       }

 }  /**/


 void ScrollingChanusersEntries (ScrollingList *sptr)

 {
  register int j=0;
  int maxy,
      last;
  register ChanUser *s_ptr;
  WINDOW *w=(WINDOW *)RetrieveCurrentWindow();
  extern Stack *const stack;

   getmaxyx (w, maxy, last);

   last=maxy+sptr->listwin_start;

    for (s_ptr=((Channel *)sptr->list)->front; 
         (s_ptr!=NULL)&&(j++!=sptr->listwin_start); 
         s_ptr=s_ptr->next)
        ;

    for (--j;
         (j<last)&&(j<((Channel *)sptr->list)->nEntries)&&(s_ptr!=NULL);
         j++, s_ptr=s_ptr->next)
        {
          if (j==sptr->hilited)
             {
              _wattron (w, A_REVERSE);
              (ChanUser *)sptr->c_item=(ChanUser *)s_ptr;
             }

         _wmove (w, j-sptr->listwin_start, 0);
         Fwprintf (w, " %c%-9.9s%$179%-36.36s",
                  s_ptr->usermodes&USER_CHANOP?'@':' ',
                  s_ptr->nick,
                  s_ptr->host); 

          if (j==sptr->hilited)
             {
              _wattroff (w, A_REVERSE);
             }
        }

    for ( ; j<last; ++j)
     {
      _wmove (w, j-sptr->listwin_start, 0);
      Fwprintf (w, " - ");
      _wmove (w, j-sptr->listwin_start, 2);
      wclrtoeol (w);
     }

 }  /**/


 void ScrollingChanusersInit (ScrollingList *sptr)

 {
    if (!isop())
     {
      sptr->attrs[1]=sptr->attrs[2]=sptr->attrs[3]=0;
     }
    else
     {
      sptr->attrs[1]=sptr->attrs[2]=sptr->attrs[3]=1;
     }

 }  /**/


 #define uptr ((ChanUser *)ptr)

 void ScrollingChanusersEnter (void *ptr, int idx)

 {
  char s[LARGBUF];

    if (!ptr)  return;

   switch (idx)
          {
           case 0:
                DisplayHeader (6, 6);
                ShowChanUsers (uptr, 1);
                finish2 (6, 6);

                break;

           case 1:
                ToServer ("MODE %s +o %s\n", uptr->chptr->channel, uptr->nick);

                break;

           case 2:
                ToServer ("KICK %s %s\n", uptr->chptr->channel, uptr->nick);

                break;

           case 3:
                ToServer ("MODE %s +b %s\n", uptr->chptr->channel, uptr->nick);

                break;
          }

 }  /**/


 boolean UsersRecordEmpty (const Channel *ptr)

 {
   return ((boolean)(ptr->nEntries==NO_USERS));

 }  /**/

