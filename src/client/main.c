#include "../packet.h"
#include <signal.h>


int main(void) {

    char inputBuffer[MAX_INPUT_SIZE];
    Packet packet;

    char *response = NULL;

    while(1) {
        //NOTE: Bad practise expecting ctrl + c to escape program
        //Relies on OS cleaning up if user does not terminate connection first
        //Maybe catch ctrl + c then cleanup manually before exiting

        printf("Client ready\n");
        fgets(inputBuffer, MAX_INPUT_SIZE, stdin);

        if(!create_packet(inputBuffer, &packet)) {
            continue;

        } 

        if(!process_packet(&packet, response)) {
            continue;
        }
    }



    return 0;
}




