#include <iostream>
#include <vector>

#include "foo.h"

#include <dlfcn.h>

static const char *LIBNAME = "./libbaz.so";

typedef void (*bazFunc)(int);

int main(void)
{
    Foo::instantiate();
    Foo::instance()->foo(5);

    std::cout << "main: Foo ptr: " << Foo::instance() << std::endl;

    void *lib = dlopen(LIBNAME, RTLD_NOW);
    if (!lib) {
        std::cerr << "dlerror: " << dlerror() << std::endl;
        return 1;
    }

    bazFunc baz = reinterpret_cast<bazFunc>(dlsym(lib, "baz"));
    if (!baz) {
        std::cerr << "Failed to resolv baz" << std::endl;
        return 1;
    }

    baz(2);

    // do not close library manually, it will be closed on exit
    // dlclose(lib);

    return 0;
}
