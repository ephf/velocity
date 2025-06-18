# Velocity Language Guide

> latest revision: v0.1.0 (unstable)

> Features may be marked `unimplemented`, `unfinished`, or `unstable` and any feature may change

## Setup

Use `make` to build the compiler

```sh
make vlc
```

The compiler automatically builds `main.vl`

```sh
./vlc
```

Then compile the output in `main.c`

```sh
cc main.c -o <executable name>
```

## Quick Hello World

```velocity
fn main() -> int {
    print("Hello World");
}
```

In the Hello World example, we start by defining a `main` function using the `fn` keyword. Usually a function is followed by arguments surrounded by parenthesis, but the `main` function does not need to take any arguments. Using the `->`, we can define a return type which we define as `int` for this function.

Inside the `main` function, similar to C, is where all of your code initially runs. Here we call the standard `print` function which takes in a string as an argument. We passed in `"Hello World"`, so when the program is compiled, we will see the text `Hello World`.

## Data Types

Data types are used to define what variables are going to contain. In the previous example, the `int` type was used. This is a numeric type that is equivalent to C's `int` type. Some more data types can be seen here.

| Type Name     | C Equivalent  | Status    |
| ---           | ---           | ---       |
| `uint` | `unsigned int` | `std` |
| `char` or `uchar` | `char` or `unsigned char` | `std` |
| `usize` or `isize` | `size_t` or `ssize_t` | `std` |
| `float` or `double` | `float` or `double` | `unimplemented` |
| `void` | `void` | `unstable` |
| `i64` or `u64` | `int64_t` or `uint64_t` | `unimplemented` |
| `i32` or `u32` | `int32_t` or `uint32_t` | `std` |
| `i16` or `u16` | `int16_t` or `uint16_t` | `unimplemented` |
| `i8` or `u8` | `int8_t` or `uint8_t` | `unimplemented` |

These are all basic data types and most of them are `numeric` and can be used to perform basic operations.

## Variables and Operations

In the main function, you can define variables and write your program logic. When defining a variable, you use the `let` keyword followed by the variable name, an equals sign, and its value.

```velocity
let x = 5;
```

This creates the variable `x` and sets its value to `5`. By default, `x` is a `numeric auto` which just means its a number without a defined type. We can give it a specific type right after the name of the variable using a `:` followed by the type name.

```velocity
let x: i32 = 5;
```

Now we know `x` is a signed 32-bit integer because we used the `i32` type. With these variables, we can do some calculations. Velocity has the same operator precedence as C.

```velocity
let x = 15;
let y = 213;

let sum = x + y;
let product = x * y;
```

These variables are going to default to the `int` type because they are `numeric autos`, but we can give them all different types and do math with them if those types are numeric.

> `unstable:` compiling code from `vlc` may cause casting errors
```velocity
let x: i32 = 195;
let y: u64 = 224;

let sum: i16 = x + y;
let prod: isize = x * y;
```

## Functions

Like the `main` function, you can define and call functions that do different things. Using the `fn` keyword, we can create a function that echos whatever it is given.

```velocity
fn echo(value: int) -> int {
    return value;
}
```

In the argument list we define an argument `value` that has the type `int`. We then use a `return` statement to bring that value out of the function. We can use this function in our main function and do math to the result.

```
fn main() -> int {
    let echoed_value = echo(3);
    let four = echoed_value + 1;
}
```

Here we passed in `3` to the `echo` function and it returned `3` back. We then added 1 to that and stored it in the variable `four`.

## Structures & Pointers

In the Hello World example, we called the function `print` which requires the `str` type as the first argument. `str` is a structure, meaning it has multiple data types within itself. This is how `str` is defined:

```velocity
struct str {
    data: &char,
    size: usize,
}
```

Here we define struct to have two fields, `data` and `size`. You might notice the `&` before the `char`. This is a pointer or reference to a `char`. We can create variables with these structures and access their fields.

> `unstable:` string literals have an incomplete type, defining variables as auto will have this incomplete type without fields, the following code is fine
```velocity
let message: str = "Hello World";
let message_size = message.size; // 11
```

Structures can also be created on the fly using the name of the structure and `{` `}` braces.

```velocity
let heap_string = str {
    data: c::lib::malloc(sizeof<char> * 16),
    size: 16,
};
```

This code allocates a string with 16 characters and creates a `str` struct. 

## Namespaces & Namespaced Declarations

`c` is a namespace that includes some of the c standard library functions. `c::lib` has functions from `stdlib.h`, and another namespace in c, `c::io`, contains functions from `stdio.h`.

You can define namespaces using the `namespace` keyword, and namespaces are automatically defined when defining a structure.

```velocity
namespace my_namespace {
    fn say_hi() {
        print("Hello World");
    }
}

fn main() -> int {
    my_namespace::say_hi(); // Hello World
}
```

You can add on to namespaces outside of their scope with the `::` syntax in function, structure, and type declarations.

```
namespace test {}

fn test::difference(a: int, b: int) -> int {
    return a - b;
}
```

The same works for structures

> `unimplemented:` boolean types and logic are not implemented
```
struct str { /* ... */ }

fn str::equals(self, other: str) -> bool {
    return self.size == other.size
        && !c::string::strncmp(self.data, other.data, self.size);
}
```

This implementation creates a function on the `str` structure that tests if two strings are equal. the `self` keyword will automatically define a `self` variable that has the type of the outer namespace, or structure in this example. We can call this equals method like accessing a field on a structure:

> `unimplemented:` control flow and boolean logic are not implemented
```velocity
fn main() -> int {
    let a: str = "Hello World";
    let b: str = "Hola Mundo";

    if(a.equals(b)) print("a and b are equal");
    else print("a and b are not equal"); // a and b are not equal
}
```