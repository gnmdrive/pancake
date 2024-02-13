# Pancake is a stack based interpreted programming langauge written in C

Highly inspiderd by Forth, currently in development

**Pancake?** Yes, because usually, each pancake lies on top of each other, just like the structure of a [stack](https://en.wikipedia.org/wiki/Stack_(abstract_data_type)) ;)

```
; code example (showcase purpose)
@text "Hello, World!"
:main text . end
```

## Features
* [Stack based](https://en.wikipedia.org/wiki/Stack-oriented_programming#:~:text=The%20programming%20languages%20Forth%2C%20Factor,data%20back%20atop%20the%20stack.), as I already said 
* Currently interpreted, in the future I'll work on the compiler
* Minimal instruction set
* Implement routines, a concept similar to macros or function (will see)
* Make use of [Reverse Polish Notation](https://en.wikipedia.org/wiki/Reverse_Polish_notation)
* Turing complete Programming Language (that's the goal)

### Next Steps
* Conclude refactoring, separate things into files
* Lexer should be able to recognize symbols only if they are space-separated
* Make control flow statement available
* Implement loops, take inspiration from Forth
* Implement module system and write standard library
* Think about new features and addons
* Write basic tests
