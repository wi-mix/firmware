#ifndef MODELS_H
#define MODELS_H
#include <stdint.h>
typedef struct dispensing_ingredient
{
    short amount;
    unsigned char order;
} dispensing_ingredient;

typedef struct recipe
{
	dispensing_ingredient ingredients[3];
    unsigned char ordered;
} recipe;

typedef struct motor_command
{
	char state;
	uint8_t motor_num;
} motor_command;

typedef struct pour_command
{
	char isOrdered;
	int32_t target_volume;
	uint8_t canister_index;
} pour_command;

#endif //MODELS_H
