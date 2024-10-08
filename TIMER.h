#ifndef TIMER_H
#define TIMER_H

#include <Arduino.h>

class Timer {
    uint32_t interval;  // Interval in milliseconds
    uint32_t lastTime;  // Last recorded time
public:
    // Constructor to initialize the timer
    Timer();
    
    // Start the timer with a specified interval (in milliseconds)
    void start(uint32_t i);
    
    // Check if the interval has elapsed
    bool hasElapsed();
    
    // Reset the timer (useful for restarting the interval)
    void reset();
};

#endif