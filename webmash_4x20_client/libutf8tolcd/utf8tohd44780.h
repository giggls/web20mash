#include <stdint.h>
#define EINVALID_SYS_LOCALE 1
#define EINVALID_INPUT 2

__attribute__ ((unused)) static const char* utf8tohd44780_msg[] = {"OK","locale setting not in UTF-8","invalid utf-8 in input data"};

uint8_t codepoint_to_hd44780(uint32_t inchar);

// use this function to convert utf-8 to HD44780 encoding
// This function will require a Null-terminated string
// the in-place replacement will work, as HD44780 encoding
// is alway smaller than utf-8
int utf8str_to_hd44780(uint8_t **data);
