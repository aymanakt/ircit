/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
**
MODULEID("$Id: u_status.h,v 1.3 1998/03/16 10:09:38 ayman Beta $")
**
*/

 #define IS_STATUS_DEFAULT() (mt_ptr->c_status==1)
 #define IS_STATUS_DCC() (mt_ptr->c_status==2)
 #define IS_STATUS_CHANNELS() (mt_ptr->c_status==3)

 void DoUpperStatus (void);

 void UpdateSockets (int, const char);
 void DisplayUpperStatusI (void);

 void DisplayUpperStatusII (void);

 void DisplayUpperStatusIII (void);

 void LoadUpperStatusI (void);
 void LoadUpperStatusII (void);
 void LoadUpperStatusIII (void);
 void LoadUpperStatusIV (void); 
 void LoadUpperStatus (void *, int);

