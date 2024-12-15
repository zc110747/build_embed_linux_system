
#include <iostream>
#include <vector>

int main(int argc, char *argv[])
{
    std::cout<<"start run crash!";

    std::vector<int> v(100 + static_cast<size_t>(-1) / sizeof(int));
    
    return 0;
}