#include <iostream>
#include <vector>

class Foo
{
public:
    Foo();
    ~Foo();

    static void instantiate();

    static Foo *instance();
    void foo(int val);

private:
    static const std::vector<const char *> mStaticVector;

    static Foo *inst;
    int mVal;
    const char *mStr = "";
};
