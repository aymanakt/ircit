  
  static const char *soo_options[]={
              " Close   ",
              " Info... "
             };
  static List soo_ent={
         0, 2, NULL, NULL
        };
  static header soo_hdr={2, NULL, soo_options};
  ScrollingList soo_list={
          SCR_OPTIONMENU,
          2,
          NULL,
          37, 0, 9, 2,
          1, 0, 0, 0, 0,
          "",
          (int *)NULL,
          (char *)NULL,
          &soo_hdr,
          (List *)&soo_ent,
          NULL, NULL, NULL, 
          DisplayOptionMenuEntries,
          NULL, NULL
         };


  static const char *so_hdrs[]={"fd", "Type", "Remote Address", "Port"};
  static const int len[]={3, 8, 25, 5};
  static header so_hdr={4, len, so_hdrs};
  static int attrs[]={1, 1};
  ScrollingList so_list={
          SCR_SCROLLINGLIST,
          0,
          attrs,
          5, 4, 48, 9,
          1, 0, 0, 0, 0,
          " Sockets ",
          (int *)NULL,
          (char *)NULL,
          &so_hdr,
          (void *)NULL,
          NULL,
          &soo_list,
          NULL,
          ScrollingSocketsEntries,
          NULL,
          ScrollingSocketsEnter
         };

