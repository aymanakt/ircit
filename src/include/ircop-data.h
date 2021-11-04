 extern void DialogOperEnter (DialogBox *);

  Field fld_ircop[]={
            {" Nick ", NULL, D_INPUTFIELD, {3, 1,  14, 15},
             {MAXNICKLEN, _echo}, {NULL, NULL, NULL, "", 0}
            },
            {" Password ", NULL, D_INPUTFIELD, {3, 3, 14, 10},
             {MAXNICKLEN, _astrisk}, {NULL, NULL, NULL, "", 0}
            },
            {"  OK  ", NULL, D_BUTTON, {2, 5, 5, 0},
             {250, _noecho}, {NULL, NULL, NULL, "", 0}
            },
            {"  Cancel  ", NULL, (D_BUTTON|1<<4), {14, 5, 5, 0},  
             {250, _noecho}, {NULL, NULL, NULL, "", 0}
            }
           };

 DialogBox dlg_ircop={
          " IRC Operator ",
          0, 4,
          2, 3,
          0,
          10, 35,
          fld_ircop,
          DialogOperEnter
         };

