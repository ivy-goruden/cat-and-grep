#include "cat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

FILE *input_file = NULL;
int end_of_the_line = 0;
int number_blank_lines = 0;
int squeze_blank_lines = 0;
int tab = 0;
int number_lines = 0;
int MAX_FILE_SIZE = 100;
int all_symbols_to_visible = 0;

void init(int argc, char *argv[]) {
  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-' && argv[i][1] == '-') {
      if (strcmp(argv[i], "--number-nonblank") == 0) {
        number_lines = 1;
      } else if (strcmp(argv[i], "--number") == 0) {
        number_blank_lines = 1;
      } else if (strcmp(argv[i], "--squeeze-blank") == 0) {
        squeze_blank_lines = 1;
      } else {
        printf("Неизвестный аргумент: %s\n", argv[i]);
      }
    } else if (argv[i][0] == '-') {
      switch (argv[i][1]) {
        case 'b':
          number_lines = 1;
          break;
        case 'e':
          end_of_the_line = 1;
          all_symbols_to_visible = 1;
          break;
        case 'E':
          end_of_the_line = 1;
          break;
        case 'n':
          number_blank_lines = 1;
          break;
        case 's':
          squeze_blank_lines = 1;
          break;
        case 't':
          tab = 1;
          all_symbols_to_visible = 1;
          break;
        case 'T':
          tab = 1;
          break;
        case '\0':
          continue;
        default:
          printf("Flag {%c} does not exist", argv[i][1]);
          return;
      }
    } else {
      // there must be a file
      if (input_file == NULL) {
        input_file = fopen(argv[i], "r");
      } else {
        printf("Flag {%s} does not exist", argv[i]);
        return;
      }
    }
  }
}

int main(int argc, char *argv[]) {
  init(argc, argv);
  int line = 1;
  char text_line[MAX_FILE_SIZE];
  if (input_file) {
    char text[MAX_FILE_SIZE];
    int blank_line = 1;
    while (fgets(text, MAX_FILE_SIZE, input_file) != NULL) {
      cat_t result = process_function(text, line, blank_line);
      blank_line = result.blank_line;
      line = result.k;
    }
    fclose(input_file);

  } else {
    while (1) {
      scanf("%s", text_line);
      process_function(text_line, line, 1);
      printf("\n");
      line++;
    }
  }

  return 0;
}

void add_number_label(char *line, int *k, int line_number) {
  char num_buffer[16];
  int chars_written = sprintf(num_buffer, "%6d\t", line_number);
  strcpy(&line[*k], num_buffer);
  (*k) += chars_written;
}

void end_of_the_linef(char *line, int *k) {
  if (end_of_the_line == 1) {
    line[*k] = '$';
    (*k)++;
    line[*k] = '\n';
    (*k)++;
  } else {
    line[*k] = '\n';
    (*k)++;
  }
}

void make_readable(const char str, char *line, int *k) {
  unsigned char ch = (unsigned char)str;

  // Обработка символов с 8-м битом (≥ 128)
  if (ch >= 128) {
    line[(*k)++] = 'M';
    line[(*k)++] = '-';
    ch -= 128;
  }

  // Замена непечатаемых символов (кроме таба)
  if ((ch < 32 || ch == 127) && ch != '\t') {
    line[(*k)++] = '^';
    line[(*k)++] = (ch == 127) ? '?' : (ch + 64);
  }
  // Печатаемые символы (включая таб)
  else {
    line[(*k)++] = ch;
  }
}

cat_t process_function(char *string, int line_number, int blank_line) {
  int char_lenght = 0;
  for (int i = 0; string[i] != '\0'; i++) {
    char_lenght++;
  }
  int k = 0;
  char *line = malloc((char_lenght * 4 + 20) * sizeof(char));
  if (blank_line == 0) {
    printf("\n");
    blank_line = 1;
  }
  for (int i = 0; string[i] != '\0'; i++) {
    if (blank_line > 0) {
      if (string[i] == '\n') {
        if (blank_line == 1 || squeze_blank_lines == 0) {
          if (number_blank_lines == 1) {
            add_number_label(line, &k, line_number);
          } else {
            line_number--;
          }
          end_of_the_linef(line, &k);
        } else {
          line_number--;
        }
        blank_line += 1;
      } else {
        if (number_lines == 1 || number_blank_lines == 1) {
          add_number_label(line, &k, line_number);
        }
        blank_line = 0;
      }
    }
    if (string[i] == '\t' && tab == 1) {
      line[k] = '^';
      k++;
      line[k] = 'I';
      k++;
    } else if (string[i] == '\n') {
      if (blank_line == 0) {
        end_of_the_linef(line, &k);
      }
      blank_line += 1;
    } else {
      if (all_symbols_to_visible == 1) {
        make_readable(string[i], line, &k);
      } else {
        line[k] = string[i];
        k++;
      }

      blank_line = 0;
    }
  }
  line_number++;
  line[k] = '\0';
  printf("%s", line);
  free(line);
  cat_t result = {line_number, blank_line};
  return result;
}
