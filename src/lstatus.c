/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
*/

#include <main.h>
#include <terminal.h>
#include <output.h>
#include <updates.h>
#include <list.h>
#include <lstatus.h>

 MODULEID("$Id: lstatus.c,v 1.1 1998/02/19 11:43:25 ayman Beta $");

 void DisplayLowerStatus (void)

 {
   lower_status_figures (0, 0, 0);

   swmove (1, 8);

   wattrset (StatusWin, COLOR_PAIR(55));
   swprintf (" [%.10s (%$%%$249%$%) (%$%%$249%$%%$249%$%)] [away --:-- (000)]",
             st_ptr->c_nick,
             isop()?111:111|COLOR_PAIR(55),
             isvoice()?118:118|COLOR_PAIR(55),
             isinvisible()?105:105|COLOR_PAIR(55),
             iswallop()?119:119|COLOR_PAIR(55),
             isrestricted()?114:114|COLOR_PAIR(55));
   wattrset (StatusWin, COLOR_PAIR(49));

 }  /**/


  void Away (int ON_OFF)

 {
  int x;

   x=26+strlen(curnick());

   swmove (1, x);

    if (ON_OFF==1)
     {
      swprintf ("[away 00%$%00 (000)]", 58|WA_BLINK);
     }
    else
     {
      wattrset (StatusWin, COLOR_PAIR(55));
      swprintf ("[away 00:00 (000)]");
      wattrset (StatusWin, COLOR_PAIR(49));
     }

   x=x+18;
   swmove (1, x); 
   whline (StatusWin, ' ', 56-x); 

 }  /**/


 void DisplayHistoryTable (const int what, const int flag)

 {
  extern List *irc_notify;
  extern List *utmp_notify;
  #define ptr ht_ptr

   switch (what)
          {
           case EVERYTHING:
                 swmove (1, 56);
                 swprintf ("[%02d:%02d] [%02d:%02d]",
                           ptr->nLines,
                           ptr->c_lineid,
                           ptr->nTabs,
                           ptr->c_tabid);
                 break;

           case H_LINES:
                swmove (1, 63);
                swprintf ("[%02d:00]", ptr->nLines);
                break;

           case H_LINES_D:
                ht_ptr->nLines=ht_ptr->c_lineid=0;
                swmove (1, 63);
                _wattron (StatusWin, COLOR_PAIR(55));
                swprintf ("[00:00] ");
                _wattroff (StatusWin, COLOR_PAIR(55));
                _wattron (StatusWin, COLOR_PAIR(49));
                break;

           case H_LINEID:
                swmove (1, 67);
                swprintf ("%02d]%$%", ptr->c_lineid,flag?ACS_UARROW:ACS_DARROW);
                break;

           case H_TABS:
                swmove (1, 56);
                swprintf ("[%02d:00]", ptr->nTabs);
                break;

           case H_TABS_D:
                ht_ptr->nTabs=ht_ptr->c_tabid=0;
                _wattron (StatusWin, COLOR_PAIR(55));
                swmove (1, 56);
                swprintf ("[00:00]");
                _wattroff (StatusWin, COLOR_PAIR(55));
                _wattron (StatusWin, COLOR_PAIR(49));
                break;

           case H_TABID:
                swmove (1, 60);
                swprintf ("%02d]", ptr->c_tabid);
                break;

           case H_AWAYMSGS:
                {
                 int x;

                  x=26+6+6+strlen(curnick());
                  swmove (1, x);
                  swprintf ("(%03d)", ptr->nAwaymsgs);
                  doupdate ();
                }

           case H_UTMP:
                swmove (0, 0);
                 if (ptr->nUTMP)
                    {
                     swprintf (" [UTMP(%02d:%02d)]",
                              utmp_notify->nEntries, ptr->nUTMP);
                    }
                 else
                    {
                     wattrset(StatusWin, COLOR_PAIR(55));
                     swprintf (" [UTMP(%02d:%02d)]",
                              utmp_notify->nEntries, ptr->nUTMP);
                     wattrset(StatusWin, COLOR_PAIR(49));
                    }
                doupdate ();
                break;

           case H_NOTIFY:
                swmove (0, 14);
                 if (ptr->nNotify)
                    {
                     swprintf (" [NOTIFY(%02d:%02d)]", 
                              irc_notify->nEntries, ptr->nNotify);
                    }
                 else
                    {
                     wattrset(StatusWin, COLOR_PAIR(55));
                     swprintf (" [NOTIFY(%02d:%02d)]",
                              irc_notify->nEntries, ptr->nNotify);
                     wattrset(StatusWin, COLOR_PAIR(49));
                    }
               doupdate (); 
          }

 }  /**/

 #undef ptr

 static LowerStatusControls Controls[]=
 {
  {"Session",       4,  white,   BLOCK, off, A_BOLD}, //0
  {"Server",        5,  cyan,    BLOCK, off, A_BOLD}, //1
  {"Telnet",        7,  magenta, BLOCK, off, A_BOLD}, //2
  {"Telnet Login",  8,  red,     BLOCK, off, A_BOLD|A_BLINK}, //3
  {"DCC CHAT",      10, magenta, BLOCK, off, A_BOLD|A_BLINK}, //4
  {"DCC GET",       11, cyan,    BLOCK, off, A_BOLD|A_BLINK}, //5
  {"DCC SEND",      12, blue,    BLOCK, off, A_BOLD|A_BLINK}, //6
  {"Mail",          14, green,   BLOCK, off, A_BOLD|A_BLINK}, //7
  {"Pending Sig",   16, cyan,    BLOCK, off, A_BOLD}, //8
  {"Reading",       17, magenta, BLOCK, off, A_BOLD}, //9
  {"Merging",       18, yellow,  BLOCK, off, A_BOLD}, //10
 };

 void DisplayControlsPanel (void)

 {
  #define CONTROL_PANEL \
           "%$218%$196%$196[%$%%$% %$%%$% %$%%$%%$% %$%  %$%%$%%$%]%$196"

   sswmove (0, 0);
   Fwprintf (SstatusWin, CONTROL_PANEL,
             Controls[0].figure|Controls[0].color|Controls[0].state,
             Controls[1].figure|Controls[1].color|Controls[1].state,
             Controls[2].figure|Controls[2].color|Controls[2].state,
             Controls[3].figure|Controls[3].color|Controls[3].state,
             Controls[4].figure|Controls[4].color|Controls[4].state,
             Controls[5].figure|Controls[5].color|Controls[5].state,
             Controls[6].figure|Controls[6].color|Controls[6].state,
             Controls[7].figure|Controls[7].color|Controls[7].state,
             Controls[8].figure|Controls[8].color|Controls[8].state,
             Controls[9].figure|Controls[9].color|Controls[9].state,
             Controls[10].figure|Controls[10].color|Controls[10].state);


 }  /**/


 __inline__ void Flash (int type, unsigned long flag)

 {
  static LowerStatusControls *lcntl;

   lcntl=&Controls[type];
   
   lcntl->state=0;
 
    flag?(lcntl->state|=lcntl->on_how):1;

    switch (type)
     {
      /*case 1:
            if (flag)  lcntl->state|=A_BLINK;
            else lcntl->state&=~A_BLINK;
           break;*/
 
      case 3:
            if (!flag)
             mt_ptr->nTelnets>0?(lcntl->state|=A_BOLD):(lcntl->state=0);
            else
             mt_ptr->lights|=TELNET;
           break;

      case 6:
           {
            int sum=mt_ptr->nChats+mt_ptr->nGets;
                
              if (!flag)
               sum>0?(lcntl->state|=A_BOLD):(lcntl->state=0);
              else
               mt_ptr->lights|=DCC;
             break;
            }

      case 7:
            if (flag) mt_ptr->lights|=MAIL;
     }

   DisplayControlsPanel ();
   wrefresh (SstatusWin);

 }  /**/


 void CheckLights (void)

 {
   if (mt_ptr->lights&TELNET)
    {
     mt_ptr->lights&=~TELNET;
     Flash (3, 0);
    }

   if (mt_ptr->lights&DCC)
    {
     mt_ptr->lights&=~DCC;
     Flash (6, 0);
    }

   if (mt_ptr->lights&MAIL)
    {
     mt_ptr->lights&=~MAIL;
     Flash (7, 0);
    }

 }  /**/

