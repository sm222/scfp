
#include "../src/scfp.h"
//#include "../src/scfp_config.h"

const char* const files[] = {
  "txt",
  "scb",
  0x0,
};

const char* const keywords[] = {
  "TEST",
  "HELLO",
  "ADD",
  "HHH",
  0x0,
};

const char* const vars[] = {
  "FO:A",
  "FOOT:B",
  "FOO:C",
  "BAR:hello world",
  0x0,
};

/*
    "C",
    "FOO",
    "VEC3",
    "FOOT",
    "BLAH",
    "",
    "PPPP",
    "IOUI",
*/

# include <unistd.h>
# include <stdio.h>

typedef struct context {
  bool isOpen;
} context;

int test(const char* line, void* con) {
  context* cast = con;
  
  fprintf(stderr, "%s %s\n", cast->isOpen ? "yes" : "no", line);
  return 1;
}

int main(void) {
  // work
  context con;
  con.isOpen = true;
  scfp_file file = scfp_open_file("config.scb");
  scfp_rule rule = scfp_init_rule("test", files, keywords, vars, '@');
  scfp_pars_file(file, rule);
  scfp_set_context(rule, &con);
  //scfp_set_ft(0, rule, &test);
  const char* const list[] = {
    "C",
    "FOO",
    "VEC3",
    "FOOT",
    "BLAH",
    "",
    "PPPP",
    "BAR",
    NULL
  };
  for (int i = 0; list[i]; i++) {
    const char* b = scfp_get_value(file, rule, list[i]);
    printf("---\n%s >%s<\n", list[i], b);
  }
  scfp_close_file(file);
  scfp_end_rule(rule);
  //
}