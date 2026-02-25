#ifndef COMMANDS_H
#define COMMANDS_H

typedef enum REQUEST {
    CONNECT, //connect"192.168.0.1"
    TERMINATE, //terminate
    INSERT, //insert "key" "value"
    DELETE, //delete "key"
    FIND, //find "key"
    REHASH, //rehash "newSize"
    EDIT_VALUE, //editv "key" "newValue"
} REQUEST;

typedef enum RESPONSE {

    SUCCESS,
    FAILURE,

} RESPONSE;

#endif
