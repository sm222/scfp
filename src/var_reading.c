#include "scfp_config.h"

# define MAX_RECURSIVE 5000
//put that in stuck

/// return line type
// good
int _is_line_valid(const char* s) {
  int lineType = s ? scfp_unknow_line : scfp_empty_line;
  if (s) {
    if (*s == '#' || *s == '\n') {
      return *s == '#' ? scfp_comment_line : scfp_empty_line;
    }
    else if (isdigit(*s) || *s == ':' ) { return scfp_invalid_line; }
    else if (isspace(*s))  { lineType = scfp_next_value; }
    else if (isalpha(*s))  { lineType = scfp_var_line;   }
    else if (!isalpha(*s)) { return  scfp_invalid_line;  }
    size_t i = 0;
    while (s[i]) {
      if ((!isspace(s[i]) && lineType == scfp_next_value) || \
      (s[i] == ':' && lineType == scfp_var_line)) {
        return lineType;
      }
      if (isspace(s[i]) && lineType == scfp_var_line)
        break ;
      i++;
    }
    lineType *= -1; //! flip value to show error
  }
  return lineType;
}

//good
size_t skip_white_space(const char* s, size_t start) {
  size_t i = 0;
  if (!s || *s == '\n')
    return 0;
  while (isspace(s[i + start])) {
    i++;
  }
  return i;
}

//good
static inline int is_tokens(int c) {
  unsigned short i = 0;
  while (TOKENS_LIST[i]) {
    if (TOKENS_LIST[i] == c)
      return c;
    i++;
  }
  return 0;
}

//good
ssize_t find_var_len(const char* s) {
  ssize_t i = 0;
  while ((s[i] && \
    !isspace(s[i]) && \
    isalnum(s[i]) && \
    !is_tokens(s[i])) \
    || s[i] == '_')
  { i++; }
  return i;
}

//good
int remove_endl(char* value) {
  if (!value)
    return 0;
  ssize_t l = s_strlen(value);
  for (; l >= 0; l--) {
    if (value[l] == '\n') {
      value[l] = 0;
      return 1;
    }
  }
  return 0;
}

//good
bool is_valid_keyword(const char* s, size_t i, doubleString_t keywords) {
  const size_t keywordLen = strlen(keywords[i]);
  if (strncmp(s, keywords[i], keywordLen) == 0) {
    return isspace(s[keywordLen]) || is_tokens(s[keywordLen]);
  }
  return false;
}

//!not finish
static int test_keyword(scfp_real_data_t* file, const char* s, size_t* dis, ssize_t* total, scfp_rules_t* rule) {
  const size_t len = find_var_len(s) - 1;
  *dis += len;
  size_t i = 0;
  for ( ; rule->keywords[i]; i++) {
    if (is_valid_keyword(s, i, rule->keywords)) { break ; }
  }
  //return 1 = continue reading 
  // test system target
  if (i < rule->keywordSize) {
    //printf("keyword -> [%zu]%s\n", i, rule->keywords[i]);
    (void)file;
    (void)total;
    return 0;
  }
  file->status = scfp_invalid_file;
  fprintf(stderr, "scfp: %.*s invalid keyword\n", (int)len, s);
  return 1;
}

ssize_t add_to(scfp_real_data_t* file, char* to, size_t curentLen, const char* s) {
  if (SCFP_VAR_SIZE - (int)curentLen < 0) {
    if (file->status == scfp_valid_file) {
      fprintf(stderr, "scfp: var is too long\n");
      file->status = scfp_buffer_full;
    }
  }
  // + 1 for null byte
  const ssize_t len = s_strlen(s) + 1;
  if ((int)(curentLen + len) >= SCFP_VAR_SIZE) {
    fprintf(stderr, "scfp: can't add str\n");
    return 0;
  }
  memcpy(to + curentLen, s, len);
  return len;
}

ssize_t add_to_c(scfp_real_data_t* file, char* to, char c, size_t curentLen) {
  if (SCFP_VAR_SIZE - (int)curentLen < 0) {
    if (file->status == scfp_valid_file) {
      fprintf(stderr, "scfp: var is too long\n");
      file->status = scfp_buffer_full;
    }
    to[SCFP_VAR_SIZE] = 0;
    return 0;
  }
  to[curentLen] = c;
  return 1;
}

static ssize_t tokensInterpretor(char t, scfp_real_data_t* file, ssize_t* total, size_t i) {
  char toAdd = ' ';
  switch (t) {
    case '\\':
    case ';':
      toAdd = t;
      break ;
    case 'n':
      toAdd = '\n';
      break ;
    case ' ':
      return 0;
    break ;
    default:
      fprintf(stderr, "warning: line %zu unknown token ascii[%d]\\%c replace by space\n", ++i , t, t);
  }
  return add_to_c(file ,file->buffer, toAdd, (*total)++);;
}

const char* _read_predefine_var(scfp_rules_t* rules, const char* name);


size_t _get_value(scfp_real_data_t* file, ssize_t* total, const char* name, scfp_rules_t* rule) {
  if ((int)*total > SCFP_VAR_SIZE || file->data.dep == MAX_RECURSIVE)
    return 0;
  file->data.dep++;
  const char* line = NULL;
  const size_t nameLen = find_var_len(name);
  size_t i = 0;
  while (file->data.raw[i]) {
    line = file->data.raw[i];
    //if (i > start) { return nameLen; }
    if (strncmp(line, name, nameLen) == 0 && line[nameLen] == ':') { break ; }
    i++;
    //fprintf(stderr, "%s\n", file->data.raw[i]);
    if (!file->data.raw[i]) {
      const char* var = _read_predefine_var(rule, name);
      (*total) += add_to(file, file->buffer, *total, var);
      return nameLen;
    }
  }
  size_t j = nameLen + 1;
  bool nlValid = false;
  do {
    const size_t lineLen = s_strlen(line);
    nlValid = false;
    while (j < lineLen) {
      if (line[j] == rule->escape) {
        if (line[j + 1] == '_') {
          if (test_keyword(file, line + j + (TOKENSIZE * 2), &j, total, rule))
            break ;
        }
        j += _get_value(file, total, line + j + TOKENSIZE, rule) + TOKENSIZE;
      }
      if (line[j] == '\\') {
        tokensInterpretor(line[j + 1], file, total, i);
        j += 2;
      }
      else {
        add_to_c(file, file->buffer, line[j], (*total));
        (*total)++;
        j++;
      }
    }
    (*total) -= remove_endl(file->buffer);
    line = file->data.raw[i] ? file->data.raw[i + 1] : NULL;
    i++;
    if (_is_line_valid(line) == scfp_next_value) {
      j = skip_white_space(line, 0);
      nlValid = true;
    }
  } while (nlValid);
  return nameLen;
}