# -*- coding: utf-8 -*-
"""
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
 
This is a simple python example that talks to my arduino uno program for an tmf882x
that shows factory calibration and reads out the crosstalk in a nicer format.

It also normalizes the crosstalk to 550K iterations to match it with the optical
design guide for the tmf8820/1.

    
Factory calibration by the arduino uno is done with 4M iterations, so we need to normalize
the crosstalk to 550K like in the optical design guide. Note that the optical design guide
gives crosstalk limits for different SPAD maps.
"""

# Importing Libraries
import serial
import time


def write(x):
    """
    Write a single byte to the arduino
    Args:
        x (character): Must be one of the commands the arduino program understands, h,e,d,w,p,m,s,c,f,l,a,x.
    """
    arduino.write(bytes(x, 'utf-8'))
    time.sleep(0.05)

def read():
    """
    Read a single line from the serial port. It is from the arduino.
    Returns:
        data (bytes): the read line from the arduino.
    """
    data = arduino.readline()
    if ( len(data) > 0 ):
        if ( data[0] != b'#'[0] ):
            print( data )
    return data

def waitForArduinoStartTalk():
    """
    Wait until Arduino starts talking.    
    Returns:
        data (bytes): the read line from the arduino.
    """
    data = b''                                              # make an empty list
    while ( len(data) == 0 ):                               # wait for the device to start talking == download completed
        data = read()
    return data

def waitForArduinoStopTalk():    
    """
    Wait until Arduino stops talking.    
    """
    data = b'0'                                             # make a non-empty list
    while ( len(data) > 0 ):                                # wait for the device to stop talking == help screen printed
        data = read()

def calibDecode():
    crosstalk = [ 0 for i in range(20)]                     # 10 channels, but ignore channel 0 for now    
    address = 0x20                                          
    status = -1
    calibData = ""       
    while ( address < 0xE0 ):
        data = waitForArduinoStartTalk()
        data = data.decode('utf-8')
        calibData = calibData + data 
        row = data.split(',')
        if ( len( row ) >= 8 ):
            if ( address == 0x28 ):                        # Kilo-Iterations used for factory calibration are at 0x2a..0x2b
                kiloIterations = int(row[2],16) + int(row[3],16)*256
            
            if ( address >= 0x60 and address <= 0x80 ):    # channel 1 = 0x60..0x63, channel 9 = 0x80..0x83
                channel = 1 + ( address - 0x60 )//4        # crosstalk is a 4 byte value in little endian
                crosstalk[ channel ] = int(row[0],16) + int(row[1],16)*256 + int(row[2],16)*256*256 + int(row[3],16)*256*256*256
                if ( address < 0x80 ):
                    channel = 1 + channel                      # crosstalk is a 4 byte value in little endian
                    crosstalk[ channel ] = int(row[4],16) + int(row[5],16)*256 + int(row[6],16)*256*256 + int(row[7],16)*256*256*256

            if ( address >= 0xB8 and address <= 0xD8 ):    # channel 11 = 0xB8..0xBB, channel 19 = 0xD0..0xD3
                channel = 11 + ( address - 0xB8 )//4       # crosstalk is a 4 byte value in little endian
                crosstalk[ channel ] = int(row[0],16) + int(row[1],16)*256 + int(row[2],16)*256*256 + int(row[3],16)*256*256*256
                if ( address < 0xD8 ):
                    channel = 1 + channel                  # crosstalk is a 4 byte value in little endian
                    crosstalk[ channel ] = int(row[4],16) + int(row[5],16)*256 + int(row[6],16)*256*256 + int(row[7],16)*256*256*256

            if ( address >= 0xD8 and address <= 0xDF):     # last 4 bytes 
                status = int(row[4],16)

            address = address + 8                           # 8 values consumed
    if ( kiloIterations == 0 ):
        print( "Error no KiloIterations found in the factory calibration page")
        kiloIterations = 1                                  # avoid div-by-0
    print( "Factory calibration was done with {} KiloIterations".format( kiloIterations ))
    
    print( "Crosstalk values: ")
    for i in range(20):
        print( "channel_{} = {}, normalized to 550K iterations = {:.0f} ".format(i,crosstalk[i], (crosstalk[i])//(kiloIterations/550)))
    if ( status == 0 ):
        print( "Factory calibration success" )
    else:
        print( "Factory calibration failed with status={}".format(status) )
        
    data = waitForArduinoStartTalk()                        # add closing } and ;
    data = data.decode('utf-8')
    calibData = calibData + data        
    return calibData

if ( True ):
    number_of_results = 3
    #  get arduino    
    arduino = serial.Serial(port='COM27', baudrate=115200, timeout=0.1)

    # give arduinto time to wakeup
    while ( True ):
        data = read()
        if ( len(data) > 0 ):
            print( "started receiving ... ")
            break                                           # leave waiting loop
        else:
            time.sleep( 1.0 )
            write( 'h' )                                    # try to get device talking
            
    waitForArduinoStopTalk()                                # wait until arduino app stops talking
    print( "Now we can instruct the device to do something ")

    write( 'd' )                                            # disable device for a clean start
    waitForArduinoStopTalk()

    print( "Enable device and download fw ----------------------- ")
    write( 'e' )                                            # enable device
    waitForArduinoStartTalk()                               # wait for the device to start talking
    waitForArduinoStopTalk()                                # wait for the device to stop talking

    # in this example we choose config 2 (instead of default config 0 )
    write( 'c' )                                            # switch to next config
    waitForArduinoStartTalk()                               # wait for the device to start talking
    waitForArduinoStopTalk()                                # wait for the device to stop talking

    write( 'c' )                                            # switch to next config = time-multiplexed one
    waitForArduinoStartTalk()                               # wait for the device to start talking
    waitForArduinoStopTalk()                                # wait for the device to stop talking

    print( "Now do factory calibration ")
    write( 'f' )                                            # start factory calibration
    waitForArduinoStartTalk()                               # wait for the device to start talking (=print state)
    waitForArduinoStopTalk()                                # wait for the device to stop talking
    
    print( "Load factory calibration")
    write( 'l' )                                            # laod factory calibration page
    calibData0 = calibDecode( )
    calibData1 = calibDecode( )
    calibData2 = calibDecode( )
    calibData3 = calibDecode( )
    waitForArduinoStopTalk()                                # wait for the device to stop talking

    print( "CALIBRATION DATA" )
    print( calibData0 )
    print( calibData1 )
    print( calibData2 )
    print( calibData3 )

    print( "Now start measurements ")
    write( 'm' )                                            # start measurements
    records = 0
    while ( records < number_of_results ):                  
        data = read()                                       # wait for the device to start talking
        if ( len(data) > 0 and data[0] == b'#'[0] ):
            data = data.decode('utf-8')
            data = data.replace('\r','')                    # remove unwanted \r and \n to not have them 
            data = data.replace('\n','')                    # in the CSV file as unwanted chars
            row = data.split(',')
            records = records + 1
            print( "Result {} received".format( records ))
    
    write( 's' )                                            # stop measurements
    waitForArduinoStopTalk()                                # wait for the device to stop talking
    
    print( "Received {} result records".format(number_of_results) )
    
    print( "Dumping registers" )
    write( 'a' )                                             # dump all registers
    waitForArduinoStartTalk()                               # wait for the device to start talking (=print state)
    waitForArduinoStopTalk()                                # wait for the device to stop talking

    write( 'd' )                                            # disable device
    waitForArduinoStopTalk()                                # wait for the device to stop talking

    arduino.close()                                         # close serial port
    print( "End of program")
    