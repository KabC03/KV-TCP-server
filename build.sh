
clear
clear
clear
clang -Wall -Werror ./src/client/main.c ./src/server/hashmap.c ./src/client/client.c -o ./bin/out -fsanitize=address
./bin/out

