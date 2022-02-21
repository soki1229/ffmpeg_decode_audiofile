#include <iostream>

#include "BaseModule.h"

int main(int, char**) {
    BaseModule* pBaseModule = BaseModule::getInstance();

    pBaseModule->decode();
    std::cout << "Hello, world!" <<endl;
}
