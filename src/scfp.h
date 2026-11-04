#ifndef  _LIB_SCFP_H_
# define _LIB_SCFP_H_

# include "scfp_types.h"

scfp_file    scfp_open_file(const char* path);
scfp_rule    scfp_init_rule(const char* name,
          const char* const* filesEx, const char* const* validKeywords,
          const char* const* preDefineVar, char escapeCar);


int          scfp_pars_file(scfp_file file, scfp_rule rule);
const char*  scfp_get_value(scfp_file file, scfp_rule rule, const char* name);

void         scfp_close_file(scfp_file file);
void         scfp_end_rule(scfp_rule rule);

#endif