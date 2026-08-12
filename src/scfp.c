# include "scfp_config.h"
# include <stdlib.h>
# include <unistd.h>
# include <fcntl.h>

# define NL '\n'
# define B_SIZE 101

# define BUFFER_SIZE 200

typedef struct s_info {
  char*  tmp;
  char*  tmp2;
  char   readtmp[BUFFER_SIZE + 1];
  int    rv;
  size_t  cut;
}  t_info;

static void* ft_sfree(void* ptr) {
  free(ptr);
  return NULL;
}

static char* ft_strjoin(char* sfree, char* s2) {
  size_t s1_i = strlen(sfree ? sfree : "");
  size_t s2_i = strlen(s2 ? s2 : "");
  char* new = calloc(s1_i + s2_i + 1, sizeof(char));

  if (!new) {
    sfree = ft_sfree(sfree);
    return (NULL);
  }
  while (s1_i + s2_i-- > s1_i)
    new[s1_i + s2_i] = s2[s2_i];
  while (s1_i--)
    new[s1_i] = sfree[s1_i];
  if (*new == 0)
    new = ft_sfree(new);
  return (ft_sfree(sfree), new);
}

static char* ft_tiny_split(char* s, size_t* cut) {
  char* new = NULL;
  size_t  i = 0;

  while (s[i])
    if (s[i++] == '\n')
      break ;
  new = calloc(i + 1, sizeof(char));
  if (!new)
    return (new = ft_sfree(new));
  *cut = i;
  while (i--)
    new[i] = s[i];
  return (new);
}

static char  ft_find_nl(char* s) {
  size_t  i = 0;

  while (s && s[i]) {
    if (s[i] == '\n')
      return ('\n');
    i++;
  }
  return ('0');
}

static char* safe_return(char**  book, t_info* t_val) {
  t_val->tmp = ft_tiny_split(*book, &t_val->cut);
  if (!t_val->tmp) {
    *book = ft_sfree(*book);
    return (NULL);
  }
  t_val->tmp2 = *book;
  *book = ft_strjoin(NULL, *book + t_val->cut);
  t_val->tmp2 = ft_sfree(t_val->tmp2);
  return (t_val->tmp);
}

char* get_next_line(int fd) {
  static char* book = NULL;
  t_info    t_val;

  if (fd < 0 || BUFFER_SIZE <= 0 || fd > 255)
    return (book = ft_sfree(book));
  if (!book)
    book = calloc(1, sizeof(char));
  if (!book)
    return (NULL);
  t_val.rv = 0;
  while (ft_find_nl(book) == '0') {
    bzero(t_val.readtmp, BUFFER_SIZE + 1);
    t_val.rv = read(fd, t_val.readtmp, BUFFER_SIZE);
    if (t_val.rv <= 0)
      break ;
    book = ft_strjoin(book, t_val.readtmp);
    if (!book)
      return (NULL);
  }
  if (t_val.rv == -1 || (t_val.rv <= 0 && *book == 0)) {
    book = ft_sfree(book);
    return (NULL);
  }
  return (safe_return(&book, &t_val));
}

void test(void) {
  const char* s = "Hello Wold";
  ssize_t len = s_strlen(s);
  printf("%zu %s\n", len, s);
}


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
  (void)rules;
  int prev = scfp_empty_line;
  while (file->data.raw[i]) {
    const char* l = file->data.raw[i];
    const int line = _is_line_valid(l);
    if (!_valid(prev, line)) {
      fprintf(stderr, "scfp: invalid config file\n" \
        "%s:%zu - > %s\n", file->data.name, i + 1, l);
      return 1;
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

int _test_keywords(scfp_real_data_t* file, scfp_rules_t* rules) {
  const char* l = "";
  (void)rules;
  while (l) {
    l = _read_line(file);
    //printf(">%s", l);
  }
  return 0;
}

static const char* _read_predefine_var(scfp_rules_t* rules, const char* name) {
  const ssize_t nameLen = s_strlen(name);
  for (size_t i = 0; i < rules->varSize; i++) {
    if (strncmp(name, rules->reserveVars[i], nameLen) == 0 \
      && rules->reserveVars[i][nameLen] == ':') {
        return rules->reserveVars[i] + nameLen + 1;
    }
  }
  return "";
}

# include "scfp_types.h"


const char* scfp_get_value(scfp_file file, scfp_rule rule, const char* name) {
  if (!file || !rule || !name) {
    return "";
  }
  scfp_real_data_t* c_file  = file;
  scfp_rules_t*     c_rules = rule;
  //
  ssize_t total = 0;
  bzero(c_file->buffer, SCFP_VAR_SIZE);
  _get_value(c_file, &total, -1, name, c_rules);
  if (c_file->buffer[0] == 0) {
    return _read_predefine_var(c_rules, name);
  }
  return c_file->buffer;
}