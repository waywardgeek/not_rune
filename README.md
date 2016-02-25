Rune is a system programming langauge under development.  It is intended to be:

- faster than C
- simple like Python
- safe like Java
- highly extensible

In short:

    Rune = Python + LALR(1) parsing + SoA memory layout

Rune is expected to outperform C on most memory-intensive tasks.  Rune will use
a structure-of-arrays memory layout, rather than array-of-structures.  This
often increases speed considerably.  For example, a 600K line C commercial
place-and-route toolkit for ASICs sped up 60% when switching to structure of
arrays memory layout using the DataDraw code generator.  For more insight into
the data structures in Rune, have a look at the DataDraw, which is also being
used to write the rune compiler.

The "core syntax" of Rune will look lot a like Lisp.  See core/README.md for
more information about the core syntax.  As an example, we can define a
factorial function like this:

    (func uint fact(uint n) (
        (if (== n 1) (
            (return 1)))
        (return (* n (fact (- n 1))))))

This Lisp-like syntax is not what you will use to write programs.  That's
because Rune has a built-in LALR(1) parser which is used to translate much
nicer looking code into its core Lisp-like syntax.  For example, you would
normally write the factorial function above like this:

    func fact(n) {
        if n == 1 {
            return 1
        }
        return n*fact(n-1)
    }

The built-in LALR(1) parser can be used to extend the syntax of Rune in almost
any way you like.  For example, the print statement will be defined with the
LALR(1) parser rather than built into the compiler itself.  Similarly, syntax
for complex number support will be contained in a library.

Rune is intended to be safer than system programming languages such as C and
C++.  Buffer overflows are either checked at runtime or optimizedout at compile
time, and there are never dangling pointers or uninitialized variables.
Dangling pointers are avoided through Rune's data modeling: every object is in
a cascade-delete relationship with one or more parent objects, all the way to
the "root" object.  Objects are either owned by a variable on the stack, or
lives in the heap.  Those owned on the stack are destroyed when they go out of
scope.

Rune will be is strongly typed.  When you do not specify types of function
parameters, they are infered from the caller.  Consider this min function:

    func min(a, b) {
        if a <= b {
            return a
        }
        return b
    }

When we call it with different parameter types, a new function is automatically
instantiated.  The following calls are to two different compiled functions
optimized for their parameter types:

    min("alice", "bob")
    min(3.4, 9.9)
