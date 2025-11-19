# --- Compiler configuration ---
CC = gcc
CFLAGS = -Wall -Wextra -I. -Iserver -Iclient -Iserver/dbms -Itcp_lib
LDFLAGS =

# --- DBMS source files (used by server) ---
DBMS_SRC = server/dbms/sql_parser.c \
		  server/dbms/sql_executor.c

# --- Server-specific source files ---
SERVER_SRC = server/server_main.c \
             tcp_lib/tcp_lib.c \
             $(DBMS_SRC)
SERVER_OBJ = $(SERVER_SRC:.c=.o)
SERVER_BIN = server_app.o

# --- Client-specific source files ---
CLIENT_SRC = client/client_main.c \
             tcp_lib/tcp_lib.c
CLIENT_OBJ = $(CLIENT_SRC:.c=.o)
CLIENT_BIN = client_app.o

# --- Default rule: build both server and client ---
all: $(SERVER_BIN) $(CLIENT_BIN)

# --- Link the final server executable ---
$(SERVER_BIN): $(SERVER_OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) $(SERVER_OBJ) -o $(SERVER_BIN)

# --- Link the final client executable ---
$(CLIENT_BIN): $(CLIENT_OBJ)
	$(CC) $(CFLAGS) $(LDFLAGS) $(CLIENT_OBJ) -o $(CLIENT_BIN)

# --- Generic rule to compile any .c file into a .o file ---
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# --- Clean all generated files ---
clean:
	rm -f server/*.o server/dbms/*.o client/*.o tcp_lib/*.o *.o $(SERVER_BIN) $(CLIENT_BIN)
run-server: $(SERVER_BIN)
	./$(SERVER_BIN)

run-client: $(CLIENT_BIN)
	./$(CLIENT_BIN)

.PHONY: all clean run-server run-client