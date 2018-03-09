#ifndef CMD_CONTROL_H
#define CMD_CONTROL_H
#include "../models/models.h"
#include <stdbool.h>

#define CMD_TASK_PRIO 4
#define DISPENSE_TASK_PRIO 5
typedef enum dispensing_status
{
    ACCEPTING,
    DISPENSING
} dispensing_status;

typedef struct command_controller command_controller;
struct command_controller
{
    dispensing_status state; //Status of the controller
    void (*dispense)(command_controller *, recipe *);
    void (*command_handler)(command_controller *);
    recipe * current_recipe;
    ALT_I2C_DEV_t command_i2c;
};

command_controller * initialize_cmd_ctrl(command_controller * controller);

//Task prototype for dispensing
//Takes a command_controller pointer
void  DispensingTask (void *p_arg);

void dispense(command_controller * controller, recipe * my_recipe);

void command_handler(command_controller * controller);

#endif //__CMD_CONTROL_H