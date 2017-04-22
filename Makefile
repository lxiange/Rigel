CC=clang
CFLAGS= -lclang -std=c11 -Wall -Werror


all: emi_checker

csmith:
	$(MAKE) -C csmith

emi_checker: csmith
	$(CC) $(CFLAGS) main.c gen_cmm.c -o emi_checker



test:
	./emi_checker

.PHONY: clean csmith all

clean:
	$(MAKE) -C csmith clean
	@rm ./emi_checker
