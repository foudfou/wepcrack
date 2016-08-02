EXE = wepcrack
SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)
CC = clang
has_clang = $(shell clang --version 2> /dev/null)
ifeq (, $(has_clang))
	CC = gcc
endif
CFLAGS = -std=c11 -g -pedantic
LDFLAGS =

all: $(EXE)
	@printf "$(EXE)\n"

$(EXE): $(OBJ)
	@$(CC) -o $@ $^ $(LDFLAGS)
	@printf "\e[33mLinking\e[0m %s\n" $@
	@printf "\e[34mDone!\e[0m\n"

%.o: %.c
	@$(CC) -o $@ -c $< $(CFLAGS)
	@printf "\e[36mCompile\e[90m %s\e[0m\n" $@

.PHONY: clean mrproper

clean:
	@rm -rf *.o
	@printf "\e[34mAll clear!\e[0m\n"&

mrproper: clean
	@rm -rf $(EXEC)


# Show variables (for debug use only.)
show:
	@echo 'EXE      :' $(EXE)
	@echo 'SRC      :' $(SRC)
	@echo 'OBJ      :' $(OBJ)
	@echo 'CFLAGS   :' $(CFLAGS)
	@echo 'LDFLAGS  :' $(LDFLAGS)
	@echo 'CC       :' $(CC)
