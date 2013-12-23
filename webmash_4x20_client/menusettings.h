#include <stdbool.h>

#define NUMMENUS 50
#define NUMITEMS 13

#define MSETTINGS_TYPE_INT 0
#define MSETTINGS_TYPE_FLOAT 1
#define MSETTINGS_TYPE_BOOL 2

#define MSETTINGS_BOOL_OFF false
#define MSETTINGS_BOOL_ON true

static const int resttime_menus[4]= {7,9,11,13};
static const int resttemp_menus[4]= {6,8,10,12};

struct s_menusettings {
  int number;
  int numitems;
  int menutype;
  char **menutext;
  
  // selection menu stuff
  // selected position at display (selection type menu)
  int arrow_pos;
  // first text to be selected (selection type menu)
  int start_pos;
  bool arrow;
  

  // settings menu stuff
  int ival;
  int imin;
  int imax;
  float fval;
  int fmin;
  int fmax;
  
  float increment;
  int datatype;
  
};    
