#ifndef  _LIB_SCFP_TYPES_H_
# define _LIB_SCFP_TYPES_H_

# define SCFP_VAR_SIZE 10000
# define SCFP_MAX_KEYWORDS 100

# include <unistd.h>
# include <stdbool.h>

typedef struct {
  const char* const from;
  ssize_t*          current;
  char*             buff;
} strs;

typedef int(*keywordFt)(strs*, void*);

typedef void*   scfp_file;
typedef void*   scfp_rule;

#endif