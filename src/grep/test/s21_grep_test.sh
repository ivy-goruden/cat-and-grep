#!/bin/bash

SUCCESS_COUNTER=0
FAIL_COUNTER=0

TEST_TEXT_FILES=(
	"lorem.txt"
	"from_developers.txt"
	"empty.txt"
)

TEST_PATTERNS=(
	"lorem"
	"[a-z]"
)

TEST_PATTERN_FILES=(
	"patterns.txt"
)

FLAGS=(
	""
	"-e"
	"-i"
	"-v"
	"-c"
	"-l"
	"-n"
	"-h"
	"-s"
	"-f"
)

# Заголовок таблицы
FORMAT="%-10s %-50s %-40s %-10s\n"
echo
printf "$FORMAT" "Flag" "Pattern" "File" "Result"
printf "$FORMAT" "--------" "------------------------------------------------" "--------------------------------------" "---------"

run_test() {
	option=$1
	pattern=$2
	file=$3
	diff <(./s21_grep $option $pattern $file) <(grep $option $pattern $file) &>/dev/null
	if [ $? -eq 0 ]; then
		result="SUCCESS"
		((SUCCESS_COUNTER++))
	else
		result="FAIL"
		((FAIL_COUNTER++))
	fi

	printf "$FORMAT" "$option" "$pattern" "$file" "$result"
}

for file in "${TEST_TEXT_FILES[@]}"; do
	for pat_1 in "${TEST_PATTERNS[@]}"; do
		for pat_2 in "${TEST_PATTERNS[@]}"; do
			for pat_f1 in "${TEST_PATTERN_FILES[@]}"; do
				for pat_f2 in "${TEST_PATTERN_FILES[@]}"; do
					for op1 in "${FLAGS[@]}"; do
						for op2 in "${FLAGS[@]}"; do
							option=""
							pattern=""

							if [ "$op1" == "-e" ] || [ "$op2" == "-e" ]; then
								if [ "$op1" == "-e" ] && [ "$op2" == "-e" ]; then
									pattern="-e $pat_1 -e $pat_2"
								else
									pattern="-e $pat_1"
								fi
							fi

							if [ "$op1" == "-f" ] || [ "$op2" == "-f" ]; then
								if [ "$op1" == "-f" ] && [ "$op2" == "-f" ]; then
									pattern="-f $pat_f1 -f $pat_f2"
								else
									pattern="-f $pat_f1"
								fi
							fi

							if [ "$op1" != "-f" ] && [ "$op1" != "-e" ] && [ "$op2" != "-f" ] && [ "$op2" != "-e" ]; then
								option="$op1 $op2"
							elif [ "$op1" != "-f" ] && [ "$op1" != "-e" ]; then
								option="$op1"
							elif [ "$op2" != "-f" ] && [ "$op2" != "-e" ]; then
								option="$op2"
							fi

							if [ -z "$pattern" ]; then
								pattern="$pat_1"
							fi


							# printf "vars: $file $pat_1 $pat_2 $pat_f1 $pat_f2 $op1 $op2\n"
							# printf "args: $option $pattern $file\n"

							run_test "$option" "$pattern" "$file"
						done
					done
				done
			done
		done
	done
done

echo
echo "SUCCESS: $SUCCESS_COUNTER"
echo "FAIL: $FAIL_COUNTER"