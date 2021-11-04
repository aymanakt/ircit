
 #include <dialogbox.h>

 const char *srv_options[]={
            " IRC Connect   ",
            " Connect       ",
            " Disconnect    ",
            " Remove        ",
            " New...        "
           };
 static List srvo_ent={
         0, 5, NULL, NULL
        };
 static header srvo_hdr={5, NULL, srv_options};
 ScrollingList srvo_list={
          SCR_OPTIONMENU,
          5,
          NULL,
          37, 0, 15, 4,
          0, 0, 0, 0, 0,
          "",
          (int *)NULL,
          (char *)NULL,
          &srvo_hdr,
          (List *)&srvo_ent,
          NULL, NULL, NULL,	
          DisplayOptionMenuEntries,
          NULL, NULL
         };


  const char *srv_hdrs[]={"Server", "Port", "When"};
  static int len[]={20, 5, 5};
  static header srv_hdr={3, len, srv_hdrs};
  static int attrs[]={1, 1, 0, 1, 1};
  ScrollingList srv_list={
          SCR_SCROLLINGLIST,
          0,
          attrs,
          4, 3, 37, 9,
          1, 0, 0, 0, 0,
          " Servers ",
          (int *)NULL,
          (char *)NULL,
          &srv_hdr,
          NULL, NULL,
          &srvo_list,
          NULL,
          ScrollingServersEntries,
          ScrollingServersInit,
          ScrollingServersEnter
         };


extern  void DialogNewServerEnter (DialogBox *);

  Field fld_srvnew[]={
            {" Server ", NULL, D_INPUTFIELD, {0, 1, 8, 22}, 
             {250, _echo}, {NULL, NULL, NULL, "", 0}
            },
            {" Port ", NULL, D_INPUTFIELD, {35, 1, 41, 6}, 
             {5, _echo}, {NULL, NULL, NULL, "", 0}
            },
            {"  OK  ", NULL, D_BUTTON, {4, 3, 3, 0},
             {250, _noecho}, {NULL, NULL, NULL, "", 0}
            },
            {
             " Connect ", NULL, D_BUTTON, {16, 3, 3, 0},
             {250, _noecho}, {NULL, NULL, NULL, "", 0}
            },
            {"  Cancel  ", 
             NULL,
             (D_BUTTON|0x1<<4),
             {31, 3, 3, 0},
             {250, _noecho},
             {NULL, NULL, NULL, "", 0},
            }
           };

 DialogBox dlg_srvnew={
          " Add new server ",
          0, 5,
          2, 3, 
          0,
          8, 55, /* hight, width=4+strlen("  Name ")+length(10) of the widest*/
          fld_srvnew,
          DialogNewServerEnter
         };

