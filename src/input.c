/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <main.h>
#include <terminal.h>
#include <output.h>
#include <updates.h>
#include <list.h>
#include <history.h>
#include <scroll.h>
#include <stack.h>
#ifdef _HAVE_GPM
 #include <gpm.h>
#endif
#include <u_status.h>
#include <funckeys.h>
#include <serverio.h>
#include <input.h>

 MODULEID("$Id: input.c,v 1.11 1998/06/07 09:20:26 ayman Beta $");

 static void RedrawInputWin (void);
 static void whline_ (WINDOW *, chtype, int);
 static void NewCursorPos (char *);
 static void Addchar (int);
 static void Delchar (int);
 static int IsWordChar (char);
 static void BDelword (void);
 static void FDelchar (void);
 static void Delline (void);
 static void GoLeft (void);
 static void GoRight (void);
 static void GoLineStart (void);
 static void GoLineEnd (void);
 static void TabKey (void);
 static void HistoryUp (void);
 static void HistoryDown (void);

 static void ShowThisKeyBinding (const FunctionKeys *);
 static void ShowKeyBindings (const FunctionKeys *);


#define H_LINES      0x15
#define H_LINEID     0x16
#define H_TABS       0x17
#define H_TABID      0x18

 int wait=0;

#define Y 0
#define X 8
#define buf_sz 510
#define visibleww 71

 extern char buf_i[];

 static char *start_win=buf_i;
 static char *end_win=buf_i+visibleww-1;
 static char *dst_buf=buf_i;
 static char *in_buf=buf_i;
 static size_t in_buflen; 
 static int update;
 extern int status_busy;

 extern InputHistory *arrows;
 static Input *CurHistLine;

 extern InputHistory *tab;
 static Input *CurTabLine,
              *LastTabLine;


 static __inline__ void whline_ (WINDOW *win, chtype ch, int n)

 {
  short line;
  short start;
  short end;

   line=win->_cury;

    if ((start=win->_curx, end=(start+n-1))>(win->_maxx)) end=win->_maxx;

    if (win->_line[line].firstchar==_NOCHANGE||win->_line[line].firstchar>start)
        win->_line[line].firstchar=start;

    if (win->_line[line].lastchar==_NOCHANGE||win->_line[line].lastchar<start)
        win->_line[line].lastchar=end;

    if (ch==0) ch=ACS_HLINE;

    if ((ch&A_CHARTEXT)==' ')
     {
      ch|=win->_bkgd;
     }
    else
    if (!(ch&A_ATTRIBUTES))
     {
      ch|=(win->_bkgd&A_ATTRIBUTES);
     }

   ch|=win->_attrs;

    while (end>=start)
     {
      win->_line[line].text[end]=ch;
      end--;
     }

 }  /**/


 static __inline__ void RedrawInputWin (void)

 {
  register char *s,
                *se;

   se=dst_buf+in_buflen, *se=0;

   _wmove (InputWin, Y, X);

    for (s=start_win; s<se; s++)
     {
       if (s>end_win)  goto refresh_;

      (*s)<32?(waddch_(InputWin, ((unsigned char)(*s)+64)|A_REVERSE)):
              (waddch_(InputWin, (unsigned char)*s));
     }

   whline_ (InputWin, ' ', (end_win-s)+1);

   refresh_:
   _wmove (InputWin, Y, X+(in_buf-start_win));

   wnoutrefresh (InputWin);
    /* doupdate (); */

   update=0;

 }  /**/


#include <useri.h>

 int Gets (int c)
 
 {
  int i,
      cc=0;
  extern int hold_screen;
  extern void log (int, ...);

    if (c<KEY_MIN) 
       {
         if (c<=31)
            {
             switch (c)
                    {
                     case ERR:
                           if (update)  RedrawInputWin();
                           wait=2;

                           return 0;

                     case '\r':
                     case '\n':
                          cc=1; 
                          goto done;

                     case '\b':
                          Delchar (1);
                          break;

                     case '\t':
                          TabKey ();
                          break;

                     case 0x03:  /* ^C */
                          {
                           Addchar (c); 
                           break;
                          }

                     case 0x10: 
                          break;

                     case 0x0E:
                          break;

                     case 0x0B:
                          FDelchar ();
                          break;

                     case 23:
                          BDelword ();
                          break;

                     /*case 0x0C:
                          touchwin (curscr);
                          wrefresh (curscr);
                          break;*/

                     case 0x06: 
                          uCTRLF ();
                          break;

                     case 0x18:
                          uCTRLX ();
                          break;

                     case 0x13:
                          GoLineStart ();
                          break;

                     case 0x05:
                          GoLineEnd ();
                          break;

                     case 0x15:
                          Delline ();
                          break;

                     case 27:
                          break;

                     default:
                          Addchar (c);
                    }
            }
         else
            {
             Addchar (c);
            }
       }
    else
    if (c<KEY_MAX)
       {
         if ((c<=KEY_F(MAX_F_KEYS))&&(c>=KEY_F0))
            {
              if (FuncKeys[(c-264)].fplexpr)
               {
                ExpandAlias (FuncKeys[(c-264)].fplexpr, "");
                update=1;
               }
              else
               {
                beep ();
               }
            }
#if 0
         if ((c<=KEY_F(12))&&(c>=KEY_F0))
            {
              if (FuncKeys[(c-264)].func)
                 {
                  FuncKeys[(c-264)].func();
                  update=1;
                 }
            }
#endif
         else
            {
             switch (c)
                    {
                     case KEY_BACKSPACE:
                          Delchar (1);
                          break;

                     case KEY_LEFT:
                          GoLeft();
                          break;

                     case KEY_RIGHT:
                          GoRight ();
                          break;

                     case KEY_UP:
                          HistoryUp();
                          break;

                     case KEY_DOWN:
                          HistoryDown();
                          break;  

                     case KEY_HOME:
                          GoLineStart ();
                          break;

                     case KEY_END:
                          GoLineEnd ();
                          break;

                     case KEY_PPAGE: 
                     case KEY_NPAGE:
                          break;

                     case KEY_IC:  /* insert-key 0513 */
                     case KEY_ENTER:
                          cc=1;
                          goto done;
                    }
             }
       }

    if (update)
     {
      RedrawInputWin();
      wait=1;

      return 0;
     }

   done:
    if (cc)
       {
        swmove (1, 71);
        whline_ (StatusWin, ' ', 1);

        i=(in_buf-start_win)+7;
        sswmove (3, (i=(in_buf-start_win)+7));
        whline_ (SstatusWin, ACS_HLINE, 1);

        dst_buf[in_buflen]='\0';
         if (in_buflen>0)
          {
           AddtoUserInputHistory (arrows, dst_buf, 1);
           //UpdateHistoryTable (0x15, arrows->nLines, '+'); 
          }

        CurHistLine=NULL; 
        LastTabLine=CurTabLine;
        CurTabLine=NULL;
    
        in_buflen=0;
        update=0;

        wait=0;

         if (hold_screen)
          {
           dequeuehold ();
          }

        start_win=in_buf=dst_buf=buf_i;
        end_win=start_win+visibleww-1;
        in_buf[buf_sz]='\0';

         if (buf_i[0])
          {
           extern void ParseUserInput (void);

            ParseUserInput ();
          }
         else
         if (mt_ptr->c_output&ANIMATE_PROMPT)
          {
           FancyOutput1 (InputWin, "<IRCIT> ", 0);
          }

        buf_i[0]=0;

        _wmove (InputWin, 0, 0);
        Fwprintf_nout (InputWin, "<IRCIT>");
        _wmove (InputWin, 0, 7);

        wclrtoeol (InputWin);

        wnoutrefresh (InputWin);
        wnoutrefresh (SstatusWin);
        wnoutrefresh (StatusWin);
        /* doupdate (); */
    
        return 1;
       } 

   return 0;

 }  /**/      


 static __inline__ FollowUp (void)

 {
  register int i;
  static chtype h1[]={[0]=196|A_ALTCHARSET,
                      [1]=25|A_ALTCHARSET,
                      [2]=196|A_ALTCHARSET
                     };
  static chtype h2[]={[0]=196|A_ALTCHARSET,
                      [1]=25|A_ALTCHARSET,
                      [2]=217|A_ALTCHARSET
                     };

   swmove (1, 71);
   swprintf ("%$%[%03d]",
             ((dst_buf+0)!=(start_win))?0x1D|A_ALTCHARSET|A_BLINK:32,
             in_buflen);

   swmove (0, 72);
   swprintf ("[%03d]", in_buf-dst_buf);

   i=(in_buf-start_win)+7;

   sswmove (3, i-1);
   (i+1==79)?
    waddstr__ (SstatusWin, h2, 3):
    waddstr__ (SstatusWin, h1, 3);

   wnoutrefresh (SstatusWin);

 }  /**/


 static __inline__ void NewCursorPos (char *new)

 {
  int i;

    if (new>end_win)
     {
      start_win=new;
      start_win-=visibleww*10/10;

      in_buf=new;
      end_win=start_win+visibleww-1;
     }    
    else 
    if (new<start_win)
     {
      start_win=new;

      start_win-=visibleww*10/10;

       if (start_win<dst_buf)
        {
         start_win=dst_buf;
        }

      in_buf=new;
        end_win=start_win+visibleww-1;
     } 
    else 
     {
      in_buf=new;
     }

    //if (!status_busy)
     {
      FollowUp ();
     }

 }  /**/


 static void Addchar (int c)

 {
  size_t n;  
  char *limit;

    if (in_buflen<buf_sz) 
     {
      limit=dst_buf+in_buflen;

       if (in_buf==limit)
        { 
         *in_buf=c;
         in_buflen++;
        } 
       else 
        {
         n=limit-in_buf;
         memmove (in_buf+1, in_buf, n);
         *in_buf=c;
         in_buflen++;
        }

      NewCursorPos (in_buf+1);

      update=1;
     } 
    else 
     {
      beep ();
     }

 }  /**/


 static void Delchar (int count)

 {
  size_t n;
  char *limit;

    if (count>(in_buf-dst_buf))
     {
      count=in_buf-dst_buf;
     }

    if (count) 
     {
      limit=dst_buf+in_buflen;
       if (in_buf!=limit) 
        {
         n=limit-in_buf;
         memcpy (in_buf-count, in_buf, n);
        }

      in_buflen-=count;
      NewCursorPos (in_buf-count);
      update=1;
     } 
    else 
     {
      /* beep (); */
     }

 }  /**/ 


 static void TabKey (void)

 {
    if (tab->nLines==0)
     {
      LastTabLine=CurTabLine=NULL;

      return;
     }

    if (CurTabLine!=NULL)
     {
      if (!CurTabLine->next)
       {
        CurTabLine=tab->head;
       }
      else
       {
        CurTabLine=CurTabLine->next;
       }
     }
    else 
     {
      LastTabLine?
       (CurTabLine=LastTabLine):
       (CurTabLine=tab->tail);
     }

    if (CurTabLine!=NULL)
     {
      swmove (1, 71);
      whline (StatusWin, ' ', 1);

      strncpy (dst_buf, CurTabLine->line, strlen(CurTabLine->line));
      UpdateHistoryTable (H_TABID, CurTabLine->counter, ' ');
      in_buflen=strlen (CurTabLine->line);
     
      GoLineEnd ();
     }

 }  /**/

 
 static void HistoryUp (void)

 {
    if (arrows->nLines==0)
     {
      CurHistLine=NULL;
     
      return;
     }

    if (CurHistLine!=NULL) 
     {
      if (!CurHistLine->prev)
       {
        CurHistLine=arrows->tail;
       }
      else
       {
        CurHistLine=CurHistLine->prev;
       }
     } 
    else 
     {
      CurHistLine=arrows->tail;
       if (CurHistLine==NULL) 
        {
         return;
        }
     }

    if (CurHistLine!=NULL) 
     {
      UpdateHistoryTable (H_LINEID, CurHistLine->counter, '+');
      strncpy (dst_buf, CurHistLine->line, strlen(CurHistLine->line)); 
      in_buflen=strlen (CurHistLine->line);

      GoLineEnd ();
     }

 }  /**/


 static void HistoryDown (void)

 {
    if (arrows->nLines==0)
     {
      CurHistLine=NULL;

      return;
     }

    if (CurHistLine!=NULL)
     {
      if (!CurHistLine->next)
       {
        CurHistLine=arrows->head;
       }
      else
       {
        CurHistLine=CurHistLine->next;
       }
     } 
    else 
     {
      CurHistLine=arrows->head;
       if (CurHistLine==NULL) 
        {
         return;
        }
     }

    if (CurHistLine!=NULL) 
     {
      UpdateHistoryTable (H_LINEID, CurHistLine->counter, '-');
      strncpy (dst_buf, CurHistLine->line, strlen(CurHistLine->line));
      in_buflen=strlen (CurHistLine->line);

      GoLineEnd ();
     }

 }  /**/


#include <prefs.h>

 extern int glob_cnt;

 void F1 (void)

 {
  int found=0,
      len=0,
      counter=0;
  char *d,
       cmnd[MINIBUF];
  ITCommand *in[glob_cnt];
  extern char *command_char_ptr;


    if ((!dst_buf[0])||(dst_buf[0]!=*command_char_ptr)||(in_buflen>MINIBUF-1))
     {
      say ("%$% Go play elsewhere!\n", MILD);

      return;
     }

   mstrncpy (cmnd, dst_buf+1, MINIBUF);
   d=cmnd;

    if (!strncasecmp(cmnd, "SET", 3)&&(cmnd[3]==' '))
     {
      register PrefsTable *aux,
                          *aux2=NULL;
      extern PrefsTable *preferences;

       d=cmnd;
       splitw (&d);

        if (!*d)
         {
          say ("%$% Go away now!\n", MILD);

          return;
         }

       len=strlen (d);

       aux=preferences;

        while ((aux)&&(!found)&&(counter<=27))
         {
           if (!strncasecmp(d, aux->entry.preference, len))
            {
             counter++;

              if (!strcasecmp(d, aux->entry.preference))
               {
                found=1;
                counter=1;
                aux2=aux;
               }
              else
               {
                aux2=aux;
               }
            }
                  
           if (strcasecmp(d, aux->entry.preference)<0)
            {
             aux=aux->left;
            }
           else
            {
             aux=aux->right;
            }
         }

        if ((aux2!=NULL)&&(counter==1))
         {
          sprintf (dst_buf, "/SET %s ", aux2->entry.preference);
          in_buflen=strlen (dst_buf);
          GoLineEnd ();

          return;
         }

        if (counter>1)
         {
          say ("%$% %d possible completions.\n", INFO, counter);

          return;
         }

        if ((!found)&&(counter==0))
         {
          beep ();

          return;
         }
     }
    else
    if (!strncasecmp(cmnd, "HELP", 4)&&(cmnd[4]==' '))
     {
      found++;
      splitw (&d);

       if (!*d)
        {
         say ("%$% Go away now!\n", MILD);

         return;
        }
     }

     counter=CompleteCommand (d, in, glob_cnt);

      if (counter==1)
       {
         if (found)
          {
           sprintf (dst_buf, "/HELP %s ", (*in)->name);
          }
         else
          {
           sprintf (dst_buf, "/%s ", (*in)->name);
          }

        in_buflen=strlen (dst_buf);
        GoLineEnd ();
       }
      else
      if (counter>1)
       {
        say ("%$% Found at least %d possible completions...\n", INFO, counter);

         for (len=0; len<counter; len++)
          {
           say (" %-19.19s", ((ITCommand *)*(in+len))->name);

            if ((len+1)%3==0)  say ("\n");
          }

         if (len%3!=0)  say ("\n");
       }
      else
       {
        beep();
       }
     
 }  /**/

   
 void F2 (void)

 {
   if (!*lastmsg_n())
    {
     say ("%$% No Private messages yet.\n", MILD);
    }
   else
    {
     LastmsgList (NULL, 0);
    }

 }  /**/


 void F4 (void)

 {
  extern  void ScrollingServers (void *, int);

   /* PlaySound(1); */
   ScrollingServers (NULL, 0);

 }  /**/


 static void BDelword (void)

 {
  register int count;
  register int off=in_buf-dst_buf-1;
        
   count=off;

    while ((off-->=0)&&(!isspace(dst_buf[off])))
          ;

   sswmove (3, 1);
   whline (SstatusWin, ACS_HLINE, 78);

   Delchar (count-=off);

 }  /**/


 static void FDelchar (void)

 {
  size_t n;
  register char *limit;

   while ((in_buflen>0)&&(in_buf<dst_buf+in_buflen))
    if (in_buflen>0) 
     {
      limit=dst_buf+in_buflen;

       if (in_buf==limit)
        {
         /* beep (); */
        } 
       else 
        {
         n=limit-in_buf-1;
         memcpy (in_buf, in_buf+1, n);
         --in_buflen;
         update=1;
        }
     } 
    else 
     {
      /* beep (); */
     }

 }  /**/


 static void GoLeft (void)

 {
    if (in_buf>dst_buf) 
     {
      NewCursorPos (in_buf-1);
      update=1;
      } 
    else 
     {
      /* beep (); */
     }

 }  /**/
 
 
 static void GoRight (void)

 {
    if (in_buf<(dst_buf+in_buflen)) 
     {
      NewCursorPos (in_buf+1);
      update=1;
     } 
    else 
     {
      /* beep (); */
     }

 }  /**/


 static void GoLineStart (void)

 {
   reset_cursor();

   NewCursorPos (dst_buf);
   update=1;
 
 }  /**/


 static void GoLineEnd (void)

 {
   reset_cursor();

   NewCursorPos (dst_buf+in_buflen);
   update=1;

 } /**/


 static void Delline (void)

 {
   in_buf=dst_buf;
   start_win=in_buf;
   end_win=start_win+visibleww-1;  
   in_buf[buf_sz]='\0';
   in_buflen=0;
   update=1;

   sswmove (3, 1);
   whline (SstatusWin, ACS_HLINE, 78);
   FollowUp ();

 } /**/


 int BindKey (int key, const char *fplexpr, const char *desc)

 {
  register FunctionKeys *fkptr;

   fkptr=&FuncKeys[key];

    if (fkptr->fplexpr)  free (fkptr->fplexpr), fkptr->fplexpr=NULL;
    if (fkptr->desc)  free (fkptr->desc), fkptr->desc=NULL;

   fkptr->fplexpr=strdup (fplexpr);

    if (desc)  fkptr->desc=strdup (desc);

   return 1;

 }  /**/


 int unBindKey (int key)

 {
    if (!FuncKeys[key].fplexpr)  return 0;

   free (FuncKeys[key].fplexpr), FuncKeys[key].fplexpr=NULL;
    if (FuncKeys[key].desc)  free (FuncKeys[key].desc), FuncKeys[key].desc=NULL;

   return 1;

 }  /**/


 /*
 ** /bind  F3 fplexpr
 */
 STD_IRCIT_COMMAND(uBINDKEY)

 {
  int n,
      undef=0;
  char *key;

    if (!args)
     {
      ShowKeyBindings (NULL);

      return;
     }

   key=splitw (&args);
    
    if (*key=='-')
     {
      undef=1;
      key++;
     }
 
    if ((n=strtoint(key)), ((n<0)||(n>MAX_F_KEYS))) 
     {
      say ("%$% Invalid key specified - %d.\n", MILD, n);

      return;
     } 

    if (undef)
     {
       if (!unBindKey(n))
        {
         say ("%$% %s wasn't bound.\n", MILD, FuncKeys[n].name);
        }
       else
        {
         say ("%$% Key-binding for %s cancelled.\n", INFO, FuncKeys[n].name);
        }

      return;
     }

    if (!args)
     {
      ShowKeyBindings (&FuncKeys[n]);

      return;
     }

   {
    BindKey (n, args, NULL);
   }

 }  /**/


 static void ShowThisKeyBinding (const FunctionKeys *fkptr)

 {
    if (strlen(fkptr->fplexpr)>20)
     {
      say (" %$179%-4.4s%$179%-15.15s%$179%-20.20s...%$179\n",
           fkptr->name, fkptr->desc?fkptr->desc:"    -     ",
           fkptr->fplexpr);   
     }
    else
     {
      say (" %$179%-4.4s%$179%-15.15s%$179%-20.20s   %$179\n",
           fkptr->name, fkptr->desc?fkptr->desc:"    -     ",
           fkptr->fplexpr);
     }

 }  /**/


 static void ShowKeyBindings (const FunctionKeys *fkptr)

 {
    if (!fkptr)
     {
      register int i;

       DisplayHeader (19, 3);
        for (i=0; i<=MAX_F_KEYS-1; i++)
         {
           if (FuncKeys[i].fplexpr)
            {
             ShowThisKeyBinding (&FuncKeys[i]);
            }
         }
       finish2(19, 3); 
     }
    else
     {
       if (!fkptr->fplexpr)
        {
         say ("%$% Key %s is not bound.\n", MILD, fkptr->name);

         return;
        }

      DisplayHeader (19, 3);
      ShowThisKeyBinding (fkptr); 
      finish2(19, 3);
     }

 }  /**/


 void InputWinOut (char *str)

 {
    if ((str)&&(*str))
     {
      register char *s=str;

        while (*s)
         {
          Addchar (*s++);
         }

       RedrawInputWin();
       doupdate ();
     }

 }  /**/


 void  InputWinClr (void)

 {
   Delline ();
   RedrawInputWin();

   doupdate ();

 }  /**/


