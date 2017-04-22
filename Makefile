CC=clang
CFLAGS= -lclang -std=c11 -Wall -Werror

all: main.c
	$(CC) $(CFLAGS) main.c -o emi_checker
	./emi_checker

.PHONY: clean
clean:
	@rm ./emi_checker
