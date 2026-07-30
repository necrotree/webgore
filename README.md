# Webgore

A native Windows browser made with the Qt WebKit Widgets API.

## Features

- Back, forward, reload, and home navigation
- Address bar that accepts URLs and search terms
- Page-title window captions and loading progress

## Requirements

- CMake 3.16 or newer
- A C++17 compiler supported by Qt
- Qt 5.15 with the `WebKitWidgets` module installed

Qt WebKit is not included with the standard Qt 6 distribution. Install a Qt 5 kit that provides `Qt5WebKitWidgets` before configuring the project.

## Build and run

Open a Developer PowerShell for the Qt kit, then run:

```powershell
cmake -S . -B build
cmake --build build --config Release
.\build\Release\Webgore.exe
```

When Qt is installed outside CMake's default search paths, add `-DCMAKE_PREFIX_PATH=<Qt-installation-path>` to the first command.

## Ignore rules

The repository's `.gitignore` is based on GitHub's public [C++ .gitignore sample](https://github.com/github/gitignore/blob/main/C%2B%2B.gitignore).
