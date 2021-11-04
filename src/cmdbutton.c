/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
*/

#include <main.h>
#include <terminal.h>
#include <output.h>
#include <windows-aux.h>
#include <colors.h>
#include <FPL.h>
#include <FPL_protos.h>
#include <fpl.h>

#include <objects.h>
#include <cmdbutton.h>

 MODULEID("$Id$");


 static void DrawCommandButton (StaticButton *, WINDOW *);
 static int CommandButtonInterface (ObjectContainer *, int, void *);


  static void
 MarkItemUnderline (const ObjectParticulars *itmptr, WINDOW *wptr)

 {
    if ((itmptr->underline>=0)&&
        (itmptr->underline<=strlen(itmptr->name)))
     {
      wptr->_line[itmptr->obgeom.starty].text[itmptr->underline]|=A_BOLD;
     }

 }  /**/


 static void DrawCommandButton (StaticButton *stbutptr, WINDOW *wptr)

 {
  extern const ObjectsAndColors *const _mbox;

    if (!*stbutptr->item.name)  stbutptr->item.stat|=ITST_DISABLED;

    if (stbutptr->item.stat&ITST_DISABLED)
     {
      wattrset (wptr, COLOR_PAIR(CLR_MENUBOXINV));
     }

    if (stbutptr->item.stat&ITST_SELECTED)
     {
      _wattron (wptr, A_REVERSE);
     }

   Fwprintf_nout (wptr, "%s", stbutptr->item.name);

   wptr->_line[stbutptr->item.obgeom.starty].text[stbutptr->item.obgeom.startx+stbutptr->item.underline]|=A_BOLD;

    if (stbutptr->item.stat&ITST_DISABLED)
     {
      wattrset (wptr, COLOR_PAIR(CLR_MENUBOX));
     }

    if (stbutptr->item.stat&ITST_SELECTED)
     {
      _wattroff (wptr, A_REVERSE);
     }

 }  /**/


  static int
 CommandButtonInterface (ObjectContainer *objcontptr,
                         int code, void *junk)

 {
  register CommandButton *cmdbutptr;

   switch (code)
    {
     case IFCMD_INVOKED:
          {
           cmdbutptr=objcontptr->cookie;

            if (!cmdbutptr->action)
             {
              cmdbutptr->item.stat|=ITST_DISABLED;

              return IFRET_IGNORE;
             }

           //exec action

           return IFRET_DONE;
          }
    }

   return IFRET_IGNORE;

 }  /**/


  ObjectContainer *
 CreateCommandButton (ControlArea *ctlptr, const char *name, int uline,
                      int startx, int starty)

 {
  ObjectContainer *objcontptr;
  CommandButton *cmdbutptr;

   xmalloc(objcontptr, (sizeof(ObjectContainer)));
   memset (objcontptr, 0, sizeof(ObjectContainer));

   xmalloc(cmdbutptr, (sizeof(CommandButton)));
   memset (cmdbutptr, 0, sizeof(CommandButton));

   objcontptr->type_name="CommandButton";

   cmdbutptr->item.name=strdup (name);
   cmdbutptr->item.underline=uline;
   cmdbutptr->item.obgeom.startx=startx;
   cmdbutptr->item.obgeom.starty=starty;
   cmdbutptr->item.obgeom.width=strlen (name);
   cmdbutptr->item.obgeom.hight=1;
   cmdbutptr->item.draw_func=(void (*)(void *, WINDOW *))DrawCommandButton;

   cmdbutptr->item.attrs|=ITATR_SELECTABLE;

   objcontptr->item=&cmdbutptr->item;  /* shared b/w slave and master */
   objcontptr->cookie=cmdbutptr;       /* establish link b/w slave and master */

   objcontptr->parent=ctlptr;

   objcontptr->if_func=CommandButtonInterface;

   AddObjectContainer (ctlptr, objcontptr);

   return (ObjectContainer *)objcontptr;

 }  /**/


 /*                 1     2          3  4  5     */
 /* handle(parent), name, underline, x, y, action */
 int STD_SCRIPT_COMMAND(intrin_addcommandbutton)

 {
  ObjectContainer *objcontptr;
  extern void *key;

   objcontptr=CreateCommandButton ((ControlArea *)arg0,
                                   (char *)arg1, (int)arg2,
                                   (int)arg3, (int)arg4);

   return(fpl_return_int((int)objcontptr));

 }  /**/

