/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
**
MODULEID("$Id: thread.h,v 1.2 1998/02/20 14:58:44 ayman Beta $")
**
*/

 struct thread_params {
         void *stack;
         char ipc_path[_POSIX_PATH_MAX],
              *lpath;
         void *params;
        };
                      
/* flags to use when creating a new thread */
#define CSIGNAL         0x000000ff    /* signal mask sent at exit */
#define CLONE_MEM       0x00000100    /* set to share memory */
#define CLONE_FS        0x00000200    /* set to share "fs info" (?) */
#define CLONE_FILES     0x00000400    /* set to share open files */
#define CLONE_SIGHAND   0x00000800    /* set to share signal handlers */

typedef void (*clonefunc)(void *);

 void th_Connect (void *);
 int new_thread (clonefunc, void *, unsigned int, void *, unsigned int);

