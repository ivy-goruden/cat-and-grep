#include "grep.h"

struct list *list_all_files(char *path) {
  struct list *new_list = NULL;
  struct list *error = NULL;
  DIR *dir = opendir(path);
  if (!dir) {
    printf("Не удалось открыть дирректорию\n");
    return error;
  }
  struct dirent *entry;
  while ((entry = readdir(dir))) {
    if (entry->d_name[0] == '.') {
      continue;
    }
    char *name_copy = strdup(entry->d_name);
    if (!name_copy) continue;
    new_list = add(new_list, name_copy);
    free(name_copy);
  }
  closedir(dir);
  return new_list;
}