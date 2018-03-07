#ifndef CMD_CONTROL_H
#define CMD_CONTROL_H
#include "../models/models.h"
#include <stdbool.h>

typedef enum dispensing_status
{
    ACCEPTING,
    DISPENSING
} dispensing_status;


typedef struct command_controller
{
    dispensing_status state, //Status of the controller
    int heartbeat_timer, // How often to send updates to the PROWF
    void (*dispense)(command_controller * this, recipe * recipe),
    void (*command_handler)(command_controller * this),
    recipe * current_recipe,
    bool busy
} command_controller;

command_controller * initialize_cmd_ctrl();

//Task prototype for dispensing
void  DispensingTask (void *p_arg);


#endif //__CMD_CONTROL_H