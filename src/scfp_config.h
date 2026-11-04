#ifndef  _LIB_SCFP_CONFIG_H_
# define _LIB_SCFP_CONFIG_H_

# include <stdio.h>
# include <ctype.h>
# include <string.h>
# include <strings.h>
# include <stdbool.h>

# ifndef  SCFP_READ_SIZE
#  define SCFP_READ_SIZE 200
# endif

# define SCFP_VAR_SIZE 10000

typedef ssize_t byte_t;

static inline ssize_t s_strlen(const char* s) {
  return s ? (ssize_t)strlen(s) : -1;
}

ssize_t find_var_len(const char* s);

# define TOKENSIZE 1
# define TOKENS_LIST "\\%!"

# define SCFP_MAX_KEYWORDS 100

typedef int(*keywordFt)(const char*, size_t, void*, void*);

static const char* const _empty[] = {
  "",
  NULL
};

typedef struct scfp_rules_s {
  char*         name;        //* rule engine, rule config
  char**        validFiles;  //* .txt, .test
  /*            |            */
  char**        keywords;    //* _TEST _FOO _BAR
  char**        reserveVars; //* NAME NUMBER_USER
  unsigned int  keywordSize; //? total
  unsigned int  varSize;     //? total
  /*            |            */
  char          escape;      // % ? * $
  keywordFt     fts[SCFP_MAX_KEYWORDS];
  struct  scfp_rules_s* next;
} scfp_rules_t;

typedef struct scfp_file_s {
  char*              name;
  char**             raw;
  size_t             nbLine;
  size_t             i;
  size_t             j;
  //
  int                dep;
} scfp_file_t;


typedef const char*         name_t;
typedef char**              doubleString_t;
typedef const char* const*  constDoubleString_t;

typedef enum {
  scfp_valid_file = 0,
  scfp_invalid_file,
  scfp_empty_file,
  scfp_buffer_full,
} scfp_data_status;

typedef enum {
  scfp_invalid_line = -1,
  scfp_comment_line,
  scfp_var_line,
  scfp_next_value,
  scfp_unknow_line,
  scfp_empty_line,
} scfp_parsing;

static const char* const scfp_parsing_msg[] = {
  "invalid line",
  "comment line",
  "var line",
  "next value",
  "unknow line",
  "empty line",
  NULL
};

typedef struct scfp_real_data {
  scfp_file_t      data;
  scfp_data_status status;
  char             buffer[SCFP_VAR_SIZE + 1];
} scfp_real_data_t;


scfp_real_data_t* _open_file(name_t);
void              _close_file(scfp_real_data_t*);
scfp_parsing      _parse_file(scfp_real_data_t*, scfp_rules_t*);

scfp_rules_t*     _init_rule(name_t, constDoubleString_t, constDoubleString_t, constDoubleString_t, char);
void              _end_rule(scfp_rules_t* rule);

int               _test_keywords(scfp_real_data_t* file, scfp_rules_t* rules);

int               _is_line_valid(const char* s);

void              reset_recursive(void);
size_t            _get_value(scfp_real_data_t* file, ssize_t* total, const char* name, scfp_rules_t* rule);

#endif