#define MAX_REST_TIME 1000

// text of all menus (only one for settings type menu, many for seelction type menu)
char* menu_txt[NUMMENUS][NUMITEMS] = {
{NULL},
{MSELECTTXT2,MSELECTTXT0,MSELECTTXT1,MSELECTTXT3,MSELECTTXT4,MENU_UP,NULL},
{MRESTTXT0,MRESTTXT1,MRESTTXT2,MRESTTXT3,MRESTTXT4,MRESTTXT5,MRESTTXT6,MRESTTXT7,MENU_UP,NULL},
{MASHSTATE1,MASHSTATE2,MASHSTATE3,MASHSTATE4,MASHSTATE5,MASHSTATE6,MASHSTATE7,MASHSTATE8,MENU_UP,NULL},
{MCTRL_START,MCTRL_STOP,MENU_UP,NULL},
{MHEATER_STATE,MSTIRRING_STATE,MENU_UP,NULL},
{MRESTTXT0,NULL},
{MRESTTXT1,NULL},
{MRESTTXT2,NULL},
{MRESTTXT3,NULL},
{MRESTTXT4,NULL},
{MRESTTXT5,NULL},
{MRESTTXT6,NULL},
{MRESTTXT7,NULL},
{NULL},
{NULL},
{NULL},
{NULL},
{NULL},
{NULL},
{NULL},
{NULL},
{NULL},
{NULL},
{MHEATER_STATE,NULL},
{MSTIRRING_STATE,NULL},
{NULL},
{IFNOTFOUND,NULL},
{MTEXTMAC,MTEXTIP4,MTEXTIP6,MTEXTIP6L,MENU_UP,NULL},
{MTEXTMAC,MTEXTIP4,MTEXTIP6,MTEXTIP6L,MENU_UP,NULL},
{MTEXTMAC,MTEXTIP4,MTEXTIP6,MTEXTIP6L,MENU_UP,NULL},
{MTEXTMAC,MTEXTIP4,MTEXTIP6,MTEXTIP6L,MENU_UP,NULL},
{"","","","",NULL}, // MAC 0
{"","","","","","",NULL}, // IP 0
{"","","","","","","","","","","","",NULL},
{"","","","",NULL}, // V6 ll 0
{"","","","",NULL}, // MAC 1
{"","","","","","",NULL}, // IP 1
{"","","","","","","","","","","","",NULL},
{"","","","",NULL}, // V6 ll 1
{"","","","",NULL}, // MAC 2
{"","","","","","",NULL}, // IP 2
{"","","","","","","","","","","","",NULL},
{"","","","",NULL}, // V6 ll 2
{"","","","",NULL}, // MAC 3
{"","","","","","",NULL}, // IP 3
{"","","","","","","","","","","","",NULL},
{"","","","",NULL}, // V6 ll 3
{NULL}
};

// next menu to be called if item is choosen
int next_menu[NUMMENUS][NUMITEMS] = {
// nothing to be done in Menu 0
  {0}, 
// 4 items "main menu" (1)
  {4, 2, 3, 5, 26, NUMMENUS-1, 0},
  {6, 7, 8, 9, 10, 11, 12, 13, 1, 0},
  {14, 15, 16, 17, 18, 19, 20, 21, 1, 0},
  {22, 23, 1, 0},
  {24, 25, 1, 0},
  {2},
  {2},
  {2},
  {2},
  {2},
  {2},
  {2},
  {2},
  {4},
  {4},
  {4},
  {4},
  {4},
  {4},
  {4},
  {4},
  {0},
  {0},
  {5},
  {5},
  {27}, // 26
  {28, 29, 30, 31, 1, 0}, //27
  {32, 33, 34, 35, 27, 0}, //28
  {36, 37, 38, 39, 27, 0}, // 29
  {40, 41, 42, 43, 27, 0}, // 30
  {44, 45, 46, 47, 27, 0}, // 31

  {28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 0},
  {28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 0},
  {28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 0},
  {28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 0},

  {29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 0},
  {29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 0},
  {29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 0},
  {29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 0},

  {30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 0},
  {30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 0},
  {30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 0},
  {30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 0},

  {31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 0},
  {31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 0},
  {31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 0},
  {31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 31, 0}
};

// menu action functions
void set_start();
void start_process();
void stop_process();
void set_rest_timetemp();
void set_start_1();
void set_start_2();
void set_start_3();
void set_start_4();
void set_start_5();
void set_start_6();
void set_start_7();
void set_start_8();
void toggle_actuator0(struct s_menusettings *settings);
void toggle_actuator1(struct s_menusettings *settings);
void netinfo();
void menu_up();

void (*menu_action[NUMMENUS]) () = {
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
set_rest_timetemp,
set_rest_timetemp,
set_rest_timetemp,
set_rest_timetemp,
set_rest_timetemp,
set_rest_timetemp,
set_rest_timetemp,
set_rest_timetemp,
set_start_1,                  
set_start_2,                  
set_start_3,                  
set_start_4,                  
set_start_5,                  
set_start_6,                  
set_start_7,                  
set_start_8,                  
start_process,
stop_process,
toggle_actuator0,
toggle_actuator1,
netinfo,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
NULL,
menu_up
};
