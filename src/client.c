#include "./packet.h"

int main(void) {
    char input[MAX_INPUT_SIZE];
    char response[MAX_INPUT_SIZE];
    Packet packet;

    while(true) {
        printf(">> ");
        if(!fgets(input, sizeof(input), stdin)) {
            break;
        }

        if(!create_packet(input, &packet)) {
            continue;
        }

        memset(response, 0, sizeof(response));

        if(process_packet(&packet, response)) {
            if(packet.metadata.op == LOOKUP) {
                printf("%s\n", response);
            }
            if(packet.metadata.op == TERMINATE) {
                break;
            }
        }
    }

    return 0;
}


