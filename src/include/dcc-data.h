
 const char *dcco_options[]={
            " Accept  ",
            " Reject  ",
            " Info    "
            };

 static List dcco_ent={
         0, 3, NULL, NULL
        };
 static header dcco_hdr={3, NULL, dcco_options};

 ScrollingList dcco_list={
          SCR_OPTIONMENU,
          3,
          NULL,
          35, 0, 9, 3,
          0, 0, 0, 0, 0,
          "",
          (int *)NULL,
          (char *)NULL,
          &dcco_hdr,
          (List *)&dcco_ent,
          NULL,
          NULL,
          NULL,
          DisplayOptionMenuEntries,
          NULL,
          NULL
         };


  const char *dcc_hdrs[]={"Nick", "Type", "Stat", "File", "Complete"};

  static int len[]={10, 4, 6, 20, 8};
  static header dcc_hdr={
                         5,
                         len,
                         dcc_hdrs
                        };
  static int attrs[]={0, 1, 1};
  ScrollingList dcc_list={
          SCR_SCROLLINGLIST,
          0,
          attrs,
          4, 3, 56, 9,
          1, 0, 0, 0, 0,
          " DCC Connections ",
          (int *)NULL,
          (char *)NULL,
          &dcc_hdr,
          NULL, NULL,
          &dcco_list,
          NULL,
          ScrollingDCCEntries,
          ScrollingDCCInit,
          ScrollingDCCEnter
         };

