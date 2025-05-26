#include "grep.h"

#include <ctype.h>
#include <regex.h>

int ignore_case = 0;           // -i
int number_of_lines_only = 0;  // -c
int invert_match = 0;          // -v
int number_lines = 0;          // -n
int hide_file_names = 0;       // -h
int surpess_errors = 0;        // -s
int output_matching_part = 0;  //-o
int FILE_MAX = 10000;
int print_only_files = 0;

void init(int argc, char *argv[], struct list **pat_list,
          struct list **lonely_args) {
  // Файлы, которые разбираются на паттерны
  struct list *file_list_f = NULL;

  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-' && argv[i][1] == '-') {
      printf("Неизвестный аргумент: %s\n", argv[i]);
    } else if (argv[i][0] == '-') {
      int x = 1;
      int a = i;
      while (1) {
        switch (argv[a][x]) {
          case 'e':
            if (argv[i + 1][0] != '-') {
              i++;
              *pat_list = add(*pat_list, argv[i]);
            }
            break;
          case 'f':
            if (argv[i + 1][0] != '-') {
              i++;
              file_list_f = add(file_list_f, argv[i]);
            }
            break;
          case 'i':
            ignore_case = 1;
            break;
          case 'c':
            number_of_lines_only = 1;
            break;
          case 'v':
            invert_match = 1;
            break;
          case 'n':
            number_lines = 1;
            break;
          case 'h':
            hide_file_names = 1;
            break;
          case 's':
            surpess_errors = 1;
            break;
          case 'o':
            output_matching_part = 1;
            break;
          case 'l':
            print_only_files = 1;
            break;
          default:
            printf("Неизвестный аргумент: %s\n", argv[i]);
            return;
        }
        if (argv[a][x + 1] != '\0') {
          x++;
        } else {
          break;
        }
      }

    }

    else {
      *lonely_args = add(*lonely_args, argv[i]);
    }
  }
  *pat_list = files_to_pattern(*pat_list, file_list_f);
}

int is_in_list(struct list *list, const char *value) {
  while (list != NULL) {
    if (strcmp(list->value, value) == 0) {
      return 1;
    }
    list = list->next;
  }
  return 0;
}

struct list *remove_substring_patterns(struct list *patterns) {
  // Step 1: Deduplicate the list
  struct list *unique_patterns = NULL;
  struct list *current = patterns;
  while (current != NULL) {
    if (!is_in_list(unique_patterns, current->value)) {
      unique_patterns = add(unique_patterns, strdup(current->value));
    }
    current = current->next;
  }

  // Step 2: Remove patterns subsumed by others
  struct list *result = NULL;
  current = unique_patterns;
  while (current != NULL) {
    const char *B = current->value;
    int is_redundant = 0;

    struct list *other = unique_patterns;
    while (other != NULL) {
      if (other == current) {
        other = other->next;
        continue;
      }
      const char *A = other->value;

      // Check if A covers B (e.g., A = "a.*", B = "abc")
      if (fnmatch(A, B, FNM_PATHNAME) == 0) {
        is_redundant = 1;
        break;
      }
      other = other->next;
    }

    if (!is_redundant) {
      result = add(result, strdup(B));
    }
    current = current->next;
  }

  // Free the temporary deduplicated list
  free_list(unique_patterns);
  return result;
}

int handle_lonely_args(struct list **lonely_args, struct list **pat_list,
                       struct list **files_patterns) {
  if (*lonely_args != NULL && (*lonely_args)->value != NULL) {
    // Add first lonely_arg to pat_list if pat_list is empty
    struct list *current = *lonely_args;
    if (*pat_list == NULL) {
      *pat_list = add(*pat_list, (*lonely_args)->value);
      current = (*lonely_args)->next;
    }

    // Add remaining lonely_args to files_patterns
    // Start from first->next
    while (current != NULL) {
      *files_patterns = add(*files_patterns, current->value);
      current = current->next;
    }

  }
  // Handle error case when no patterns specified
  else if (*lonely_args == NULL &&
           (*pat_list == NULL || (*pat_list)->value == NULL)) {
    printf("%s", "Error: No patterns specified\n");
    satisfy_valgrind(valgrind_list);
    return 1;
  }
  return 0;
}
int main(int argc, char *argv[]) {
  struct list *all_files_in_dir = NULL;
  // Файлы, которые проверяются на паттерны
  struct list *files_patterns = NULL;
  struct list *files = NULL;

  // Одинокие аргументы. Могут быть и файлами и паттернами
  struct list *lonely_args = NULL;
  // Паттерны
  struct list *pat_list = NULL;
  init(argc, argv, &pat_list, &lonely_args);
  int hla = handle_lonely_args(&lonely_args, &pat_list, &files_patterns);
  pat_list = remove_substring_patterns(pat_list);
  if (hla == 1) {
    return 1;
  }
  if (files_patterns == NULL) {
    read_from_stdin(pat_list);
    satisfy_valgrind(valgrind_list);
    return 0;
  } else {
    int file_num = 0;
    all_files_in_dir = list_all_files(".");
    for (int i = 0; get_at(files_patterns, i); i++) {
      for (int j = 0; get_at(all_files_in_dir, j); j++) {
        if (fnmatch(get_at(files_patterns, i)->value,
                    get_at(all_files_in_dir, j)->value, 0) == 0) {
          files = add(files, get_at(all_files_in_dir, j)->value);
          file_num++;
        }
      }
    }

    if (files == NULL) {
      read_from_stdin(pat_list);
      satisfy_valgrind(valgrind_list);
      return 0;
    } else {
      for (int f = 0; get_at(files, f); f++) {
        FILE *file = fopen(get_at(files, f)->value, "r");
        grep(file, get_at(files, f)->value, pat_list, file_num);
        fclose(file);
      }
    }
  }
  satisfy_valgrind(valgrind_list);
  return 0;
}

struct list *files_to_pattern(struct list *patterns, struct list *files) {
  for (int i = 0; get_at(files, i); i++) {
    FILE *file = fopen(get_at(files, i)->value, "r");
    if (file == NULL) {
      if (surpess_errors == 0) {
        printf("Не удалось открыть файл %s\n", get_at(files, i)->value);
      }
      continue;
    }
    char *file_str = safe_file_read(file);
    while (file_str != NULL) {
      file_str[strcspn(file_str, "\n")] = '\0';
      patterns = add(patterns, file_str);
      free(file_str);
      file_str = safe_file_read(file);
    }
    fclose(file);
    free(file_str);
  }
  return patterns;
}

void highlight_in_red(const char *line, const char *pattern) {
  const char *match = strstr(line, pattern);
  size_t len = strlen(line);
  if (match) {
    // Print part before the match
    printf("%.*s", (int)(match - line), line);
    // Print the match in red
    printf("%.*s", (int)strlen(pattern), match);
    // Print the rest of the line
    printf("%s", match + strlen(pattern));
  } else {
    printf("%s", line);
  }
  if (len > 0 && line[len - 1] != '\n') {
    printf("\n");
  }
}

int check_pattern(int *found, char *file_string, char *value, char *file_str,
                  char *pattern, int l, int file_num) {
  regex_t reegex;
  int flags = REG_EXTENDED;
  if (ignore_case) flags |= REG_ICASE;
  if (regcomp(&reegex, pattern, flags) != 0) {
    if (surpess_errors == 0) {
      printf("Не удалось скомпилировать регулярное выражение\n");
    }
    return 1;
  }
  int final_break = 0;
  if (regexec(&reegex, file_string, 0, NULL, 0) == 0) {
    *found = 1;
    if (invert_match == 0) {
      if (print_only_files == 1) {
        printf("%s\n", value);
        regfree(&reegex);
        return 1;
      }
      if (number_of_lines_only == 0) {
        if (output_matching_part == 0) {
          if (hide_file_names == 0 && file_num > 1) {
            printf("%s:", value);
          }
          if (number_lines == 1) {
            printf("%d:", l + 1);
          }
          highlight_in_red(file_str, pattern);
          final_break = 2;
        }

        else {
          extract_all_matches(file_str, pattern, l, value, file_num);
        }
      }
    }
  }
  regfree(&reegex);
  return final_break;
}

int if_invert(int found, int *number_of_lines, char *value, int l,
              char *file_str, int file_num) {
  if (found == 0 && invert_match == 1) {
    (*number_of_lines)++;
    if (!output_matching_part) {
      if (print_only_files == 1) {
        printf("%s\n", value);
        return 1;

      } else if (number_of_lines_only == 0) {
        if (hide_file_names == 0 && file_num > 1) {
          printf("%s:", value);
        }
        if (number_lines == 1) {
          printf("%d:", l + 1);
        }
        printf("%s", file_str);
        if (file_str[strlen(file_str) - 1] != '\n') {
          printf("\n");
        }
      }
    }
  } else if (found > 0 && invert_match == 0) {
    (*number_of_lines)++;
  }
  return 0;
}

int grep(FILE *file, char *value, struct list *patterns, int file_num) {
  int number_of_lines = 0;
  if (file == NULL) {
    if (surpess_errors == 0) {
      printf("Не удалось открыть файл %s\n", value);
    }
    return 1;
  }
  for (int l = 0;; l++) {
    char *file_str = safe_file_read(file);
    if (file_str == NULL) {
      free(file_str);
      break;
    }
    int found = 0;
    for (int p = 0; get_at(patterns, p); p++) {
      char *file_string = file_str;
      char *pattern = get_at(patterns, p)->value;
      int check = check_pattern(&found, file_string, value, file_str, pattern,
                                l, file_num);
      if (check == 1) {
        return 0;
      } else if (check == 2) {
        break;
      }
    }
    int a = if_invert(found, &number_of_lines, value, l, file_str, file_num);
    if (a == 1) {
      break;
    }
    free(file_str);
  }
  if (number_of_lines_only == 1 && print_only_files == 0) {
    if (hide_file_names == 0 && file_num > 1) {
      printf("%s:", value);
    }
    printf("%d\n", number_of_lines);
  }
  return 0;
}

char *to_lower(char *string) {
  char *lower_string = malloc(strlen(string) + 1);
  if (!lower_string) return NULL;

  for (int i = 0; string[i] != '\0'; i++) {
    lower_string[i] = tolower(string[i]);
  }
  lower_string[strlen(string)] = '\0';
  return lower_string;
}

void free_list(struct list *list) {
  struct list *current = list;
  if (current->value != NULL) {
    free(current->value);
  }
  free(current);
}

void print_list(struct list *list) {
  struct list *current = list;
  while (current != NULL) {
    printf("%s\n", current->value);
    current = current->next;
  }
}

void extract_all_matches(char *str, char *pattern, int l, char *value,
                         int file_num) {
  regex_t regex;
  regmatch_t match;
  int offset = 0;
  char *small_str = str;
  char *small_pattern = pattern;
  if (ignore_case == 1) {
    small_str = to_lower(str);
    small_pattern = to_lower(pattern);
  }
  if (regcomp(&regex, small_pattern, REG_EXTENDED) != 0) {
    fprintf(stderr, "Regex compilation failed\n");
    if (ignore_case == 1) {
      free(small_str);
      free(small_pattern);
    }
    return;
  }
  while (regexec(&regex, small_str + offset, 1, &match, 0) == 0) {
    int start = offset + match.rm_so;
    int end = offset + match.rm_eo;
    if (hide_file_names == 0 && file_num > 1) {
      printf("%s:", value);
    }
    if (number_lines == 1) {
      printf("%d:", l + 1);
    }
    printf("%.*s\n", end - start, str + start);
    offset = end;
  }

  regfree(&regex);
  if (ignore_case == 1) {
    free(small_str);
    free(small_pattern);
  }
}

char *safe_read() {
  char *string = malloc(FILE_MAX);
  char *input = malloc(FILE_MAX);
  int len = 0;
  int end = 0;
  int iteration = 1;
  while (fgets(input, FILE_MAX, stdin) != NULL) {
    int add_len = len;
    for (int i = 0; i < FILE_MAX; i++) {
      if (input[i] == '\0') {
        string[i + add_len] = '\0';
        end = 1;
        break;
      }
      string[i + add_len] = input[i];
      len++;
    }
    if (end == 1) {
      string = realloc(string, len++);
      free(input);
      return string;
    } else {
      string = realloc(string, FILE_MAX * (iteration++));
    }
  }
  free(input);
  string[0] = '\0';
  char *tmp = realloc(string, 1);
  if (!tmp) {
    return NULL;
  }
  return tmp;
}

char *safe_file_read(FILE *file) {
  char *string = calloc(FILE_MAX, 1);
  char *input = calloc(FILE_MAX, 1);
  if (!string || !input) {
    free(string);
    free(input);
    return NULL;
  }

  int len = 0;
  while (fgets(input, FILE_MAX, file) != NULL) {
    strcat(string, input);
    len += strlen(input);
    if (strchr(input, '\0')) break;
  }

  free(input);
  if (len == 0) {
    free(string);
    return NULL;
  }
  return string;
}
char *read_all_stdin() {
  size_t size = 1024;
  size_t len = 0;
  char *buffer = malloc(size);
  if (!buffer) return NULL;

  int ch = getchar();
  if (ch == EOF) {
    free(buffer);
    return NULL;
  }

  if (ch == '\n') {
    buffer[0] = '\0';
    return buffer;
  }
  while (ch != '\n' && ch != EOF) {
    if (len + 1 >= size) {
      size *= 2;
      char *new_buffer = realloc(buffer, size);
      if (!new_buffer) {
        free(buffer);
        return NULL;
      }
      buffer = new_buffer;
    }
    buffer[len++] = ch;
    ch = getchar();
  }

  buffer[len] = '\0';
  return buffer;
}

int read_from_stdin(struct list *pat_list) {
  printf("Не удалось найти файлы, соответствующие паттернам\n");
  printf("Идёт считывание с ввода. Для завершения введите 'q'\n");
  while (1) {
    int ch = getchar();
    char *new = malloc(sizeof(char) * 2);
    if (!new) {
      return 1;
    }
    new[0] = (char)ch;
    new[1] = '\0';
    char *string = read_all_stdin();
    if (string != NULL) {
      char *result = malloc(2 + strlen(string) + 1);
      if (!result) {
        free(string);
        free(new);
        return 1;
      }
      strcpy(result, new);
      strcat(result, string);
      if (result[0] == 'q' && result[1] == '\0') {
        free(string);
        free(result);
        free(new);
        return 1;
      }
      FILE *fake_file = fmemopen(result, strlen(result) + 1, "r");
      if (!fake_file) {
        free(new);
        free(string);
        free(result);
        return 1;
      }
      grep(fake_file, "", pat_list, 0);
      fclose(fake_file);
      free(string);
      free(result);
    }
    free(new);
  }
  return 0;
}