#include "ADC.h"
#include <wiring_private.h>  // For pinPeripheral()

// Constructor to initialize the ADC pin
ADCModule::ADCModule(uint8_t p) {
    pin = p;
}

// Begin the ADC module (initialization function)
void ADCModule::begin() {
    // Configure the pin for analog input
    pinPeripheral(pin, PIO_ANALOG);

    // Enable the ADC and wait for it to be ready
    ADC->CTRLA.bit.ENABLE = 1;
    while (ADC->STATUS.bit.SYNCBUSY);
}

// Read the analog value from the ADC pin
uint16_t ADCModule::readValue() {
    // Select the input channel based on the pin
    ADC->INPUTCTRL.bit.MUXPOS = g_APinDescription[pin].ulADCChannelNumber;

    // Start the conversion
    ADC->SWTRIG.bit.START = 1;
    
    // Wait for the conversion to complete
    while (ADC->INTFLAG.bit.RESRDY == 0);

    // Return the result
    return ADC->RESULT.reg;
}