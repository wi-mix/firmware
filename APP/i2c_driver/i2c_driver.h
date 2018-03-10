#define I2C2_INTERRUPT_VECTOR 192

void init_I2C2_interrupt(void);

void init_I2C2(ALT_I2C_DEV_t * device);

//Destructive read of RX buffer
int read(void * data, size_t size);

//Bulk write of size bytes from data
ALT_STATUS_CODE write(void * data, size_t size);