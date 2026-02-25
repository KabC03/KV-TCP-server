#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include "../commands.h"
#include "../message.h"

#define VERSION 1
#define PORT 9000 //Common for TCP
#define MAX_MESSAGE_LENGTH 64000


bool connect(ClientRequest *request) {


    return true;
}


/** parse_input
 * 
 * @brief parse an input command and output a corrosponding requestOut
 * 
 * @param input :: Input command string
 * @param requestOut :: Corrosponding output request to input
 * 
 * @return bool :: Indication of if parsing occured successfully
 */
bool parse_input(char *input, ClientRequest *requestOut) {
    
    requestOut->field1 = NULL;
    requestOut->field1Length = 0;
    requestOut->field2 = NULL;
    requestOut->field2Length = 0;
    requestOut->field3 = 0;

    requestOut->version = VERSION;

    char *operation = strtok(input, " \n");
    if(!operation) {
        //No operation passed
        return false;
    }

    if(strcmp(operation, "connect") == 0) {
        requestOut->request = CONNECT;
        requestOut->field1 = strtok(NULL, " \n"); //ip

        if(!requestOut->field1) {
            return false;
        }
        requestOut->field1Length = strlen(requestOut->field1);

    } else if(strcmp(operation, "terminate") == 0) {
        requestOut->request = TERMINATE;

    } else if(strcmp(operation, "insert") == 0) {
        requestOut->request = INSERT;
        requestOut->field1 = strtok(NULL, " \n"); //key

        requestOut->field2 = strtok(NULL, " \n"); //value
        if(!requestOut->field1 || !requestOut->field2) {
            return false;
        }
        requestOut->field1Length = strlen(requestOut->field1);
        requestOut->field2Length = strlen(requestOut->field2);

    } else if(strcmp(operation, "find") == 0) {
        requestOut->request = FIND;
        requestOut->field1 = strtok(NULL, " \n"); //key

        if(!requestOut->field1) {
            return false;
        }
        requestOut->field1Length = strlen(requestOut->field1);

    } else if(strcmp(operation, "rehash") == 0) {
        requestOut->request = REHASH;
        requestOut->field1 = strtok(NULL, " \n"); //new size
        if(!requestOut->field1) {
            return false;
        }
        //Check if conversion to size_t is valid

        char *end = NULL;
        requestOut->field3 = strtoul(requestOut->field1, &end, 10);
        if(end == requestOut->field1 || *end != '\0') { //Invalid size passed
            return false;
        }
        requestOut->field1Length = strlen(requestOut->field1);

    } else if(strcmp(operation, "editv") == 0) {
        requestOut->request = EDIT_VALUE;
        requestOut->field1 = strtok(NULL, " \n"); //key

        requestOut->field2 = strtok(NULL, " \n"); //value

        if(!requestOut->field1 || !requestOut->field2) {
            return false;
        }
        requestOut->field1Length = strlen(requestOut->field1);
        requestOut->field2Length = strlen(requestOut->field2);

    } else if(strcmp(operation, "delete") == 0) {
        requestOut->request = DELETE;
        requestOut->field1 = strtok(NULL, " \n"); //key

        if(!requestOut->field1) {
            return false;
        }
        requestOut->field1Length = strlen(requestOut->field1);

    } else {
        return false;
    }



    return true;
}


int main(void) {

    char arr[] = "rehash dsa";
    ClientRequest req;

    if(!parse_input(arr, &req)) {
        return -1;
    }

    printf("%d, %s, %s, %zu\n", req.request, req.field1, req.field2, req.field3);

    return 0;
}


