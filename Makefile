
NAME=spark
proc=main
elib=functions
ecli=client

CC=gcc
#STRIP=strip
RM=rm

CFLAG =-std=gnu99 -O0 -Wall -g -D_GNU_SOURCE -D_REENTERANT

INC=/usr/include

MACHINE := $(shell uname -m)
ifeq ($(MACHINE), x86_64)
	LIB_DIR = /usr/lib
else
	LIB_DIR = /usr/lib32
endif

$(NAME): $(elib).o $(ecli).o $(proc).o
	$(CC) -o $(NAME) $(elib).o $(ecli).o $(proc).o libiconv_hook.a -L$(LIB_DIR) -lpthread -ldl
#	$(STRIP) $(NAME)
$(proc).o: $(proc).c
	$(CC) -c $(proc).c $(CFLAG) -I$(INC)
$(ecli).o: $(ecli).c
	$(CC) -c $(ecli).c $(CFLAG) -I$(INC)
$(elib).o: $(elib).c
	$(CC) -c $(elib).c $(CFLAG) -I$(INC)

clean:
	$(RM) -f *.o $(NAME)

