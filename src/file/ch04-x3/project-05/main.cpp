
#include <iostream>
#include "test.hpp"

extern int test1_func();

int main() 
{
    std::cout << "Hello, World!" << std::endl;

    test1_func();
    return 0;
}