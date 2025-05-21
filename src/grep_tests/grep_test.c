#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_FLAGS 8
#define MAX_TESTS 256

const char *valid_flags[] = {"-i", "-c", "-v", "-n", "-h", "-s", "-o", "-l"};
const char *flag_desc[] = {"ignore case",    "count lines",
                           "invert match",   "line numbers",
                           "hide filenames", "suppress errors",
                           "output matches", "print filenames only"};

void create_test_files() {
  FILE *f;

  f = fopen("test1.txt", "w");
  fprintf(f, "Hello World\nThis is a Test\nLine with hello\nGoodbye\n");
  fclose(f);

  f = fopen("test2.txt", "w");
  fprintf(f, "HELLO world\nAnother TEST\nline with Hello\nGOODBYE\n");
  fclose(f);

  f = fopen("patterns.txt", "w");
  fprintf(f, "Hello\nTest\n");
  fclose(f);
}

void generate_flag_combinations(int index, char *current, char **combinations,
                                int *count) {
  if (index == MAX_FLAGS) {
    if (strlen(current) > 0) {
      combinations[*count] = strdup(current);
      (*count)++;
    }
    return;
  }

  // Path without current flag
  generate_flag_combinations(index + 1, current, combinations, count);

  // Path with current flag
  char new[100];
  if (strlen(current) == 0) {
    snprintf(new, sizeof(new), "%s", valid_flags[index]);
  } else {
    snprintf(new, sizeof(new), "%s %s", current, valid_flags[index]);
  }
  generate_flag_combinations(index + 1, new, combinations, count);
}

bool is_invalid_combination(const char *flags, const char *pattern) {
  // -l with -o doesn't make sense
  if (strstr(flags, "-l") && strstr(flags, "-o")) return true;

  // -c with -n doesn't make sense
  if (strstr(flags, "-c") && strstr(flags, "-n")) return true;

  // -f pattern with other patterns
  if (strcmp(pattern, "-f patterns.txt") == 0 && strstr(flags, "-e"))
    return true;

  return false;
}

void run_test(const char *flags, const char *pattern) {
  if (!flags || !pattern) return;
  if (is_invalid_combination(flags, pattern)) {
    return;
  }

  char cmd1[500], cmd2[500];
  FILE *f1 = NULL, *f2 = NULL;
  char line1[1000], line2[1000];
  int diff = 0;

  printf("\n=== Testing: s21_grep %s '%s' test1.txt test2.txt ===\n", flags,
         pattern);

  // Build commands
  if (strcmp(pattern, "-f patterns.txt") == 0) {
    snprintf(cmd1, sizeof(cmd1),
             "./s21_grep %s -f patterns.txt test1.txt test2.txt", flags);
    snprintf(cmd2, sizeof(cmd2), "grep %s -f patterns.txt test1.txt test2.txt",
             flags);
  } else {
    snprintf(cmd1, sizeof(cmd1), "./s21_grep %s \"%s\" test1.txt test2.txt",
             flags, pattern);
    snprintf(cmd2, sizeof(cmd2), "grep %s \"%s\" test1.txt test2.txt", flags,
             pattern);
  }

  // Run commands
  if (!(f1 = popen(cmd1, "r"))) {
    perror("Failed to run s21_grep");
    goto cleanup;
  }
  if (!(f2 = popen(cmd2, "r"))) {
    perror("Failed to run grep");
    goto cleanup;
  }

  // Compare outputs
  int line_num = 1;
  while (true) {
    bool r1 = fgets(line1, sizeof(line1), f1) != NULL;
    bool r2 = fgets(line2, sizeof(line2), f2) != NULL;

    if (!r1 && !r2) break;

    if (r1 && !r2) {
      printf("Extra output from s21_grep at line %d: %s", line_num, line1);
      diff = 1;
      continue;
    }
    if (!r1 && r2) {
      printf("Missing output from s21_grep at line %d: %s", line_num, line2);
      diff = 1;
      continue;
    }

    // Remove newlines for comparison
    line1[strcspn(line1, "\n")] = 0;
    line2[strcspn(line2, "\n")] = 0;

    if (strcmp(line1, line2) != 0) {
      printf("Difference at line %d:\n", line_num);
      printf("s21_grep: %s\n", line1);
      printf("grep:     %s\n", line2);
      diff = 1;
    }
    line_num++;
  }

  if (!diff) {
    printf("SUCCESS - outputs match!\n");
  }

cleanup:
  if (f1) pclose(f1);
  if (f2) pclose(f2);
}

int main() {
  char *combinations[MAX_TESTS] = {0};
  int count = 0;

  create_test_files();

  // Generate flag combinations including empty combination
  generate_flag_combinations(0, "", combinations, &count);

  // Add empty flags case
  combinations[count++] = strdup("");

  // Test patterns
  const char *patterns[] = {"Hello",           "Test", "hello", "test",
                            "-f patterns.txt", NULL};

  for (int i = 0; i < count && combinations[i]; i++) {
    for (int j = 0; patterns[j]; j++) {
      run_test(combinations[i], patterns[j]);
    }
  }

  // Clean up
  for (int i = 0; i < count; i++) {
    free(combinations[i]);
  }

  return 0;
}