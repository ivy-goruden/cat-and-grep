FLAGS= -Wall -Werror -Wextra -std=c11

all: s21_grep

clean:
	rm -rf grep/s21_grep s21_grep grep test/s21_grep

s21_grep: grep.c grep.h
	gcc $(FLAGS) grep.c grep.h list_all_files.c list_functions.c -g -o s21_grep
	gcc $(FLAGS) grep.c grep.h list_all_files.c list_functions.c -g -o test/s21_grep
