TUYA_BUILD := ../lib/tuya_build
TUYA_SRC := ../lib/tuya_sdk

LIBTUYA_LIBS := -L ${TUYA_BUILD}/lib -lplatform_port -lmiddleware_implementation -lutils_modules -llink_core
LIBTUYA_INCL := -I ${TUYA_SRC}/utils -I ${TUYA_SRC}/interface -I ${TUYA_SRC}/include -I ../include


CC := gcc
CFLAGS = -Wall -Wextra

.PHONY: all clean fresh tuyadbg

all: main.out

clean:
	$(RM) $(wildcard *.o) main.out

fresh: clean all

debug: CFLAGS += -ggdb3
debug: fresh

tuyadbg: tuya_interface.c
	$(CC) $(CFLAGS) $(LIBTUYA_INCL) $^ -o $@ $(LIBTUYA_LIBS) 

lib_tuya.o: tuya_interface.c
	$(CC) $(CFLAGS) $(LIBTUYA_INCL) $< -c -o $@ 

tuyarepd.o: tuyarepd.c
	$(CC) $(CFLAGS) $(LIBTUYA_INCL) $< -c -o $@ 

main.out: lib_tuya.o tuyarepd.o
	$(CC) $(CFLAGS) $^ -o $@ $(LIBTUYA_LIBS) 
	
