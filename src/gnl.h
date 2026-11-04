#ifndef  _GNL_H_
# define _GNL_H_

# ifndef  SCFP_READ_SIZE
#  define SCFP_READ_SIZE 200
# endif

# define NL '\n'
# define B_SIZE 101
# define BUFFER_SIZE 200

char* get_next_line(int fd);

#endif