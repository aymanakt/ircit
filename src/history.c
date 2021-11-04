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
#include <prefs.h>
#include <history.h>

 static int ClearHistoryEntry (InputHistory *, int, void *);

 MODULEID("$Id: history.c,v 1.6 1998/06/12 07:33:09 ayman Beta $");

 InputHistory *arrows,
              *tab;

 void InitHistories (void)

 {
   InitUserInputHistory ();
   InitTabHistory ();
   InitServersList ();
   InitIRCNotify ();
   InitUTMPNotify ();
   InitIgnoreList ();

 }  /**/
   

 void InitUserInputHistory (void)

 {
  static InputHistory ArrowsHistory;

   ArrowsHistory.head=NULL;
   ArrowsHistory.tail=NULL;
   ArrowsHistory.ID=H_LINES;
   ArrowsHistory.nLines=0;

   arrows=&ArrowsHistory;

 }  /**/


 void InitTabHistory (void)

 {
  static InputHistory TabHistory;

   TabHistory.head=NULL;
   TabHistory.tail=NULL;
   TabHistory.ID=H_TABS;
   TabHistory.nLines=0;

   tab=&TabHistory;

 }  /**/


 __inline__ Input *RemovefromInputHistory (InputHistory *ptr, Input *input)
 
 {
  int i=1;
  register Input *next, 
                 *prev;

   next=input->next;
   prev=input->prev;

    if (ptr->head==input)
     {
      ptr->head=next;
     }
    if (ptr->tail==input)
     {
      ptr->tail=prev;
     }

    if (next!=NULL)
     {
      next->prev=prev;
     }
    if (prev!=NULL)
     {
      prev->next=next;
     }

   free (input);
   ptr->nLines--;

   UpdateHistoryTable (ptr->ID, 0, '-');

   prev=ptr->head;

    for ( ; prev!=NULL; prev=prev->next)
     {
      prev->counter=i++;
     }

   return (Input *)next;

 }  /**/


 Input *AddtoUserInputHistory (InputHistory *ptr, char *buf, int flag)

 {
  int sz, type;
  char *tmp;
  register Input *aux;

    if (ptr==arrows)  {type=H_LINES; tmp="ARROWS_HISTORY";}
    else  {type=H_TABS; tmp="TAB_HISTORY";}

    if ((sz=atoi(valueof(tmp))))
     {
       if (!flag)
        {
          if (InmyList(ptr, aux->line))
           {
            return NULL;
           }
        }

       if (ptr->nLines==sz)
        {
         RemovefromInputHistory (ptr, ptr->head);
        }

      xmalloc(aux, (sizeof(Input)));
      memset (aux, 0, sizeof(Input));

      strncpy (aux->line, buf, strlen(buf));
        
       if (ptr->head==NULL)
        {
         ptr->head=ptr->tail=aux; 
         aux->counter=1;
         ptr->nLines=1;
        }
       else
        {
         aux->prev=ptr->tail;  
         ptr->tail->next=aux;
         ptr->tail=aux;
         ptr->nLines++;
         aux->counter=aux->prev?aux->prev->counter+1:1;
        }

      UpdateHistoryTable (type, ptr->nLines, 0);

      return aux;
     }

   return NULL;

 }  /**/


 int InmyList (InputHistory *ptr, char *str)

 {
  int found=0;
  register Input *aux;

   aux=ptr->head;

    while ((aux!=NULL)&&(!found))
     {
      if (!strcasecmp(str, aux->line))
       {
        found=1;
       }
      else
       {
        aux=aux->next;
       }
     }

    if (!found)
     {
      return 0;
     }

   return 1;

 }  /**/


 static __inline__ int ClearHistoryEntry (InputHistory *ptr, int how, void *v)

 {
    if (ptr->nLines==0)
     {
      say ("%$% No entries found.\n", MILD);

      return;
     }

   switch (how)
    {
     case EVERYTHING:
          {
           int i=0;
           register Input *iptr=ptr->head,
                          *aux;

             while (iptr!=NULL)
              {
               i++;
               aux=iptr->next;
               free (iptr);
               iptr=aux;
              }

            memset (ptr, 0, sizeof(InputHistory));

            say ("%$% Cleared %d entr%s.\n", INFO, i, i>1?"ies":"y");

            return i;
          }

     case NAME:
          {
           char *name;
           register Input *iptr=ptr->head;

            name=(char *)v;

             for ( ; iptr!=NULL; iptr=iptr->next)
              {
               if (arethesame(name, iptr->line))
                {
                 RemovefromInputHistory (ptr, iptr);

                 return 1;
                }
              }

            say ("%$% Specified entry not found.\n", MILD);

            return 0;
          }

     case NUMBER:
          {
           int *m=0,
                n=0;
           register Input *iptr=ptr->head;

            m=(int *)v;

             if (*m>ptr->nLines)
              {
               say ("%$% You don't have that many entries.\n", MILD);

               return 0;
              }

             for ( ; iptr!=NULL; iptr=iptr->next)
              {
                if (++n==*m)
                 {
                  RemovefromInputHistory (ptr, iptr);

                  return 1;
                 }
              }

            return 0;
          }
    }

 }  /**/


 int AdjustHistorySize (InputHistory *inhptr, int n)

 {
  int sz=inhptr->nLines-n;

    if (n==0)
     {
      ClearHistoryEntry (inhptr, EVERYTHING, NULL);

      DisplayHistoryTable (inhptr==arrows?H_LINES_D:H_TABS_D, 0);
     }
    else
    if (inhptr->nLines>n)
     {
      register int diff=sz;

        while (diff--)
         {
          RemovefromInputHistory (inhptr, inhptr->head);

          UpdateHistoryTable (inhptr==arrows?H_LINES:H_TABS,
                              inhptr->nLines, 0);
         }
     }

   return 0;

 }  /**/


 void RemovefromHistory (char *str, int mode)

 {
  int i,
      j,
      esc=0;
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
              if (mode&2)
               {
                ClearHistoryEntry (arrows, EVERYTHING, NULL);

                ht_ptr->nLines=0;
                ht_ptr->c_lineid=0;
                UpdateHistoryTable (H_LINES, arrows->nLines, 0);
               }
              else
              if (mode&4)
               {
                ClearHistoryEntry (tab, EVERYTHING, NULL);

                ht_ptr->nTabs=0;
                ht_ptr->c_tabid=0;
                UpdateHistoryTable (H_TABS, tab->nLines, 0);
               }

             return;
            }
           else
           if ((!esc)&&(*aux=='#'))
            {
             int i=0;
             //char *p=tok[j]+1;

               if (!(i=atoi(aux+1))||(i<0))
                {
                 say ("%$% You really need -h.\n", MILD);

                 continue;
                }

               /*if ((!isdigits(p))||(!*p))
                {
                 say ("%$% You really need -h.\n", MILD);
                }
               else
                {
                 int i=atoi(p);*/

                   if (mode&2)
                    {
                     ClearHistoryEntry (arrows, NUMBER, &i); 
                    }
                   else
                   if (mode&4)
                    {
                     ClearHistoryEntry (tab, NUMBER, &i);
                    }
                //}

              continue;
            }
           else
            {
             plain_entry:
               if (mode&2)
                {
                 ClearHistoryEntry (arrows, NAME, aux);
                }
               else
               if (mode&4)
                {
                 char s[LARGBUF];

                  sprintf (s, "%s ", aux);
                  ClearHistoryEntry (tab, NAME, s);
                }

             continue;
            }
         }
     }

 }  /**/


 char *GetHistoryDir (const char *file, char *path)

 {
  char *upath;
   
   upath=valueof ("HISTORY_DIR");

    if (*file)
     {
       if (*file=='/')
        {
         snprintf (path, _POSIX_PATH_MAX, "%s", file);
        }
       else
        {
         if (upath)
          {
           snprintf (path, _POSIX_PATH_MAX, "%s/%s", upath, file);
          }
         else
          {
           snprintf (path, _POSIX_PATH_MAX, "%s/.ircit/%s", homedir(), file);
          }
        }
     }
    else
     {
      if (upath)
       {
        snprintf (path, _POSIX_PATH_MAX, "%s/history.txt", upath);
       }
      else
       {
        snprintf (path, _POSIX_PATH_MAX, "%s/.ircit/history.txt", homedir());
       }
     }  

   return (char *)path;

 }  /**/


#include <sys/stat.h>

 #define LoadHistoryFile(x, y, z) _LoadHistoryFile_ (0, x, y, z)
 #define PlayHistoryFile(x, y, z) _LoadHistoryFile_ (1, x, y, z)

  
 int _LoadHistoryFile_ (int p,InputHistory *hptr, const char *file, char *range)

 {
  register int i=1, j=0;
  int x=1, y=-1;
  char path[_POSIX_PATH_MAX]={0};
  FILE *fp;
  struct stat st;
  register Input *iptr=hptr->head;

    if (!(fp=fopen((GetHistoryDir(file, path)), "r")))
     {
      say ("%$% Unable to open History file (%s) - %s.\n",
           MILD, path, strerror(errno));

      return 0;
     }

    if ((stat(path, &st))<0)
     {
      say ("%$% Configuration file's missing! Reverting to defaults - %s.\n",
           HOT, strerror(errno));

      return 0;
     }

    if (st.st_size==0)
     {
      say ("%$% Unable to Load history file (%s) - file has zero length.\n",
           MILD, path);
     }

    if (*range)
     {
      if (!(DetermineRange(range, &x, &y)))
       {
        say ("%$% I can't handle this range.\n", MILD);

        return 0;
       }
     }

    if (!x)
     {
       if (!y)  return 0; /* 0-0 */
      x=1;
     }

    if (!(fp=fopen((GetHistoryDir(file, path)), "r")))
     {
      say ("%$% Unable to open History file (%s) - %s.\n", 
           MILD, path, strerror(errno));

      return 0;
     }

   {
    char line[st.st_size+1];
    register char *aux=line,
                  *aux2;
    char *ptrs[]={[0 ... 100]=""};

      if ((read(fileno(fp), line, st.st_size))<0)
       {
        say ("%$% Unable to read History file (%s) - %s.\n", MILD, 
             path, strerror(errno));

        return 0;
       }

     line[st.st_size-1]=0; 

      while ((j<=98)&&(aux2=strchr(aux, '\n')))  
       {
        *aux2=0, ptrs[j++]=aux, aux=++aux2;
       } 

     ptrs[j++]=aux;
      if (x>j)  x=j;
      if (y>j||y<0)  y=j;
      if (x>y)  {say ("%$% Conflict-of-Range detected.\n", MILD); return 0;}

      while (x<=y)
       {
         if (*ptrs[x-1])
          {
            if (strlen(ptrs[x-1])>512)  *(ptrs[x-1]+512)=0;
           AddtoUserInputHistory (hptr, ptrs[x-1], (hptr==arrows)?1:0);

            if (p)
             {
              extern char buf_i[];

               strcpy (buf_i, ptrs[x-1]);
               ParseUserInput ();
             }
          }  

        x++;
       }
   }

   fclose (fp);

    if (!p)
     {
      say ("%$% Loaded %d entr%s into Input History...\n", 
           INFO, j, j>1?"ies":"y");
     }

   return 1;

 }  /**/


 int SavetoHistoryFile (InputHistory *hptr, const char *file, char *range)

 {
  register int i=1;
  int x=1, y=hptr->nLines;
  char path[_POSIX_PATH_MAX]={0};
  FILE *fp;
  register Input *iptr=hptr->head;

    if (hptr->nLines==0)
     {
      say ("%$% No entries found.\n", MILD);

      return 0;
     }

    if (*range)
     {
      if (!(DetermineRange(range, &x, &y)))
       {
        say ("%$% Conflict-of-Range detected.\n", MILD);

        return 0;
       }
     }

    if (!x)
     {
       if (!y)  return; /* 0-0 */
      x=1;
     }

    if (!(fp=fopen((GetHistoryDir(file, path)), "w")))
     {
      say ("%$% Unable to open History file (%s) - %s.\n", 
           MILD, path, strerror(errno));

      return 0;
     }

    while ((i++)<x) 
     {
      iptr=iptr->next;
     }
   i--;

    do 
     {
      fprintf (fp, "%s\n", iptr->line);
      iptr=iptr->next;
     }
    while ((iptr!=NULL)&&(i++<y));
    
   fclose (fp);

   say ("%$% Saved %d History entr%s to %s...\n", 
        INFO, i-x, (i-x>1)?"ies":"y", path);

   return 1;

 }  /**/


 void uHISTORY (char *args)

 {
  register int add=0, j=0;
  int mode=0;
  char *what;
  extern char tok[][80];

    if (!args)
     {
      say ("%$% arguments invalid or incomplete.\n", MILD);

      return;
     }

   splitargs2 (args, ' ');

    if (arethesame(tok[j], "-ARR"))  mode|=(0x1<<1);
    else
    if (arethesame(tok[j], "-TAB"))  mode|=(0x1<<2);
    else
     {
      say ("%$% you really need -h.\n", MILD);

      return;
     }

    if (!(atoi(valueof(mode&2?"ARROWS_HISTORY":"TAB_HISTORY"))))
     {
      say ("%$% %s facility currently disabled.\n",
           MILD, mode&2?"ARROWS_HISTORY":"TAB_HISTORY");

      return;
     }

    if (mode&0x2)
     {
      j++;

       if (!*tok[j])
        {
         ShowInputHistory (arrows, (1-1), arrows->nLines);

         return;
        }
     }
    else
    if (mode&0x4)
     {
      j++;

       if (!*tok[j])
        {
         ShowInputHistory (tab, (1-1), tab->nLines);;

         return;
        }
     }
    else
     {
      say ("%$% you really need -h.\n", MILD);

      return;
     }

    if (*tok[j]=='-')  
     {
      add=0;
       if ((what=tok[j]+1), (*what=='\\'))  j++;
       else
        {
          if (arethesame(what, "SAVE"))
           {
            /* range should always be specified along with file name */
            /* otherwise the range will be interpreted as file name *
            /* and the default range will be assumed (1-nEntries) */
            SavetoHistoryFile (mode&2?arrows:tab, tok[j+1], tok[j+2]);

            return;
           }
          else
          if (arethesame(what, "LOAD"))
           {
            LoadHistoryFile (mode&2?arrows:tab, tok[j+1], tok[j+2]);

            return;
           }
          else
          if (arethesame(what, "PLAY"))
           {
            PlayHistoryFile (mode&2?arrows:tab, tok[j+1], tok[j+2]);

            return;
           }
          else
           {
            j++;  /* entry.. */
           }
        }
     }
    else
    if (*tok[j]=='+')  
     {
      add=1;
      what=tok[j]+1;
     }
    else
     { 
      int x, y;

        if (!(DetermineRange(tok[j], &x, &y)))
         {
          say ("%$% Conflict-of-Range detected.\n", MILD);

          return;
         }

      ShowInputHistory (mode&2?arrows:tab, (x-1), y);

      return;
     }

   what=splitw (&args), args?:(args=what); 

    if (add)
     {
      AddtoUserInputHistory (mode&2?arrows:tab, ++args, mode&2?1:0);
     }
    else
     {
      RemovefromHistory (++args, mode);
     }

 }   /**/


 void ShowInputHistory (InputHistory *ptr, int x, int y)

 {
  register int i=0;
  register Input *iptr;

    if (ptr->nLines==0)
     {
      say ("%$% No entries found.\n", MILD);

      return;
     }

   DisplayHeader (14, 2);

   iptr=ptr->head;

    while ((++i)<=x) 
     {
      iptr=iptr->next;
     }
   i--;

    for ( ; iptr!=NULL, ((++i)<=(y)); iptr=iptr->next)
     {
      if (strlen(iptr->line)>60)
       {
        say (" %$179%-3d%$179%-.57s...%$179\n", i, iptr->line);
       }
      else 
       {
        say (" %$179%-3d%$179%-60.60s%$179\n", i, iptr->line);
       }
     }

    finish2 (14, 2);

 }  /**/


 STD_IRCIT_PREF(_HISTORY_SIZE)

 {
  InputHistory *inhptr=NULL;

    if ((!pref->value)||((pref->value)&&(!atoi(pref->value))))
     {
       if (pref->value)  free (pref->value);

       pref->value=strdup ("0");
     }
    else
     {
       if (atoi(pref->value)>99)
        {
         free (pref->value);

         pref->value=strdup ("99");
        }
     }

    if (arethesame(pref->preference, "ARROWS_HISTORY"))
     {
      inhptr=arrows;
     }
    else
    if (arethesame(pref->preference, "TAB_HISTORY"))
     {
      inhptr=tab;
     }

    if (inhptr)
     {
      AdjustHistorySize (inhptr, atoi(pref->value));
     }

 }  /**/

