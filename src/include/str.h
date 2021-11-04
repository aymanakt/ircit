/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
**
MODULEID("$Id: str.h,v 1.5 1998/04/08 08:54:34 ayman Beta $")
**
*/

#define HASHSIZE 79

#define isspecial(ch) (strchr("[]\\_-|`^{}", (ch))!=NULL)
#define wildcards(ch) (strchr("*!?", (ch))!=NULL)

#define FIXIT(x) \
 (*(x)=='~'||*(x)=='+'||*(x)=='-'||*(x)=='^'||*(x)=='=')?((x)++):((x))

#define TRIM(x) \
  if (x) \
     for ( ; (*(x))&&(isspace(*(x))); ++(x))

#define arethesame(x, y) (!strcasecmp((x), (y)))

#define mstrncpy(s, p, n) \
   strncpy (s, p, strlen(p)>n-1?n:strlen(p)+1); \
   s[n-1]='\0'

#define randomize(min, max) ((rand()%(int)(((max)+1)-(min)))+(min))

#define isdigits(x) ({ \
                      int numbered=1; \
                      register int n=strlen(x)-1; \
                                         \
                        while (n>=0) \
                         {  \
                           if ((x[n]<'0')||(x[n]>'9')) \
                            { \
                             numbered=0; \
                             break; \
                            } \
                          n--; \
                         } \
                                \
                       (numbered?1:0); \
                     })

 #define SPLIT_RMSPC(x, y, z) \
         otokenize ((x), (y), (z)); \
          if (*(y)) \
           { \
            for ( ; (*(y))&&(isspace(*(y))); ++(y)) \
                ; \
           }

 #define splitw(x) tokenize((x), ' ')
 #define splits(x) tokenize((x), ',')
 #define splitn(x) tokenize((x), '!')
 #define splitcolon(x) tokenize((x), ':')

 void otokenize (char *, char *, const char);
 int splitargs2 (const char *, const char);
 int splitargss (const char *, const char);
 char *tokenize (char **, const char);
 int DetermineRange (char *, int *, int *);
 void fixfrom (char *);
 char *strcpyd (char *);
 char *strcpyfd (char *);
 char *strncopy (char *, char *, size_t);
 char *strcatd (char *, char *);
 void *realloci (void *, size_t);
 int hashstr (const char *);
 void wipespaces (char *);
 char *wipespace (char *);
 char *trimspaces (char *);
 char *strtolower (char *);
 char *strtoupper (char *);
 int strtoint (char *);
 char *mbasename (char *);
 unsigned long atoul (char *);
 int ProcessEscaped (const char *) __attribute__ ((const));
 int chstrlen (const unsigned long *);
 unsigned long *chstrcpy (unsigned long *, unsigned long *);
 unsigned long *chstrcat (unsigned long *, unsigned long *);
 unsigned long *chstrdup (const unsigned long *);

