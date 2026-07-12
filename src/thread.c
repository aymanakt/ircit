/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
*/

#include <main.h>
#include <sched.h>
#include <thread.h>

#include <fcntl.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <sched.h>
#include <errno.h>

 MODULEID("$Id: thread.c,v 1.3 1998/02/15 11:27:29 ayman Beta $");

/* Trampoline: glibc clone() expects int(*)(void*); IRCIT uses void(*)(void*). */
struct clone_wrapper {
    clonefunc fn;
    void *data;
};

static int clone_trampoline(void *arg)
{
    struct clone_wrapper *w = (struct clone_wrapper *)arg;
    w->fn(w->data);
    _exit(0);
}

int new_thread (clonefunc fn, void *data,
                unsigned int flags, void *stack,
                unsigned int stacksize)
{
  long retval;
  void **newstack;
  struct clone_wrapper wrap;

   wrap.fn = fn;
   wrap.data = data;

   /* make the new stack: */
   newstack = (void **)stack;

   /* rearrange pointer so it's at the other end of the memory block */
   newstack = (void **)(stacksize + (char *)newstack);

   /* place the wrapper struct at the top of the new stack */
   newstack = (void **)((char *)newstack - sizeof(struct clone_wrapper));
   memcpy(newstack, &wrap, sizeof(struct clone_wrapper));

  retval = clone(clone_trampoline, newstack,
                 flags | SIGCHLD, (void *)newstack);

  if (retval < 0)
     {
      retval = -1;
     }

  return retval;

 }  /**/

