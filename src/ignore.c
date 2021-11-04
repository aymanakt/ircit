/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
*/

#include <main.h>
#include <terminal.h>
#include <list.h>
#include <updates.h>
#include <ignore.h>

 MODULEID("$Id: ignore.c,v 1.4 1998/04/06 06:13:44 ayman Beta $");

 List *ignore;

 void InitIgnoreList (void)

 {
  static List IgnoreList;

   IgnoreList.head=NULL;
   IgnoreList.tail=NULL;
   IgnoreList.nEntries=0;
   ignore=&IgnoreList;

 }  /**/


 static __inline__ int _AddtoIgnoreList (List *ptr, Ignore *gptr)

 {
  register ListEntry *aux;

   aux=AddtoList (ptr);

   (Ignore *)aux->whatever=(Ignore *)gptr;
   (ListEntry *)gptr->link=(ListEntry *)aux;

   return ptr->nEntries;

 }  /**/


 #define RESOLVE_FLAGS(x) \
         if (IS_IGINVITE(gptr)) \
            { \
             strcpy (&s[strlen(s)], "INVITE-"); \
            } \
              \
         if (IS_IGCTCP(gptr)) \
            { \
             strcpy (&s[strlen(s)], "CTCP-"); \
            } \
              \
         if (IS_IGMSG(gptr)) \
            { \
             strcpy (&s[strlen(s)], "MSG-"); \
            } \
              \
         if (IS_IGNOTICE(gptr)) \
            { \
             strcpy (&s[strlen(s)], "NOTICE-"); \
            } \
              \
         if (IS_IGTELNET(gptr)) \
            { \
             strcpy (&s[strlen(s)], "TELNET-"); \
            } \
              \
        s[strlen(s)-1]=0

 #define MODIFY_FLAGS() \
          if (FLAG_ALREADY_SET()) \
             { \
               if (yell_when_error) \
                  { \
                   say ("%$% Ignore mask already set for %s.\n", MILD,\
                         gptr->host); \
                  } \
             } \
          else \
             { \
              char s[]={[0 ... MINIBUF]=0}; \
                               \
               gptr->flags|=mode; \
               RESOLVE_FLAGS(gptr); \
               say ("%$% Ignore mask for %s now set to\n" \
                   "  %$192%$196%$16 %s\n", INFO, gptr->host, s); \
             }
       
 #define FLAG_ALREADY_SET() (gptr->flags&mode)

 int AddtoIgnore (List *ptr, char *str, int mode, int yell_when_error)

 {
  int i,
      j;
  int found=0, escaped=0;
  register char *aux;
  register ListEntry *eptr;
  register Ignore *gptr;
  extern char tok[][80];

   if (str)
      {
       i=splitargs2 (str, ',');

        for (j=0; j<=i; j++)
            {
              if (strlen(tok[j])>MAXHOSTLEN)
                 {
                   if (yell_when_error)
                      {
                       say ("%$% Entry too long - %s.\n", MILD, tok[j]);
                      }

                  continue;
                 }

              if (!*tok[j])  continue;

             {
              if (*tok[j]=='\\')
                 {
                   if ((escaped=ProcessEscaped(tok[j]+1)))
                      {
                       aux=tok[j]+1;
                      }
                   else
                      {
                       aux=tok[j];
                      }
                 }
              else
                 aux=tok[j];
             }

             eptr=ptr->head;

              if (arethesame(aux, "*")) /* could escaped or not */
                 {
                   if (!escaped)
                      {
                        if (NO_ENTRIES(ignore))
                           {
                            say ("%$% You really need -h.\n", MILD);

                            return;
                           }

                        for ( ; eptr!=NULL; eptr=eptr->next)/*apply change all*/
                            {
                             gptr=(Ignore *)eptr->whatever;

                             MODIFY_FLAGS();
                            }

                       continue;
                      }
                   else
                      {
                       goto plain_entry;  /* treat it as normal entry */
                      }
                 }
              else
              if (*aux=='#')
                 {
                  int m,
                      n=0;
                  char *p=aux+1;

                    if (escaped)
                       {
                        goto plain_entry;  /* treat as normal entry.. */
                       }

                    if ((!isdigits(p))||(!*p))
                       {
                        say ("%$% You really need -h.\n", MILD);
                        continue;
                       }

                   m=atoi(p);

                    if ((m>ignore->nEntries)||(m<=0))
                       {
                        say ("%$% Ignore entry %d undefined.\n", MILD, m);
                        continue;
                       }

                    while ((eptr!=NULL)&&(!found))
                          {
                            if (++n==m)
                               {
                                gptr=(Ignore *)eptr->whatever;

                                found=1;
                               }
                            else
                               {
                                eptr=eptr->next;
                               }
                          }
                 }
              else
                 {
                  plain_entry:
                   while ((eptr!=NULL)&&(!found))
                         {
                          (Ignore *)gptr=(Ignore *)eptr->whatever;

                           if (arethesame(aux, gptr->host))
                              {
                               found=1;
                              }
                           else
                              {
                               eptr=eptr->next;
                              }
                         }
                 }

              if (found)
                 {
                  MODIFY_FLAGS();

                  found=0;
                  continue;
                 }

             {
              char s[]={[0 ... MINIBUF]=0};	

               xmalloc(gptr, (sizeof(Ignore)));
               memset (gptr, 0, sizeof(Ignore));

               strcpy (gptr->host, aux);
               gptr->when=time (NULL);
               gptr->flags|=mode;

               RESOLVE_FLAGS(gptr);

               _AddtoIgnoreList (ignore, gptr); 

               say ("%$% Ignore mask for %s\n  %$192%$196%$16 %s\n", INFO,
                    gptr->host, s); 
             }
            }
      }

 }   /**/
 

#define COMMUNIQUE() \
 say ("%$% Ignore mask for %s is reset.\n  %$192%$196%$16 Ignored for" \
      " %s seconds.\n", \
       INFO, gptr->host, secstostr((time(NULL)-gptr->when), 1))

#define ACTUAL_REMOVAL() \
        if (gptr->flags&mode) \
           { \
            gptr->flags&=~mode; \
                                \
             if (!gptr->flags) \
                { \
                 COMMUNIQUE(); \
                               \
                 memset (gptr, 0, sizeof(Ignore)); \
                 free (gptr); \
                 RemovefromList (ptr, eptr); \
                } \
             else \
                { \
                 char s[]={[0 ... MINIBUF]=0}; \
                                  \
                  RESOLVE_FLAGS(gptr); \
                                       \
                  say ("%$% Ignore mask for %s was not reset\n" \
                       "  %$192%$196%$16 %s.\n", MILD,\
                       gptr->host, s); \
                } \
                  \
            /*return 1;*/ \
           } \
        else \
           { \
            say("%$% Ignore mask for %s was not set.\n", MILD, gptr->host); \
                                                                            \
            /*return 0;*/ \
           }


 static __inline__ int ClearIgnoreList (List *ptr, int how, int mode, void *v)

 {
  extern MiscTable *const mt_ptr;

    if (ptr->nEntries==0)
       {
        say ("%$% No entries found in Ignore List.\n", MILD);

        return;
       }

   switch (how)
          {
           case EVERYTHING:
                {
                 int i=0;
                 register ListEntry *eptr=ptr->head;
                          ListEntry *aux;
                 register Ignore *gptr;

                   while (eptr!=NULL)
                         {
                          i++;
                          aux=eptr->next;
                          (Ignore *)gptr=(Ignore *)eptr->whatever;
                          COMMUNIQUE();
                          free (gptr);
                          free (eptr);
                          eptr=aux;
                         }

                  ptr->nEntries=0;
                  ptr->head=ptr->tail=NULL;

                  say ("%$% Cleared %d entr%s in Ignore List.\n", INFO,
                       i, i>1?"ies":"y");

                  return i;
                }

           case WILDCARD:
                {
                 int n=0;
                 register ListEntry *eptr=ptr->head;
                          ListEntry *aux;
                 register Ignore *gptr;

                   while (eptr!=NULL)
                       {
                        aux=eptr->next;
                        (Ignore *)gptr=(Ignore *)eptr->whatever;

                         if (gptr->flags&mode)
                            {
                             n++;
                             ACTUAL_REMOVAL();
                            }

                        eptr=aux;
                       }

                   if (n==0)
                      {
                       say ("%$% No change whatsoever to Ignore List.\n", INFO);

                       return 0;
                      }
                   else
                      {
                       say ("%$% Modified %d entr%s in Ignore List.\n", INFO,
                            n, n>1?"ies":"y");

                       return n;
                      }
                }

           case NAME:
                {
                 char *name;
                 register ListEntry *eptr=ptr->head;
                 register Ignore *gptr;

                  name=(char *)v;

                   for ( ; eptr!=NULL; eptr=eptr->next)
                       {
                        (Ignore *)gptr=(Ignore *)eptr->whatever;

                          if (arethesame(name, gptr->host))
                             {
                              ACTUAL_REMOVAL()

                              return 1;
                             }
                        }

                 say ("%$% Unable to find %s in Ignore List.\n", MILD, name);

                 return 0;
                }

           case NUMBER:
                {
                 int *m=0,
                      n=0;
                 register ListEntry *eptr=ptr->head;
                 register Ignore *gptr;

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
                             (Ignore *)gptr=(Ignore *)eptr->whatever;

                             ACTUAL_REMOVAL()

                             return 1;
                            }
                       }

                 return 0;
                }
          }

 }  /**/


 void RemovefromIgnore (List *ptr, char *str, int mode)

 {
  int i,
      j;
  int escaped=0;
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
                   if ((escaped=ProcessEscaped(tok[j]+1)))
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

             if (arethesame(aux, "*"))
                 {
                   if (!escaped)
                      {
                       if ((mode^IG_ALL)==0)
                          {
                           ClearIgnoreList (ignore, EVERYTHING, 0, NULL);

                           return;
                          }
                       else
                          {
                           ClearIgnoreList (ignore, WILDCARD, mode, NULL);
                          }
                      }
                   else
                      {
                       goto plain_entry;
                      }
                 }
              else
              if (*aux=='#')
                 {
                  char *p=aux+1;

                   if (escaped)  goto plain_entry;

                   if ((!isdigits(p))||(!*p))
                      {
                       say ("%$% You really need -h.\n", MILD);

                       continue;
                      }
                   else
                      {
                       int i=atoi(p);

                        ClearIgnoreList (ignore, NUMBER, mode, &i);
                      }
                 }
              else
                 {
                  plain_entry:

                  ClearIgnoreList (ignore, NAME, mode, aux);
                 }
            }
      }

 }  /**/


 #define LOADMODE(x, y) ((x)|=(y))


 int ValidIgnoreType (const char *type)

 {
   if (arethesame(type, "-ALL"))  return IG_ALL;
   else
   if (arethesame(type, "-CTCP"))  return IG_CTCP;
   else
   if (arethesame(type, "-INV"))  return IG_INVITE;
   else
   if (arethesame(type, "-MSG"))  return IG_MSG;
   else
   if (arethesame(type, "-NOTC"))  return IG_NOTICE;
   else
   if (arethesame(type, "-TEL"))  return IG_TELNET;
   else
   if (arethesame(type, "-GLOB"))  return IG_GLOBAL;
   else
   return 0;

 }  /**/


 STD_IRCIT_COMMAND(uIGNORE)

 {
  register int add=0, j=0;
  int type=0;
  extern char tok[][80];

    if (!args)
     {
      ShowIgnore (ignore, 0);

      return;
     }

   splitargs2 (args, ' ');

    if (*tok[j]=='-') 
     {
       if (!(type=ValidIgnoreType(tok[j])))
        {
         LOADMODE(type, IG_ALL);  /* type implicit 'ig -|+entry' */
        }
       else
        {
          if (!*tok[++j])
           {
            ShowIgnore (ignore, type);

            return;
           }
        }
     }
    else
     {
      LOADMODE(type, IG_ALL);
     }

    if (*tok[j]=='+') 
     {
      add=1;
     }
    else
    if (*tok[j]!='-')
     {
      say ("%$% You need -h.\n", MILD);

      return;
     }

   {
    char s[strlen(tok[j])];

     strcpy (s, tok[j]+1);

      if (add)
       {
        AddtoIgnore (ignore, s, type, 1);
       }
      else
       {
        RemovefromIgnore (ignore, s, type);
       }
   }

 }  /**/


 __inline__ char *MaskIgnoreEntry (const char *ent)

 {
  char *nick=NULL,
       *user=NULL,
       *host=NULL,
       *userhost=NULL;
  char aux[]={[0 ... MAXHOSTLEN]=0};
  static char masked[MAXHOSTLEN];

   strcpy (aux, ent);
   nick=aux;

    if ((userhost=strchr(aux, '!')))
     {
      *userhost++=0;

      user=userhost;

       if ((host=strchr(userhost, '@')))
        {
         *host++=0;
        }
       else
       if (strchr(userhost, '.'))
        {
         user = "*";
         host=userhost;
        }

       if (!user)  user="*";
       if (!host)  host="*";
     }
    else
     {
      user=aux;

       if ((host=strchr(user, '@')))
        {
         nick="*";
         *host++=0;
        }
       else
        {
          if (strchr(user, '.'))
           {
            nick="*";
            host=user;
            user="*";
           }
          else
           {
            nick=user;
            user="*";
            host="*";
           }
        }
     }

  sprintf (masked, "%s!%s@%s", nick, user, *host?host:"*");

  return masked;

 }  /**/


 #define IS_MATCHINGIGNORETYPE() (gptr->flags&type)

 int IsSheIgnored (char *str, int type)

 {
  register ListEntry *eptr;
  register Ignore *gptr;

    if (NO_ENTRIES(ignore))
     {
      return 0;
     }

   eptr=ignore->head;

    for ( ; eptr!=NULL; eptr=eptr->next)
     {
      (Ignore *)gptr=(Ignore *)eptr->whatever;

       if (IS_MATCHINGIGNORETYPE())
        {
         if (match(MaskIgnoreEntry(gptr->host), str))
          {
           gptr->hits++;

           return 1;
          }
        }
     }

   return 0;

 }  /**/

static  ActuallyPrint (const Ignore *, char *, size_t);

 static __inline__ ActuallyPrint (const Ignore *gptr, char *s, size_t cnt)

 {
  char t[6];

    if (gptr->when)
     {
      strftime (t, 10, "%H:%M", localtime(&gptr->when));
     }
    else
     {
      strcpy (t, "--:--");
     }

   RESOLVE_FLAGS(gptr);

   say (" %$179%2d%$179%-15.15s%$179%-30.30s%$179%-5.5s%$179%-4d%$179\n",
        cnt, gptr->host, s, t, gptr->hits);

 }  /**/

       
 void ShowIgnore (List *ptr, int mode)

 {
  int i=0;
  char s[]={[0 ... MINIBUF]=0};
  register ListEntry *eptr;

    if (NO_ENTRIES(ignore))
     {
      say ("%$% No Ignore entries.\n", MILD);

      return;
     }

   DisplayHeader (13, 5);

    for (eptr=ignore->head; eptr!=NULL; eptr=eptr->next)
     {
       if (mode)
        {
         if (((Ignore *)eptr->whatever)->flags&mode)
          {
           ActuallyPrint ((Ignore *)eptr->whatever, s, ++i);
          }
        }
       else
        {
         ActuallyPrint ((Ignore *)eptr->whatever, s, ++i);
        }

      memset (s, 0, MINIBUF);
     }

   finish2 (13, 5);

 }  /**/
 
