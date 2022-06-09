BUILDDIR= build
SRCDIR= src

SRCS= $(wildcard $(SRCDIR)/*.c)
OBJS= $(patsubst $(SRCDIR)/%.c, $(BUILDDIR)/%.o, $(SRCS))

TARGET= $(BUILDDIR)/wistc

CFLAGS= -Wall -Wextra -Werror -std=c99 -g -O2 -fno-strict-aliasing

.PHONY: all clean

all: $(TARGET)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c | $(BUILDDIR)
	$(CC) -c -o $@ $? $(CFLAGS)

$(TARGET): $(OBJS) | $(BUILDDIR)
	$(CC) -o $@ $? $(CFLAGS)

$(BUILDDIR):
	@mkdir -p $@

clean:
	rmdir $(BUILDDIR)