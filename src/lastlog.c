/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
*/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifdef LAST__LOG

#include <main.h>
#include <terminal.h>
#include <updates.h>
#include <list.h>
#include <lastlog.h>
#include <chanrec.h>


 MODULEID("$Id: lastlog.c,v 1.1 1998/05/30 11:31:45 ayman Beta $");

 static void LastLogTables (void);

 /*List lastlog;

 struct LastLog {
         time_t when;
         unsigned type;
         char *msg;
        };
 typedef struct LastLog LastLog;

 struct LastLogParams {
         const char *name;
         size_t max;
         List log;
        };
 typedef struct LastLogParams LastLogParams;*/

 LastLogParams LastLogs[]={
                {"MSG",  0, {0, 0, NULL, NULL}},
                {"NOTC", 0, {0, 0, NULL, NULL}},
                {"CTCP", 0, {0, 0, NULL, NULL}},
                {"MISC", 0, {0, 0, NULL, NULL}}, /*invite,notify, yr nick*/
                {"TEL",  0, {0, 0, NULL, NULL}},
               };

 int static ActuallyAddtoLastLog (List *, void *);


 int static __inline__ ActuallyAddtoLastLog (List *lptr, void *ptr)

 {
  register ListEntry *aux;

   aux=AddtoList (lptr);

   aux->whatever=ptr;

   return lptr->nEntries;

 }  /**/


 int AddtoLastLog (LogType type, void *chptr, chtype *s)

 {
  List *lptr;
  LastLogParams *pptr=NULL;
  LastLog *gptr;

    if (chptr)
     {
      pptr=&((Channel *)chptr)->lastlogs[type];
     }
    else
     {
      pptr=&LastLogs[type];
     }

   lptr=&pptr->log;

   xmalloc(gptr, (sizeof(LastLog)));
   memset (gptr, 0, sizeof(LastLog));

   gptr->when=time (NULL);
   gptr->msg=chstrdup (s);

    if (lptr->nEntries==pptr->max)
     {
      RemoveListHead (lptr, 1, FinishoffLastLog);
     }

   return (ActuallyAddtoLastLog(lptr, gptr));

 }  /**/


 void FinishoffLastLog (ListEntry *eptr)

 {
  register LastLog *gptr;

    if (gptr=eptr->whatever) 
     {
       if (gptr->msg)  free (gptr->msg);

      free (gptr);
     }

 }  /**/


 void ClearLastLog (LastLogParams *lgptr)

 {
  List *lptr=&lgptr->log;
  register ListEntry *aux,
                     *eptr=lptr->head;

    while (eptr!=NULL)
     {
      aux=eptr->next;
      FinishoffLastLog (eptr);
      free (eptr);
      eptr=aux;
     }

   lptr->nEntries=0;
   lptr->head=lptr->tail=NULL;

 }  /**/


 int isLogLevel (LogType type)

 {
   return (LastLogs[type].max?1:0);

 }  /**/


 LastLogParams *GetLog (const char *type, const char *chan, int *c)

 {
  register Channel *chptr;
  register LastLogParams *lgptr=NULL;

    if (arethesame(type, "MSG"))   
     {
      //c==NULL?:(*c=0x1);
      lgptr=&LastLogs[0];
     }
    else
    if (arethesame(type, "NOTC"))  
     {
      //c==NULL?:(*c=0x2);
      lgptr=&LastLogs[1];
     }
    else
    if (arethesame(type, "CTCP"))  
     {
      //c==NULL?:(*c=0x4);
      lgptr=&LastLogs[2];
     }
    else
    if (arethesame(type, "MISC"))
     {
      lgptr=&LastLogs[3];
     }
    else
    if (arethesame(type, "TEL"))
     {
      lgptr=&LastLogs[4];
     }
    else
    if (*chan)
     {
       if (chptr=ChannelbyName(chan))
        {
         if (arethesame(type, "PUB")) /*privmsg, notc, ctcp */  
          {
           //c==NULL?:(*c=0x1);
           lgptr=&chptr->lastlogs[0];
          }
         else
         if (arethesame(type, "JOIN"))  
          {
           lgptr=&chptr->lastlogs[1];
          }
         else
         if (arethesame(type, "PART"))  /* part, quit, netsplit */
          {
           lgptr=&chptr->lastlogs[2];
          }
         else
         if (arethesame(type, "KICK"))  
          {
           lgptr=&chptr->lastlogs[3];
          }
         else
         if (arethesame(type, "MODE"))  
          {
           lgptr=&chptr->lastlogs[4]; 
          }  
         else
         if (arethesame(type, "OTHR")) /*topic, */
          {
           lgptr=&chptr->lastlogs[5];
          }  
        }
     }

   return lgptr;

 }  /**/


 /*
 ** lastlog -msg, -notc, -ctcp, -srv[ -pub, kick, mode, join, part, all
 */
#define ISCHANNEL(x) ((*(x)=='#')||(*(x)=='&')||(*(x)=='+'))

 STD_IRCIT_COMMAND(uLASTLOG)

 {
  int i=0;
  char *s="",
       *ch="";
  extern char tok[][80];
 
    if (!args)
     {
      LastLogTables ();

      return;
     }
    
   splitargs2 (args, ' ');

    if (*tok[i]=='-')  s=tok[i++]+1;
    if ((*tok[i])&&(ISCHANNEL(tok[i])))  ch=tok[i++];
    else
    if (*curchan())  ch=curchan();

    if (*s)
     {
      LastLogParams *auxlgptr;

       while (s)
        {
          if (auxlgptr=GetLog(tokenize(&s, ','), ch, NULL))
           {
            ShowLastLog (&auxlgptr->log);
           } 
          else
           {
            say ("Invalid switch?\n");
           }
        } 
     }

 }  /**/


 /*
 ** Non-channel specific LastLogs
 */
 char *FetchLogLevel (char **b)

 {
  char s[SMALLBUF]={0};
  char p[LARGBUF]={0};

    if (LastLogs[0].max)  
     {
      snprintf (s, SMALLBUF, "%s:%d,", "MSG", LastLogs[0].max);
      strcat (p, s);
     }

    if (LastLogs[1].max)
     {
      snprintf (s, SMALLBUF, "%s:%d,", "NOTC", LastLogs[1].max);
      strcat (p, s);
     }

    if (LastLogs[2].max) 
     {
      snprintf (s, SMALLBUF, "%s:%d,", "CTCP", LastLogs[2].max);
      strcat (p, s);
     }

    if (LastLogs[3].max)
     {
      snprintf (s, SMALLBUF, "%s:%d,", "MISC", LastLogs[3].max);
      strcat (p, s);
     }

    if (LastLogs[4].max)
     {
      snprintf (s, SMALLBUF, "%s:%d,", "TEL", LastLogs[4].max);
      strcat (p, s);
     }


   p[strlen(p)-1]=0;

    if (*p)  return (*b=strdup(p));
    else  return (*b=NULL);
 
 }  /**/


 int ParseLastLogPref (char *s, const char *ch)

 {
  char c;
  int m, 
      n=0;
  register char *aux=NULL;
  register LastLogParams *lgptr;

   (*s=='+'||*s=='-')?(c=*s++):(c=0);  

    if (aux=strchr(s, ':'))
     {
      *aux=0;
      aux++;
     }

    if (!(lgptr=GetLog(s, ch, &n)))  return 0;

   switch (c)
    {
     case 0:
     case '+':
           {
             if (aux)
              {
                if (!isdigits(aux))  m=-1;  /* +msg:hh3 */
                if ((m=atoi(aux))<0)  m=-2; /* +msg:-34 */
              }
             else
              {
               m=-3;  /* +msg */
              }

            break;
           }

     case '-':
           {
            m=0;
            break;
           }
    }

     if (m<0)  return 0;  /* incorrect entry */
     if ((m==lgptr->max)&&(lgptr->max))  return 1;

     if (m==0)
      {
       lgptr->max=0;

        if (lgptr->log.nEntries)
         {
          say ("%$% Clearing %d %s-LastLog entries...\n", 
               INFO, lgptr->log.nEntries, s);

          ClearLastLog (lgptr);
         }
      }
     else
      {
        if (m>lgptr->max) 
         {
          lgptr->max=m;
         }
        else
         {
          lgptr->max=m;

           if (lgptr->log.nEntries>m)
            {
             say ("%$% Resizing %s-LastLog...\n", INFO, s);
          
             RemoveListHead (&lgptr->log, lgptr->log.nEntries-m, 
                             FinishoffLastLog);   
            }
         }
      }
  
 }  /**/


#include <prefs.h>

 /*
 ** -msg,+mode:23,mode:20 [#channel]
 */
 STD_IRCIT_PREF(_LASTLOG_LEVEL)

 {
    if (!pref->value)
     {
      //st_ptr->loglevel=0;

      /* reset all logs */
      return;
     }

   {
    char *se, *ch, *aux; 
    char s[strlen(pref->value)+1];

     strcpy (s, pref->value), se=s;

      if ((aux=splitw(&se)), (!se))
       {
        (*curchan())?(ch=curchan()):(ch="");
       }
      else
       {
        ch=se;
       }
         
      while (aux)
       {
        ParseLastLogPref (tokenize(&aux, ','), ch);
       }

     free (pref->value);
     FetchLogLevel (&pref->value);
   }

 }  /**/


 void LastLogTables (void)

 {
  register int i=0;
  
   DisplayHeader (18, 3);

    for (i=0; i<=4; i++)
     {
      say (" %$179%-8.8s%$179%-5d%$179%-7d%$179\n", 
           LastLogs[i].name, LastLogs[i].max, LastLogs[i].log.nEntries);
     }

   finish2(18, 3);

 }  /**/


 int ShowLastLog (List *lptr)

 {
  register ListEntry *eptr;

    if (!lptr->nEntries)
     {
      say ("%$% No entries found in Log.\n", MILD);

      return 0;
     }

   
    for (eptr=lptr->head; eptr!=NULL; eptr=eptr->next)
     {
      //say ("%z", ((LastLog *)eptr->whatever)->msg);
      nYelll (((LastLog *)eptr->whatever)->msg);
     } 

   return lptr->nEntries;

 }  /**/
#endif

