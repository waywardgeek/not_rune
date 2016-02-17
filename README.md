Rune is a system programming langauge under development.  It is intended to be:

- faster than C
- simple like Python
- safe like Java
- highly extensible

In short:

    Rune = Lisp + LALR(1) + DataDraw

Rune is expected to outperform C on most memory-intensive tasks.  Rune will use
a structure-of-arrays memory layout, rather than array-of-structures.  This
often increases speed considerably.  For example, a 600K line C commercial
place-and-route toolkit for ASICs sped up 60% when switching to structure of
arrays memory layout using the DataDraw code generator.  For more insight into
the data structures in Rune, have a look at the DataDraw, which is also being
used to write the rune compiler.

The "core syntax" of Rune will look lot a like Lisp.  For example, to define a
factorial function:

(func uint fact(uint n)
    (if (= n 1)
        (return 1))
    (return (* n fact(- n 1))))

Don't worry, because this ancient 1960's Lisp-like syntax is not what you will
use to write programs.  That's because Rune will have a built-in LALR(1) parser
which is used to translate much nicer looking code into its core Lisp-like
syntax.  For example, you would normally write a factorial function like this:

func fact(n) {
    if n == 1 {
        return 1
    }
    return n*fact(n-1)
}

Or for those of us who care a lot about speed:

func fact(n) {
    result = 1
    for i = 2 to n {
        result *= n
    }
    return result
}

The built-in LALR(1) parser will be used to extend the syntax of Rune in almost
any way you like.  For example, the print statement will be defined with the
LALR(1) parser rather than built into the compiler itself.  Similarly, syntax
for complex number support will be contained in a library.

Rune is intended to be safer than system programming languages such as C and
C++.  Buffer overflows are not possible, and there are never dangling pointers
or uninitialized variables.

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

As for simplicity, Rune will take a Python-like approach, minimizing the
knowledge of syntax and rules that the programmer must keep resident in
short-term memory.

The Go programming language will be heavily borrowed from.  It is not wrong to
think of Rune as an attempt at improving the speed and extensibility of Go to
make a more suitable systems programming language.
