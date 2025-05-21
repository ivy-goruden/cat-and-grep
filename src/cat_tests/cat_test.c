#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_FLAGS 5
#define MAX_TESTS 32

const char *valid_flags[] = {"-b", "-e", "-n", "-s", "-t"};
const char *flag_desc[] = {"number nonblank", "show $ at end",
                           "number all lines", "squeeze blank",
                           "show tabs as ^I"};

void create_test_files() {
  FILE *f;

  // Create test file with various content
  f = fopen("test_cat1.txt", "w");
  fprintf(f, "First line\n\nThird line\twith tab\n\n\nLast line\n");
  fclose(f);

  // Create second test file
  f = fopen("test_cat2.txt", "w");
  fprintf(f, "\n\nEmpty at start\nMiddle\tline\n\n");
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

bool is_invalid_combination(const char *flags) {
  // -b and -n are mutually exclusive
  if (strstr(flags, "-b") && strstr(flags, "-n")) return true;
  return false;
}

void run_test(const char *flags, const char *filename) {
  if (!flags || !filename) return;
  if (is_invalid_combination(flags)) {
    return;
  }

  char cmd1[500], cmd2[500];
  FILE *f1 = NULL, *f2 = NULL;
  char line1[1000], line2[1000];
  int diff = 0;

  printf("\n=== Testing: cat %s %s ===\n", flags, filename);

  // Build commands
  snprintf(cmd1, sizeof(cmd1), "./s21_cat %s %s", flags, filename);
  snprintf(cmd2, sizeof(cmd2), "cat %s %s", flags, filename);

  // Run commands
  if (!(f1 = popen(cmd1, "r"))) {
    perror("Failed to run s21_cat");
    goto cleanup;
  }
  if (!(f2 = popen(cmd2, "r"))) {
    perror("Failed to run cat");
    goto cleanup;
  }

  // Compare outputs line by line
  int line_num = 1;
  while (true) {
    bool r1 = fgets(line1, sizeof(line1), f1) != NULL;
    bool r2 = fgets(line2, sizeof(line2), f2) != NULL;

    if (!r1 && !r2) break;

    if (r1 && !r2) {
      printf("Extra output from s21_cat at line %d: %s", line_num, line1);
      diff = 1;
      continue;
    }
    if (!r1 && r2) {
      printf("Missing output from s21_cat at line %d: %s", line_num, line2);
      diff = 1;
      continue;
    }

    // Remove newlines for comparison
    line1[strcspn(line1, "\n")] = 0;
    line2[strcspn(line2, "\n")] = 0;

    if (strcmp(line1, line2) != 0) {
      printf("Difference at line %d:\n", line_num);
      printf("%s\n", line1);
      printf("%s\n", line2);
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

  // Test files
  const char *filenames[] = {"test_cat1.txt", "test_cat2.txt", NULL};

  for (int i = 0; i < count && combinations[i]; i++) {
    for (int j = 0; filenames[j]; j++) {
      run_test(combinations[i], filenames[j]);
    }
  }

  // Clean up
  for (int i = 0; i < count; i++) {
    free(combinations[i]);
  }

  return 0;
}