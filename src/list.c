/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
*/

#include <main.h>
#include <terminal.h>
#include <list.h>

#include <signal.h>

 MODULEID("$Id: list.c,v 1.5 1998/04/06 06:13:44 ayman Beta $");


 ListEntry *AddtoList (List *ptr)

 {
  const size_t sizeofListEntry=sizeof(ListEntry);
  register ListEntry *lptr;

   xmalloc(lptr, sizeofListEntry);
   memset (lptr, 0, sizeofListEntry);

    if (!ListEmpty(ptr))
     {
      ptr->tail->next=lptr;
      ptr->tail=lptr;
     }
    else
     {
      ptr->head=ptr->tail=lptr;
     }

   ptr->nEntries++;

   return (ListEntry *)lptr;

 }  /**/


 int RemovefromList (List *ptr, ListEntry *eptr)

 {
  int found=0;
  register ListEntry *lptr,
                     *prev=NULL;

   lptr=ptr->head;

    while ((lptr!=NULL)&&(!found))
     {
      if (lptr==eptr)
       {
        found=1;
       }
      else
       {
        prev=lptr;
        lptr=lptr->next;
       }
     }

    if (!found)  return 0;

    if (prev==NULL)
     {
      ptr->head=lptr->next;
     }
    else
    if (lptr->next==NULL)
     {
      ptr->tail=prev;
      ptr->tail->next=NULL;
     }
    else
     {
      prev->next=lptr->next;
     }

   ptr->nEntries--;

    if (((ptr->glock>>0)&0x1)&&((ptr->glock>>1)&0x1))
     {
      if (ptr->nEntries==0)
       {
        ptr->head=ptr->tail=NULL;
        DestroyWindow (4, ptr);
       }
      else
       {
        RefreshListContents (ptr);
       }
     }

   memset (lptr, 0, sizeof(ListEntry));
   free (lptr);

   return 1;

 }  /**/


 int RemoveListHead (List *lptr, int n, void (*free_func)(ListEntry *))

 {
  ListEntry *eptr;

    if (n>lptr->nEntries)  n=lptr->nEntries;

    while (n--)
     {
      eptr=lptr->head;
      lptr->head=lptr->head->next;

       if (free_func)
        {
         (*free_func)(eptr);
        }

      free (eptr);

      lptr->nEntries--;
     }

 }  /**/


  int 
 RemoveListEntry (List *lptr, ListEntry *eptr, int how, void (*f)(void *), ...)

 {
   switch (how)
          {
           case EVERYTHING:
                {
                 int i=0;
                 register ListEntry *eptr=lptr->head;
                          ListEntry *aux;

                   while (eptr!=NULL)
                         {
                          ++i;
                          aux=eptr->next;
                           if (eptr->whatever)
                              {
                               if (f)
                                  {  
                                   (*f) (eptr->whatever);
                                  }
                               else
                                  {
                                   free (eptr->whatever);
                                  }
                              } 

                          free (eptr);
                          eptr=aux;
                         }

                  lptr->nEntries=0;
                  lptr->head=lptr->tail=NULL;

                  return i;
                }

           case NAME:
                {
                 register ListEntry *eptr;

                   if (eptr->whatever)
                      {
                       if (f)
                          {
                           (*f) (eptr->whatever);
                          }
                       else
                          {
                           free (eptr->whatever);
                          }
                      }

                  RemovefromList (lptr, eptr);

                  return 1;
                }

           case NUMBER:
                {
                 int *m=0,
                      n=0;
                 register ListEntry *eptr=lptr->head;
                  //m=(int *)v;

                   if (*m>lptr->nEntries)
                      {
                       say ("%$% You don't have that many entries.\n", MILD);

                       return 0;
                      }

                   for ( ; eptr!=NULL; eptr=eptr->next)
                       {
                         if (++n==*m)
                            {

                             return 1;
                            }
                       }

                 return 0;
                }
          }

 }  /**/


 List *ListfromArray (void *a, size_t sz, size_t n)

 {
  const size_t sizeofList=sizeof(List);
  register int i=0, j=0;
  List *ptr;
  ListEntry *lptr;

    if (!a)
       {
        return;
       }

   xmalloc(ptr, sizeofList);
   memset (ptr, 0, sizeofList);

    for ( ; i<=n-1; i++)
        {
         lptr=AddtoList (ptr);

         lptr->whatever=(a+j);
         j+=sz;
        }

   return (List *)ptr;

 }  /**/


 void CleanupListfromArray (List *ptr)

 {
  register ListEntry *eptr,
                     *aux;

    if (!ptr)
       {
        return;
       }

   eptr=ptr->head;
 
    while (eptr!=NULL)
          {
           aux=eptr->next;
           RemovefromList (ptr, eptr);
           eptr=aux;
          }

   memset (ptr, 0, sizeof(List));
   free (ptr); 

 }  /**/


 ListEntry *LocateEntry (List *ptr, ListEntry *eptr)

 {
  int found=0;
  register ListEntry *aux;

   aux=ptr->head;

    while ((aux!=NULL)&&!(found))
          {
            if (aux==eptr)
               {
                found=1;
               }
            else
               {
                aux=aux->next;
               }
          }

    if (!found)
       {
        return (ListEntry *)NULL;
       }

   return (ListEntry *)aux;

 }  /**/  
  

 boolean ListEmpty (const List *ptr)

 {
   return ((boolean)(ptr->nEntries==0));

 }  /**/


 dListEntry *AddtodList (dList *ptr)

 {
  register dListEntry *aux;

   xmalloc(aux, (sizeof(dListEntry)));
   memset (aux, 0, sizeof(dListEntry));

    if (!dListEmpty(ptr))
       {
        aux->prev=ptr->tail;
        ptr->tail->next=aux;
        ptr->tail=aux;
       }
    else
       {
        ptr->head=ptr->tail=aux;
        aux->prev=NULL;
       }

   ptr->nEntries++;

   return (dListEntry *)aux;

 }  /**/


 dListEntry *RemovefromdList (dList *ptr, dListEntry *eptr)

 {
  register dListEntry *next,
                      *prev;

   next=eptr->next;
   prev=eptr->prev;

    if (ptr->head==eptr)  ptr->head=next;
    if (ptr->tail==eptr)  ptr->tail=prev;

    if (next!=NULL)  next->prev=prev;
    if (prev!=NULL)  prev->next=next;

   free (eptr);
   ptr->nEntries--;

   return (next);

 }  /**/


 boolean dListEmpty (const dList *ptr)

 {
   return ((boolean)(ptr->nEntries==0));

 }  /**/

