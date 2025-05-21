all: s21_grep
	gcc grep_test.c -o test_my_grep
s21_grep: 
	gcc $(FLAGS) ../grep/grep.c ../grep/grep.h ../grep/list_all_files.c ../grep/list_functions.c -g -o s21_grep
clean: 
	rm -f *.txt test_my_grep s21_grep