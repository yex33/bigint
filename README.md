# bigint
This project provides a high-precision integer arithmetic library.

## About
- The main goal of this project was to refresh my C++ skills and explore features from the C++23 standard. I enjoyed working with new additions like `std::string_view` and `std::ranges`, which make writing C++ feel more like writing Python.

- The implementation is very straightforward. Each digit of a `bigint` is stored in a `std::vector<int32_t>`. Arithmetics are performed digit by digit. The underlying algorithms mostly follow the "Naive" methods from [Modern Computer Arithmetic by Richard P. Brent and Paul Zimmermann (2010)](https://members.loria.fr/PZimmermann/mca/mca-cup-0.5.3.pdf). I know the memory model of using a `int32_t` to store digits of value 10 at max is very wasteful. This is kept because the code was originally written for another memory model discussed below.

- I attempted to convert and store string represented integers from arbitrary base to `uint32_t` (referred as a `WORD`). That is converting say base 10 numbers to base $2^{32}$ numbers. I managed to implement all operators correctly except division. Proper `to_string(int base)` also requires division so that's also not implemented. This version of the implementation is branched off to the [develop branch](https://github.com/yex33/bigint/tree/develop). I gave up on moving forward with that due to time constraints.

## Build

Do something similar will run the custom test driver.
```bash
g++ test.cpp -Wall -Wextra -Wconversion -Wsign-conversion -Wshadow -Wpedantic -std=c++23 -o test.out
./test.out
```

However, I know this is not allowed for submission, but code is also setup for CMake. Simply do
```bash
mkdir build
cd build && cmake ..
cmake --build .
ctest
```

This will download all required dependencies from Github and run unit tests with [doctest](https://github.com/doctest/doctest). In fact, these steps are also automated as Continuous Integration (CI) with Github Actions. You can see that all unit tests are passing with a little check mark on the latest commit.

All doctest dependent codes are surrounded with `#ifdef` guards so both test drivers will work :)
