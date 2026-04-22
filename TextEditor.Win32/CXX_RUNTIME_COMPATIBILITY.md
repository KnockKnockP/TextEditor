# C++ Runtime Compatibility Notes

This project should reason about **compiler features** and **runtime dependencies**
separately.

- The bundled runtime baseline is `TextEditor.Win32/Resources/msvcrt.dll`.
- That file reports version `4.20.6201` and a PE timestamp of `1996-10-23`.
- A newer compiler can still target that runtime if the final executable only
  imports functions that the old DLL actually exports.

## Runtime Baseline

The bundled `msvcrt.dll` exports the classic C layer (`malloc`, `free`,
`realloc`, `memcpy`, `memmove`, `memset`, `strlen`, `wcscpy`, `wcslen`, and
friends) and also several C++ support entries:

- `_CxxThrowException`
- `__CxxFrameHandler`
- `_purecall`
- `type_info` methods such as `name`, `raw_name`, and comparisons
- `terminate` and `unexpected`

That means the old runtime is not "C only". It can host classic C++ runtime
mechanisms, but that does **not** automatically make every standard library
choice a good fit for this project.

## Local Link Tests

All test programs below were built locally and checked with `objdump -p`.

### Newer core language features

A small C++14 sample using:

- default member initializers
- `auto`
- a lambda

linked to only:

- `KERNEL32.dll`
- `msvcrt.dll`

and produced a `42005` byte executable.

Takeaway: **newer core language syntax is usually runtime-neutral**.

### `std::vector`

`std::vector<int>` with `-static-libstdc++ -static-libgcc`:

- imports only `KERNEL32.dll` and `msvcrt.dll`
- executable size: `319877` bytes

`std::vector<int>` with the default dynamic libstdc++ link:

- imports `libstdc++-6.dll`
- executable size: `42510` bytes

Takeaway: `std::vector` is technically possible, but only if we are willing to
either:

- ship extra runtime DLLs, or
- statically link libstdc++ and accept the code-size hit

For this project, the second option is the only runtime-compatible one, and it
is still much heavier than the in-house container.

### `std::string`

`std::string` with `-static-libstdc++ -static-libgcc`:

- imports only `KERNEL32.dll` and `msvcrt.dll`
- executable size: `319652` bytes

Takeaway: same tradeoff as `std::vector`.

### `iostream`

`iostream` with `-static-libstdc++ -static-libgcc`:

- imports only `KERNEL32.dll` and `msvcrt.dll`
- executable size: `1751990` bytes

It also pulls in much more CRT surface such as locale and wide-character
formatting helpers.

Takeaway: technically workable, practically a bad fit here.

### Exceptions and RTTI

A sample using:

- virtual dispatch
- `dynamic_cast`
- `try`/`catch`
- `throw`

with `-static-libstdc++ -static-libgcc`:

- imports only `KERNEL32.dll` and `msvcrt.dll`
- executable size: `186484` bytes

Takeaway: exceptions and RTTI can work against this runtime, but they are still
optional policy choices. The current project keeps `-fno-exceptions` and
`-fno-rtti` because they reduce size and keep the dependency model easier to
audit.

## Practical Rule Set

### Safe to use freely

- classes, namespaces, constructors, destructors
- references, overloading, operator overloading
- templates
- virtual functions
- RAII wrappers
- newer syntax-only language features, as long as they do not drag in a new
  library dependency

### Technically possible, but use carefully

- `std::vector`
- `std::string`
- exceptions
- RTTI

These are only acceptable if the final binary still imports the intended old
runtime surface. Static linking can make them work, but it also increases code
size significantly.

### Avoid by default

- `iostream`
- locale-heavy standard library features
- anything that requires `libstdc++-6.dll`, `libgcc_s_*.dll`, or another extra
  runtime DLL

## Current Project Direction

The project should prefer:

- a custom container layer for hot-path/editor data structures
- custom UTF-8 and UTF-16 handling
- Win32 APIs at the boundary
- a modern compiler only as a code generator, not as permission to accept extra
  runtime baggage

In short:

- **yes**, newer compilers are fine
- **yes**, even some newer C++ language features are fine
- **no**, that does not mean the full standard library is a free pass
- **for this repo**, a custom `Vector` and custom string layer still make sense

## Build-System Note

GCC's documentation says that with `-nostdlib` or `-nodefaultlibs`, the driver
stops adding the usual runtime libraries automatically, and options such as
`-static-libgcc` are ignored. In that link mode, any future STL experiment would
need explicit library handling instead of relying on the default `g++` link.
