
# Demo Code for demonstrating thread-aware software library boundaries.

This repository contains the demo code supplementing my [discussion of passing thread information across software library boundaries](https://www.karlrupp.net/2016/03/multithreading-software-library-design/).

The demo library `mylib` does not try to 'hide' the use of threads, but instead exposes a thread control object for the API calls.
Thus, the user is free to select his or her threading model of choice (usually one out of pthread, OpenMP, or C++11 threads) rather than the library mandating a particular threading model.
As such, the library is portable across different compilers: mylib does not need to be compiled with the same compiler as the user code.
For example, on Windows you could compile mylib with an Intel compiler and compile your user application with Visual Studio.

Note that the code in this repository is a tech-demo and not a full-fledged library solution.
The user-code for calling into the library is thus a bit bloated at times.
Many of the low-level details can be taken care of through additional convenience interfaces.

## Build

On *nix-like systems it is usually enough to call `make`:

    $> make

If you run into issues, have a look at `makefile` and adjust compilers, etc.

Windows users should just create a new project file in their favorite IDE and link `mylib.o` with one of the three main applications `with_openmp`, `with_pthread`, or `with_cpp11threads`.

## Run

Run the executables via

    $> ./with_cpp11threads
    $> ./with_openmp
    $> ./with_pthread

Each of the three executables should print a vector consisting of ten values of '10' and a dot product result of 165.

## License

The code is provided under a permissive MIT/X11-style license.
See file LICENSE.txt for details.

