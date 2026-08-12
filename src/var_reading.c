#include "scfp_config.h"

/// return line type
int _is_line_valid(const char* s) {
  int lineType = s ? scfp_unknow_line : scfp_empty_line;
  if (s) {
    if (*s == '#' || *s == '\n') { return scfp_comment_line; }
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

size_t skip_white_space(const char* s, size_t start) {
  size_t i = 0;
  if (!s || *s == '\n')
    return 0;
  while (isspace(s[i + start])) {
    i++;
  }
  return i;
}

static inline int is_tokens(int c) {
  unsigned short i = 0;
  while (TOKENS_LIST[i]) {
    if (TOKENS_LIST[i] == c)
      return c;
    i++;
  }
  return 0;
}

static inline ssize_t find_var_len(const char* s) {
  ssize_t i = 0;
  while ((s[i] && \
    !isspace(s[i]) && \
    isalnum(s[i]) && \
    !is_tokens(s[i])) \
    || s[i] == '_')
  { i++; }
  return i;
}

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


bool is_valid_keyword(const char* s, size_t i, doubleString_t keywords) {
  const size_t keywordLen = strlen(keywords[i]);
  if (strncmp(s, keywords[i], keywordLen) == 0) {
    return isspace(s[keywordLen]) || is_tokens(s[keywordLen]);
  }
  return false;
}

static int test_keyword(scfp_real_data_t* file, const char* s, size_t* dis, ssize_t* total, scfp_rules_t* rule) {
  const size_t len = find_var_len(s);
  *dis += len;
  size_t i = 0;
  for ( ; rule->keywords[i]; i++) {
    if (is_valid_keyword(s, i, rule->keywords)) { break ; }
  }
  printf(">>%zu\n", i);
  //return 1 = continue reading 
  // test system target
  if (i < rule->keywordSize) {
    printf("keyword -> [%zu]%s\n", i, rule->keywords[i]);
    (void)file;
    (void)total;
    return 0;
  }
  fprintf(stderr, "scfp: %.*s invalid keyword\n", (int)len, s);
  return 1;
  return 0;
}

ssize_t add_to_c(char* to, char c, size_t curentLen) {
  if (SCFP_VAR_SIZE - curentLen <= 0) {
    fprintf(stderr, "scfp: var is too long\n");
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
  add_to_c(file->buffer, toAdd, (*total)++);
  return 1;
}


size_t _get_value(scfp_real_data_t* file, ssize_t* total, const size_t start, const char* name, scfp_rules_t* rule) {
  if (*total >= SCFP_VAR_SIZE)
    return 0;
  size_t i = 0;
  const char* line = NULL;
  const size_t nameLen = find_var_len(name);
  while (file->data.raw[i]) {
    line = file->data.raw[i];
    if (i > start) { return nameLen; }
    if (strncmp(line, name, nameLen) == 0 && line[nameLen] == ':') { break ; }
    i++;
    if (!line) { return nameLen; }
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
        j += _get_value(file, total, i, line + j + TOKENSIZE, rule) + TOKENSIZE;
      }
      if (line[j] == '\\') {
        tokensInterpretor(line[j + 1], file, total, i);
        j += 2;
      }
      else {
        add_to_c(file->buffer, line[j], (*total)++);
        j++;
      }
    }
    (*total) -= remove_endl(file->buffer);
    line = file->data.raw[++i];
    if (_is_line_valid(line) == scfp_next_value) {
      j = skip_white_space(line, 0);
      nlValid = true;
    }
  } while (nlValid);
  return nameLen;
}