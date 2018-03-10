#include  <os.h>
#include <stdbool.h>
#include "command_controller.h"
#include "../HWLIBS/alt_i2c.h"
#include "../models/models.h"
#include "../i2c_driver/i2c_driver.h"

void DispensingTask (void *p_arg)
{
    //Dereference controller
    command_controller * ctrl = (command_controller *)p_arg;

    recipe * recipe = ctrl->current_recipe;
    ctrl->state = DISPENSING;

    //Dispense liquid

    //Update liquid levels

    //Unbusy
    ctrl->state = ACCEPTING;
    OSTaskDel(OS_PRIO_SELF);
}

void command_handler(command_controller * controller)
{
    //Use I2C to read new command from the PROWF
    recipe * my_recipe = (recipe *)malloc(sizeof(recipe));
    //dispense
    controller->dispense(controller, my_recipe);
}

void dispense(command_controller * controller, recipe * my_recipe)
{
    controller->state = DISPENSING;
    free(controller->current_recipe);

    controller->current_recipe = my_recipe;
}

command_controller * initialize_cmd_ctrl(command_controller * controller)
{
    controller->state = ACCEPTING;
    controller->dispense = &dispense;
    controller->command_handler = &command_handler;

    init_I2C2(&(controller->command_i2c));
}
