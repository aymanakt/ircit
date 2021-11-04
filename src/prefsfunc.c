/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
*/

#include <main.h>
#include <terminal.h>
#include <updates.h>
#include <prefs.h>
#include <prefsfunc.h>

 MODULEID("$Id: prefsfunc.c,v 1.7 1998/04/06 06:13:44 ayman Beta $");


 STD_IRCIT_PREF(_BEEP_ONMSG)

 {
    if (arethesame(pref->value, "OFF"))
     {
      if (IS_BEEPONMSG())
       {
        mt_ptr->c_output&=~BEEPONMSG;
       }
     }
    else
    if (arethesame(pref->value, "ON"))
     {
      if (!IS_BEEPONMSG())
       {
         mt_ptr->c_output|=BEEPONMSG;
       }
     }

 }   /**/


 STD_IRCIT_PREF(_BLOCK_SIZE)

 {
    if ((!pref->value)||(!atoi(pref->value)))
     {
       if (pref->value)  free (pref->value);

       if (!(pref->value=strdup("512")))
        {
         say ("%$% Memory exhastion.\n", HOT);
         exit (1);
        } 
     }

 }  /**/


 STD_IRCIT_PREF(_COMMAND_CHAR)

 {
  extern char *command_char_ptr;

    if (!pref->value)
     {
      *command_char_ptr=0;

      return;
     }
      
    if (strlen(pref->value)>1)
     {
      say ("%$% Illegal preference definition.\n", MILD);

      pref->value[1]=0;
      *command_char_ptr=*pref->value;
     }

 }  /**/


 STD_IRCIT_PREF(_DISCARD_MIRC)

 {
    if (arethesame(pref->value, "OFF"))
     {
      if (IS_DISCARD_MIRC())
       {
        UNDISCARDMIRC();
       }
     }
    else
    if (arethesame(pref->value, "ON"))
     {
      if (!IS_DISCARD_MIRC())
       {
        DISCARDMIRC();
       }
     }

 }   /**/


 STD_IRCIT_PREF(_OPTION_MENU)

 {
    if (arethesame(pref->value, "OFF"))
     {
      if (IS_SHOWOPTMENU())
       {
        mt_ptr->c_output&=~SHOWOPTMENU;
       }
     }
    else
    if (arethesame(pref->value, "ON"))
     {
      if (!IS_SHOWOPTMENU())
       {
        mt_ptr->c_output|=SHOWOPTMENU;
       }
     }

 }  /**/


 STD_IRCIT_PREF(_STICKY_LISTS)

 {
  #define IS_STICKYLISTSON() (mt_ptr->c_output&STICKY_LISTS)

    if (arethesame(pref->value, "OFF"))
     {
      if (IS_STICKYLISTSON())
       {
        mt_ptr->c_output&=~STICKY_LISTS;
       }
     }
    else
    if (arethesame(pref->value, "ON"))
     {
      if (!IS_STICKYLISTSON())
       {
        mt_ptr->c_output|=STICKY_LISTS;
       }
     }

 }   /**/


 STD_IRCIT_PREF(_STICKY_MENU)

 {
  #define IS_STICKYMENUON() (mt_ptr->c_output&STICKY_MENU)

    if (arethesame(pref->value, "OFF"))
     {
      if (IS_STICKYMENUON())
       {
        mt_ptr->c_output&=~STICKY_MENU;
       }
     }
    else
    if (arethesame(pref->value, "ON"))
     {
      if (!IS_STICKYMENUON())
       {
        mt_ptr->c_output|=STICKY_MENU;
       }
     }

 }   /**/


 STD_IRCIT_PREF(_TELNET_ATTRS)

 {
    if (arethesame(pref->preference, "TELNET_MSG"))
     {
      if (arethesame(pref->value, "OFF"))
       {
        if (IS_SHOWTELNETMSG())
         {
          mt_ptr->c_output&=~TELNET_MSG;
         }
       }
      else
      if (arethesame(pref->value, "ON"))
       {
        if (!IS_SHOWTELNETMSG())
         {
          mt_ptr->c_output|=TELNET_MSG;
         }
       }
     }
    else
    if (arethesame(pref->preference, "TELNET_CMND"))
     {
      if (arethesame(pref->value, "OFF"))
       {
        if (IS_SHOWTELNETCMND())
         {
          mt_ptr->c_output&=~TELNET_CMND;
         }
       }
      else
      if (arethesame(pref->value, "ON"))
       {
        if (!IS_SHOWTELNETCMND())
         {
          mt_ptr->c_output|=TELNET_CMND;
         }
       }
     }

 }   /**/

