/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
*/

#include <main.h>
#include <terminal.h>
#include <list.h>
#include <scroll.h>
#include <serverio.h>

 MODULEID("$Id: help.c,v 1.3 1998/09/19 01:10:06 ayman Beta $");


 STD_IRCIT_COMMAND(uHELP)

 {
  int cnt=0;
  register int i=0;
  extern size_t glob_cnt;
  ITCommand *matches[glob_cnt];
  register ITCommand *ptr=NULL;

    if (!args)
     {
      extern void  ScrollingHelp (void *, int);

       ScrollingHelp (NULL, 0);

       return;
     }

   cnt=CompleteCommand (args, matches, glob_cnt);

    if (!cnt)
     {
      say ("%$% Unable to find %s in symbol table.\n", MILD, args);

      return;
     }

    if (cnt==1)
     {
      ptr=matches[0];

       if ((!ptr->help)||(!*ptr->help))
        {
	 say ("%$% Help on %s is not defined.\n", MILD, args);
	}
       else
        {
         say ("%$% Help on %s:\n%s\n", 
	      INFO, ptr->name, ptr->help);
	} 
     }
    else
     {
      int len;

       say ("%$% Found at least %d possible completions...\n", INFO, cnt);

        for (len=0; len<cnt; len++)
         {
          say (" %-19.19s", ((ITCommand *)*(matches+len))->name);

           if ((len+1)%3==0)  say ("\n");
         }

        if (len%3!=0)  say ("\n");	 
     }	 

 }  /**/


 #include <windows-aux.h>
 #include <helpdef.h>


 void ScrollingHelp (void *ptr, int idx)

 {
  List *lptr; ListEntry *eptr;
  ScrollingList *scrptr=&hlp_list;
  extern int nUserCmnds;
  extern size_t glob_cnt;
  extern ITCommand *itcmnd;

   lptr=ListfromArray (itcmnd, sizeof(ITCommand), glob_cnt);
   scrptr->nEntries=lptr->nEntries;
   (List *)scrptr->list=lptr;

   LoadScrollingList (&hlp_list);

 }  /**/


 void ScrollingHelpEntries (ScrollingList *sptr)

 {
  register int  j=0;
  int maxy,
      last;
  char s[sptr->width],
       t[6];
  register ListEntry *eptr;
  WINDOW *w=RetrieveCurrentWindow();
  #define attr ((ITCommand *)(eptr->whatever))->attrs
  #define command ((ITCommand *)(eptr->whatever))->name

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
         wattron (w, A_REVERSE);
         (ListEntry *)sptr->c_item=(ListEntry *)eptr;
        }

      wmove (w, j-sptr->listwin_start, 0);
      Fwprintf_nout  (w, " %c%-15.15s ", attr&2?'*':' ', command);

       if (j==sptr->hilited)
        {
         wattroff (w, A_REVERSE);
        }
     }

 }  /**/


 void HelpEnter (void *sptr, int idx)

 {
  register ITCommand *itcmd_ptr;

    if (sptr)
     {
      itcmd_ptr=(ITCommand *)((ListEntry *)sptr)->whatever;

       if ((!itcmd_ptr->help)||(!*itcmd_ptr->help))
        {
         say ("%$% Help on %s is not defined.\n", MILD, itcmd_ptr->name);
        }
       else
        {
         say ("%$% Help on %s:\n%s\n",
              INFO, itcmd_ptr->name, itcmd_ptr->help);
        }
     }

 }  /**/

