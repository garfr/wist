BUILDDIR= build
LIBDIR= lib
BINDIR= bin
INCDIR= inc

CFLAGS= -g -O0 -Wall -Wextra -I$(INCDIR) -I. -std=c99 -fno-strict-aliasing
LIB_CFLAGS=$(CFLAGS)

REPL_TARGET= $(BUILDDIR)/wisti
STATIC_TARGET= $(BUILDDIR)/libwist.a 

LIBSRCS= $(wildcard $(LIBDIR)/*.c)
LIBOBJS= $(patsubst $(LIBDIR)/%.c, $(BUILDDIR)/%.o, $(LIBSRCS))

.PHONY: all clean run

all: $(REPL_TARGET) $(STATIC_TARGET)

$(REPL_TARGET): $(BINDIR)/wisti.c $(STATIC_TARGET)
	$(CC) $< -o $@ $(CFLAGS) -Lbuild -lwist

$(STATIC_TARGET): $(LIBOBJS)
	ar rcs $@ $?

$(BUILDDIR)/%.o: $(LIBDIR)/%.c | $(BUILDDIR)
	$(CC) -c $< -o $@ $(LIB_CFLAGS)


$(BUILDDIR):
	@mkdir $@ -p


clean:
	rm -rf $(BUILDDIR)
