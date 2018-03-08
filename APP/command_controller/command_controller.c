#include  <os.h>
#include <stdbool.h>
#include "command_controller.h"
#include "../models/models.h"


void DispensingTask (void *p_arg)
{
    //Dereference controller
    command_controller * ctrl = (command_controller *)p_arg;

    recipe * recipe = ctrl->recipe;
    ctrl->busy = true;

    //Dispense liquid

    //Update liquid levels

    //Unbusy
    ctrl->busy = false;
    OSTaskDel(OS_PRIO_SELF);
}