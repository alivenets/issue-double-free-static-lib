# Double free issue when mixing static and dynamic libraries

## Desctiption

This code reproduces double-free issue with static libraries linked to binary and dynamically loaded shared library.

Foo - static library with statically initialized objects
Bar - binary using static library foo, and dynamically loading library baz
Bar (No Foo) - binary without foo m dynamically loading baz
Baz - shared library using static library bar

When running `valgrind ./bar`, valgrind shows double-free issue, output is similar:

```
==12432== Invalid free() / delete / delete[] / realloc()
==12432==    at 0x4C3123B: operator delete(void*) (in /usr/lib/valgrind/vgpreload_memcheck-amd64-linux.so)
==12432==    by 0x10AD05: __gnu_cxx::new_allocator<char const*>::deallocate(char const**, unsigned long) (new_allocator.h:125)
==12432==    by 0x10AC48: std::allocator_traits<std::allocator<char const*> >::deallocate(std::allocator<char const*>&, char const**, unsigned long) (alloc_traits.h:462)
==12432==    by 0x10AAFD: std::_Vector_base<char const*, std::allocator<char const*> >::_M_deallocate(char const**, unsigned long) (stl_vector.h:180)
==12432==    by 0x10A992: std::_Vector_base<char const*, std::allocator<char const*> >::~_Vector_base() (stl_vector.h:162)
==12432==    by 0x10A8AE: std::vector<char const*, std::allocator<char const*> >::~vector() (stl_vector.h:435)
==12432==    by 0x5624040: __run_exit_handlers (exit.c:108)
==12432==    by 0x5624139: exit (exit.c:139)
==12432==    by 0x5602B9D: (below main) (libc-start.c:344)
==12432==  Address 0x5d824a0 is 0 bytes inside a block of size 56 free'd
==12432==    at 0x4C3123B: operator delete(void*) (in /usr/lib/valgrind/vgpreload_memcheck-amd64-linux.so)
==12432==    by 0x10AD05: __gnu_cxx::new_allocator<char const*>::deallocate(char const**, unsigned long) (new_allocator.h:125)
==12432==    by 0x10AC48: std::allocator_traits<std::allocator<char const*> >::deallocate(std::allocator<char const*>&, char const**, unsigned long) (alloc_traits.h:462)
==12432==    by 0x10AAFD: std::_Vector_base<char const*, std::allocator<char const*> >::_M_deallocate(char const**, unsigned long) (stl_vector.h:180)
==12432==    by 0x10A992: std::_Vector_base<char const*, std::allocator<char const*> >::~_Vector_base() (stl_vector.h:162)
==12432==    by 0x10A8AE: std::vector<char const*, std::allocator<char const*> >::~vector() (stl_vector.h:435)
==12432==    by 0x5624614: __cxa_finalize (cxa_finalize.c:83)
==12432==    by 0x6172662: ???
==12432==    by 0x4015C7B: _dl_close_worker.part.0 (dl-close.c:288)
==12432==    by 0x4016AA9: _dl_close_worker (dl-close.c:125)
==12432==    by 0x4016AA9: _dl_close (dl-close.c:842)
==12432==    by 0x57482DE: _dl_catch_exception (dl-error-skeleton.c:196)
==12432==    by 0x574836E: _dl_catch_error (dl-error-skeleton.c:215)
==12432==  Block was alloc'd at
==12432==    at 0x4C3017F: operator new(unsigned long) (in /usr/lib/valgrind/vgpreload_memcheck-amd64-linux.so)
==12432==    by 0x10AD49: __gnu_cxx::new_allocator<char const*>::allocate(unsigned long, void const*) (new_allocator.h:111)
==12432==    by 0x10AC93: std::allocator_traits<std::allocator<char const*> >::allocate(std::allocator<char const*>&, unsigned long) (alloc_traits.h:436)
==12432==    by 0x10AB97: std::_Vector_base<char const*, std::allocator<char const*> >::_M_allocate(unsigned long) (stl_vector.h:172)
==12432==    by 0x10A9F1: void std::vector<char const*, std::allocator<char const*> >::_M_range_initialize<char const* const*>(char const* const*, char const* const*, std::forward_iterator_tag) (stl_vector.h:1328)
==12432==    by 0x10A82F: std::vector<char const*, std::allocator<char const*> >::vector(std::initializer_list<char const*>, std::allocator<char const*> const&) (stl_vector.h:387)
==12432==    by 0x6172A15: ???
==12432==    by 0x6172A8F: ???
==12432==    by 0x4010732: call_init (dl-init.c:72)
==12432==    by 0x4010732: _dl_init (dl-init.c:119)
==12432==    by 0x40151FE: dl_open_worker (dl-open.c:522)
==12432==    by 0x57482DE: _dl_catch_exception (dl-error-skeleton.c:196)
==12432==    by 0x40147C9: _dl_open (dl-open.c:605)
```
## Prerequisites

- cmake
- valgrind

## Usage

```
$ mkdir build
$ cd build
$ cmake ..
$ valgrind ./bar
```

## Analysis

### What is happening

As we can see from stacktraces, main program and library deinitialize the same statically initialized objects. 
On linux, for the common static library, the object will be the same in binary and shared library who are using it.

Generally, using statically allocated objects produces unpredictable behavior in most cases, since compiler may reorder 
static variables and call constructors in unspecified order. The destructors here are called two times.

### Similar issues

The issue is mentioned in the following links:
- https://stackoverflow.com/questions/4925233/global-variable-has-multiple-copies-on-windows-and-a-single-on-linux-when-compil
- https://stackoverflow.com/questions/25051679/c-program-crashes-when-linked-to-two-3rd-party-shared-libraries
- http://www.unixguide.net/unix/programming/1.1.3.shtml

### How to solve

There are several solutions/workarounds for the issue:

- Use _exit() instead of exit(), thus ignoring static resource deallocation
- Do not use const statically initlialized objects which call `malloc`/`new`, like `std::vector`, `std::string`, etc. In this case, `std::vector` can be easily replaced with `const char*[]`
- Use `RTLD_LOCAL` flag in `dlopen` (Side effect: impossible to use C++-features: RTTI, exceptions over shared library interface, only for C-style interfaces)
- Use visibility modifier attributes other than `default`:`internal`, `protected`. Usage: `__attribute__((visibility: (<visibility_mode>)))`
