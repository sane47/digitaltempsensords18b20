
#include <stddef.h>                     // Defines NULL
#include <stdbool.h>                    // Defines true
#include <stdlib.h>                     // Defines EXIT_FAILURE
#include "definitions.h"                // SYS function prototypes
#include <string.h>
#include <stdio.h>
// *****************************************************************************
// *****************************************************************************
// Section: Main Entry Point
// *****************************************************************************
// *****************************************************************************

/* Thermometer Connections (At your choice) */

#define THERM_CMD_CONVERTTEMP 0x44
#define THERM_CMD_RSCRATCHPAD 0xbe
#define THERM_CMD_WSCRATCHPAD 0x4e
#define THERM_CMD_CPYSCRATCHPAD 0x48
#define THERM_CMD_RECEEPROM 0xb8
#define THERM_CMD_RPWRSUPPLY 0xb4
#define THERM_CMD_SEARCHROM 0xf0
#define THERM_CMD_READROM 0x33
#define THERM_CMD_MATCHROM 0x55
#define THERM_CMD_SKIPROM 0xcc
#define THERM_CMD_ALARMSEARCH 0xec


#define THERM_DECIMAL_STEPS_12BIT 625 //.0625
#define THERM_DECIMAL_STEPS_11BIT 1250 
#define THERM_DECIMAL_STEPS_10BIT 2500
#define THERM_DECIMAL_STEPS_9BIT  5000 


void delayUS(uint32_t count)
{
    
   for(uint32_t i = 0; i < 10*count; i++) {
       
     __NOP();

      }
}

_Bool therm_reset() {
    
GPIO_PA07_Clear();

// configure DS18B20_PIN pin as input  
GPIO_PA07_OutputEnable();

  delayUS(480);      // wait 500 us
  
// configure DS18B20_PIN pin as input    
GPIO_PA07_InputEnable();
  
// wait 100 us to read the DS18B20 sensor response
delayUS(60);
  if (!GPIO_PA07_Get())
  {
    delayUS(420);       // wait 400 us
    return 1;           // DS18B20 sensor is present
 
  }
  else{
  return 0;   // connection error
  }
      
}

void therm_write_bit(uint8_t bit) {
   
 GPIO_PA07_Clear();      // send reset pulse to the DS18B20 sensor
  
 // configure DS18B20_PIN pin as output
 GPIO_PA07_OutputEnable();
 
delayUS(1);        // wait 2 us
    
  if (bit > 0){
      
  GPIO_PA07_Set();
  
  } else {
      
  GPIO_PA07_Clear();
  
  }
  
  delayUS(60);       // wait 80 us

  // configure DS18B20_PIN pin as input  
  GPIO_PA07_InputEnable();
   
 delayUS(2);        // wait 2 us   
       
}

_Bool therm_read_bit(void) {
   
 _Bool value = 0;

GPIO_PA07_Clear();
     
// configure DS18B20_PIN pin as output
GPIO_PA07_OutputEnable();

delayUS(1);

// configure DS18B20_PIN pin as input  

GPIO_PA07_InputEnable();

  delayUS(14);        // wait 5 us

  value = GPIO_PA07_Get();   // read and store DS18B20 state
  delayUS(45);               // wait 100 us

  return value;
       
}

uint8_t therm_read_byte(void) {
    uint8_t i = 8, n = 0;
    while (i--) {

        //Shift one position right and store read value
        n >>= 1;
        n |= (therm_read_bit() << 7);
    }
    return n;
    
}

void therm_write_byte(uint8_t byte) {
    uint8_t i = 8;
    while (i--) {
        //Write actual bit and shift one position right to make the next bit ready

        therm_write_bit(byte & 1);
        byte >>= 1;
    }
}


int main ( void )
{
    uint8_t temperature[2];
    char buffer[50];
    int digit;
    int digitf;
//    uint16_t decimal;
    /* Initialize all modules */
    SYS_Initialize ( NULL );

    while ( true )
    {
        
    /* Maintain state machines of all polled MPLAB Harmony modules. */
    SYS_Tasks ( );
        
    //Read Scratchpad (only 2 first bytes)
    therm_reset();
    therm_write_byte(THERM_CMD_SKIPROM);
    therm_write_byte(THERM_CMD_CONVERTTEMP);
    //Wait until conversion is complete
    while (!therm_read_bit());
    
    //Reset, skip ROM and send command to read Scratchpad
    therm_reset();
    therm_write_byte(THERM_CMD_SKIPROM);
    therm_write_byte(THERM_CMD_RSCRATCHPAD);
    
    temperature[0] = therm_read_byte();
    temperature[1] = therm_read_byte();
    therm_reset();
    //Store temperature integer digits and decimal digits
    digit=temperature[0]>>4;
    digit|=(temperature[1]&0x7)<<4;
    
    digitf = (int)(1.8*digit)+32;
    
    
    sprintf(buffer, "%d F \t %d C \r\n", digitf,digit);
    SERCOM3_USART_Write(buffer, strlen(buffer));  
        
    delayUS(1000000);
                
    }

    /* Execution should not come here during normal operation */

    return ( EXIT_FAILURE );
}


/*******************************************************************************
 End of File
*/

