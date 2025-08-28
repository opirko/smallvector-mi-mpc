# Small Vector in C++

Container similar to `std::vector` created as a school project at FIT CTU, specifically in MI-MPC course.

## Build Instructions

This project uses CMake as its build system and supports C++11 or higher.

### Building the Tests

1. Create a build directory:
   ```bash
   mkdir -p _build
   cd _build
   ```

2. Configure the project:
   ```bash
   cmake ..
   ```

   To specify a different C++ standard:
   ```bash
   cmake -DCXX_STANDARD=23 ..
   ```

3. Build the project:
   ```bash
   make -j`nproc`
   ```

## Test Run Instructions

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
