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

//  simple tmf8828 driver

// --------------------------------------------------- includes --------------------------------

#include "tmf8828_shim.h"
#include "tmf8828.h"

#define TMF8828_ENABLE 0xe0
#define TMF8828_ENABLE__cpu_reset__MASK  0x80
#define TMF8828_ENABLE__cpu_reset__WIDTH 1
#define TMF8828_ENABLE__cpu_reset__SHIFT 7
#define TMF8828_ENABLE__cpu_reset__RESET 0
#define TMF8828_ENABLE__cpu_reset 128
#define TMF8828_ENABLE__cpu_ready__MASK  0x40
#define TMF8828_ENABLE__cpu_ready__WIDTH 1
#define TMF8828_ENABLE__cpu_ready__SHIFT 6
#define TMF8828_ENABLE__cpu_ready__RESET 0
#define TMF8828_ENABLE__cpu_ready 64
#define TMF8828_ENABLE__powerup_select__MASK  0x30
#define TMF8828_ENABLE__powerup_select__WIDTH 2
#define TMF8828_ENABLE__powerup_select__SHIFT 4
#define TMF8828_ENABLE__powerup_select__RESET 0
// Enumeration for powerup_select
#define TMF8828_ENABLE__powerup_select__no_override 0 // Do what is selected in the boot_select fuses
#define TMF8828_ENABLE__powerup_select__start_boot_monitor 1 // Always start the boot monitor, do not go to sleep
#define TMF8828_ENABLE__powerup_select__start_app_in_ram 2 // Start the application that is in RAM
#define TMF8828_ENABLE__powerup_select__reserved 3 // Reserved for future use

#define TMF8828_ENABLE__pinmux_unlock__MASK  0x8
#define TMF8828_ENABLE__pinmux_unlock__WIDTH 1
#define TMF8828_ENABLE__pinmux_unlock__SHIFT 3
#define TMF8828_ENABLE__pinmux_unlock__RESET 0
#define TMF8828_ENABLE__pinmux_unlock 8
#define TMF8828_ENABLE__timed_standby_mode__MASK  0x4
#define TMF8828_ENABLE__timed_standby_mode__WIDTH 1
#define TMF8828_ENABLE__timed_standby_mode__SHIFT 2
#define TMF8828_ENABLE__timed_standby_mode__RESET 0
#define TMF8828_ENABLE__timed_standby_mode 4
#define TMF8828_ENABLE__standby_mode__MASK  0x2
#define TMF8828_ENABLE__standby_mode__WIDTH 1
#define TMF8828_ENABLE__standby_mode__SHIFT 1
#define TMF8828_ENABLE__standby_mode__RESET 0
#define TMF8828_ENABLE__standby_mode 2
#define TMF8828_ENABLE__pon__MASK  0x1
#define TMF8828_ENABLE__pon__WIDTH 1
#define TMF8828_ENABLE__pon__SHIFT 0
#define TMF8828_ENABLE__pon__RESET 1
#define TMF8828_ENABLE__pon 1

#define TMF8828_INT_STATUS 0xe1
#define TMF8828_INT_STATUS__int8__MASK  0x80
#define TMF8828_INT_STATUS__int8__WIDTH 1
#define TMF8828_INT_STATUS__int8__SHIFT 7
#define TMF8828_INT_STATUS__int8__RESET 0
#define TMF8828_INT_STATUS__int8 128
#define TMF8828_INT_STATUS__int7__MASK  0x40
#define TMF8828_INT_STATUS__int7__WIDTH 1
#define TMF8828_INT_STATUS__int7__SHIFT 6
#define TMF8828_INT_STATUS__int7__RESET 0
#define TMF8828_INT_STATUS__int7 64
#define TMF8828_INT_STATUS__int6__MASK  0x20
#define TMF8828_INT_STATUS__int6__WIDTH 1
#define TMF8828_INT_STATUS__int6__SHIFT 5
#define TMF8828_INT_STATUS__int6__RESET 0
#define TMF8828_INT_STATUS__int6 32
#define TMF8828_INT_STATUS__int5__MASK  0x10
#define TMF8828_INT_STATUS__int5__WIDTH 1
#define TMF8828_INT_STATUS__int5__SHIFT 4
#define TMF8828_INT_STATUS__int5__RESET 0
#define TMF8828_INT_STATUS__int5 16
#define TMF8828_INT_STATUS__int4__MASK  0x8
#define TMF8828_INT_STATUS__int4__WIDTH 1
#define TMF8828_INT_STATUS__int4__SHIFT 3
#define TMF8828_INT_STATUS__int4__RESET 0
#define TMF8828_INT_STATUS__int4 8
#define TMF8828_INT_STATUS__int3__MASK  0x4
#define TMF8828_INT_STATUS__int3__WIDTH 1
#define TMF8828_INT_STATUS__int3__SHIFT 2
#define TMF8828_INT_STATUS__int3__RESET 0
#define TMF8828_INT_STATUS__int3 4
#define TMF8828_INT_STATUS__int2__MASK  0x2
#define TMF8828_INT_STATUS__int2__WIDTH 1
#define TMF8828_INT_STATUS__int2__SHIFT 1
#define TMF8828_INT_STATUS__int2__RESET 0
#define TMF8828_INT_STATUS__int2 2
#define TMF8828_INT_STATUS__int1__MASK  0x1
#define TMF8828_INT_STATUS__int1__WIDTH 1
#define TMF8828_INT_STATUS__int1__SHIFT 0
#define TMF8828_INT_STATUS__int1__RESET 0
#define TMF8828_INT_STATUS__int1 1

#define TMF8828_INT_ENAB 0xe2
#define TMF8828_INT_ENAB__int8_enab__MASK  0x80
#define TMF8828_INT_ENAB__int8_enab__WIDTH 1
#define TMF8828_INT_ENAB__int8_enab__SHIFT 7
#define TMF8828_INT_ENAB__int8_enab__RESET 0
#define TMF8828_INT_ENAB__int8_enab 128
#define TMF8828_INT_ENAB__int7_enab__MASK  0x40
#define TMF8828_INT_ENAB__int7_enab__WIDTH 1
#define TMF8828_INT_ENAB__int7_enab__SHIFT 6
#define TMF8828_INT_ENAB__int7_enab__RESET 0
#define TMF8828_INT_ENAB__int7_enab 64
#define TMF8828_INT_ENAB__int6_enab__MASK  0x20
#define TMF8828_INT_ENAB__int6_enab__WIDTH 1
#define TMF8828_INT_ENAB__int6_enab__SHIFT 5
#define TMF8828_INT_ENAB__int6_enab__RESET 0
#define TMF8828_INT_ENAB__int6_enab 32
#define TMF8828_INT_ENAB__int5_enab__MASK  0x10
#define TMF8828_INT_ENAB__int5_enab__WIDTH 1
#define TMF8828_INT_ENAB__int5_enab__SHIFT 4
#define TMF8828_INT_ENAB__int5_enab__RESET 0
#define TMF8828_INT_ENAB__int5_enab 16
#define TMF8828_INT_ENAB__int4_enab__MASK  0x8
#define TMF8828_INT_ENAB__int4_enab__WIDTH 1
#define TMF8828_INT_ENAB__int4_enab__SHIFT 3
#define TMF8828_INT_ENAB__int4_enab__RESET 0
#define TMF8828_INT_ENAB__int4_enab 8
#define TMF8828_INT_ENAB__int3_enab__MASK  0x4
#define TMF8828_INT_ENAB__int3_enab__WIDTH 1
#define TMF8828_INT_ENAB__int3_enab__SHIFT 2
#define TMF8828_INT_ENAB__int3_enab__RESET 0
#define TMF8828_INT_ENAB__int3_enab 4
#define TMF8828_INT_ENAB__int2_enab__MASK  0x2
#define TMF8828_INT_ENAB__int2_enab__WIDTH 1
#define TMF8828_INT_ENAB__int2_enab__SHIFT 1
#define TMF8828_INT_ENAB__int2_enab__RESET 0
#define TMF8828_INT_ENAB__int2_enab 2
#define TMF8828_INT_ENAB__int1_enab__MASK  0x1
#define TMF8828_INT_ENAB__int1_enab__WIDTH 1
#define TMF8828_INT_ENAB__int1_enab__SHIFT 0
#define TMF8828_INT_ENAB__int1_enab__RESET 0
#define TMF8828_INT_ENAB__int1_enab 1
 
#define TMF8828_ID 0xe3
#define TMF8828_ID__id__MASK  0x3f
#define TMF8828_ID__id__WIDTH 6
#define TMF8828_ID__id__SHIFT 0
#define TMF8828_ID__id__RESET 08

#define TMF8828_REVID 0xe4
#define TMF8828_REVID__rev_id__MASK  0x7
#define TMF8828_REVID__rev_id__WIDTH 3
#define TMF8828_REVID__rev_id__SHIFT 0
#define TMF8828_REVID__rev_id__RESET 0

#define TMF8828_CLOCK 0xec
#define TMF8828_CLOCK__pll_clk_select__MASK  0x40
#define TMF8828_CLOCK__pll_clk_select__WIDTH 1
#define TMF8828_CLOCK__pll_clk_select__SHIFT 6
#define TMF8828_CLOCK__pll_clk_select__RESET 0
#define TMF8828_CLOCK__pll_clk_select 64
#define TMF8828_CLOCK__pll_on__MASK  0x20
#define TMF8828_CLOCK__pll_on__WIDTH 1
#define TMF8828_CLOCK__pll_on__SHIFT 5
#define TMF8828_CLOCK__pll_on__RESET 0
#define TMF8828_CLOCK__pll_on 32
#define TMF8828_CLOCK__enab_pllclk__MASK  0x10
#define TMF8828_CLOCK__enab_pllclk__WIDTH 1
#define TMF8828_CLOCK__enab_pllclk__SHIFT 4
#define TMF8828_CLOCK__enab_pllclk__RESET 0
#define TMF8828_CLOCK__enab_pllclk 16
#define TMF8828_CLOCK__pll_lost_lock__MASK  0x8
#define TMF8828_CLOCK__pll_lost_lock__WIDTH 1
#define TMF8828_CLOCK__pll_lost_lock__SHIFT 3
#define TMF8828_CLOCK__pll_lost_lock__RESET 0
#define TMF8828_CLOCK__pll_lost_lock 8
#define TMF8828_CLOCK__enable_500khz_mode__MASK  0x4
#define TMF8828_CLOCK__enable_500khz_mode__WIDTH 1
#define TMF8828_CLOCK__enable_500khz_mode__SHIFT 2
#define TMF8828_CLOCK__enable_500khz_mode__RESET 0
#define TMF8828_CLOCK__enable_500khz_mode 4
#define TMF8828_CLOCK__clock_spare__MASK  0x3
#define TMF8828_CLOCK__clock_spare__WIDTH 2
#define TMF8828_CLOCK__clock_spare__SHIFT 0
#define TMF8828_CLOCK__clock_spare__RESET 0

#define TMF8828_RESETREASON 0xf0
#define TMF8828_RESETREASON__soft_reset__MASK  0x80
#define TMF8828_RESETREASON__soft_reset__WIDTH 1
#define TMF8828_RESETREASON__soft_reset__SHIFT 7
#define TMF8828_RESETREASON__soft_reset__RESET 0
#define TMF8828_RESETREASON__soft_reset 128
#define TMF8828_RESETREASON__rrsn_power_on_timer__MASK  0x20
#define TMF8828_RESETREASON__rrsn_power_on_timer__WIDTH 1
#define TMF8828_RESETREASON__rrsn_power_on_timer__SHIFT 5
#define TMF8828_RESETREASON__rrsn_power_on_timer__RESET 0
#define TMF8828_RESETREASON__rrsn_power_on_timer 32
#define TMF8828_RESETREASON__rrsn_power_on_i2c__MASK  0x10
#define TMF8828_RESETREASON__rrsn_power_on_i2c__WIDTH 1
#define TMF8828_RESETREASON__rrsn_power_on_i2c__SHIFT 4
#define TMF8828_RESETREASON__rrsn_power_on_i2c__RESET 0
#define TMF8828_RESETREASON__rrsn_power_on_i2c 16
#define TMF8828_RESETREASON__rrsn_watchdog__MASK  0x8
#define TMF8828_RESETREASON__rrsn_watchdog__WIDTH 1
#define TMF8828_RESETREASON__rrsn_watchdog__SHIFT 3
#define TMF8828_RESETREASON__rrsn_watchdog__RESET 0
#define TMF8828_RESETREASON__rrsn_watchdog 8
#define TMF8828_RESETREASON__rrsn_cpu_reset__MASK  0x4
#define TMF8828_RESETREASON__rrsn_cpu_reset__WIDTH 1
#define TMF8828_RESETREASON__rrsn_cpu_reset__SHIFT 2
#define TMF8828_RESETREASON__rrsn_cpu_reset__RESET 0
#define TMF8828_RESETREASON__rrsn_cpu_reset 4
#define TMF8828_RESETREASON__rrsn_soft_reset__MASK  0x2
#define TMF8828_RESETREASON__rrsn_soft_reset__WIDTH 1
#define TMF8828_RESETREASON__rrsn_soft_reset__SHIFT 1
#define TMF8828_RESETREASON__rrsn_soft_reset__RESET 0
#define TMF8828_RESETREASON__rrsn_soft_reset 2
#define TMF8828_RESETREASON__rrsn_coldstart__MASK  0x1
#define TMF8828_RESETREASON__rrsn_coldstart__WIDTH 1
#define TMF8828_RESETREASON__rrsn_coldstart__SHIFT 0
#define TMF8828_RESETREASON__rrsn_coldstart__RESET 0
#define TMF8828_RESETREASON__rrsn_coldstart 1


// --------------------------------------------------- defines --------------------------------

#define TMF8828_COM_APP_ID                                  0x0   // register address
#define TMF8828_COM_APP_ID__application                     0x3   // measurement application id
#define TMF8828_COM_APP_ID__bootloader                      0x80  // bootloader application id

#define TMF8828_COM_TMF8828_MODE                            0x10 // mode register is either 0x00 == tmf8820/1 or 0x08 == tmf8828                 
#define TMF8828_COM_TMF8828_MODE__mode__TMF8821             0    // the device is operating in 3x3/3x6/4x4 (TMF8820/TMF8821) mode       
#define TMF8828_COM_TMF8828_MODE__mode__TMF8828             8

// --------------------------------------------------- bootloader -----------------------------

#define TMF8X2X_BL_MAX_DATA_SIZE                  0x80  // Number of bytes that can be written or read with one BL command
#define TMF8828_COM_CMD_STAT                      0x08

#define TMF8828_COM_CMD_STAT__bl_cmd_ok           0x00  
#define TMF8828_COM_CMD_STAT__bl_cmd_errors       0x0F  // all success/error are below or equal to this number
#define TMF8828_COM_CMD_STAT__bl_cmd_ramremap     0x11  // Bootloader command to remap the vector table into RAM (Start RAM application).
#define TMF8828_COM_CMD_STAT__bl_cmd_r_ram        0x40  // Read from BL RAM.
#define TMF8828_COM_CMD_STAT__bl_cmd_w_ram        0x41  // Write to BL RAM.
#define TMF8828_COM_CMD_STAT__bl_cmd_addr_ram     0x43  // Set address pointer in RAM for Read/Write to BL RAM.

#define BL_HEADER           2     // bootloader header is 2 bytes
#define BL_MAX_DATA_PAYLOAD 128   // bootloader data payload can be up to 128
#define BL_FOOTER           1     // bootloader footer is 1 byte

// Bootloader maximum wait sequences
#define BL_CMD_SET_ADDR_TIMEOUT_MS    1
#define BL_CMD_W_RAM_TIMEOUT_MS       1
#define BL_CMD_RAM_REMAP_TIMEOUT_MS   1

// wait time for clock source select change to take effect
#define CLK_SRC_SELECT_WAIT_MS        1
// wait for version readout, to switch from ROM to RAM (and have the version published on I2C)
#define APP_PUBLISH_VERSION_WAIT_TIME_MS 10

// --------------------------------------------------- application ----------------------------

// application status, we check only for ok or accepted, everything between 2 and 15 (inclusive) 
// is an error
#define TMF8828_COM_CMD_STAT__stat_ok                       0x0  // Everything is okay
#define TMF8828_COM_CMD_STAT__stat_accepted                 0x1  // Everything is okay too, send sop to halt ongoing command

// application commands
#define TMF8828_COM_CMD_STAT__cmd_measure                             0x10  // Start a measurement
#define TMF8828_COM_CMD_STAT__cmd_stop                                0xff  // Stop a measurement
#define TMF8828_COM_CMD_STAT__cmd_write_config_page                   0x15  // Write the active config page
#define TMF8828_COM_CMD_STAT__cmd_load_config_page_common             0x16  // Load the common config page
#define TMF8828_COM_CMD_STAT__cmd_load_config_page_factory_calib      0x19  // Load the factory calibration config page
#define TMF8828_COM_CMD_STAT__cmd_stat__CMD_RESET_FACTORY_CALIBRATION 0x1F // Manually reset the factory calibration. Only supported if 8x8_measurements = 1.                      
#define TMF8828_COM_CMD_STAT__cmd_factory_calibration                 0x20  // Perform a factory calibration

#define TMF8828_COM_CMD_STAT__cmd_i2c_slave_address                   0x21  // change I2C address

#define TMF8828_COM_CMD_STAT__cmd_stat__CMD_SWITCH_TMF8821_MODE       0x65 // Switch to 3x3/3x6/4x4 mode. The device will need to be re-configured after this command. Only supported if 8x8_measurements = 1.  
#define TMF8828_COM_CMD_STAT__cmd_stat__CMD_SWITCH_TMF8828_MODE       0x6C // Switch to 8x8 mode. The device will need to be re-configured after this command. Only supported if 8x8_measurements = 1. 

#define TMF8828_COM_SERIAL_NUMBER_0                         0x1c // register with serial number

// configuration page addresses and defines
#define TMF8828_COM_PERIOD_MS_LSB                           0x24  // period in milliseconds
#define TMF8828_COM_PERIOD_MS_MSB                           0x25
#define TMF8828_COM_KILO_ITERATIONS_LSB                     0x26  // Kilo (1024) iterations
#define TMF8828_COM_KILO_ITERATIONS_MSB                     0x27
#define TMF8828_COM_SPAD_MAP_ID                             0x34  // configure the SPAD map id, with some example maps
#define TMF8828_COM_SPAD_MAP_ID__map_last                   0x15  // maximum allowed spad map id, for tmf8828 only 15 is allowed
#define TMF8X2X_COM_HIST_DUMP                               0x39  // 0 ... all off, 1 ... raw histograms, 2 ... ec histograms
#define TMF8X2X_COM_I2C_SLAVE_ADDRESS                       0x3b  // register that holds the 7-bit shifted slave address
#define TMF8X2X_COM_ALG_SETTING_0                           0x35  // register that holds the algorithm settings

/* show distance results with extended confidence range
   report distances -> 0x04
   switch on extended confidence range -> 0x80
   0x04 | 0x80 -> 0x84 
   please see TMF882X data sheet for details
*/
#define TMF8828_ENABLE_LOGARITHMIC_CONFIDENCE               0x84
// Application maximum wait sequences
#define APP_CMD_LOAD_CONFIG_TIMEOUT_MS                      3
#define APP_CMD_WRITE_CONFIG_TIMEOUT_MS                     3
#define APP_CMD_MEASURE_TIMEOUT_MS                          5
#define APP_CMD_STOP_TIMEOUT_MS                             25
#define APP_CMD_FACTORY_CALIB_TIMEOUT_MS                    2000
#define APP_CMD_I2C_SLAVE_ADDRESS_TIMEOUT_MS                1
#define APP_CMD_SWITCH_MODE_CMD_TIMEOUT_MS                  1     // timeout until command is accepted
#define APP_CMD_SWITCH_MODE_TIMEOUT_MS                      10

// -------------------------------------------------------- some checks --------------------------------------------

// check that we can read a complete result page also in the dataBuffer
#define DATA_BUFFER_SIZE                  (TMF8828_COM_CONFIG_FACTORY_CALIB__factory_calibration_size)

#if ( ( (BL_HEADER + BL_MAX_DATA_PAYLOAD + BL_FOOTER + 1) > DATA_BUFFER_SIZE ) || ( (TMF8828_COM_CONFIG_RESULT__measurement_result_size) > DATA_BUFFER_SIZE ) )
  #error "Increase data buffer size"
#endif


// clock correction pairs index calculation
#define CLK_CORRECTION_IDX_MODULO( x )    ( (x) & ( (CLK_CORRECTION_PAIRS)-1 ) )

// how accurate the calculation is going to be. The higher the accuracy the less far apart are
// the pairs allowed. An 8 precision means that the factor is 1/256 accurate.
#define CALC_PRECISION                                                  8
// Need this to add to the corrected distance before shifting right
#define HALF_CALC_PRECISION                                             ( 1 << ((CALC_PRECISION) - 1 ) )
#define CALC_DISTANCE_CORR_FACTOR( hostTickDiff, tmf8828TickDiff )      ( ( ( (hostTickDiff) * (TMF8828_TICKS_PER_US) ) << (CALC_PRECISION) ) / ( (tmf8828TickDiff) * (HOST_TICKS_PER_US) ) ) 
// Round before performing the division (right shift), make sure it is a logical shift right and not an arithmetical shift right
#define CALC_DISTANCE( distance, hostTickDiff, tmf8828TickDiff )        ( ( (uint32_t)( (distance) * CALC_DISTANCE_CORR_FACTOR( hostTickDiff, tmf8828TickDiff ) + (HALF_CALC_PRECISION) ) ) >> (CALC_PRECISION) )

// Find the maximum distance values to avoid mathematical errors due to overflow 
#define MAX_HOST_DIFF_VALUE                                             ( ( 0xFFFFFFFFUL / (TMF8828_TICKS_PER_US) ) >> CALC_PRECISION )
#define MAX_TMF8828_DIFF_VALUE                                          ( ( 0xFFFFFFFFUL / (HOST_TICKS_PER_US) )

// Saturation macro for 16-bit
#define SATURATE16( v )                                                 ( (v) > 0xFFFF ? 0xFFFF : (v) )

// For TMF882x sys ticks to be valid the LSB must be set.
#define TMF8828_SYS_TICK_IS_VALID( tick )                               ( (tick) & 1 )

// -------------------------------------------------------- constants ----------------------------------------------


// Driver Version
const tmf8828DriverInfo tmf8828DriverInfoReset = 
{ .version = { TMF8828_DRIVER_MAJOR_VERSION , TMF8828_DRIVER_MINOR_VERSION }
};

const tmf8828DeviceInfo tmf8828DeviceInfoReset =
{ .deviceSerialNumber = 0
, .appVersion = { 0, 0, 0, 0 }
, .chipVersion = { 0, 0}
};

// -------------------------------------------------------- variables ----------------------------------------------

uint8_t dataBuffer[ DATA_BUFFER_SIZE ];           // transfer/receive buffer

// -------------------------------------------------------- functions ----------------------------------------------

static void tmf8828ResetClockCorrection( tmf8828Driver * driver );

void tmf8828Initialise ( tmf8828Driver * driver )
{
  tmf8828ResetClockCorrection( driver );
  driver->device = tmf8828DeviceInfoReset;
  driver->info = tmf8828DriverInfoReset;
  driver->i2cSlaveAddress = TMF8828_SLAVE_ADDR;
  driver->clkCorrectionEnable = 1;                  // default is on
  driver->logLevel =TMF8828_LOG_LEVEL_ERROR;
}

// Function to overwrite the default log level
void tmf8828SetLogLevel ( tmf8828Driver * driver, uint8_t level )
{
  driver->logLevel = level;
}

// Function to set clock correction on or off.
// enable ... if <>0 clock correction is enabled (default)
// enable ... if ==0 clock correction is disabled
void tmf8828ClkCorrection ( tmf8828Driver * driver, uint8_t enable )
{
  driver->clkCorrectionEnable = !!enable;
}

// Function executes a reset of the device 
void tmf8828Reset ( tmf8828Driver * driver ) 
{
  i2cRxReg( driver, driver->i2cSlaveAddress, TMF8828_ENABLE, 1, dataBuffer );      // read the enable register to determine if chip can handle i2c communication to registers != 0xE0
  if ( dataBuffer[0] & TMF8828_ENABLE__pon__MASK )  
  {
    // make sure that the PLL is off before performing a reset
    i2cRxReg( driver, driver->i2cSlaveAddress, TMF8828_CLOCK, 1, dataBuffer );
    dataBuffer[0] &= (TMF8828_CLOCK__pll_on__MASK | TMF8828_CLOCK__enab_pllclk__MASK ); // clear pll as clock source
    i2cTxReg( driver, driver->i2cSlaveAddress, TMF8828_CLOCK, 1, dataBuffer );
    delayInMicroseconds( CLK_SRC_SELECT_WAIT_MS * 1000UL );
    dataBuffer[0] = 0;
    i2cTxReg( driver, driver->i2cSlaveAddress, TMF8828_CLOCK, 1, dataBuffer );            // switch off pll
 
  	dataBuffer[0] = TMF8828_RESETREASON__soft_reset__MASK;
    i2cTxReg( driver, driver->i2cSlaveAddress, TMF8828_RESETREASON, 1, dataBuffer );
  	if ( driver->logLevel >=TMF8828_LOG_LEVEL_VERBOSE ) 
  	{
      PRINT_STR( "reset" );
      PRINT_LN( );
    }
  }
  else
  {
    if ( driver->logLevel >=TMF8828_LOG_LEVEL_ERROR ) 
    {
      PRINT_STR( "Device PON=0, cannot reset" );
      PRINT_LN( );
    }
  }
  tmf8828ResetClockCorrection( driver );
}

// Function sets the enable PIN high
void tmf8828Enable ( tmf8828Driver * driver ) 
{
  enablePinHigh( driver );
  tmf8828Initialise( driver );           // when enable gets high, the HW resets to default slave addr
}

// Function clears the enable PIN (=low)
void tmf8828Disable ( tmf8828Driver * driver ) 
{
  enablePinLow( driver );
}

// Function checks if the CPU becomes ready within the given time
int8_t tmf8828IsCpuReady ( tmf8828Driver * driver, uint8_t waitInMs )
{
  i2cRxReg( driver, driver->i2cSlaveAddress, TMF8828_ENABLE, 1, dataBuffer );        // Need to read it twice after a PON=0, so do it 1 additional time 
  do 
  {
    dataBuffer[0] = 0;                                        // clear before reading
    i2cRxReg( driver, driver->i2cSlaveAddress, TMF8828_ENABLE, 1, dataBuffer );      // read the enable register to determine cpu ready
    if ( !!( dataBuffer[0] & TMF8828_ENABLE__cpu_ready__MASK ) )
    {
      if ( driver->logLevel >=TMF8828_LOG_LEVEL_VERBOSE )
      {
        PRINT_STR( "CPU ready" );
        PRINT_LN( );
      }
      return 1;                                               // done                  
    }
    else if ( waitInMs )                                      // only wait until it is the last time we go through the loop, that would be a waste of time to wait again
    {
      delayInMicroseconds( 1000 );
    }
  } while ( waitInMs-- );
  if ( driver->logLevel >=TMF8828_LOG_LEVEL_ERROR )
  {
    PRINT_STR( "#Err" );
    PRINT_CHAR( SEPARATOR );
    PRINT_STR( "CPU not ready" );
    PRINT_LN( );
  }
  return 0;                                                 // cpu did not get ready
}

// Function attemps a wakeup of the device 
void tmf8828Wakeup ( tmf8828Driver * driver ) 
{
  dataBuffer[0] = 0;                                         // clear before reading
  i2cRxReg( driver, driver->i2cSlaveAddress, TMF8828_ENABLE, 1, dataBuffer );      // read the enable register to dermine power state
  if ( ( dataBuffer[0] & TMF8828_ENABLE__cpu_ready__MASK ) == 0 )                  
  {
    dataBuffer[0] = dataBuffer[0] | TMF8828_ENABLE__pon__MASK;      // make sure to keep the remap bits
    i2cTxReg( driver, driver->i2cSlaveAddress, TMF8828_ENABLE, 1, dataBuffer );    // set PON bit in enable register
    if ( driver->logLevel >=TMF8828_LOG_LEVEL_VERBOSE ) 
    {
      PRINT_STR( "PON=1" );
      PRINT_LN( );
    }
  }
  else
  {
    if ( driver->logLevel >=TMF8828_LOG_LEVEL_VERBOSE ) 
    {
      PRINT_STR( "awake TMF8828_ENABLE=0x" );
      PRINT_UINT_HEX( dataBuffer[0] );
      PRINT_LN( );
    }  
  }
}

// Function puts the device in standby state
void tmf8828Standby ( tmf8828Driver * driver ) 
{
  dataBuffer[0] = 0;                                                   // clear before reading
  i2cRxReg( driver, driver->i2cSlaveAddress, TMF8828_ENABLE, 1, dataBuffer );      // read the enable register to determine power state
  if ( ( dataBuffer[0] & TMF8828_ENABLE__cpu_ready__MASK ) != 0 )                  
  {
    dataBuffer[0] = dataBuffer[0] & ~TMF8828_ENABLE__pon__MASK;                         // clear only the PON bit
    i2cTxReg( driver, driver->i2cSlaveAddress, TMF8828_ENABLE, 1, dataBuffer );   // clear PON bit in enable register
    if ( driver->logLevel >=TMF8828_LOG_LEVEL_VERBOSE ) 
    {
      PRINT_STR( "PON=0" );
      PRINT_LN( );
    }
  }
  else
  {
    if ( driver->logLevel >=TMF8828_LOG_LEVEL_VERBOSE ) 
    {
      PRINT_STR( "standby TMF8828_ENABLE=0x" );
      PRINT_UINT_HEX( dataBuffer[0] );
      PRINT_LN( );
    }  
  }
}

// function to check if a register has a specific value
static int8_t tmf8828CheckRegister ( tmf8828Driver * driver, uint8_t regAddr, uint8_t expected, uint8_t len, uint16_t timeoutInMs )
{
  uint8_t i;
  uint32_t t = getSysTick();
  do 
  {
    dataBuffer[0] = ~expected;
    i2cRxReg( driver, driver->i2cSlaveAddress, regAddr, len, dataBuffer );
    if ( dataBuffer[0] == expected )
    {
      return APP_SUCCESS_OK; 
    }
    else if ( timeoutInMs )                             // do not wait if timeout is 0
    {
      delayInMicroseconds(1000);  
    }
  } while ( timeoutInMs-- > 0 );
  if ( driver->logLevel >=TMF8828_LOG_LEVEL_ERROR ) 
  {
    t = getSysTick() - t;
    PRINT_STR( "#Err" );
    PRINT_CHAR( SEPARATOR );
    PRINT_STR( "timeout " );
    PRINT_INT( t );
    PRINT_STR( " reg=0x" );
    PRINT_UINT_HEX( regAddr );
    PRINT_STR( " exp=0x" );
    PRINT_UINT_HEX( expected );
    for ( i = 0; i < len; i++ )
    {
      PRINT_STR( " 0x" );
      PRINT_UINT_HEX( dataBuffer[i] );
    }
    PRINT_LN( );
  }      
  return APP_ERROR_TIMEOUT;        // error timeout
}


// --------------------------------------- bootloader ------------------------------------------

// calculate the checksum according to bootloader spec
static uint8_t tmf8828BootloaderChecksum ( uint8_t * data, uint8_t len )      
{
  uint8_t sum = 0;
  while ( len-- > 0 )
  {
    sum += *data;
    data++;
  }
  sum = sum ^ 0xFF;
  return sum;
}

// execute command to set the RAM address pointer for RAM read/writes
static int8_t tmf8828BootloaderSetRamAddr ( tmf8828Driver * driver, uint16_t addr )
{
  dataBuffer[0] = TMF8828_COM_CMD_STAT__bl_cmd_addr_ram;
  dataBuffer[1] = 2;
  dataBuffer[2] = (uint8_t)addr;        // LSB of addr
  dataBuffer[3] = (uint8_t)(addr>>8);    // MSB of addr
  dataBuffer[4] = tmf8828BootloaderChecksum( dataBuffer, 4 );
  i2cTxReg( driver, driver->i2cSlaveAddress, TMF8828_COM_CMD_STAT, 5, dataBuffer );
  return tmf8828CheckRegister( driver, TMF8828_COM_CMD_STAT, TMF8828_COM_CMD_STAT__bl_cmd_ok, 3, BL_CMD_SET_ADDR_TIMEOUT_MS );      // many BL errors only have 3 bytes 
}

// execute command to write a chunk of data to RAM
static int8_t tmf8828BootloaderWriteRam ( tmf8828Driver * driver, uint8_t len )
{
  dataBuffer[0] = TMF8828_COM_CMD_STAT__bl_cmd_w_ram;
  dataBuffer[1] = len;
  dataBuffer[BL_HEADER+len] = tmf8828BootloaderChecksum( dataBuffer, BL_HEADER+len );
  i2cTxReg( driver, driver->i2cSlaveAddress, TMF8828_COM_CMD_STAT, BL_HEADER+len+BL_FOOTER, dataBuffer );
  return tmf8828CheckRegister( driver, TMF8828_COM_CMD_STAT,TMF8828_COM_CMD_STAT__bl_cmd_ok, 3, BL_CMD_W_RAM_TIMEOUT_MS );    // many BL errors only have 3 bytes 
}

// execute command RAM remap to address 0 and continue running from RAM 
static int8_t tmf8828BootloaderRamRemap ( tmf8828Driver * driver, uint8_t appId )
{
  int8_t stat;
  dataBuffer[0] = TMF8828_COM_CMD_STAT__bl_cmd_ramremap;
  dataBuffer[1] = 0;
  dataBuffer[BL_HEADER] = tmf8828BootloaderChecksum( dataBuffer, BL_HEADER );
  i2cTxReg( driver, driver->i2cSlaveAddress, TMF8828_COM_CMD_STAT, BL_HEADER+BL_FOOTER, dataBuffer );
  delayInMicroseconds( APP_PUBLISH_VERSION_WAIT_TIME_MS * 1000 );
  // ram remap -> the bootloader will not answer to this command if successfull, so check the application id register instead
  stat = tmf8828CheckRegister( driver, TMF8828_COM_APP_ID, appId, 4, BL_CMD_RAM_REMAP_TIMEOUT_MS );  // tmf8828 application has 4 verion bytes
  tmf8828ReadDeviceInfo( driver );
  if ( driver->logLevel >=TMF8828_LOG_LEVEL_INFO )
  {
    PRINT_STR( "#Vers" );
    PRINT_CHAR( SEPARATOR );
    PRINT_INT( dataBuffer[0] );
    PRINT_CHAR(  '.' );
    PRINT_INT( dataBuffer[1] );
    PRINT_CHAR(  '.' );
    PRINT_INT( dataBuffer[2] );
    PRINT_CHAR(  '.' );
    PRINT_INT( dataBuffer[3] );
    PRINT_LN( );
  }
  return stat;
}

// download the image file to RAM
int8_t tmf8828DownloadFirmware ( tmf8828Driver * driver, uint32_t imageStartAddress, const uint8_t * image, int32_t imageSizeInBytes ) 
{
  int32_t idx = 0;
  int8_t stat = BL_SUCCESS_OK;
  uint8_t chunkLen;
  if ( driver->logLevel >=TMF8828_LOG_LEVEL_VERBOSE ) 
  {
    PRINT_STR( "Image addr=0x" );
    PRINT_UINT_HEX( PTR_TO_UINT(image) );
    PRINT_STR( " len=" );
    PRINT_INT( imageSizeInBytes );
    for ( idx = 0; idx < 16; idx++ )
    {
      uint8_t d = readProgramMemoryByte( (image + idx) );
      PRINT_STR( " 0x" );
      PRINT_UINT_HEX( d );      // read from program memory space
    }
    PRINT_LN( );
  }
  stat = tmf8828BootloaderSetRamAddr( driver, imageStartAddress );
  idx = 0;  // start again at the image begin
  while ( stat == BL_SUCCESS_OK && idx < imageSizeInBytes )
  {
      if ( driver->logLevel >=TMF8828_LOG_LEVEL_VERBOSE )
      {
        PRINT_STR( "Download addr=0x" );
        PRINT_UINT_HEX( (uint32_t)idx );
        PRINT_LN( );
      }
      for( chunkLen=0; chunkLen < BL_MAX_DATA_PAYLOAD && idx < imageSizeInBytes; chunkLen++, idx++ )
      {
        dataBuffer[BL_HEADER + chunkLen] = readProgramMemoryByte( (image + idx) );              // read from code memory into local ram buffer
      }
      stat = tmf8828BootloaderWriteRam( driver, chunkLen );
  }
  if ( stat == BL_SUCCESS_OK )
  {
    stat = tmf8828BootloaderRamRemap( driver, TMF8828_COM_APP_ID__application );      // if you load a test-application this may have another ID
    if ( stat == BL_SUCCESS_OK )
    {
      if ( driver->logLevel >=TMF8828_LOG_LEVEL_INFO )
      {
        PRINT_STR( "FW downloaded" );
        PRINT_LN( );
      }
      return stat;
    }
  }
  if ( driver->logLevel >=TMF8828_LOG_LEVEL_ERROR ) 
  {
    PRINT_STR( "#Err" );
    PRINT_CHAR( SEPARATOR );
    PRINT_STR( "FW downl or REMAP" );
    PRINT_LN( );
  }
  return stat;
}

// --------------------------------------- application -----------------------------------------

// function reads complete device information from the tmf8806
int8_t tmf8828ReadDeviceInfo ( tmf8828Driver * driver )
{
  driver->device = tmf8828DeviceInfoReset; 
  i2cRxReg( driver, driver->i2cSlaveAddress, TMF8828_ID, 2, driver->device.chipVersion );
  i2cRxReg( driver, driver->i2cSlaveAddress, TMF8828_COM_APP_ID, 4, driver->device.appVersion );  // tmf8828 application has 4 verion bytes
  if ( driver->device.appVersion[0] == TMF8828_COM_APP_ID__application )
  {
    i2cRxReg( driver, driver->i2cSlaveAddress, TMF8828_COM_SERIAL_NUMBER_0, 4, dataBuffer );
    driver->device.deviceSerialNumber = tmf8828GetUint32( &(dataBuffer[0]) );
  }
  return APP_SUCCESS_OK;
}


// Reset clock correction calculation
static void tmf8828ResetClockCorrection ( tmf8828Driver * driver )
{
  uint8_t i;
  driver->clkCorrectionIdx = 0;                      // reset clock correction
  driver->clkCorrRatioUQ = (1<<15);                  // this is 1.0 in UQ1.15 
  for ( i = 0; i < CLK_CORRECTION_PAIRS; i++ )
  {
    driver->hostTicks[ i ] = 0;
    driver->tmf8828Ticks[ i ] = 0;                  // initialise the tmf8828Ticks to a value that has the LSB cleared -> can identify that these are no real ticks
  }
  if ( driver->logLevel &TMF8828_LOG_LEVEL_CLK_CORRECTION )
  {
    PRINT_STR( "ClkCorr reset" );
    PRINT_LN( );
  }
}
#define MAX_UINT_VALUE                  (0x80000000)
// function calculates the ratio between hDiff and tDiff in Uq1.15
static uint16_t tmf8828CalcClkRatioUQ16 ( uint32_t hDiff, uint32_t tDiff, uint16_t prevRatioUQ )
{
  uint32_t ratioUQ = prevRatioUQ;
  if (  (hDiff >= HOST_TICKS_PER_US)
     && (tDiff >= TMF8828_TICKS_PER_US)
     && (hDiff < tDiff )                  // if this condition is wrong than the tmf8806 has wrapped over but host not
     ) 
  { /* move both values to be as big as possible to increase precision to max. possible */
    while ( hDiff < MAX_UINT_VALUE && tDiff < MAX_UINT_VALUE ) 
    {
        hDiff <<= 1;            
        tDiff <<= 1;
    }
    tDiff = tDiff / TMF8828_TICKS_PER_US;
    hDiff = hDiff / HOST_TICKS_PER_US;
    while ( hDiff < MAX_UINT_VALUE && tDiff < MAX_UINT_VALUE )      /* scale up again */ 
    {
        hDiff <<= 1;            
        tDiff <<= 1;
    }
    tDiff = ( tDiff + (1<<14)) >> 15;    /* The number of shifts defines the number of "digits" after the decimal point. UQ1.15 range is [0..2), 1<<15=32768==1.0 */
    if ( tDiff )                         /* this can get 0 if the value was close to 2^32 because of the adding of 2^14 */
    {
      ratioUQ = (hDiff + (tDiff>>1)) / tDiff;                                       /* round the ratio */

      /* ratioUQ16 is the range of [0..2), restrict the ratioUQ to 0.5..1.5 */    
      if ( ( ratioUQ > (1<<15)+(1<<14) ) || ( ratioUQ < (1<<15)-(1<<14) ) ) // this check ensures that the new value fits in 16-bit also.
      {
        ratioUQ = prevRatioUQ; 
      }
    }  
  }
  return (uint16_t)ratioUQ;           // return an UQ1.15 = [0..2)
}

// Add a host tick and a tmf8828 tick to the clock correction list and update ratioUQ
static void tmf8828ClockCorrectionAddPair ( tmf8828Driver * driver, uint32_t hostTick, uint32_t tmf8828Tick )
{
  if ( TMF8828_SYS_TICK_IS_VALID( tmf8828Tick ) )                             // only use ticks if tmf8828Tick has LSB set
  {
    uint8_t idx;
    uint8_t idx2;
    driver->clkCorrectionIdx = CLK_CORRECTION_IDX_MODULO( driver->clkCorrectionIdx + 1 );     // increment and take care of wrap-over
    driver->hostTicks[ driver->clkCorrectionIdx ] = hostTick;
    driver->tmf8828Ticks[ driver->clkCorrectionIdx ] = tmf8828Tick;
    /* check if we have enough entries to re-calculate the ratioUQ */
    idx = driver->clkCorrectionIdx;                                                 // last inserted
    idx2 = CLK_CORRECTION_IDX_MODULO( idx + CLK_CORRECTION_PAIRS - 1 );     // oldest available
    if ( TMF8828_SYS_TICK_IS_VALID( driver->tmf8828Ticks[ idx ] ) && TMF8828_SYS_TICK_IS_VALID( driver->tmf8828Ticks[ idx2 ] ) )    // only do a correction if both tmf8828 ticks are valid 
    { 
      uint32_t hDiff = driver->hostTicks[ idx ] - driver->hostTicks[ idx2 ];
      uint32_t tDiff = driver->tmf8828Ticks[ idx ] - driver->tmf8828Ticks[ idx2 ];
      driver->clkCorrRatioUQ = tmf8828CalcClkRatioUQ16( hDiff, tDiff, driver->clkCorrRatioUQ );
      if ( driver->logLevel & TMF8828_LOG_LEVEL_CLK_CORRECTION )
      {
        PRINT_STR( "ClkCorr ratio " );  
        PRINT_UINT( hDiff * TMF8828_TICKS_PER_US );
        PRINT_CHAR( ' ' );
        PRINT_UINT( tDiff * HOST_TICKS_PER_US );
        PRINT_CHAR( ' ' );
        PRINT_UINT( hDiff / HOST_TICKS_PER_US );
        PRINT_CHAR( ' ' );
        PRINT_UINT( tDiff / TMF8828_TICKS_PER_US );
        PRINT_CHAR( ' ' );
        PRINT_UINT( driver->clkCorrRatioUQ ); 
        PRINT_LN( );                              
      }                                             
    } /* else use last valid clock correction Ration UQ */
  }
  else if ( driver->logLevel & TMF8828_LOG_LEVEL_CLK_CORRECTION )
  {
    PRINT_STR( "ClkCorr ticks invalid " );      // this can happen if the host did read out the data very, very fast,
    PRINT_INT( tmf8828Tick );                   // and the device was busy handling other higher priority interrupts
    PRINT_LN( );                                // The device does always set the LSB of the sys-tick to indicate that
  }                                             // the device did set the sys-tick.      
}    

// execute command to load a given config page
static int8_t tmf8828LoadConfigPage ( tmf8828Driver * driver, uint8_t pageCmd )
{
  int8_t stat;
  dataBuffer[0] = pageCmd;
  i2cTxReg( driver, driver->i2cSlaveAddress, TMF8828_COM_CMD_STAT, 1, dataBuffer );                                                      // instruct device to load page
  stat = tmf8828CheckRegister( driver, TMF8828_COM_CMD_STAT, TMF8828_COM_CMD_STAT__stat_ok, 1, APP_CMD_LOAD_CONFIG_TIMEOUT_MS );  // check that load command is completed
  if ( stat == APP_SUCCESS_OK )
  {
    stat = tmf8828CheckRegister( driver, TMF8828_COM_CONFIG_RESULT, pageCmd, 1, APP_CMD_LOAD_CONFIG_TIMEOUT_MS );     // check that correct config page is loaded
  }
  return stat;
}

// Convert 4 bytes in little endian format into an uint32_t  
uint32_t tmf8828GetUint32 ( uint8_t * data ) 
{
  uint32_t t =    data[ 3 ];
  t = (t << 8 ) + data[ 2 ];
  t = (t << 8 ) + data[ 1 ];
  t = (t << 8 ) + data[ 0 ];
  return t;
}

// Function that executes command to write any configuration page
int8_t tmf8828WriteConfigPage ( tmf8828Driver * driver )
{
  int8_t stat;
  dataBuffer[0] = TMF8828_COM_CMD_STAT__cmd_write_config_page;
  i2cTxReg( driver, driver->i2cSlaveAddress, TMF8828_COM_CMD_STAT, 1, dataBuffer );                                                      // instruct device to load page
  stat = tmf8828CheckRegister( driver, TMF8828_COM_CMD_STAT, TMF8828_COM_CMD_STAT__stat_ok, 1, APP_CMD_WRITE_CONFIG_TIMEOUT_MS ); // check that write command is completed
  return stat;
}

// Function to load the common config page into I2C ram that the host can read/write it via I2C
int8_t tmf8828LoadConfigPageCommon ( tmf8828Driver * driver )
{
  return tmf8828LoadConfigPage( driver, TMF8828_COM_CMD_STAT__cmd_load_config_page_common );
}

// Function to load the factory calibration config page into I2C ram that the host can read/write it via I2C
int8_t tmf8828LoadConfigPageFactoryCalib ( tmf8828Driver * driver )
{
  return tmf8828LoadConfigPage( driver, TMF8828_COM_CMD_STAT__cmd_load_config_page_factory_calib );
}

// function to change the I2C address of the device
int8_t tmf8828ChangeI2CAddress ( tmf8828Driver * driver, uint8_t newI2cSlaveAddress )
{
  uint8_t oldAddr = driver->i2cSlaveAddress;
  int8_t stat = tmf8828LoadConfigPageCommon( driver );          // first load the page, then only overwrite the registers you want to change 
  if ( stat == APP_SUCCESS_OK )
  {
    dataBuffer[0] = newI2cSlaveAddress << 1;          // i2c slave address is shifted into the upper 7 bits of the 8-bit register
    i2cTxReg( driver, driver->i2cSlaveAddress, TMF8X2X_COM_I2C_SLAVE_ADDRESS, 1, dataBuffer );
    stat = tmf8828WriteConfigPage( driver );                 //  write the config page back
    if ( stat == APP_SUCCESS_OK )
    {
      dataBuffer[0] = TMF8828_COM_CMD_STAT__cmd_i2c_slave_address;
      i2cTxReg( driver, driver->i2cSlaveAddress, TMF8828_COM_CMD_STAT, 1, dataBuffer );      // instruct device to change i2c address
      driver->i2cSlaveAddress = newI2cSlaveAddress;                                 // from now on try to read from new address
      stat = tmf8828CheckRegister( driver, TMF8828_COM_CMD_STAT, TMF8828_COM_CMD_STAT__stat_ok, 1, APP_CMD_I2C_SLAVE_ADDRESS_TIMEOUT_MS ); // check that command ok
     }
  }
  if ( stat != APP_SUCCESS_OK )
  {
    driver->i2cSlaveAddress = oldAddr;                                            // switch failed, use old address again
    if ( driver->logLevel >=TMF8828_LOG_LEVEL_ERROR ) 
    {
      PRINT_STR( "#Err" );
      PRINT_CHAR( SEPARATOR );
      PRINT_STR( "I2C-Addr " );
      PRINT_INT( stat );
      PRINT_LN( );
    }
  }
  return stat; 
}

// internal configuraiton function
static int8_t tmf8828ConfigInternal( tmf8828Driver * driver, uint16_t periodInMs, uint16_t kiloIterations, uint8_t spadMapId, uint16_t lowThreshold, uint16_t highThreshold, uint8_t persistence, uint32_t intMask, uint8_t dumpHistogram )
{
  int8_t stat = APP_ERROR_PARAM;
  stat = tmf8828LoadConfigPageCommon( driver );          // first load the page, then only overwrite the registers you want to change 
  if ( stat == APP_SUCCESS_OK )
  {
    dataBuffer[0] = (uint8_t)periodInMs;            // lsb
    dataBuffer[1] = (uint8_t)(periodInMs>>8);       // msb
    dataBuffer[2] = (uint8_t)kiloIterations;        // lsb  - kilo iterations are right behind the period so we can write with one i2c tx
    dataBuffer[3] = (uint8_t)(kiloIterations>>8);   // msb
    dataBuffer[4] = (uint8_t)lowThreshold;          // lsb
    dataBuffer[5] = (uint8_t)(lowThreshold>>8);     // msb
    dataBuffer[6] = (uint8_t)highThreshold;         // lsb
    dataBuffer[7] = (uint8_t)(highThreshold>>8);    // msb
    dataBuffer[8] = (uint8_t)intMask;               // lsb
    dataBuffer[9] = (uint8_t)(intMask>>8);          // mid
    dataBuffer[10] = (uint8_t)(intMask>>16);        // msb
    dataBuffer[11] = persistence; 
    i2cTxReg( driver, driver->i2cSlaveAddress, TMF8828_COM_PERIOD_MS_LSB, 12, dataBuffer );
    dataBuffer[0] = spadMapId;                      // spad map ID is a different reg, so use a seperate i2c tx
    i2cTxReg( driver, driver->i2cSlaveAddress, TMF8828_COM_SPAD_MAP_ID, 1, dataBuffer );
    dataBuffer[0] = dumpHistogram & 0x3;            // only raw histograms and/or EC histograms
    i2cTxReg( driver, driver->i2cSlaveAddress, TMF8X2X_COM_HIST_DUMP, 1, dataBuffer );
    dataBuffer[0] = TMF8828_ENABLE_LOGARITHMIC_CONFIDENCE;                           
    i2cTxReg( driver, driver->i2cSlaveAddress, TMF8X2X_COM_ALG_SETTING_0, 1, dataBuffer );
    stat = tmf8828WriteConfigPage( driver );               // as a last step write the config page back
  }
  if ( stat != APP_SUCCESS_OK )
  {
    if ( driver->logLevel >=TMF8828_LOG_LEVEL_ERROR ) 
    {
      PRINT_STR( "#Err" );
      PRINT_CHAR( SEPARATOR );
      PRINT_STR( "Config " );
      PRINT_INT( stat );
      PRINT_LN( );
    }
  }
  return stat;
}

// configure device according to given parameters
int8_t tmf8828Configure ( tmf8828Driver * driver, uint16_t periodInMs, uint16_t kiloIterations, uint8_t spadMapId, uint16_t lowThreshold, uint16_t highThreshold, uint8_t persistence, uint32_t intMask, uint8_t dumpHistogram  )
{
  i2cRxReg( driver, driver->i2cSlaveAddress, TMF8828_COM_TMF8828_MODE, 1, dataBuffer );
  if ( dataBuffer[0] == TMF8828_COM_TMF8828_MODE__mode__TMF8828 )
  {
    spadMapId = TMF8828_COM_SPAD_MAP_ID__spad_map_id__map_no_15;	// 8x8 only can work with SPAD map 15, override it for convenience to 15 always
  }
  else if ( spadMapId > TMF8828_COM_SPAD_MAP_ID__spad_map_id__map_no_13 )   // no custom spad maps supported
  {
    return  APP_ERROR_PARAM;
  }
  return tmf8828ConfigInternal( driver, periodInMs, kiloIterations, spadMapId, lowThreshold, highThreshold, persistence, intMask, dumpHistogram );
}


// Function to reset the factory calibration. Call this function before providing the 4 factory
// calibration pages for tmf8828.
int8_t tmf8828ResetFactoryCalibration ( tmf8828Driver * driver )
{
  dataBuffer[0] = TMF8828_COM_CMD_STAT__cmd_stat__CMD_RESET_FACTORY_CALIBRATION;
  i2cTxReg( driver, driver->i2cSlaveAddress, TMF8828_COM_CMD_STAT, 1, dataBuffer );                                 
  return tmf8828CheckRegister( driver, TMF8828_COM_CMD_STAT, TMF8828_COM_CMD_STAT__stat_ok, 1, APP_CMD_WRITE_CONFIG_TIMEOUT_MS ); // check that command is done
}

// Function to execute a factory calibration
int8_t tmf8828FactoryCalibration ( tmf8828Driver * driver )
{
  int8_t status;
  dataBuffer[0] = TMF8828_COM_CMD_STAT__cmd_factory_calibration;
  i2cTxReg( driver, driver->i2cSlaveAddress, TMF8828_COM_CMD_STAT, 1, dataBuffer );   
  status = tmf8828CheckRegister( driver, TMF8828_COM_CMD_STAT, TMF8828_COM_CMD_STAT__stat_ok, 1, APP_CMD_FACTORY_CALIB_TIMEOUT_MS ); // check that factory calib command is done
  return status;
}

// Function to write a pre-stored calibration page
int8_t tmf8828SetStoredFactoryCalibration ( tmf8828Driver * driver, const uint8_t * calibPage )
{
  int8_t status = tmf8828LoadConfigPageFactoryCalib( driver );
  if ( APP_SUCCESS_OK == status )
  {
    dataBuffer[0] = readProgramMemoryByte( ( calibPage ) );
    if ( dataBuffer[0] == TMF8828_COM_CMD_STAT__cmd_load_config_page_factory_calib )
    {
      for ( uint8_t i = 1; i < TMF8828_COM_CONFIG_FACTORY_CALIB__factory_calibration_size; i++ )
      {
        dataBuffer[i] = readProgramMemoryByte( ( calibPage + i ) );
      }
      // actually write the calibration data
      i2cTxReg( driver, driver->i2cSlaveAddress, TMF8828_COM_CONFIG_RESULT, TMF8828_COM_CONFIG_FACTORY_CALIB__factory_calibration_size, dataBuffer );
      // issue the write page command
      return tmf8828WriteConfigPage( driver );
    }
    else
    {
      status = APP_ERROR_NO_CALIB_PAGE;
    }
  }
  return status;
}

// function starts a measurement
int8_t tmf8828StartMeasurement ( tmf8828Driver * driver ) 
{
  tmf8828ResetClockCorrection( driver );                                                                                         // clock correction only works during measurements
  dataBuffer[0] = TMF8828_COM_CMD_STAT__cmd_measure;
  i2cTxReg( driver, driver->i2cSlaveAddress, TMF8828_COM_CMD_STAT, 1, dataBuffer );                                                      // instruct device to load page
  return tmf8828CheckRegister( driver, TMF8828_COM_CMD_STAT, TMF8828_COM_CMD_STAT__stat_accepted, 1, APP_CMD_MEASURE_TIMEOUT_MS ); // check that measure command is accepted
}

// function stops a measurement
int8_t tmf8828StopMeasurement ( tmf8828Driver * driver ) 
{
  dataBuffer[0] = TMF8828_COM_CMD_STAT__cmd_stop;
  i2cTxReg( driver, driver->i2cSlaveAddress, TMF8828_COM_CMD_STAT, 1, dataBuffer );                                                      // instruct device to load page
  return tmf8828CheckRegister( driver, TMF8828_COM_CMD_STAT, TMF8828_COM_CMD_STAT__stat_ok, 1, APP_CMD_STOP_TIMEOUT_MS );         // check that stop command is accepted
}

// function to switch to one of the 2 modes
static int8_t tmf8828SwitchToMode ( tmf8828Driver * driver, uint8_t modeCmd, uint8_t mode )
{
  int8_t status;
  dataBuffer[0] = modeCmd;
  i2cTxReg( driver, driver->i2cSlaveAddress, TMF8828_COM_CMD_STAT, 1, dataBuffer );                                                      // instruct device to load page
  status = tmf8828CheckRegister( driver, TMF8828_COM_CMD_STAT, TMF8828_COM_CMD_STAT__stat_ok, 1, APP_CMD_SWITCH_MODE_CMD_TIMEOUT_MS );         // check that switch command is accepted
  if ( status == APP_SUCCESS_OK )
  {
    status = tmf8828CheckRegister( driver, TMF8828_COM_TMF8828_MODE, mode, 1, APP_CMD_SWITCH_MODE_TIMEOUT_MS );         
  }
  return status;
}

// function switches to 8x8 mode
int8_t tmf8828SwitchTo8x8Mode ( tmf8828Driver * driver )
{
  return tmf8828SwitchToMode( driver, TMF8828_COM_CMD_STAT__cmd_stat__CMD_SWITCH_TMF8828_MODE, TMF8828_COM_TMF8828_MODE__mode__TMF8828 );
}

// function switches to legacy mode
int8_t tmf8828SwitchToLegacyMode ( tmf8828Driver * driver )
{
  return tmf8828SwitchToMode( driver, TMF8828_COM_CMD_STAT__cmd_stat__CMD_SWITCH_TMF8821_MODE, TMF8828_COM_TMF8828_MODE__mode__TMF8821 );
}

// function reads and clears the specified interrupts
uint8_t tmf8828GetAndClrInterrupts ( tmf8828Driver * driver, uint8_t mask )
{
  uint8_t setInterrupts;
  dataBuffer[0] = 0;
  i2cRxReg( driver, driver->i2cSlaveAddress, TMF8828_INT_STATUS, 1, dataBuffer );            // read interrupt status register
  setInterrupts = dataBuffer[0] & mask;
  if ( setInterrupts )
  {
    dataBuffer[0] = dataBuffer[0] & mask;                             // clear only those that were set when we read the register, and only those we want to know
    i2cTxReg( driver, driver->i2cSlaveAddress, TMF8828_INT_STATUS, 1, dataBuffer );       // clear interrupts by pushing a 1 to status register
  }
  return setInterrupts;
}

// function clears and enables the specified interrupts
void tmf8828ClrAndEnableInterrupts ( tmf8828Driver * driver, uint8_t mask )
{
  dataBuffer[0] = 0xFF;                                               // clear all interrupts  
  i2cTxReg( driver, driver->i2cSlaveAddress, TMF8828_INT_STATUS, 1, dataBuffer );         // clear interrupts by pushing a 1 to status register
  dataBuffer[0] = 0;
  i2cRxReg( driver, driver->i2cSlaveAddress, TMF8828_INT_ENAB, 1, dataBuffer );            // read current enabled interrupts 
  dataBuffer[0] = dataBuffer[0] | mask;                             // enable those in the mask, keep the others if they were enabled
  i2cTxReg( driver, driver->i2cSlaveAddress, TMF8828_INT_ENAB, 1, dataBuffer );
}

// function disables the specified interrupts
void tmf8828DisableInterrupts ( tmf8828Driver * driver, uint8_t mask )
{
  dataBuffer[0] = 0;
  i2cRxReg( driver, driver->i2cSlaveAddress, TMF8828_INT_ENAB, 1, dataBuffer );            // read current enabled interrupts 
  dataBuffer[0] = dataBuffer[0] & ~mask;                            // clear only those in the mask, keep the others if they were enabled
  i2cTxReg( driver, driver->i2cSlaveAddress, TMF8828_INT_ENAB, 1, dataBuffer );
  dataBuffer[0] = mask; 
  i2cTxReg( driver, driver->i2cSlaveAddress, TMF8828_INT_STATUS, 1, dataBuffer );         // clear interrupts by pushing a 1 to status register
}

// function reads the result page (if there is none the function returns an error, else success)
int8_t tmf8828ReadResults ( tmf8828Driver * driver ) 
{
  uint32_t hTick;            // get the sys-tick just before the I2C rx
  dataBuffer[0] = 0;
  hTick = getSysTick( );            // get the sys-tick just before the I2C rx
  i2cRxReg( driver, driver->i2cSlaveAddress, TMF8828_COM_CONFIG_RESULT, TMF8828_COM_CONFIG_RESULT__measurement_result_size, dataBuffer );
  if ( dataBuffer[0] == TMF8828_COM_CONFIG_RESULT__measurement_result )
  {
    uint32_t tTick = tmf8828GetUint32( dataBuffer + RESULT_REG( SYS_TICK_0 ) );
    tmf8828ClockCorrectionAddPair( driver, hTick, tTick );
    printResults( driver, dataBuffer, TMF8828_COM_CONFIG_RESULT__measurement_result_size );
    return APP_SUCCESS_OK;
  }
  return APP_ERROR_NO_RESULT_PAGE;
}

// Correct the distance based on the clock correction pairs 
uint16_t tmf8828CorrectDistance ( tmf8828Driver * driver, uint16_t distance )
{
  if ( driver->clkCorrectionEnable )
  {
    uint32_t d = distance;
    d = ( driver->clkCorrRatioUQ * d + (1<<14) ) >> 15;
    distance = SATURATE16( d );
  }
  return distance;
}

// Function to read histograms and print them on UART. 
int8_t tmf8828ReadHistogram ( tmf8828Driver * driver )
{
  dataBuffer[0] = 0;
  i2cRxReg( driver, driver->i2cSlaveAddress, TMF8828_COM_CONFIG_RESULT, TMF8828_COM_HISTOGRAM_PACKET_SIZE, dataBuffer );
  if ( ( dataBuffer[0] & TMF8828_COM_OPTIONAL_SUBPACKET_HEADER_MASK ) == TMF8828_COM_OPTIONAL_SUBPACKET_HEADER_MASK ) // histograms must have MSB set
  {
    printHistogram( driver, dataBuffer, TMF8828_COM_HISTOGRAM_PACKET_SIZE );
    return APP_SUCCESS_OK;
  }
  return APP_ERROR_NO_RESULT_PAGE;
}
