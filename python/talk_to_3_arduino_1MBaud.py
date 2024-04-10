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

"""

# Importing Libraries
import csv
import serial
import time

COM_PORT='COM3'

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
       
if ( True ):
    number_of_results = 1000
    CSV_FILE = "./tmf8828_arduino_uno_3_file.csv"
    f = open( CSV_FILE, 'w', encoding='UTF8', newline='' )
    f.write( "sep=,\n")    
    f.write( "Tmf8828 Arduino Uno + 2 tmf8828 \n");

    #  get arduino    
    arduino = serial.Serial(port=COM_PORT, baudrate=1000000, timeout=0.1)

    #dump csv
    csvout = csv.writer( f, delimiter=',')  

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
       
    print( "Now start measurements ")
    write( 'm' )                                            # start measurements
    records = 0
    while ( records < number_of_results ):                  
        data = read()                                       # wait for the device to start talking
        if ( len(data) > 1 and data[0] == b'#'[0] ):
            data = data.decode('utf-8')
            data = data.replace('\r','')                    # remove unwanted \r and \n to not have them 
            data = data.replace('\n','')                    # in the CSV file as unwanted chars
            row = data.split(',')
            csvout.writerow( row )
            if ( row[0] == "#Obj" ):                       # dump all strings that start with # but only count objects
                records = records + 1
                print( "Result {} received".format( records ))
    
    write( 's' )                                            # stop measurements
    waitForArduinoStopTalk()                                # wait for the device to stop talking
    
    print( "Received {} result records".format(number_of_results) )
    
    write( 'd' )                                            # disable device
    waitForArduinoStopTalk()                                # wait for the device to stop talking

    f.close()                                               # close file
    arduino.close()                                         # close serial port
    print( "End of program")
    