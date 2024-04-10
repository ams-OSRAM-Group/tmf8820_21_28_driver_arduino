/*
 *****************************************************************************
 * Copyright by ams OSRAM AG                                                       *
 * All rights are reserved.                                                  *
 *                                                                           *
 * IMPORTANT - PLEASE READ CAREFULLY BEFORE COPYING, INSTALLING OR USING     *
 * THE SOFTWARE.                                                             *
 *                                                                           *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS       *
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT         *
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS         *
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT  *
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,     *
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT          *
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     *
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY     *
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT       *
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE     *
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.      *
 *****************************************************************************
 */

#ifndef TMF8828_SHIM_H
#define TMF8828_SHIM_H

/** @file This is the shim for the arduino uno. 
 * Any define, macro and/or function herein must be adapted to match your
 * target platform
 */

// ---------------------------------------------- includes ----------------------------------------

#include <stdint.h>
#include <avr/pgmspace.h>
#include <Arduino.h>


#if defined( __cplusplus)
extern "C"
{
#endif

// ---------------------------------------------- defines -----------------------------------------

#define ARDUINO_MAX_I2C_TRANSFER                  32    /**< Arduino Uno can only handle up to 32 bytes in a single i2c tx/rx */

#define ENABLE_PIN                                6     /**< on the arduino uno the enable pin is connected to digital 6 */
#define INTERRUPT_PIN                             7     /**< interupt to digital 7 */

// for 2nd tmf8828 on the arduino uno the alternate enable pin is connected to digital 4, alternate interrupt to digital 5
#define ALT_ENABLE_PIN                            4
#define ALT_INTERRUPT_PIN                         5

// for 3rd tmf8828 on the arduino uno the alternate enable pin is connected to digital 
#define ALT2_ENABLE_PIN                           2
#define ALT2_INTERRUPT_PIN                        3

// if only a single TMF882x is used we can also used interrupt pin
#define USE_INTERRUPT_TO_TRIGGER_READ             0     /**< do not define this, or set to 0 to use i2c polling instead of interrupt pin */
#define TRIGGER_INTERRUPT_PIN                     2     /**< the arduino uno can only handle interrupts on pin 2, 3 so re-route your pin 7 also to pin2 if you want to use interrupt */

// for clock correction insert here the number in relation to your host
#define HOST_TICKS_PER_US                     1         // host counts ticks every microsecond
#define TMF8828_TICKS_PER_US                  5         // tmf8828 counts ticks 0.2 mircoseconds (5x faster than host)               


// ---------------------------------------------- macros ------------------------------------------
/** @brief macros to cast a pointer to an address - adapt for your machine-word size
 */ 
#define PTR_TO_UINT(ptr)                     ( (intptr_t)(ptr) )

/** @brief macros to replace the platform specific printing
 */ 
#define PRINT_CHAR(c)                         printChar( c )
#define PRINT_INT(i)                          printInt( i )
#define PRINT_UINT(i)                         printUint( i )
#define PRINT_UINT_HEX(i)                     printUintHex( i )
#define PRINT_STR(str)                        printStr( str )  
#define PRINT_CONST_STR(str)                  printConstStr( (const char *)str )  
#define PRINT_LN()                            printLn( )

/** Which character to use to seperate the entries in printing */
#define SEPARATOR                             ','

// ---------------------------------------------- forward declarations ----------------------------

/** forward declaration of driver structure to avoid cyclic dependancies */
typedef struct _tmf8828Driver tmf8828Driver;


// ---------------------------------------------- functions ---------------------------------------

/** @brief Function to allow to wait for some time in microseconds
 *  @param[in] wait number of microseconds to wait before this function returns
 */
void delayInMicroseconds( uint32_t wait );

/** @brief Function returns the current sys-tick.
 * \return current system tick (granularity is host specific - see macro HOST_TICKS_PER_10_US) 
 */
uint32_t getSysTick( );

/** @brief Function reads a single byte from the given address. This is only needed on 
 * systems that have special memory access methods for constant segments. Like e.g. Arduino Uno
 *  @param[in] ptr to memory to read from
 * \return single byte from the given address 
 */
uint8_t readProgramMemoryByte( const uint8_t * ptr );

/** @brief Function sets the enable pin HIGH. Note that the enable pin must be configured
 * for output (with e.g. function pinOutput)
 * @param[in] dptr ... a pointer to a data structure the function needs for setting the enable pin, can
 * be 0-pointer if the function does not need it
 */
void enablePinHigh( void * dptr );

/** @brief Function sets the enable pin LOW. Note that the enable pin must be configured
 * for output (with e.g. function pinOutput)
 * @param[in] dptr ... a pointer to a data structure the function needs for setting the enable pin, can
 * be 0-pointer if the function does not need it
 */
void enablePinLow( void * dptr );

/** @brief Function configures enable and interrupt pins for I/O.
 * @param[in] dptr ... a pointer to a data structure the function needs for configuring the pins, can
 * be 0-pointer if the function does not need it
 */
void configurePins( void * dptr );

/** @brief Function will open the I2C master and configure for the given speed (if possible),
 * else it will reduce the speed to the available frequency
 * @param[in] dptr a pointer to a data structure the function may need, can
 * be 0-pointer if the function does not need it
 * @param[in] i2cClockSpeedInHz ... desired i2c clock speed in hertz
 */
void i2cOpen( void * dptr, uint32_t i2cClockSpeedInHz );

/** @brief Function closes the i2c master 
 * @param[in] dptr a pointer to a data structure the function may need, can
 * be 0-pointer if the function does not need it
 */
void i2cClose( void * dptr );

/** @brief Function outputs a single character. E.g. on a UART.
 *  @param[in] c the character to be printed 
 */
void printChar( char c );

/** @brief Function outputs a signed integer. E.g. on a UART.
 *  @param[in] i the integer to be printed 
 */
void printInt( int32_t i );

/** @brief Function outputs an unsigned integer. E.g. on a UART.
 *  @param[in] i the integer to be printed 
 */
void printUint( uint32_t i );

/** @brief Function outputs an unsigned integer in HEX format. E.g. on a UART.
 *  @param[in] i the integer to be printed 
 */
void printUintHex( uint32_t i );

/** @brief Function outputs a zero terminated string. E.g. on a UART.
 *  @param[in] str pointer to string to be printed  
 */
void printStr( char * str );

/** @brief Function outputs a new-line. E.g. on a UART.
 */
void printLn( void );


// ---------------------------------- I2C functions ---------------------------------------------

/**  Return codes for i2c functions: 
 */
#define I2C_SUCCESS             0       /**< successfull execution no error */
#define I2C_ERR_DATA_TOO_LONG   -1      /**< driver cannot handle given amount of data for tx/rx */
#define I2C_ERR_SLAVE_ADDR_NAK  -2      /**< device nak'ed slave address */
#define I2C_ERR_DATA_NAK        -3      /**< device nak'ed written data */
#define I2C_ERR_OTHER           -4      /**< any other error */
#define I2C_ERR_TIMEOUT         -5      /**< timeout in waiting for slave to respond */

/** There are 2 styles of functions available:
 * 1. those that always require a register address to be specified: i2cTxReg, i2cRxReg
 * 2. the more generic (more I2C standard like) that can transmit and/or receive (here the
 *  register address if needed is the first transmitted byte): i2cTxRx
 * Only one set of those two *have to be* available. Both can be available.
 */

/** @brief I2C transmit only function.
 * @param[in] dptr a pointer to a data structure the function needs for transmitting, can
 * be 0-pointer if the function does not need it
 * @param[in] slaveAddr the i2c slave address to be used (7-bit unshifted)
 * @param[in] regAddr the register to start writing to
 * @param[in] toTx number of bytes in the buffer to transmit
 * @param[in] txData pointer to the buffer to transmit
 * \return 0 when successfully transmitted, else an error code
 */ 
int8_t i2cTxReg( void * dptr, uint8_t slaveAddr, uint8_t regAddr, uint16_t toTx, const uint8_t * txData );

/** @brief I2C transmit register address and receive function.
 * @param[in] dptr a pointer to a data structure the function needs for receiving, can
 * be 0-pointer if the function does not need it
 * @param slaveAddr the i2c slave address to be used (7-bit)
 * @param regAddr the register address to start reading from
 * @param toRx number of bytes in the buffer to receive
 * @param rxData pointer to the buffer to be filled with received bytes
 * \return 0 when successfully received, else an error code
 */ 
int8_t i2cRxReg( void * dptr, uint8_t slaveAddr, uint8_t regAddr, uint16_t toRx, uint8_t * rxData );

/** @brief I2C transmit and receive function.
 * @param dptr a pointer to a data structure the function needs for transmitting, can
 * be 0-pointer if the function does not need it
 * @param slaveAddr the i2c slave address to be used (7-bit)
 * @param toTx number of bytes in the buffer to transmit (set to 0 if receive only)
 * @param txData pointer to the buffer to transmit
 * @param toRx number of bytes in the buffer to receive (set to 0 if transmit only)
 * @param rxData pointer to the buffer to be filled with received bytes
 * \return 0 when successfully transmitted and received, else an error code
 */ 
int8_t i2cTxRx( void * dptr, uint8_t slaveAddr, uint16_t toTx, const uint8_t * txData, uint16_t toRx, uint8_t * rxData );



/* --------------------- functions used by the application only (not driver) -------------------------------- */
/* I.e. you must only implement these functions if you want to use the application. if you only use the 
 * c driver code you need not implement these functions. 
 */

/** @brief Function will open the serial input and clear the input pipe
 * @param[in] baudrate serial rate in baud
 */
void inputOpen( uint32_t baudrate );

/** @brief Function will close the serial input
 */
void inputClose( );

/** @brief Function reads the next character from standard input.
 * @param[out] c pointee is set to the character read from standard input; must not be a null pointer
 * \return 1 when a character was read and written to the pointee of c, else 0 */
int8_t inputGetKey( char * c );

/** @brief Function outputs a zero terminated constant string. E.g. on a UART.
 *  @param[in] str pointer to constant string to be printed. On some systems constants can
 * be stored in special memory and require special access for reading.   
 */
void printConstStr( const char * str );

/** @brief Function sets the given pin to output.
 *  @param[in] pin to be configured as output pin.   
 */
void pinOutput( uint8_t pin );

/** @brief Function sets the given pin to input.
 *  @param[in] pin to be configured as input pin.   
 */
void pinInput( uint8_t pin );

/** @brief Function registers given function handler as Interrupt Handler function for the
 *  interrupt pin
 *  @param[in] handler pointer to the interrupt service routine   
 */
void setInterruptHandler( void (* handler)( void ) );

/** @brief Function removes any Interrupt Handler function
 */
void clrInterruptHandler( void );

/** @brief Function globally disables interrupts
 */
void disableInterrupts( void );

/** @brief Function globally enables interrupts
 */
void enableInterrupts( void );

/** @brief Function to print the results in a kind of CSV like format
 * @param[in] dptr a pointer to a data structure the function may need, can
 * be 0-pointer if the function does not need it
 *  @param[in] data ... pointer to the result structure as defined for tmf882x
 *  @param[in] len ... number of bytes the data pointer points to
 */
void printResults( void * dptr, uint8_t * data, uint8_t len );


/** @brief Function to print a histogram part in a kind of CSV like format 
 * @param[in] dptr a pointer to a data structure the function may need, can
 * be 0-pointer if the function does not need it
 *  @param[in] data ... pointer to the result structure as defined for tmf882x
 *  @param[in] len ... number of bytes the data pointer points to
 */
void printHistogram( void * dptr, uint8_t * data, uint8_t len );


#if defined( __cplusplus)
}
#endif

#endif
