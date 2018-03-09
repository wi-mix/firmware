#ifndef MODELS_H
#define MODELS_H
typedef struct ingredient
{
    int id;
    short amount;
    short order;
} ingredient;

typedef struct recipe
{
    ingredient ingredients[3];
    unsigned char ordered;
    unsigned char ingredient_count;
} recipe;

typedef struct canister
{
    int canister_id;
    ingredient * ingredient;
} canister;

#endif //MODELS_H