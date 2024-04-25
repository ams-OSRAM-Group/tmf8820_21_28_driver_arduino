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

#ifndef TMF8828_H
#define TMF8828_H

// ---------------------------------------------- includes ----------------------------------------

#include "tmf8828_shim.h"

#ifdef __cplusplus
extern "C" {
#endif  


// Version
// 1.7 ... tmf8828 only, add presistence and low/high thresholds
// 1.8 ... tmf8828 and tmf882x combined in one driver application
// 1.9 ... fixed bug in waiting time for page loading (too short)
// 1.10 .. only changed application, nothing in driver
// 1.11 .. plain C driver, no more c++
// 1.12 .. clock correction more robust for very long intervals
// 1.13 .. internal test version
// 1.14 .. fix serial number readout

#define TMF8828_DRIVER_MAJOR_VERSION  1
#define TMF8828_DRIVER_MINOR_VERSION  14


// ---------------------------------------------- defines -----------------------------------------

#define TMF8828_NUMBER_RESULT_RECORDS   4                 // TMF8828 has 4 result records to report all 64 results

// print only the first xx results, it this is more than total exist, it will be shortend to (36== maximum)
#define PRINT_NUMBER_RESULTS        36         

// tmf8828 has as default i2c slave address
#define TMF8828_SLAVE_ADDR          0x41

// important wait timings
#define CAP_DISCHARGE_TIME_MS       3                     // wait time until we are sure the PCB's CAP has dischared properly
#define ENABLE_TIME_MS              1                     // wait time after enable pin is high
#define CPU_READY_TIME_MS           1                     // wait time for CPU ready 

// return codes from the bootloader
#define BL_SUCCESS_OK               0                     // success
#define BL_ERROR_CMD                -1                    // command/communication failed
#define BL_ERROR_TIMEOUT            -2                    // communication timeout

// return codes from the measurement application
#define APP_SUCCESS_OK              (BL_SUCCESS_OK)       // success
#define APP_ERROR_CMD               (BL_ERROR_CMD)        // command/communication failed
#define APP_ERROR_TIMEOUT           (BL_ERROR_TIMEOUT)    // timeout
#define APP_ERROR_PARAM             -3                    // invalid parameter (e.g. spad map id wrong)
#define APP_ERROR_NO_RESULT_PAGE    -4                    // did not receive a measurement result page
#define APP_ERROR_NO_CALIB_PAGE     -5                    // this is no factory calibration page

// Interrupt bits
#define TMF8828_APP_I2C_ANY_IRQ_MASK                        0x01        /*!< any of below interrupts has occured */
#define TMF8828_APP_I2C_RESULT_IRQ_MASK                     0x02        /*!< a measurement result is ready for readout */
#define TMF8828_APP_I2C_ALT_RESULT_IRQ_MASK                 0x04        /*!< used for statistics and electrical calibration results */
#define TMF8828_APP_I2C_RAW_HISTOGRAM_IRQ_MASK              0x08        /*!< a raw histogram is ready for readout */
#define TMF8828_APP_I2C_BREAKPOINT_IRQ_MASK                 0x10        /*!< a breakpoint has been hit */
#define TMF8828_APP_I2C_CMD_DONE_IRQ_MASK                   0x20        /*!< a received I2C command has been handled (successfully or failed) */
#define TMF8828_APP_I2C_ERROR_IRQ_MASK                      0x40        /*!< one of the <status> registers has been set to a non-zero value */

// result page addresses and defines 
#define TMF8828_COM_CONFIG_RESULT                           0x20  // config/result register address
#define TMF8828_COM_CONFIG_RESULT__measurement_result       0x10  // page contains measurment result
#define TMF8828_COM_CONFIG_RESULT__measurement_result_size  (0xa4-0x20)

// some more info registers from the results page
#define TMF8828_COM_RESULT_NUMBER         0x24
#define TMF8828_COM_TEMPERATURE           0x25
#define TMF8828_COM_NUMBER_VALID_RESULTS  0x26
#define TMF8828_COM_SYS_TICK_0            0x34      // sys tick is 4 bytes

// each of the result records consist of 3 bytes
#define TMF8828_COM_RES_CONFIDENCE_0      0x38
#define TMF8828_COM_RES_DISTANCE_0_LSB    0x39
#define TMF8828_COM_RES_DISTANCE_0_MSB    0x3a

// Use this macro like this: data[ RESULT_REG( RESULT_NUMBER ) ], it calculates the offset into the data buffer
#define RESULT_REG( regName )             ( (TMF8828_COM_##regName) - (TMF8828_COM_CONFIG_RESULT) )

// available SPAD map IDs known by this driver version
#define TMF8828_COM_SPAD_MAP_ID__spad_map_id__map_no_1 1 // 3x3 map, size 14x6    1. Normal Mode (29°x29°)        
#define TMF8828_COM_SPAD_MAP_ID__spad_map_id__map_no_2 2 // 3x3 map, size 14x9    2. Macro Mode (29°x43,5°)       
#define TMF8828_COM_SPAD_MAP_ID__spad_map_id__map_no_3 3 // 3x3 map, size 14x9    3. Macro Mode (29°x43,5°)       
#define TMF8828_COM_SPAD_MAP_ID__spad_map_id__map_no_4 4 // 4x4 map, size 14x9    4. Time-multiplexed, Normal/Macro Mode (29°x43,5°)  
#define TMF8828_COM_SPAD_MAP_ID__spad_map_id__map_no_5 5 // 4x4 map, size 14x9    5. Time-multiplexed, Normal/Macro Mode (29°x43,5°)  
#define TMF8828_COM_SPAD_MAP_ID__spad_map_id__map_no_6 6 // 3x3 map, size 18x10   6. Normal Mode (44°x48°)      
#define TMF8828_COM_SPAD_MAP_ID__spad_map_id__map_no_7 7 // 4x4 map, size 18x10   7. Time-multiplexed, Normal Mode (44°x48°)      
#define TMF8828_COM_SPAD_MAP_ID__spad_map_id__map_no_8 8 // 9 zones map, size 14x9  8. Normal/Macro Mode (29°x43,5°)  
#define TMF8828_COM_SPAD_MAP_ID__spad_map_id__map_no_9 9 // 9 zones map, size 14x9  9. Normal/Macro Mode (29°x43,5°)  
#define TMF8828_COM_SPAD_MAP_ID__spad_map_id__map_no_10 10 // 3x6 map, size 18x12       10. Time-multiplexed, (29°x57°)   
#define TMF8828_COM_SPAD_MAP_ID__spad_map_id__map_no_11 11 // 3x3 map, size 14x6        11. Checkerboard, Normal Mode(29°x29°)        
#define TMF8828_COM_SPAD_MAP_ID__spad_map_id__map_no_12 12 // 3x3 map, size 14x6        12. Reverse-Checkerboard, Normal Mode(29°x29°)        
#define TMF8828_COM_SPAD_MAP_ID__spad_map_id__map_no_13 13 // 4x4 map, size 18x8        13. Time-multiplexed, Narrow Mode (29°x39°)         
#define TMF8828_COM_SPAD_MAP_ID__spad_map_id__map_no_15 15 // 8x8 mode         

// Factory calibration page size - store the complete page in e.g. a file, and reload the complete page
#define TMF8828_COM_CONFIG_FACTORY_CALIB__factory_calibration_size    ((0xE0)-(TMF8828_COM_CONFIG_RESULT)) 

// Histogram dumping requires sub-packets
// Register offset of sub-packets
#define TMF8828_COM_SUBPACKET_NUMBER                          0x24      // sub-packet number
#define TMF8828_COM_SUBPACKET_PAYLOAD                         0x25      // sub-packet payload
#define TMF8828_COM_SUBPACKET_CFG_IDX                         0x26      // sub-packet config index (0,1)
#define TMF8828_COM_SUBPACKET_PAYLOAD_0                       0x27
#define TMF8828_COM_HISTOGRAM_PACKET_SIZE                     (4+3+128) // 4 bytes packet header, 3 bytes sub-packet header, 128 paylaod
#define TMF8828_COM_OPTIONAL_SUBPACKET_HEADER_MASK            (0x80)                          /*!< this is the bit that has to be set to indicated in the RID that there is a sub-packet header */
#define TMF8828_COM_HIST_DUMP__histogram__raw_24_bit_histogram 1 // Raw 24 bit histogram             
#define TMF8828_COM_HIST_DUMP__histogram__electrical_calibration_24_bit_histogram 2 // Electrical calibration 24 bit histogram  
#define TMF8828_NUMBER_OF_BINS_PER_CHANNEL                    128       // how many bins are in a raw histogram per channel


// Clock correction pairs must be a power of 2 value.
#define CLK_CORRECTION_PAIRS                4   // how many clock correction pairs are stored

// ---------------------------------------------- logging -----------------------------------------

#define TMF8828_LOG_LEVEL_NONE              0
#define TMF8828_LOG_LEVEL_ERROR             1           // only error logging - recommended
#define TMF8828_LOG_LEVEL_CLK_CORRECTION    8           // this is a bit-mask check for clock correction logging
#define TMF8828_LOG_LEVEL_INFO              0x10        // some information
#define TMF8828_LOG_LEVEL_VERBOSE           0x20        // very chatty firmware
#define TMF8828_LOG_LEVEL_I2C               0x80        // this is a bit-mask check for i2c logging
#define TMF8828_LOG_LEVEL_DEBUG             0xFF        // everything

// ---------------------------------------------- types -------------------------------------------
/** @brief driver info: compile time info of this driver
 */ 
typedef struct _tmf8828DriverInfo
{
  uint8_t version[2];                               /**< this driver version number major.minor*/ 
} tmf8828DriverInfo;

/** @brief device information: read from the device
 */
typedef struct _tmf8828DeviceInfo
{
  uint32_t deviceSerialNumber;                      /**< serial number of device, if 0 not read */
  uint8_t appVersion[4];                            /**< application version number (app id, major, minor, patch) */
  uint8_t chipVersion[2];                           /**< chip version (Id, revId) */
} tmf8828DeviceInfo;

// Each tmf8828 driver instance needs a data structure like this
typedef struct _tmf8828Driver
{
  tmf8828DeviceInfo device;                         /**< information record of device */
  tmf8828DriverInfo info;                           /**< information record of driver */  
  uint32_t hostTicks[ CLK_CORRECTION_PAIRS ];       // host ticks for clock correction
  uint32_t tmf8828Ticks[ CLK_CORRECTION_PAIRS ];    // device ticks for clock correction
  uint16_t clkCorrRatioUQ;                          // clock ratio in UQ1.15 [0..2)
  uint8_t clkCorrectionIdx;                         // index of the last inserted pair
  uint8_t i2cSlaveAddress;                          // i2c slave address to talk to device
  uint8_t clkCorrectionEnable;                      // default is clock correction on 
  uint8_t logLevel;                                 // how chatty the program is
} tmf8828Driver;

// ---------------------------------------------- functions ---------------------------------------
// Power and bootloader functions are available with ROM code.
// ---------------------------------------------- functions ---------------------------------------

// Function to initialise the driver data structure, call this as the first function
// of your program, before using any other function of this driver
// driver ... pointer to an instance of the tmf8828 driver data structure
void tmf8828Initialise( tmf8828Driver * driver );

// Function to configure the driver how chatty it should be. This is only the driver itself.
// driver ... pointer to an instance of the tmf8828 driver data structure
// level ... the log level for printing
void tmf8828SetLogLevel( tmf8828Driver * driver, uint8_t level );

// Function to set clock correction on or off.
// driver ... pointer to an instance of the tmf8828 driver data structure
// enable ... if <>0 clock correction is enabled (default)
// enable ... if ==0 clock correction is disabled
void tmf8828ClkCorrection( tmf8828Driver * driver, uint8_t enable );

// Function to reset the HW and SW on the device
// driver ... pointer to an instance of the tmf8828 driver data structure
void tmf8828Reset( tmf8828Driver * driver );

// Function to set the enable pin high
// driver ... pointer to an instance of the tmf8828 driver data structure
void tmf8828Enable( tmf8828Driver * driver );

// Function to set the enable pin low
// driver ... pointer to an instance of the tmf8828 driver data structure
void tmf8828Disable( tmf8828Driver * driver );

// Function to put the device in standby mode
// driver ... pointer to an instance of the tmf8828 driver data structure
void tmf8828Standby( tmf8828Driver * driver );

// Function to wake the device up from standby mode
// driver ... pointer to an instance of the tmf8828 driver data structure
void tmf8828Wakeup( tmf8828Driver * driver );

// Function returns true if CPU is ready, else false. If CPU is not ready, device cannot be used.
// driver ... pointer to an instance of the tmf8828 driver data structure
// Returns !=0 if CPU is ready (device can be used), else it returns ==0 (device cannot be accessed).
int8_t tmf8828IsCpuReady( tmf8828Driver * driver, uint8_t waitInMs );

// Function to download the firmware image that was linked against the firmware (tmf8828_image.{h,c} files)
// driver ... pointer to an instance of the tmf8828 driver data structure
// imageStartAddress ... destination in RAM to which the image shall be downloaded
// image ... pointer to the character array that represents the downloadable image
// imageSizeInBytes ... the number of bytes to be downloaded
// Function returns BL_SUCCESS_OK if successfully downloaded the FW, else it returns an error BL_ERROR_*
int8_t tmf8828DownloadFirmware( tmf8828Driver * driver, uint32_t imageStartAddress, const uint8_t * image, int32_t imageSizeInBytes );


// ------------------------------- application functions --------------------------------------------
// Application functions are only available after a successfull firmware download.
// ------------------------------- application functions --------------------------------------------

/** @brief Function reads complete device information
 * @param driver ... pointer to an instance of the tmf8828 driver data structure
 * @return Function returns APP_SUCCESS_OK if successfully read the complete device information, else 
 * it returns an error APP_ERROR_* 
 */ 
int8_t tmf8828ReadDeviceInfo( tmf8828Driver * driver );

// Convert 4 bytes in little endian format into an uint32_t  
uint32_t tmf8828GetUint32( uint8_t * data );
 
// Function writes the current configuration page.
// driver ... pointer to an instance of the tmf8828 driver data structure
// Function returns APP_SUCCESS_OK if successfully written, else it returns an error APP_ERROR_*
int8_t tmf8828WriteConfigPage( tmf8828Driver * driver );

// Function to load the common config page into I2C ram that the host can read/write it via I2C
// driver ... pointer to an instance of the tmf8828 driver data structure
// Function returns APP_SUCCESS_OK if successfully loaded, else it returns an error APP_ERROR_*
int8_t tmf8828LoadConfigPageCommon( tmf8828Driver * driver );

// Function to load the factory calibration config page into I2C ram that the host can read/write it via I2C
// driver ... pointer to an instance of the tmf8828 driver data structure
// Function returns APP_SUCCESS_OK if successfully loaded, else it returns an error APP_ERROR_*
int8_t tmf8828LoadConfigPageFactoryCalib( tmf8828Driver * driver );

// Function to configure the tmf8828 - convenience function that show how to use the tmf8828LoadConfigPage* 
// and tmf8828WriteConfigPage functions.
// driver ... pointer to an instance of the tmf8828 driver data structure
// periodInMs ... measurement repetion period (may be higher if kilo-iterations require a longer integration time)
// kiloIterations .. the number of 1024 iterations 
// spadMapId ... the SPAD map to be selected (2,3,6), must be smaller than 16
// lowThreshold ... the low threshold for interrupt generation in mm
// highThreshold ... the high threshold for interrupt generation in mm            
// persistence ... the consecuive times an object must be between lower threshold and high threshold to be reported  
// intMask ... an 18-bit mask where a 1 means that this zone can report distances if thresholds + persistence are fullfilled
// dumpHistogram ... if 1 then dump raw histograms, if ==0 do not dump them, if 2 dump EC histograms, if 3 dump both 
// Function returns APP_SUCCESS_OK if successfully configure the device, else it returns an error APP_ERROR_*
int8_t tmf8828Configure( tmf8828Driver * driver, uint16_t periodInMs, uint16_t kiloIterations, uint8_t spadMapId, uint16_t lowThreshold, uint16_t highThreshold, uint8_t persistence, uint32_t intMask, uint8_t dumpHistogram  );


// Function to execute an i2c address chagne
// driver ... pointer to an instance of the tmf8828 driver data structure
// newI2cSlaveAddress ... the i2c slave address to be used, until the next device disable+enable
// Function returns APP_SUCCESS_OK if successfully changed i2c Address, else it returns an error APP_ERROR_*
int8_t tmf8828ChangeI2CAddress( tmf8828Driver * driver, uint8_t newI2cSlaveAddress );

// Function to execute a factory calibration
// driver ... pointer to an instance of the tmf8828 driver data structure
// Function returns APP_SUCCESS_OK if successfully factory calibrated, else it returns an error APP_ERROR_*
int8_t tmf8828FactoryCalibration( tmf8828Driver * driver );

// Function to reset the factory calibration. Call this function before providing the 4 factory
// calibration pages for tmf8828.
// driver ... pointer to an instance of the tmf8828 driver data structure
int8_t tmf8828ResetFactoryCalibration( tmf8828Driver * driver );

// Function does load the factory calibration page data from file tmf8828_calibration.c and writes
// it to the device's factory calibration page.
// driver ... pointer to an instance of the tmf8828 driver data structure
// calibPage ... pointer to a complete calibration page (must match the SPAD ID)
// Function returns APP_SUCCESS_OK if successfully factory calibrated, else it returns an error APP_ERROR_*
int8_t tmf8828SetStoredFactoryCalibration( tmf8828Driver * driver, const uint8_t * calibPage );

// Function to start a measurement
// driver ... pointer to an instance of the tmf8828 driver data structure
// Function returns APP_SUCCESS_OK if successfully started a measurement, else it returns an error APP_ERROR_*
int8_t tmf8828StartMeasurement( tmf8828Driver * driver );

// Function to stop a measurement 
// driver ... pointer to an instance of the tmf8828 driver data structure
// Function returns APP_SUCCESS_OK if successfully stopped the application, else it returns an error APP_ERROR_*
int8_t tmf8828StopMeasurement( tmf8828Driver * driver );

// Function to switch to 8x8 mode 
// driver ... pointer to an instance of the tmf8828 driver data structure
int8_t tmf8828SwitchTo8x8Mode( tmf8828Driver * driver );

// Function to switch to legacy mode (tmf8821/8820 mode = 4x4 or 3x3) 
// driver ... pointer to an instance of the tmf8828 driver data structure
int8_t tmf8828SwitchToLegacyMode( tmf8828Driver * driver );

// Function reads the interrupts that are set and clears those. 
// driver ... pointer to an instance of the tmf8828 driver data structure
// Function returns the bit-mask of the interrupts that were set.
uint8_t tmf8828GetAndClrInterrupts( tmf8828Driver * driver, uint8_t mask );

// Function clears the given interrupts and enables them.
// driver ... pointer to an instance of the tmf8828 driver data structure
void tmf8828ClrAndEnableInterrupts( tmf8828Driver * driver, uint8_t mask );

// Function disable the given interrupts.
// driver ... pointer to an instance of the tmf8828 driver data structure
void tmf8828DisableInterrupts( tmf8828Driver * driver, uint8_t mask );

// Function to read results and print them on UART. This function should only be called when there was a 
// result interrupt (use function tmf8828GetAndClrInterrupts to find this out). 
// driver ... pointer to an instance of the tmf8828 driver data structure
// Function returns APP_SUCCESS_OK if there was a result page, else APP_ERROR_NO_RESULT_PAGE.
int8_t tmf8828ReadResults( tmf8828Driver * driver );

// Correct the distance based on the clock correction pairs 
// driver ... pointer to an instance of the tmf8828 driver data structure
uint16_t tmf8828CorrectDistance( tmf8828Driver * driver, uint16_t distance );

// Function to read histograms and print them on UART. This function should only be calle dwhen there was a 
// raw histogram interrupt (use function tmf8828GetAndClrInterrupts to find this out). 
// driver ... pointer to an instance of the tmf8828 driver data structure
// Function returns APP_SUCCESS_OK if there was a histogram page, else APP_ERROR_NO_RESULT_PAGE.
int8_t tmf8828ReadHistogram( tmf8828Driver * driver );

#ifdef __cplusplus
}
#endif  

#endif // TMF8828_H 
