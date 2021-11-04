/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
*/

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include <main.h>
#include <terminal.h>
#include <output.h>
#include <list.h>
#include <scroll.h>
#include <serverr.h>
#include <events.h>
#include <uDCC.h>
#include <useri.h>
#include <useri-data.h>
#include <serverio.h>
#include <updates.h>

 MODULEID("$Id: serverio.c,v 1.10 1998/09/19 01:12:39 ayman Beta $");


#define CTCP_CHAR 0x1

 char buf_q[1024];
 static char command_char='/';
 char *const command_char_ptr=&command_char;

 const int nServerCmnds=((int)(sizeof(sFunction)/sizeof(ServerCmnd)));
 const int nUserCmnds=((int)(sizeof(uFunction)/sizeof(Command)));

 ITCommand *itcmnd;
 size_t glob_cnt;

 const struct InResponseToUser *u=uFunction;


 int ParseServerMsg (void)

 {
  char *command="",
       *from="";
  int r;
  register size_t i;
  size_t l=0,
         u=nServerCmnds;
  char *buf=buf_q;
  const register ServerCmnd *p;

   /* log (1, "'%s'", buf); */

    if (*buf==':')
     {
      buf++;
      from=splitw (&buf);
     }

   command=splitw (&buf);

    while (l<u)
     {
      p=(sFunction+(i=(l+u)/2));

       if ((r=strcasecmp(command, p->identifier))<0) 
        {
         u=i;
        }
       else 
       if (r>0) 
        {
         l=i+1;
        }
       else 
        {
         (*p->answer)(from, buf?buf:""); 

         buf_q[0]=0;

         return;
        }
     }

   {
    register void *auxptr;

     if ((auxptr=EventRegistered(EVNT_ODDSRVCMND)))
      {
       ExecHook (auxptr, "%s %s %s", from, command, buf?buf:"");
      }
     else 
      {
       say ("%s=%s=%s\n", from, command, buf?buf:"");
      }
   } 

   buf_q[0]=0;

 }  /**/


#define c_idp (itcmnd+(i+l))->name
#define c_idm (itcmnd+(i-l))->name

 void ParseUserInput ()   

 {
  register char *cmnd;
  char *buf;
  int id=0,
      len,
      found=0,
      cnt=0;
  register int i=0;
  const ITCommand *matches[]={[0 ... 100]=NULL};
  const register ITCommand *ptr;
  extern char buf_i[];
  extern void ExecAlias (void *, char *);
 
   buf=buf_i;
 
    if (*(buf+0)==command_char)
       {
        buf++;
        wipespaces (buf);
        cmnd=splitw (&buf);

         if ((len=strlen(cmnd))>20)
          {
           say ("%$% Commmand too long.\n", MILD);

           return;
          }

        {
         int r;
         int i,
             l=0,
             u=glob_cnt;

          while (l<u)
           {
            ptr=(itcmnd+(i=(l+u)/2));

             if ((r=strncasecmp(cmnd, ptr->name, len))<0)
              {
               u=i;
              }
             else
             if (r>0)
              {
               l=i+1;
              }
             else
              {
                if (strcasecmp(cmnd, ptr->name)==0)
                 {
                  matches[(cnt=1)-1]=ptr;
                  break;
                 }
                else
                 {
                  int plus=1, minus=1;

                   matches[cnt++]=ptr;

                    for (l=1; (plus||minus); l++)
                     {
                       if (plus)
                        {
                         if ((i+l<glob_cnt)&&
                             (ptr=&itcmnd[i+l], 
                             (strncasecmp(cmnd, ptr->name, len))==0))
                          {
                            if (*(ptr->name+len)==0)
                             {
                              matches[(cnt=1)-1]=ptr;
                              break;
                             }

                           matches[cnt++]=ptr;
                          }
                          else  plus=0;
                        }

                       if (minus)
                        {
                         if ((i-l>=0)&&
                             (ptr=&itcmnd[i-l],
                              (strncasecmp(cmnd, ptr->name, len))==0))
                          {
                            if (*(ptr->name+len)==0)
                             {
                              matches[(cnt=1)-1]=ptr;
                              break;
                             }

                           matches[cnt++]=ptr;
                          }
                         else  minus=0;
                        }
                     }

                  break;
                 }
              }
           }
        }
   
      if (cnt==1)
       {
        if (((ITCommand *)*matches)->attrs&1)
         {
          ((ITCommand *)*matches)->how(buf?buf:NULL);
         }
        else
        if (((ITCommand *)*matches)->attrs&2)
         {
          ExecAlias ((void *)((ITCommand *)*matches)->how, buf?buf:"");
         }
       }
      else
      if (cnt>1)
       {
        say ("%$% Found at least %d matches..\n", INFO, cnt);

         for (i=0; i<cnt; i++)
          {
           say (" %-19.19s", ((ITCommand *)*(matches+i))->name);

            if ((i+1)%3==0)  say ("\n");
          }

         if (i%3!=0)  say ("\n");
       }
      else
       {
        if (set("DISPATCH_UNKNOWN_TO_SRV"))
         {
          ToServer ("%s %s\n", cmnd, buf?buf:" ");
         }
        else
         {
          say ("%$% I don't know what to do with this.\n", MILD);
         }
       }
     }
    else
     {
      if (*curchan())
       {
        ToServer ("PRIVMSG %s :%s\n", curchan(), buf);

        say ("%$%%s%$% %s\n", 243|A_BOLD, curnick(), 242|A_BOLD, buf);
       }
      else
       {
        say ("%$% No one will see that!\n", MILD);
       }
     }
  
 }  /**/


 int CompleteCommand (const char *in, ITCommand **out, int max)

 {
  register int cnt=0;
  int i, 
      l=0,
      r,
      u=glob_cnt;
  size_t len;
  register ITCommand *ptr;

   len=strlen (in);

    while (l<u)
     {
      ptr=(itcmnd+(i=(l+u)/2));

       if ((r=strncasecmp(in, ptr->name, len))<0)
        {
         u=i;
        }
       else
       if (r>0)
        {
         l=i+1;
        }
       else
        {
         if ((r=strcasecmp(in, ptr->name))==0)
          {
           out[(cnt=1)-1]=ptr;
           break;
          }
         else
          {
           int plus=1, minus=1;

            out[cnt++]=ptr;

             for (l=1; (plus||minus)&&(cnt<=max); l++)
              {
                if (plus)
                 {
                  if ((i+l<glob_cnt)&&
                      ((strncasecmp(in, c_idp, len))==0))
                   {
                     if (*(c_idp+len)==0)
                      {
                       out[(cnt=1)-1]=&itcmnd[i+l];
                       break;
                      }

                    out[cnt++]=&itcmnd[i+l];
                   }
                  else  plus=0;
                 }

                if (minus)
                 {
                  if ((i-l>=0)&&((strncasecmp(in, c_idm, len))==0)) 
                   {
                     if (*(c_idm+len)==0)
                      {
                       out[(cnt=1)-1]=&itcmnd[i-l];
                       break;
                      }

                    out[cnt++]=&itcmnd[i-l];
                   }
                  else  minus=0;
                 }
              }

            break;
          }
        }
     }

   return cnt;

 }  /**/


 void BuildSymbolTable (void)

 {
  register int i=0;
  register ITCommand *ptr;

   itcmnd=calloc (nUserCmnds, sizeof(ITCommand));

    for ( ; i<=nUserCmnds-1; i++)
     {
      itcmnd[i].name=uFunction[i].identifier;
      itcmnd[i].how=uFunction[i].read;
      itcmnd[i].help=uFunction[i].alias;
      itcmnd[i].attrs|=0x1;
     }

   glob_cnt=nUserCmnds;

 }  /**/


 int  cmpcmnd (const void *ptr1, const void *ptr2)

 {
   return strcasecmp (((ITCommand *)ptr1)->name, ((ITCommand *)ptr2)->name);

 }  /**/


 void UpdateSymbolTable (char *name, void *how, char *hlp, int attrs)

 {
  register int i=0;
  register ITCommand *ptr;

    {
     int r;
     int l=0,
         u=glob_cnt,
         cnt=0,
         len=strlen(name);

       while (l<u)
        {
         ptr=(itcmnd+(i=(l+u)/2));

          if ((r=strncasecmp(name, ptr->name, len))<0)
           {
            u=i;
           }
          else
          if (r>0)
           {
            l=i+1;
           }
          else
           {
            if (arethesame(name, ptr->name))
             {
              cnt++;
              break;
             }
            else
             {
              int plus=1, minus=1;

                 for (l=1; (plus||minus); l++)
                  {
                   if (plus)
                    {
                     if ((i+l<glob_cnt)&&
                         (ptr=&itcmnd[i+l],
                         (strncasecmp(name, ptr->name, len))==0))
                      {
                       if (*(ptr->name+len)==0)  {cnt++; break;}
                      }
                     else  plus=0;
                    }

                   if (minus)
                    {
                     if ((i-l>=0)&&
                         (ptr=&itcmnd[i-l],
                         (strncasecmp(name, ptr->name, len))==0))
                      {
                       if (*(ptr->name+len)==0)  {cnt++; break;}
                      }
                     else  minus=0;
                    }
                  }
               break;
             }
           }
        }

       if (cnt==0)  
        {
         glob_cnt++;
         itcmnd=realloc (itcmnd, sizeof(ITCommand)*glob_cnt);

         ptr=&itcmnd[glob_cnt-1];

         ptr->name=name;
         ptr->how=how;
	 ptr->help=hlp;
         ptr->attrs=0, ptr->attrs|=attrs;

         qsort (itcmnd, glob_cnt, sizeof(ITCommand), cmpcmnd);
        }
       else 
        {
         ptr->name=name;
         ptr->how=how;
         ptr->help=hlp;
         ptr->attrs=0, ptr->attrs|=attrs;
        }

      return;
    }

 }  /**/


 void RemovefromSymbolTablee (const char *name)

 {
  int cnt=0;
  register int i=0;
  ITCommand *matches[glob_cnt];
  register ITCommand *ptr=NULL;
  char dummy=254;

   cnt=CompleteCommand (name, matches, glob_cnt);

    if (!cnt)
     {
      say ("%$% Unable to find %s in symbol table.\n", MILD, name);

      return;
     }
    
    for ( ; i<=(cnt-1); i++)
     {
      if (arethesame(name, ((ITCommand *)*(matches+i))->name))
       {
        ptr=matches[i];
        break;
       }
     }

    if (!ptr)
     {
      say ("%$% Unable to find %s in symbol table.\n", MILD, name);

      return;
     }

   ptr->name=&dummy;
   qsort (itcmnd, glob_cnt, sizeof(ITCommand), cmpcmnd);
   glob_cnt--;

   itcmnd=realloc (itcmnd, sizeof(ITCommand)*glob_cnt); /* shrink! */

 }  /**/


 void mCTCPToServer (const char *nick, const char *format, ...)

 {
  char buffer [600];
  va_list va;

   va_start (va, format);
   vsprintf (buffer, format, va);
   va_end (va);

   ToServer ("PRIVMSG %s :\01%s\01\n", nick, buffer);

 }  /**/


 void nCTCPToServer (const char *nick, const char *format, ...)

 {
  char buffer [600];
  va_list va;

   va_start (va, format);
   vsprintf (buffer, format, va);
   va_end (va);

   ToServer ("NOTICE %s :\01%s\01\n", nick, buffer);

 }  /**/


 int ToServer (const char *format, ...)

 {
  static char buffer [600];
  va_list va;

   va_start (va, format);
   vsprintf (buffer, format, va);
   va_end (va);

    if (strlen(buffer)>510)
     {
      buffer[510]=0;
     }

    if (isup())
     {
      return WriteToSocket(isup(), buffer);
     }

 }  /**/


 #include <windows-aux.h>


 void LastmsgEntries (ScrollingList *);
 void LastmsgEnter (void *, int);

#include <lastmsg-data.h>


 void LastmsgList (void *ptr, int idx)

 {
  ScrollingList *scrptr=&msg_list;

   ((ScrollingList *)(scrptr->opt_menu))->parent=scrptr;

   LoadScrollingList (&msg_list);

 }  /**/


 void LastmsgEntries (ScrollingList *sptr)

 {
  WINDOW *w=RetrieveCurrentWindow();

   wmove (w, 0, 0);
   Fwprintf_nout (w, " %-15.15s%$179%-26.26s", lastmsg_n(), lastmsg_h());

 }  /**/


 void LastmsgInit (ScrollingList *sptr)

 {
  extern List *irc_notify;

    if (!isup())  sptr->attrs[0]=0;
    else sptr->attrs[0]=1;

    if (inNotifyList(irc_notify, lastmsg_n()))  sptr->attrs[2]=0;
    else  sptr->attrs[2]=1;

 }  /**/


 void LastmsgEnter (void *sptr, int idx)

 {
  char s[LARGBUF];

   switch (idx)
    {
     case 0:
          ToServer ("WHOIS %s\n", lastmsg_n());
          break;

     case 1:
          {
           register char *p;

            p=strchr (lastmsg_h(), '@');

            sprintf (s, "-ADDR %s", p?p+1:"");
            uLOOKUP (s);
            break;
          }

     case 2:
          sprintf (s, "+%s", lastmsg_n());
          uNOTIFY (s);
          break;

     case 3:
          sprintf (s, "+%s!%s", lastmsg_n(), lastmsg_h());
          uIGNORE (s);
          break;
    }

 }  /**/   

