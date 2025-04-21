
#include <iostream>

extern void test_file1();
extern void test_file2();

int main()
{
    std::cout << "Hello World!\n";

    test_file1();

    test_file2();
    return 0;
}