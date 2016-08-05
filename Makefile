SHELL = /bin/sh

SRCDIR   = src
OBJDIR   = obj
BINDIR   = bin

TARGET = $(BINDIR)/wepcrack
SRC = $(wildcard $(SRCDIR)/*.c)
OBJ = $(SRC:.c=.o)
CC = clang
has_clang = $(shell clang --version 2> /dev/null)
ifeq (, $(has_clang))
	CC = gcc
endif
CFLAGS = -std=c11 -g -pedantic -fopenmp
LDFLAGS = -lm -fopenmp

.SUFFIXES:
.SUFFIXES: .c .o

all: $(TARGET)
	@printf "$(TARGET)\n"

$(TARGET): $(OBJ)
	@printf "\e[33mLinking\e[0m %s\n" $@
	$(CC) -o $@ $^ $(LDFLAGS)
	@printf "\e[34mDone!\e[0m\n"

%.o: %.c
	@printf "\e[36mCompile\e[90m %s\e[0m\n" $@
	$(CC) -o $@ -c $< $(CFLAGS)

.PHONY: clean distclean

clean:
	rm -rf $(OBJ)
	@printf "\e[34mClean\e[0m\n"&

distclean: clean
	rm -rf $(TARGET)
	@printf "\e[34mAll clear!\e[0m\n"&


# Show variables (for debug use only.)
show:
	@echo 'TARGET   :' $(TARGET)
	@echo 'SRC      :' $(SRC)
	@echo 'OBJ      :' $(OBJ)
	@echo 'CFLAGS   :' $(CFLAGS)
	@echo 'LDFLAGS  :' $(LDFLAGS)
	@echo 'CC       :' $(CC)
