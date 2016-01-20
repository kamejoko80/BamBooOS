#include  <stdio.h>
#include  "MKL46Z4.h"
#include  "uart.h"

static uint8_t       g_len = 0;
static uint8_t       g_rx_count = 0;
static uint8_t      *g_buffer = 0;
static CallBack_Type g_callback = 0;

/**
 * @brief file handle structure
 */
struct __FILE 
{ 
	int handle;
};

FILE __stdout;
FILE __stdin;

/**
 * @brief         UART0_PinConf
 * @param[in]     void
 * @param[in,out] void
 * @return        void
 */
void UART0_PinConf(void)
{
    /* PTA1 : UART0_RX */
    PORTA->PCR[1] = 0x00000200UL;
    /* PTA2 : UART0_TX */
    PORTA->PCR[2] = 0x00000200UL;
}

/**
 * @brief         UART0_Init
 * @param[in]     void
 * @param[in,out] void
 * @return        void
 */
void UART0_Init(void)
{
    char dummy;
    
    /* Config pin for UART0 */
    UART0_PinConf();
    
    /* Enable clock gate UART0 */
    SIM->SCGC4 |=   1UL << 10; 
    
    /* Choose MCGPLLCLK/2 (48MHz/2) */
    SIM->SOPT2 |=   1UL << 16;     
    SIM->SOPT2 &= ~(1UL << 27);
    SIM->SOPT2 |=   1UL << 26;

    /* Baudrate = clock/(SBR[12:0] x (OSR + 1))   */
    /* OSR = 15, clock = 24MHz, baudrate = 115200 */
    /* -> SBR = 13 */
    /* LBKDIE = 0, RXEDGIE = 0, SBNS = 0 */
    UART0->BDH = 0x00;
    UART0->BDL = 0x0D;

    /* UART0 control */
    UART0->C1 = 0x00;
    UART0->C2 = 0x00;
    UART0->C3 = 0x00;
    UART0->C4 = 0x0F; /* OSR = 15 */
    UART0->C5 = 0x00;
    
    /* Enable UART0 transmit/receive */
    UART0->C2 |= 0x03 << 2;
    
    /* Dummy read to ensure UART0 
    receive buffer is empty already */
    dummy = UART0->D;
    dummy = dummy; /* Make compiler happy */
}

/**
 * @brief         UART0_PutChar
 * @param[in]     void
 * @param[in,out] void
 * @return        void
 */
void UART0_PutChar(char c)
{
    UART0->D   = c;
    /* Wait for transmition complete */
    while(!(UART0->S1 & (0x01 << 6))) {}
}

/**
 * @brief         UART0_GetChar
 * @param[in]     void
 * @param[in,out] void
 * @return        char
 */
char UART0_GetChar(void)
{
    /* Wait for incomming character */
    while(!(UART0->S1 & (0x01 << 5))) {}
    return UART0->D; 
}

/**
 * @brief         fputc
 * @param[in]     void
 * @param[in,out] void
 * @return        char
 */
int fputc(int ch, FILE *f) 
{ 
  UART0_PutChar((char)ch);
  return((int)ch);
}

/**
 * @brief         fgetc
 * @param[in]     void
 * @param[in,out] void
 * @return        char
 */
int fgetc(FILE *f) {
    
    char c;
    /* get key */
    c =  UART0_GetChar();
    /* print echo */
    UART0_PutChar(c);
    return (int)c;
}

/**
 * @brief         UART0_AsyncReceive
 * @param[in]     CallBack_Type CallBack
 * @param[in,out] uint8_t * buffer
 * @return        void
 */
void UART0_AsyncReceive(uint8_t len, uint8_t * buffer, CallBack_Type CallBack)
{
    /* Disable IRQ */
    NVIC_DisableIRQ(UART0_IRQn);
    
    /* Initialize global variables */
    if ((g_buffer == NULL) && (len > 0))
    {
        g_buffer = buffer;
        g_len = len;
        g_rx_count = 0;
        g_callback = CallBack;
  }

    /* Receive Complete Interrupt Enable */
    UART0->C2 |= UART0_C2_RIE_MASK;
    NVIC_SetPriority(UART0_IRQn,1);
    NVIC_EnableIRQ(UART0_IRQn);
}

/**
 * @brief         UART0 interrupt handler
 * @param[in]     void
 * @param[in,out] void
 * @return        void
 */

void UART0_IRQHandler(void)
{
    char val;

    /* Check RX IRQ or not */  	
    if(!(UART0->S1 & (0x01 << 5)))
    {
        return;
    }

    /* read the RX register */
    val = (char)UART0->D;
    
    /* Echo received byte */
    UART0_PutChar(val);
    
    if ('\r' == val)
    {
        UART0_PutChar('\n');
    }

    if ((g_len > 0) && g_buffer && (g_rx_count < g_len))
    {
        if (('\n' != val) && ('\r' != val))
        {
            g_buffer[g_rx_count] = val;
            g_rx_count++;
        }
        
        if (('\r' == val) || ('\n' == val) || (g_rx_count >= g_len))
        {
            if (g_callback != NULL)
            {
                g_callback(g_buffer, g_rx_count);
            }
            
            /* Reset global variables */
            g_buffer = 0;
            g_rx_count = 0;
            g_len = 0;

            /* RIE = 0, disable IRQ */ 
            UART0->C2 &= ~UART0_C2_RIE_MASK;         
        }
    }  	
}
