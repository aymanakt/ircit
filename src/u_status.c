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
#include <colors.h>
#include <lastlog.h>
#include <chanrec.h>
#include <people.h>
#include <sockets.h>
#include <updates.h>
#include <u_status.h>

#include <sys/resource.h>


 MODULEID("$Id: u_status.c,v 1.8 1998/03/21 09:41:50 ayman Beta $");

 void (*status[])(void)={[0 ... 6]=NULL};

 extern struct U_Status *ustatus;


 void DoUpperStatus (void)

 {
    if (IS_MENUBAR())  return;

    if (mt_ptr->c_status)
     {
      (*status[mt_ptr->c_status])();
     }

 }  /**/


#define SOCK_CONNECTEDTELNET 3
#define SOCK_CONNECTEDDCC    5
#define SOCK_SOCKSCOUNT      8

 void UpdateSockets (int type, const char c)

 {
   switch (type)
          {
           case SOCK_CONNECTEDDCC:
                c=='-'?mt_ptr->nDccs--:mt_ptr->nDccs++;
                break;
           case M_CHATS:
                c=='-'?mt_ptr->nChats--:mt_ptr->nChats++;
                break;
           case M_SENDS:
                c=='-'?mt_ptr->nSends--:mt_ptr->nSends++;
                break;
           case M_GETS:
                c=='-'?mt_ptr->nGets--:mt_ptr->nGets++;
                break;
           case SOCK_CONNECTEDTELNET:
                c=='-'?mt_ptr->nTelnets--:mt_ptr->nTelnets++;
                break;
           case SOCK_SOCKSCOUNT:
                c=='-'?mt_ptr->nSockets--:mt_ptr->nSockets++;
                break;
           case SOCK_OTHER:
                c=='-'?mt_ptr->nOthers--:mt_ptr->nOthers++;
                break;
           case SOCK_IRCIT:
                //c=='-'?mt_ptr->nIRCIT--:mt_ptr->nIRCIT++;
                break;
          }

    if (mt_ptr->c_status==1)
       {
        DisplayUpperStatusI ();
       }

 }  /**/


#define DASH 179|COLOR_PAIR(CLR_UPPERSTATINV)
#define LTEE 195|COLOR_PAIR(CLR_UPPERSTATINV)
#define RTEE 180|COLOR_PAIR(CLR_UPPERSTATINV)
#define BULK 221|COLOR_PAIR(CLR_UPPERSTATINV)
#define BULK2 222|COLOR_PAIR(CLR_UPPERSTATINV)
#define MIN 196|COLOR_PAIR(CLR_UPPERSTATINV)

 void DisplayUpperStatusI (void)

 {
    if (IS_MENUBAR())  return;

   uswmove (0, 0);
   uswprintf (" %$%%$%TOTAL[%d]  TEL[%d]%$% OTHER[%d]%$% CHAT[%d]%$% GET[%d]%$% SEND[%d]%$%%$%,5%$%MAIL[%d]%$%%$%", 
   MIN, 221|COLOR_PAIR(CLR_UPPERSTATINV), 
   mt_ptr->nSockets,
   mt_ptr->nTelnets, DASH,
   mt_ptr->nOthers, DASH,
   mt_ptr->nChats, DASH,
   mt_ptr->nGets, DASH,
   mt_ptr->nSends, BULK2, MIN, BULK,
   mt_ptr->nMail, BULK2, MIN);

   doupdate();

 }  /**/


 void DisplayUpperStatusII (void)

 {
  int i=0;
  float n;
  char s[10]={0};
  register Someone *pptr;
  register Index *iptr;
  extern People *const people;
  #define dcc ((DCCParams *)(iptr->pindex))

    if (IS_MENUBAR())  return;

   uswmove (0, 0);

   pptr=people->head;
 
    for ( ; pptr!=NULL; pptr=pptr->next)
        {
          if (pptr->nEntries>=1)
             {
              iptr=pptr->head;

               for ( ; iptr!=NULL; iptr=iptr->next)
                   {
                     if (i==9)
                        {
                         wprintf(UpperStatusWin, "....");

                         goto end;
                        }

                     if (iptr->protocol==PROTOCOL_DCC)
                        {
                          if ((dcc->type&DCCGETTING)&&(dcc->flags&CONNECTED))
                             {
                              i++;
                              sprintf (s, "%.0f%%", ((float)dcc->xfer/(float)dcc->size)*100.0);
                              wprintf (UpperStatusWin, "[%s]%$16 ", s);
                             }
                          else
                          if ((dcc->type&DCCSENDING)&&(dcc->flags&CONNECTED))
                             {
                              i++;
                              sprintf (s, "%.0f%%", ((float)dcc->recv/(float)dcc->size)*100.0);
                              wprintf (UpperStatusWin, "[%s]%$17 ", s);
                             }
                        }
                   }
             }
        }

    if (i==0)
       {
        wprintf (UpperStatusWin, " %$196%$180 No active DCC GET/SEND connections! %$195%$196,38");
       }

 end:   
   wclrtoeol (UpperStatusWin);
   doupdate ();

 }  /**/


 void DisplayUpperStatusIII (void)

 {
  int i=0;
  char s[]={[0 ... LARGBUF]=0};
  register Channel *chptr;
  extern ChannelsTable *channels;

    if (IS_MENUBAR())
       {
        return;
       }

   uswmove (0, 0);

    if (!isup())
       {
        uswprintf (" %$196%$180 You are not connected to IRC server! %$195%$196,36");
        goto end;
       }

    if (cht_ptr->nChannels==0)
       {
        uswprintf (" %$196%$180 You are afloat! %$195%$196,57");
        goto end;
       }    

    for (chptr=channels->head; chptr!=NULL; chptr=chptr->next)
        {
         char p[MAXCHANLEN+4];

           if (iscurchan(chptr->channel))
              {
               sprintf (p, "<%s> ", chptr->channel);
              }
           else
              {
               sprintf (p, "[%s] ", chptr->channel);
              }

          strcat (s, p);
        }

   if (sizeof(s)>80)
      {
       s[76]=s[77]=s[78]='.';
       s[79]=0;
      }

  uswprintf  ("%s", s);

 end:
  wclrtoeol (UpperStatusWin);
  doupdate ();

 }  /**/


 void LoadUpperStatusI (void)

 {
   mt_ptr->c_status=1;

   DisplayUpperStatusI ();

 }  /**/


 void LoadUpperStatusII (void)

 {
   mt_ptr->c_status=2;

   DisplayUpperStatusII ();

 }  /**/


 void LoadUpperStatusIII (void)

 {
   mt_ptr->c_status=3;

   DisplayUpperStatusIII ();

 }  /**


 void LoadUpperStatusIV (void)

 {
   mt_ptr->c_status=4;

   DisplayUpperStatusIV ();

 }  /**/


 void LoadUpperStatus (void *ptr, int idx)

 {
   switch (idx)
          {
           case 0:
                LoadUpperStatusI ();
                break;

           case 1:
                LoadUpperStatusII ();
                break;

           case 2:
                LoadUpperStatusIII ();
                break;

           case 3:
                //LoadUpperStatusIV ();
                break;
          }

 }  /**/

