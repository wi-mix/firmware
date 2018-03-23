#include  <os.h>
#include <stdbool.h>
#include "command_controller.h"
#include "../HWLIBS/alt_i2c.h"
#include "../models/models.h"
#include "../i2c_driver/i2c_driver.h"

#define CMD_QUEUE_SIZE 1
#define CONTROLLER_STACK_SIZE 4096

CPU_STK Controller_Task_Stack[CONTROLLER_STACK_SIZE];

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
    // If they're ordered, the task will dispense each item in turn
        if(my_recipe->ordered){
        	startDispense(my_recipe->ingredient);
        }// Otherwise spawn all threads simultaneously
        else{
        	for(int i =0; i<3;i++){
        		startDispense(my_recipe.ingredient[i]);
        	}
        }
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
    printf("Command task started!\r\n");
    while(true)
    {
        //Use I2C to read new command from the PROWF
        read((void *)&command, sizeof(command_t));
        printf("Processing command: %d!\r\n", command);
        command_handler(ctrl, command);
    	OSTimeDlyHMSM(0, 0, 1, 0);
    }
}

void command_handler(command_controller * controller, command_t command)
{
    recipe* my_recipe = (recipe *)malloc(sizeof(recipe));
    switch(command)
    {
        case READ_LEVELS:
            read_levels();
            break;
        case DISPENSE_REQUEST:
            write((void *)&(controller->state), sizeof(dispensing_status));
            break;
        case DISPENSE:
            get_recipe(my_recipe);
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
    uint16_t levels[3] = {0xDEAD, 0xBEEF, 0xBABE};

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
	printf("Initializing cmd controller\r\n");
	uint8_t os_err;
    cmd_controller.state = ACCEPTING;
    cmd_controller.dispense = &dispense;
    cmd_controller.command_handler = &command_handler;
    cmd_queue = OSQCreate(cmd_msg_queue, CMD_QUEUE_SIZE);
    init_I2C2(&(cmd_controller.command_i2c));

    printf("Initialized I2C2\r\n");
    os_err = OSTaskCreateExt((void (*)(void *)) CommandProcessingTask,   /* Create the start task.                               */
                                 (void          * ) 0,
                                 (OS_STK        * )&Controller_Task_Stack[CONTROLLER_STACK_SIZE - 1],
                                 (INT8U           ) CMD_TASK_PRIO,
                                 (INT16U          ) CMD_TASK_PRIO,  // reuse prio for ID
                                 (OS_STK        * )&Controller_Task_Stack[0],
                                 (INT32U          ) CONTROLLER_STACK_SIZE,
                                 (void          * )0,
                                 (INT16U          )(OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK));

        if (os_err != OS_ERR_NONE) {
            ; /* Handle error. */
        }
    return &cmd_controller;
}
