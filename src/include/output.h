/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
**
MODULEID("$Id: output.h,v 1.6 1998/03/16 10:09:38 ayman Beta $")
**
*/

  struct Lines {
          chtype  *line;
          int len;
         };
 typedef struct Lines Lines;


 #define mvwaddch_(win, y, x, ch) \
         (wmove(win, y, x)==ERR?ERR:waddch_(win, ch))

 #define waddstr_(win, str) waddnstr((win), (str), -1)
 
 extern __inline__ void _wattron (WINDOW *const win, const attr_t at)
 {
   if (PAIR_NUMBER(at)>0)
    {
     win->_attrs=(win->_attrs&(~(A_COLOR)))|at;
    }
   else
    {
     win->_attrs|=at;
    }
 }

 extern __inline__ void _wattroff (WINDOW *const win, const attr_t at)
 {
   if (FALSE==TRUE) 
    {
      if (PAIR_NUMBER(at)==0xff)
       {
        win->_attrs&=~at;
       }
      else
       {
        win->_attrs&=~(at&(~(A_COLOR)));
       }
    } 
   else 
    {
      if (PAIR_NUMBER(at)>0x00)
       {
        win->_attrs&=~(at|A_COLOR);
       }
      else
       {
        win->_attrs&=~at;
       }
    }
 }

 extern __inline__ int _wmove (WINDOW *const win, const int y, const int x)
 {
   if ((x<=win->_maxx)&&(y<=win->_maxy))
    {
     win->_curx=(short)x;
     win->_cury=(short)y;

     win->_flags&=~0x40;
     win->_flags|=_HASMOVED;

     return 1;
    }
   else
    {
     return -1;
    }

 }

 int nYelll (chtype *);
 int nYell (const char *, ...);// __attribute__ ((format (printf, 1, 2)));
 int lYell (int, void *, int (*)(int, void *, chtype *), chtype *);
 chtype *PreSay (const char *, ...);
 int chsprintf (chtype *, const char *, ...);// __attribute__ ((format (printf, 2, 3)));

 int wprintf (WINDOW *, const char *, ...);// __attribute__ ((format (printf, 2, 3)));
 int Fwprintf (WINDOW *, const char *format, ...);// __attribute__ ((format (printf, 2, 3)));
 int Fwprintf_nout (WINDOW *, const char *format, ...);
 int scrlprintf (WINDOW *, const char *, ...);

 void FancyOutput (WINDOW *, char *, int, int, int);
 void FancyOutput2 (WINDOW *, chtype *, int, int, int);
// void ExplodeWindow (WINDOW *, int);
// void H_ExplodeWindow (WINDOW *);
// void Box (WINDOW *, int, int, int, int);
 void DisplayBanner (const int);

 int waddch_ (WINDOW *, const chtype);
 int waddstr__ (WINDOW *, chtype *, size_t);

 void InitHoldQueue (void);
 int AddtoHold (chtype *, int);
 int dequeuehold (void);
 void F5 (void);

