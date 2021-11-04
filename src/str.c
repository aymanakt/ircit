/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
*/

#include <main.h>

 MODULEID("$Id: str.c,v 1.8 1998/04/08 08:54:11 ayman Beta $");


 inline void wipespaces (char *s)

 {
  #define whitespace(c) (((c)==32)||((c)==9)||((c)==13)||((c)==10))
  register char *p;

    for (p=s+strlen(s)-1; ((whitespace(*p))&&(p>=s)); p--)
        ;

   if (p!=s+strlen(s)-1)  *(p+1)=0;

   for (p=s; ((whitespace(*p)) && (*p)); p++)
       ;

   if (p!=s)  strcpy (s, p);

 }  /**/


 __inline__ char *tokenize (char **str, const char c)

 {
  register char *s, 
                *p;

    if ((!str)||(!*str))  return *str="";

   s=*str;

    while (*s==' ')  s++;

    if (p=strchr(s,  c))
     {
      *str=(*p++=0, p);

      return (char *)s;
     }

   *str=0;

   return (char *)s;

 }  /**/


 int ProcessEscaped (const char *str)

 {
    if (*str=='*')  return 1;
    if (*str=='#')  return 1;
    if (*str=='\\')  return 1;

   return 0;

 }  /**/


 int DetermineRange (char *s, int *x, int *y)

 {
  register char *aux;

   aux=tokenize (&s, '-');

    if (!s)
     {
      s=aux;
        
      return 0;
     }

    if ((!isdigits(aux))||(!isdigits(s)))  return 0;

    if ((*x=atoi(aux))-(*y=atoi(s))>0)  return 0;

   return 1;

 }  /**/


 void otokenize (char *first, char *rest, char divider)

 {
  register char *p;

    if (!(p=strchr(rest, divider)))
     {
       if ((first!=rest)&&(first!=NULL))
        {
         strcpy (first, rest);
         rest[0]=0;
        }

      return;
     }

   *p=0; 

    if (first!=NULL)  strcpy (first, rest);
    if (first!=rest)  strcpy (rest, p+1);

 }  /**/


 int splitargss (const char *s, const char div)

 {
  static char src[LARGBUF];
  char *p;
  int i=0;
  extern char tok[][80];

   strcpy (src, s);

   memset (tok, 0, sizeof(char)*10*80);

    do
     {
      p=strchr (src, div);

       if (p==(char *)NULL)
        {
         mstrncpy (tok[i], src, 79);
         src[0]=0;
        }
       else
        {
         *p=0;
         mstrncpy (tok[i], src, 79);
         wipespaces(tok[i]);
         strcpy (src, p+1);
         wipespaces(src);
        }
     }
    while ((src[0])&&(++i<=9));

  return i+1;

 }  /**/


 int splitargs2 (const char *s, const char div)

 {
  static char src[LARGBUF];
  register char *p,
                *q=src,
                *r;
  int i=-1;
  extern char tok[][80];

   memset (tok, 0, sizeof(char)*10*80);

    if (!s)  return 0;

   strcpy (src, s);

    while ((*q)&&(++i<10))
     {
       while ((*q)&&(isspace(*q)))  q++;

      p=strchr (q, div);

       if (p==(char *)NULL)
        {
          for (r=q+strlen(q)-1; ((isspace(*r))&&(r>=q)); r--)
              ;
         *(r+1)=0;

         mstrncpy (tok[i], q, 79);
         *q=0;
        }
       else
        {
         *p=0;
          for (r=q+strlen(q)-1; ((isspace(*r))&&(r>=q)); r--)
              ;
         *(r+1)=0;

         mstrncpy (tok[i], q, 79);
         q=p+1;
        }
     }

    if (i>=10)
     {
      return 9;
     }
    else
     {
      return i;
     }
  
 }  /**/


 void fixfrom (char *s)

 {
  char from[150];
  
    if (!strchr(s, '@')) 
     {
      return;
     }

   strcpy (from, s);
 
    if (strchr("~+-^=", from[0])!=NULL) 
     {
      strcpy (from, &from[1]);
     }

   sprintf (s, "%s", from);

 }  /**/


 char *strcpyd (char *str)

 {
  char *str2;
  
   str2=(char *)malloc (strlen(str)+1);

    if (str2!=NULL)
     {
      strcpy (str2, str);
     }

   return (str2);

 }  /**/


 char *strcpyfd (char *str)

 {
  char *str2;

   str2=(char *)alloca (strlen(str)+1);

    if (str2!=NULL)
     {
      strcpy (str2, str);
     }

   return (str2);

 }  /**/


 char *wipespace (char *str)
 
 {
  char *s;

    if (!*str)
        return str;
   s=str+strlen(str)-1;
    while (isspace(*s))
          s--;
  s[1]=0;
  return str;

 }  /**/


 char *strtolower (char *s)

 {
  register char *p;

   p=s;

    for ( ; *s; s++)
     {
      if (isupper(*s))
       {
        *s=tolower(*s);
       }
     }

   return p;

 }  /**/


 char *strtoupper (char *s)

 {
  register char *p;

   p=s;

    for (; *s; s++)
     {
      if (islower(*s))
       {
        *s=toupper(*s);
       }
     }

   return p;

 }  /**/


 unsigned long atoul (char *addr)

 {
  unsigned long ip=0L;

   if (!addr)  return 0L;

    while ((*addr>='0')&&(*addr<='9'))
     {
      ip=(ip*10)+(*addr++)-'0';
     }

   return ip;

 }  /**/


 int strtoint (char *s)

 {
  register int x=0;
  register char *p=s;

    do
     {
      if (isdigit(*p))
       {
        x=(x*10)+(*p)-'0';
       }
     }
    while (*p++);

   return x;

 }  /**/


 char *mbasename (char *fname)

 {
  register char *p=strrchr(fname, '/');

   return (p?p+1:fname);

 }  /**/


 int hashstr (const char *s)

 {
  register int i=0;
  register const unsigned char *t=s;

    while (*t)
     {
      i+=isupper(*t)?tolower(*t++):*t++;
     }
  
   return abs(i%HASHSIZE);

 }  /**/


typedef unsigned long chtype;

 __inline__ int chstrlen (const chtype *s)

 {
  register const chtype  *p;

    if (!s)  return 0;

    for (p=s; *p; p++)
        ;

  return p-s;

 }  /**/


 chtype *chstrcpy (chtype *d, chtype *s)

 {
  register chtype *n,
                  *o;

    if ((!d)||(!s))  return (chtype *)NULL;

   n=d;
   o=s;

   while (*o)  *n++=*o++; *n=0;

  return (chtype *)d;

 }  /**/


 chtype *chstrcat (chtype *d, chtype *s)

 {
  register chtype *n,
                  *o;

    if ((!d)||(!s))  return (chtype *)NULL;

   n=d;

    while (*n) n++;

    if (!n)  return (chtype *)NULL;

   o=s;

    while (*o)  *n++=*o++; 

   *n=0;

   return d;

 }  /**/


 chtype *chstrdup (const chtype *s)

 {
  chtype *p;
  register chtype *q;
  register const chtype  *m;
  int len;

    if (!s)  return (chtype *)NULL;

    if (!(len=chstrlen(s)))  return (chtype *)NULL;

    if (!(p=(chtype *)malloc((sizeof(chtype)*(len+1)))))  return (chtype *)NULL;

   q=p;
   m=s;

    while (len--)  *q++=*m++;  

   *(q)=0;

   return (chtype *)p;

 }  /**/

#undef chtype

