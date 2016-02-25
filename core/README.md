This directory holds the "core" Rune compiler.  It only recognizes the "core"
list form of Rune code, and only low-level forms that can be directly
translated to C code.  The core syntax is low-level, but still hides memory
layout.  There are new and delete operators to allocate/free objects on the
heap.
