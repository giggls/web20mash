#include <libintl.h>
#define WAITTEXT gettext("Waiting for data    from mashctld")

#define MSELECTTXT0 gettext_noop("Rest-Settings")
#define MSELECTTXT1 gettext_noop("Mash-State")
#define MSELECTTXT2 gettext_noop("Mash-Control")
#define MSELECTTXT3 gettext_noop("Actuator-Settings")

#define MRESTTXT0 gettext_noop("Temp. Protein Rest")
#define MRESTTXT1 gettext_noop("Dur. Protein Rest")
#define MRESTTXT2 gettext_noop("Temp. Maltose Rest") 
#define MRESTTXT3 gettext_noop("Dur. Maltose Rest") 
#define MRESTTXT4 gettext_noop("Temp. Dextrose Rest")
#define MRESTTXT5 gettext_noop("Dur. Dextrose Rest")
#define MRESTTXT6 gettext_noop("Temp. @Mashing out")
#define MRESTTXT7 gettext_noop("Dur. Mashing out")

#define MCTRL_START gettext_noop("start process")
#define MCTRL_STOP gettext_noop("cancel process")

#define MHEATER_STATE gettext_noop("heater")
#define MSTIRRING_STATE gettext_noop("stirring device")

#define MSTATE_ON  gettext("on ")
#define MSTATE_OFF gettext("off")

#define MAX_REST_TIME 1000

// text of all menus (only one for settings type menu, many for seelction type menu)
static const char* menu_txt[NUMMENUS][NUMITEMS] = {
{NULL},
{MSELECTTXT0,MSELECTTXT1,MSELECTTXT2,MSELECTTXT3,NULL},
{MRESTTXT0,MRESTTXT1,MRESTTXT2,MRESTTXT3,MRESTTXT4,MRESTTXT5,MRESTTXT6,MRESTTXT7,NULL},
{MASHSTATE1,MASHSTATE2,MASHSTATE3,MASHSTATE4,MASHSTATE5,MASHSTATE6,MASHSTATE7,MASHSTATE8,NULL},
{MCTRL_START,MCTRL_STOP,NULL},
{MHEATER_STATE,MSTIRRING_STATE,NULL},
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
{NULL},
{NULL},
{NULL}
};

// next menu to be called if item is choosen
static const int next_menu[NUMMENUS][NUMITEMS] = {
// nothing to be done in Menu 0
  {0}, 
// 4 items "main menu" (1)
  {2, 3, 4, 5, 0},
  {6, 7, 8, 9, 10, 11, 12, 13, 0},
  {14, 15, 16, 17, 18, 19, 20, 21, 0},
  {22, 23, 0},
  {24, 25, 0},
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
  {0},
  {0},
  {0},
  {0}
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
NULL,
NULL,
NULL,
NULL
};
