#define gettext_noop(String) String

void debug(char* fmt, ...);
void die(char* fmt, ...);

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
                    