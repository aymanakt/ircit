/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
**
MODULEID("$Id: exec.h,v 1.2 1998/02/20 14:58:44 ayman Beta $")
**
*/

 struct exec_params {
         int flags;
         pid_t pid;
         char *prog,
              *shell,
              *shell_args;
         void *redir;
        };
 typedef struct exec_params EXEC;

 void ProcessExec (Socket *, const char *);
 void uEXEC (char *);
 void ShowEXEC (const Someone *, int, int);
 void th_exec (void *);

