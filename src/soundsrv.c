/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
*/

#include <main.h>

#include <netdb.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>

#include <soundsrv.h>

#define sound_path "/home/ayman/.ircit/sound"

 MODULEID("$Id: soundsrv.c,v 1.3 1998/02/15 11:27:29 ayman Beta $");


 void InitSoundServer (void *v)

 {
  int n,
      len;
  fd_set fd;
  char *snd_path;
  struct sockaddr_un snd;  

#if 0

   unlink (sound_path);

    if ((n=socket(AF_UNIX, SOCK_STREAM, 0))<0)
       {
        return;
       }

   memset (&snd, 0, sizeof(snd));

   snd.sun_family=AF_UNIX;
   strcpy (snd.sun_path, sound_path);
   len=strlen(snd.sun_path)+sizeof(snd.sun_family);

    if (bind(n, (struct sockaddr *)&snd, len)<0)
       {
        return;
       }

    if (chmod(snd.sun_path, S_IRWXU)<0)
       {
        return;
       }

    if (listen (n, 1)<0)
       {
        return;
       }

   SoundServer (n);

#endif 

 }  /**/


 void SoundServer (int s)

 {
  fd_set fd;

   FD_ZERO (&fd);
   FD_SET (s, &fd);

    while (1)
          {
            if ((select(s+1, &fd, NULL, NULL, NULL))>0)
               {
                 if (FD_ISSET(s, &fd))	
                    {
                     ProcessSoundRequest (s);
                    }
               }
          }

 }  /**/


 void ProcessSoundRequest (int s)

 {
  int n,
      fd,
      len;
  char req[100];
  struct stat st;
  struct sockaddr_un addr;

   len=sizeof(addr);

    if ((fd=accept(s, (struct sockaddr *)&addr, &len))<0)
       {
        return;
       }
    
   n=read (fd, req, 99);

    if (n<=0)
       {
        close (fd);

        return;
       }

   close (fd);

   req[n]=0;
    
   ParseRequest (req);
   
 }  /**/


 void ParseRequest (char *req)

 {
   switch (atoi(req))
          {
           case 1:
                ActuallyPlayIt ("/home/ayman/.ircit/scr_op.au");
                break;

           case 2:
                break;
          }

 }  /**/


 void ActuallyPlayIt (const char *file)

 {
  int f1, 
      f2;
  char buf[512];

    if ((f1=open("/dev/audio", O_WRONLY)))
       {
         if ((f2=open(file, O_RDONLY)))
            {
              while((read(f2, &buf, 512))>0)
                   {
                    write (f1, &buf, 512);
                   }

             close (f1);
             close (f2);
            }
         else
            {
             close (f1);
            }
       }

 }  /**/

