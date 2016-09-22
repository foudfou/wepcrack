SHELL = /bin/sh

SRCDIR   = src
OBJDIR   = $(SRCDIR)
BINDIR   = bin
TESTDIR  = tests

TARGET = $(BINDIR)/wepcrack
SRC    = $(wildcard $(SRCDIR)/*.c)
INC    = $(wildcard $(SRCDIR)/*.h)
OBJ    = $(SRC:.c=.o)

ifneq "$(origin CC)" "environment"
  has_clang = $(shell clang --version 2> /dev/null)
  ifneq ($(has_clang), undefined)
    CC = clang
  else
    CC = gcc
  endif
endif

uname_s = $(shell uname -s)
ifeq ($(uname_s), Linux)
  PKGCONF = pkg-config
else ifeq ($(uname_sS), FreeBSD)
  PKGCONF = pkgconf
endif

DEFS = -D_XOPEN_SOURCE=700
CFLAGS = $(DEFS) -std=c11 -Wall -Wextra -I$(SRCDIR) \
  `${PKGCONF} --cflags openssl`
LDFLAGS = -lm `${PKGCONF} --libs openssl` -pthread

BUILD_ENV ?= dev
ifneq ($(BUILD_ENV),release)
  CFLAGS += -g -pedantic -Wfatal-errors
endif

TEST_SRC = $(wildcard $(TESTDIR)/*_tests.c)
TEST_OBJ = $(filter-out $(SRCDIR)/main.o,$(OBJ))
TESTS    = $(patsubst %.c,%,$(TEST_SRC))

.SUFFIXES:
.SUFFIXES: .c .o

all: $(TARGET) $(TESTS) test

$(TARGET): $(OBJ)
	@printf "${colyellow}Linking${colreset} %s\n" $@
	@$(CC) -o $@ $^ $(LDFLAGS)
	@printf "${colblue}Main target built${colreset}\n"

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@printf "${colcyan}Compiling${colreset} %s\n" $@
	@$(CC) -o $@ -c $< $(CFLAGS)

test: $(TEST_OBJ) $(TESTS)
	@sh $(TESTDIR)/runtests.sh

$(TESTS): $(TEST_OBJ)
$(TESTDIR)/%: $(TESTDIR)/%.c
	@printf "${colgreen}Building test${colreset} %s\n" $@
	@$(CC) -o $@ $< $(TEST_OBJ) $(CFLAGS) $(LDFLAGS)

valgrind:
	VALGRIND="valgrind -v --leak-check=full --show-leak-kinds=all \
  --log-file=/tmp/valgrind-%p.log" \
  $(MAKE)

.PHONY: clean distclean TAGS

clean:
	@rm -rf $(OBJ) $(TESTS)
	@rm -rf $(OBJDIR)/*.o  # spurious object files
	@rm -f $(TESTDIR)/tests.log
	@rm -f /tmp/valgrind-*
	@rm -f $(TESTDIR)/*_tests
	@printf "${colblue}Cleaned${colreset}\n"&

distclean: clean
	@rm -rf $(TARGET)
	@printf "${colblue}All clear!${colreset}\n"&

TAGS: $(SRC) $(TEST_SRC)
	etags $(SRC) $(TEST_SRC)

# Show variables (for debug use only.)
show:
	@echo 'TARGET   :' $(TARGET)
	@echo 'SRC      :' $(SRC)
	@echo 'OBJ      :' $(OBJ)
	@echo 'CFLAGS   :' $(CFLAGS)
	@echo 'LDFLAGS  :' $(LDFLAGS)
	@echo 'CC       :' $(CC)
	@echo 'TEST     :' $(TESTS)

colreset  = \033[0;0m
colgreen  = \033[0;32m
colyellow = \033[0;33m
colblue   = \033[0;34m
colcyan   = \033[0;36m
