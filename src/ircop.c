/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
*/

#include <main.h>
#include <terminal.h>
#include <updates.h>
#include <useri.h>
#include <dialogbox.h>
#include <ircop-data.h>

 MODULEID("$Id: ircop.c,v 1.1 1998/04/22 04:16:19 ayman Beta $");

 void DialogOperEnter (DialogBox *);

 STD_IRC_COMMAND(uOPER)

 {
  register char *n="";
  register Field *fptr;

    if (args)
       {
         if (n=splitw(&args), args)
            {
             ToServer ("OPER %s %s\n", n, args);

             return;
            }
        
        fptr=&dlg_ircop.fptr[0];
        strcpy (fptr->edit.buffer/*(dptr->fptr+0)->edit.buffer*/, n);
        fptr->flags|=(1<<3);
       }

   LoadDialogBox (&dlg_ircop);

 }  /**/


 void DialogOperEnter (DialogBox *dptr)

 {
  register Field *fptr;

    if ((!dptr)||(fptr=&dptr->fptr[dptr->c_field], !fptr))  return;

    if (fptr->flags&(0x1<<4))
       {
        return;
       }
     
    if ((!*((dptr->fptr+0)->edit.buffer))||(!*((dptr->fptr+1)->edit.buffer)))
       {
        return;
       }

   ToServer ("OPER %s %s\n", (dptr->fptr+0)->edit.buffer, 
                             (dptr->fptr+1)->edit.buffer);

 }  /**/

