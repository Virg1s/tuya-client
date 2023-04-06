LIBTUYA_LIBS := -L ./templib -lplatform_port -lmiddleware_implementation -lutils_modules -llink_core 
LIBTUYA_INCL := -I ./tempinc
CC := gcc
CFLAGS = -Wall -Wextra

.PHONY: all clean fresh tuyadbg

all: main.out

clean:
	$(RM) $(wildcard *.o) main.out

fresh: clean all

debug: CFLAGS += -ggdb3
debug: fresh

tuyadbg: tuya_lib.c
	$(CC) $(CFLAGS) $(LIBTUYA_INCL) $^ -o $@ $(LIBTUYA_LIBS) 

lib_tuya.o: tuya_lib.c
	$(CC) $(CFLAGS) $(LIBTUYA_INCL) $< -c -o $@ 

tuyarepd.o: tuyarepd.c
	$(CC) $(CFLAGS) $(LIBTUYA_INCL) $< -c -o $@ 

main.out: lib_tuya.o tuyarepd.o
	$(CC) $(CFLAGS) $^ -o $@ $(LIBTUYA_LIBS) 
	
