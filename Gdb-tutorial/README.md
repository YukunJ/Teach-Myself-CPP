## GDB Tutorial

**GDB** stands for GNU debugger. It's a powerful debugging tool to let people step-by-step go through their program while inspecting values, addresses and states on the fly. This has proven to be very effective in debugging complicated programs with subtle bugs.

**GDB** is typically used for C/C++ programs and are available on Linux systems. 

The reference for this tutorial is mostly from <15213-Intro to Computer System>'s Bootcamp2 [Debugging & GDB](https://www.cs.cmu.edu/~213/bootcamps/f22_lab2.pdf) at Carnegie Mellon University.

---

#### Start GDB

There are multiple ways to start gdb inspection on a program.

Say you have a source code `sample.cpp` which has been compiled into executable `sample`.

We can first log in gdb and run the program:

```console
$ gdb
(gdb)$ file sample // run this sample program
```

or we could directly run gdb on this executable:

```console
$ gdb sample
```

But the executable needs to be compiled with additional `+g` flag to contain debug information.

Thirdly, we could directly compile the source code and pass gbd the executable like this:

```console
$ gdb --args g++ -g -o sample sample.cpp
```

---

#### Common GDB Commands

+ (gdb) CTRL + c: stop execution
+ (gdb) next (n): run next line of program and does NOT step into function calls
	+ (gdb) next X (n X): run next X lines of program
	+ (gdb) nexti: run next line of assembly code and does NOT step into function call
+ (gdb) step (s): run next line of program AND will step into function call
	+ (gdb) step X (s X): step next X lines of program
	+ (gdb) stepi: step next line of assembly code
+ (gdb) continue (c): continue running program until hit next breakpoint or error
+ (gdb) finish (f): run code until current function is finished
+ (gdb) list (l): list 10 lines of source code from current line
	+ (gdb) list X (l X): list 10 lines of source code from line X
	+ (gdb) list fnName (l fnName): list 10 lines of source code from fnName function


We can set a breakpoint in function so that it stops wheneven it hits the break point.
+ (gdb) break fnName (b fnName): breaks once we call a specific function
+ (gdb) info b: display information about all breakpoints currently set
+ (gdb) disable #: disable breakpoint with ID equal to #

A special break point is called **watchpoint**, which stops the program whenever the value of an expression changes. The expression might be variable or values combined by operators

+ (gdb) watch foo: watch the value of a variable called foo
+ (gdb) watch *(int *)0x600850: watch the integer value stored at address 0x600850

We are also able to print out global and local variable values.

+ (gdb) info variables: list all global and static variable names
+ (gdb) info locals: list local variables of current stack frame
+ (gdb) info args: list arguments of the current stack
+ (gdb) print (p) [any valid expression]: print a expression

We might want to see what's the history function call stack that brings the program here:

+ (gdb) backtrace (bt): print a summary of function call stacks

And actually we can modify variables and call functions on the fly by gdb:

+ (gdb) call expr: evaluate the expression expr
+ (gdb) set [variable] expression: set the value associated with a variable  to expression

---

#### Experiments

Enough dry content of gdb commands. We will use a very simple program to show a sample debugging process of function.

Suppose we have a very simple C++ program that multiply up a integer vector, and  print out the product to the standard output. However, this program contains a small bug that leads to accessing out of bound.

```CPP
// prod.cpp
#include <iostream>
#include <vector>

/**
 * @brief return the product of the input array
 * @param array the input array
 * @ret prod of the array, or 0 if it is empty
 */
int prod(std::vector<int> array) {
  if (array.empty()) {
    return 0;
  }
  int prod = 1;
  for (int i = 0; i <= array.size(); i++) { // <- problem here, the '<='
    prod *= array[i];
  }
  return prod;
}

int main() {
  std::vector<int> experiment {1, 2, 3, 4, 5};
  std::cout << "The prod of <1, 2, 3, 4, 5> is " << prod(experiment) << std::endl; // should be 120
}
```

It's obvious to you and me that the bug in this program is that, it access outside the boundary of the vector, it should be `i < array.size()` instead of `i <= array.size()`.

Currently, this program outputs that the product of `<1, 2, 3, 4, 5>` is `0`. It's most likely because the "one out of bound" element is 0. But let's assume we didn't know this bug and just want to use gdb to debug.

Let's get started!

We first load this program into gdb

```console
$ g++ -std=c++11 -g -o prod prod.cpp
$ gdb prod
```

And then we set breakpoint for the `main` and `prod` function:

```console
$ (gdb) break main
$ (gdb) break prod
```

And we run:

```console
$ (gdb) run

Starting program: ./prod 

Breakpoint 1, main () at prod.cpp:21
21        std::vector<int> experiment {1, 2, 3, 4, 5};
```

We step forward until we step into the `prod` function and hit the second break point:

```
$ (gdb) step
...

Breakpoint 2, prod (array=std::vector of length 5, capacity 5 = {...}) at prod.cpp:10
```

Since this one, we want to keep an eye on the value of `prod`. We inspect its initial value:

```console
$ (gdb) info locals
prod = 0
```

```console
$ (gdb) next
13        int prod = 1;
```

And as we enter the loop, we keep an eye on both `prod` and the index `i`:

```console
$ (gdb) next
14        for (int i = 0; i <= array.size(); i++) {


$ (gdb) info locals
i = 0
prod = 1
```

We step until we are at index 4:

```console
$ (gdb) next
...
...
$ (gdb) info locals
i = 5
prod = 120
```

Here it is! When we are at index 5, the prod is already 120, we should have return. Instead, it's doing one more multiplication with `array[5]`, which is out of bound and of value `0`. This makes the `prod` into zero:

```console
$ (gdb) print i
$2 = 5
$ (gdb) print array[i]
$3 = 0
(gdb) print prod
$4 = 0
```

Bugs found! High praise for **GDB**.