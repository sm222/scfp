#ifndef  _LIB_SCFP_TYPES_H_
# define _LIB_SCFP_TYPES_H_

# define SCFP_MAX_KEYWORDS 100

typedef int(*keywordFt)(const char*, void*);

typedef void*   scfp_file;
typedef void*   scfp_rule;
# include <unistd.h>
# include <stdbool.h>

#endif