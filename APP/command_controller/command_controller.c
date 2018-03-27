#include  <os.h>
#include <stdbool.h>
#include <stdlib.h>
#include "command_controller.h"
#include "../HWLIBS/alt_i2c.h"
#include "../models/models.h"
#include "../dispensing/dispensing.h"
#include "../i2c_driver/i2c_driver.h"
#include "../adc/adc.h"

#define CMD_QUEUE_SIZE 1
#define CONTROLLER_STACK_SIZE 4096

CPU_STK Controller_Task_Stack[CONTROLLER_STACK_SIZE];
CPU_STK Dispensing_Task_Stack[CONTROLLER_STACK_SIZE];

OS_EVENT* cmd_queue;
void* cmd_msg_queue[CMD_QUEUE_SIZE];

static command_controller cmd_controller;

void DispensingTask (void *p_arg)
{
	printf("Starting dispensing \r\n");
    //Dereference controller
    command_controller * ctrl = (command_controller *)p_arg;

    recipe * my_recipe = ctrl->current_recipe;
    ctrl->state = DISPENSING;

    //Dispense liquid
    // If they're ordered, the task will dispense each item in turn
        if(my_recipe->ordered){
        	startDispenseOrdered(my_recipe->ingredients);
        }// Otherwise spawn all threads simultaneously
        else{
        	for(int i =0; i<3;i++){
        		startDispenseSingle(my_recipe->ingredients[i], i);
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
//        read((void *)&command, sizeof(command_t));
        //For testing:
        command = DISPENSE;
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
        	printf("DISPENSE option selected\r\n");
            get_recipe(my_recipe);
            dispense(controller, my_recipe);
            break;
        default:
            break;
    }
}

void get_recipe(recipe * my_recipe)
{
//    read((void*)my_recipe, sizeof(recipe));
	my_recipe->ordered = 0;
	my_recipe->ingredients[0].amount=100;
	my_recipe->ingredients[0].order=0;

	my_recipe->ingredients[0].amount=200;
	my_recipe->ingredients[0].order=0;

	my_recipe->ingredients[0].amount=300;
	my_recipe->ingredients[0].order=0;

}

void read_levels(void)
{
    //Read levels from the dac into this array
    uint16_t levels[3] = { 0 };
    for(int i =0;i<3;i++){
    	levels[i] = getCurrentVolume(i);
    }

    write((void *)&(levels[0]), sizeof(levels));
}

void dispense(command_controller * controller, recipe * my_recipe)
{
    controller->state = DISPENSING;
    if(controller->current_recipe != NULL)
	{
    	free(controller->current_recipe);
	}

    controller->current_recipe = my_recipe;
    printf("Starting dispensing task\r\n");
	uint8_t os_err;
    os_err = OSTaskCreateExt((void (*)(void *)) DispensingTask,   /* Create the start task.                               */
                                     (void          * ) controller,
                                     (OS_STK        * )&Dispensing_Task_Stack[CONTROLLER_STACK_SIZE - 1],
                                     (INT8U           ) DISPENSE_TASK_PRIO,
                                     (INT16U          ) DISPENSE_TASK_PRIO,  // reuse prio for ID
                                     (OS_STK        * )&Dispensing_Task_Stack[0],
                                     (INT32U          ) CONTROLLER_STACK_SIZE,
                                     (void          * )0,
                                     (INT16U          )(OS_TASK_OPT_STK_CLR | OS_TASK_OPT_STK_CHK));

            if (os_err != OS_ERR_NONE) {
                ; /* Handle error. */
            }
}

command_controller * initialize_cmd_ctrl()
{
	printf("Initializing cmd controller\r\n");
	uint8_t os_err;
    cmd_controller.state = ACCEPTING;
    cmd_controller.dispense = &dispense;
    cmd_controller.command_handler = &command_handler;
    cmd_controller.current_recipe = NULL;
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
