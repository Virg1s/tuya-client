TUYA_BUILD := ../lib/tuya_build
TUYA_SRC := ../lib/tuya_sdk

LDFLAGS := -L ${TUYA_BUILD}/lib
LDLIBS :=  -lplatform_port -lmiddleware_implementation -lutils_modules -llink_core
CPPFLAGS := -I ${TUYA_SRC}/utils -I ${TUYA_SRC}/interface -I ${TUYA_SRC}/include -I ../include

BIN:=main.out
SRCS:=$(wildcard *.c)
OBJS:=$(SRCS:.c=.o)

CC := gcc
CFLAGS = -Wall -Wextra

.PHONY: all clean fresh

all: main.out

clean:
	$(RM) $(wildcard *.o) main.out

fresh: clean all

debug: CFLAGS += -ggdb3
debug: fresh

%.o:%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $^


$(BIN): $(OBJS)
	$(CC) $^ -o $@ $(LDFLAGS) $(LDLIBS) 
	
