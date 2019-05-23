#include <iostream>
#include <vector>

#include "foo.h"

const std::vector<const char *> Foo::mStaticVector __attribute__((visibility ("default"))) = {"a", "b", "c", "d", "e", "f", "g"};

Foo *Foo::inst = nullptr;

Foo::Foo()
{
    std::cout << "Foo" << std::endl;
}

Foo::~Foo()
{
    std::cout << "~Foo" << std::endl; 
    if (inst) { 
        delete inst; 
        inst = NULL;
    }
}

void Foo::instantiate()
{
    if (!inst)
        inst = new Foo();
}

Foo *Foo::instance()
{
    if (!inst) {
       inst = new Foo();
       std::cerr << "No instantiate() called!" << std::endl;
    }
    return inst;
}

void Foo::foo(int val)
{
    mVal = val;
    std::cout << "Foo::bar " << val << std::endl;
    if (mVal >= 0 && mVal < mStaticVector.size())
        mStr = mStaticVector[mVal];
}
