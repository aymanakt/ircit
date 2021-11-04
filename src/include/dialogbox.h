/*
** IRCIT Copyright (c) 1998 Ayman Akt
**
** This file is part of the IRCIT (c) source distribution.
** See the COPYING file for terms of use and conditions.
**
"$Id: dialogbox.h,v 1.1 1998/03/11 11:22:35 ayman Beta $"
**
*/

 #define D_INPUTFIELD  (0x1<<0)
 #define D_BUTTON      (0x1<<1)
 #define D_CHECKBOX    (0x1<<2)
 #define D_USECONTENTS (0x1<<3)
 #define D_BUTTCANCEL  (0x1<<4)


 enum EchoMode {_echo, _noecho, _astrisk};
 typedef enum EchoMode EchoMode;

 struct FieldGeometry {
         int cstartx,
             starty,
             startx; 
         int length;
        };
 typedef struct FieldGeometry FieldGeometry;


 struct FieldEdit {
        char *fw_start,
             *fw_end,
             *fw_buf;
        char buffer[200];
        int buf_len;
       };
 typedef struct FieldEdit EditData;

 struct FieldEditParams {
        size_t  size;
        EchoMode echo;
       };
 typedef struct FieldEditParams EditParams;

 struct Field 
             {
              char *caption;
              void *data;
              unsigned flags;
              FieldGeometry geo;
              EditParams params;
              EditData edit;
             };
 typedef struct Field Field;
 
 struct DialogBox
                 {
                  char *title;
                  unsigned flags;
                  int nEntries;
                  int startx,
                      starty;
                  int c_field;
                  int hight,
                      width;
                  Field *fptr;
                  void (*dialog_enter) (struct DialogBox *);
                 };
 typedef struct DialogBox DialogBox;

 const ObjectsAndColors *const _dialog;
 const ObjectsAndColors *const _dialogborder;

 void LoadDialogBox (DialogBox *);
 int GetDialogBoxInput (int);
 void DialogBoxCleanup (void *);

