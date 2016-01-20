#include "MKL46Z4.h"
#include "board.h"

/**
 * @brief         Init_Led
 * @param[in]     void
 * @param[in,out] void
 * @return        void
 */
void Init_Led (void)
{
    /* Enable clock for PORTE & PORTD*/
    SIM->SCGC5 |= ( SIM_SCGC5_PORTD_MASK
               | SIM_SCGC5_PORTE_MASK );
    /* 
    * Initialize the RED LED (PTE5)
    */
    PORTE->PCR[29] = PORT_PCR_MUX(1);

    /* Set the pins direction to output */
    FPTE->PDDR |= RED_LED_PIN;

    /* Set the initial output state to high */
    FPTE->PSOR |= RED_LED_PIN;

    /* 
    * Initialize the Green LED (PTE5)
    */

    /* Set the PTE29 pin multiplexer to GPIO mode */
    PORTD->PCR[5]= PORT_PCR_MUX(1);

    /* Set the pins direction to output */
    FPTD->PDDR |= GREEN_LED_PIN;

    /* Set the initial output state to high */
    FPTD->PSOR |= GREEN_LED_PIN;
}
