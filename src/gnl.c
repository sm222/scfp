# include "gnl.h"

# include <string.h>
# include <strings.h>
# include <unistd.h>
# include <stdlib.h>

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