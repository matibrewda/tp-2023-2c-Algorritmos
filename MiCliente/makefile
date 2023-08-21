NAME=Ejecutable/cliente.o
LIBS= -lcommons -lpthread -lreadline
CC=gcc
SRCS=$(shell find  *.c)
SRCS2=$(shell find Source/*.c)
COMNS=$(shell find ../Shared/*.c)
CONFIG=$(shell find Configuracion/*.config)
LOGS=$(shell find Logs/*.log)
RM=rm -rf

all: clean build
build:
	$(CC) $(MODS) $(SRCS) $(SRCS2) $(COMNS) -o $(NAME) $(LIBS) -Werror
clean:
	$(RM) $(NAME)
	clear
debug:
	$(RM) $(LOGS)
	touch $(LOGS)
	clear