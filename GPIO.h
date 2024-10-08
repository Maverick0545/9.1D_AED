#ifndef GPIO_H
#define GPIO_H

#include <Arduino.h>

class GPIO_DEAKIN {
    uint8_t pin;  // Pin number associated with the GPIO
    PortGroup* portGroup;  // Port group (PORT->Group[0] for PORTA, etc.)
    uint32_t pinMask;      // Mask for pin position in the port group

public:
    // Constructor to initialize the GPIO pin as OUTPUT
    GPIO_DEAKIN(uint8_t p);
    
    // Set the GPIO pin to HIGH
    void setHigh();
    
    // Set the GPIO pin to LOW
    void setLow();

    // Toggle the state of the GPIO pin (HIGH to LOW or LOW to HIGH)
    void toggle();
};

#endif