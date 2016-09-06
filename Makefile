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

DEFS = -D _XOPEN_SOURCE=700
CFLAGS = $(DEFS) -std=c11 -Wall -Wextra -I$(SRCDIR) \
  `pkg-config --cflags openssl`
LDFLAGS = -lm `pkg-config --libs openssl`

BUILD_ENV ?= dev
ifneq ($(BUILD_ENV),release)
  CFLAGS += -g -pedantic
endif

TEST_SRC = $(wildcard $(TESTDIR)/*_tests.c)
TEST_OBJ = $(filter-out $(SRCDIR)/main.o,$(OBJ))
TESTS    = $(patsubst %.c,%,$(TEST_SRC))

.SUFFIXES:
.SUFFIXES: .c .o

all: $(TARGET) $(TESTS) test

$(TARGET): $(OBJ)
	@printf "\e[33mLinking\e[0m %s\n" $@
	@$(CC) -o $@ $^ $(LDFLAGS)
	@printf "\e[34mMain target built\e[0m\n"

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@printf "\e[36mCompiling\e[0m %s\n" $@
	@$(CC) -o $@ -c $< $(CFLAGS)

test: $(TEST_OBJ) $(TESTS)
	@sh $(TESTDIR)/runtests.sh

$(TESTS): $(TEST_OBJ)
$(TESTDIR)/%: $(TESTDIR)/%.c
	@printf "\e[32mBuilding test\e[0m %s\n" $@
	@$(CC) -o $@ $< $(TEST_OBJ) $(CFLAGS) $(LDFLAGS)

valgrind:
	VALGRIND="valgrind -v --leak-check=full --show-leak-kinds=all \
  --log-file=/tmp/valgrind-%p.log" \
  $(MAKE)

.PHONY: clean distclean TAGS

clean:
	@rm -rf $(OBJ) $(TESTS)
	@rm -f $(TESTDIR)/tests.log
	@rm -f /tmp/valgrind-*
	@rm -f $(TESTDIR)/*_tests
	@printf "\e[34mCleaned\e[0m\n"&

distclean: clean
	@rm -rf $(TARGET)
	@printf "\e[34mAll clear!\e[0m\n"&

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
