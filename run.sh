#!/bin/bash

SRC_DIR="src"
BIN_DIR="bin"

mkdir -p $BIN_DIR

compile_client() {
    echo "compiling client..."
    gcc -Wall -Wextra -O2 -g -c $SRC_DIR/client.c -o client.o
    gcc -Wall -Wextra -O2 -g -c $SRC_DIR/packet.c -o packet.o
    gcc client.o packet.o -o $BIN_DIR/client -lm
    rm client.o packet.o
    echo "client done"
}

compile_server() {
    echo "compiling server..."
    gcc -Wall -Wextra -O2 -g -c $SRC_DIR/server.c -o server.o
    gcc -Wall -Wextra -O2 -g -c $SRC_DIR/hashmap.c -o hashmap.o
    gcc -Wall -Wextra -O2 -g -c $SRC_DIR/packet.c -o packet.o
    gcc server.o hashmap.o packet.o -o $BIN_DIR/server -lm
    rm server.o hashmap.o packet.o
    echo "server done"
}

run_client() {
    if [ ! -f "$BIN_DIR/client" ]; then
        compile_client
    fi
    ./$BIN_DIR/client
}

run_server() {
    if [ ! -f "$BIN_DIR/server" ]; then
        compile_server
    fi
    ./$BIN_DIR/server
}

clean() {
    rm -rf $BIN_DIR
}

case "$1" in
    compile-server)
        compile_server
        ;;
    compile-client)
        compile_client
        ;;
    compile)
        compile_server
        compile_client
        ;;
    run-server)
        run_server
        ;;
    run-client)
        run_client
        ;;
    clean)
        clean
        ;;
    *)
        echo "options: compile, compile-server, compile-client, run-server, run-client, clean"
        ;;
esac



