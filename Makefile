CC=clang
CFLAGS= -lclang -std=c11 -Wall -Werror


all: Rigel

csmith:
	$(MAKE) -C csmith


Rigel: csmith main.c gen_cmm.c emi.c clang_helper.c
	$(CC) $(CFLAGS) main.c gen_cmm.c emi.c clang_helper.c -o Rigel


test: Rigel
	./Rigel

.PHONY: clean csmith all clean_all

clean:
	@rm ./Rigel

clean_all:
	$(MAKE) -C csmith clean
	@rm ./Rigel