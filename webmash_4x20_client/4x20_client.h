#define gettext_noop(String) String
// maximum Number of network Interfaces to be displayed on LCD
#define MAXINTERFACES 4

void debug(char* fmt, ...);
void die(char* fmt, ...);
void err_print(char* fmt, ...);

// these are the settings, the server currently uses
struct s_pstate {
  char curtemp[10];
  float musttemp;       
  int rstate[2];
  int ctrl;
  int mpstate;
  char acttype[20];
  float resttimer; 
  int stirring;     
  int resttime[4];
  float resttemp[4];
};
                    