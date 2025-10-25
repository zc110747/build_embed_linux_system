
#include <iostream>
#include "includes.h"

class A {
public:
    A() {
        std::cout << "A::A()" << std::endl;
    }
    ~A() {
        std::cout << "A::~A()" << std::endl;
    }
    void show() {
        std::cout << "A::show()" << std::endl;
    }
};

static A *a = nullptr;

int a_init(void) 
{
    a = new A();

    return 0;
}

int a_show(void)
{
    if (a != nullptr) {
        a->show();
    }

    return 0;
}

void a_release(void)
{
    if (a != nullptr) {
        delete a;
        a = nullptr;
    }
}