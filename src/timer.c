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
#include <updates.h>
#include <notify.h>
#include <timer.h>

 void ScrollingTimersEntries (ScrollingList *);
 void ScrollingTimersEnter (void *, int);
 void ScrollingTimersInit (ScrollingList *);

#include <timer-data.h>

#include <signal.h>

 MODULEID("$Id: timer.c,v 1.8 1998/04/06 06:13:44 ayman Beta $");

 int nTimers=8;

 void InitTimers (void)

 {
  int i;
  time_t now=time(NULL);

    for (i=0; i<=nTimers-1; i++)
     {
       if (timers[i].flag&UNUSED)  continue;

      timers[i].when=now;
      timers[i].flag|=LEAVEME;
     }

   alarm (15);
  
 }  /**/


 struct Timer *GetFreeEntry (void)

 {
  register int i;
           int found=0;

    if (nTimers==NRTIMERS)
     {
      return (struct Timer *)NULL;
     }

    for (i=nTimers; i<=NRTIMERS-1; i++)
     {
      if (timers[i].flag&UNUSED)
       {
        found=1;
        break;
       }
     }

    if (!found)
     {
      return (struct Timer *)NULL;
     }

   nTimers++;

   return &timers[i];

 }  /**/


 int AddmetoTimers (int flag, int lapse, char *info, void(*func)(void))

 {
  struct Timer *ptr;

    if ((ptr=GetFreeEntry())!=NULL)
     {
      ptr->flag|=flag;
      ptr->flag&=~UNUSED;
      ptr->when=time (NULL);
      ptr->lapse=lapse;

       if (info)
        {
         ptr->info=strdup (info);
        }
       else
        {
         ptr->info=strdup ("Unknown");
        }

      ptr->func=func;

      return 1;
     }

   return 0;

 }  /**/ 


#define OUT_OF_RANGE() (id>nTimers)

 void ResetThisTimerEntry (int id)

 {
  int i;

   if (OUT_OF_RANGE())
    {
     say ("%$% Unable to reset Timer Entry %d - out of range.\n", MILD, id);

     return;
    }

    for (i=0; i<=nTimers-1; i++)
     {
      if (id==timers[i].id)
       {
        timers[i].flag|=UNUSED;
        timers[i].flag&=~(DOMEONCE|LEAVEME);
        timers[i].func=(void(*)())NULL;
         if (timers[i].info)
          {
           free (timers[i].info);
          }
       }
     }

   nTimers--;

 }  /**/
  

 int istimeractive (int id)

 {
  struct Timer *ptr;

   ptr=&timers[id-1];

    if (ptr->flag&LEAVEME)
     {
      return 1;
     }
    else
     {
      return 0;
     }

 }  /**/


 void ActivateTimer (int id, char *desc)

 {
  register int i;

    if (OUT_OF_RANGE())
     {
      say ("%$% Unable to activate Timer Entry %d - out of range.\n",MILD, id);

      return;
     }

    for (i=0; i<=nTimers-1; i++)
     {
      if (id==timers[i].id)
       {
        timers[i].flag&=~UNUSED;
        timers[i].flag|=LEAVEME;
        timers[i].when=time (NULL);
       }
     }

 }  /**/


 void DeActivateTimer (int id, char *desc)

 {
  register int i;

    if (OUT_OF_RANGE())
     {
      say ("%$% Unable to de-activate Timer Entry %d - out of range.\n", 
            MILD, id);

      return;
     }

    for (i=0; i<=nTimers-1; i++)
     {
      if (id==timers[i].id)
       {
        timers[i].flag&=~LEAVEME;
        timers[i].flag|=UNUSED;
       }
     }

 }  /**/


 void ExceTimers (int signo) 

 {
  int sec=15,
      aux=0;
  register int i;
  time_t now=time (NULL);

    for (i=0; i<=nTimers-1; i++)
     {
      if ((timers[i].flag&LEAVEME)||(timers[i].flag&DOMEONCE))
       {
        if ((now-timers[i].when)>=timers[i].lapse)
         {
          timers[i].when=now;
          timers[i].func();

           if (timers[i].flag&DOMEONCE)
            {
             ResetThisTimerEntry (timers[i].id);
            }
         }
        else
         { 
          aux=timers[i].lapse-(now-timers[i].when);

           if (aux<sec)
            {
             sec=aux;
            }
         }
       }
     }

   alarm (sec>0?sec:1);

 }  /**/

 static int usable30=10;
 static void (*Every30Sec[])(void)={[0 ... 9]=NULL};


 int RegisterWithEvery30Seconds (void (*func)(void))

 {
    if (usable30>0)
     {
      register int i=0;

       for ( ; i<=9; i++)
        {
         if (!(Every30Sec[i]))
          {
           Every30Sec[i]=func;
           usable30--;

           return 1;
          }
        }
     }

   return 0;  

 }  /**/


 int UnregisterWithEvery30Seconds (void (*func)(void))

 {
  register int i=0;

    for ( ; i<=9; i++)
     {
      if (Every30Sec[i]==func)
       {
        Every30Sec[i]=NULL;
        usable30++;

        return 1;
       }
     }

   return 0;

 }  /**/


 int RegisteredWithEvery30Seconds (void (*func)(void))

 {
  register int i=0;

    for ( ; i<=9; i++)
     {
      if (Every30Sec[i]==func)
       {
        return 1;
       }
     }

   return 0;

 }  /**/


 void ExceEvery30Seconds (void)

 {
  register int i=0;

    for ( ; i<=9; i++)
     {
      if (Every30Sec[i])
       {
        Every30Sec[i]();
       }
     }

 }  /**/


 void ShowTimerEntries (void)

 {
  register int i;
  char s[20];

    if (!nTimers)
     {
      say ("%$% No Timer entries.\n", INFO);

      return;
     }
  
   say ("%$% Reporting on Timer entries...\n", INFO);

   DisplayHeader (5, 5);

    for (i=0; i<=nTimers-1; i++)
     {
       if (timers[i].flag&UNUSED)
           strcpy (s, "UNUSED");
       else
       if (timers[i].flag&LEAVEME)
           strcpy (s, "LEAVEME");
       else
       if (timers[i].flag&DOMEONCE)
           strcpy (s, "DOMEONCE");

      say (" %$179%-3.2d%$179%-14.14s%$179%-5.4d%$179%-8.8s%$179%-4.4d secs%$179\n",
                    timers[i].id,
                    timers[i].info,
                    timers[i].lapse,
                    s,
                    timers[i].flag&(1|2)?
                    timers[i].lapse-(time(NULL)-(timers[i].when)):0);
     }

    finish2 (5, 5);

 }  /**/


 void ScrollingTimers (void *ptr, int idx)

 {
  List *lptr;
  ScrollingList *scrptr=&tmr_list;

   lptr=ListfromArray (timers, sizeof(struct Timer), nTimers-1);
   scrptr->nEntries=lptr->nEntries;
   (List *)scrptr->list=lptr;
   ((ScrollingList *)(scrptr->opt_menu))->parent=scrptr;

   LoadScrollingList (&tmr_list);

 }  /**/


 #define DETERMINE_FLAGS() \
          if (tptr->flag&UNUSED) \
             sprintf (s, "%-7s", "UNUSED"); \
          else \
          if (tptr->flag&LEAVEME) \
             sprintf (s, "%-7s", "LEAVEME"); \
          else \
          if (tptr->flag&DOMEONCE) \
             sprintf (s, "%-7s", "DOMEONCE")

 void ScrollingTimersEntries (ScrollingList *sptr)

 {
  register j=0;
  int maxy,
      last;
  char s[sptr->width];
  register ListEntry *eptr;
  WINDOW *w;
  #define tptr ((struct Timer *)(eptr->whatever))

   w=(WINDOW *)RetrieveCurrentWindow ();

   getmaxyx (w, maxy, last);

   last=maxy+sptr->listwin_start;

    for (eptr=((List *)sptr->list)->head; 
         (eptr!=NULL)&&(j++!=sptr->listwin_start); 
         eptr=eptr->next)
        ;

    for (--j; (j<last)&&(j<sptr->nEntries);
         j++, eptr=eptr->next)
     {
       if (j==sptr->hilited)
        {
         _wattron (w, A_REVERSE);
         sptr->c_item=eptr->whatever;
        }

      _wmove (w, j-sptr->listwin_start, 0);
      DETERMINE_FLAGS();
      Fwprintf (w, " %-2d%$179%-15.15s%$179%-6d%$179%s%$179%-9d ",
                tptr->id, tptr->info, tptr->lapse, s,
                tptr->flag&(1|2)?
                tptr->lapse-(time(NULL)-(tptr->when)):0);

       if (j==sptr->hilited)
        {
         _wattroff (w, A_REVERSE);
        }
     }

 }  /**/

#undef tptr


 void ScrollingTimersInit (ScrollingList *sptr)

 {
  #define tptr ((struct Timer *)sptr->c_item)

    if (tptr->flag&LEAVEME)  sptr->attrs[0]=0;
    else sptr->attrs[0]=1;

    if ((tptr->flag&UNUSED)&&(tptr->func))  sptr->attrs[1]=0;
    else  sptr->attrs[1]=1;
   
    if (!(tptr->flag&(LEAVEME|DOMEONCE)))  sptr->attrs[2]=0;
    else  sptr->attrs[2]=1;

 }  /**/

 #undef tptr


 #define tptr ((struct Timer *)ptr)

 void ScrollingTimersEnter (void *ptr, int idx)

 {
    if (!ptr)  return;

   switch (idx)
          {
           case 0:
                  if ((tptr->flag&UNUSED)&&(tptr->func))
                     {
                      ActivateTimer (tptr->id, NULL);

                      say ("%$% Timer %d (%s) activated.\n", INFO,
                            tptr->id, tptr->info);
                     }

                break;

           case 1:
                  if (tptr->flag&LEAVEME)
                     {
                      DeActivateTimer (tptr->id, NULL);

                      say ("%$% Timer %d (%s) deactivated.\n", INFO,
                            tptr->id, tptr->info);
                     }
                  else
                  if ((tptr->flag&UNUSED)&&(tptr->func))
                     {
                      say ("%$% Timer currently inactive.\n", MILD);
                     }

                break;

           case 2:
                  if (tptr->flag&(LEAVEME|DOMEONCE))
                     {
                      say ("%$% Timer was due in %d seconds time...\n",
                           INFO, tptr->lapse-(time(NULL)-(tptr->when)));

                      tptr->when=time(NULL);
                      tptr->func();

                       if (tptr->flag&DOMEONCE)
                          {
                           ResetThisTimerEntry (tptr->id);
                          }
                     }
                  else
                     {
                      say ("%$% Timer currently inactive.\n", MILD);
                     }

                break;

           case 3:
                say ("%$% Option unsupported.\n", MILD);
                break;
          }

 }  /**/

