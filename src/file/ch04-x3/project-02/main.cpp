#include <iostream>

using namespace std;

extern int test(void);

int main(void) 
{
    cout<< __STDC_HOSTED__ << endl;
    cout<< __cplusplus << endl;

    test();
    return 0;
}