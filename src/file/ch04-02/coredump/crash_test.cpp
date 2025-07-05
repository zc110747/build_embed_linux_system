
#include <iostream>
#include <vector>

int main(int argc, char *argv[])
{
    int *p = nullptr;
    std::vector<int *> vec;

    std::cout<<"start run crash!";

    p = new int(5);
    *p = 5;

    delete p;
    p = nullptr;

    vec.push_back(p);

    std::cout<<*vec[0]<<std::endl;

    
    return 0;
}
