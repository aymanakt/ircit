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
#include <misc.h>
#include <list.h>
#include <stack.h>
#include <scroll.h>
#include <lastlog.h>
#include <chanrec.h>
#include <match.h>
#include <serverio.h>
#include <updates.h>

#include <signal.h>

 MODULEID("$Id: chanrec.c,v 1.10 1998/04/08 08:54:11 ayman Beta $");

 static ChannelsTable ChannelsMaster;
 ChannelsTable *const channels=&ChannelsMaster;
 ChannelsTable *const channels;

 void InitChannelsTable (void)

 {
  register int i;

   ChannelsMaster.head=NULL;
   ChannelsMaster.tail=NULL;
   ChannelsMaster.nEntries=0;
   ChannelsMaster.glock=0;

    for (i=0; i<=10; i++)
     {
      ChannelsMaster.id_hash[i]=(Channel *)NULL;
     }
 
 }  /**/


 void _InitChannelsTable (ChannelsTable *ptr)

 {
  register int i;
  
   memset (ptr, 0, (sizeof(ChannelsTable)));

    for (i=0; i<=10; i++)
     {
      ptr->id_hash[i]=(Channel *)NULL;
     }

 }  /**/


 Channel *ChannelbyName (const char *channel)

 {
  register Channel *chptr=channels->head;

    while (chptr!=NULL)
     {
      if (!strcasecmp(chptr->channel, channel))
       {
        return (Channel *)chptr;
       }
      else
       {
        chptr=chptr->next;
       }
     }

   return (Channel *)NULL;

 }  /**/


 __inline__ int id (const char *channel)

 {
  register Channel *chptr;
  
    if ((chptr=ChannelbyName(channel))!=NULL)
     {
      return chptr->id;
     }
    else
     {
      return 0;
     }

 }  /**/


 Channel *AddtoChannelsTable (ChannelsTable *ptr, const char *channel)

 {
  int i;
  const size_t sizeofChannel=sizeof(Channel);
  register Channel *chptr;

   xmalloc(chptr, sizeofChannel);
   memset (chptr, 0, sizeof(Channel)); 

   /*lptr=&chptr->BanList;
   lptr->head=lptr->tail=NULL;
   lptr->nEntries=0;
   //memset (&chptr->BanList, 0, sizeof(List));*/

   memset (&chptr->chfloods, 0, sizeof(ChannelFlood)*(CHAN_FLOODS-1));

    for (i=0; i<=HASHSIZE; i++)
     {
      chptr->Hash[i]=(ChanUser *)NULL;
     } 

   mstrncpy (chptr->channel, channel, MAXCHANLEN);
  
    if (!ChannelsTableEmpty(ptr))
     {
      ptr->tail->next=chptr;
      ptr->tail=chptr;
     }
    else
     {
      ptr->head=ptr->tail=chptr; 
     }

   ptr->nEntries++;

    if (((ptr->glock>>0)&0x1)&&((ptr->glock>>1)&0x1))
     {
      RefreshListContents (ptr);
     }

   return (Channel *)chptr;

 }  /**/


 Channel *AddChannel (const char *channel)

 {
  register int i=0;
  int k[]={[0 ... 11]=0};
  register Channel *chptr;

   chptr=AddtoChannelsTable (channels, channel);

   chptr->when=time (NULL);

   {
    register Channel *aux=channels->head;

      for ( ; aux!=NULL; aux=aux->next)
       {
        k[aux->id]=1;
       }

      while (k[++i])
            ;
   }

   chptr->id=i;
   channels->id_hash[i]=(Channel *)chptr;
   chptr->status|=(PROCESSING|FRESHLYJOINED);

   Postjoin (channel);
        
   return (Channel *)chptr;

 }  /**/ 


 int RemovefromChannelsTable (ChannelsTable *ptr, const char *channel)

 {
  int found=0;
  register Channel *chptr=ptr->head,
                   *prev=NULL;

    while ((chptr!=NULL)&&(!found))
     {
      if (arethesame(chptr->channel, channel))
       {
        found=1;
       }
      else
       {
        prev=chptr;
        chptr=chptr->next;
       }
     }

    if (!found)  return 0;

    if (prev==NULL)
     {
      ptr->head=chptr->next;
     }
    else
    if (chptr->next==NULL)
     {
      ptr->tail=prev;
      ptr->tail->next=NULL;
     }
    else
     {
      prev->next=chptr->next;
     }

   ptr->nEntries--;

    if (chptr->topic)  free (chptr->topic);

   ptr->id_hash[chptr->id]=(Channel *)NULL;

   //memset (chptr, 0, sizeof(Channel));
   //free (chptr);

    if (((ptr->glock>>0)&0x1&&(ptr->glock>>1)&0x1)||
        ((chptr->glock>>0)&0x1&&(chptr->glock>>1)&0x1))
     {
       if (((chptr->glock>>0)&0x1)&&((chptr->glock>>1)&0x1))
        {
         DestroyWindow (4, chptr);
        }

       if (ptr->nEntries==0)
        {
         DestroyWindow (4, ptr);
        } 
       else 
        {
         RefreshListContents (ptr);
        } 
     }

   memset (chptr, 0, sizeof(Channel));
   free (chptr);

   return 1;

 }  /**/


 int ResetChannelsTable (void)

 {
  int i=0; 
  register Channel *chptr=channels->head,
                   *aux;

    if (ChannelsTableEmpty(channels))  return 0;
       
    while (chptr!=NULL)
     {
      i++;
      aux=chptr->next;

       if (chptr->topic)  free (chptr->topic);
       if (((chptr->glock>>0)&0x1)&&((chptr->glock>>1)&0x1))
        {
         DestroyWindow (4, chptr);
        }

      //ResetBanList (chptr);
      ResetChanUsers (chptr);
      free (chptr);

      chptr=aux;
     }

    for (i=0; i<=10; i++)
     {
      channels->id_hash[i]=(Channel *)NULL;
     }

    if (((channels->glock>>0)&0x1)&&((channels->glock>>1)&0x1)) 
     {
      DestroyWindow (4, channels);
     }

   channels->nEntries=0;
   channels->head=channels->tail=(Channel *)NULL;

   return i;

 }  /**/
        

 Ban *AddBan (Channel *chptr, char *mask)

 {
  register ListEntry *lptr;
  register Ban *bptr;

   lptr=AddtoList (&chptr->BanList);
    if (lptr==(ListEntry *)NULL)
       {
        return (Ban *)NULL;
       }

   bptr=(Ban *)malloc (sizeof(Ban));
    if (bptr==(Ban *)NULL)
       {
        return (Ban *)NULL;
       }

   memset (bptr, 0, sizeof(Ban));

   mstrncpy (bptr->mask, mask, MAXHOSTLEN);

   (Ban *)lptr->whatever=(Ban *)bptr;
   (ListEntry *)bptr->link=(ListEntry *)lptr;

   return (Ban *)bptr;

 }  /**/


 #define MASK(x) (((Ban *)((x)->whatever))->mask)

 int RemoveBan (Channel *chptr, int how, ...)

 {
  List *ptr;
  register ListEntry *lptr;
  register Ban *bptr;

   ptr=(List *)&chptr->BanList;

    if (ptr->nEntries==0)
       {
        return 0;
       }

   switch (how)
          {
           case NUMBER:
                {
                 int i;
                 register int n=0;
                 va_list va;

                  va_start (va, how);
                  i=va_arg (va, int);
                  va_end (va);

                   if (i>ptr->nEntries)
                      {
                       say("%$% You don't have that many Ban entries.\n", INFO);

                       return;
                      }

                  lptr=ptr->head;

                   for ( ; lptr!=NULL; lptr=lptr->next)
                       {
                         if (++n==i)
                            {
                             (Ban *)bptr=(Ban *)lptr->whatever;

                             memset (bptr, 0, sizeof(Ban));
                             free (bptr);
                             RemovefromList (ptr, lptr);

                             return 1; 
                            }
                       }
                 }
                break;

           case NAME:
                {
                 char *mask;
                 unsigned long port;
                 va_list va;

                  va_start (va, how);
                  mask=va_arg (va, char *);
                  va_end (va);

                  lptr=ptr->head;

                  for ( ; lptr!=NULL; lptr=lptr->next)
                      {
                        if (!strcasecmp(mask, MASK(lptr)))
                           {
                            (Ban *)bptr=(Ban *)lptr->whatever;

                            memset (bptr, 0, sizeof(Ban));
                            free (bptr);
                            RemovefromList (ptr, lptr);

                            return 1;
                           }
                       }

                  return 0;
                }
               break;


           case WILDCARD:
                {
                 int i=0;
                 char *mask;
                 unsigned long port;
                 va_list va;
                 ListEntry *aux;

                  va_start (va, how);
                  mask=va_arg (va, char *);
                  va_end (va);

                  lptr=ptr->head;

                   while (lptr!=NULL)
                       {

                         if (match(MASK(lptr), mask))
                            {
                             (Ban *)bptr=(Ban *)lptr->whatever;

                             i++;
                             aux=lptr->next;
                             memset (bptr, 0, sizeof(Ban));
                             free (bptr);
                             RemovefromList (ptr, lptr);
                             lptr=aux;
                            }
                       }

                  return i;
                }

           case ALL:
                {
                 register int i=0;
                 ListEntry *aux;

                  lptr=ptr->head;

                   while (lptr!=NULL)
                         {
                          i++;
                          aux=lptr->next;
                          (Ban *)bptr=(Ban *)lptr->whatever;
                          free (bptr);
                          free (lptr);
                          lptr=aux;
                         }

                 ptr->nEntries=0;
                 ptr->head=ptr->tail=NULL;

                 return i;
                 //return (ResetBanList(chptr));
                }
          }


 }  /**/


 int ResetBanList (Channel *chptr)

 {
  int i=0;
  register List *lptr;
  register ListEntry *eptr,
                     *aux;
  register Ban *bptr;
               
   lptr=(List *)&chptr->BanList;

    if (lptr->nEntries==0)
       {
        return 0;
       }

  eptr=lptr->head;

   while (eptr!=NULL)
    {
     i++;
     aux=eptr->next;
     (Ban *)bptr=(Ban *)eptr->whatever;
     free (bptr);
     free (eptr);
     eptr=aux;
    }

   lptr->nEntries=0;
   lptr->head=lptr->tail=NULL;

   return i;

 }  /**/


 int FlushBans (Channel *chptr)

 {
  int i=0;
  register List *lptr;
  register ListEntry *eptr;
  register Ban *bptr;

   lptr=(List *)&chptr->BanList;

   eptr=lptr->head;   

    for ( ; eptr!=NULL; eptr=eptr->next)
     {
      (Ban *)bptr=(Ban *)eptr->whatever;

       if (bptr->flag&BAN_ENFORCE)
        {
         i++;

          if (bptr->how) //+
           {
            ToServer ("MODE %s +b %s\n", chptr->channel, bptr->mask);
           }
          else
           {
            ToServer ("MODE %s -b %s\n", chptr->channel, bptr->mask);
           }
        }
     }

   return i;

 }  /**/ 


 void ShowBanList (Channel *chptr, int how)

 {
  int i=0;
  register List *ptr;
  register ListEntry *lptr;
  register Ban *bptr;
  extern chtype *f;

   ptr=(List *)&chptr->BanList;

    if (ptr->nEntries==0)
     {
      say ("%$% Ban list on %s is empty.\n", f[chptr->id], chptr->channel);

      return;
     }

   lptr=ptr->head;

    for ( ; lptr!=NULL; lptr=lptr->next)
     {
      i++;
      (Ban *)bptr=(Ban *)lptr->whatever;

      say ("%d: ban: %s, by: %s\n", i, bptr->mask, bptr->who);
     }

 }  /**/


 void RecordChanMode (Channel *chptr, char *m)

 {
  char *s,
       *t;
  int i=1;

    if ((m)&&(*m))
       {
        chptr->status|=PROCESSINGCHANMODE;
        chptr->chanmode=0;
        chptr->key[0]=0;
        chptr->limit=0;

         while (m[i]!=0)
               {
                 if (m[i]=='m')
                    chptr->chanmode|=CHAN_MODERATE;
                 if (m[i]=='i')
                    chptr->chanmode|=CHAN_INVITE;
                 if (m[i]=='t')
                    chptr->chanmode|=CHAN_TOPIC;
                 if (m[i]=='s')
                    chptr->chanmode|=CHAN_SECRET;
                 if (m[i]=='p')
                    chptr->chanmode|=CHAN_PRIVATE;
                 if (m[i]=='n')
                    chptr->chanmode|=CHAN_NO_MSG;
                 if (m[i]=='a')
                    chptr->chanmode|=CHAN_ANON;
                 if (m[i]=='q')
                    chptr->chanmode|=CHAN_QUIET;
                 if (m[i]=='k')
                    {
                     s=strchr (m, ' ');
                     s++;
                     t=strchr (s, ' ');

                      if (t!=NULL)
                         {
                          *t=0;
                          strcpy (chptr->key, s);
                          strcpy (s, t+1);
                         }
                      else
                         {
                          strcpy (chptr->key, s);
                          *s=0;
                         }
                     }
                 if (m[i]=='l')
                    {
                     s=strchr (m, ' ');
                     s++;
                     t=strchr (s, ' ');
                      if (t!=NULL)
                         {
                          *t=1;
                          chptr->limit=atoi(s);
                          strcpy (s, t+1);
                         }
                      else
                         {
                          chptr->limit=atoi(s);
                          *s=0;
                         }
                    } 
                i++;
               }

        chptr->status&=~PROCESSINGCHANMODE;
       }
  
 }  /**/


 void ResetChanmode (char *channel)

 {
  register Channel *ptr=ChannelbyName(channel);

   ptr->chanmode=0;
   ptr->limit=0;
   ptr->key[0]=0;

 }  /**/
 

 void ParseChanmode (const char *channel, char *heree)

 {
  int i=1;
  char s[SMALLBUF];
  register Channel *chptr;

    if ((chptr=ChannelbyName(channel))!=NULL)
       {
        s[0]='+';
         if (chptr->chanmode&CHAN_ANON)
            s[i++]='a';
         if (chptr->chanmode&CHAN_INVITE)
            s[i++]='i';
         if (chptr->limit>0)
            s[i++]='l';
         if (chptr->chanmode&CHAN_MODERATE)
            s[i++]='m';
         if (chptr->chanmode&CHAN_NO_MSG)
            s[i++]='n';
         if (chptr->chanmode&CHAN_SECRET)
            s[i++]='s';
         if (chptr->chanmode&CHAN_TOPIC)
            s[i++]='t';
         if (chptr->chanmode&CHAN_PRIVATE)
            s[i++]='p';
         if (chptr->chanmode&CHAN_QUIET)
            s[i++]='q';

         if (chptr->key[0])
            {
              if (chptr->chanmode) 
                 {
                  s[i++]='+';
                  s[i++]='k';  
                 }
              else
                 {
                  s[i++]='k';
                 }
            }
        s[i]=0;
        
        chptr->key[0]?({s[i]=' ';strcpy(&s[++i], chptr->key);1;}):1;
        chptr->limit>0?({sprintf(heree, "%s %d", s, chptr->limit);1;}):
                       ({sprintf(heree, "%s", s);1;});
       }

 }  /**/


 int LeaveAllChannels (void)

 {
  register Channel *chptr;

   if (channels->nEntries>=1)
    {
     chptr=channels->head;

      for (; chptr!=NULL; chptr=chptr->next)
       {
        ToServer ("PART %s\n", chptr->channel);
       }

     return channels->nEntries;
    }

  return 0;

 }  /**/


 int MassOV (const Channel *chptr, const char *modes)

 {
  register int n=1;
           int len,
               len2;
  char buf[LARGBUF];
  register ChanUser *chuptr;

   sprintf (buf, "MODE %s %s ", chptr->channel, modes);
   len2=strlen (buf);

    for (chuptr=chptr->front->next; chuptr!=NULL; chuptr=chuptr->next)
     {
      strcat (buf, chuptr->nick);
      len=strlen (buf);
      buf[len+1]=0; buf[len]=' ';

       if ((n++)==3)
        {
         ToServer ("%s\n", buf);
         buf[len2]=0;
         n=1;
        }
     }

    if (n>1)
     {
      ToServer ("%s\n", buf);
     }

 }  /**/


 int MassOVWithMask (const Channel *chptr, const char *modes, char *mask)

 {
  register int n=1;
           int len,
               len2;
  char buf[LARGBUF],
       hbuf[MAXHOSTLEN+MAXNICKLEN];
  register ChanUser *chuptr;

   sprintf (buf, "MODE %s %s ", chptr->channel, modes);
   len2=strlen (buf);

    for (chuptr=chptr->front->next; chuptr!=NULL; chuptr=chuptr->next)
     {
      sprintf (hbuf, "%s!%s@%s", 
               chuptr->nick, chuptr->user, chuptr->host);

       if (match(mask, hbuf))
        {
         strcat (buf, chuptr->nick);
         len=strlen (buf);
         buf[len+1]=0; buf[len]=' ';

          if ((n++)==3)
           {
            ToServer ("%s\n", buf);
            buf[len2]=0;
            n=1;
           }
        }
     }

    if (n>1)
     {
      ToServer ("%s\n", buf);
     }

 }  /**/


 int MassBan (const Channel *chptr, char *mask)

 {
  register int n=1;
           int len,
               len2;
  char buf[LARGBUF],
       hbuf[MAXHOSTLEN+MAXNICKLEN];
  register ChanUser *chuptr;

   sprintf (buf, "MODE %s +bbb ", chptr->channel);
   len2=strlen (buf);

    for (chuptr=chptr->front->next; chuptr!=NULL; chuptr=chuptr->next)
     {
      sprintf (hbuf, "%s!%s@%s",
                     chuptr->nick, chuptr->user, chuptr->host);
       if (mask)
        {
         if (!match(mask, hbuf))
          {
           continue;
          }
        }

      strcat (buf, hbuf);
      len=strlen (buf);
      buf[len+1]=0; buf[len]=' ';

       if ((n++)==3)
        {
         ToServer ("%s\n", buf);
         buf[len2]=0;
         n=1;
        }
     }

    if (n>1)
     {
      ToServer ("%s\n", buf);
     }

 }  /**/


 int MassKick (const Channel *chptr, char *mask)

 {
  register int n=1;
           int len,
               len2;
  char buf[LARGBUF],
       hbuf[MAXHOSTLEN+MAXNICKLEN];
  register ChanUser *chuptr;

   sprintf (buf, "KICK %s ", chptr->channel);
   len2=strlen (buf);

    for (chuptr=chptr->front->next; chuptr!=NULL; chuptr=chuptr->next)
     {
      sprintf (hbuf, "%s!%s@%s",
                      chuptr->nick, chuptr->user, chuptr->host);
       if (mask)
        {
         if (!match(mask, hbuf))
          {
           continue;
          }
        }

      strcat (buf, chuptr->nick);
      len=strlen (buf);
      buf[len+1]=0; buf[len]=',';

       if ((n++)==4)
        {
         ToServer ("%s\n", buf);
         buf[len2]=0;
         n=1;
        }
     }

    if (n>1)
     {
      ToServer ("%s\n", buf);
     }

 }  /**/


 char *chLastLogs[]={"PUB", "JOIN", "PART", "KICK", "MODE", "OTHR"};
#ifdef LAST__LOG
 char *FetchChanLogLevel (const Channel *chptr, char *buf)

 {
  register int i=-1;
  char s[SMALLBUF]={0};
  char *p[]={"PUB", "JOIN", "PART", "KICK", "MODE", "OTHR"};
  
   buf[0]=0;

    while (++i<6)
     {
       if (chptr->lastlogs[i].max)
        {
         snprintf(s,MEDIUMBUF, "%s:%d,", chLastLogs[i], chptr->lastlogs[i].max);
         strcat (buf, s);
        }
     }
   
   buf[strlen(buf)-1]=0;
 
   return buf;  

 }  /**/
#endif


 void ShowChLastLogs (const Channel *chptr)

 {
#ifdef LAST__LOG
  register int i=0;

   say ("%$% Last Log settings on channel %s...\n", INFO, chptr->channel);

   DisplayHeader (18, 3);

    for (i=0; i<=4; i++)
     {
      say (" %$179%-8.8s%$179%-5d%$179%-7d%$179\n",
           chLastLogs[i], chptr->lastlogs[i].max, 
           chptr->lastlogs[i].log.nEntries);
     }

   finish2(18, 3);
#endif

 }  /**/


 void ShowChannelsTable (const ChannelsTable *ptr, int x, int y)

 {
  int i=0;
  char s[10],
       p[MINIBUF],
       q[LARGBUF];
  register Channel *chptr;
  extern chtype *f;

    if (channels->nEntries==0)
     {
      say ("%$% You are afloat.\n", MILD);

      return;
     }

   chptr=ptr->head;

    while ((++i)<=x)
     {
      chptr=chptr->next;
     }
   i--; 

   DisplayHeader (2, 7);

    for ( ; chptr!=NULL, ((++i)<=(y)); chptr=chptr->next)
     {
      strftime (s, 10, "%H:%S", localtime(&chptr->when));

      ParseChanmode (chptr->channel, p);
#ifdef LAST__LOG
      FetchChanLogLevel (chptr, q);
#endif
      say (" %$179%-2d%$179%-2d %$%%$179%-12.10s%$179%-5.3d%$179%-6.6s"
           "%$179%-5.5s%$179%-20.20s%$179\n", i,
           chptr->id, f[chptr->id], chptr->channel, chptr->nEntries, 
           p, s, *q?q:"  -  ");
     }

   finish2 (2, 7);

   //say ("%$% LastLog settings for %s...\n", INFO, chptr->channel);
   //ShowChLastLogs (chptr);

 }  /**/


 void AdjustChannels (ChannelsTable *ptr, char *chan, int all)

 {
  char m[SMALLBUF];
  register Channel *chptr;

    if (chan==(char *)NULL)  
     {
       if (ChannelsTableEmpty(ptr))
        {
         ResetChannelTable (EVERYTHING);
         (Channel *)cht_ptr->c_chptr=(Channel *)NULL;

         return;
        } 

      chptr=ptr->tail;
      strcpy (cht_ptr->c_channel, chptr->channel);
      cht_ptr->nChannels=ptr->nEntries;
      cht_ptr->c_channelid=chptr->id;
      cht_ptr->nUsers=chptr->nEntries;

      ParseChanmode (chptr->channel, m);
      strcpy (cht_ptr->c_channelmode, m);

       if (chptr->chanop)
        {
         UpdateServerTable (SERV_UMODE, USER_CHANOP, NULL, '+');
        }
       else
        {
         UpdateServerTable (SERV_UMODE, USER_CHANOP, NULL, '-');
        }

      (Channel *)cht_ptr->c_chptr=(Channel *)chptr;
     }
    else 
     {
       if ((chptr=ChannelbyName(chan))!=NULL)
        {
         ParseChanmode (chptr->channel, m);
         strcpy (cht_ptr->c_channel, chptr->channel);
         strcpy (cht_ptr->c_channelmode, m);
         cht_ptr->nChannels=ptr->nEntries;
         cht_ptr->c_channelid=chptr->id;
         cht_ptr->nUsers=chptr->nEntries;

          if (chptr->chanop)
           {
            UpdateServerTable (SERV_UMODE, USER_CHANOP, NULL, '+');
           }
          else
           {
            UpdateServerTable (SERV_UMODE, USER_CHANOP, NULL, '-');
           }

         (Channel *)cht_ptr->c_chptr=(Channel *)chptr;
        }
     }

 }  /**/


 char *SwitchtoChannel (const char *channel)

 {
  register Channel *chptr;

   chptr=ChannelbyName (channel);

    if (chptr!=NULL)
     {
       if (chptr->next)
        {
         AdjustChannels (channels, chptr->next->channel, 1);
         UpdateUpper ();

         return (char *)chptr->next->channel;
        }
       else
        {
         chptr=channels->head;
         AdjustChannels (channels, chptr->channel, 1);
         UpdateUpper ();

         return (char *)chptr->channel;
        }
     }
    else
     {
      return (char *)NULL;
     }

 }  /**/


 void Postjoin (const char *channel)

 {
   ToServer ("MODE %s\n", channel);
   ToServer ("WHO %s\n", channel);

 }  /**/



 void ScrollingChannelsList (ChannelsTable *);
 void ScrollingChannelsInit (ScrollingList *);
 void ScrollingChannelsEntries (ScrollingList *);
 void ScrollingChannelsEnter (void *, int);


 #include <chanrec-data.h>


 void ScrollingChannels (void *ptr, int idx)

 {
   ScrollingChannelsList (channels);

 }  /**/


 void ScrollingChannelsList (ChannelsTable *ptr)

 {
  extern ScrollingList ch_list;
  ScrollingList *scrptr=&ch_list;

    if (ptr->nEntries==0)
     {
      say ("%$% You are afloat.\n", MILD);

      return;
     }

   ((ScrollingList *)(scrptr->opt_menu))->parent=scrptr;

   scrptr->nEntries=ptr->nEntries;
   (ChannelsTable *)scrptr->list=ptr;

   ch_list.lock=&ptr->glock;

   LoadScrollingList (&ch_list);
  
 }  /**/


 void ScrollingChannelsInit (ScrollingList *sptr)

 {
  Channel *chptr=(Channel *)sptr->c_item;

    if (iscurchan(chptr->channel))
     {
      sptr->attrs[0]=0;
     }
    else
     {
      sptr->attrs[0]=1;
     }

    if (!chptr->chanop)
     {
      sptr->attrs[3]=sptr->attrs[4]=sptr->attrs[5]=0;
     }
    else
     {
      sptr->attrs[3]=sptr->attrs[4]=sptr->attrs[5]=1;
     }

 }  /**/


 #define chptr ((Channel *)ptr)

 void ScrollingChannelsEnter (void *ptr, int idx)

 {
  char s[MEDIUMBUF];

    if (!ptr)  return;

   switch (idx)
           {
            case 0:
                 sprintf (s, "%s", chptr->channel);
                 uJOIN (s);
                 break;

            case 1:
                 ScrollingChanusers (chptr);
                 break;
      
            case 2:
                sprintf (s, "%s", chptr->channel);
                uPART (s);
                break;

            case 3: 
                 sprintf (s, "%s +o", chptr->channel);
                 uMASS (s);
                 break;

            case 4:
                 sprintf (s, "%s -o", chptr->channel);
                 uMASS (s);
                 break;

            case 5:
                 sprintf (s, "%s +k", chptr->channel);
                 uMASS (s);
                 break;
           }

 }  /**/

 #undef chptr


 void ScrollingChannelsEntries (ScrollingList *sptr)

 {
  register int j=0;
  int maxy,
      last;
  chtype ch=' ';
  register Channel *chptr;
  WINDOW *w=(WINDOW *)RetrieveCurrentWindow();
  extern Stack *const stack;

   getmaxyx (w, maxy, last);
   last=maxy+sptr->listwin_start;

    for (chptr=((ChannelsTable *)sptr->list)->head; 
         (chptr!=NULL)&&(j++!=sptr->listwin_start); 
         chptr=chptr->next)
        ;

    for (--j;
         (j<last)&&(j<((ChannelsTable *)sptr->list)->nEntries)&&(chptr!=NULL);
         j++, chptr=chptr->next)
     {
       if (j==sptr->hilited)
        {
         _wattron (w, A_REVERSE);
         (Channel *)sptr->c_item=(Channel *)chptr;
        }

       if (iscurchan(chptr->channel))
        {
         ch=17|A_BLINK;
        }
       else
        {
         ch=' ';
        }

      _wmove (w, j-sptr->listwin_start, 0);
      Fwprintf (w, " %-15.15s%$179%-5d%$179%-10s %$% ",
                chptr->channel, chptr->nEntries, " ", ch);

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


 boolean ChannelsTableEmpty (const ChannelsTable *ptr)

 {
   return ((boolean) (ptr->nEntries==NO_CHANNEL));

 }  /**/


 boolean ChannelsTableFull (const ChannelsTable *ptr)

 {
   return ((boolean) (ptr->nEntries==NO_MORE_CHANNELS));

 }  /**/

