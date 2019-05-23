#include <iostream>

#include "foo.h"

extern "C" void baz(int b)
{
    Foo *inst = Foo::instance();
    std::cout << "baz: Foo ptr: " << inst << std::endl;
    inst->foo(b);
}
