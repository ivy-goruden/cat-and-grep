FLAGS= -Wall -Werror -Wextra

all: s21_cat

s21_cat: cat.c cat.h
	gcc $(FLAGS) cat.c cat.h -o s21_cat
	gcc $(FLAGS) cat.c cat.h -o test/s21_cat

clean:
	rm -rf s21_cat s21_cat cat
