#include <iostream>

using namespace std;

extern int test(void);

int main(void) 
{
    test();
    
    cout << "Hello World!" << endl;

#ifdef DEBUG_MODE
    cout << "DEBUG_MODE is defined" << endl;
#endif

#ifdef DEBUG_SIZE
    cout << "DEBUG_SIZE is defined: " << DEBUG_SIZE << endl;
#endif
    return 0;
}