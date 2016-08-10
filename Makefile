SHELL = /bin/sh

SRCDIR   = src
OBJDIR   = $(SRCDIR)
BINDIR   = bin
TESTDIR  = tests

TARGET = $(BINDIR)/wepcrack
SRC    = $(wildcard $(SRCDIR)/*.c)
INC    = $(wildcard $(SRCDIR)/*.h)
OBJ    = $(SRC:.c=.o)
CC     = clang
has_clang = $(shell clang --version 2> /dev/null)
ifeq (, $(has_clang))
	CC = gcc
endif
CFLAGS = -std=c11 -Wall -Wextra -I$(SRCDIR) -fopenmp \
  `pkg-config --cflags openssl`
LDFLAGS = -lm -fopenmp `pkg-config --libs openssl`

TEST_SRC = $(wildcard $(TESTDIR)/*_tests.c)
TEST_OBJ = $(filter-out $(SRCDIR)/main.o,$(OBJ))
TESTS    = $(patsubst %.c,%,$(TEST_SRC))

.SUFFIXES:
.SUFFIXES: .c .o

all: $(TARGET) $(TESTS)

dev: CFLAGS += -g -pedantic
dev: all

$(TARGET): $(OBJ)
	@printf "\e[33mLinking\e[0m %s\n" $@
	$(CC) -o $@ $^ $(LDFLAGS)
	@printf "\e[34mMain target built\e[0m\n"

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@printf "\e[36mCompile\e[90m %s\e[0m\n" $@
	$(CC) -o $@ -c $< $(CFLAGS)

test: $(TESTS)
	@sh $(TESTDIR)/runtests.sh

$(TESTS): $(TEST_SRC)
	@printf "\e[32mBuilding test\e[0m %s\n" $@
	$(CC) -o $@ $^ $(TEST_OBJ) $(CFLAGS) $(LDFLAGS)
	@printf "\e[32mTests built\e[0m %s\n" $@

valgrind:
	VALGRIND="valgrind -v --leak-check=full --show-leak-kinds=all \
  --log-file=/tmp/valgrind-%p.log" \
  $(MAKE)

.PHONY: clean distclean

clean:
	rm -rf $(OBJ) $(TESTS)
	rm -f $(TESTDIR)/tests.log
	rm -f /tmp/valgrind-*
	rm -f $(TESTDIR)/*_tests
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
	@echo 'TEST     :' $(TESTS)
