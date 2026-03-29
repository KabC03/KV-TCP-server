#ifndef PACKET_H
#define PACKET_H 
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

#define MAX_INPUT_SIZE 1000
#define PORT 9000

typedef enum OPERATION {
    CONNECT, //Connect to a server
    INSERT, //Insert new key into map
    DELETE, //Delete key from map
    LOOKUP, //Lookup key in map
    REHASH, //Rehash entire map
    TERMINATE, //Terminate connection
} OPERATION;


typedef struct PacketMetadata {
    OPERATION op;
    size_t field1Length;
    size_t field2Length;
} PacketMetadata;

typedef struct PacketData {
    char *field1; 
    char *field2;
    size_t field3; //Number field
} PacketData;

typedef struct Packet {
    PacketMetadata metadata;
    PacketData data; 
} Packet;

/** 
 * @brief Create a packet from an input string, handling all errors
 * 
 * @param input :: Input string
 * @param packetOut :: Packet destination to write too
 * 
 * @return bool :: Indication of successful packet generation (true == success)
 */
bool create_packet(char *input, Packet *packetOut);




/** 
 * @brief Process a packet 
 * 
 * @param packet:: Packet to process (client -> server)
 * @param response :: Response from server
 * 
 * @return bool :: Indication of successful packet send (true == success)
 */
bool process_packet(Packet *packet, char *response);

#endif

