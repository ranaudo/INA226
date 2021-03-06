/*******************************************************************************************************************
** Class definition header for the INA226 class. This library allows access to the INA226 High-Side or Low-Side   **
** Measurement, Bi-Directional Current and Power Monitor with I2C Compatible Interface. The datasheet can be      **
** download from Texas Instruments at http://www.ti.com/lit/ds/symlink/ina226.pdf. While there are breakout boards**
** for the little brother INA219 along with sample libraries, I had a need for a device that would take over 28V  **
** and found that this chip could not only handle the higher voltage but was also significantly more accurate.    **
**                                                                                                                **
** Detailed documentation can be found on the GitHub Wiki pages at https://github.com/SV-Zanshin/INA226/wiki      **
**                                                                                                                **
** The INA226 requires an external shunt of known resistance to be placed across the high-side or low-side supply **
** or ground line and it uses the small current generated by the shunt to compute the amperage going through the  **
** circuit.  This value, coupled with the voltage measurement, allows the Amperage and Wattage to be computed by  **
** the INA226 and all of these values can be read using the industry standard I2C protocol.                       **
**                                                                                                                **
** Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated   **
** documentation files (the "Software"), to deal in the Software without restriction, including without limitation**
** the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,   **
** and to permit persons to whom the Software is furnished to do so, subject to the following conditions:         **
** The above copyright notice and this permission notice shall be included in all copies or substantial portions  **
** of the Software.                                                                                               **
**                                                                                                                **
** Although programming for the Arduino and in c/c++ is new to me, I'm a professional programmer and have learned,**
** over the years, that it is much easier to ignore superfluous comments than it is to decipher non-existent ones;**
** so both my comments and variable names tend to be verbose. The code is written to fit in the first 80 spaces   **
** and the comments start after that and go to column 117 - allowing the code to be printed in A4 landscape mode. **
**                                                                                                                **
** This program is free software: you can redistribute it and/or modify it under the terms of the GNU General     **
** Public License as published by the Free Software Foundation, either version 3 of the License, or (at your      **
** option) any later version. This program is distributed in the hope that it will be useful, but WITHOUT ANY     **
** WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the   **
** GNU General Public License for more details. You should have received a copy of the GNU General Public License **
** along with this program.  If not, see <http://www.gnu.org/licenses/>.                                          **
**                                                                                                                **
** Vers.  Date       Developer                     Comments                                                       **
** ====== ========== ============================= ============================================================== **
** 1.0.7  2018-06-08 https://github.com/SV-Zanshin https://github.com/SV-Zanshin/INA226/issues/14. Missing calls  **
**                                                 EEPROM.Get() for device number caused errors sporadic errors   **
** 1.0.6  2018-06-01 https://github.com/SV-Zanshin https://github.com/SV-Zanshin/INA226/issues/12. Add getMode()  **
** 1.0.6  2018-05-29 https://github.com/SV-Zanshin https://github.com/SV-Zanshin/INA226/issues/10. Limit Scan addr**
** 1.0.5  2017-09-24 https://github.com/SV-Zanshin https://github.com/SV-Zanshin/INA226/issues/6. Multiple INA226 **
** 1.0.5b 2017-09-23 https://github.com/SV-Zanshin https://github.com/SV-Zanshin/INA226/issues/6. Multiple INA226 **
** 1.0.5a 2017-09-18 https://github.com/SV-Zanshin https://github.com/SV-Zanshin/INA226/issues/6. Multiple INA226 **
** 1.0.4  2017-08-13 https://github.com/SV-Zanshin Enhancement #5, removed while() loop after Wire.requestFrom()  **
** 1.0.3  2017-08-09 https://github.com/SV-Zanshin Fix https://github.com/SV-Zanshin/INA226/issues/4. Overflows   **
**                                                 in computations of begin() and getShuntMicroVolts() functions. **
** 1.0.2  2017-07-30 https://github.com/SV-Zanshin Optional parameter default values only in function prototypes  **
** 1.0.1  2017-05-26 https://github.com/SV-Zanshin Changed _CurrentLSB from uint16_t to uint32_t due to overflow  **
** 1.0.0  2017-01-10 https://github.com/SV-Zanshin Fixed library file name, added constants for setMode() call    **
** 1.0.0  2017-01-09 https://github.com/SV-Zanshin Added reset() and setMode() calls                              **
** 1.0.b2 2017-01-08 https://github.com/SV-Zanshin Removed INA219 code, concentrating on only the INA226          **
** 1.0.b1 2017-01-05 https://github.com/SV-Zanshin Created class                                                  **
**                                                                                                                **
*******************************************************************************************************************/
#include "Arduino.h"                                                          // Arduino data type definitions    //
#ifndef INA226_Class_h                                                        // Guard code definition            //
  #define debug_Mode                                                          // Comment out when not needed      //
  #define INA226__Class_h                                                     // Define the name inside guard code//
  /*****************************************************************************************************************
  ** Declare structures used in the class                                                                         **
  *****************************************************************************************************************/
  typedef struct {                                                            // Structure of values per device   //
    uint8_t  address;                                                         // I2C Address of device            //
    uint16_t calibration;                                                     // Calibration register value       //
    uint32_t current_LSB;                                                     // Amperage LSB                     //
    uint32_t power_LSB;                                                       // Wattage LSB                      //
    uint8_t  operatingMode;                                                   // Default continuous mode operation//
  } inaDet; // of structure                                                   //                                  //
  /*****************************************************************************************************************
  ** Declare constants used in the class                                                                          **
  *****************************************************************************************************************/
  const uint8_t  I2C_DELAY                    =     10;                       // Microsecond delay on write       //
  const uint8_t  INA_CONFIGURATION_REGISTER   =      0;                       // Registers common to all INAs     //
  const uint8_t  INA_SHUNT_VOLTAGE_REGISTER   =      1;                       //                                  //
  const uint8_t  INA_BUS_VOLTAGE_REGISTER     =      2;                       //                                  //
  const uint8_t  INA_POWER_REGISTER           =      3;                       //                                  //
  const uint8_t  INA_CURRENT_REGISTER         =      4;                       //                                  //
  const uint8_t  INA_CALIBRATION_REGISTER     =      5;                       //                                  //
  const uint8_t  INA_MASK_ENABLE_REGISTER     =      6;                       //                                  //
  const uint8_t  INA_MANUFACTURER_ID_REGISTER =   0xFE;                       //                                  //
  const uint16_t INA_RESET_DEVICE             = 0x8000;                       // Write to configuration to reset  //
  const uint16_t INA_DEFAULT_CONFIGURATION    = 0x4127;                       // Default configuration register   //
  const uint16_t INA_BUS_VOLTAGE_LSB          =    125;                       // LSB in uV *100 1.25mV            //
  const uint16_t INA_SHUNT_VOLTAGE_LSB        =     25;                       // LSB in uV *10  2.5uV             //
  const uint16_t INA_CONFIG_AVG_MASK          = 0x0E00;                       // Bits 9-11                        //
  const uint16_t INA_CONFIG_BUS_TIME_MASK     = 0x01C0;                       // Bits 6-8                         //
  const uint16_t INA_CONFIG_SHUNT_TIME_MASK   = 0x0038;                       // Bits 3-5                         //
  const uint16_t INA_CONVERSION_READY_MASK    = 0x0080;                       // Bit 4                            //
  const uint16_t INA_CONFIG_MODE_MASK         = 0x0007;                       // Bits 0-3                         //
  const uint8_t  INA_MODE_TRIGGERED_SHUNT     =   B001;                       // Triggered shunt, no bus          //
  const uint8_t  INA_MODE_TRIGGERED_BUS       =   B010;                       // Triggered bus, no shunt          //
  const uint8_t  INA_MODE_TRIGGERED_BOTH      =   B011;                       // Triggered bus and shunt          //
  const uint8_t  INA_MODE_POWER_DOWN          =   B100;                       // shutdown or power-down           //
  const uint8_t  INA_MODE_CONTINUOUS_SHUNT    =   B101;                       // Continuous shunt, no bus         //
  const uint8_t  INA_MODE_CONTINUOUS_BUS      =   B110;                       // Continuous bus, no shunt         //
  const uint8_t  INA_MODE_CONTINUOUS_BOTH     =   B111;                       // Both continuous, default value   //
  /*****************************************************************************************************************
  ** Declare class header                                                                                         **
  *****************************************************************************************************************/
  class INA226_Class {                                                        // Class definition                 //
    public:                                                                   // Publicly visible methods         //
      INA226_Class();                                                         // Class constructor                //
      ~INA226_Class();                                                        // Class destructor                 //
      uint8_t  begin(const uint8_t  maxBusAmps,                               // Class initializer                //
                     const uint32_t microOhmR,                                //                                  //
                     const uint8_t  deviceNumber = UINT8_MAX );               //                                  //
      uint16_t getBusMilliVolts(const bool waitSwitch=false,                  // Retrieve Bus voltage in mV       //
                                const uint8_t deviceNumber=0);                //                                  //
      int16_t  getShuntMicroVolts(const bool waitSwitch=false,                // Retrieve Shunt voltage in uV     //
                                  const uint8_t deviceNumber=0);              //                                  //
      int32_t  getBusMicroAmps(const uint8_t deviceNumber=0);                 // Retrieve micro-amps              //
      int32_t  getBusMicroWatts(const uint8_t deviceNumber=0);                // Retrieve micro-watts             //
      void     reset(const uint8_t deviceNumber=0);                           // Reset the device                 //
      void     setMode(const uint8_t mode,const uint8_t devNumber=UINT8_MAX); // Set the monitoring mode          //
      uint8_t  getMode(const uint8_t devNumber=UINT8_MAX);                    // Get the monitoring mode          //
      void     setAveraging(const uint16_t averages,                          // Set the number of averages taken //
                            const uint8_t deviceNumber=UINT8_MAX);            //                                  //
      void     setBusConversion(uint8_t convTime,                             // Set timing for Bus conversions   //
                                const uint8_t deviceNumber=UINT8_MAX);        //                                  //
      void     setShuntConversion(uint8_t convTime,                           // Set timing for Shunt conversions //
                                  const uint8_t deviceNumber=UINT8_MAX);      //                                  //
      void     waitForConversion(const uint8_t deviceNumber=UINT8_MAX);       // wait for conversion to complete  //
      void     setAlertPinOnConversion(const bool alertState,                 // Enable pin change on conversion  //
                                       const uint8_t deviceNumber=UINT8_MAX); //                                  //
    private:                                                                  // Private variables and methods    //
      uint8_t  readByte(const uint8_t addr, const uint8_t deviceAddress);     // Read a byte from an I2C address  //
      int16_t  readWord(const uint8_t addr, const uint8_t deviceAddress);     // Read a word from an I2C address  //
      void     writeByte(const uint8_t addr, const uint8_t data,              // Write a byte to an I2C address   //
                         const uint8_t deviceAddress);                        //                                  //
      void     writeWord(const uint8_t addr, const uint16_t data,             // Write two bytes to an I2C address//
                         const uint8_t deviceAddress);                        //                                  //
      uint8_t  _TransmissionStatus = 0;                                       // Return code for I2C transmission //
      uint8_t  _DeviceCount        = 0;                                       // Number of INA226s detected       //
  }; // of INA226_Class definition                                            //                                  //
#endif                                                                        //----------------------------------//
