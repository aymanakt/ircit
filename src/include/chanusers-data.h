  static const char *chu_options[]={
              " Info ",
              " Op   ",
              " Kick ",
              " Ban  "
             };
  static List chuo_ent={
         0, 4, NULL, NULL
        };
  static header chuo_hdr={4, NULL, chu_options};
  ScrollingList chuo_list={
          SCR_OPTIONMENU,
          4,
          NULL,
          50, 0, 6, 4,
          1, 0, 0, 0, 0,
          "",
          (int *)NULL,
          (char *)NULL,
          &chuo_hdr,
          (List *)&chuo_ent, 
          NULL, NULL, NULL,
          DisplayOptionMenuEntries,
          NULL, NULL 
         };

  static const char *chu_hdrs[]={"User", "Host"};
  static const int len[]={10, 20};
  static header chu_hdr={2, len, chu_hdrs};
  static int attrs[]={1, 0, 0, 0};
  ScrollingList chu_list={
          SCR_SCROLLINGLIST,
          0,
          attrs,
          14, 4, 50, 10,
          1, 0, 0, 0, 0,
          "",
          (int *)NULL,
          (char *)NULL,
          &chu_hdr,
          (void *)NULL,
          NULL,
          &chuo_list,
          NULL,
          ScrollingChanusersEntries,
          NULL, NULL
         };

