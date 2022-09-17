# wist

Wist is a statically typed, impure functional langauge designed for efficient,
high level software development.  It is extremely work in progress.

## language 

The semantics of Wist are quite similar to Standard ML or OCaml, with a focus on functional programming and static types.  It looks quite similar to Haskell, but don't be fooled, it is strict-by-default and offers statefulness.  It includes an advanced module system for organizing and parameterizing complex packages of code.

### examples

```haskell
x : Int
x = 3

id : A => A -> A
id x = x
```

## implementation

> ** Warning ** 
> Wist is extremely work in progress.  All that is described here is what 
> intends to be implemented.  How much of it is implemented is changing too 
> fast to note. 

Wist's implementation is a bit unconventional.  Similar to Clang, Wist is 
designed as a library for analyzing and introspecting on Wist code first, and 
as a toolchain second.  It is written from top to bottom in C99 and all the 
command line tools are written as programs ontop of the underlying Wist library.

The Wist library allows you to parse, typecheck, introspect, compile (AOT and 
for a bytecode interpreter) and interact with currently interpreting Wist code 
all from C.

