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
#include <prefs.h>
#include <list.h>
#include <stack.h>
#include <windows.h>
#include <scroll.h>
#include <colors.h>
#include <colorstab.h>

 MODULEID("$Id: colors.c,v 1.1 1998/03/12 12:34:37 ayman Beta $");

 static int DrawColorMapBox (ColorMap *, Windows *);
 static void ColorsList (const PrefsTable *, register List *);
 static void DrawColorMap (ColorMap *);

 void ScrollingColorsEntries (ScrollingList *);

 int LocateColorDefinition (const char *bg, const char *fg, int *inv)

 {
  register int i, 
               j;

    for  (i=0; i<=6; i++)
     {
      if (!strcasecmp(*ColorsTable[i].bg, bg))
       {
        register const struct Foreground *fgptr;

         fgptr=ColorsTable[i].fg;

          for (j=0; j<=4; j++, fgptr++)
           {
            if (!strcasecmp(*fgptr->fg, fg))
             {
              *inv=ColorsTable[i].inv;

              return fgptr->value;
             }
           }

         break;
       }
     }

   return 0;  /* not found */

 }  /**/


 void _COLORS (Preference *pref, char *value)

 {
  int i, j, idx=-1;
  char s[LARGBUF];
  char *p=s;
  register char *aux;
  register ObjectsAndColors *objptr;

    if (!pref->value)
     {
      pref->value=strdup (value); 

      return;
     } 

   strcpy (s, pref->value);
   aux=splits (&p);

    if ((!p)||(!*aux))
     {
      free (pref->value);
      pref->value=strdup (value); 

      return;
     }
 
   TRIM(p);
    if (!*p) 
     {
      free (pref->value);
      pref->value=strdup (value);

      return;
     }

    if (!(i=LocateColorDefinition(aux, p, &j)))
     {
      free (pref->value);
      pref->value=strdup (value);

      return;
     }

    if (!strcmp(pref->preference, "COLOR_MENUBAR")) idx=0;
    else if (!strcmp(pref->preference, "COLOR_MENUBOX")) idx=1;
    else if (!strcmp(pref->preference, "COLOR_MENUBORDER")) idx=2;
    else if (!strcmp(pref->preference, "COLOR_LIST")) idx=3;
    else if (!strcmp(pref->preference, "COLOR_LISTBORDER")) idx=4;
    else if (!strcmp(pref->preference, "COLOR_LISTMENU")) idx=5;
    else if (!strcmp(pref->preference, "COLOR_LISTMENUBORDER")) idx=6;
    else if (!strcmp(pref->preference, "COLOR_DIALOGBOX")) idx=7;
    else if (!strcmp(pref->preference, "COLOR_DIALOGBORDER")) idx=8;
    else if (!strcmp(pref->preference, "COLOR_BOX")) idx=9;
    else if (!strcmp(pref->preference, "COLOR_BOXBORDER")) idx=10;
    else if (!strcmp(pref->preference, "COLOR_TABLES")) idx=17;

    if (idx<0)
     {
      return;
     }

   objptr=&obj_ptr[idx];

   objptr->color=i;
   objptr->inv=j;

   sprintf (pref->value, "%s,%s", aux, p);
   pref->level=objptr->color;

 }  /**/


 void _COLOR_LOWERSTATUS (Preference *pref, char *value)

 {
  int i, j, idx=-1;
  char s[LARGBUF];
  char *p=s;
  register char *aux;
  register ObjectsAndColors *objptr;

    if (!pref->value)
     {
      pref->value=strdup (value);

      return;
     }

   strcpy (s, pref->value); 
   aux=splits (&p);

    if ((!p)||(!*aux))
     {
      free (pref->value);
      pref->value=strdup (value);

      return;
     }

   TRIM(p);
    if (!*p) 
     {
      free (pref->value);
      pref->value=strdup (value);

      return;
     }

    if (!(i=LocateColorDefinition(aux, p, &j)))
     {
      free (pref->value); 
      pref->value=strdup (value);

      return;
     }

    if (!strcmp(pref->preference, "COLOR_LOWERSTAT"))
     {
      idx=11;

      objptr=&obj_ptr[idx];
      objptr->color=i;
      objptr->inv=j;

      /* refresh */
     }
    else
    if (!strcmp(pref->preference, "COLOR_STATBORDER"))
     {
      idx=12;

      objptr=&obj_ptr[idx];
      objptr->color=i;
      objptr->inv=j;

      /* refresh */
     }
    else
    if (!strcmp(pref->preference, "COLOR_LOWERSTATTXT"))
     {
      idx=13;

      objptr=&obj_ptr[idx];
      objptr->color=i;
      objptr->inv=j;
     }
    else  /* unknown pref */
     {
      return;
     }

   sprintf (pref->value, "%s,%s", aux, p);
   pref->level=objptr->color;

 }  /**/


 void _COLOR_UPPERSTATUS (Preference *pref, char *value)

 {
  int i, j, idx=-1;
  char s[LARGBUF];
  char *p=s;
  register char *aux;
  register ObjectsAndColors *objptr;

    if (!pref->value)
     {
      pref->value=strdup (value);

      return;
     }

   strcpy (s, pref->value);
   aux=splits (&p); 

    if ((!p)||(!*aux)) 
     {
      free (pref->value);
      pref->value=strdup (value);

      return;
     }

   TRIM(p);
    if (!*p) 
     {
      free (pref->value);
      pref->value=strdup (value);

      return;
     }

    if (!(i=LocateColorDefinition(aux, p, &j)))
     {
      free (pref->value); 
      pref->value=strdup (value);

      return;
     }

   idx=14;

   objptr=&obj_ptr[idx];
   objptr->color=i;
   objptr->inv=j;

   sprintf (pref->value, "%s,%s", aux, p);
   pref->level=objptr->color;

   RedrawUpperStatus ();

 }  /**/


 void _COLOR_MAINBORDER	(Preference *pref, char *value)

 {
  int i, j, idx=-1;
  char s[LARGBUF];
  char *p=s;
  register char *aux;
  register ObjectsAndColors *objptr;

    if (!pref->value)
     {
      pref->value=strdup (value);

      return;
     }

   strcpy (s, pref->value);
   aux=splits (&p);

    if ((!p)||(!*aux))
     {
      free (pref->value);
      pref->value=strdup (value);

      return;
     }

   TRIM(p);
    if (!*p) 
     {
      free (pref->value);
      pref->value=strdup (value);

      return;
     }

    if (!(i=LocateColorDefinition(aux, p, &j))) 
     {
      free (pref->value); 
      pref->value=strdup (value);

      return;
     }

    if (!strcmp(pref->preference, "COLOR_MAINBORDER"))
     {
      idx=15;

      objptr=&obj_ptr[idx];
      objptr->color=i;
      objptr->inv=j;
     }
    else
    if (!strcmp(pref->preference, "COLOR_MAINBORDERTXT"))
     {
      idx=16;

      objptr=&obj_ptr[idx];
      objptr->color=i;
      objptr->inv=j;
     }
    else
     {
      say ("%$% Unknow preference.\n", HOT);

      return;
     }

   sprintf (pref->value, "%s,%s", aux, p);
   pref->level=objptr->color;

 }  /**/


 static void ColorsList (const PrefsTable *ptr, register List *lptr)

 {
  ListEntry *eptr;

    if (ptr)
     {
      ColorsList (ptr->left, lptr);

       if (!strncmp(ptr->entry.preference, "COLOR_", 6))
        {
         eptr=AddtoList (lptr);

         (Preference *)eptr->whatever=&ptr->entry;
        }
	
      ColorsList (ptr->right, lptr);
     }

 }  /**/


 #include <colors-data.h>

 void SetupColorScheme (void *xptr, int idx)

 {
    if (!colors_list.list)
     {
      const size_t sizeofList=sizeof(List);
      List *ptr;
      extern PrefsTable *preferences;

       xmalloc(ptr, sizeofList);
       memset (ptr, 0, sizeofList);

       ColorsList (preferences, ptr);

       colors_list.nEntries=ptr->nEntries;
       (List *)colors_list.list=ptr;
     }

   LoadScrollingList (&colors_list);

 }  /**/


 void ScrollingColorsEntries (ScrollingList *sptr)

 {
  int i,
      j=0;
  int maxy,
      last;
  register ListEntry *eptr;
  WINDOW *w=RetrieveCurrentWindow();
  WINDOW *w1=RetrieveCurrentParentWindow();
  #define _entry ((Preference *)(eptr->whatever))->preference
  #define c_value \
   ((Preference *)((ListEntry *)sptr->c_item)->whatever)->value

   getmaxyx (w, maxy, last);
   last=maxy+sptr->listwin_start;

    for (eptr=((List *)sptr->list)->head; eptr!=NULL; eptr=eptr->next)
     {
      if (j==sptr->listwin_start)
       {
        break;
       }
      else
       {
        j++;
       }
     }

    for (i=sptr->listwin_start; (i<last)&&(i<sptr->nEntries);
         i++, eptr=eptr->next)
     {
       if (i==sptr->hilited)
        {
         _wattron (w, A_REVERSE);
         (ListEntry *)sptr->c_item=eptr;
        }

      _wmove (w, i-sptr->listwin_start, 0);
      Fwprintf  (w, " %-16.16s ", _entry);

       if (i==sptr->hilited)
        {
         _wattroff (w, A_REVERSE);

         {
          register int x;

           x=1+((w1->_maxy-strlen(c_value))/2);
           _wmove (w1, 1, 1);

           Fwprintf (w1, "%$196,*[%s]%$196,*", x, c_value, 
                     ((w1->_maxy+4)-(x+strlen(c_value)+2)));
         }
        }
     }

 }  /**/


 extern Stack *const stack;

 #define c_cmptr \
   ((ColorMap *)((InputProcessor *)(stack->top->whatever))->whatever)
 #define c_x c_cmptr->cx
 #define c_y c_cmptr->cy
 #define c_attr c_cmptr->cattr

 void ColorsListEnter (void *ptr, int idx)

 {
  #define __pref ((Preference *)((ListEntry *)ptr)->whatever)

   (Preference *)colormap.ptr=(Preference *)__pref;

   LoadColorMap (&colormap);

 }  /**/

 #undef __pref


 void LoadColorMap (ColorMap *cptr)

 {
  register Windows *wins;

    switch (LoadStack(cptr, NULL, GetColorMapInput, NULL, 
                      (int (*)(void *, Windows *))DrawColorMapBox,
                      7, 0, FLGS_MSGBOX))
     {
      case 3:
           LoadPrevLevel ();
           break;
     }

 }  /**/


 static __inline__ void DrawColorMap (ColorMap *cptr)

 {
  const char *s="                ";
  register int y, x;
  register const struct Foreground *fgptr;
  WINDOW *w=RetrieveCurrentWindow();
  char p[30];
  #define HINV ACS_HLINE|A_DIM
  #define VINV ACS_VLINE|A_DIM

    for (y=0; y<=6; y++)
     {
      fgptr=ColorsTable[y].fg;

       for (x=0; x<=4; x++)
        { /* what's going on here? */
         wmove (w, (y*2)+2, (x*2)+6);
         waddch (w, 111|A_ALTCHARSET|COLOR_PAIR(fgptr->value));
         y==c_y?(whline(w, ACS_HLINE, 1)):(whline(w, HINV, 1));
         wmove (c_inw1, (y*2)+2, (-1)+6);
         y==c_y?(whline(w, ACS_HLINE, 1)):(whline(w, HINV, 1));

         wmove (w, (y*2)+3, (x*2)+6);
         x==c_x?(whline(w, ACS_VLINE, 1)):(whline(w, VINV, 1));
         wmove (c_inw1, (-1)+2, (x*2)+6);
         x==c_x?(whline(w, ACS_VLINE, 1)):(whline(w, VINV, 1));

          if ((x==c_x)&&(y==c_y))
           {
            c_attr=fgptr->value;
            sprintf (p, "%s,%s", *ColorsTable[y].bg, *fgptr->fg);
           }

         fgptr++;
        }
     } 

   wmove (w, (c_y*2)+2, (c_x*2)+6);
   whline (w, 9|A_ALTCHARSET|COLOR_PAIR(c_attr), 1);

   wmove (w, 0, 1);

   wattron (w, COLOR_PAIR(c_attr));
   Fwprintf (w, s);

   wmove (w, 0, 1+((w->_maxy-strlen(p))/2));
   Fwprintf (w, p);
   wattroff (w, COLOR_PAIR(c_attr));

   wnoutrefresh (w);
   doupdate ();

 }  /**/


 static int DrawColorMapBox (ColorMap *cptr, Windows *wins)

 {
  register Windows *auxwins=wins;
  struct dummy {
                int hight,
                    width,
                    starty,
                    startx;
               };
  static struct dummy d={17, 18, 2, 25};
 

    if (!(NewWindow(d.hight, d.width, &d.starty, &d.startx, auxwins, 0)))
     {
      return 0;
     }

   box (auxwins->w1, ACS_VLINE, ACS_HLINE);

   DrawColorMap (cptr);

   return 1;    

 }  /**/


 void ColorMapEnter (ColorMap *cptr)

 {
  const register struct Foreground *fgptr;
  #define c_pref ((Preference *)cptr->ptr)
 
   fgptr=&ColorsTable[cptr->cy].fg[cptr->cx];

    if (fgptr->value==c_pref->level)
     {
      return;
     }
    else 
     {
      char p[30];
      char *aux;

       sprintf (p, "%s,%s", *ColorsTable[cptr->cy].bg, *fgptr->fg);

       aux=c_pref->value;

       c_pref->value=strdup(p);

       c_pref->func(c_pref, aux);

        if (aux)  free (aux);
     }

 }  /**/


 int GetColorMapInput (int key)

 {
  extern int Gets (int);

    if (key<KEY_MAX)
       {
        switch (key)
               {
                case 27:
                   LoadPrevLevel ();
                   /* CleanupListfromArray (colors_list.list); */

                   wnoutrefresh (MainWin);
                   doupdate ();
                   
                   return key;

                case KEY_UP:
                      if (--c_y<0) c_y=6;
                     break;

                case KEY_DOWN:
                      if (++c_y>6) c_y=0;
                     break;

                case KEY_LEFT:
                      if (--c_x<0) c_x=4;
                     break;

                case KEY_RIGHT:
                      if (++c_x>4) c_x=0;
                     break;

                /*case 0x18:
                     NextPrev (); 
                     break;*/

                case '\t':
                     break;

                case '\n':
                case '\r':
                     ColorMapEnter (c_cmptr);
                     return 0;

                default:
                     Gets (key);
                     return key;
               }

          DrawColorMap (NULL);
        }

   return key;
  
 }  /**/


 STD_IRCIT_COMMAND(uCOLORS)

 {
   SetupColorScheme (NULL, 0);

 }  /**/

