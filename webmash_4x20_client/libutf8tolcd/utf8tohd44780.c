/*

convert UTF-8 system string to LCD-string via wchar_t

This is certainly platform dependent code likely
running on linux and glibc only

It also uses case ranges which is a gcc extension

*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>
#include <langinfo.h>
#include "utf8tohd44780.h"

// convert unicode codepoint to HD44780 charset
uint8_t codepoint_to_hd44780(uint32_t inchar) {
  uint8_t outchar;

  // printf("%lc\t0x%04x\n",inchar,inchar);
  switch(inchar) {
  
    // '\n' (new line)
    case 10:
      outchar=inchar;
      break;

    // ASCII compatible ranges
    // 32-91
    case 32 ... 91:
      outchar=inchar;
      break;
    // 93-125
    case 93 ... 125:
      outchar=inchar;
      break;

    // Yen sign
    case 0x00A5:
      outchar=92;
      break;
    
    // right arrow
    case 0x2192:
      outchar=126;
      break;
    // left arrow
    case 0x2190:
      outchar=127;
      break;
    
    // degree sign
    case 0x00b0:
      outchar=223;
      break;
    
    // latin small letter sharp s
    case 0x00DF:
      outchar=226;
      break;
    
    // greek small letter alpha
    case 0x03B1:
      outchar=224;
      break;
    // latin small letter a with diaeresis
    case 0x00E4:
      outchar=225;
      break;
    // greek small letter beta
    case 0x03B2:
      outchar=226;
      break;
    // greek small letter epsilon
    case 0x03B5:
      outchar=227;
      break;
    // micro sign
    case 0x03BC:
      outchar=228;
      break;
    // greek small letter sigma
    case 0x03C3:
      outchar=229;
      break;
    // greek small letter rho
    case 0x03C1:
      outchar=230;
      break;
    // square root
    case 0x221A:
      outchar=232;
      break;
    // cent sign
    case 0x00A2:
      outchar=236;
      break;
    // latin small letter o with diaeresis
    case 0x00F6:
      outchar=239;
      break;
    // greek small letter theta
    case 0x03B8:
      outchar=242;
      break;
    // infinity
    case 0x221E:
      outchar=243;
      break;
    // greek capital letter omega
    case 0x03A9:
      outchar=244;
      break;
    // latin small letter u with diaeresis
    case 0x00FC:
      outchar=245;
      break;
    // greek capital letter sigma
    case 0x03A3:
      outchar=246;
      break;
    // greek small letter pi
    case 0x03C0:
      outchar=247;
      break;
    // division sign
    case 0x00F7:
      outchar=253;
      break;
    // full block
    case 0x2588:
      outchar=255;
      break;
    
    // Half-width kana (rest)
    case 0x3081:
      outchar=210;
      break;
    case 0x3083:
      outchar=172;
      break;
    case 0x3002:
      outchar=161;
      break;
    case 0x3085:
      outchar=173;
      break;
    case 0x3087:
      outchar=174;
      break;
    case 0x3089:
      outchar=215;
      break;
    case 0x308b:
      outchar=217;
      break;
    case 0x308d:
      outchar=219;
      break;
    case 0x300c:
      outchar=162;
      break;
    case 0x308f:
      outchar=220;
      break;
    case 0x3093:
      outchar=221;
      break;
    case 0x309b:
      outchar=222;
      break;
    case 0x30a1:
      outchar=167;
      break;
    case 0x30a3:
      outchar=168;
      break;
    case 0x30a5:
      outchar=169;
      break;
    case 0x30a7:
      outchar=170;
      break;
    case 0x30a9:
      outchar=171;
      break;
    case 0x30ab:
      outchar=182;
      break;
    case 0x30ad:
      outchar=183;
      break;
    case 0x30af:
      outchar=184;
      break;
    case 0x30b1:
      outchar=185;
      break;
    case 0x30b3:
      outchar=186;
      break;
    case 0x30b5:
      outchar=187;
      break;
    case 0x30b7:
      outchar=188;
      break;
    case 0x30b9:
      outchar=189;
      break;
    case 0x30bb:
      outchar=190;
      break;
    case 0x30bd:
      outchar=191;
      break;
    case 0x30bf:
      outchar=192;
      break;
    case 0x30c1:
      outchar=193;
      break;
    case 0x30c3:
      outchar=175;
      break;
    case 0x3042:
      outchar=177;
      break;
    case 0x30c5:
      outchar=194;
      break;
    case 0x3044:
      outchar=178;
      break;
    case 0x30c7:
      outchar=195;
      break;
    case 0x3046:
      outchar=179;
      break;
    case 0x30c9:
      outchar=196;
      break;
    case 0x3048:
      outchar=180;
      break;
    case 0x30cb:
      outchar=198;
      break;
    case 0x304a:
      outchar=181;
      break;
    case 0x30cd:
      outchar=200;
      break;
    case 0x304c:
      outchar=182;
      break;
    case 0x30cf:
      outchar=202;
      break;
    case 0x304e:
      outchar=183;
      break;
    case 0x30d1:
      outchar=202;
      break;
    case 0x3050:
      outchar=184;
      break;
    case 0x30d3:
      outchar=203;
      break;
    case 0x3052:
      outchar=185;
      break;
    case 0x30d5:
      outchar=204;
      break;
    case 0x3054:
      outchar=186;
      break;
    case 0x30d7:
      outchar=204;
      break;
    case 0x3056:
      outchar=187;
      break;
    case 0x30d9:
      outchar=205;
      break;
    case 0x3058:
      outchar=188;
      break;
    case 0x30db:
      outchar=206;
      break;
    case 0x305a:
      outchar=189;
      break;
    case 0x30dd:
      outchar=206;
      break;
    case 0x305c:
      outchar=190;
      break;
    case 0x30df:
      outchar=208;
      break;
    case 0x305e:
      outchar=191;
      break;
    case 0x30e1:
      outchar=210;
      break;
    case 0x3060:
      outchar=192;
      break;
    case 0x30e3:
      outchar=172;
      break;
    case 0x3062:
      outchar=193;
      break;
    case 0x30e5:
      outchar=173;
      break;
    case 0x3064:
      outchar=194;
      break;
    case 0x30e7:
      outchar=174;
      break;
    case 0x3066:
      outchar=195;
      break;
    case 0x30e9:
      outchar=215;
      break;
    case 0x3068:
      outchar=196;
      break;
    case 0x30eb:
      outchar=217;
      break;
    case 0x306a:
      outchar=197;
      break;
    case 0x30ed:
      outchar=219;
      break;
    case 0x306c:
      outchar=199;
      break;
    case 0x30ef:
      outchar=220;
      break;
    case 0x306e:
      outchar=201;
      break;
    case 0x3070:
      outchar=202;
      break;
    case 0x30f3:
      outchar=221;
      break;
    case 0x3072:
      outchar=203;
      break;
    case 0x3074:
      outchar=203;
      break;
    case 0x3076:
      outchar=204;
      break;
    case 0x3078:
      outchar=205;
      break;
    case 0x30fb:
      outchar=165;
      break;
    case 0x307a:
      outchar=205;
      break;
    case 0x307c:
      outchar=206;
      break;
    case 0x307e:
      outchar=207;
      break;
    case 0x3001:
      outchar=164;
      break;
    case 0x3080:
      outchar=209;
      break;
    case 0x3082:
      outchar=211;
      break;
    case 0x3084:
      outchar=212;
      break;
    case 0x3086:
      outchar=213;
      break;
    case 0x3088:
      outchar=214;
      break;
    case 0x308a:
      outchar=216;
      break;
    case 0x300d:
      outchar=163;
      break;
    case 0x308c:
      outchar=218;
      break;
    case 0x3092:
      outchar=166;
      break;
    case 0x309c:
      outchar=223;
      break;
    case 0x30a2:
      outchar=177;
      break;
    case 0x30a4:
      outchar=178;
      break;
    case 0x30a6:
      outchar=179;
      break;
    case 0x30a8:
      outchar=180;
      break;
    case 0x30aa:
      outchar=181;
      break;
    case 0x30ac:
      outchar=182;
      break;
    case 0x30ae:
      outchar=183;
      break;
    case 0x30b0:
      outchar=184;
      break;
    case 0x30b2:
      outchar=185;
      break;
    case 0x30b4:
      outchar=186;
      break;
    case 0x30b6:
      outchar=187;
      break;
    case 0x30b8:
      outchar=188;
      break;
    case 0x30ba:
      outchar=189;
      break;
    case 0x30bc:
      outchar=190;
      break;
    case 0x30be:
      outchar=191;
      break;
    case 0x3041:
      outchar=167;
      break;
    case 0x30c0:
      outchar=192;
      break;
    case 0x3043:
      outchar=168;
      break;
    case 0x30c2:
      outchar=193;
      break;
    case 0x3045:
      outchar=169;
      break;
    case 0x30c4:
      outchar=194;
      break;
    case 0x3047:
      outchar=170;
      break;
    case 0x30c6:
      outchar=195;
      break;
    case 0x3049:
      outchar=171;
      break;
    case 0x30c8:
      outchar=196;
      break;
    case 0x304b:
      outchar=182;
      break;
    case 0x30ca:
      outchar=197;
      break;
    case 0x304d:
      outchar=183;
      break;
    case 0x30cc:
      outchar=199;
      break;
    case 0x304f:
      outchar=184;
      break;
    case 0x30ce:
      outchar=201;
      break;
    case 0x3051:
      outchar=185;
      break;
    case 0x30d0:
      outchar=202;
      break;
    case 0x3053:
      outchar=186;
      break;
    case 0x30d2:
      outchar=203;
      break;
    case 0x3055:
      outchar=187;
      break;
    case 0x30d4:
      outchar=203;
      break;
    case 0x3057:
      outchar=188;
      break;
    case 0x30d6:
      outchar=204;
      break;
    case 0x3059:
      outchar=189;
      break;
    case 0x30d8:
      outchar=205;
      break;
    case 0x305b:
      outchar=190;
      break;
    case 0x30da:
      outchar=205;
      break;
    case 0x305d:
      outchar=191;
      break;
    case 0x30dc:
      outchar=206;
      break;
    case 0x305f:
      outchar=192;
      break;
    case 0x30de:
      outchar=207;
      break;
    case 0x3061:
      outchar=193;
      break;
    case 0x30e0:
      outchar=209;
      break;
    case 0x3063:
      outchar=175;
      break;
    case 0x30e2:
      outchar=211;
      break;
    case 0x3065:
      outchar=194;
      break;
    case 0x30e4:
      outchar=212;
      break;
    case 0x3067:
      outchar=195;
      break;
    case 0x30e6:
      outchar=213;
      break;
    case 0x3069:
      outchar=196;
      break;
    case 0x30e8:
      outchar=214;
      break;
    case 0x306b:
      outchar=198;
      break;
    case 0x30ea:
      outchar=216;
      break;
    case 0x306d:
      outchar=200;
      break;
    case 0x30ec:
      outchar=218;
      break;
    case 0x306f:
      outchar=202;
      break;
    case 0x3071:
      outchar=202;
      break;
    case 0x3073:
      outchar=203;
      break;
    case 0x30f2:
      outchar=166;
      break;
    case 0x3075:
      outchar=204;
      break;
    case 0x30f4:
      outchar=179;
      break;
    case 0x3077:
      outchar=204;
      break;
    case 0x3079:
      outchar=205;
      break;
    case 0x307b:
      outchar=206;
      break;
    case 0x307d:
      outchar=206;
      break;
    case 0x30fc:
      outchar=176;
      break;
    case 0x307f:
      outchar=208;
      break;
    
    default:
      outchar='?';
      break;
  }
  return outchar;
}

int utf8str_to_hd44780(uint8_t **data) {
  uint64_t i,len;
  uint8_t* s;
  wchar_t wc;
  
  s=*data;
  
  i=0;
  while (**data) {
    len = mblen((char *)*data, MB_CUR_MAX);
    mbstowcs(&wc,(char *)*data,1);
    s[i]=codepoint_to_hd44780(wc);
    *data += len;
    i++;
  }
  s[i]='\0';
  *data=s;
  return(0);
}
