# Introduction

Hello, Claude.

This is the Lavi programming language.
It is designed to lightweigh and easy to use.

Please, follow the rules carefully.

Thank you.

# Rules

## Help with English
As the writer of this language is not a native English speaker, please write in clear and simple English. Fix all mistakes you find.

## Write code in the working tree unless otherwise instructed
Do not make commits or new branches outside the working tree.

## Patches

When you are asked to fix a bug in a patch, please make it with as few modifications as possible. You are free to cherry-pick changes and to search for modifications in
other branches if necessary.

# Code guidelines

## Code style

- Use 2 spaces for indentation.
- Use snake_case for variable and function names.
- Use PascalCase for class names.
- Keep lines under 80 characters when possible.
- Use descriptive names for variables and functions.

## C++ to Lavi

### Converting C++ values to Lavi objects

To convert a C++ value to a Lavi object, you can use the `lavi::lang::api::to_object` function. For example:

```cpp
auto lavi_false = lavi::lang::api::to_object(interpreter, false);
auto lavi_true = lavi::lang::api::to_object(interpreter, true);
auto lavi_number = lavi::lang::api::to_object(interpreter, 42);
auto lavi_string = lavi::lang::api::to_object(interpreter, "hello");
```