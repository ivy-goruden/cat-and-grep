#include "grep.h"

struct list_of_lists *add_special(struct list_of_lists *list,
                                  struct list *value) {
  struct list_of_lists *new_pattern = malloc(sizeof(struct list_of_lists));

  new_pattern->value = value;
  new_pattern->next = NULL;
  if (list == NULL) {
    list = new_pattern;
  } else if (list->value == NULL) {
    list->value = value;
    list->next = NULL;
  } else {
    struct list_of_lists *current = list;
    while (current->next != NULL) {
      current = current->next;
    }
    current->next = new_pattern;
  }
  return list;
}

struct list *add(struct list *list, char *value) {
  struct list *new_pattern = malloc(sizeof(struct list));

  if (!new_pattern) {
    return list;
  }
  new_pattern->value = strdup(value);
  new_pattern->next = NULL;
  new_pattern->index = 0;
  if (list == NULL) {
    list = new_pattern;
  } else {
    struct list *current = list;
    int index = 0;
    current->index = index;
    while (current->next != NULL) {
      current = current->next;
      index++;
      current->index = index;
    }
    current->next = new_pattern;
    new_pattern->index = index + 1;
  }
  return list;
}

struct list *get_at(struct list *list, int index) {
  struct list *current = list;
  for (int i = 0; i < index; i++) {
    if (current->next == NULL) {
      return NULL;
    }
    current = current->next;
  }
  return current;
}

void satisfy_valgrind(struct list_of_lists *list) {
  struct list_of_lists *current = list;
  struct list *value;
  while (current != NULL) {
    value = current->value;
    if (value != NULL) {
      free_list(value);
    }
    struct list_of_lists *next = current->next;
    free(current);
    current = next;
  }
}