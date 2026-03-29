#include "../packet.h"


const char *OPERATION_KEYWORDS[] = {
    "connect",
    "insert",
    "delete",
    "rehash",
    "terminate",
    "lookup",
};


/* internal_str_to_size_t */
/* Convert a string to a valid size_t */
bool internal_str_to_size_t(const char *string, size_t *out) {

    char *end = NULL;
    errno = 0;

    size_t val = strtoull(string, &end, 10);
    if(errno != 0 || *end != '\0' || string == end) {
        return false;
    }
    *out = val;
    return true;
}


/* internal_clean_string */
/* Clean an input string by removing newlines */
void clean_input(char *string) {
    
    char *c = string;
    while(c) {
        if(*c == '\n') {
            *c = '\0';
        }
    }

    return;
}


/* create_packet */
/* Create a packet from a corrosponding string, handling all errors */
bool create_packet(char *input, Packet *packetOut) {
    if(!input) {
        printf("\tEmpty client request\n");
        return false;
    }
    size_t inputLength = strlen(input);
    if(inputLength > MAX_INPUT_SIZE) {
        printf("\tMaximum input size (%d chars) exceeded\n", MAX_INPUT_SIZE);
        return false;
    }

    char *operation = strtok(input, " \r\n");
    if(!operation) {
        return false;
    }
    char *field1 = strtok(NULL, " \r\n");
    char *field2 = strtok(NULL, " \r\n");
    char *field3 = strtok(NULL, " \r\n");

    if(strcmp(OPERATION_KEYWORDS[0], operation) == 0) { //Connect
        packetOut->metadata.op = CONNECT;
        if(!field1) {
            printf("\tExpect IP address field for connection\n");
            return false;
        }
        packetOut->data.field1 = field1; //ip address
        packetOut->metadata.field1Length = strlen(field1);
        
    } else if(strcmp(OPERATION_KEYWORDS[1], operation) == 0) { //Insert
        packetOut->metadata.op = INSERT;
        if(!field1) {
            printf("\tExpect key field\n");
            return false;
        }
        if(!field2) {
            printf("\tExpect value field\n");
            return false;
        }
        packetOut->data.field1 = field1; //Key
        packetOut->metadata.field1Length = strlen(field1);

        packetOut->data.field2 = field2; //Value
        packetOut->metadata.field2Length = strlen(field2);

    } else if(strcmp(OPERATION_KEYWORDS[2], operation) == 0) { //Delete
        packetOut->metadata.op = DELETE;
        if(!field1) {
            printf("\tExpect key field\n");
            return false;
        }
        packetOut->data.field1 = field1; //Key
        packetOut->metadata.field1Length = strlen(field1);

    } else if(strcmp(OPERATION_KEYWORDS[3], operation) == 0) { //Rehash
        packetOut->metadata.op = REHASH;
        if(!internal_str_to_size_t(field3, &(packetOut->data.field3))) {
            printf("\tInvalid map size passed\n");
            return false;
        }

    } else if(strcmp(OPERATION_KEYWORDS[4], operation) == 0) { //Terminate
        packetOut->metadata.op = TERMINATE;

    } else if(strcmp(OPERATION_KEYWORDS[5], operation) == 0) { //Lookup
        packetOut->metadata.op = LOOKUP;
        if(!field1) {
            printf("\tExpect key field\n");
            return false;
        }
        packetOut->data.field1 = field1; //Key
        packetOut->metadata.field1Length = strlen(field1);

    } else {
        printf("\tUnrecognised operation: '%s'\n", operation);
        return false;
    }


    return true;
}


/* process_packet */
/* Process a packet request from a client */
bool process_packet(Packet *packet, char *response) {

    static bool connected = false;
    static int sockfd = -1; //Only allow connection to one server for now
    if(!connected && packet->metadata.op != CONNECT) {
        printf("Not connected to server\n");
        return false;
    }

    switch(packet->metadata.op) {
        case CONNECT: {
           
            printf("\tAttempting connection to '%s'\n", packet->data.field1);

            sockfd = socket(AF_INET, SOCK_STREAM, 0); 
            if(sockfd < 0) {
                printf("Socket establishment error\n");
                return false;
            }

            struct sockaddr_in serverAddr;
            memset(&serverAddr, 0, sizeof(serverAddr));
            serverAddr.sin_family = AF_INET;
            serverAddr.sin_port = htons(PORT);

            if(inet_pton(AF_INET, packet->data.field1, &(serverAddr.sin_addr)) <= 0) {
                printf("inet_pton error\n");
                close(sockfd);
                return false;
            }

            if(connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
                printf("Connection error: %s\n", strerror(errno));
                close(sockfd);
                return false;
            }
            printf("Connected\n");
            connected = true;



            return true;

        } case INSERT: {
            //Send operation, key, value

            ssize_t sent = send(sockfd, &(packet->metadata.op), sizeof(packet->metadata.op), 0);
            if(sent < 0) {
                printf("Error sending operation to server\n");
                return false;
            }

            sent = send(sockfd, packet->data.field1, packet->metadata.field1Length, 0); //Key
            if(sent < 0) {
                printf("Error sending field_1 to server\n");
                return false;
            }

            sent = send(sockfd, packet->data.field2, packet->metadata.field2Length, 0); //Value
            if(sent < 0) {
                printf("Error sending field_2 to server\n");
                return false;
            }

            return true;

        } case DELETE: {

            //Send operation, key

            ssize_t sent = send(sockfd, &(packet->metadata.op), sizeof(packet->metadata.op), 0);
            if(sent < 0) {
                printf("Error sending operation to server\n");
                return false;
            }

            sent = send(sockfd, packet->data.field1, packet->metadata.field1Length, 0); //Key
            if(sent < 0) {
                printf("Error sending field_1 to server\n");
                return false;
            }

            return true;

        } case LOOKUP: {

            //Send operation, key

            ssize_t sent = send(sockfd, &(packet->metadata.op), sizeof(packet->metadata.op), 0);
            if(sent < 0) {
                printf("Error sending operation to server\n");
                return false;
            }

            sent = send(sockfd, packet->data.field1, packet->metadata.field1Length, 0); //Key
            if(sent < 0) {
                printf("Error sending field_1 to server\n");
                return false;
            }


            response[MAX_INPUT_SIZE - 1] = '\0';
            if(recv(sockfd, response, MAX_INPUT_SIZE, 0) <= 0) {
                printf("Incomplete stream recieved from server\n");
                return false;
            }
            if(response[MAX_INPUT_SIZE - 1] != '\0') {
                printf("Server response overflowed client buffer\n");
                return false;
            }
            return true;

        } case REHASH: {

            ssize_t sent = send(sockfd, &(packet->metadata.op), sizeof(packet->metadata.op), 0);
            if(sent < 0) {
                printf("Error sending operation to server\n");
                return false;
            }

            sent = send(sockfd, &(packet->data.field3), sizeof(packet->data.field3), 0); //New size
            if(sent < 0) {
                printf("Error sending field_3 to server\n");
                return false;
            }
            return true;

        } case TERMINATE: {

            close(sockfd);
            return true;
        }
    }

    return true;
}

