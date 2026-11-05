# C for Python Programmers

You know Python and now have to write C? This guide explains the differences you will meet in the exercises, with examples from the starter code. It is not a complete C course, only what you need here.

> [!NOTE]
> This guide is about **C on the ESP32 with ESP-IDF**, as used in this project. A few things differ from C on a PC. For example, you print with `ESP_LOGI()` instead of the standard C function `printf()`, and the program doesn't end like a script: `app_main()` starts tasks that run until the robot is switched off.

> [!IMPORTANT]
> **Read the table of common mistakes first.** It covers most of the bugs that take hours to find.

## Common mistakes

| Wrong | Right | Why |
|---|---|---|
| `1 / 2` | `1.0f / 2.0f` | **Integer division:** if both numbers are integers, the result is an integer. `1 / 2` is `0` |
| `x ^ 2` | `x * x` | **`^` is not a power** but a bitwise XOR. No error, just a wrong result. Use `x * x` or `powf(x, 2.0f)` |
| `if (a = b)` | `if (a == b)` | **`=` assigns, `==` compares.** With `=`, the condition is almost always true |
| `float sum;` | `float sum = 0.0f;` | **Uninitialized variables** contain random values until you assign one |
| `q[2]` | `q[0]`, `q[1]` | **Index out of range:** `float q[2]` has the indices 0 and 1. No error, other variables are silently overwritten |
| `get_angle(angle);` | `get_angle(&angle);` | **Missing `&`:** to return a single value, the function needs the address of your variable |
| `const float M[2]` | `static const float M[2]` | **Pointer to a deleted array:** without `static`, the array is deleted when the function returns, see [static](#static) |
| `0.5` | `0.5f` | **Missing `f`:** `0.5` is a `double`, which is much slower on the ESP32 |
| `a and not b` | `a && !b` | **No `and`, `or`, `not` in C**, see below |

In C, `and` is written `&&`, `or` is written `||` and `not` is written `!`.

> [!TIP]
> When the build fails, **read the first error message**. Later errors are often consequences of the first one. The message shows the file and line number, and you can click on it to jump there.

## The basics

C is **compiled**: the whole program is translated before it runs (the PlatformIO **Build** button). Many mistakes that Python only reports when a line runs are found by C during the build.

| Python | C |
|---|---|
| Indentation defines blocks | `{ }` define blocks, indentation is only for readability |
| No line ending | Every statement ends with `;` |
| `# comment` | `// comment` or `/* comment */` |
| `x = 1.5` | `float x = 1.5f;` (the type must be declared) |
| `True`, `False` | `true`, `false` (from `<stdbool.h>`) |
| `x ** 2` | `x * x` or `powf(x, 2.0f)` |
| `import math` | `#include <math.h>` |
| `math.sin(x)`, `math.pi` | `sinf(x)`, `M_PI` |
| `abs(x)` | `fabsf(x)` for `float` (`abs()` only works for integers) |
| `print(f"x = {x}")` | `ESP_LOGI(TAG, "x = %.3f", x);` (standard C on a PC: `printf("x = %.3f\n", x);`) |

### `if`, `for` and `while`

```python
# Python
for i in range(dim):
    if q[i] > 0.0 and not done:
        Q[i] = kp * q[i]
    else:
        Q[i] = 0.0
```

```c
// C
for (uint8_t i = 0; i < dim; i++)
{
    if (q[i] > 0.0f && !done)
    {
        Q[i] = kp * q[i];
    }
    else
    {
        Q[i] = 0.0f;
    }
}
```

## Types

In C, every variable has a fixed type that you declare once:

| Type | Use for | Example |
|---|---|---|
| `float` | All physical values (angles, voltages, gains, ...) | `float voltage = 3.3f;` |
| `int8_t`, `int16_t`, `int32_t` | Signed integers with 8, 16 or 32 bits | `int32_t position_counts = -120;` |
| `uint8_t`, `uint16_t`, `uint32_t` | Unsigned integers (0 or positive) with 8, 16 or 32 bits: counters, indices, sizes | `uint8_t dim = 2;` |
| `bool` | `true` / `false` | `bool done = false;` |

Choose the smallest integer type that fits your values: a `uint8_t` holds 0 to 255, a `uint32_t` 0 to about 4.3 billion. If a value exceeds the range, it silently wraps around (255 + 1 becomes 0 in a `uint8_t`).

> [!NOTE]
> You will also see the type `int`. Its size depends on the processor (32 bits on the ESP32, but only 16 bits on many other microcontrollers). The types with a number in their name (from `<stdint.h>`) have the same size everywhere, so prefer them.

Use `float` (not `double`) for all calculations and always write float literals with `f`: `0.5f`, `2.0f`. Use the float versions of the math functions: `sinf()`, `cosf()`, `sqrtf()`, `atan2f()`, `fabsf()`.

## Arrays

C arrays have a **fixed size** that is set when they are declared. There is no `len()`, no `append()` and no negative index.

```c
float q[2] = {0.0f, 0.0f}; // array with two elements, both 0
q[0] = 1.0f;               // first element
q[1] = 2.0f;               // second (and last) element
```

Because an array doesn't know its own length, functions that take an array also take its length. In the starter code this is called `dim` or `nr_of_motors`.

**Matrices** are stored row by row in a flat array. The element in row `i` and column `j` of a matrix with `n` columns is `M[i * n + j]`:

```c
// 2x2 matrix  | 1  2 |
//             | 3  4 |
static const float M[4] = {1.0f, 2.0f,
                           3.0f, 4.0f};
float m10 = M[1 * 2 + 0]; // row 1, column 0 -> 3.0f
```

## Pointers and addresses

The memory of the ESP32 is a long row of numbered cells. Every variable is stored in some of these cells, and the number of its first cell is its **address**.

- **Address of a variable:** `&x` gives the address of the variable `x`.
- **Pointer:** a variable that stores an address. `float *p` is a pointer to a `float`.
- **Value at an address:** `*p` accesses the value stored at the address in `p`.

```c
float x = 1.5f;  // x is stored somewhere in memory
float *p = &x;   // p contains the address of x
*p = 2.0f;       // writes 2.0f to that address, so x is now 2.0f
```

Why is this useful? In Python, a function cannot change a number variable of its caller: `def set_zero(v): v = 0` has no effect outside the function. With a pointer, a C function knows *where* the caller's variable is and can write to it:

```c
void set_zero(float *v)
{
    *v = 0.0f;   // writes to the caller's variable
}

float x = 1.5f;
set_zero(&x);    // pass the address of x; x is now 0.0f
```

**Arrays** work like pointers: the name of an array is the address of its first element. This is why you pass arrays to functions without `&`, and why the function can write into them, similar to a Python list passed to a function.

> [!WARNING]
> A pointer can point anywhere, including to memory that is no longer valid (see [static](#static)) or to `NULL` (address 0, "no address"). Writing through such a pointer doesn't cause a clear error message: the robot behaves strangely or restarts.

## Functions return results through pointers

In Python, a function returns its result. Most functions in the starter code instead **write their result into an array that you pass in**:

```python
# Python
Q = pd_controller_calc_Q(q, q_d)
```

```c
// C
float Q[2];                      // reserve space for the result
pd_controller_calc_Q(q, q_d, Q); // the function fills Q
```

The declaration in the header file gives a first hint which parameters are inputs and which are outputs:

```c
void pd_controller_calc_Q(const float *q, const float *q_d, float *Q);
//                        ^^^^^ input    ^^^^^ input     ^^^^ output
```

- `const float *q` means the function promises to only **read** `q`. It is an input.
- A pointer **without** `const` means the function **may write** to it. This is usually an output, but it can also be **input and output at the same time**. Example from `common_math.h`:

  ```c
  void derivative(const float *current, float *previous, const float *dt, float *ret, uint8_t dim);
  ```

  `previous` provides the last value, which is needed to calculate the derivative, and is then overwritten with `current` for the next call.

> [!IMPORTANT]
> `const` is only a hint: it can also be missing by mistake. **Always read the documentation of the parameters** (`@param`) in the header file or in the [online documentation](https://robotics-ost.github.io/met3_r_template/).

**Arrays** are passed by name, without `&`. For a **single variable**, you must pass its address with `&`:

```c
float angles[2];
encoder_driver_get_angles(angles);   // array: no &

float angle_A;
encoder_driver_get_angle_A(&angle_A); // single variable: & gives its address
```

## Structs

A struct groups several values, similar to a small class without methods or a dict with fixed keys. The starter code uses structs for all configurations:

```c
typedef struct
{
    float dt;
    float kp;
    float kd;
} pd_controller_config_t;
```

Create one with named fields, similar to keyword arguments in Python:

```c
pd_controller_config_t config = {
    .dt = 0.005f,
    .kp = 10.0f,
    .kd = 1.0f,
};
```

Access fields with `.` on a struct and with `->` on a pointer to a struct:

```c
config.kp = 12.0f;  // config is a struct

pd_controller_config_t *p = &config;
p->kp = 12.0f;      // p is a pointer to a struct, same effect
```

## Header and source files

Each module consists of two files:

| File | Contains | Comparable to |
|---|---|---|
| `motor_model.h` (header) | Declarations: types and function signatures, with documentation | The list of functions a Python module offers |
| `motor_model.c` (source) | The implementation of the functions | The Python module itself |

`#include "motor_model.h"` makes the declarations available in another file, similar to `import`. Headers start with `#pragma once`, which prevents them from being included twice.

When you create your own module (Exercise 06), the header goes into `lib/control_system/include/` and the source file into `lib/control_system/src/`.

## `static`

`static` has two different meanings in C, depending on where it is used.

**1. Outside a function: private to the file.** A `static` variable or function at the top of a `.c` file can only be used in this file, like a name starting with `_` in Python, but enforced by the compiler. The starter code uses this for the internal state of each module:

```c
static pd_controller_config_t cfg = {0}; // only visible in pd_controller.c
```

**2. Inside a function: keeps its value.** Normal local variables are deleted when the function returns. A `static` local variable lives as long as the program runs:

```c
void app_main()
{
    float a[2] = {1.0f, 2.0f};               // deleted when app_main() returns
    static const float M[2] = {1.0f, 2.0f};  // stays valid
}
```

This matters when you pass an array to a config struct that only stores a pointer (e.g. `M` or `params`). If the array is deleted, the pointer points to memory that is used for something else. Your code still compiles and runs, but reads wrong values. **Arrays referenced by config structs must be `static`.**

## Constants with `#define`

`#define` defines a constant that the compiler replaces in the text before compiling. All configuration constants in `include/config.h` are defined this way:

```c
#define CONTROL_SYSTEM_TASK_PERIOD_MS 5 /**< Control system task period [ms] */
```

> [!WARNING]
> No `=` and no `;` in a `#define`. `#define KP = 10.0f;` compiles into nonsense and causes strange errors in the lines where `KP` is used.

## Error handling

C has no exceptions. ESP-IDF functions return an error code of the type `esp_err_t`, which is `ESP_OK` on success. `ESP_ERROR_CHECK()` checks the code and stops the program with an error message in the Serial Monitor if it is not `ESP_OK`:

```c
ESP_ERROR_CHECK(pd_controller_init(&config));
```

If your robot restarts right after start-up, look for an `ESP_ERROR_CHECK failed` message in the Serial Monitor. It shows the file and line of the failing call.

## Printing values

`ESP_LOGI()` works like the standard C function `printf()`, but adds the log level, a time stamp and the tag to each message. It needs a placeholder for each value:

| Placeholder | Type | Example output |
|---|---|---|
| `%d` | signed integers: `int8_t`, `int16_t`, `int32_t` | `-42` |
| `%u` | unsigned integers: `uint8_t`, `uint16_t`, `uint32_t` | `42` |
| `%f` | `float` | `3.141593` |
| `%.3f` | `float` with 3 decimals | `3.142` |
| `%s` | text | `hello` |

```c
ESP_LOGI(TAG, "cycle %u: q = [%.3f, %.3f] rad", (unsigned)cycle_count, q[0], q[1]);
```

For `int32_t` and `uint32_t` values, write `(int)` or `(unsigned)` in front of them, as above. Otherwise the compiler warns that the type doesn't match the placeholder.

See [Working with the Code](WORKING_WITH_THE_CODE.md) for how often you should print from the control loop.
