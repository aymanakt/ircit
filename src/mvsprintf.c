/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
** from old kernel source with modifications...
*/

#include <main.h>
#include <terminal.h>
#include <updates.h>
#include <mvsprintf.h>
#include <latin1.h>

 MODULEID("$Id: mvsprintf.c,v 1.5 1998/04/06 06:13:44 ayman Beta $");


 #define is_digit(c) ((c)>='0'&&(c)<='9')

 __inline__ int skip_atoi(const char **s)

 {
  register int i=0;

    while (is_digit(**s))  i=i*10+*((*s)++)-'0';

   return i;

 }  /**/


#define ZEROPAD	1		/* pad with zero */
#define SIGN	2		/* unsigned/signed long */
#define PLUS	4		/* show plus */
#define SPACE	8		/* space if plus */
#define LEFT	16		/* left justified */
#define SPECIAL	32		/* 0x */
#define SMALL	64		/* use 'abcdef' instead of 'ABCDEF' */

 #define do_div(n,base) ({ \
 int __res; \
  __asm__("divl %4":"=a" (n),"=d" (__res):"0" (n),"1" (0),"r" (base)); \
  __res; })

 unsigned long *number (unsigned long *str, 
                        int num, 
                        int base, 
                        int size, 
                        int precision, 
                        int type, 
                        int attr)
 {
  int i;
  char c, 
       sign, 
       tmp[36];
  const char *digits="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

    if (type&SMALL)  digits="0123456789abcdefghijklmnopqrstuvwxyz";
    if (type&LEFT)  type&=~ZEROPAD;

    if (base<2||base>36)  return 0;

   c=(type&ZEROPAD)?('0'|attr):' ';

    if ((type&SIGN)&&(num<0)) 
     {
      sign='-';
      num=-num;
     } 
    else 
     {
      sign=(type&PLUS)?'+':((type&SPACE)?' ':0);
     }

    if (sign)  size--;

    if (type&SPECIAL)
     {
      if (base==16)  size-=2;
      else 
      if (base==8)  size--;
     }

   i=0;

    if (num==0)  tmp[i++]='0';
    else while (num!=0)
		tmp[i++]=digits[do_div(num,base)];
	if (i>precision) precision=i;
	size -= precision;
	if (!(type&(ZEROPAD+LEFT)))
		while(size-->0)
			*str++=' ';
	if (sign)
		*str++=sign|attr;
	if (type&SPECIAL)
		if (base==8)
			*str++='0'|attr;
		else if (base==16) {
			*str++='0'|attr;
			*str++=digits[33]|attr;
		}
	if (!(type&LEFT))
		while(size-->0)
			*str++=c|attr;
	while(i<precision--)
		*str++='0'|attr;
	while(i-->0)
		*str++=tmp[i]|attr;
	while(size-->0)
		*str++=' ';
	return str;

 }  /**/


 int mvsprintf (unsigned long *buf, const char *fmt, va_list args)

 {
  int len;
  int i;
  int attr=0;
  static int colr;
  unsigned long *str,
                *p;
  char *s;
  int *ip;

  int flags;	
  unsigned long fig, d;
  int wdth, ok;

  int field_width;	      /* width of output field */
  int precision;	      /* min. # of digits for integers; max
			         number of chars for from string */
  int qualifier;      	      /* 'h', 'l', or 'L' for integer fields */
  extern int colors[];

    for (str=buf; *fmt; ++fmt) 
     {
       if (*fmt!='%') 
        {
         ADD1(*fmt);
         continue;
        }
			
      flags=0;

      repeat:
      ++fmt;

      switch (*fmt) 
       {
        case '-': 
              flags|=LEFT; 
              goto repeat;
	case '+': 
              flags|=PLUS; 
              goto repeat;
        case ' ': 
              flags|=SPACE; 
              goto repeat;
        case '#': 
              flags|=SPECIAL; 
              goto repeat;
        case '0': 
              flags|=ZEROPAD; 
              goto repeat;
       }
		
      field_width=-1;

       if (is_digit(*fmt)) 
        {
         field_width=skip_atoi (&fmt);
        }
       else 
       if (*fmt=='*') 
        {
         field_width=va_arg (args, int);

	  if (field_width<0) 
           {
	    field_width=-field_width;
	    flags|=LEFT;
	   }
	}

      precision=-1;

       if (*fmt=='.') 
        {
         ++fmt;	
          if (is_digit(*fmt))  
           {
            precision=skip_atoi (&fmt);
           }
	  else 
          if (*fmt=='*') 
           {
	    precision=va_arg (args, int);
	   }

	  if (precision<0)  precision=0;
	}

      qualifier=-1;

       if ((*fmt=='h')||(*fmt=='l')||(*fmt=='L')) 
        {
         qualifier=*fmt;
         ++fmt;
        }

      switch (*fmt) 
       {
        case 'c':
	       if (!(flags&LEFT))  while (--field_width>0)  *str++=' ';
	      *str++=((unsigned char) va_arg(args, int))|attr;
	       while (--field_width>0)  *str++=' ';
	      break;

	case 's':
	      s=va_arg (args, char *);
	      len=strlen (s);

	       if (precision<0)  precision=len;
	       else 
               if (len>precision)  len=precision;

	       if (!(flags&LEFT))  while (len<field_width--)  *str++=' ';

	       for (i=0; i<len&&(*s); ++i) 
                {
                 ADD2(*s); 
                 s++;
                }

	       while (len<field_width--)  *str++=' ';
	      break;

        case 'z':
             {
              unsigned long *z;

               p=va_arg (args, unsigned long *);
               len=0;
               z=p;

                while (*z++!=0)  len++;

                if (precision<0)  precision=len;
                else
                if (len>precision)  len=precision;

                if (!(flags&LEFT))  while (len<field_width--)  *str++=' ';

                for (i=0; i<len; ++i)  *str++=*p++|attr;

                while (len<field_width--)  *str++=' ';

               break; 
              }

	/*case 'o':
	        str=number(str, va_arg (args, unsigned long), 8,
		field_width, precision, flags);

	        break;*/

	case 'p':
	       if (field_width==-1) 
                {
		 field_width=8;
		 flags|=ZEROPAD;
		}

	      str=number (str, (unsigned long) va_arg(args, void *), 16,
			  field_width, precision, flags, attr);
	      break;
                  
        case '~':
              attr=0;
              break;

        case 'x':
	      flags|=SMALL;
	case 'X':
	      str=number (str, va_arg(args, unsigned long), 16,
			  field_width, precision, flags, attr);

	      break;

        case 'd':
	case 'i':
	      flags|=SIGN;
	case 'u':
	      str=number (str, va_arg(args, unsigned long), 10,
			  field_width, precision, flags, attr);

	      break;

	case 'n':
	      ip=va_arg(args, int *);
	      *ip=(str-buf);

	      break;

        case 'D':
              ip=va_arg (args, int *);
              str=number (str, *ip, 10,
                          field_width, precision, flags, attr);

              break;

        /*case 'A':
                       if ((*(fmt+1)=='+')||(*(fmt+1)=='-')
                          {
                           unsigned long a;

                            ++fmt;
                            a=va_arg (args, unsigned long);

                             if (*fmt=='+')
                                {
                                }
                             else
                                {
                                }

                           break;
                          }
                       else
                          {
                           --fmt;
                           break;
                          }*/

                 case '$':
                       if (is_digit(*(fmt+1)))
                          {
                           ++fmt;
                           fig=skip_atoi (&fmt);
                           ok=1;
                          }
                       else
                       if (*(fmt+1)=='%')
                          {
                           ++fmt; ++fmt; ok=1;
                           fig=va_arg (args, unsigned long);
                          }
                       else
                          {
                           --fmt;
                           break;
                          }

                       *str++=fig|A_ALTCHARSET|attr;

                        if (*fmt==',')
                           {
                             if (is_digit(*(fmt+1)))
                               {
                                ++fmt;
                                 wdth=skip_atoi(&fmt); --fmt;

                                  while (--wdth>0) *str++=fig|A_ALTCHARSET|attr;
                                 }
                              else
                              if (*(fmt+1)=='*')
                                 {
                                  ++fmt;
                                  wdth=va_arg (args, int);

                                   while(--wdth>0) *str++=fig|A_ALTCHARSET|attr;
                                 }
                              else
                              if (*(fmt+1)=='&')
                                 {
                                  ++fmt;
                                  ip=va_arg (args, int *);

                                   while(--*ip>0) *str++=fig|A_ALTCHARSET|attr;
                                 }
                              else
                                 {
                                   if (ok) --fmt;
                                 }
                           }
                        else
                           {
                             if (ok)  --fmt;
                           }

                       break;
                       
		 default:
		       if (*fmt!='%')  *str++='%'|attr;

		       if (*fmt)  *str++=*fmt|attr;
		       else --fmt;

		      break;
		}
	}

   *str='\0';

   return str-buf;

 }  /**/

