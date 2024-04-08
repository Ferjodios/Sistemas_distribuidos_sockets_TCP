# Variables 
CC = gcc
CFLAGS = -Wall -Werror

# LIBRERIA DINAMICA
PROXY_SOURCES = proxy.c
PROXY_OBJECTS = $(PROXY_SOURCES:.c=.o)
PROXY = libclaves.so

#SERVIDOR 
SERVER_SOURCES = servidor.c claves.c list.c lines.c
SERVER_OBJECTS = $(SERVER_SOURCES:.c=.o)
SERVER = servidor

#CLIENTE
CLIENT_SOURCES = cliente.c
CLIENT_OBJECTS = $(CLIENT_SOURCES:.c=.o)
CLIENT = cliente

all: $(PROXY) $(SERVER) $(CLIENT)

$(PROXY): $(PROXY_OBJECTS)
	$(CC) -shared -o $(PROXY) $(PROXY_OBJECTS) -lrt

$(PROXY_OBJECTS): $(PROXY_SOURCES)
	$(CC) $(CFLAGS) -fPIC -c $< -o $@

$(CLIENT): $(CLIENT_OBJECTS)
	$(CC) $(CFLAGS) -o $(CLIENT) $(CLIENT_OBJECTS) -L. -lclaves -Wl,-rpath,. -lpthread

$(SERVER): $(SERVER_OBJECTS)
	$(CC) $(CFLAGS) -o $(SERVER) $(SERVER_OBJECTS) -lrt -lpthread

runc1:
	./cliente 1

runc2:
	./cliente 2

runc3:
	./cliente 3
runs:
	./servidor

fclean: clean
	rm -f $(CLIENT) $(SERVER) $(PROXY)
clean:
	rm -f $(PROXY_OBJECTS) $(SERVER_OBJECTS) $(CLIENT_OBJECTS)

re: fclean all
