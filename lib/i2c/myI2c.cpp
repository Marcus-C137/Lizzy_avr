#include "myI2c.h"
#include "avr/io.h"
#include <stdbool.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include "mUSART.h"
#include "NTCthermistor.h"
#include "CurrentSensor.h"
#include "Heat.h"

#define Slave_Addr_Read ((TWSR0 & 0xF8) == 0xA8)             // Own SLA+R addr received and ACK has been returned
#define Slave_Addr_Write ((TWSR0 & 0xF8) == 0x60)            // Own SLA+W addr received and ACK has been returned
#define Slave_Read ((TWSR0 & 0xF8) == 0xB8)                  // Read data byte transmitted and ACK received
#define Slave_Write_Arb_lost ((TWSR0 & 0xF8) == 0x38)        // Arbitration Lost in SLA+W
#define Slave_Write_Received_back ((TWSR0 & 0xF8) == 0x68)   // Arbitration Lost in SLA+R/W as Master
#define Slave_Gencall_Received_back ((TWSR0 & 0xF8) == 0x78) // General Call Address received
#define Slave_Read_Received_back ((TWSR0 & 0xF8) == 0xB0)    // Own SLA+R has been received and ACK has been returned
#define Slave_Bus_Error ((TWSR0 & 0xF8) == 0x00)             // Bus Error
#define Slave_Write_Ack ((TWSR0 & 0xF8) == 0x80) // Earlier addressed with own SLA+W; Data received and ACK returned
#define Slave_Data_IRQ ((TWSR0 & 0xF8) == 0xC8)  // Last Data Byte has been transmitted and ACK has been received
#define Slave_Stop ((TWSR0 & 0xF8) == 0xA0)      // STOP Condition, Repeated START condition has been received
#define Slave_Not_Ack ((TWSR0 & 0xF8) == 0xC0)   // Data Byte has been transmitted and NOT ACK has been received
#define lowByte(w) ((uint8_t) ((w) & 0xff))
#define highByte(w) ((uint8_t) ((w) >> 8))

// Read Event Interrupt Handlers
void I2C_0_read_callback(void);
void (*I2C_0_read_interrupt_handler)(void);

// Write Event Interrupt Handlers
void I2C_0_write_callback(void);
void (*I2C_0_write_interrupt_handler)(void);

// Address Event Interrupt Handlers
void I2C_0_address_callback(void);
void (*I2C_0_address_interrupt_handler)(void);

// Stop Event Interrupt Handlers
void I2C_0_stop_callback(void);
void (*I2C_0_stop_interrupt_handler)(void);

// Bus Collision Event Interrupt Handlers
void I2C_0_collision_callback(void);
void (*I2C_0_collision_interrupt_handler)(void);

// Bus Error Event Interrupt Handlers
void I2C_0_bus_error_callback(void);
void (*I2C_0_bus_error_interrupt_handler)(void);

volatile int rdPosCounter = 0;
volatile int wPosCounter = 0;
volatile uint8_t wRead[5];


void i2csetup(){
    TWI0.SADDR = 0x69 << TWI_ADDRMASK_gp /* Slave Address: 0x69 */
                | 1 << TWI_ADDREN_bp;   

    TWI0.SCTRLA = 1 << TWI_APIEN_bp    /* Address/Stop Interrupt Enable: enabled */
            | 1 << TWI_DIEN_bp   /* Data Interrupt Enable: enabled */
            | 1 << TWI_ENABLE_bp /* Enable TWI Slave: enabled */
            | 1 << TWI_PIEN_bp   /* Stop Interrupt Enable: enabled */
            | 0 << TWI_PMEN_bp   /* Promiscuous Mode Enable: disabled */
            | 0 << TWI_SMEN_bp;  /* Smart Mode Enable: disabled */

	I2C_0_set_write_callback(NULL);
	I2C_0_set_read_callback(NULL);
	I2C_0_set_address_callback(NULL);
	I2C_0_set_stop_callback(NULL);
	I2C_0_set_collision_callback(NULL);
	I2C_0_set_bus_error_callback(NULL);
}


/**
 * \brief Open the I2C for communication. Enables the module if disabled.
 *
 * \return Nothing
 */
void I2C_0_open(void)
{
	TWI0.SCTRLA |= TWI_ENABLE_bm;
}

/**
 * \brief Close the I2C for communication. Disables the module if enabled.
 *
 * \return Nothing
 */
void I2C_0_close(void)
{
	TWI0.SCTRLA &= ~TWI_ENABLE_bm;
}

/**
 * \brief The function called by the I2C IRQ handler.
 * Can be called in a polling loop in a polled driver.
 *
 * \return Nothing
 */
void I2C_0_isr()
{
	static char isFirstByte = true; // to bypass the NACK flag for the first byte in a transaction

	if (TWI0.SSTATUS & TWI_COLL_bm) {
		I2C_0_collision_callback();
		return;
	}

	if (TWI0.SSTATUS & TWI_BUSERR_bm) {
		I2C_0_bus_error_callback();
		return;
	}

	if ((TWI0.SSTATUS & TWI_APIF_bm) && (TWI0.SSTATUS & TWI_AP_bm)) {
		I2C_0_address_callback();
		isFirstByte = true;
		return;
	}

	if (TWI0.SSTATUS & TWI_DIF_bm) {
		if (TWI0.SSTATUS & TWI_DIR_bm) {
			// Master wishes to read from slave
			if (!(TWI0.SSTATUS & TWI_RXACK_bm) || isFirstByte) {
				// Received ACK from master or First byte of transaction
				isFirstByte = false;
				I2C_0_read_callback();
				TWI0.SCTRLB = TWI_ACKACT_ACK_gc | TWI_SCMD_RESPONSE_gc;
			} else {
				// Received NACK from master
				I2C_0_goto_unaddressed();
			}
		} else // Master wishes to write to slave
		{
			I2C_0_write_callback();
		}
		return;
	}

	// Check if STOP was received
	if ((TWI0.SSTATUS & TWI_APIF_bm) && (!(TWI0.SSTATUS & TWI_AP_bm))) {
		I2C_0_stop_callback();
		TWI0.SCTRLB = TWI_SCMD_COMPTRANS_gc;
		return;
	}
}

ISR(TWI0_TWIS_vect)
{
	I2C_0_isr();
}

/**
 * \brief Read one byte from the data register of I2C_0
 *
 * Function will not block if a character is not available, so should
 * only be called when data is available.
 *
 * \return Data read from the I2C_0 module
 */
uint8_t I2C_0_read(void)
{
	return TWI0.SDATA;
}

/**
 * \brief Write one byte to the data register of I2C_0
 *
 * Function will not block if data cannot be safely accepted, so should
 * only be called when safe, i.e. in the read callback handler.
 *
 * \param[in] data The character to write to the I2C
 *
 * \return Nothing
 */
void I2C_0_write(uint8_t data)
{

	TWI0.SDATA = data;
	TWI0.SCTRLB |= TWI_SCMD_RESPONSE_gc;
}

/**
 * \brief Enable address recognition in I2C_0
 * 1. If supported by the clock system, enables the clock to the module
 * 2. Enables the I2C slave functionality  by setting the enable-bit in the HW's control register
 *
 * \return Nothing
 */
void I2C_0_enable(void)
{
	TWI0.SCTRLA |= TWI_ENABLE_bm;
}

/**
 * \brief Send ACK to received address or data. Should
 * only be called when appropriate, i.e. in the callback handlers.
 *
 * \return Nothing
 */
void I2C_0_send_ack(void)
{
	TWI0.SCTRLB = TWI_ACKACT_ACK_gc | TWI_SCMD_RESPONSE_gc;
}

/**
 * \brief Send NACK to received address or data. Should
 * only be called when appropriate, i.e. in the callback handlers.
 *
 * \return Nothing
 */
void I2C_0_send_nack(void)
{
	TWI0.SCTRLB = TWI_ACKACT_NACK_gc | TWI_SCMD_COMPTRANS_gc;
}

/**
 * \brief Goto unaddressed state. Used to reset I2C HW that are aware
 * of bus state to an unaddressed state.
 *
 * \return Nothing
 */
void I2C_0_goto_unaddressed(void)
{
	// Reset module
	TWI0.SSTATUS |= (TWI_DIF_bm | TWI_APIF_bm);
	TWI0.SCTRLB = TWI_SCMD_COMPTRANS_gc;
}

// Read Event Interrupt Handlers
void I2C_0_read_callback(void)
{
    //dMsg("read");
	//dIMsg((uint8_t)I2C_0_read());
	rdPosCounter++;
	switch (rdPosCounter){
		case 1: I2C_0_write(highByte(thstor1.temp));  break;
		case 2: I2C_0_write(lowByte (thstor1.temp)); break;
		case 3: I2C_0_write(highByte(thstor2.temp));  break;
		case 4: I2C_0_write(lowByte (thstor2.temp)); break;
		case 5: I2C_0_write(highByte(thstor3.temp));  break;
		case 6: I2C_0_write(lowByte (thstor3.temp)); break;
		case 7: I2C_0_write(highByte(thstor4.temp));  break;
		case 8: I2C_0_write(lowByte (thstor4.temp)); break;
		case 9:  I2C_0_write(highByte(curSens1.current));  break;
		case 10: I2C_0_write(lowByte (curSens1.current)); break;
		case 11: I2C_0_write(highByte(curSens2.current));  break;
		case 12: I2C_0_write(lowByte (curSens2.current)); break;
		case 13: I2C_0_write(highByte(curSens3.current));  break;
		case 14: I2C_0_write(lowByte (curSens3.current)); break;
		case 15: I2C_0_write(highByte(curSens4.current));  break;
		case 16: I2C_0_write(lowByte (curSens4.current)); break;
		default: dMsg("error out of bounds I2C_0_read_callback");

	}
     
	if (I2C_0_read_interrupt_handler) {
		I2C_0_read_interrupt_handler();
	}
}

/**
 * \brief Callback handler for event where master wishes to read a byte from slave.
 *
 * \return Nothing
 */
void I2C_0_set_read_callback(I2C_0_callback handler)
{
	I2C_0_read_interrupt_handler = handler;
}

// Write Event Interrupt Handlers
void I2C_0_write_callback(void)
{
    dMsg("write");
    wRead[wPosCounter] = I2C_0_read();
	wPosCounter++;
	dIMsg(wRead[wPosCounter]); 
    I2C_0_send_ack();
	if (I2C_0_write_interrupt_handler) {
		I2C_0_write_interrupt_handler();
	}
}

/**
 * \brief Callback handler for event where master wishes to write a byte to slave.
 *
 * \return Nothing
 */
void I2C_0_set_write_callback(I2C_0_callback handler)
{
	I2C_0_write_interrupt_handler = handler;
}

// Address Event Interrupt Handlers
void I2C_0_address_callback(void)
{
    dMsg("addr");
    I2C_0_write(0x69);
	if (I2C_0_address_interrupt_handler) {
		I2C_0_address_interrupt_handler();
	}
}

/**
 * \brief Callback handler for event where slave has received its address.
 *
 * \return Nothing
 */
void I2C_0_set_address_callback(I2C_0_callback handler)
{
	I2C_0_address_interrupt_handler = handler;
}

// Stop Event Interrupt Handlers
void I2C_0_stop_callback(void)
{
    dMsg("stop");
	if(wPosCounter > 0){
		volatile int port = (int)wRead[0] % 4;
		dMsg("port: "); dIMsg(port);
		if(wRead[0] < 4){
			dMsg("changing Temp:");
			setTemps[port] = (wRead[1] << 8) | wRead[2];
			dIMsg(setTemps[port]);			
		}else if (wRead[0] < 8){
			dMsg("changing portOn:");
			portsOn[port] = (bool) wRead[1];
			dIMsg((int)portsOn[port]);
		}else if(wRead[0] < 12){
			dMsg("changing gain:");
			gains[port] = (wRead[1] << 8) | wRead[2];
			dIMsg(gains[port]);
		}
		
	}
	wPosCounter = 0;
	rdPosCounter = 0;
	if (I2C_0_stop_interrupt_handler) {
		I2C_0_stop_interrupt_handler();
	}
}

/**
 * \brief Callback handler for event where slave has received a STOP condition after being addressed.
 *
 * \return Nothing
 */
void I2C_0_set_stop_callback(I2C_0_callback handler)
{
	I2C_0_stop_interrupt_handler = handler;
}

// Bus Collision Event Interrupt Handlers
void I2C_0_collision_callback(void)
{
    dMsg("col");
	if (I2C_0_collision_interrupt_handler) {
		I2C_0_collision_interrupt_handler();
	}
}

/**
 * \brief Callback handler for event where slave detects a bus collision.
 *
 * \return Nothing
 */
void I2C_0_set_collision_callback(I2C_0_callback handler)
{
	I2C_0_collision_interrupt_handler = handler;
}

// Bus Error Event Interrupt Handlers
void I2C_0_bus_error_callback(void)
{
    dMsg("bus_e");
	if (I2C_0_bus_error_interrupt_handler) {
		I2C_0_bus_error_interrupt_handler();
	}
}

/**
 * \brief Callback handler for event where slave detects a bus error.
 *
 * \return Nothing
 */
void I2C_0_set_bus_error_callback(I2C_0_callback handler)
{
	I2C_0_bus_error_interrupt_handler = handler;
}