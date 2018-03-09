#include  <os.h>
#include <stdbool.h>
#include "../HWLIBS/alt_i2c.h"
#include "../models/models.h"
static ALT_I2C_DEV_t * i2c2_device;
void init_I2C2(ALT_I2C_DEV_t * device)
{
    i2c2_device = device;
    alt_i2c_init(ALT_I2C_I2C2, i2c2_device);

    alt_i2c_op_mode_set(i2c2_device, ALT_I2C_MODE_SLAVE);

    ALT_I2C_SLAVE_CONFIG_t slave_config = { .addr_mode = ALT_I2C_ADDR_MODE_7_BIT,
                                            .addr = 17,
                                            .nack_enable = false};
    
    alt_i2c_slave_config_set(i2c2_device, &slave_config);
    
}

void init_I2C2_interrupt(void)
{
    
}