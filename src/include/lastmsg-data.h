
 static const char *msg_options[]={
            " WHOIS      ",
            " NSLOOKUP   ",
            " ADD NOTIFY ",
            " IGNORE     ",
           };
 static List msgo_ent={
         0, 4, NULL, NULL
        };
 static header msgo_hdr={4, NULL, msg_options};
 ScrollingList msgo_list={
          SCR_OPTIONMENU,
          4,
          NULL,
          43, 0, 12, 3,
          0, 0, 0, 0, 0,
          "", 
          (int *)NULL,
          (char *)NULL,
          &msgo_hdr, 
          (List *)&msgo_ent,
          NULL, NULL, NULL,
          DisplayOptionMenuEntries,
          NULL, NULL
         };

  const char *msg_hdrs[]={"Nick", "Host"};
  static int len[]={15, 26};
  static header msg_hdr={2, len, msg_hdrs};
  static int attrs[]={1, 1, 1};
  ScrollingList msg_list={
          SCR_SCROLLINGLIST,
          0,
          attrs, 
          4, 3, 45, 5,
          1, 0, 0, 0, 2,
          " Last msg from... ",
          (int *)NULL,
          (char *)NULL,
          &msg_hdr,
          NULL, NULL,
          &msgo_list,
          NULL,
          LastmsgEntries,
          LastmsgInit,
          LastmsgEnter
         };

