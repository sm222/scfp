#ifndef  _LIB_SCFP_H_
# define _LIB_SCFP_H_

# include "scfp_types.h"

scfp_file   scfp_open_file(const char* path);

const char* scfp_get_value(scfp_file file, scfp_rule rule, const char* name);

#endif