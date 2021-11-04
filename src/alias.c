/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
*/

#include <main.h>
#include <terminal.h>
#include <updates.h>
#include <list.h>
#include <alias.h>
#include <FPL.h>

 MODULEID("$Id: alias.c,v 1.2 1998/09/19 02:45:51 ayman Beta $");

 List AliasList;
 List *const als_ptr=&AliasList;

 int AddAlias (List *lptr, const char *name, char *expand, const char *hlp)

 {
  register ListEntry *eptr=NULL;

    for (eptr=lptr->head; eptr!=NULL; eptr=eptr->next)
     {
      if (arethesame(name, (((Alias *)eptr->whatever)->name)))
       {
        break;
       }
     }

    if (eptr)  /* update already existing entry */
     {
      free ((((Alias *)eptr->whatever)->what));
       if (*((Alias *)eptr->whatever)->help)
        {
         free (((Alias *)eptr->whatever)->help);
         ((Alias *)eptr->whatever)->help=empty_string;
        }
     }
    else
     {
      Alias *alsptr;

       xmalloc(alsptr, (sizeof(Alias)));
       memset (alsptr, 0, sizeof(Alias));

       eptr=AddtoList (lptr);
       (Alias *)eptr->whatever=alsptr;

       ((Alias *)eptr->whatever)->name=strdup (name);
     }

   /* Build the new alias definition and update the help if available */
   {
    ((Alias *)eptr->whatever)->what=strdup (expand);

     if (*hlp)  ((Alias *)eptr->whatever)->help=strdup (hlp);
     else  ((Alias *)eptr->whatever)->help=empty_string;
   } 

   UpdateSymbolTable (((Alias *)eptr->whatever)->name,
                      ((Alias *)eptr->whatever)->what, 
                      ((Alias *)eptr->whatever)->help,
                      2);

   return 1;

 }  /**/


 int RemoveAlias (List *lptr, const char *name)

 {
  register ListEntry *eptr;

    for (eptr=lptr->head; eptr!=NULL; eptr=eptr->next)
     {
      if (arethesame(name, (((Alias *)eptr->whatever)->name)))
       {
        RemovefromSymbolTablee ((((Alias *)eptr->whatever)->name));

        free ((((Alias *)eptr->whatever)->what));
        free ((((Alias *)eptr->whatever)->name));
         if (*(((Alias *)eptr->whatever)->help))
            free ((((Alias *)eptr->whatever)->help)); 
 
        RemovefromList (lptr, eptr);

        return 1;
       }
     }

   return 0;

 }  /**/


 int MarkWords (char *str, char **pos)

 {
  int n=0;
  register char *s=str;
  register char **aux=pos;

    while ((*s)&&(n<260))
     {
      while (isspace(*s))  ++s;

      if (*s)
       {
        *aux++=s;
        ++n;

         while ((!isspace(*s))&&(*s))  ++s;
       }
     }

   return n;

 }  /**/


 static char *convert (char *str, unsigned long num)

 {
  register int i = 0;
  char tmp[]={[0 ... 32]=0};
  const char *digits="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  #define do_div(n, base) ({ \
     int __res; \
      __asm__("divl %4":"=a" (n),"=d" (__res):"0" (n),"1" (0),"r" (base)); \
      __res; })

   if (num==0)
    {
     tmp[i++]='0';
    }
   else
    {
     while (num!=0)
      {
       tmp[i++]=digits[do_div(num, 10)];
      }
    }

   while (i-->0)  *str++=tmp[i];

  return str;

 }   /**/


 #define QT() \
          if (h)  {*str++='"';}

 #include <prefs.h>

 char *Substitute2 (char **, char *, char *[], int, int);

 char *Substitute2 (char **fmtt, char *b, char *words[], int nwords, int h)

 {
  int cnt=0,
      num,
      num2;
  unsigned flags=0;
  char *fmt=*fmtt;
  register char *s, 
                *str,
                *stre; 

   str=b;
   stre=(b+519);

    if (h) 
     {
      memmove (str+1, str, strlen(str)+1); 
      *str++='"';
       if (fmt==(str-1))  fmt++;
     }

    again:
    switch (*fmt)
     {
      case 'C':
             if ((flags&1)==0)
              {
               s=curchan();
                while ((*s)&&(*str++=*s++)); 
              }
             else
              {
               str=convert (str, strlen(curchan()));
              }
            break;

      case 'N':
             if ((flags&1)==0)
              {
               s=curnick();
                while ((*s)&&(*str++=*s++));
              }
             else
              {
               str=convert (str, strlen(curnick()));
              }
            break;

      case 'S':
             if ((flags&1)==0)
              {
               s=curserver();
                while ((*s)&&(*str++=*s++));
              }
             else
              {
               str=convert (str, strlen(curserver()));
              }
            break;

      case '*':
            s=words[0];
             if ((flags&1)==0)
              {
               while ((stre!=str)&&(*s)&&(*str++=*s++));
              }
             else
              {
               str=convert (str, strlen(s));
              }
            break;

      case 't':
            {
             time_t now=time (NULL);

              str=convert (str, now);
              break;
            }

      case '$':  /* last word */
            s=words[nwords?nwords-1:0];
             if ((flags&1)==0)
              {
               while ((stre!=str)&&(*s)&&(*str++=*s++));
              }
             else
              {
               str=convert (str, strlen(s));
              }
            break;

      case '^':  /* first word */
            s=words[0];
             if ((flags&1)==0)
              {
               while ((stre!=str)&&(*s)&&!isspace(*s))  *str++=*s++;
              }
             else
              {
               while ((*s)&&!isspace(*s))  s++;
               str=convert (str, s-words[0]);
              }
            break;

      case '?':
            str=convert (str, nwords);
            break;

      case '@':
            fmt++; 
            flags|=(0x1<<0); 
            goto again;

      case '{':
            {
             register int z=0;
             char q[strlen(fmt)+1];

               while ((*++fmt)&&(*fmt!='}'))   q[z++]=*fmt;

              q[z]=0;
               if (*fmt=='}')
                {
                 if (s=getenv(q))
                  {
                   if ((flags&1)==0)
                    {
                     while ((stre!=str)&&(*str++=*s++));  str--;
                    }
                   else
                    {
                     str=convert (str, strlen(s));
                    }
                  }
                }

             break;
            }

      case '<':
           {
            register int z=0;
            char q[strlen(fmt)+1];
            register PrefsTable *prefptr;

              while ((*++fmt)&&(*fmt!='>'))   q[z++]=*fmt;

             q[z]=0;
              if (*fmt=='>')
               {
                if (!q[0])  s="";
                else
                if (!(prefptr=FindEntry(q)))  s="";
                else
                if (!(s=prefptr->entry.value)) s="";

                if ((prefptr)&&(prefptr->entry.level==DIGITS))
                 {
                  str=convert (str, atoi(s));
                 }
                else
                 {
                  if ((flags&1)==0)
                   {
                    while ((stre!=str)&&(*str++=*s++));  str--;
                   }
                  else
                   {
                    str=convert (str, strlen(s));
                   }
                 }
               }
             break;
           }

      case '\\':  /* $\# */
             if (stre!=str)  *str++=*fmt;
            break;

      case '1' ... '9':
            {
             num=(int)skip_atoi ((char **)&fmt);

              if (!nwords)
               {
                num=num2=1;
               }
              else
              if (num<=0)
               {
                num=1;
               }
              else
              if (num>260) 
               {
                num=260;
               }

               {
                switch (*fmt)
                 {

                  case '+':
                         if (num>nwords)  num2=num;
                         else  num2=nwords;

                        fmt++;
                        break;

                  case '-':
                        num2=num, num=1;
                        fmt++;
                        break;

                  case '>':
                        fmt++;
                        num2=(int)skip_atoi ((char **)&fmt);

                         if (num2>nwords)  num2=nwords;
                         if (num2<=0)  num2=num;
                        break;

                  case '^':
                       {
                        register char *q;

                         fmt++;
                         num2=(int)skip_atoi ((char **)&fmt);
  
                          if (num2>nwords)  num2=nwords;
                          if (num2<=0)  num2=num;

                          s=words[num-1];
                          q=words[num2-1];

                           while ((str!=stre)&&(*s)&&(!isspace(*s)))
                            {
                             ((flags>>0)&0x1)?(s++, cnt++):(*str++=*s++);
                            }
                           while ((str!=stre)&&(*q)&&(!isspace(*q)))
                            {
                             ((flags>>0)&0x1)?(q++, cnt++):(*str++=*q++);
                            }

                           if ((flags>>0)&0x1)
                            {
                             str=convert (str, cnt);
                            }

                          fmt--;

                          goto end;
                        }
               
                  case '~': /* $2~ reverse $2 */
                       {
                        register char *q;

                         q=s=words[num-1];
                          while ((*s)&&(!isspace(*s)))  q=s++;

                          while (q!=words[num-1]) *str++=*q--;

                          if (*q)  *str++=*q;
                         goto end;
                       }

                  default:
                        num2=num;
                 }

                {
                 if (num==num2)
                  {
                   s=words[num-1];
                    if ((flags>>0)&0x1)
                     {
                      register int cnt=0;

                        while ((*s)&&(!isspace(*s++)))  ++cnt;
                       str=convert (str, cnt);
                     }
                    else
                     {
                      while ((*s)&&(!isspace(*s)))  *str++=*s++;
                     }
                  }
                 else
                 if (num2>num)  /* 2-5 */
                  {
                   register char *q;

                    s=words[num-1];
                    q=words[num2-1];

                     if ((flags&0x1)==0)
                      {
                       while ((*s)&&(s!=q))  *str++=*s++;
                       while ((*s)&&(!isspace(*s)))  *str++=*s++;
                      }
                     else
                      {
                       register int cnt=0;
                         while ((*s)&&(s++!=q))  ++cnt;
                         while ((*s)&&(!isspace(*s++)))  ++cnt;
                        str=convert (str, cnt);
                      }
                  }
                 else
                  {
                   register char *q;

                    if (((flags>>0)&0x1)==0)
                     {
                       while (num!=num2-1)
                        {
                         s=words[num-1];
                          while ((*s)&&(!isspace(*s)))  *str++=*s++; *str++=' ';                         num--;
                        }
                      str--;  /* remove last added space */
                     }
                    else
                     {
                      /* implement len for 5>2 */
                     }
                  }
                }
                --fmt;
               }

             break;
            }

      default:
            if (*fmt)  
             {
               while (*str++=*fmt++);
              --str, --fmt;
             }
            else  --fmt;
     }

   end:
   QT()
   *str=0;
   *fmtt=fmt;

   return b;

 }   /**/


 static char *Prescan (char *in)
 
 {
  register char *s;
  char *buf;

   xmalloc(buf, (strlen(in)*2)*sizeof(char));
   s=buf;

   start_lable:
    while ((*in)&&(*in!='"'))  *s++=*in++;

    if (*in=='"')
     {
      *s++='\\';
      *s++='"';
      in++;

      goto start_lable;
     }

  *s=0;

  return (char *)buf;

 }  /**/


 typedef struct Token_ {
                 unsigned attr;
                } Token_;

 int ExpandAlias (char *fmtt, char *in)

 {
  int qt=0,
      nargs;
  char *str,
       *fmt, 
       *fmte,
       *scanned;
  char gg[]={[0 ... 520]=0};
  char ident[]={[0 ... 66]=0};
  char buf[]={[0 ... XLARGBUF]=0}; 
  char *words[]={[0 ... 270]=""};
  extern void *key;

   scanned=Prescan (in);

   nargs=MarkWords (/*in*/scanned, words); 

   fmt=&buf[0];
   fmte=&buf[XLARGBUF-1]; 

    for (str=fmtt; (fmt!=fmte)&&(*str); ++str)
     {
       if ((fmt!=fmte)&&(*str!='$'))
        {
         *fmt++=*str;
         continue;
        }

      again:
      ++str;
      switch (*str)
             {
              case '\\':
                   {
                     if (fmt!=fmte)  *fmt++='$';
                    break;
                   }

              case '(':
                   {
                    int nf=0, np=0;
                    register int cnt=0, melyseg=0;
                    Token_ toks[]={[0 ... 40]=0};

                      if (qt)  toks[0].attr|=1;
                     melyseg++, np++;
                      while ((melyseg<39)&&(nf!=np)&&(*++str))
                       {
                        switch (*str)
                         {
                          case '$':
                                melyseg++;
                                break;
                          case '(':
                                np++;
                                break;
                          case ')':
                                nf++;
                                break;
                          case '#':
                                toks[melyseg-1].attr|=0x1;
                                break;

                          default:
                                {
                                  while ((cnt<65)&&(*str)&&(*str!=')'))
                                   {
                                    ident[cnt++]=*str++;
                                   }

                                 ident[cnt]=0; 
                                 str--;
                                }
                         }
                       }

                     ident[cnt]=0;
                      if (nf!=np)
                       {
                        say ("error..nf=%d, np=%d\n", nf, np);
                        return;
                       }
                      else
                       {
                        char *q=ident;
                  
                          while (melyseg--)
                           {
                            char *w; 

                             w=Substitute2 (&q, gg, words, nargs, 
                                            toks[melyseg].attr&1?1:0);
                             q=w;
                            }

                         qt=0;
                         while ((fmt!=fmte)&&(*fmt++=*q++));

                          if (!*fmt)  fmt--;
                       }

                     break;
                   }

              case '#':
                   {
                    qt++; 
                    goto again;
                   }

              default:
                   {
                    char *w;
                   
                     w=Substitute2 (&str, gg, words, nargs, qt);

                      while ((fmt!=fmte)&&(*fmt++=*w++));

                      if (!*fmt)  fmt--;
                     qt=0;
                   }
             }
     }

    if (fmt==fmte)  *fmt=0;

   fmte=buf;
   fplExecuteScriptTags (key, &fmte, 1, FPLTAG_DONE);

   free (scanned);

 }  /**/


 /* obsolete! */
 int TryAliasesInstead (char *cmnd, char *args)

 {
  register ListEntry *eptr;

    for (eptr=als_ptr->head; eptr!=NULL; eptr=eptr->next)
     {
      if (arethesame(cmnd, (((Alias *)eptr->whatever)->name)))
       {
        ExpandAlias ((((Alias *)eptr->whatever)->what), args);

        return 1;
       }
     }

   return 0;

 }  /**/


 void ExecAlias (void *what, char *args)

 {
   ExpandAlias ((char *)what, args);

 }  /**/


 /*
 ** /alis -alis
 ** /alias mw "kksokkoso"
 */
 STD_IRCIT_COMMAND(uALIAS)

 {
  extern char tok[][80];

    if (!args)
     {
      ShowAliases (als_ptr, NULL);

      return;
     }

   splitargs2 (args, ' ');

    if (*tok[0]=='-')
     {
       if (RemoveAlias(als_ptr, tok[0]+1))
        {
         say ("%$% Alias deleted.\n", INFO);
        }
       else 
        {
         say ("%$% Alias not found.\n", MILD);
        }

      return;
     }
    else
     {
      char *w;

       if (!*tok[1])
        {
         ShowAliases (als_ptr, tok[0]);

         return;
        }

      w=splitw(&args);

       if (strlen(w)>20)
        {
         say ("%$% Poorly formatted alias.\n", MILD);

         return;
        }

      AddAlias (als_ptr, w, args, empty_string);

      say ("%$% Added alias %s\n  %$192%$196%$16 %-20s\n", INFO, w, args);
     }

 }  /**/


 static void ShowThisAlias (const Alias *alsptr, int i)

 {
   if (alsptr)
    {
     if (strlen(alsptr->what)>40)
       {
        say (" %$179%-3d%$179%-15.15s%~%$179%-.37s...%~%$179\n",
             i, alsptr->name, alsptr->what);
       }
      else
       {
        say (" %$179%-3d%$179%-15.15s%~%$179%-40.40s%~%$179\n",
             i, alsptr->name, alsptr->what);
       }
    }

 }  /**/


 void ShowAliases (List *lptr, const char *name)

 {
  register int i=1;
  register ListEntry *eptr;

    if (NO_ENTRIES(lptr))
     {
      say ("%$% No aliases found.\n", MILD);

      return;
     }

   DisplayHeader (17, 3);

    for (eptr=lptr->head; eptr!=NULL; eptr=eptr->next, i++)
     {
      if (!name)
       {
        ShowThisAlias ((Alias *)eptr->whatever, i);
       }
      else
       {
        if (arethesame(name, ((Alias *)eptr->whatever)->name))
         {
          ShowThisAlias ((Alias *)eptr->whatever, i);

          break;
         }
       }
     }

   finish2(17, 3);

 }  /**/

