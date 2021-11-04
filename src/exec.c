/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
*/

#include <main.h>
#include <terminal.h>
#include <updates.h>
#include <prefs.h>
#include <list.h>
#include <scroll.h>
#include <people.h>
#include <sockets.h>
#include <dcc.h>
#include <thread.h>
#include <exec.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <fcntl.h>

 MODULEID("$Id: exec.c,v 1.6 1998/04/06 06:13:44 ayman Beta $");


 #define YOU_NEED_HELP() \
        say ("%$% You really need -h.\n", MILD); \
        return

 void ProcessExec (Socket *sptr, const char *s)

 {
  extern Socket *so_hash[];
  #define iptr ((Index *)sptr->index)
  #define th ((struct thread_params *)iptr->pindex)
  #define exec ((EXEC *)th->params)

    if (exec->flags&(2|4))
     {
       if (ToServer("PRIVMSG %s :%s\n", (char *)exec->redir, s))
        {
          if (exec->flags&4)
           {
            say ("<[%s]> %s\n", (char *)exec->redir, s);
           }
          else
          if (exec->flags&2)
           {
            say ("%$243%s:%s%$242 %s\n", curnick(), (char *)exec->redir, s);
           }
        }
     }
    else
    if (exec->flags&8)
     {
       if (ToServer("NOTICE %s :%s\n", (char *)exec->redir, s))
        {
         say ("<-%s-> %s\n", (char *)exec->redir, s);
        }
     }
    else
    if (exec->flags&16)
     {
      #define iiptr ((Index *)exec->redir)
      #define ssptr ((Socket *)iiptr->index)
      #define pptr  ((Someone *)iiptr->sindex)

        if ((ssptr)&&(so_hash[ssptr->sock]))
         {
           if (ToSocket(ssptr->sock, "%s\n", s))
            {
             say ("6<=%s=> %s\n", pptr->name, s);
            }
           else
            {
             sptr->flag|=FIRING_SQUAD;
            }
         }
       
      #undef iiptr
      #undef ssptr
      #undef pptr
     }
    else
    if (exec->flags&128)
     {
       if (((Socket *)exec->redir)&&(so_hash[((Socket *)exec->redir)->sock]))
        {
          if (ToSocket(((Socket *)exec->redir)->sock, "%s\n", s))
           {
            say ("<|%d|> %s\n", ((Socket *)exec->redir)->sock, s);
           }
          else
           {
            sptr->flag|=FIRING_SQUAD;
           }
        }
     }
    else 
    if (exec->flags&256)
     {
      #define iiptr ((Index *)exec->redir)
      #define ssptr ((Socket *)iiptr->index)
      #define pptr  ((Someone *)iiptr->sindex)

        if ((ssptr)&&(so_hash[ssptr->sock]))
         {
           if (ToSocket(ssptr->sock, "%s\n", s))
            {
             say ("3<:%s:> %s\n", pptr->name, s);
            }
           else
            {
             sptr->flag|=FIRING_SQUAD;
            }
         }

      #undef iiptr
      #undef ssptr
      #undef pptr
     }
    else
     {
      say ("|%d| %s\n", sptr->sock, s);
     }

  #undef iptr
  #undef th
  #undef exec

 }  /**/


 #define IS_IN(x) (!strncasecmp((x), "-IN", 3))
 #define IS_MSG(x) (!strncasecmp((x), "-MSG", 4))
 #define IS_OUT(x) (!strncasecmp((x), "-OUT", 4))
 #define IS__CHAT(x) (!strncasecmp((x), "-CHAT", 5))
 #define IS_CLOSE(x) (!strncasecmp((x), "-CLOSE", 6))
 #define IS_NOTICE(x) (!strncasecmp((x), "-NOTC", 7))
 #define IS_SOCK(x) (!strncasecmp((x), "-SOCK", 5))


 static unsigned DetermineType (const char *) __attribute__ ((const));

 static unsigned DetermineType (const char *s)

 {
    if (arethesame(s, "WIN"))    return 0x1<<0;
    if (arethesame(s, "CHAN"))   return 0x1<<1;
    if (arethesame(s, "MSG"))    return 0x1<<2;
    if (arethesame(s, "NOTC"))   return 0x1<<3;
    if (arethesame(s, "CHAT"))   return 0x1<<4;
    if (arethesame(s, "IN"))     return 0x1<<5;
    if (arethesame(s, "CLOSE"))  return 0x1<<6;
    if (arethesame(s, "SOCK"))   return 0x1<<7;
    if (arethesame(s, "TEL"))    return 0x1<<8;

   return 0;

 }  /**/


 static int HandleChat (Someone **, Index **, const char *);
 static int HandleTelnet (Someone **, Index **, const char *);
 static int HandleSocket (Socket **, const char *);

 static int HandleChat (Someone **pptr, Index **iptr, const char *name)

 {
    if (!name)
     {
      say ("%$% Badly formatted request.\n", HOT);

      return 0;
     }
   
    if (!(*pptr=SomeonebyName(name)))
     {
      say ("%$% Unable to redirect exec output to %s - non-existent user.\n",
           MILD, name);

      return 0;
     }

    if (!(*iptr=FindDCCIndex(DCC_CHAT, *pptr)))
     {
      say ("%$% Unable to redirect exec output to %s - non-existent DCC"
           " CHAT connection.\n", MILD, name);

      return 0;
     }

    if (!IS_CONNECTED((DCCParams *)(*iptr)->pindex))
     {
      say ("%$% Unable to redirect exec output to %s - DCC connection"
           " in progress.\n", MILD, name);

      return 0;
     }

   return 1;

 }  /**/


 static int HandleTelnet (Someone **pptr, Index **iptr, const char *name)

 {
    if (!name)
     {
      say ("%$% Badly formatted request.\n", HOT);

      return 0;
     }

    if (!(*pptr=SomeonebyName(name)))
     {
      say ("%$% Unable to redirect exec output to Telnet user %s -"
           " non-existent user.\n", MILD, name);

      return 0;
     }

    if (!(*iptr=(Index *)FindTelnetIndex(*pptr)))
     {
      say ("%$% Unable to redirect exec output to %s - user"
           " not logged on.", MILD, name);

      return 0;
     }

   return 1;

 }  /**/


 static int HandleSocket (Socket **sptr, const char *name)

 {
  register int socket;
  extern Socket *so_hash[];

    if (!name)  
     {
      say ("%$% Badly formatted request.\n", HOT); 

      return 0;
     }

    if (!(socket=atoi(name))||(socket<0)||(socket>MAXSOCKSCOUNT))
     {
      say ("%$% Unable to redirect exec output to socket %s -"
           " invalid socket.\n", MILD, name);

      return 0;
     }

    if (!(*sptr=so_hash[socket]))
     {
      say ("%$% Unable to redirect exec output to socket %s -"
           " socket not connected.\n", MILD, name);

      return 0;
     }

   return 1;

 }  /**/


 void uEXEC (char *args)

 {
  register int j=0;
  int type=0;
  char *nick,
       *ipc_path;
  Index *aux;
  Someone *pptr;
  extern char tok[][80];
  extern Someone *ipc_ptr;
  #define th_flags CLONE_MEM|CLONE_FS

    if (!args)
     {
      ShowEXEC (ipc_ptr, (1-1), ipc_ptr->nEntries-1);

      return;
     }

    if (mt_ptr->s_attrs&S_NOIPC)
     {
      say ("%$% Exec functionality is unavailable.\n", MILD);

      return;
     }

    if (!(ipc_path=valueof("IPC_PATH")))
     {
      say ("%$% IPC_PATH is not set.\n", MILD);

      return;
     }

   splitargs2 (args, ' ');

    if ((type=DetermineType(tok[j]+1)))  
     {
      if (!*tok[++j])  /* '/exec -type' */
       {
        YOU_NEED_HELP();
       }
     }
    else
     {
      type=1;
     }

   switch (type&(1<<1|1<<2|1<<3|1<<4|1<<5|1<<6|1<<7|1<<8))
          {
           case 2:
           case 4:
           case 8:
                {
                  if (!isup())
                     {
                      say ("%$% You are not connected to IRC server.\n",
                           MILD);

                      return;
                     }

                  if (!*tok[j+1]) /* '/exec -type nick' */
                     {
                      YOU_NEED_HELP();
                     }

                 nick=tok[j++];

                 break;
                }

           case 16:
                 if (!*tok[j+1]) /* '/exec -type nick' */
                    {
                     YOU_NEED_HELP();
                    }

                 if (!(HandleChat (&pptr, &aux, tok[j++])))  return;

                break;

           case 128:  /* sock */
                 if (!*tok[j+1])  {YOU_NEED_HELP();}

                 if (!(HandleSocket ((Socket **)&aux, tok[j++])))  return;

                break;
                
           case 256: /* telnet */
                 if (!*tok[j+1])  {YOU_NEED_HELP();}

                 if (!(HandleTelnet (&pptr, &aux, tok[j++])))  return;

                break;
          }

   {
    char *path,
         *shell,
         *shell_args;
    EXEC *xptr;
    Index *iptr;
    struct thread_params *th;
    char s[_POSIX_PATH_MAX];

     {
      register int i;

        for (i=0; i<=(j-1); i++)
         {
          splitw (&args);
         }
     }

     say ("'%s'\n", args);

     xmalloc(th, (sizeof(struct thread_params)));
     memset (th, 0, sizeof(struct thread_params));

     sprintf (th->ipc_path, "%s-%d", ipc_path, mt_ptr->idx);

     xmalloc(th->stack, 1024);
     xptr=xmalloc(xptr, (sizeof(EXEC)));
     memset (xptr, 0, sizeof(EXEC));

     xptr->flags|=type;

      if (type&(16|128|256))  (Index *)xptr->redir=aux;
      else 
      if (type&(2|4|8))  (char *)xptr->redir=strdup (nick);

      if (!(shell=valueof("SHELL")))
       {
        xptr->shell=strdup ("/bin/sh");
       }
      else
       {
        xptr->shell=strdup (shell);
       }

      if (!(shell_args=valueof("SHELL_ARGS")))
       {
        xptr->shell_args=strdup ("-c");
       }
      else
       {
        xptr->shell_args=strdup (shell_args);
       }

     xptr->prog=strdup (args);

     sprintf (s, "%s.%d", th->ipc_path, rand()%100);
     th->lpath=strdup (s);

     (EXEC *)th->params=xptr; 

      if ((iptr=AddtoSomeoneIndex(ipc_ptr)))
       {
        iptr->protocol=PROTOCOL_IPC;
        iptr->when=time (NULL);
        (struct thread_params *)iptr->pindex=th;

         if ((xptr->pid=new_thread(th_exec, (void *)th, th_flags, 
                                   th->stack, 1024))<0)
          {
           say ("%$% Unable to start EXEC thread.\n", HOT);

           RemoveIndexbyptr (ipc_ptr, iptr);

           return;
          }
       }
   }

 }  /**/

 #undef DETERMINE_MODE

 #define th ((struct thread_params *)iptr->pindex)
 #define exec ((EXEC *)((struct thread_params *)iptr->pindex)->params)

 void ShowEXEC (const Someone *pptr, int x, int y)

 {
  char t[10],
       r[10];
  register int i=0;
  register Index *iptr;

    if (pptr->nEntries<=1)
     {
      say ("%$% No running processes found.\n", MILD);

      return;
     }

   DisplayHeader (15, 5);

   iptr=pptr->head->next;

    while ((++i)<=x)
     {
      iptr=iptr->next;
     }
   i--;

    for ( ; iptr!=NULL, ((++i)<=(y)); iptr=iptr->next)
     {
      strftime (t, 10, "%H:%M", localtime(&iptr->when));

       if (exec->flags&1) strcpy (r, "WIN");
       else if (exec->flags&2) strcpy (r, "CHAN");
       else if (exec->flags&4) strcpy (r, "NICK");
       else if (exec->flags&8) strcpy (r, "NOTICE");
       else if (exec->flags&16) strcpy (r, "CHAT");
       else if (exec->flags&32) strcpy (r, "INPUT");
       else if (exec->flags&64) strcpy (r, "CLOSE");
       else if (exec->flags&128) strcpy (r, "SOCK");
       else if (exec->flags&256) strcpy (r, "TELNET");
       else strcpy (r, "UNKNWN");

      say (" %$179%-2d%$179%-5.5lu%$179%-15.15s%$179%-5.5s%$179%-6.6s"
           "%$179\n", i, exec->pid, exec->prog, t, r);
     }

    finish2 (15, 5);

 }  /**/


#include <signal.h>

 #define shell ((EXEC *)y->params)->shell
 #define shell_args ((EXEC *)y->params)->shell_args
 #define prog ((EXEC *)y->params)->prog
 #define pid  ((EXEC *)y->params)->pid

 void th_exec (void *x)

 {
  int fd,
      len;
  struct thread_params *y=x;
  struct sockaddr_un addr;

    if ((fd=socket(AF_UNIX, SOCK_STREAM, 0))<0)
     {
      say ("%$% Unable to obtain socket for IPC client - %s.\n", 
           HOT, strerror(errno));

      return;
     }

   memset (&addr, 0, sizeof(addr));

   addr.sun_family=AF_UNIX;
   strcpy (addr.sun_path, y->lpath);
   //sprintf (addr.sun_path, "%s.%d", y->ipc_path, pid);
   len=strlen(addr.sun_path)+sizeof(addr.sun_family);

   unlink (addr.sun_path);

    if (bind(fd, (struct sockaddr *)&addr, len)<0)
     {
      close (fd);

      say ("%$% Unable to bind path in IPC client - %s.\n", 
           HOT, strerror(errno));

      return;
     }

    if (chmod(addr.sun_path, S_IRWXU)<0)
     {
      close (fd);

      say ("%$% Unable to procede with IPC connection - %s.\n", 
           HOT, strerror(errno));

      return;
     }

   memset (&addr, 0, sizeof(addr));

   addr.sun_family=AF_UNIX;
   strcpy (addr.sun_path, y->ipc_path);
   len=strlen(addr.sun_path)+sizeof(addr.sun_family);

    if (connect(fd, (struct sockaddr *)&addr, len)<0)
     {
      close (fd);

      say ("%$% Unable to connect to IPC server - %s.\n", HOT, strerror(errno));

      return;
     }

   dup2 (fd, STDIN_FILENO);
   dup2 (fd, STDOUT_FILENO);
   dup2 (fd, STDERR_FILENO);

   execl (shell, shell, shell_args, prog, (char *)0);

   close (fd);

 }  /**/

 #undef shell
 #undef shell_args
 #undef prog
 #undef args

