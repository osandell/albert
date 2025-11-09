# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Albert is a plugin-based, desktop-agnostic C++/Qt keyboard launcher. The project is version 33.0.1 and uses C++23 with Qt6 (minimum Qt 6.4). It supports macOS, Linux (XDG), and Windows platforms.

## Build Commands

### Initial Setup
```bash
# Configure the build with CMake
cmake -S . -B build

# On Windows with Visual Studio, you may need:
cmake -S . -B build -G "Ninja"

# On macOS with homebrew dependencies:
cmake -S . -B build \
  -DCMAKE_C_COMPILER=$(brew --prefix llvm@18)/bin/clang \
  -DCMAKE_CXX_COMPILER=$(brew --prefix llvm@18)/bin/clang++ \
  -DCMAKE_OSX_DEPLOYMENT_TARGET=11
```

### Building
```bash
# Build everything
cmake --build build

# Build with specific number of jobs
cmake --build build -j$(nproc)
```

### Testing
```bash
# Enable tests during configuration
cmake -S . -B build -DBUILD_TESTS=ON

# Run tests
ctest --test-dir build --output-on-failure
```

### Installation
```bash
# Install to system (or CMAKE_INSTALL_PREFIX if set)
cmake --install build --prefix /usr
```

### Plugin-Specific Options
```bash
# Build without specific plugins
cmake -S . -B build -DBUILD_PLUGIN_DEBUG=OFF -DBUILD_PLUGIN_DOCS=OFF

# Disable all plugins
cmake -S . -B build -DBUILD_PLUGINS=OFF
```

### Translation Updates
```bash
# Update translation files (lupdate)
cmake --build build --target update_translations
# Or use convenience target:
cmake --build build --target global_lupdate
```

### Packaging (macOS)
```bash
cd build
cpack -V
```

## Dependencies

### Required
- CMake 3.22+ (Ubuntu 22.04 baseline)
- C++23 capable compiler (Clang or GCC on Unix, MSVC on Windows)
- Qt6 6.4+ (Core, Concurrent, Network, Sql, Svg, Widgets, LinguistTools)
- Qt6Keychain
- QHotkey (bundled in lib/QHotkey)
- QNotification (bundled in lib/QNotification)

### Platform-Specific
- **Linux**: libgl1-mesa-dev, libglvnd-dev, libxml2-utils
- **macOS**: Cocoa framework, LLVM 18 (for builds)
- **Python plugins**: python3-dev

### Optional Plugin Dependencies
- **calculator-qalculate**: libqalculate-dev
- **python**: Python 3 development headers

## Architecture

### Core Components

**libalbert** (`src/`, `include/albert/`): The main shared library providing the plugin infrastructure and core functionality.

- **app/**: Application core - plugin management (`pluginregistry`), query handler infrastructure (`pluginqueryhandler`), telemetry, RPC server, and message handling
- **common/**: Base implementations of extension and item classes
- **frontend/**: Frontend interface and session management for the UI
- **platform/**: Platform-specific code (Unix signals, macOS Cocoa integration, XDG icon lookup)
- **plugin/**: Plugin system - registry, loader (`pluginloader`), provider interfaces, dependency resolution
- **query/**: Query engine (`queryengine`), query execution, fallback handlers, usage tracking database
- **settings/**: Settings window, plugin management widgets, query configuration UI
- **util/**: Utilities - icon handling, notifications, OAuth, network operations, item indexing, fuzzy matching (Levenshtein)

### Plugin System

Plugins inherit from `albert::PluginInstance` and use the `ALBERT_PLUGIN` macro to register themselves. Each plugin must have a `metadata.json` file defining its properties.

**Extension Types**:
- `TriggerQueryHandler`: Triggered by specific prefix (e.g., `calc ` for calculator)
- `GlobalQueryHandler`: Participates in every query
- `FallbackHandler`: Provides fallback results when no other handlers match
- `Frontend`: Provides UI implementations
- `UrlHandler`: Handles URL schemes

**Plugin Metadata** (`metadata.json`):
- `id`: Auto-generated from project name
- `version`: Auto-generated from project version
- `name`, `description`: Human-readable plugin info (supports localization)
- `license`: SPDX identifier
- `url`, `readme_url`: Documentation links
- `authors`, `maintainers`: GitHub usernames
- `runtime_dependencies`, `binary_dependencies`, `plugin_dependencies`: Required dependencies
- `loadtype`: Either `frontend` or `user`

### Creating a Plugin

Use the `albert_plugin()` CMake macro (defined in `cmake/albert-macros.cmake`):

```cmake
cmake_minimum_required(VERSION 3.16)
project(myplugin VERSION 1.0.0)

find_package(Albert REQUIRED)

albert_plugin(
    QT Widgets Network        # Qt components to link
    LINK mylib                # Additional libraries
    INCLUDE include           # Additional include dirs
    SOURCES src/*.cpp src/*.h # Source files (optional, auto-globbed)
)
```

**Plugin Structure**:
- Must define a class inheriting `albert::PluginInstance`
- Class must contain `ALBERT_PLUGIN` macro
- Implement `extensions()` to return extension instances
- Optionally implement `buildConfigWidget()` for settings UI
- Translation files: `i18n/<plugin_id>_<lang>.ts`

### Key Classes

- `albert::PluginInstance`: Base plugin class with access to settings, cache, keychain
- `albert::Extension`: Base extension interface (id, name, description)
- `albert::Item`: Represents a result item with text, icon, and actions
- `albert::Query`: Query context passed to handlers, supports adding results
- `albert::RankItem`: Item with match score for ranking
- `albert::StandardItem`: Concrete item implementation with builder pattern

### Plugin Loading

Plugins are loaded from:
1. Build directory: `${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/albert/`
2. Install directory: `${CMAKE_INSTALL_LIBDIR}/albert/`

The plugin loader uses `topologicalsort.hpp` to resolve dependencies and load plugins in correct order.

## Code Style & Compiler Warnings

The project uses strict compiler warnings:
- `-Werror=return-type`: Missing returns are errors
- `-Werror=float-conversion`: Implicit precision loss is an error
- `-Wall -Wextra -Wpedantic`: General warnings enabled
- MSVC: `/W4` with `/wd4996` (suppressing deprecation warnings)

Precompiled headers are used for the main library to speed up compilation.

## Platform Notes

### Windows
- Uses Visual Studio Build Tools (vcvarsall.bat)
- Ninja recommended as the build generator
- Signal handling via Windows-specific implementation

### macOS
- Objective-C++ (.mm) files for Cocoa integration
- Framework bundle structure (`FRAMEWORK TRUE`)
- Deployment target: macOS 11+
- Universal binaries supported via `CMAKE_OSX_ARCHITECTURES`

### Linux (XDG)
- Desktop entry parsing for application indexing
- XDG icon theme lookup
- FreeDesktop.org standards compliance
- Desktop file: `dist/xdg/albert.desktop`

## Internationalization

Translation source files are in `i18n/*.ts`. The build system:
1. Runs `lupdate` to extract translatable strings
2. Generates `.qm` files from `.ts` files
3. Embeds them in the binary via Qt's translation system
4. Adds translation statistics to plugin metadata (if xmllint available)

## Testing

Test implementation is in `test/test.cpp` using Qt Test framework. Tests are only built when `BUILD_TESTS=ON`.

## Git Submodules

The project has git submodules for dependencies:
- `lib/QNotification`: Cross-platform notification library
- `lib/QHotkey`: Global hotkey registration
- `plugins/*`: Some plugins are git submodules with modified status

When cloning, use `git clone --recursive` or run `git submodule update --init --recursive`.
