#include "TIMER.h"

// Constructor to initialize the timer
Timer::Timer() {
    lastTime = 0;

    // Enable the TC3 timer
    PM->APBCMASK.reg |= PM_APBCMASK_TC3;

    // Enable the GCLK for TC3 (replace GCM_TC3_TC4 with GCM_TC4_TC5)
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(GCM_TC4_TC5) | GCLK_CLKCTRL_GEN_GCLK0 | GCLK_CLKCTRL_CLKEN;
    while (GCLK->STATUS.bit.SYNCBUSY);

    // Configure the TC3 timer in 16-bit mode
    TC3->COUNT16.CTRLA.reg |= TC_CTRLA_MODE_COUNT16 | TC_CTRLA_PRESCALER_DIV1024 | TC_CTRLA_WAVEGEN_MFRQ;
    while (TC3->COUNT16.STATUS.bit.SYNCBUSY);

    // Enable the TC3 timer
    TC3->COUNT16.CTRLA.reg |= TC_CTRLA_ENABLE;
    while (TC3->COUNT16.STATUS.bit.SYNCBUSY);
}

// Start the timer with a specified interval
void Timer::start(uint32_t i) {
    interval = i;
    lastTime = millis();  // Record the current time
}

// Check if the interval has elapsed
bool Timer::hasElapsed() {
    return (millis() - lastTime) >= interval;
}

// Reset the timer
void Timer::reset() {
    lastTime = millis();  // Update the last time to current
}