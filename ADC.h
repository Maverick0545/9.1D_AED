#ifndef ADC_H
#define ADC_H

#include <Arduino.h>

class ADCModule {
    uint8_t pin;  // Pin number for ADC
public:
    // Constructor to initialize the ADC pin
    ADCModule(uint8_t p);
    
    // Begin the ADC module (can include setup here if needed)
    void begin();
    
    // Read the analog value from the ADC pin
    uint16_t readValue();
};

#endif