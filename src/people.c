/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
*/

#include <main.h>
#include <terminal.h>
#include <updates.h>
#include <sockets.h>
#include <people.h>
#include <telnet.h>

 MODULEID("$Id: people.c,v 1.7 1998/04/06 06:13:44 ayman Beta $");

 static People PeopleMaster;
 People *const people=&PeopleMaster;
 Someone *ipc_ptr;


 void InitPeople (void)
 
 {
   PeopleMaster.nEntries=0;
   PeopleMaster.head=NULL;
   PeopleMaster.tail=NULL;

 }  /**/

 
 Someone *AddtoPeople (const char *name)

 {
  register Someone *pptr;
   
   xmalloc(pptr, (sizeof(Someone)));
   memset (pptr, 0, sizeof(Someone));

   mstrncpy (pptr->name, name, MAXNICKLEN);

    if (!None(people))
       {
        people->tail->next=pptr;
        people->tail=pptr;
       }
    else
       {
        people->head=people->tail=pptr;
       }

   people->nEntries++;

   return (Someone *)pptr;

 }  /**/


 Index *AddtoSomeoneIndex (Someone *ptr)

 {
  register Index *iptr;

   xmalloc(iptr, (sizeof(Index)));
   memset (iptr, 0, sizeof(Index));

   (Someone *)iptr->sindex=(Someone *)ptr;

    if (!IndexEmpty(ptr))
       {
        ptr->tail->next=iptr;
        ptr->tail=iptr;
       }
    else
       {
        ptr->head=ptr->tail=iptr;
       }

   ptr->nEntries++;

   return (Index *)iptr;

 }  /**/


 int RemovefromSomeoneIndex (Someone *ptr, int whatever)

 {
  int found=0;
  register Index *iptr,
                 *prev=NULL;

   iptr=ptr->head;

    while ((iptr!=NULL)&&(!found))
          {
            if (iptr->whatever==whatever)
               {
                found=1;
               }
            else
               {
                prev=iptr;
                iptr=iptr->next;
               }
          }
    if (!found)
       {
        return 0;
       }

    if (prev==NULL)
       {
        ptr->head=iptr->next;
       }
    else
    if (iptr->next==NULL)
       {
        ptr->tail=prev;
        ptr->tail->next=NULL;
       }
    else
       {
        prev->next=iptr->next;
       }

   ptr->nEntries--;
   memset (iptr, 0, sizeof(Index));
   free (iptr);

    if (!ptr->nEntries)
       {
        RemovefromPeople (ptr->name);
       }      
       
   return 1;

 }  /**/


 int RemoveIndexbyptr (Someone *ptr, Index *idxptr)

 {
  int found=0;
  register Index *iptr,
                 *prev=(Index *)NULL;

   iptr=ptr->head;

    while ((iptr!=NULL)&&(!found))
          {
            if (iptr==idxptr)
               {
                found=1;
               }
            else
               {
                prev=iptr;
                iptr=iptr->next;
               }
          }

    if (!found)
       {
        say ("%$% index not found, user: %s\n", HOT, ptr->name);

        return 0;
       }

    if (prev==NULL)
       {
        ptr->head=iptr->next;
       }
    else
    if (iptr->next==NULL)
       {
        ptr->tail=prev;
        ptr->tail->next=NULL;
       }
    else
       {
        prev->next=iptr->next;
       }

   ptr->nEntries--;

    if (iptr->pindex!=NULL)
       {
        FinishoffIndex (iptr);
       }

   memset (iptr, 0, sizeof(Index));
   free (iptr);

    if (!ptr->nEntries)
       {
        say ("%$% No longer maintaining record for %s.\n", INFO, ptr->name);

        RemovefromPeople (ptr->name);
       }

   return 1;

 }  /**/


 Index *GetSomeoneIndex (Someone *ptr, int whatever)

 {
  int found=0;
  register Index *iptr=ptr->head;

    while ((iptr!=NULL)&&(!found))
          {
            if (iptr->whatever==whatever)
               {
                found=1;
               }
            else
               {
                iptr=iptr->next;
               }
          }
    if (!found)
       {
        return (Index *)NULL;
       }

   return (Index *)iptr;

 }  /**/


 #include "thread.h"
 #include "exec.h"

 void FinishoffIndex (Index *iptr)

 {
   switch (iptr->protocol)
          {
           case PROTOCOL_DCC:
                {
                 register DCCParams *dcc;

                  dcc=INDEXTOPROTOCOL(iptr);

                   if (dcc->file!=(char *)NULL)
                      {
                       free (dcc->file);
                      }

                   if (dcc->fd>0)
                      {
                       close (dcc->fd);
                      }
              
                  RemoveDCCEntry (dcc); 
                  memset (dcc, 0, sizeof(DCCParams));
                  free (dcc);

                  break;
                }
           
           case PROTOCOL_TELNET:
                {
                 register Telnet *tptr;

                  (Telnet *)tptr=INDEXTOPROTOCOL(iptr);
                  
                  memset (tptr, 0, sizeof(Telnet));
                  free (tptr);

                  break;
                }

           case PROTOCOL_IPC:
                {
                 register struct thread_params *thptr;

                  thptr=(struct thread_params *)iptr->pindex;

                  free (((EXEC *)thptr->params)->shell);
                  free (((EXEC *)thptr->params)->shell_args);
                  free (((EXEC *)thptr->params)->prog);
                  free (thptr->params);
                  free (thptr->stack);
                  free (thptr->lpath);
                  free (thptr);

                  break;
                }
          }

 }  /**/

#define IS_DCC() iptr->protocol==PROTOCOL_DCC

 int ResetDCCPeople (int how, ...)

 {
  int n=0;
  register Someone *pptr,
                   *p_next=NULL;
  register Index *iptr,
                 *i_next=NULL;

   switch (how)
          {
           case EVERYTHING:
                {
                 pptr=people->head;

                  while (pptr!=NULL)
                        {
                         p_next=pptr->next;

                         iptr=pptr->head;

                          while (iptr!=NULL)
                                {
                                 i_next=iptr->next;

                                  if (IS_DCC())
                                     {
                                      FinishoffIndex (iptr);
                                      free (iptr);
                                      pptr->nEntries--;
                                     } 

                                 iptr=i_next;
                                }

                          if (NOMORE_INDICES(pptr))
                             {
                              RemovefromPeople (pptr->name);
                              //free (pptr);
                              //people->nEntries--;
                             }

                         pptr=p_next; 
                        }    

                  if (people->nEntries==0)
                     {
                      people->head=people->tail=(Someone *)NULL;
                     }       
                }
                break;
            
           case NAME:
                {
                 int what;
                 va_list va;

                  va_start (va, how);
                  what=va_arg (va, int);
                  pptr=va_arg (va, Someone *);

                   if (pptr->nEntries==0)
                      {
                       return 0;
                      }

                  iptr=pptr->head;

                  switch (what)
                         {
                          case EVERYTHING:
                               {
                                 while (iptr!=NULL)
                                       {
                                        i_next=iptr->next;

                                         if (IS_DCC())
                                            {
                                             FinishoffIndex (iptr);
                                             free (iptr);
                                             pptr->nEntries--;
                                            }

                                        iptr=i_next;
                                       }

                                 if (NOMORE_INDICES(pptr))
                                    {
                                     RemovefromPeople (pptr->name);
                                    }
                               }
                               break;

                          default:
                               {
                                DCCParams *dcc;

                                 while (iptr!=NULL)
                                       {
                                        i_next=iptr->next;

                                         if (IS_DCC())
                                            {
                                             dcc=INDEXTODCC(iptr);

                                              if (dcc->type&what)
                                                 {
                                                  FinishoffIndex (iptr);
                                                  free (iptr);
                                                  pptr->nEntries--;
                                                 }
                                            }

                                        iptr=i_next;
                                       }

                                 if (pptr->nEntries==0)
                                    {
                                     free (pptr);
                                     people->nEntries--;
                                    }
                               }
                               break;
                         }

                }
                break;

          }

 }  /**/


 int RemovefromPeople (/*People *ptr,*/ char *name)

 {
  int found=0;
  register Someone *pptr,
                   *prev=NULL;

   pptr=people->head;

    while ((pptr!=NULL)&&(!found))
          {
            if (!strcasecmp(pptr->name, name))
               {
                found=1;
               }
            else
               {
                prev=pptr;
                pptr=pptr->next;
               }
          }

    if (!found)
       {
        return 0;
       }

    if (pptr->nEntries<1)
       {
         if (prev==NULL)
            {
             people->head=pptr->next;
            }
         else
         if (pptr->next==NULL)
            {
             people->tail=prev;
             people->tail->next=NULL;
            }
         else
            {
             prev->next=pptr->next;
            }

        people->nEntries--;
        memset (pptr, 0, sizeof(Someone));
        free (pptr);

         if (people->nEntries==0)
            {
             people->head=people->tail=(Someone *)NULL;
            }

        return 1;
       }

 }  /**/


 Someone *FindSomeone (char *name, int *what, void *extra)

 {
  int found=0;
  register Someone *pptr=people->head;

    while ((pptr!=NULL)&&(!found))
          {
            if (!strcasecmp(pptr->name, name))
               {
                found=1;
               }
            else
               {
                pptr=pptr->next;
               }
          }
    if (!found)
       {
        *what=0;
        return (Someone *)NULL;
       }
    
    if (!pptr->nEntries)
       {
        *what=0;
        return (Someone *)pptr;
       }
    else
       {
        int i=0;
        register Index *iptr=pptr->head;

          while (iptr!=NULL)
                {
                  if (iptr->what==*what) 
                     {
                      switch (*what)
                             {
                              case DCCCHATTOUS:
                              case DCCCHATTOHIM:
                                   {
                                     if (iptr->flag&CONNECTED)
                                        {
                                         *what=1;
                                         return (Someone *)pptr;
                                        }
                                     else
                                        {
                                         *what=0;
                                         return (Someone *)pptr;
                                        }
                                   }
                              case DCCSENDING:
                              case DCCGETTING:
                                   break;
                             }
                     }
                 iptr=iptr->next;
                } 
       }

 }  /**/
 

 Someone *SomeonebyName (const char *name)

 {
  register Someone *pptr=people->head;

    if (people->nEntries==0)
     {
      return (Someone *)NULL;
     }

    while (pptr!=NULL)
     {
      if (arethesame(pptr->name, name))
       {
        return (Someone *)pptr;
       }
      else
       {
        pptr=pptr->next;
       }
     }

   return (Someone *)NULL;

 }  /**/


 int isvalidname (char *name)

 {
  char p[strlen(name)+1];
  int i;

   strcpy (p, name);

    if (*p==' ')
     {
      return 0;
     }

    for (i=0 ; i<=strlen(p)-1; i++)
     {
      switch (*(p+i))
       {
        case 'a' ... 'z':
        case 'A' ... 'Z':
        case '0' ... '9':
        case ' ':
              continue;

        default:
              return 0;
       }
     }

   return 1;

 }  /**/


 void ShowPeople (void)

 {
  char s[]={[0 ... 100]=0};
  char p[]={[0 ... 100]=0};
  int q,
      i=0;
  register Someone *ptr=people->head;
  register Index *iptr;
  DCCParams *dcc;

    if (None(people))
     {
      say ("%$% You are on your own.\n", INFO);

      return;
     }
   
   say ("%$% Reporting on People: %d entr%s.\n", INFO, people->nEntries,
        (people->nEntries==1)?"y":"ies");

   q=DisplayHeader (9, 5);

    for ( ; ptr!=NULL; ptr=ptr->next)
        {
         i++;
          if (ptr->nEntries>=1)
             {
              iptr=ptr->head;

                for ( ; iptr!=NULL; iptr=iptr->next)
                    {
                      if (iptr->protocol==PROTOCOL_DCC)
                         {
                          (DCCParams *)dcc=INDEXTODCC(iptr);

                           if (dcc->type&(DCCCHATTOUS|DCCCHATTOHIM))
                              {
                               strcpy (&s[strlen(s)], "CHAT");
                               s[strlen(s)]='/';
                              }
                           else
                           if (dcc->type&DCCSENDING)
                              {
                               strcpy (&s[strlen(s)], "GET");
                               s[strlen(s)]='/';
                              } 
                           else
                           if (dcc->type&DCCGETTING)
                              {
                               strcpy (&s[strlen(s)], "SEND");
                               s[strlen(s)]='/';
                              }

                           if (dcc->flags&AWAITINGCONNECTION)
                              {
                               strcpy (&p[strlen(p)], "W");
                               p[strlen(p)]='/';
                              }
                           else
                           if (dcc->flags&CONNECTED)
                              {
                               strcpy (&p[strlen(p)], "A");
                               p[strlen(p)]='/';
                              }
                         }
                    }

              s[strlen(s)]=0;
              p[strlen(p)]=0;
             }

         say (" %$179%-.3d%$179%-13.13s%$179%-.4d%$179%-15.15s%$179%-15.15s%$179\n", i, ptr->name, ptr->nEntries, s, p);

         memset (p, 0, 100);
         memset (s, 0, 100);
         
          if (!ptr->next)
             {
              finish2(9, 5);
             }
          else
             {
              seperator(9, 5);
             }
        }

 }  /**/


 boolean None (const People *ptr)

 {
   return ((boolean)(ptr->nEntries==0));

 }  /**/


 boolean IndexEmpty (const Someone *ptr)

 {
   return ((boolean)(ptr->nEntries==0));

 }  /**/
 
