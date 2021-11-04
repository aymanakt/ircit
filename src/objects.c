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
#include <stack.h>
#include <windows-aux.h>
#include <colors.h>
#include <FPL.h>
#include <FPL_protos.h>
#include <objects.h>

 MODULEID("$Id:$");


  ObjectContainer *
 AddObjectContainer (ControlArea *ctlptr, ObjectContainer *objcontptr)

 {
    if (ctlptr->nEntries)
     {
      objcontptr->prev=ctlptr->tail;
      ctlptr->tail->next=objcontptr;
      ctlptr->tail=objcontptr;
     }
    else
     {
      ctlptr->head=ctlptr->tail=objcontptr;
      objcontptr->prev=NULL;
     }

   ctlptr->nEntries++;

   {
    if (objcontptr->item->obgeom.hight>ctlptr->obgeom.hight)
     {
      objcontptr->item->obgeom.hight=ctlptr->obgeom.hight;
     }

    if (objcontptr->item->obgeom.width>ctlptr->obgeom.width)
     {
      objcontptr->item->obgeom.width=ctlptr->obgeom.width;
     }

    if (objcontptr->item->obgeom.startx>ctlptr->obgeom.width)
     {
      objcontptr->item->obgeom.startx=0;
     }

    if (objcontptr->item->obgeom.starty>ctlptr->obgeom.hight)
     {
      objcontptr->item->obgeom.starty=0;
     }
   }

   return (ObjectContainer *)objcontptr;

}  /**/


  ObjectContainer *
 RemoveObjectContainer (ControlArea *ctlptr, ObjectContainer *objcontptr)

 {
  register ObjectContainer *next,
                           *prev;

   next=objcontptr->next;
   prev=objcontptr->prev;

    if (ctlptr->head==objcontptr)  ctlptr->head=next;
    if (ctlptr->tail==objcontptr)  ctlptr->tail=prev;

    if (next!=NULL)  next->prev=prev;
    if (prev!=NULL)  prev->next=next;

   free (objcontptr);

   ctlptr->nEntries--;

   return (next);

 }  /**/


  ObjectContainer *
 SelectNearestNextEntry (ControlArea *ctlptr, ObjectContainer *objcontptr)

 {
  register ObjectContainer *auxptr=objcontptr;

    if (!ctlptr->nEntries)  return (ObjectContainer *)NULL;

    if (auxptr==NULL)  auxptr=ctlptr->head;

    if (auxptr->item->attrs&ITATR_SELECTABLE)
     {
      auxptr->item->stat|=ITST_SELECTED;

      return (ObjectContainer *)auxptr;
     }
    else
     {
      register ObjectContainer *auxptr2;

       (auxptr->next==NULL)?
        (auxptr2=ctlptr->head):
        (auxptr2=auxptr->next);

        while (auxptr2!=auxptr)
         {
           if (auxptr2->item->attrs&ITATR_SELECTABLE)
            {
             auxptr2->item->stat|=ITST_SELECTED;

             return (ObjectContainer *)auxptr2;
            }

            if (auxptr2->next==NULL)  auxptr2=ctlptr->head;
            else  auxptr2=auxptr2->next;
         }
     }

   return (ObjectContainer *)NULL;

 }  /**/


  ObjectContainer *
 SelectNearestPrevEntry (ControlArea *ctlptr, ObjectContainer *objcontptr)

 {
  register ObjectContainer *auxptr=objcontptr;

    if (!ctlptr->nEntries)  return (ObjectContainer *)NULL;

    if (auxptr==NULL)  auxptr=ctlptr->tail;

    if (auxptr->item->attrs&ITATR_SELECTABLE)
     {
      auxptr->item->stat|=ITST_SELECTED;

      return (ObjectContainer *)auxptr;
     }
    else
     {
      register ObjectContainer *auxptr2;

       (auxptr->prev==NULL)?
        (auxptr2=ctlptr->tail):
        (auxptr2=auxptr->prev);

        while (auxptr2!=auxptr)
         {
           if (auxptr2->item->attrs&ITATR_SELECTABLE)
            {
             auxptr2->item->stat|=ITST_SELECTED;

             return (ObjectContainer *)auxptr2;
            }

            if (auxptr2->prev==NULL)  auxptr2=ctlptr->tail;
            else  auxptr2=auxptr2->prev;
         }
     }

   return (ObjectContainer *)NULL;

 }  /**/


#include <fpl.h>
#include <menubox.h>
extern void *key;
 ControlArea *QueryControlArea (ObjectContainer *objcontptr)

 {
   return (& ((MenuButton *)objcontptr->cookie) -> entries);

 }  /**/


 int STD_SCRIPT_COMMAND(intrin_getresource)

 {
    return(fpl_return_int((int)QueryControlArea((ObjectContainer *)arg0)));

 }  /**/

