/* 
** trans.c v1 (Sun Jun 28 1998)
** utility to convert ncurses attributes to produce file
** with ANSI escape sequences corresponding to those attributes
**
** 2 input files, dump.neg and dump.attr are used to produce
** the final output file, dump.ansi
** The format of the attribute-file:
** '49 0 0 0 0 60 2059 '
** COLOR_PAIR, A_BOLD, A_REVERSE, A_BLINK, RED, GREEN, BLUE
** This utility is specifically designed to work with IRCIT.
** Do not ask more!
** gcc -O -m486 trans.c -o trans
*/

#include <stdio.h>
#include <string.h>

#include "color_table.h"

 void ParseColor (char *, int *, int *, int *, int *, int *, int *);

 void main (void)

 {
  register int i, j;
  int bold, reverse, color, ccolor=-1, creverse=-1;
  int red, green, blue;
  char s[81], neg_file[100];
  FILE *fp, *fp2, *fp3;

    if (!(fp=fopen("dump.attr", "r")))
     {
      printf (" Unable to open dump.attr for reading.\n");

      return;
     }

    if (!(fp2=fopen("dump.neg", "r")))
     {
      printf (" Unable to open dump.neg for reading.\n");

      return;
     }

   fgets (neg_file, 20, fp2);

    if (!(fp3=fopen("dump.ansi", "w")))
     {
      printf (" Unable to open ansi-file for writing.\n");

      return;
     }

  fprintf (fp3, "(U\n");

   while (1)
    {
     fgets (neg_file, 100, fp2); /* line in pic-file */

      if (!feof(fp2))
       {
        for (i=0; i<81; i++)
         {
          fgets (s, 80, fp);

           if (!feof(fp))  /* entry in attr-file */
            {
             ParseColor (s, &bold, &reverse, &color, &red, &green, &blue);

              if (ccolor!=color||reverse!=creverse)  /* color-variation */
               {
                struct ColorTable *ptr=&table[color];

                 if (ptr->bg)
                  {
                    if (reverse)
                     {
                       if (ptr->inv)
                        {
                         struct ColorTable *ptr2;

                          ptr2=&table[ptr->inv];

                          fprintf (fp3, "[0m[1;%d;%dm[%dm%c", 
                                   bold?1:0, ptr2->fg, ptr2->bg, neg_file[i]);
                        } 
                     }
                    else
                     {
                      fprintf (fp3, "[0m[1;%d;%dm[%dm%c", 
                               bold?1:0, ptr->fg, ptr->bg, neg_file[i]);
                     }
                  }
                 else
                  {
                   fprintf (fp3, "[0m[1;%d;%dm%c", 
                            bold?1:0, ptr->fg, neg_file[i]);
                  }

                ccolor=color, reverse=creverse;
               }
              else
               {
                fprintf (fp3, "%c", neg_file[i]);
               }
            }
           else  break;
         }

        memset (neg_file, 100, 0);
       }
      else  break;
    }

 }  /**/


 void ParseColor (char *in, int *b, int *r, int *c, int *red, int *green, int *blue)

 {
  register int i;
  register char *aux;
  int *lot[6]={c, b, r, red, green, blue};

    for (i=1; i<=7; i++)
     {
       if ((aux=strchr(in, ' ')))
        {
         *aux=0;
         *lot[i-1]=atoi(in);
         in=++aux; 
        }
     }

 }  /**/

