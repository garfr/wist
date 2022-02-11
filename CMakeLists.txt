cmake_minimum_required(VERSION 3.21)

project(wist)

add_executable(wist
               src/wist.c
               src/index.c
               src/mem.c
               src/petal.c
               src/str.c
               src/win32_fs.c
               src/membuf.c
               src/lexer.c
               src/token.c
               src/sym.c
               src/span.c
               src/error.c
               src/uast.c
               src/parser.c)

include_directories(wist PUBLIC include)
