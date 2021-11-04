/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
**
MODULEID("$Id: useri.h,v 1.6 1998/05/05 03:37:55 ayman Beta $")
**
*/

 struct InResponseToUser {
        char *identifier;
        char *alias;
        void (*read) (char *);
       };

 typedef struct InResponseToUser Command;

 void uABOUT (char *);
 void uAWAY (char *);
 void uBAN (char *);
 void uCD (char *);
 void uCHANNELS (char *);
 void uCLEAR (char *);
 void uCTCP (char *);
 void uDESCRIBE (char *);
 void uEXIT (char *) __attribute__ ((noreturn));
 void uHIDE (char *);
 void uINVITE (char *);
 void uIRCIT (char *);
 void uFLUSH (char *);
 void uJOIN (char *);
 void uCTRLF (void);
 void uCTRLX (void);
 void uKICK (char *);
 void uLOCALHOST (char *);
 void uLS (char *);
 void uLUSERS (char *);
 void uLOOKUP (char *);
 void uME (char *);
 void uMASS (char *);
 void uMODE (char *);
 void uMSG (char *);
 void uDMSG (char *);
 void uSMSG (char *);
 void uSETUID (char *);
 void uTMSG (char *);
 void uNICK (char *);
 void uNOTICE (char *);
 void uNOTIFY (char *);
 void uOPER (char *);
 void uPART (char *);
 void uPEOPLE (char *);
 void uQUOTE (char *);
 void uSCREENSHOT (char *);
 void uSET (char *);
 void uSHOOT (char *);
 void uSTATUS (char *);
 void uTIMERS (char *);
 void uUPDATE (char *);
 void uWHO (char *);
 void uWHOAMI (char *);
 void uWHOIS (char *);
 void uWHOWAS (char *);
 void uVOLUME (char *);

 extern void uALIAS (char *);
 extern void uASSOC (char *);
 extern void uBINDKEY (char *);
 extern void uCOLORS (char *);
 extern void uDCC (char *);
 extern void uEXEC (char *);
 extern void uHELP (char *);
 extern void uHISTORY (char *);
 extern void uIGNORE (char *);
 extern void uFPL (char *);
 extern void uLASTLOG (char *);
 extern void uLOAD (char *);
 extern void uREHASH (char *);
 extern void uSAVE (char *);
 extern void uSERVER (char *);
 extern void uSOCKETS (char *);

