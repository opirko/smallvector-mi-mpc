# Small Vector in C++

Container similar to `std::vector` created as a school project at FIT CTU, specifically in MI-MPC course.
No leaks detected (by `valgrind`).

## Build Instructions

This project uses CMake as its build system and requires C++20.

### Building the Project

1. Create a build directory:
   ```bash
   mkdir -p _build
   cd _build
   ```

2. Configure the project:
   ```bash
   cmake ..
   ```

3. Build the project:
   ```bash
   make -j`nproc`
   ```

## Run Instructions

After building, you can run the test executable:

```bash
# From the _build directory
./test_small_vector
```

## Code Formatting

If `clang-format` is available, you can format the source code:

```bash
# From the _build directory
make format
```
