#include <dirent.h>
#include <fnmatch.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
struct list {
  int index;
  char *value;
  struct list *next;
};

struct list_of_lists {
  struct list *value;
  struct list_of_lists *next;
};

struct Tulip {
  struct list *first;
  struct list *second;
};

struct list *list_all_files(char *path);
struct list *add(struct list *list, char *value);
struct list_of_lists *add_special(struct list_of_lists *list,
                                  struct list *value);
struct list *get_at(struct list *list, int index);

char *to_lower(char *string);
struct list *files_to_pattern(struct list *patterns, struct list *files);
void highlight_in_red(const char *line, const char *pattern);
int grep(FILE *file, char *value, struct list *patterns, int);
void free_list(struct list *list);
void print_list(struct list *list);
void extract_all_matches(char *str, char *pattern, int l, char *value, int);
char *safe_read();
char *safe_file_read(FILE *file);
void satisfy_valgrind(struct list_of_lists *list);
char *read_all_stdin();
int read_from_stdin(struct list *pat_list);
