/*
 *****************************************************************************
 * Copyright by ams OSRAM AG                                                 *
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
//
// tmf8828 arduino uno sample program
//


// ---------------------------------------------- includes ----------------------------------------

#include "tmf8828_shim.h"
#include "tmf8828.h"
#include "tmf8828_app.h"
#include "tmf8828_calib.h"
#include "tmf882x_calib.h"
#include "tmf8828_image.h"
#include "tmf882x_image.h"


// ---------------------------------------------- defines -----------------------------------------



// tmf states
#define TMF8828_STATE_DISABLED      0
#define TMF8828_STATE_STANDBY       1     
#define TMF8828_STATE_STOPPED       2
#define TMF8828_STATE_MEASURE       3
#define TMF8828_STATE_ERROR         4


// number of log-levels in array
#define NR_LOG_LEVELS           7

// number of register that are printed in the dump on one line
#define NR_REGS_PER_LINE        8

// number of TMF8828 instances
#define NR_OF_TMF8828   1

// ---------------------------------------------- constants -----------------------------------------

// to increase/decrease logging
const uint8_t logLevels[ NR_LOG_LEVELS ] = 
{ TMF8828_LOG_LEVEL_NONE
, TMF8828_LOG_LEVEL_ERROR
, TMF8828_LOG_LEVEL_CLK_CORRECTION
, TMF8828_LOG_LEVEL_INFO
, TMF8828_LOG_LEVEL_VERBOSE
, TMF8828_LOG_LEVEL_I2C
, TMF8828_LOG_LEVEL_DEBUG
};

// first dimention of all configurations defines if it is for tmf882x or tmf8828
#define TMF882X_CONFIG_IDX  0
#define TMF8828_CONFIG_IDX  1

// for each configuration specifiy a period in milli-seconds
const uint16_t configPeriod[2][3] = 
{ {  33, 500, 100 }       // TMF882X config
, { 132, 264, 528 }       // TMF8828 config
};

// for each configuration specify the number of Kilo Iterations (Kilo = 1024)
const uint16_t configKiloIter[2][3] = 
{ { 537, 550,  900 }
, { 250, 500, 1000 }
};

// for each configuration select a SPAD map through the id
const uint8_t  configSpadId[2][3] = 
{ {TMF8828_COM_SPAD_MAP_ID__spad_map_id__map_no_1, TMF8828_COM_SPAD_MAP_ID__spad_map_id__map_no_2, TMF8828_COM_SPAD_MAP_ID__spad_map_id__map_no_7}
, {TMF8828_COM_SPAD_MAP_ID__spad_map_id__map_no_15, TMF8828_COM_SPAD_MAP_ID__spad_map_id__map_no_15, TMF8828_COM_SPAD_MAP_ID__spad_map_id__map_no_15} // TMF8828 can only have 1 mask
};

// set the lower threshold to 10cm
const uint16_t configLowThreshold = 100;
// set the upper threshold to 50cm
const uint16_t configHighThreshold = 500;
// select perstistence to be: 0==report every distance, even no distance; 1== report every distance that is a distance, 3== report distance only if 3x in range
const uint8_t configPersistance[3] = { 0, 1, 3 }; 
// interrupt selection mask is 18-bits, if bit is set, zone can report an interrupt
const uint32_t configInterruptMask = 0x3FFFF; 

// ---------------------------------------------- variables -----------------------------------------


tmf8828Driver tmf8828[NR_OF_TMF8828]; // instances of tmf8828
uint8_t logLevel;                 // how chatty the program is 
int8_t stateTmf8828;              // current state of the device 
int8_t modeIsTmf8828;             // if set to 1 this is the tmf8828 else this is the tmf882x
int8_t configNr;                  // this sample application has only a few configurations it will loop through, the variable keeps track of that
int8_t persistenceNr;             // this is to keep track of the selected persistence setting (out of three for this sample application)
int8_t clkCorrectionOn;           // if non-zero clock correction is on
int8_t dumpHistogramOn;           // if non-zero, dump all histograms
uint8_t logLevelIdx;              // log level indes into logLevels array
volatile uint8_t irqTriggered;    // interrupt is triggered or not

// ---------------------------------------------- function declaration ------------------------------

void printDeviceInfo();
void printHelp();
void printRegisters( uint8_t regAddr, uint16_t len, char seperator, uint8_t calibId );
void resetAppState();
void setMode();

// ---------------------------------------------- functions -----------------------------------------

// Switch I2C address.
void changeI2CAddress ( )
{
  if ( stateTmf8828 == TMF8828_STATE_STOPPED )
  {
    uint8_t newAddr = tmf8828[0].i2cSlaveAddress;
    if ( newAddr == TMF8828_SLAVE_ADDR )
    {
       newAddr = TMF8828_SLAVE_ADDR + 1;      // use next i2c slave address
    }
    else
    {
      newAddr = TMF8828_SLAVE_ADDR;         // back to original
    }
    if ( tmf8828ChangeI2CAddress( &(tmf8828[0]), newAddr ) != APP_SUCCESS_OK )
    {
      PRINT_CONST_STR( F(  "#Err" ) );
      PRINT_CHAR( SEPARATOR );    
    }
  }
  PRINT_CONST_STR( F(  "I2C Addr=" ) );
  PRINT_INT( tmf8828[0].i2cSlaveAddress );
  PRINT_LN( );
}

// enable/disable clock correction
void clockCorrection ( )
{
  clkCorrectionOn = !clkCorrectionOn;       // toggle clock correction on/off  
  tmf8828ClkCorrection( &(tmf8828[0]), clkCorrectionOn );
  PRINT_CONST_STR( F(  "Clk corr is " ) );
  PRINT_INT( clkCorrectionOn );
  PRINT_LN( );
}

// wrap through the available configurations and configure the device accordingly.
void configure ( )
{
  if ( tmf8828Configure( &(tmf8828[0]), configPeriod[modeIsTmf8828][configNr], configKiloIter[modeIsTmf8828][configNr], configSpadId[modeIsTmf8828][configNr], configLowThreshold, configHighThreshold, configPersistance[persistenceNr], configInterruptMask, dumpHistogramOn ) == APP_SUCCESS_OK )
  {
    PRINT_CONST_STR( F(  "#Conf" ) );
    PRINT_CHAR( SEPARATOR );
    PRINT_CONST_STR( F(  "Period=" ) );
    PRINT_INT( configPeriod[modeIsTmf8828][configNr] );
    PRINT_CONST_STR( F(  "ms" ) );
    PRINT_CHAR( SEPARATOR );
    PRINT_CONST_STR( F(  "KIter=" ) );
    PRINT_INT( configKiloIter[modeIsTmf8828][configNr] );
    PRINT_CONST_STR( F(  " SPAD=" ) );
    PRINT_INT( configSpadId[modeIsTmf8828][configNr] );
    PRINT_CONST_STR( F(  " Pers=" ) );
    PRINT_INT( configPersistance[persistenceNr] );
   }
  else
  {
    PRINT_CONST_STR( F(  "#Err" ) );
    PRINT_CHAR( SEPARATOR );
    PRINT_CONST_STR( F(  "Config" ) );
  }
  PRINT_LN( );
}

// enable device and download firmware
void enable ( uint32_t imageStartAddress, const unsigned char * image, int32_t imageSizeInBytes )
{
  if ( stateTmf8828 == TMF8828_STATE_DISABLED )
  {
    tmf8828Enable( &(tmf8828[0]) );
    delayInMicroseconds( ENABLE_TIME_MS * 1000 );
    tmf8828ClkCorrection( &(tmf8828[0]), clkCorrectionOn ); 
    tmf8828SetLogLevel( &(tmf8828[0]), logLevels[ logLevelIdx ] );
    tmf8828Wakeup( &(tmf8828[0]) );
    if ( tmf8828IsCpuReady( &(tmf8828[0]), CPU_READY_TIME_MS ) )
    {
      if ( tmf8828DownloadFirmware( &(tmf8828[0]), imageStartAddress, image, imageSizeInBytes ) == BL_SUCCESS_OK ) 
      {
        PRINT_CONST_STR( F( " DWNL" ) );
        PRINT_LN( );
        resetAppState();
        setMode( );
        configure();
        stateTmf8828 = TMF8828_STATE_STOPPED;
        printHelp(); // prints on UART usage and waits for user input on serial
        printDeviceInfo( );
      }
      else
      {
        stateTmf8828 = TMF8828_STATE_ERROR;
      }
    }
    else
    {
      stateTmf8828 = TMF8828_STATE_ERROR;
    }
  } // else device is already enabled
  else
  {
    tmf8828ReadDeviceInfo( &(tmf8828[0]) );
    printDeviceInfo( );
  }
}

// execute factory calibration in state stopped only
void factoryCalibration ( )
{
  if ( stateTmf8828 == TMF8828_STATE_STOPPED )
  {
    PRINT_CONST_STR( F(  "Fact Cal" ) );
    PRINT_LN( );
    tmf8828Configure( &(tmf8828[0]), 1, 4000, configSpadId[modeIsTmf8828][configNr], 0, 0xffff, 0, 0x3ffff, 0 );    // no histogram dumping in factory calibration allowed, 4M iterations for factory calibration recommended
    if ( modeIsTmf8828 )
    {
      tmf8828ResetFactoryCalibration( &(tmf8828[0]) );
      if (  APP_SUCCESS_OK == tmf8828FactoryCalibration( &(tmf8828[0]) ) // walk through all 4 calibration 
         && APP_SUCCESS_OK == tmf8828FactoryCalibration( &(tmf8828[0]) ) 
         && APP_SUCCESS_OK == tmf8828FactoryCalibration( &(tmf8828[0]) ) 
         && APP_SUCCESS_OK == tmf8828FactoryCalibration( &(tmf8828[0]) ) 
         )
      {
        configure( );
        return;
      }
    }
    else 
    {
      if ( APP_SUCCESS_OK == tmf8828FactoryCalibration( &(tmf8828[0]) ) ) 
      {
        configure( );
        return;
      }
    }
    PRINT_CONST_STR( F(  "#Err" ) );
    PRINT_CHAR( SEPARATOR );
    PRINT_CONST_STR( F(  "fact calib" ) );
    PRINT_LN( );
  }
}

// configure histogram dumping (next dumping bit-mask)
void histogramDumping ( )
{
  if ( stateTmf8828 == TMF8828_STATE_STOPPED )
  {
    dumpHistogramOn = dumpHistogramOn + 1;       // select histogram dump on/off, and type of histogram dumping
    if ( dumpHistogramOn > (TMF8828_COM_HIST_DUMP__histogram__electrical_calibration_24_bit_histogram + TMF8828_COM_HIST_DUMP__histogram__raw_24_bit_histogram) )
    {
      dumpHistogramOn = 0; // is off again
    }
    configure( );
    PRINT_CONST_STR( F(  "Histogram is " ) );
    PRINT_INT( dumpHistogramOn );
    PRINT_LN( );
  }
}


static const uint8_t * getPrecollectedFactoryCalibration ( uint8_t id )
{
  const uint8_t * factory_calib;
  if ( modeIsTmf8828 )      // tmf8828 has only 1 SPAD map, but needs 4 sets of calibraitond data for this 1 spad map
  {
    factory_calib = tmf8828_calib_0;
    if ( id == 1 ) 
    {
      factory_calib = tmf8828_calib_1;
    }
    else if ( id == 2 )
    {
      factory_calib = tmf8828_calib_2;
    }
    else if ( id == 3 )
    {
      factory_calib = tmf8828_calib_3;
    }
  }
  else    // tmf882x can have different SPAD maps, so need different calibration sets
  {
    factory_calib = tmf882x_calib_0;
    if ( configNr == 1) 
    {
      factory_calib = tmf882x_calib_1;
    }
    else if ( configNr == 2 )
    {
      factory_calib = tmf882x_calib_2;
    }    
  }
  return factory_calib;
}


// load factory calibration page to I2C registers 0x20...
void loadFactoryCalibration ( )
{
  if ( stateTmf8828 == TMF8828_STATE_STOPPED )
  {
    if ( modeIsTmf8828 )                                  // tmf8828 has 4 calibration pages 
    {
      tmf8828ResetFactoryCalibration( &(tmf8828[0]) );
      tmf8828LoadConfigPageFactoryCalib( &(tmf8828[0]) ); 
      printRegisters( 0x20, 0xE0-0x20, ',', 0 );  
      tmf8828WriteConfigPage( &(tmf8828[0]) );                // advance to next calib page
      tmf8828LoadConfigPageFactoryCalib( &(tmf8828[0]) );
      printRegisters( 0x20, 0xE0-0x20, ',', 1 );   
      tmf8828WriteConfigPage( &(tmf8828[0]) );               // advance to next calib page
      tmf8828LoadConfigPageFactoryCalib( &(tmf8828[0]) );
      printRegisters( 0x20, 0xE0-0x20, ',', 2 );  
      tmf8828WriteConfigPage( &(tmf8828[0]) );               // advance to next calib page
      tmf8828LoadConfigPageFactoryCalib( &(tmf8828[0]) );
      printRegisters( 0x20, 0xE0-0x20, ',', 3 );  
      tmf8828WriteConfigPage( &(tmf8828[0]) );                // advance to next calib page
    }
    else
    {
      tmf8828LoadConfigPageFactoryCalib( &(tmf8828[0]) ); 
      printRegisters( 0x20, 0xE0-0x20, ',', configNr );  
    }
  }
}

// decrease logging level
void logLevelDec ( )
{
  if ( logLevelIdx > 0 )
  {
    logLevelIdx--;
    tmf8828SetLogLevel( &(tmf8828[0]), logLevels[ logLevelIdx ] );
  }
  PRINT_CONST_STR( F(  "Log=" ) );
  PRINT_INT( logLevels[ logLevelIdx ] );
  PRINT_LN( );
}

// increase logging level
void logLevelInc ( )
{
  if ( logLevelIdx < NR_LOG_LEVELS - 1 )
  {
    logLevelIdx++;
    tmf8828SetLogLevel( &(tmf8828[0]), logLevels[ logLevelIdx ] );
  }
  PRINT_CONST_STR( F(  "Log=" ) );
  PRINT_INT( logLevels[ logLevelIdx ] );
  PRINT_LN( );
}

// start measurement
void measure ( )
{
  if ( stateTmf8828 == TMF8828_STATE_STOPPED )
  {
    tmf8828ClrAndEnableInterrupts( &(tmf8828[0]), TMF8828_APP_I2C_RESULT_IRQ_MASK | TMF8828_APP_I2C_RAW_HISTOGRAM_IRQ_MASK );
    tmf8828StartMeasurement( &(tmf8828[0]) );
    stateTmf8828 = TMF8828_STATE_MEASURE;
  }
}

// select the next configuration and configure
void nextConfiguration ( )
{
  if ( stateTmf8828 == TMF8828_STATE_STOPPED )
  {
    configNr = configNr + 1;
    if ( configNr > 2 )
    {
      configNr = 0;     // wrap around
    }
    configure( );
  }
}

// power down by setting PON=0
void powerDown ( )
{
  if ( stateTmf8828 == TMF8828_STATE_MEASURE )      // stop a measurement first
  {
    tmf8828StopMeasurement( &(tmf8828[0]) );
    tmf8828DisableInterrupts( &(tmf8828[0]), 0xFF );               // just disable all
    stateTmf8828 = TMF8828_STATE_STOPPED;
  }
  if ( stateTmf8828 == TMF8828_STATE_STOPPED )
  {
    tmf8828Standby( &(tmf8828[0]) );
    stateTmf8828 = TMF8828_STATE_STANDBY;
  }
}

// perform a hardware + software reset
void reset ( )
{
  if ( stateTmf8828 != TMF8828_STATE_DISABLED )
  {
    tmf8828Reset( &(tmf8828[0]) );
    PRINT_CONST_STR( F(  "Reset TMF8828" ) );
    PRINT_LN( );
    stateTmf8828 = TMF8828_STATE_STOPPED;
    setMode();        
  }
}

// restore factory calibration for file tmf8828_calib.c
void restoreFactoryCalibration ( )
{
  if ( stateTmf8828 == TMF8828_STATE_STOPPED )
  {
    if ( modeIsTmf8828 )
    {
      if (  APP_SUCCESS_OK == tmf8828ResetFactoryCalibration( &(tmf8828[0]) )                                             // First reset, then load all 4 calib pages
         && APP_SUCCESS_OK == tmf8828SetStoredFactoryCalibration( &(tmf8828[0]), getPrecollectedFactoryCalibration( 0 ) )
         && APP_SUCCESS_OK == tmf8828SetStoredFactoryCalibration( &(tmf8828[0]), getPrecollectedFactoryCalibration( 1 ) )
         && APP_SUCCESS_OK == tmf8828SetStoredFactoryCalibration( &(tmf8828[0]), getPrecollectedFactoryCalibration( 2 ) )
         && APP_SUCCESS_OK == tmf8828SetStoredFactoryCalibration( &(tmf8828[0]), getPrecollectedFactoryCalibration( 3 ) )
         )             
      {   
        PRINT_CONST_STR( F(  "Set fact cal" ) );
        PRINT_LN( );
        return;
      }
    }
    else if ( APP_SUCCESS_OK == tmf8828SetStoredFactoryCalibration( &(tmf8828[0]), getPrecollectedFactoryCalibration( configNr ) ) )
    {   
      PRINT_CONST_STR( F(  "Set fact cal" ) );
      PRINT_LN( );
      return;
    }
    PRINT_CONST_STR( F(  "#Err" ) );
    PRINT_CHAR( SEPARATOR );
    PRINT_CONST_STR( F(  "loadCal" ) );  
    PRINT_LN( );
  }
}

// set mode to tmf8828 or tmf882x
void setMode ( )
{
  int8_t res;
  if ( modeIsTmf8828 )
  {
    res = tmf8828SwitchTo8x8Mode( &(tmf8828[0]) );
  }
  else
  {
    res = tmf8828SwitchToLegacyMode( &(tmf8828[0]) );
  }
  if ( APP_SUCCESS_OK != res )
  {
    PRINT_CONST_STR( F(  "#Err" ) );
    PRINT_CHAR( SEPARATOR );
    PRINT_CONST_STR( F(  "mode switch to" ) );
    PRINT_CHAR( SEPARATOR );
    PRINT_INT( modeIsTmf8828 );
    PRINT_LN( );
    modeIsTmf8828 = 0;  // force back to tmf882x mode
  }
}

// execute a stop
void stop ( )
{
  if ( stateTmf8828 == TMF8828_STATE_MEASURE || stateTmf8828 == TMF8828_STATE_STOPPED )
  {
    tmf8828StopMeasurement( &(tmf8828[0]) );
    tmf8828DisableInterrupts( &(tmf8828[0]), 0xFF );               // just disable all
    stateTmf8828 = TMF8828_STATE_STOPPED;
  }
}

// set the thresholds to the next configuration
void thresholds ( )
{
  if ( stateTmf8828 == TMF8828_STATE_STOPPED )
  {
    persistenceNr = persistenceNr + 1;
    if ( persistenceNr > 2 )
    {
      persistenceNr = 0;     // wrap around
    }
    configure( );
  }
}

// wakeup sequence
void wakeup ( )
{
  if ( stateTmf8828 == TMF8828_STATE_STANDBY )
  {
    tmf8828Wakeup( &(tmf8828[0]) );
    if ( tmf8828IsCpuReady( &(tmf8828[0]), CPU_READY_TIME_MS ) )
    {
      stateTmf8828 = TMF8828_STATE_STOPPED;
    }
    else
    {
      stateTmf8828 = TMF8828_STATE_ERROR;
    }
  }
}


// Print the current state (stateTmf8828) in a readable format
void printState ( )
{
  if ( modeIsTmf8828 )
  {
    PRINT_CONST_STR( F(  "TMF8828" ) );
  }
  else
  {
    PRINT_CONST_STR( F(  "TMF882x" ) );
  }
  PRINT_CONST_STR( F(  " state=" ) );
  switch ( stateTmf8828 )
  {
    case TMF8828_STATE_DISABLED: PRINT_CONST_STR( F(  "disabled" ) ); break;
    case TMF8828_STATE_STANDBY: PRINT_CONST_STR( F(  "standby" ) ); break;
    case TMF8828_STATE_STOPPED: PRINT_CONST_STR( F(  "stopped" ) ); break;
    case TMF8828_STATE_MEASURE: PRINT_CONST_STR( F(  "measure" ) ); break;
    case TMF8828_STATE_ERROR: PRINT_CONST_STR( F(  "error" ) ); break;   
    default: PRINT_CONST_STR( F(  "???" ) ); break;
  }
  PRINT_LN( );
}

// print registers either as c-struct or plain
void printRegisters ( uint8_t regAddr, uint16_t len, char seperator, uint8_t calibId )
{
  if ( stateTmf8828 != TMF8828_STATE_DISABLED )
  {
    uint8_t buf[NR_REGS_PER_LINE];
    uint16_t i;
    uint8_t j;
    if ( seperator == ',' )
    {
      if ( modeIsTmf8828 )
      {
        PRINT_CONST_STR( F(  "const PROGMEM uint8_t tmf8828_calib_" ) );    // different name for tmf8828
      }
      else
      {
        PRINT_CONST_STR( F(  "const PROGMEM uint8_t tmf882x_calib_" ) );    // different name for tmf882x
      }
      PRINT_INT( calibId );
      PRINT_CONST_STR( F(  "[] = {" ) );
      PRINT_LN( );
    }
    for ( i = 0; i < len; i += NR_REGS_PER_LINE )            // if len is not a multiple of 8, we will print a bit more registers ....
    {
      uint8_t * ptr = buf;    
      i2cRxReg( &(tmf8828[0]), tmf8828[0].i2cSlaveAddress, regAddr, NR_REGS_PER_LINE, buf );
      if ( seperator == ' ' )
      {
        PRINT_CONST_STR( F(  "0x" ) );
        PRINT_UINT_HEX( regAddr );
        PRINT_CONST_STR( F(  ": " ) );
      }
      for ( j = 0; j < NR_REGS_PER_LINE; j++ )
      {
        PRINT_CONST_STR( F(  " 0x" ) ); PRINT_UINT_HEX( *ptr++ ); PRINT_CHAR( seperator ); 
      }
      PRINT_LN( );
      regAddr = regAddr + 8;
    }
    if ( seperator == ',' )
    {
      PRINT_CONST_STR( F(  "};" ) );
      PRINT_LN( );
    }
  }
}



// -------------------------------------------------------------------------------------------------------------

void printDeviceInfo ( )
{
  PRINT_CONST_STR( F(  "Driver " ) );
  PRINT_INT( tmf8828[0].info.version[0] ); PRINT_CHAR( '.' );
  PRINT_INT( tmf8828[0].info.version[1] ); 
  PRINT_CONST_STR( F(  " FW " ) );
  PRINT_INT( tmf8828[0].device.appVersion[0] ); PRINT_CHAR( '.' );
  PRINT_INT( tmf8828[0].device.appVersion[1] ); PRINT_CHAR( '.' );
  PRINT_INT( tmf8828[0].device.appVersion[2] ); PRINT_CHAR( '.' );
  PRINT_INT( tmf8828[0].device.appVersion[3] ); PRINT_CHAR( '.' );
  PRINT_CONST_STR( F(  " Chip " ) );
  PRINT_INT( tmf8828[0].device.chipVersion[0] ); PRINT_CHAR( '.' );
  PRINT_INT( tmf8828[0].device.chipVersion[1] ); 
  PRINT_CONST_STR( F(  " Serial " ) );
  PRINT_INT( tmf8828[0].device.deviceSerialNumber );
  PRINT_LN( );
}

// Function prints a help screen
void printHelp ( )
{
  PRINT_CONST_STR( F(  "TMF8828 Arduino Driver" ) );
  PRINT_LN( ); PRINT_CONST_STR( F(  "UART commands" ) );
  PRINT_LN( ); PRINT_CONST_STR( F(  "a ... dump registers" ) );
  PRINT_LN( ); PRINT_CONST_STR( F(  "c ... next configuration" ) );
  PRINT_LN( ); PRINT_CONST_STR( F(  "d ... disable device" ) );
  PRINT_LN( ); PRINT_CONST_STR( F(  "e ... enable device and download TMF8828 FW" ) );
  PRINT_LN( ); PRINT_CONST_STR( F(  "E ... enable device and download TMF8821 FW" ) );
  PRINT_LN( ); PRINT_CONST_STR( F(  "f ... do fact calib" ) );
  PRINT_LN( ); PRINT_CONST_STR( F(  "h ... help " ) );
  PRINT_LN( ); PRINT_CONST_STR( F(  "i ... i2c addr. change" ) );
  PRINT_LN( ); PRINT_CONST_STR( F(  "l ... load fact calib" ) );
  PRINT_LN( ); PRINT_CONST_STR( F(  "m ... measure" ) );
  PRINT_LN( ); PRINT_CONST_STR( F(  "o ... toggle between TMF8828 and TMF882X" ) );
  PRINT_LN( ); PRINT_CONST_STR( F(  "p ... power down" ) );
  PRINT_LN( ); PRINT_CONST_STR( F(  "r ... restore fact calib from file" ) );  
  PRINT_LN( ); PRINT_CONST_STR( F(  "s ... stop measure" ) );
  PRINT_LN( ); PRINT_CONST_STR( F(  "t ... next persistance set" ) );
  PRINT_LN( ); PRINT_CONST_STR( F(  "w ... wakeup" ) );
  PRINT_LN( ); PRINT_CONST_STR( F(  "x ... clock corr on/off" ) );
  PRINT_LN( ); PRINT_CONST_STR( F(  "z ... histogram" ) );
  PRINT_LN( ); PRINT_CONST_STR( F(  "+ ... log+" ) );
  PRINT_LN( ); PRINT_CONST_STR( F(  "- ... log-" ) );
  PRINT_LN( ); PRINT_CONST_STR( F(  "# ... reset" ) );
  PRINT_LN( ); 
}

// Function checks the UART for received characters and interprets them
int8_t serialInput ( )
{
  char rx;
  int8_t recv;
  do
  {
    recv = inputGetKey( &rx );
    if ( rx < 33 || rx >=126 ) // skip all control characters and DEL  
    {
      continue; // nothing to do here
    }
    else
    { 
      if ( rx == 'h' )
      {
        printHelp(); 
      }
      else if ( rx == 'c' ) // show and use next configuration
      {
        nextConfiguration( );
      }
      else if ( rx == 'e' ) // enable
      {  
        enable( tmf8828_image_start, tmf8828_image, tmf8828_image_length );
      }
      else if ( rx == 'E' ) // enable
      {  
        enable( tmf882x_image_start, tmf882x_image, tmf882x_image_length );
      }
      else if ( rx == 'd' )       // disable
      {  
        tmf8828Disable( &(tmf8828[0]) );
        stateTmf8828 = TMF8828_STATE_DISABLED;
      }
      else if ( rx == 'w' )       // wakeup
      {
        wakeup( );
      }
      else if ( rx == 'p' )       // power down
      {
        powerDown( );
      }
      else if ( rx == 'o' )
      { 
        modeIsTmf8828 = !modeIsTmf8828;
        setMode( );
      }
      else if ( rx == 'm' )
      {  
        measure( );
      }
      else if ( rx == 's' )
      {
        stop( );
      }
      else if ( rx == 'f' )
      {  
        factoryCalibration( );
      }
      else if ( rx == 'l' )
      {  
        loadFactoryCalibration( );
      }
      else if ( rx == 'r' )
      {
        restoreFactoryCalibration( );
      }
      else if ( rx == 'z' )
      {
        histogramDumping( );
      }
      else if ( rx == 'a' )
      {  
        if ( stateTmf8828 != TMF8828_STATE_DISABLED )
        {
          printRegisters( 0x00, 256, ' ', 0 );  
        }
      }
      else if ( rx == 'x' )
      {
        clockCorrection( );
      }
      else if ( rx == 'i' )
      {
          changeI2CAddress( );
        }
     else if ( rx == 't' ) // show and use next persistanc configuration
      {
        thresholds( );
      }
      else if ( rx == '+' ) // increase logging
      {
        logLevelInc( );
      }
      else if ( rx == '-' ) // decrease logging
      {
        logLevelDec( );
      }
      else if ( rx == '#' ) // reset chip to test the reset function itself
      {
        reset( );
      }
      else if ( rx == 'q' ) // terminate on device where this can be done
      {
        return 0; // terminate if possible
      }
      else 
      {
        PRINT_CONST_STR( F(  "#Err" ) );
        PRINT_CHAR( SEPARATOR );
        PRINT_CONST_STR( F(  "Cmd " ) );
        PRINT_CHAR( rx );
        PRINT_LN( );
      }
    }
    printState();
  } while ( recv );
  return 1;
}

// set target to defined configuration after enabling, needed by demo GUI
void resetAppState ( )
{
  stateTmf8828 = TMF8828_STATE_DISABLED;
  configNr = 0;        // rotate through the given configurations
  persistenceNr = 0;
  clkCorrectionOn = 1;
  dumpHistogramOn = 0; // default is off
  irqTriggered = 0;
  modeIsTmf8828 = 1;  // default is tmf8828
}

// interrupt handler is called when INT pin goes low
void interruptHandler ( void )
{
  irqTriggered = 1;
}

// -------------------------------------------------------------------------------------------------------------

// Arduino setup function is only called once at startup. Do all the HW initialisation stuff here.
void setupFn( uint8_t logLevelIdx, uint32_t baudrate, uint32_t i2cClockSpeedInHz )
{
  logLevel = logLevelIdx;                                            

  configurePins( &(tmf8828[0]) );

  // start serial and i2c
  inputOpen( baudrate );
  i2cOpen( &(tmf8828[0]), i2cClockSpeedInHz );

  resetAppState( );
  tmf8828Initialise( &(tmf8828[0]) );
  tmf8828SetLogLevel( &(tmf8828[0]), logLevels[ logLevelIdx ] );
  setInterruptHandler( interruptHandler );
  tmf8828Disable( &(tmf8828[0]) );                                     // this resets the I2C address in the device
  delayInMicroseconds(CAP_DISCHARGE_TIME_MS * 1000); // wait for a proper discharge of the cap
  printHelp();
}

// Arduino main loop function, is executed cyclic
int8_t loopFn ( )
{
  int8_t res = APP_SUCCESS_OK;
  uint8_t intStatus = 0;
  int8_t exit = serialInput();                                                            // handle any keystrokes from UART

#if ( defined( USE_INTERRUPT_TO_TRIGGER_READ ) && (USE_INTERRUPT_TO_TRIGGER_READ != 0) )
  if ( irqTriggered )
#else
  if ( /*stateTmf8828 == TMF8828_STATE_STOPPED ||*/ stateTmf8828 == TMF8828_STATE_MEASURE )
#endif
  { 
    disableInterrupts( );
    irqTriggered = 0;
    enableInterrupts( );
    intStatus = tmf8828GetAndClrInterrupts( &(tmf8828[0]), TMF8828_APP_I2C_RESULT_IRQ_MASK | TMF8828_APP_I2C_ANY_IRQ_MASK | TMF8828_APP_I2C_RAW_HISTOGRAM_IRQ_MASK );   // always clear also the ANY interrupt
    if ( intStatus & TMF8828_APP_I2C_RESULT_IRQ_MASK )                      // check if a result is available (ignore here the any interrupt)
    {
      res = tmf8828ReadResults( &(tmf8828[0]) );
    }
    if ( intStatus & TMF8828_APP_I2C_RAW_HISTOGRAM_IRQ_MASK )
    {
      res = tmf8828ReadHistogram( &(tmf8828[0]) );                                              // read a (partial) raw histogram
    }
  }

  if ( res != APP_SUCCESS_OK )                         // in case that fails there is some error in programming or on the device, this should not happen
  {
    tmf8828StopMeasurement( &(tmf8828[0]) );
    tmf8828DisableInterrupts( &(tmf8828[0]), 0xFF );
    stateTmf8828 = TMF8828_STATE_STOPPED;
    PRINT_CONST_STR( F(  "#Err" ) );
    PRINT_CHAR( SEPARATOR );
    PRINT_CONST_STR( F(  "inter" ) );
    PRINT_CHAR( SEPARATOR );
    PRINT_INT( intStatus );
    PRINT_CHAR( SEPARATOR );
    PRINT_CONST_STR( F(  "but no data" ) );
    PRINT_LN( );
  }
  return exit;
}


// Arduino has no terminate function but PC has.
void terminateFn ( )
{
  tmf8828Disable( &(tmf8828[0]) );
  clrInterruptHandler( );

  i2cClose( &(tmf8828[0]) );
  inputClose( );
}
