<kbd>🌸 CppDrive</kbd>

CppDrive is a simple, lightweight file-sharing application built in C/C++.

Hoặc đơn giản là bài tập lớn môn thực hành lập trình mạng - IT4062 Hedspi k65 HUST, chủ đề file-sharing 🥴

This app simulates the action when you ssh to your cloud server then use unix-like commands to do file operations.

![CleanShot 2024-01-06 at 11 12 32](https://github.com/betty2310/cppdrive/assets/75170473/c8125f3e-a6f8-4ef7-a1a9-0e1f5fb30d3e)


## Features


 - [x] User-friendly shell interface, offering a familiar environment for developers.
  - [x] Secure authentication using hashed passwords.
  - [x] Comprehensive file and folder management: creation, deletion, renaming, moving, copying, searching, viewing (cat), along with upload and download capabilities.
  - [x] File and folder sharing feature, allowing permissions to be set for other users.
  - [x] Robust end-to-end encryption for enhanced security.
  - [x] Detailed logging system.

## Structure
``` bash
.
├── CMakeLists.txt
├── app
│   └── main.cpp  # Application source code.    
├── include
│   ├── example.h # Library header file.
├── src
    └── example.cpp # Library source code.

```

Sources go in [src/](src/), header files in [include/](include/), main programs in [app/](app).

If you add a new executable, say `app/hello.cpp`, you only need to add the following two lines to [CMakeLists.txt](CMakeLists.txt):

```cmake
add_executable(main app/main.cpp)   # Name of exec. and location of file.
target_link_libraries(main PRIVATE ${LIBRARY_NAME})  # Link the executable to lib built from src/*.cpp (if it uses it).
```

## Setup
### Dependencies
+ [CMake](https://cmake.org/)
+ C++ Compiler: `g++`, `clang` or `msvc`
+ `zip`, `unzip`, [`fd`](https://github.com/sharkdp/fd)


On **Ubuntu**:
```
$ sudo apt-get install build-essential cmake fd-find zip unzip libssl-dev

$ ln -s $(which fdfind) ~/.local/bin/fd # or add it to your PATH
```

> [!NOTE]  
> Should use branch `tokyo` for stable version. `master` branch may have some error now.


## Building

Build by making a build directory (i.e. `build/`), run `cmake` in that dir, and then use `make` to build the desired target.

Example:

```bash
$ mkdir build && cd build
$ cmake ..
$ make
$ ./main
```

> [!WARNING]  
> After register account, should exit app, then login again, login right after register may contain error.

### Visual Studio Code
Install the following extensions:
- [C/C++](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cpptools)
- [CMake Tools](https://marketplace.visualstudio.com/items?itemName=ms-vscode.cmake-tools)


## Authors
+ Dương Hữu Huynh - 20205087
