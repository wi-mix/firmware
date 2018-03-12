#ifndef MODELS_H
#define MODELS_H
typedef struct dispensing_ingredient
{
    short amount;
    unsigned char order;
} dispensing_ingredient;

typedef struct recipe
{
	dispensing_ingredient ingredient[3];
    unsigned char ordered;
} recipe;

#endif //MODELS_H
