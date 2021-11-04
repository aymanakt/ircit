/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
*/

#include <main.h>
#include <thread.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <asm/unistd.h>

 MODULEID("$Id: thread.c,v 1.3 1998/02/15 11:27:29 ayman Beta $");


int new_thread (clonefunc fn, void *data, 
                unsigned int flags, void *stack, 
                unsigned int stacksize)
{
  long retval; 
  void **newstack;

   /* make the new stack: */
   newstack=(void **)stack;

   /* rearrange pointer so it's at the other end of the memory block */
   newstack=(void **)(stacksize+(char *)newstack);

   /* put the (void *)data on the new stack */
   *--newstack=data;

  __asm__ __volatile__(
    "int $0x80\n\t"
    "testl %0,%0\n\t"
    "jne 1f\n\t"
    "call *%3\n\t"
    "movl %2,%0\n\t"
    "int $0x80\n"
    "1:\t"
    :"=a" (retval)
    :"0" (__NR_clone),"i" (__NR_exit),
     "r" (fn),"b" (flags|SIGCHLD),"c" (newstack));

  if (retval<0) 
     {
      errno=(-retval);
      retval=(-1);
     }

  return retval;

 }  /**/

