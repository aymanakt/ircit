/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
*/

#include <main.h>
#include <terminal.h>
#include <output.h>
#include <colors.h>
#include <dialogbox.h>
#include <windows-aux.h>

 MODULEID("$Id: dialogbox.c,v 1.1 1998/03/11 11:23:01 ayman Beta $");


 static int DrawDialogBox (DialogBox *const, Windows *);

 static void DrawFields (DialogBox *const);
 static void DisplayFields (DialogBox *);
 static void DialogNewCursorPos (Field *, register char *const);
 static void DialogInflateCont (Field *, int);
 static void DialogDelchar (Field *, int);
 static void DialogLeft (Field *);
 static void DialogRight (Field *);
 static void DialogHome (Field *);
 static void DialogEnd (Field *);
 static void DialogBoxEnter (DialogBox *const);
 static void Visualize (Field *);
 static void DialogNextField (DialogBox *const);


 #define geom fptr->geo

 #define cfw_start fptr->edit.fw_start
 #define cfw_end fptr->edit.fw_end

 #define cfw_buf fptr->edit.fw_buf

 #define cfw_buflen fptr->edit.buf_len

 #define cfw_bufsz fptr->params.size

 #define cfw_inbuf fptr->edit.buffer

 #define c_begy fptr->geo.starty

 #define cfw_width fptr->geo.length

 #define c_begx \
                fptr->geo.startx
/*
 #define c_begx \
                fptr->geo.startx+ \
                w1->_begx+2
*/

 static __inline__ void DialogInflateCont (Field *fptr, int c)

 {
  size_t n;
  register char *end;

    if (cfw_buflen<cfw_bufsz)
     {
      end=&cfw_inbuf[cfw_buflen];

       if (cfw_buf==end)
        {
         *cfw_buf=c;
         cfw_buflen++;

         DialogNewCursorPos (fptr, cfw_buf+1);
        }
       else
        {
         n=end-cfw_buf;

         memmove (cfw_buf+1, cfw_buf, n);
         *cfw_buf=c;
         cfw_buflen++;

         DialogNewCursorPos (fptr, cfw_buf+1);
        }
     }

 }  /**/


 static __inline__ void Visualize (Field *fptr)

 {
  register char *es;
  register char *s;
  register WINDOW *w=RetrieveCurrentWindow();
  WINDOW *w1=RetrieveCurrentParentWindow();

   es=&cfw_inbuf[cfw_buflen];
   *es='\0';

   wattrset (w, COLOR_PAIR(7));
   _wmove (w, c_begy, c_begx);

    for (s=cfw_start; s<es; s++)
     {
       if (s>cfw_end)  goto refresh_;

       if (fptr->params.echo==_echo)
        {
         (*s)<32?(waddch(w, ((unsigned char)(*s)+64)|A_REVERSE)):
                 (waddch(w, s==(cfw_buf-1)?(unsigned char)*s|A_BLINK:*s));
                           //(unsigned char)*s));
        }
       else
       if (fptr->params.echo==_astrisk)
        {
         waddch (w, '*');
        }
     }

   whline (w, ' ', (cfw_end-s)+1);

   refresh_:
   //_wmove (w, c_begy, c_begx+(cfw_buf-cfw_start)-1);
   //waddch (w, *(cfw_buf-1)|A_BOLD);

   wattrset (w, COLOR_PAIR(CLR_DIALOG));

   wnoutrefresh (w);

 }  /**/


 void LoadDialogBox (DialogBox *dptr)

 {
  register Windows *wins;

    switch (LoadStack(dptr, NULL, 
                      GetDialogBoxInput, DialogBoxCleanup, 
                      (int (*)(void *, Windows *))DrawDialogBox,
                      6, 0, FLGS_DIALOGBOX))
     {
      case 3:
           LoadPrevLevel ();
     }

 }  /**/


 /*
 ** hilight current field 
 */
 static void DisplayFields (DialogBox *const dptr)

 {
  register int i;
  register Field *fptr;
  WINDOW *w=RetrieveCurrentWindow();
  WINDOW *w1=RetrieveCurrentParentWindow();

    for (i=0; i<=(dptr->nEntries-1); i++)
     {
      fptr=&dptr->fptr[i];

      _wmove (w, geom.starty, geom.cstartx);//+w1->_begx);

       if (i==dptr->c_field)
        {
         _wattron (w, A_REVERSE);
        }

       if (fptr->flags&D_BUTTON)
        {
         Fwprintf (w, "<%s>", fptr->caption);
        }
       else
        {
         Fwprintf (w, "%s", fptr->caption);
        }

       if (i==dptr->c_field)
        {
         _wattroff (w, A_REVERSE);
        }
     }


    /* cursor position */
    if (fptr=&dptr->fptr[dptr->c_field], fptr->flags&D_INPUTFIELD)
     {
      _wmove (w, c_begy, 
              c_begx+(fptr->edit.fw_buf-fptr->edit.fw_start));
      /* curs_set (1); */
     }
    else
     {
      /* curs_set (0); */
     }

   wnoutrefresh (w1);
   wnoutrefresh (w);
   /* doupdate (); */

   {
    char s[200];

     if (fptr->flags&(1<<3))
      {
       i=0;
       strcpy (s, fptr->edit.fw_buf);
       memset (fptr->edit.fw_buf, 0, fptr->edit.buf_len);
       fptr->edit.buf_len=0;

        while (*(s+i))
         {
          DialogInflateCont (fptr, *(s+i++));
          Visualize (fptr);
         }

       fptr->flags&=~(1<<3);
      }
   }

 }  /**/


 static __inline__ void DrawFields (DialogBox *const dptr)

 {
  register int i;
           int len;
  register Field *fptr;
  WINDOW *w=RetrieveCurrentWindow();
  WINDOW *w1=RetrieveCurrentParentWindow();

    for (i=0; i<=(dptr->nEntries-1); i++)
     {
      fptr=&dptr->fptr[i];

      _wmove (w, geom.starty, geom.cstartx);//+w1->_begx);
      Fwprintf (w, fptr->caption);

       if (fptr->flags&D_INPUTFIELD)
        {
         _wmove (w,
                 geom.starty,
                /*strlen(fptr->caption)+*/geom.startx);//+w1->_begx+2);

         wattrset (w, COLOR_PAIR(7));
         len=geom.length;
          while (len--)  waddch (w, ' ');
         wattrset (w, COLOR_PAIR(CLR_DIALOG));

         fptr->edit.fw_start=fptr->edit.buffer;
         fptr->edit.fw_end=&fptr->edit.buffer[fptr->geo.length]-1;
         fptr->edit.fw_buf=fptr->edit.buffer;
             
          if ((fptr->flags&D_USECONTENTS)==0)
           {
            memset (fptr->edit.fw_buf, '\0', fptr->edit.buf_len);
            fptr->edit.buf_len=0;
           }
          else
           {
            fptr->edit.buf_len=strlen (fptr->edit.fw_buf);
           }
        }
       else
       if (fptr->flags&D_CHECKBOX)
        {
         _wmove (w,
                 geom.starty, (geom.startx+w->_begx)-2);
         wattrset (w, COLOR_PAIR(7));
         Fwprintf (w, "%$245 ");
         wattrset (w, COLOR_PAIR(CLR_DIALOG));
        }
     }

   dptr->c_field=0;

   DisplayFields (dptr);

 }  /**/


 static int DrawDialogBox (DialogBox *const dptr, Windows *wins)

 {
  register Windows *auxwins=wins;

    if (!(NewWindow(dptr->hight, dptr->width, &dptr->starty, 
                    &dptr->startx, auxwins, 0)))
     {
      return 0;
     }

    if (!(NewSubWindow(dptr->hight-3, dptr->width-2, dptr->starty+2, 
          dptr->startx+1,auxwins)))
     {
      return 0;
     }

   wattrset (auxwins->w1, COLOR_PAIR(CLR_DIALOGBORDER));
   box (auxwins->w1, ACS_VLINE, ACS_HLINE);

   _wmove (auxwins->w1, 0, 2);
   Fwprintf (auxwins->w1, "%$240%$180");
   _wmove (auxwins->w1, 0, 4);
   wattrset (auxwins->w1, COLOR_PAIR(CLR_DIALOG));
   Fwprintf (auxwins->w1, dptr->title);
   wattrset (auxwins->w1, COLOR_PAIR(CLR_DIALOGBORDER));
   _wmove (auxwins->w1, 0, strlen(dptr->title)+4);
   Fwprintf (auxwins->w1, "%$195%$240");

   wattrset (auxwins->w2, COLOR_PAIR(CLR_DIALOG));
   wbkgd (auxwins->w2, COLOR_PAIR (CLR_DIALOG));

   DrawFields (dptr);

   return 1;

  }  /**/


 static void DialogNewCursorPos (Field *fptr, register char *npos)

 {
    if (npos>cfw_end)
     {
      cfw_start=npos;

      cfw_start-=(cfw_width-1)*10/10;

      cfw_buf=npos;
      cfw_end=cfw_start+cfw_width-1;
     }
    else
    if (npos<cfw_start)
     {
      cfw_start=npos;

      cfw_start-=(cfw_width-1)*10/10;

       if (cfw_start<cfw_inbuf)
        {
         cfw_start=cfw_inbuf;
        }

      cfw_buf=npos;
      cfw_end=cfw_start+cfw_width-1;
     }
    else
     {
      cfw_buf=npos;
     }

 }  /**/


 static __inline__ void DialogDelchar (Field *fptr, int count)

 {
  size_t n;
  register char *end;

    if (count>(cfw_buf-cfw_inbuf))
     {
      count=cfw_buf-cfw_inbuf;
     }

    if (count)
     {
      end=&cfw_inbuf[cfw_buflen];

       if (cfw_buf!=end)
        {
         n=end-cfw_buf;

         memcpy (cfw_buf-count, cfw_buf, n);
        }

      cfw_buflen-=count;

      DialogNewCursorPos (fptr, cfw_buf-count);
     }

 }  /**/


 static __inline__ void DialogLeft (Field *fptr)

 {
   if (cfw_buf>cfw_inbuf)
    {
     DialogNewCursorPos (fptr, cfw_buf-1);
    }

  /* beep */

 }  /**/


 static __inline__ void DialogRight (Field *fptr)

 {
    if (cfw_buf<&cfw_inbuf[cfw_buflen])
     {
      DialogNewCursorPos (fptr, cfw_buf+1);
     }

 }  /**/


 static __inline__ void DialogHome (Field *fptr)

 {
   DialogNewCursorPos (fptr, cfw_inbuf);

 }  /**/


 static __inline__ void DialogEnd (Field *fptr)

 {
   DialogNewCursorPos (fptr, &cfw_inbuf[cfw_buflen]);

 }   /**/


 static __inline__ void DialogNextField (DialogBox *dptr)

 {
    if (++dptr->c_field>dptr->nEntries-1)
     {
      dptr->c_field=0;
     }

  DisplayFields (dptr);

 }  /**/


 static __inline__ void DialogBoxEnter (DialogBox *const dptr)

 {
   LoadPrevLevel ();

   wnoutrefresh (MainWin);
   doupdate ();

    if (dptr->dialog_enter)
     {
      (*dptr->dialog_enter)(dptr);
     }

 }  /**/ 


 int GetDialogBoxInput (int key)

 {
  register DialogBox *dlgptr=Hello();
  register Field *fptr=&dlgptr->fptr[dlgptr->c_field];

    if (fptr->flags&D_BUTTON)
     {
       if (key=='\n')
        {
         DialogBoxEnter (dlgptr);

         return 0;
        }
       else
       if (key=='\t')
        {
         DialogNextField (dlgptr);
        }

      return key;
     }
    else
    if (key<KEY_MIN)
       {
         if (key<=31)
            {
             switch (key)
                    {
                     case '\b':
                          DialogDelchar (fptr, 1);
                          break;

                     case '\n':
                     case '\r':
                          DialogBoxEnter (dlgptr);
                          return 0;

                     case '\t':
                          DialogNextField (dlgptr); 
                          return key;
                          //break;

                     case 27:
                          LoadPrevLevel ();
                          return 1;

                     case 0x18:
                          NextPrev ();
                          break;

                     default:
                          DialogInflateCont (fptr, key);
                          break;
                    }
            }
         else
            {
             DialogInflateCont (fptr, key);
            }
       }
    else
    if (key<KEY_MAX)
       {
        switch (key)
               {
                case KEY_LEFT:
                     DialogLeft (fptr);
                     break;

                case KEY_RIGHT:
                     DialogRight (fptr);
                     break;

                case KEY_BACKSPACE:
                     DialogDelchar (fptr, 1);
                     break;

                case KEY_HOME:
                     DialogHome (fptr);
                     break;

                case KEY_END:
                     DialogEnd (fptr);
                     break;

                case KEY_PPAGE:
                case KEY_NPAGE:
                     break;

                case KEY_ENTER:
                     DialogBoxEnter (dlgptr);
                     break;

               default:
                     Gets (key);
                     return 1;
               }
       }

   Visualize (fptr);

   return 1;

 }  /**/


 void DialogBoxCleanup (void *ptr)

 {
   /* curs_set (0); */

 }  /**/


#if 0
 DialogBox *CreateDialogBox (const void *ptr)

 {
  const size_t sizeofDialogBox=sizeof(DialogBox);
  DialogBox *auxdxptr;

    if (!((void *)auxdxptr=RetrievePosted(6, ptr))) /*don't trust strangers */ 
       {
        xmalloc(auxdxptr, sizeofDialogBox);
        memset (auxdxptr, 0, sizeofDialogBox);

        auxdxptr->flags|=1;

        return (DialogBox *)auxdxptr;
       }
    else
       {
        LoadDialogBox (auxscrptr); /* we do the chore */

        return NULL;
       }

 }  /**/
#endif

