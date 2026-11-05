# include "scfp_config.h"
# include <stdlib.h>
# include <unistd.h>
# include <fcntl.h>

#include "gnl.h"



//*
static size_t _read_file(const int fd, scfp_real_data_t* file) {
  size_t size = 20;
  size_t i = 0;
  file->data.raw = calloc(size, sizeof(char*));
  const char* line = NULL;
  do {
    line = get_next_line(fd);
    file->data.raw[i++] = (char*)line;
    if (line && i > size / 2) {
      size *= 2;
      file->data.raw = realloc(file->data.raw, (size * sizeof(char*)));
    }
  } while (line);
  return i;
}

static char* s_strdup(const char* s) {
  ssize_t l = s_strlen(s);
  if (l == -1)
    return NULL;
  char* str = calloc(l + 1, sizeof(char));
  memcpy(str, s, l);
  return str;
}


scfp_real_data_t* _open_file(const char* name) {
  scfp_real_data_t* file = NULL;
  file = calloc(1, sizeof(*file));
  if (!file) { return NULL; }
  file->status = scfp_valid_file;
  const int fd = open(name, O_RDONLY);
  if (fd <= 0) {
    perror(name);
    return file;
  }
  file->data.name = s_strdup(name);
  file->data.nbLine = _read_file(fd, file);
  return file;
}


void  _close_file(scfp_real_data_t* file) {
  for (size_t i = 0; file->data.raw[i]; i++) {
    free(file->data.raw[i]);
  }
  free(file->data.raw);
  free(file->data.name);
  free(file);
}


static inline bool _valid(int prev, int now) {
  if (now == scfp_invalid_line || now == scfp_unknow_line)
    return false;
  if (now == scfp_next_value) {
    if (prev == scfp_var_line || prev == scfp_next_value) {
      return true;
    }
    return false;
  }
  return now >= 0 ? true : false;
}


scfp_parsing  _parse_file(scfp_real_data_t* file, scfp_rules_t* rules) {
  size_t i = 0;
  int prev = scfp_empty_line;
  while (file->data.raw[i]) {
    const char* l = file->data.raw[i];
    const int line = _is_line_valid(l);
    if (!_valid(prev, line)) {
      fprintf(stderr, "scfp: invalid config file error: %s\n" \
        "%s:%zu - > %s\n", scfp_parsing_msg[(prev * -1)], file->data.name, i + 1, l);
        file->status = scfp_invalid_file;
      return 1;
    }
    if (line == scfp_var_line) {
      ssize_t t = 0;
      _get_value(file, &t, l, rules);
    }
    i++;
    prev = line;
  }
  return 0;
}

ssize_t _dstr(constDoubleString_t s) {
  ssize_t i = !s ? -1 : 0;
  while (s[i]) {
    i++;
  }
  return i;
}

ssize_t _cp_doubleString(doubleString_t* dest, constDoubleString_t src) {
  ssize_t l = _dstr(src);
  *dest = calloc(l + 1, sizeof(*dest));
  const ssize_t size = l;
  while (l--) {
    (*dest)[l] = s_strdup(src[l]);
  }
  return size;
}

void  _free_doubleString(doubleString_t str) {
  size_t i = 0;
  for (; str[i]; i++) {
    free(str[i]);
  }
  free(str);
}

scfp_rules_t* _init_rule(name_t name, constDoubleString_t files, constDoubleString_t keywords, constDoubleString_t vars, char escape) {
  scfp_rules_t* rule = calloc(1, sizeof(*rule));
  if (!rule) {
    perror("calloc");
    return NULL;
  }
  rule->name = s_strdup(name);
  _cp_doubleString(&rule->validFiles, files);
  rule->keywordSize = _cp_doubleString(&rule->keywords, keywords);
  rule->varSize = _cp_doubleString(&rule->reserveVars, vars);
  rule->escape = escape;
  return rule;
}

void  _end_rule(scfp_rules_t* rule) {
  _free_doubleString(rule->keywords);
  _free_doubleString(rule->reserveVars);
  _free_doubleString(rule->validFiles);
  free(rule->name);
  free(rule);
}

const char* _read_line(scfp_real_data_t* file) {
  if (!file) { return NULL; }
  if (file->data.i >= file->data.nbLine) { return NULL; }
  return file->data.raw[file->data.i++];
}

//todo
int _test_keywords(scfp_real_data_t* file, scfp_rules_t* rules) {
  //const char* l = "";
  (void)rules;
  (void)file;
  //while (l) {
   // l = _read_line(file);
    //printf(">%s", l);
  //}
  return 0;
}

const char* _read_predefine_var(scfp_rules_t* rules, const char* name) {
  const ssize_t nameLen = find_var_len(name);
  for (size_t i = 0; i < rules->varSize; i++) {
    if (strncmp(name, rules->reserveVars[i], nameLen) == 0 \
      && rules->reserveVars[i][nameLen] == ':') {
        return rules->reserveVars[i] + nameLen + 1;
    }
  }
  return "";
}

# include "scfp_types.h"
# include <errno.h>

int scfp_pars_file(scfp_file file, scfp_rule rule) {
  return _parse_file((scfp_real_data_t*)file, (scfp_rules_t*)rule);
}


const char* scfp_get_value(scfp_file file, scfp_rule rule, const char* name) {
  if (!file || !rule || !name) {
    fprintf(stderr, "scfp: missing info\n");
    return "";
  }
  scfp_real_data_t* c_file  = file;
  scfp_rules_t*     c_rule  = rule;
  //
  bzero(c_file->buffer, SCFP_VAR_SIZE);
  if (c_file->status == scfp_buffer_full) {
    c_file->status = scfp_valid_file;
    fprintf(stderr, "scfp: buffer is full\n");
  }
  if (c_file->status) {
    fprintf(stderr, "scfp: file not valid: %d\n", c_file->status);
    return "";
  }
  ssize_t total = 0;
  c_file->data.dep = 0;
  _get_value(c_file, &total, name, c_rule);
  if (c_file->buffer[0] == 0) {
    return _read_predefine_var(c_rule, name);
  }
  return c_file->buffer;
}

scfp_file   scfp_open_file(const char* path) {
  return _open_file(path);
}


scfp_rule    scfp_init_rule(const char* name,
    const char* const* filesEx, const char* const* validKeywords,
    const char* const* preDefineVar, char escapeCar) {
  //
  scfp_rules_t* rules = _init_rule(name, filesEx, validKeywords, preDefineVar, escapeCar);
  if (!rules || !rules->name || !rules->validFiles || !rules->keywords || !rules->reserveVars) {
    if (rules) {
      _end_rule(rules);
    }
    fprintf(stderr, "scfp: faild to init rules %s\n", strerror(errno));
    return NULL;
  }
  return rules;
}

void scfp_close_file(scfp_file file) {
  _close_file(file);
}

void scfp_end_rule(scfp_rule rule) {
  _end_rule(rule);
}


bool    scfp_set_context(scfp_rule rule, void* context) {
  if (!rule || !context) {
    fprintf(stderr, "scfp: setmissing value");
    return false;
  }
  _scfp_set_context(rule, context);
  return true;
}

