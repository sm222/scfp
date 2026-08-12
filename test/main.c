
#include "../src/scfp.h"
#include "../src/scfp_config.h"

const char* const files[] = {
  "txt",
  "scb",
  NULL,
};

const char* const keywords[] = {
  "TEST",
  "HELLO",
  NULL,
};

const char* const vars[] = {
  "FO:A",
  "FOOT:B",
  "FOO:C",
  "BAR:D",
  NULL,
};

int main(void) {
  // work
  scfp_rules_t*     r    = _init_rule("test", files, keywords, vars, '@');
  scfp_real_data_t* file = _open_file("config.scb");
  
  //
  //
  _test_keywords(file, r);
  _parse_file(file, r);
  
  printf("\n>%s<\n", scfp_get_value(file, r, "CFLAGS"));
  printf("\n>%s<\n", scfp_get_value(file, r, "FOO"));
  //
  _close_file(file);
  _end_rule(r);
  //
}