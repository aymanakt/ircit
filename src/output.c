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
#include <updates.h>
#include <list.h>
#include <queue.h>
#include <lastlog.h>
#include <output.h>

 static Lines *SplitLine (chtype *);

 MODULEID("$Id: output.c,v 1.10 1998/04/08 08:54:11 ayman Beta $");

 int nLines;
 int hold_screen;
 static unsigned long w_buf[LARGBUF];
 
 Queue *hold;
 dList *scr_back;

 const int colors[]={7, 7, 4, 2, 1, 1, 5, 1, 2, 6, 6, 6, 5, 4, 5, 3};

#define is7bits_(c) ((unsigned)(c)<128)
#define __NEED_WRAP 0x40
#define __NOCHANGE  -1
#define ismainwin() win==MainWin

#define DO_NEWLINE_     x=0; \
                        win->_flags&=~__NEED_WRAP; \
                        y++; \
                             \
                        nLines++; \
                         if ((nLines>win->_regbottom-1)&&(ismainwin())) \
                          { \
                            if (ISHOLD()) \
                             { \
                              hold_screen=1; \
                             } \
                           nLines=0; \
                          } \
                              \
                         if (y>win->_regbottom) \
                          { \
                           y--; \
                            if (win->_scroll) \
                             { \
                              scroll(win); \
                             } \
                          } \

 __inline__ int waddch_ (WINDOW *win, const chtype c)

 {
  register chtype ch=c;
  register int x, 
               y;

   x=win->_curx;
   y=win->_cury;

    if (ch&A_ALTCHARSET)  goto noctrl;

    switch (ch&A_CHARTEXT)
           {
            case '\n':
                 wclrtoeol (win);
                 DO_NEWLINE_
                 break;

            default:
            noctrl:
                  if (win->_flags&__NEED_WRAP)
                     {
                      DO_NEWLINE_
                     }

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

                  if (win->_line[y].text[x]!=ch)
                     {
                       if (win->_line[y].firstchar==__NOCHANGE)
                          {
                           win->_line[y].firstchar=win->_line[y].lastchar=x;
                          }
                       else
                       if (x<win->_line[y].firstchar)
                          {
                           win->_line[y].firstchar=x;
                          }
                       else
                       if (x>win->_line[y].lastchar)
                          {
                           win->_line[y].lastchar=x;
                          }
                     }

                 win->_line[y].text[x++]=ch; 

                  if (x>win->_maxx)
                   {
                    win->_flags|=__NEED_WRAP;
                    x--;
                   }

                 break;
           }

 finish:
        win->_curx=x;
        win->_cury=y;

 }  /**/


 __inline__ int waddstr__ (WINDOW *win, chtype *s, size_t n)

 {
  register chtype *p;
  short line;
  short start;
  short end;

   line=win->_cury;

    if ((start=win->_curx, end=(start+n-1))>(win->_maxx)) end=win->_maxx;

    if (win->_line[line].firstchar==_NOCHANGE||win->_line[line].firstchar>start)        win->_line[line].firstchar=start;

    if (win->_line[line].lastchar==_NOCHANGE||win->_line[line].lastchar<start)
        win->_line[line].lastchar=end;

   p=s+(n-1);

    while (end>=start/*&&p>=s*/)
     {
       if ((*p&A_CHARTEXT)==' ')
        {
         *p|=win->_bkgd;
        }
       else
       if (!(*p&A_ATTRIBUTES))
        {
         *p|=(win->_bkgd&A_ATTRIBUTES);
        }

      *p|=win->_attrs;

      win->_line[line].text[end]=*p--;
      end--;
     }

 }  /**/


 static __inline__ Lines *SplitLine (chtype *str)

 {
  #define COL_ 77

  static Lines output_[]=
         {
          {NULL, 0}, {NULL, 0}, {NULL, 0}, {NULL, 0},
          {NULL, 0}, {NULL, 0}, {NULL, 0}, {NULL, 0},
          {NULL, 0}, {NULL, 0}, {NULL, 0}, {NULL, 0},
          {NULL, 0}, {NULL, 0}, {NULL, 0}, {NULL, 0},
          {NULL, 0}, {NULL, 0}, {NULL, 0}, {NULL, 0}
         };

  static chtype buffer[LARGBUF];
 
  chtype spc[]={[0]=' '|(CURWIN()->_bkgd&A_ATTRIBUTES)};
  register chtype *ptr;
  chtype  c,
          mc,
          *mstr;
  int i=0,
      pos=0,
      col=0,
      line=0,
      wbreak=0;

   memset (buffer, 0, sizeof(chtype)*LARGBUF);

    for ( ; i<20; i++) 
     {
      if (output_[i].line)
       {
        free (output_[i].line);
        output_[i].line=NULL;
        output_[i].len=0;
       }
     }

    if (!*str)  str=chstrdup (spc);

    for (ptr=(chtype *)str; *ptr; ptr++)
        {
         switch (*ptr&A_CHARTEXT)
                {
                 case ' ':
                      wbreak=pos;
                      buffer[pos++]=*ptr;
                      col++;
                      break;

                 case '\n':
                       if (col<=COL_)
                          {
                           buffer[pos++]=*ptr;
                           buffer[pos]=0;
                           output_[line].line=chstrdup (buffer);
                           output_[line++].len=pos;
                           pos=col=wbreak=0;
                           continue;
                          }

                 default:
                      buffer[pos++]=*ptr;
                      col++;
                      break;
                }

          if (col>=COL_)
             {
               if (wbreak==0) 
                {
                 c=buffer[col-1];
                 buffer[col-1]='\n'; buffer[col]=0; 
                 mstr=&buffer[col+1];
                 output_[line].line=chstrdup(buffer);
                 output_[line++].len=col;
                 buffer[0]=buffer[1]=spc[0]; buffer[2]=c; col=pos=3;

                  while (*mstr)
                   {
                    buffer[pos++]=*mstr++;
                    col++;
                   }

                 continue;
                }

              c=buffer[wbreak];
              mc=buffer[wbreak+1];

              buffer[wbreak]='\n';
              buffer[wbreak+1]=0;

              output_[line].line=chstrdup(buffer);
              output_[line++].len=wbreak+1;

              mstr=&buffer[wbreak+2];
              pos=col=3;
              buffer[0]=spc[0]; buffer[1]=c; buffer[2]=mc; wbreak=0;

               while (*mstr)
                { 
                 buffer[pos++]=*mstr++; 
                 col++;
                }
 
              col--; pos--; ptr--;
             }
        }

   buffer[pos]=0;

    if (buffer[0])
     {
      output_[line].line=chstrdup (buffer);
      output_[line++].len=pos;
     }

   return &output_[0];

 }  /**/


 /*
 ** shouldn't be called from signal-handler
 */ 
 int nYell (const char *format, ...)

 {
  va_list va;
  register Lines *lptr;

   memset (w_buf, 0, LARGBUF);

   va_start (va, format);
   mvsprintf (w_buf, format, va);
   va_end (va);

    for (lptr=SplitLine(w_buf); lptr->line; lptr++)
     {
       if (hold_screen)
        {
         AddtoHold (lptr->line, lptr->len);
        }
       else
        {
         register chtype *q=lptr->line;

           while (*q)  waddch_ (CURWIN(), *q++);

          wUpdate (CURWIN());
        }
     }

 }  /**/


 chtype *PreSay (const char *format, ...)

 {
  va_list va;

   memset (w_buf, 0, sizeof(chtype)*LARGBUF);

   va_start (va, format);
   mvsprintf (w_buf, format, va);
   va_end (va);

   return w_buf;

 }  /**/


 int nYelll (chtype *chs)

 {
  register Lines *lptr;

    for (lptr=SplitLine(chs); lptr->line; lptr++)
     {
       if (hold_screen)
        {
         AddtoHold (lptr->line, lptr->len);
        }
       else
        {
         register chtype *q=lptr->line;

           while (*q)  waddch_ (CURWIN(), *q++);

          wUpdate (CURWIN());
        }
     }

 }  /**/


 int lYell (int logidx, void *ptr, int (*func)(int, void *, chtype *),
            chtype *chs)

 {
  va_list va;
  register Lines *lptr;

   /*memset (w_buf, 0, LARGBUF);

   va_start (va, format);
   mvsprintf (w_buf, format, va);
   va_end (va);*/

   (*func)(logidx, ptr, chs); 

    for (lptr=SplitLine(chs); lptr->line; lptr++)
     {
       if (hold_screen)
        {
         AddtoHold (lptr->line, lptr->len);
        }
       else
        {
         register chtype *q=lptr->line;

           while (*q)  waddch_ (CURWIN(), *q++);

          wUpdate (CURWIN());
        }
     }

 }  /**/


 int chsprintf (chtype *buf, const char *format, ...)

 {
  int len;
  va_list va;

   va_start (va, format);
   len=mvsprintf (buf, format, va);
   va_end (va);

   return len;

 }  /**/


 int Fwprintf (WINDOW *w, const char *format, ...)

 {
  int i,
      len;
  va_list va;
  static chtype s[80];

   memset (s, 0, 80);

   va_start (va, format);
   len=mvsprintf (s, format, va);
   va_end (va);

   waddstr__ (w, s, len);

   wnoutrefresh (w);

   return len;

 }  /**/


 int Fwprintf_nout (WINDOW *w, const char *format, ...)

 {
  int i,
      len;
  va_list va;
  chtype s[80];

   va_start (va, format);
   len=mvsprintf (s, format, va);
   va_end (va);

   waddstr__ (w, s, len);

   return len;

 }  /**/


 int wprintf (WINDOW *w, const char *format, ...)

 {
  int n;
  va_list va;
  chtype wbuf[LARGBUF];
  register chtype *s=wbuf;

   va_start (va, format);
   n=mvsprintf (wbuf, format, va);
   va_end (va);

    while (*s)  waddch_ (w, *s++);

   wnoutrefresh (w);

   return n;

 }  /**/


 int scrlprintf (WINDOW *w, const char *format, ...)

 {
  int n;
  va_list va;
  static chtype scrl_buf[80];
  register chtype *s=scrl_buf;

   va_start (va, format);
   n=mvsprintf (scrl_buf, format, va);
   va_end (va);

   wnoutrefresh (w);

   return n;

 }  /**/


 void FancyOutput (WINDOW *window, char *s, int y, int x, int z)
                     
 {
  register int i, 
               j;
  int maxy, 
      maxx;
  int len;

   len=strlen(s)-1;
 
   getmaxyx (window, maxy,maxx);

    if (y>maxy)  y=maxy;
    if (x>maxx)  x=maxx;
    if (z>maxy)  z=maxy;
 
    for (i=0; i<=len; i++)
     {
      for (j=x; j>=(i+1)+z; j--)
       {
        mvwaddch (window, y, j, *(s+i)|COLOR_PAIR(j%8)|A_BOLD);
        mvwdelch (window, y, j-1);
        wrefresh (window);
       }
     }

 } /**/


void FancyOutput2 (WINDOW   *w,
                    chtype  *s,
                    int     line,
                    int     col,
                    int     flags)

 {
  register int i,
               j=0,
               k=1;
  int len,
      maxy,
      maxx;
  typedef struct {int pos; chtype ch;} loc;
  loc betu[80];

   len=chstrlen(s)-1;

   getmaxyx (w, maxy, maxx);
    if (line>maxy) line=maxy;
    if (col>maxx)  col=maxx;

    for (i=col; i>=0; i--) 
     {
      betu[i].pos=i;
      betu[i].ch=*(s+0);

      _wmove (w, line, betu[i].pos);
      waddch_ (w, betu[i].ch);

       for (j=i+1; j<=col; j++)
        {
         betu[j].pos=j;

          if (++k<=len)
           {
            betu[j].ch=*(s+k);
            _wmove (w, line, betu[j].pos);
            waddch_ (w, betu[j].ch);
           }

         wclrtoeol (w);
        }

      k=0;
      (flags&TAKE_IT_EASY)?napms(150):delay_output(50);

      wrefresh (w);
     }

    if (flags&EAT_IT)
     {
      _wmove (w, line, 0);

      j=chstrlen (s);

       for (i=0; i<=j; i++)
        {
         wdelch (w);
         (flags&TAKE_IT_EASY)?napms(150):delay_output(50);
         wrefresh (w);
        }
     }

 } /**/


#if 0
 void ExplodeWindow (WINDOW *win, int how)

 {
  register int i, 
               j;
  int nlines,
      ncols;
  int nline, 
      ncol;
  register WINDOW *w=win,
                  *ptr;
  typedef struct coordinates {
                  int lines,
                      cols;
                  int y,
                      x;
                 } coordinates;
  coordinates wins[13];

   memset (wins, 0, 13*sizeof(coordinates));

    /* what's going on here? */
   nlines=((w->_maxy+1)-((w->_maxy+1)%2))/2;
   nline=(w->_maxy+1)+2;
   ncols=((w->_maxx+1)-((w->_maxx+1)%2))/2;
   ncol=(w->_maxx+1)+2;

    for (i=1; (((ncol-=2)>1)&&(i<=nlines)); i++)
     {
      wins[i].lines=nline-=2;
      wins[i].cols=ncol;
      wins[i].y=(i-1)+w->_begy;
      wins[i].x=(i-1)+w->_begx;
     }

    if (how&EXPLODE_IN)
    for (j=1; j<=i-1; j++)
     {
      if ((ptr=newwin(wins[j].lines, wins[j].cols, wins[j].y, wins[j].x)))
       {
        box (ptr, ACS_VLINE, ACS_HLINE);
        wrefresh (ptr); 
        delwin (ptr);
        napms(1);//delay_output (50);
       }
     }

    if (how&EXPLODE_OUT)
    for (j=i-1; j>=1; j--)
     {
      if ((ptr=newwin(wins[j].lines, wins[j].cols, wins[j].y, wins[j].x)))
       {
        box (ptr, ACS_VLINE, ACS_HLINE);
        wrefresh (ptr);
        delwin (ptr);
        napms(1);delay_output (50);
       }
     }  

 }  /**/
#endif


#if 0
 void H_ExplodeWindow (WINDOW *w)

 {
  register int i;
  register WINDOW *ptr;
  int ncols=w->_maxx+1;
  int nlines=w->_maxy+1;

    for (i=1; i<ncols+1; i++)
     {
      if ((ptr=newwin(nlines, i, w->_begy, w->_begx)))
       {
        ptr->_attrs=w->_attrs;
        box (ptr, ACS_VLINE, ACS_HLINE);
        wrefresh (ptr);
        delwin (ptr);
        napms (10);
       }
     }

 }  /**/


 void V_ExplodeWindow (WINDOW *w)

 {
  register int i;
  register WINDOW *ptr;
  int ncols=w->_maxx+1;
  int nlines=w->_maxy+1;

    for (i=1; i<nlines+1; i++)
     {
      if ((ptr=newwin(i, ncols, w->_begy, w->_begx)))
       {
        ptr->_attrs=w->_attrs;
        box (ptr, ACS_VLINE, ACS_HLINE);
        wrefresh (ptr);
        delwin (ptr);
        napms (10);
       }
     }

 }  /**/


 #define mwmvaddch(y, x, z) \
         wmove (win, (y), (x)); \
         whline (win, (z), 1)

 void Box (WINDOW *win, int h, int w, int y, int x)

 {
  register int _x,
               _y;


    if ((w>2)&&(h>2))
     {
      mwmvaddch(y, x, ACS_ULCORNER);
      mwmvaddch(y, x+w-1, ACS_URCORNER);
      mwmvaddch(y+h-1, x, ACS_LLCORNER);
      mwmvaddch(y+h-1, x+w-1, ACS_LRCORNER);

       for (_x=x+1; _x<x+w-1; _x++)
        {
         mwmvaddch(y, _x, ACS_HLINE);
         mwmvaddch(y+h-1, _x, ACS_HLINE);
        }

       for (_y=y+1; _y<y+h-1; _y++)
        {
         mwmvaddch(_y, x, ACS_VLINE);
         mwmvaddch(_y, x+w-1, ACS_VLINE);
        }
     }

 }  /**/
#endif


 void DisplayBanner (int i)

 {
#if 0
  struct Ban {
              int i;
              char *banner;
             };
  struct Ban Banners [] =
  {
   {1, "!"},
   {2, "New mail has arrived!"}
  };
   status_busy=1;
   FancyOutput2(StatusWin, Banners[i].banner, 0, 79, EAT_IT);
   status_busy=0;
#ifdef _HAVE_PTHREAD
   pthread_exit ((void *)1);
#endif
#endif
 }  /**/


 void InitHoldQueue (void)

 {
  static Queue HoldQueue;

   HoldQueue.front=NULL;
   HoldQueue.rear=NULL;
   HoldQueue.nEntries=0;

   hold=&HoldQueue;

 }  /**/


 int AddtoHold (unsigned long *b, int n)

 {
  register QueueEntry *qptr;

   qptr=AddQueue (hold);

   memcpy (qptr->tx, b, sizeof(unsigned long)*(n+1));
   qptr->len=n;
   qptr->tx[n+1]=0;

   swmove (0, 30);
   swprintf (" - [%$% (%d)]           ", 23, hold->nEntries);

   doupdate();

 }  /**/


 int dequeuehold (void)

 {
  register int len=0,
               n=0;
  chtype *s;
  QueueEntry *qptr;

    if (hold->nEntries==0)
     {
      hold_screen=0;

      return;
     }

    while (hold->nEntries)
     {
      n++;

       if (n>16)  break;
       else
        {
         qptr=deQueue (hold);
         s=qptr->tx;                

          while (qptr->len--)  waddch (CURWIN(), *s++);

         free (qptr);
        }
     }

    if (hold->nEntries==0) 
     {
      swmove (0, 30);
      swprintf (" - [%$% (0)]            ", 23|COLOR_PAIR(55));
      hold_screen=0;
     }
    else
     {
      swmove (0, 30);
      swprintf (" - [%$23 (%d)]              ", hold->nEntries);
     }

   wnoutrefresh (CURWIN());
   doupdate();

 }  /**/


 extern STD_IRCIT_COMMAND(uFLUSH);

 STD_IRCIT_COMMAND(uFLUSH)

 {
  register QueueEntry *qptr; 

    if (hold->nEntries)
     {
      say ("%$% Flushing screen buffer ...\n", INFO); /* you'll not see it */

       while (hold->nEntries)
        {
         qptr=deQueue (hold);
         free (qptr);

         swmove (0, 30);
         swprintf (" - [%$23 (%d)]              ", hold->nEntries);
         doupdate();
        }
     }

 }  /**/


 #include <prefs.h>

 void F5 (void)

 {
  char s[strlen("HOLD_MODE")+4];

    if (ISHOLD())
     {
      strcpy (s, "HOLD_MODE OFF");
     }
    else
     {
      strcpy (s, "HOLD_MODE ON");
     }

   uSET (s);

 }  /**/


 void HOLD_MODE (Preference *, char *);

 void HOLD_MODE (Preference *pref, char *value)

 {
    if (arethesame(pref->value, "OFF"))
     {
       if (ISHOLD())
        {
         UNHOLDSCREEN();
         hold_screen=0;

          if (hold->nEntries)
           {
            register int n=0;
            register chtype *s;
            QueueEntry *qptr;

             while (hold->nEntries)
              {
               n++;
               qptr=deQueue (hold);
               s=qptr->tx;

                while (qptr->len--)  waddch (CURWIN(), *s++);

               free (qptr);

                if (n==16)
                 {
                  n=0;
                  swmove (0, 30);

                  swprintf (" - [%$23 (%d)]        ", hold->nEntries);
                  wnoutrefresh (CURWIN());
                  doupdate();
                 }
              }
           }

         _wattron (StatusWin, COLOR_PAIR(55));
         swmove (0, 30);
         swprintf (" - [%$23 (0)]       ");
         _wattroff (StatusWin, COLOR_PAIR(55));
         _wattron (StatusWin, COLOR_PAIR(49));
        }
     }
    else
    if (arethesame(pref->value, "ON"))
     {
      if (!ISHOLD())
       {
        HOLDSCREEN();

        swmove (0, 30);
        swprintf (" - [%$% (0)]       ", 23|COLOR_PAIR(55));
       }
     }

 }  /**/

