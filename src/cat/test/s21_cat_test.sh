#!/bin/bash

SUCCESS_COUNTER=0
FAIL_COUNTER=0

TEST_FILE="ascii.txt"

FLAGS=(
	""
	"-b"
	"--number-nonblank"
	"-e"
	"-E"
	"-n"
	"--number"
	"-s"
	"--squeeze-blank"
	"-t"
	"-T"
)

# Заголовок таблицы
printf "%-20s %-10s\n" "Flag" "Result"
printf "%-20s %-10s\n" "-------------------" "---------"

for i in "${FLAGS[@]}"; do
	printf "%-20s " "$i"

	diff <(./s21_cat $i $TEST_FILE) <(cat $i $TEST_FILE) &>/dev/null

	if [ $? -eq 0 ]; then
		echo "SUCCESS"
		((SUCCESS_COUNTER++))
	else
		echo "FAIL"
		((FAIL_COUNTER++))
	fi
done

echo
echo "SUCCESS: $SUCCESS_COUNTER"
echo "FAIL: $FAIL_COUNTER"
