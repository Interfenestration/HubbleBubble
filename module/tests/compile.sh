echo "compiling qqtest\n"
gcc -Wall -g -o qqtest qqtest.c
echo "compiling qqtest_onefork\n"
gcc -Wall -g -o qqtest_onefork qqtest_onefork.c
echo "compiling qqtest_twoforks\n"
gcc -Wall -g -o qqtest_twoforks qqtest_twoforks.c
