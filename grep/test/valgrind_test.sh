#!/bin/bash

FILE="1.txt"
PATTERNS_FILE="patterns.txt"

SUCCESS_COUNTER=0
FAIL_COUNTER=0

FUNCTIONS=(
    "-e [a-g]"
    "-i g"
    "-v g"
    "-c g"
    "-l g"
    "-n g"
    "-h g"
    "-s g"
    "-f $PATTERNS_FILE g"
    "-o g"
)

# Check required files
[[ ! -f "$FILE" ]] && echo -e "\033[31mMissing $FILE\033[0m" && exit 1
[[ ! -f "$PATTERNS_FILE" ]] && echo -e "\033[31mMissing $PATTERNS_FILE\033[0m" && exit 1

for f in "${FUNCTIONS[@]}"; do
    IFS=' ' read -ra args <<< "$f"
    
    # Run Valgrind with error checking
    valgrind --leak-check=full \
             --show-leak-kinds=all \
             --track-origins=yes \
             --error-exitcode=1 \
             --quiet \
             ./s21_grep "${args[@]}" "$FILE" &>/dev/null

    VALGRIND_EXIT=$?
    
    if [[ $VALGRIND_EXIT -eq 0 ]]; then
        result="SUCCESS"
        color="\033[32m"
        ((SUCCESS_COUNTER++))
    else
        result="FAIL"
        color="\033[31m"
        ((FAIL_COUNTER++))
    fi
    
    # Print colored result
    echo -e "Valgrind check: ./s21_grep $f $FILE - ${color}$result\033[0m"
done

echo
echo "VALGRIND SUCCESS: $SUCCESS_COUNTER"
echo "VALGRIND FAIL: $FAIL_COUNTER"

# Exit with appropriate code
if [ $FAIL_COUNTER -gt 0 ]; then
    exit 1
else
    exit 0
fi