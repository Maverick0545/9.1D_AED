#include "GPIO.h"

// Constructor to initialize the GPIO pin as OUTPUT
GPIO_DEAKIN::GPIO_DEAKIN(uint8_t p) {
    pin = p;
    
    // Determine the port group and pin mask based on the pin number
    portGroup = &PORT->Group[g_APinDescription[pin].ulPort];
    pinMask = (1 << g_APinDescription[pin].ulPin);

    // Configure the pin as output by setting the direction register
    portGroup->DIRSET.reg = pinMask;
}

// Set the GPIO pin to HIGH
void GPIO_DEAKIN::setHigh() {
    portGroup->OUTSET.reg = pinMask;  // Set pin high
}

// Set the GPIO pin to LOW
void GPIO_DEAKIN::setLow() {
    portGroup->OUTCLR.reg = pinMask;  // Set pin low
}

// Toggle the state of the GPIO pin
void GPIO_DEAKIN::toggle() {
    portGroup->OUTTGL.reg = pinMask;  // Toggle pin
}