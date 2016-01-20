#include "i2c.h"

void i2c_delay(void)
{
    uint32_t i;
    for(i = 0; i < 100; i++) {  }
}


void i2c_start(I2C_Type *p)
{
    i2c_set_master(p);
    i2c_set_tx(p);
}

void i2c_stop(I2C_Type *p)
{
    i2c_set_slave(p);
    i2c_set_rx(p);
}

void i2c_wait(I2C_Type *p)
{
    // Spin wait for the interrupt flag to be set
    while((p->S & I2C_S_IICIF_MASK) == 0);
    p->S |= I2C_S_IICIF_MASK; // Clear flag
}

int i2c_write(I2C_Type *p, uint8_t data)
{
    // Send data, wait, and return ACK status
    p->D = data;
    i2c_wait(p);
    return ((p->S & I2C_S_RXAK_MASK) == 0);
}

void i2c_init(I2C_Type *p)
{
    // Enable clocks    
    SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;
    SIM->SCGC4 |= SIM_SCGC4_I2C0_MASK;  	

    // Configure GPIO for I2C    
    PORTE->PCR[24] = PORT_PCR_MUX(5); 
    PORTE->PCR[25] = PORT_PCR_MUX(5);

    p->F  = 0x14;              // Baudrate settings:  ICR=0x14, MULT=0
    p->C1 = I2C_C1_IICEN_MASK; // Enable:  IICEN=1
}

