
 struct CommandButton {
         ObjectParticulars item;
         char *action;
        };
 typedef struct StaticButton CommandButton;

#ifdef 0
 struct VerticalSeperator {
         ObjectParticulars item;
         unsigned sample;
         unsigned attrs;  /* bold|dim|blink */
        };
 typedef struct VerticalSeperator VerticalSeperator;
#endif

 struct StaticButton {
         ObjectParticulars item;
         char *action;
        };
 typedef struct StaticButton StaticButton;


  ObjectContainer *
 CreateCommandButton (ControlArea *, const char *, int, int, int);
 int intrin_addcommandbutton (struct fplArgument *);

