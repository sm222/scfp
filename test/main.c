
#include "../src/scfp.h"


const char* const files[] = {
  "txt",
  "scb",
  0x0,
};

const char* const keywords[] = {
  "TEST",
  "HELLO",
  "EN_CA",
  "FR_CA",
  0x0,
};

const char* const vars[] = {
  "FO:A",
  "FOOT:B",
  "FOO:C",
  "BAR:hello world",
  "NOT_SUPORT:!",
  0x0,
};


# include <unistd.h>
# include <stdio.h>

enum {
  language_english,
  language_french,
  language_not_suport,
};

typedef struct context {
  bool isOpen;
  int  language;
} context;

int test(strs* s, void* con) {
  context* cast = con;
  add_to_buffer(s, " i see you");
  fprintf(stderr, "%s %s\n", cast->isOpen ? "yes" : "no", s->from);
  return 1;
}

int en_ca(strs* s, void* con) {
  context* cast = con;
  (void)s;
  return cast->language == language_english;
}

int fr_ca(strs* s, void* con) {
  context* cast = con;
  (void)s;
  return cast->language == language_french;
}


int main(void) {
  // work
  context con;
  con.isOpen = true;
  con.language = 0;
  //
  scfp_file file = scfp_open_file("config.scb");
  scfp_rule rule = scfp_init_rule("test", files, keywords, vars, '@');
  //
  scfp_set_context(rule, &con);
  scfp_set_ft(0, rule, &test);
  scfp_set_ft(1, rule, &test);
  scfp_set_ft(2, rule, &en_ca);
  scfp_set_ft(3, rule, &fr_ca);
  scfp_pars_file(file, rule);
  //
  const char* const list[] = {
    "SETTING",
    "SETTING",
    "SETTING",
    NULL
  };
  for (int i = 0; list[i]; i++) {
    const char* b = scfp_get_value(file, rule, list[i]);
    con.language++;
    printf("---\n%s >%s<\n", list[i], b);
  }
  scfp_close_file(file);
  scfp_end_rule(rule);
  //
}