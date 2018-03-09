#include  <os.h>
#include <stdbool.h>
#include "command_controller.h"
#include "../models/models.h"


void DispensingTask (void *p_arg)
{
    //Dereference controller
    command_controller * ctrl = (command_controller *)p_arg;

    recipe * recipe = ctrl->current_recipe;
    ctrl->busy = true;

    //Dispense liquid

    //Update liquid levels

    //Unbusy
    ctrl->busy = false;
    OSTaskDel(OS_PRIO_SELF);
}

void command_handler(command_controller * controller)
{

}

void dispense(command_controller * controller, recipe * recipe)
{

}

command_controller * initialize_cmd_ctrl(command_controller * controller)
{
    controller->busy = false;
    controller->state = ACCEPTING;
    controller->heartbeat_timer = 100;

    controller->dispense = &dispense;
    controller->command_handler = &command_handler;
}