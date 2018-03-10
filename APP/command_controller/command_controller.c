#include  <os.h>
#include <stdbool.h>
#include "command_controller.h"
#include "../HWLIBS/alt_i2c.h"
#include "../models/models.h"
#include "../i2c_driver/i2c_driver.h"

#define CMD_QUEUE_SIZE 1
OS_EVENT* cmd_queue;
void* cmd_msg_queue[CMD_QUEUE_SIZE];

static command_controller cmd_controller;

void DispensingTask (void *p_arg)
{
    //Dereference controller
    command_controller * ctrl = (command_controller *)p_arg;

    recipe * my_recipe = ctrl->current_recipe;
    ctrl->state = DISPENSING;

    //Dispense liquid

    //Update liquid levels

    //Unbusy
    ctrl->state = ACCEPTING;
    OSTaskDel(OS_PRIO_SELF);
}

/*
* Processes commands and launches dispensing or level reading depending on outcome
*/
void CommandProcessingTask(void *p_arg)
{
    command_t command;
    command_controller * ctrl = (command_controller *)p_arg;
    while(true)
    {
        //Use I2C to read new command from the PROWF
        read((void *)&command, sizeof(command_t));
        command_handler(ctrl, command);
    }
}

void command_handler(command_controller * controller, command_t command)
{
    my_recipe = (recipe *)malloc(sizeof(recipe));
    switch(command)
    {
        case READ_LEVELS:
            read_levels();
            break;
        case DISPENSE_REQUEST:
            write((void *)&(controller->state), sizeof(dispensing_status));
            break;
        case DISPENSE:
            get_recipe(controller, my_recipe);
            dispense(controller, my_recipe);
            break;
        default:
            break;
    }
}

void get_recipe(recipe * my_recipe)
{
    read((void*)my_recipe, sizeof(recipe));
}

void read_levels(void)
{
    //Read levels from the dac into this array
    uint16_t levels[3] = {0};

    write((void *)&(levels[0]), sizeof(levels));
}

void dispense(command_controller * controller, recipe * my_recipe)
{
    controller->state = DISPENSING;
    free(controller->current_recipe);

    controller->current_recipe = my_recipe;
}

command_controller * initialize_cmd_ctrl()
{
    cmd_controller->state = ACCEPTING;
    cmd_controller->dispense = &dispense;
    cmd_controller->command_handler = &command_handler;
    cmd_queue = OSQCreate(cmd_msg_queue, CMD_QUEUE_SIZE);
    init_I2C2(&(cmd_controller->command_i2c));
    return &cmd_controller;
}
