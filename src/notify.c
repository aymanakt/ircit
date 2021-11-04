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
#include <servers.h>
#include <serverio.h>
#include <updates.h>
#include <history.h>
#include <notify.h>

#include <sys/stat.h>
#include <utmp.h>
#include <fcntl.h>

 MODULEID("$Id: notify.c,v 1.6 1998/04/06 06:13:44 ayman Beta $");

 List *irc_notify;
 List *utmp_notify;
 List *inet_notify;
 

 void InitIRCNotify (void)

 {
  static List NotifyList;

   NotifyList.head=NULL;
   NotifyList.tail=NULL;
   NotifyList.nEntries=0;
   irc_notify=&NotifyList;

 }  /**/


 void InitUTMPNotify (void)

 {
  static List UTMPNotifyList;

   UTMPNotifyList.head=NULL;
   UTMPNotifyList.tail=NULL;
   UTMPNotifyList.nEntries=0;
   utmp_notify=&UTMPNotifyList;

 }  /**/


 int inNotifyList (const List *ptr, const char *name)

 {
  register const ListEntry *eptr=ptr->head;

    while (eptr!=NULL)
     {
       if (arethesame(name, ((Notify *)eptr->whatever)->name))
        {
         return 1;
        }
       else
        {
         eptr=eptr->next;
        }
     }

   return 0;

 }  /**/


 static __inline__ int _AddtoNotifyList (List *ptr, Notify *nptr)

 {
  register ListEntry *aux;

   aux=AddtoList (ptr);

    if (aux==(ListEntry *)NULL)
     {
      return 0;
     }

   (Notify *)aux->whatever=(Notify *)nptr;
   (ListEntry *)nptr->link=(ListEntry *)aux;

   return ptr->nEntries;

 }  /**/


 void AddtoNotify (List *ptr, char *str, int yell_when_error)

 {
  int i,
      j;
  int found=0;
  register ListEntry *eptr;
  register Notify *nptr;
  size_t sizeofNotify=sizeof(Notify);
  extern char tok[][80];

   if (str)
    {
     i=splitargs2 (str, ',');

      for (j=0; j<=i; j++)
       {
         if (strlen(tok[j])>MAXNICKLEN)
          {
            if (yell_when_error)
             {
              say ("%$% Name too long - %s.\n", MILD, tok[j]);
             }

           continue;
          }

        eptr=ptr->head;

         while ((eptr!=NULL)&&(!found))
          {
           (Notify *)nptr=(Notify *)eptr->whatever;

            if (arethesame(tok[j], nptr->name))
             {
              found=1;
             }
            else
             {
              eptr=eptr->next;
             }
          }

         if (found)
          {
            if (yell_when_error)
             {
              say ("%$% Dublicate entry - %s.\n", MILD, tok[j]);
             }

           found=0;
           continue;
          }

        xmalloc(nptr, sizeofNotify);
        memset (nptr, 0, sizeofNotify);

        strcpy (nptr->name, tok[j]);

        _AddtoNotifyList (ptr, nptr);
       }
    }

 }  /**/


 #define nptr ((Notify *)eptr->whatever)

 int ClearNotifyList (List *ptr, int how, void *v)

 {
    if (ptr->nEntries==0)
     {
      say ("%$% No entries found in Notify list.\n", MILD);

      return;
     }

   switch (how)
          {
           case EVERYTHING:
                {
                 int i=0;
                 register ListEntry *eptr=ptr->head;
                          ListEntry *aux;

                   while (eptr!=NULL)
                         {
                          i++;
                          aux=eptr->next;
                          free (nptr);
                          free (eptr);
                          eptr=aux;
                         }

                  ptr->nEntries=0;
                  ptr->head=ptr->tail=NULL;

                  return i;
                }

           case NAME:
                {
                 char *name;
                 register ListEntry *eptr=ptr->head;

                  name=(char *)v;

                   for ( ; eptr!=NULL; eptr=eptr->next)
                       {
                         if (arethesame(name, nptr->name))
                            {
                             memset (nptr, 0, sizeof(Notify));
                             free (nptr);
                             RemovefromList (ptr, eptr);

                             return 1;
                            }
                       }

                 say ("%$% %s isn't in Notify list.\n", MILD, name);

                 return 0;
                }

           case NUMBER:
                {
                 int *m=0,
                     n=0;
                 register ListEntry *eptr=ptr->head;

                  m=(int *)v;

                   if (*m>ptr->nEntries)
                      {
                       say ("%$% You don't have that many entries.\n", MILD);

                       return 0;
                      }

                   for ( ; eptr!=NULL; eptr=eptr->next)
                       {
                         if (++n==*m)
                            {
                             memset (nptr, 0, sizeof(Notify));
                             free (nptr);
                             RemovefromList (ptr, eptr);

                             return 1;
                            }
                       }

                 return 0;
                }
          }

 }  /**/


 void RemovefromNotify (List *ptr, char *str)

 {
  int i,
      j;
  int esc=0;
  register char *aux;
  extern char tok[][80];

   if (str)
    {
     i=splitargs2 (str, ',');

      for (j=0; j<=i; j++)
       {
         if (!*tok[j])  continue;

        {
         if (*tok[j]=='\\')
          {
           if ((esc=ProcessEscaped(tok[j]+1)))
            {
             aux=tok[j]+1;
            }
           else
            {
             aux=tok[j];
            }
          }
         else
          {
           aux=tok[j];
          }
        }

         if ((!esc)&&(arethesame(aux, "*")))
          {
           ClearNotifyList (ptr, EVERYTHING, NULL);

           return;
          }
         else
         if ((!esc)&&(*aux=='#'))
          {
           int i=0;

             if (!(i=atoi(aux+1))||(i<0))
              {
               say ("%$% You really need -h.\n", MILD);

               continue;
              }

            ClearNotifyList (ptr, NUMBER, &i);
          }
         else
          {
           ClearNotifyList (ptr, NAME, aux);
          }
       }
    }

 }  /**/


 void ResetIRCNotify (void)

 {
  register ListEntry *eptr=irc_notify->head;

    if (irc_notify->nEntries>0)
     {
      for ( ; eptr!=NULL; eptr=eptr->next)
       {
        nptr->flags=0;
        nptr->when=0L;
       }
     }

   UpdateHistoryTable (H_NOTIFY, 0, 0);

 }  /**/

 #undef nptr


 void ShowNotify (List *ptr, char *how)

 {
  int q,
      i=0;
  char t[10];
  register ListEntry *lptr=ptr->head;
  #define nptr ((Notify *)(lptr->whatever))

    if (ptr->nEntries==0)
     {
      say ("%$% Notify list empty, add some...\n", MILD);

      return;
     }

   say ("%$% You have the following %d entr%sin your Notify list:\n", INFO,
        ptr->nEntries, ptr->nEntries>1?"ies ":"y ");

   q=DisplayHeader (12, 4);

    for ( ; lptr!=NULL; lptr=lptr->next)
     {
      i++;
       if (nptr->when)
        {
         strftime (t, 10, "%H:%M", localtime(&nptr->when));
        }
       else
        {
         strcpy (t, "--:--");
        }

      say (" %$179%-2d%$179%-15.15s%$179%-3s %$% %$179%-5s%$179\n",
           i, nptr->name, (nptr->flags&1)?"ON":"OFF",
           (nptr->flags&1)?(17|COLOR_PAIR(CYAN)|A_BLINK):32, t);
     }

   finish2 (12, 4);

 }  /**/

 #undef nptr


 void ircNotify (void)

 {
    if ((irc_notify->nEntries==0)||(!isup()))
     {
      return;
     }
    else
     {
      char s[((irc_notify->nEntries*MAXNICKLEN)+irc_notify->nEntries)];
      register ListEntry *lptr=irc_notify->head;
      register Notify *nptr;

       memset (s, 0,((irc_notify->nEntries*MAXNICKLEN)+irc_notify->nEntries));

        for ( ; lptr!=NULL; lptr=lptr->next)
         {
          (Notify *)nptr=(Notify *)lptr->whatever;
          strcpy (&s[strlen(s)], nptr->name);
          s[strlen(s)]=' ';
         }

       ToServer ("ISON %s\n", s);
     }

 }  /**/


 __inline__ void utmpCheck (void)

 {
  int i=0;
  register ListEntry *eptr;
  register Notify *tptr;
  struct utmp *ut;

   eptr=utmp_notify->head;

   setutent ();

    while ((ut=getutent())!=NULL)
     {
       for ( ; eptr!=NULL; eptr=eptr->next)
        {
         (Notify *)tptr=(Notify *)eptr->whatever;

          if ((arethesame(ut->ut_name, tptr->name))&&(tptr->xtra==0))
           {
            i++;
             if (!(tptr->flags&1))
              {
               tptr->flags|=(1|2);
               tptr->xtra=1;
               tptr->flags&=~4;
              }
             else
              {
               tptr->flags|=4;
              }
           }
        }

      eptr=utmp_notify->head;
     }

   endutent ();

   eptr=utmp_notify->head;

    for ( ; eptr!=NULL; eptr=eptr->next)
     {
      (Notify *)tptr=(Notify *)eptr->whatever;

       if ((tptr->flags&1)&&(tptr->flags&2)&&!(tptr->flags&4))
        {
         say ("%$% LOCALHOST %s logged on.\n", UTMPH, tptr->name);
         tptr->when=time (NULL);

         tptr->flags&=~2;

         UpdateHistoryTable (H_UTMP, 0, '+');
        }
       else
       if ((tptr->flags&1)&&!(tptr->flags&2)&&!(tptr->flags&4))
        {
         say ("%$% LOCALHOST %s logged out.\n", UTMPH, tptr->name);
              tptr->when=time (NULL);

         tptr->flags&=~1;

         UpdateHistoryTable (H_UTMP, 0, '-');
        }

      tptr->flags&=~4;
      tptr->xtra=0;
     }

 }  /**/


 int utmpNotify (void)

 {
  struct stat st;

    if (stat(UTMP_FILE, &st)<0)
     {
      say ("%$% Unable to open utmp: %s.\n", HOT, strerror(errno));

      return 0;
     }

    if ((st.st_size!=0)&&
        (mt_ptr->utmp_mt!=st.st_mtime||mt_ptr->utmp_sz!=st.st_size))
     {
      utmpCheck ();

      mt_ptr->utmp_mt=st.st_mtime;
      mt_ptr->utmp_sz=st.st_size;
     }

 }  /**/


 void ShowUTMPNotify (void)

 {
  int i=0;
  char t[10];
  register ListEntry *eptr;
  #define tptr ((Notify *)(eptr->whatever))

    if (NO_ENTRIES(utmp_notify))
     {
      say ("%$% No utmp entries.\n", MILD);

      return;
     }

   DisplayHeader (12, 4);

   eptr=utmp_notify->head;

    for ( ; eptr!=NULL; eptr=eptr->next)
     {
      i++;
       if (tptr->when)
        {
         strftime (t, 10, "%H:%M", localtime(&tptr->when));
        }
       else
        {
         strcpy (t, "--:--");
        }

      say (" %$179%-2d%$179%-15.15s%$179%-3s %$% %$179%-5s%$179\n",
           i, tptr->name, (tptr->flags&1)?"ON":"OFF",
           (tptr->flags&1)?(17|COLOR_PAIR(CYAN)|A_BLINK):32, t);
     }

   finish2 (12, 4);

 }  /**/

 #undef tptr

 STD_IRCIT_COMMAND(uNOTIFY)

 {
  int add,
      mode=0;
  register int j=0;
  extern char tok[][80];

    if (!args)
     {
      ShowNotify (irc_notify, NULL);

      return;
     }

   mode|=1;

   splitargs2 (args, ' ');

    if (arethesame(tok[j], "-IRC"))
     {
      mode|=1;
      j++;

       if (!*tok[j])
        {
         ShowNotify (irc_notify, NULL);

         return;
        }
     }
    else
    if (arethesame(tok[j], "-UTMP"))
     {
      mode&=~1; mode|=2;
      j++;

       if (!*tok[j])
        {
         ShowUTMPNotify ();

         return;
        }
     }
    else
    if (arethesame(tok[j], "-INET"))
     {
      mode&=~1; mode|=4;
      j++;

       if (!*tok[j])
        {
         say ("%$% This option is not implemented yet.\n");

         return;
        }
     }
 
    if (*tok[j]=='-')  add=0;
    else 
    if (*tok[j]=='+')  add=1;
    else
     {
      say ("%$% You really need -h.\n", MILD);

      return;
     }

  {
   char s[strlen(tok[j])];

    strcpy (s, tok[j]+1); 

    switch (mode&(1|2))
     {
      case 1:
           {
             if (add)
              {
               AddtoNotify (irc_notify, s, 1);
               UpdateHistoryTable (H_NOTIFY, ht_ptr->nNotify,0); //trick it
              }
             else
              {
               RemovefromNotify (irc_notify, s);
               UpdateHistoryTable (H_NOTIFY, ht_ptr->nNotify, 0);
              }

            break;
           }

      case 2:
           {
             if (add)
              {
               AddtoNotify (utmp_notify, s, 1);
               UpdateHistoryTable (H_UTMP, ht_ptr->nUTMP, 0);//trick it
              }
             else 
              {
               RemovefromNotify (utmp_notify, s);
               UpdateHistoryTable (H_UTMP, ht_ptr->nUTMP, 0);
              }
    
            break;
           }
     }
  }

 }  /**/

