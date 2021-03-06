# http://c.learncodethehardway.org/book/ex28.html
path=${0%%/*}

printf "Running unit tests: \n\n"

for i in $path/*_tests
do
    if test -f $i
    then
        printf "$i "
        if $VALGRIND ./$i 2>> tests/tests.log
        then
            printf " PASS\n"
        else
            printf " ERROR see tests/tests.log\n"
            printf "  --------\n"
            tail tests/tests.log
            exit 1
        fi
    fi
done

printf "\nAll good!\n"
