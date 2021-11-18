#ifndef MYI2C
#define MYI2C
#include "avr/io.h"
#include <stdbool.h>
#include <stdlib.h>
#include <avr/interrupt.h>


void i2csetup();
typedef void(I2C_0_callback)(void);

void I2C_0_init(void);

void I2C_0_open(void);

void I2C_0_close(void);

void I2C_0_isr(void);

uint8_t I2C_0_read(void);

void I2C_0_write(uint8_t data);

void I2C_0_enable(void);

void I2C_0_send_ack(void);

void I2C_0_send_nack(void);

void I2C_0_goto_unaddressed(void);

void I2C_0_set_read_callback(I2C_0_callback handler);

void I2C_0_set_write_callback(I2C_0_callback handler);

void I2C_0_set_address_callback(I2C_0_callback handler);

void I2C_0_set_stop_callback(I2C_0_callback handler);

void I2C_0_set_collision_callback(I2C_0_callback handler);

void I2C_0_set_bus_error_callback(I2C_0_callback handler);



#endif