CC=clang
CFLAGS= -lclang -std=c11 -Wall -Werror


all: emi_checker

csmith:
	$(MAKE) -C csmith


emi_checker: csmith main.c gen_cmm.c emi.c clang_helper.c
	$(CC) $(CFLAGS) main.c gen_cmm.c emi.c clang_helper.c -o emi_checker


test: emi_checker
	./emi_checker

.PHONY: clean csmith all clean_all

clean:
	@rm ./emi_checker

clean_all:
	$(MAKE) -C csmith clean
	@rm ./emi_checker