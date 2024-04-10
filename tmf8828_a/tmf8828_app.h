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

/** @file This is the tmf8828 arduino uno driver example console application. 
 */


#ifndef TMF8828_APP_H
#define TMF8828_APP_H

// ---------------------------------------------- includes ----------------------------------------

#include "tmf8828_shim.h"


// ---------------------------------------------- functions ---------------------------------------

/** @brief Arduino setup function is only called once at startup. Do all the HW initialisation stuff here.
 * @param logLevelIdx ...  the log level index to be used (0..7 -> see logLevels array in tmf8828_app.cpp)
 * @param  baudrate ... for the serial input the baudrate
 * @param  i2cClockSpeedInHz ... the i2c frequency
 */
void setupFn( uint8_t logLevelIdx, uint32_t baudrate, uint32_t i2cClockSpeedInHz );

/** @brief Arduino main loop function, is executed cyclic
 * @return 1 if wants to be called again
 * @return 0 if program should terminate
 */
int8_t loopFn( );

/** @brief Arduino terminate function is only called once when exit key 'q' is pressed. Write a message and wait for shutdown of arduino.
 */
void terminateFn( );

#endif // TMF8828_APP_H
