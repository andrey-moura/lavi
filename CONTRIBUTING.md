# Index

- [Introduction](#introduction)
- [Contributing Guidelines](#contributing-guidelines)
- [Code guidelines](#code-guidelines)
- [Must-know](#must-know)

# Introduction

Hello, dear contributor.

This is the Lavi programming language.
It is designed to be lightweight and easy to use.

Please, read the contributing guidelines carefully.

Thank you.

# Contributing Guidelines

## Help with English
As the writer of this language is not a native English speaker, please write in clear and simple English. Fix all misspellings and grammatical errors you find.

## Patches

When writing a bug fix in a patch, please make it with as few modifications as possible

# Code guidelines

## Code style

### General Code Style
- Use 2 spaces for indentation.
- Keep lines under 80 characters when possible.
- Use descriptive names for variables and functions.

### Code style in Lavi
- Use snake_case for variable and function names.
- Use PascalCase for class names.

### Code style in C++
- Use snake_case for variable, function and class names.

## Must-know

### Converting C++ values to Lavi objects

To convert a C++ value to a Lavi object, you can use the `lavi::lang::api::to_object` function. For example:

```cpp
auto lavi_false = lavi::lang::api::to_object(interpreter, false);
auto lavi_true = lavi::lang::api::to_object(interpreter, true);
auto lavi_number = lavi::lang::api::to_object(interpreter, 42);
auto lavi_string = lavi::lang::api::to_object(interpreter, "hello");
```