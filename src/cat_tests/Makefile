all: s21_cat
	gcc cat_test.c -o test_my_cat
s21_cat: 
	gcc $(FLAGS) ../cat/cat.c ../cat/cat.h -o s21_cat
clean: 
	rm -f *.txt s21_cat test_my_cat