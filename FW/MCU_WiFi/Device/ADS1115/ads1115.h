#ifndef ADS1115_H_
#define ADS1115_H_

/******************************************************************************
 *      INCLUDES
 *****************************************************************************/

#include <stdint.h>
#include "driver/gpio.h"
#include "i2c.h"

/*****************************************************************************
 *      PUBLIC DEFINES
 *****************************************************************************/

#define ADS1115_VREF 4.096f // V

/*****************************************************************************
 *      I2C ADDRESS/BITS - Connect the following pin to ADDR
 *****************************************************************************/

#define ADS_ADDR_GND (0x48) ///< 1001 000 (ADDR -> GND)
#define ADS_ADDR_VDD (0x49) ///< 1001 001 (ADDR -> VDD)
#define ADS_ADDR_SDA (0x4A) ///< 1001 010 (ADDR -> SDA)
#define ADS_ADDR_SCL (0x4B) ///< 1001 011 (ADDR -> SCL)

/*****************************************************************************
 *      CONVERSION DELAY (in mS)
 *****************************************************************************/

#define ADS1115_CONVERSIONDELAY_1 (1) ///< Conversion delay
#define ADS1115_CONVERSIONDELAY_8 (8) ///< Conversion delay

/*****************************************************************************
 *      POINTER REGISTERS
 *****************************************************************************/

#define ADS1115_REG_POINTER_MASK      (0x03) ///< Point mask
#define ADS1115_REG_POINTER_CONVERT   (0x00) ///< Conversion
#define ADS1115_REG_POINTER_CONFIG    (0x01) ///< Configuration
#define ADS1115_REG_POINTER_LOWTHRESH (0x02) ///< Low threshold
#define ADS1115_REG_POINTER_HITHRESH  (0x03) ///< High threshold

/*****************************************************************************
 *      CONFIG REGISTERS
 *****************************************************************************/

#define ADS1115_REG_CONFIG_OS_MASK (0x8000) ///< OS Mask
#define ADS1115_REG_CONFIG_OS_SINGLE \
  (0x8000) ///< Write: Set to start a single-conversion
#define ADS1115_REG_CONFIG_OS_BUSY \
  (0x0000) ///< Read: Bit = 0 when conversion is in progress
#define ADS1115_REG_CONFIG_OS_NOTBUSY \
  (0x8000) ///< Read: Bit = 1 when device is not performing a conversion

#define ADS1115_REG_CONFIG_MUX_MASK (0x7000) ///< Mux Mask
#define ADS1115_REG_CONFIG_MUX_DIFF_0_1 \
  (0x0000) ///< Differential P = AIN0, N = AIN1 (default)
#define ADS1115_REG_CONFIG_MUX_DIFF_0_3 \
  (0x1000) ///< Differential P = AIN0, N = AIN3
#define ADS1115_REG_CONFIG_MUX_DIFF_1_3 \
  (0x2000) ///< Differential P = AIN1, N = AIN3
#define ADS1115_REG_CONFIG_MUX_DIFF_2_3 \
  (0x3000) ///< Differential P = AIN2, N = AIN3
#define ADS1115_REG_CONFIG_MUX_SINGLE_0 (0x4000) ///< Single-ended AIN0
#define ADS1115_REG_CONFIG_MUX_SINGLE_1 (0x5000) ///< Single-ended AIN1
#define ADS1115_REG_CONFIG_MUX_SINGLE_2 (0x6000) ///< Single-ended AIN2
#define ADS1115_REG_CONFIG_MUX_SINGLE_3 (0x7000) ///< Single-ended AIN3

#define ADS1115_REG_CONFIG_PGA_MASK   (0x0E00) ///< PGA Mask
#define ADS1115_REG_CONFIG_PGA_6_144V (0x0000) ///< +/-6.144V range = Gain 2/3
#define ADS1115_REG_CONFIG_PGA_4_096V (0x0200) ///< +/-4.096V range = Gain 1
#define ADS1115_REG_CONFIG_PGA_2_048V \
  (0x0400) ///< +/-2.048V range = Gain 2 (default)
#define ADS1115_REG_CONFIG_PGA_1_024V (0x0600) ///< +/-1.024V range = Gain 4
#define ADS1115_REG_CONFIG_PGA_0_512V (0x0800) ///< +/-0.512V range = Gain 8
#define ADS1115_REG_CONFIG_PGA_0_256V (0x0A00) ///< +/-0.256V range = Gain 16

#define ADS1115_REG_CONFIG_MODE_MASK   (0x0100) ///< Mode Mask
#define ADS1115_REG_CONFIG_MODE_CONTIN (0x0000) ///< Continuous conversion mode
#define ADS1115_REG_CONFIG_MODE_SINGLE \
  (0x0100) ///< Power-down single-shot mode (default)

#define ADS1115_REG_CONFIG_DR_MASK  (0x00E0) ///< Data Rate Mask
#define ADS1115_REG_CONFIG_DR_8SPS  (0x0000) ///< 8 samples per second
#define ADS1115_REG_CONFIG_DR_16SPS (0x0020) ///< 16 samples per second
#define ADS1115_REG_CONFIG_DR_32SPS (0x0040) ///< 32 samples per second
#define ADS1115_REG_CONFIG_DR_64SPS (0x0060) ///< 64 samples per second
#define ADS1115_REG_CONFIG_DR_128SPS \
  (0x0080) ///< 128 samples per second (default)
#define ADS1115_REG_CONFIG_DR_250SPS (0x00A0) ///< 250 samples per second
#define ADS1115_REG_CONFIG_DR_475SPS (0x00C0) ///< 475 samples per second
#define ADS1115_REG_CONFIG_DR_860SPS (0x00E0) ///< 860 samples per second (max)

#define ADS1115_REG_CONFIG_CMODE_MASK (0x0010) ///< CMode Mask
#define ADS1115_REG_CONFIG_CMODE_TRAD \
  (0x0000) ///< Traditional comparator with hysteresis (default)
#define ADS1115_REG_CONFIG_CMODE_WINDOW (0x0010) ///< Window comparator

#define ADS1115_REG_CONFIG_CPOL_MASK (0x0008) ///< CPol Mask
#define ADS1115_REG_CONFIG_CPOL_ACTVLOW \
  (0x0000) ///< ALERT/RDY pin is low when active (default)
#define ADS1115_REG_CONFIG_CPOL_ACTVHI \
  (0x0008) ///< ALERT/RDY pin is high when active

#define ADS1115_REG_CONFIG_CLAT_MASK \
  (0x0004) ///< Determines if ALERT/RDY pin latches once asserted
#define ADS1115_REG_CONFIG_CLAT_NONLAT \
  (0x0000) ///< Non-latching comparator (default)
#define ADS1115_REG_CONFIG_CLAT_LATCH (0x0004) ///< Latching comparator

#define ADS1115_REG_CONFIG_CQUE_MASK (0x0003) ///< CQue Mask
#define ADS1115_REG_CONFIG_CQUE_1CONV \
  (0x0000) ///< Assert ALERT/RDY after one conversions
#define ADS1115_REG_CONFIG_CQUE_2CONV \
  (0x0001) ///< Assert ALERT/RDY after two conversions
#define ADS1115_REG_CONFIG_CQUE_4CONV \
  (0x0002) ///< Assert ALERT/RDY after four conversions
#define ADS1115_REG_CONFIG_CQUE_NONE \
  (0x0003) ///< Disable the comparator and put ALERT/RDY in high state (default)

/*****************************************************************************
 *      PUBLIC TYPEDEFS
 *****************************************************************************/

typedef enum _ads1115_channel_t
{
  DEV_ADS1115_CHANNEL_0 = 0,
  DEV_ADS1115_CHANNEL_1,
  DEV_ADS1115_CHANNEL_2,
  DEV_ADS1115_CHANNEL_3
} ads1115_channel_t;

typedef enum ads1115_gain_t
{
  GAIN_TWOTHIRDS = ADS1115_REG_CONFIG_PGA_6_144V,
  GAIN_ONE       = ADS1115_REG_CONFIG_PGA_4_096V,
  GAIN_TWO       = ADS1115_REG_CONFIG_PGA_2_048V,
  GAIN_FOUR      = ADS1115_REG_CONFIG_PGA_1_024V,
  GAIN_EIGHT     = ADS1115_REG_CONFIG_PGA_0_512V,
  GAIN_SIXTEEN   = ADS1115_REG_CONFIG_PGA_0_256V
} ads1115_gain_t;

/*****************************************************************************
 *      PUBLIC FUNCTIONS
 *****************************************************************************/

/**
 * @brief Initializes the ADS1115 device.
 *
 * This function sets up the ADS1115 analog-to-digital converter (ADC) for use.
 * It configures the necessary settings and prepares the device for operation.
 * Call this function before using any other ADS1115 functions.
 */
void     DEV_ADS1115_Init(void);

/**
 * @brief Retrieves data from the ADS1115 ADC for the specified channel and gain.
 *
 * This function reads the analog-to-digital conversion result from the ADS1115
 * for the given channel and gain settings. The ADS1115 is a precision analog-to-digital
 * converter with a programmable gain amplifier.
 *
 * @param e_channel The ADS1115 channel to read from. This parameter is of type
 *                  ads1115_channel_t, which specifies the input multiplexer configuration.
 * @param e_gain    The gain setting for the ADS1115. This parameter is of type
 *                  ads1115_gain_t, which specifies the programmable gain amplifier setting.
 *
 * @return The 16-bit digital value representing the analog input voltage for the specified
 *         channel and gain settings.
 */
uint16_t DEV_ADS1115_GetData(ads1115_channel_t e_channel,
                             ads1115_gain_t    e_gain);

/**
 * @brief Reads the voltage from the specified ADS1115 channel.
 *
 * This function reads the voltage from the ADS1115 analog-to-digital converter
 * on the specified channel with the given gain setting.
 *
 * @param e_channel The ADS1115 channel to read from. This is of type 
 *                  `ads1115_channel_t` which specifies the input multiplexer 
 *                  configuration.
 * @param e_gain The gain setting for the ADS1115. This is of type 
 *               `ads1115_gain_t` which specifies the programmable gain 
 *               amplifier (PGA) setting.
 * @return The voltage read from the specified channel in volts.
 */
float    DEV_ADS1115_ReadVoltage(ads1115_channel_t e_channel,
                                 ads1115_gain_t    e_gain);

/**
 * @brief Converts the raw ADC value to a voltage.
 *
 * This function takes a 16-bit raw ADC value and converts it to the corresponding voltage.
 * The conversion is based on the reference voltage and the resolution of the ADS1115 ADC.
 *
 * @param u16_value_adc The raw 16-bit ADC value to be converted.
 * @return The corresponding voltage as a float.
 */
float    DEV_ADS1115_GetVoltage(uint16_t u16_value_adc);

#endif /* ADS1115_H_ */