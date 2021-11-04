
 /* Item flags */
 /* pass non-ctrl keyboard keys (input field) */
 #define ITATR_KEBOARDINPUT (0x1<<0)
 #define ITATR_PASSIVE      (0x1<<1)
 #define ITATR_UNTAGGABLE   (0x1<<2)
 /* takeover the input-foucs (e.g to load new control are) */
 #define ITATR_KIDNAPFOCUS  (0x1<<3)
 #define ITATR_SELECTABLE   (0x1<<4)

 /* Item current state */
 #define ITST_TAGGED         (0x1<<0)
 #define ITST_DISABLED       (0x1<<1)
 #define ITST_SELECTED       (0x1<<2)
 #define ITST_TOUCHED        (0x1<<3)
 
 #define ST_MOVING    (0x1<<0)
 #define ST_RESIZING  (0x1<<1)

 #define FLG_SAVEPREVIMAGE    (0x1<<0)
 #define FLG_LOADSPAREWIN     (0x1<<1)
 #define FLG_DONTOVERLAY      (0x1<<2)
 #define FLG_NOBENEATH        (0x1<<3)
 #define FLG_KEEPCOORDINATES  (0x1<<4)
 #define FLG_CURSORVISIBLE    (0x1<<5)
 #define FLG_OVERLAYED        (0x1<<6)
 #define FLG_ACTIVEBORDER     (0x1<<7)
 #define FLG_SHADOW           (0x1<<8)
 #define FLG_MOVEABLE         (0x1<<9)
 #define FLG_RESIZEABLE       (0x1<<10)

 #define FLGS_MENUBAR    (FLG_LOADSPAREWIN|FLG_NOBENEATH|FLG_KEEPCOORDINATES)
                      //(2|8|16)
 #define FLGS_MENUBOX    (1|2|16|FLG_SHADOW)
 #define FLGS_SCROLLMAIN (1|2|FLG_ACTIVEBORDER|FLG_SHADOW|FLG_MOVEABLE)
 #define FLGS_SCROLLSUB  (1|2|4|FLG_SHADOW|FLG_MOVEABLE)
 #define FLGS_MSGBOX     (1|2|FLG_ACTIVEBORDER|FLG_SHADOW|FLG_MOVEABLE)
 #define FLGS_DIALOGBOX  (1|2|32|FLG_ACTIVEBORDER|FLG_SHADOW|FLG_MOVEABLE)

 /*#define SAVE_PREV_IMAGE  (0x1<<0)
 #define LOAD_SPARE_WIN   (0x1<<1)
 #define DONT_OVERLAY     (0x1<<2)
 #define NO_BENEATH       (0x1<<3)
 #define KEEP_COORDINATES (0x1<<4)
 #define CURSOR_VISIBLE   (0x1<<5)
 #define OVERLAYED        (0x1<<6)
 #define ACTIVE_BORDER    (0x1<<7)

 #define FLGS_MENUBAR    (LOAD_SPARE_WIN|NO_BENEATH|KEEP_COORDINATES)//(2|8|16)
 #define FLGS_MENUBOX    (1|2|16)
 #define FLGS_SCROLLMAIN (1|2|ACTIVE_BORDER)
 #define FLGS_SCROLLSUB  (1|2|4)
 #define FLGS_MSGBOX     (1|2|ACTIVE_BORDER)
 #define FLGS_DIALOGBOX  (1|2|32|ACTIVE_BORDER)
*/
 struct Geometry {
        int h,
            w,
            y,
            x;
        };
 typedef struct Geometry Geometry;

  extern /*Windows **/int
 LoadStack (void *, void *, int (*)(int), void (*)(void *), 
            int (*)(void *, Windows *), int, int, int);
 extern void LoadPrevLevel (void);
 extern  int RetrieveType (const void *);
 extern void *RetrievePosted (int, const void *);
 extern void *RetrieveCurrentWindow (void);
 extern void *RetrieveCurrentParentWindow (void);
 extern void *RetrieveParentWindow (void);
 extern void RefreshListContents (void *);
 extern void *Hello (void);
 extern int NewWindow (int, int, int *, int *, Windows *, int);
 extern int NewSubWindow (int, int, int, int, Windows *);
 extern void DestroyWindow (int, void *);
 extern void MoveWindow (int, int);
 extern void NextPrev (void);
 extern void ReadKeyboardInput (void);

 extern void ExplodeWindow (WINDOW *, int);
 extern void ExplodeWindowGravityEast (WINDOW *);
 extern void ExplodeWindowGravitySouth (WINDOW *);
 extern void Box (WINDOW *, int, int, int, int);
 extern void BorderOn (WINDOW *);
 extern void BorderOff (WINDOW *);

