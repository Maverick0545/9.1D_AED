#include <FreeRTOS_SAMD21.h>
#include <DHT.h>
#include "GPIO.h"
#include "ADC.h"
#include "Timer.h"

// Pin Configuration
#define PRESSURE_SENSOR_PIN A0
#define DHT_PIN 2
#define DHT_TYPE DHT22
#define LED_PIN 13
#define BUTTON_PIN 3  // Pin connected to R16503 button
#define BUZZER_PIN 4

// State Tracking Variables for the Alarm System
int pressCount = 0;
unsigned long lastPressTime = 0;
unsigned long lastBtnPress = 0;  // Last time the button was pressed
unsigned long debounceStart = 0; // Last time debounce was checked
unsigned long debounceInterval = 200;   // Debouncing time (200ms)
unsigned long pressWindow = 10000;       // Time frame to count 5 presses (10 seconds)
unsigned long nurseDelay = 5000;        // 5-second wait before resetting after nurse call
bool isAlarmOn = false;
bool isButtonActive = false;
bool btnCurrentState = HIGH;  // Initially HIGH due to internal pull-up
bool btnPrevState = HIGH;     // Previous button state
bool nurseCalledFlag = false; // Flag for nurse call
unsigned long nurseCallStart = 0; // Time nurse was called
bool canResetAlarm = false;   // Tracks if alarm reset is allowed after the wait

// Object Initialization
DHT dhtSensor(DHT_PIN, DHT_TYPE);
GPIO_DEAKIN ledControl(LED_PIN);
GPIO_DEAKIN buzzerControl(BUZZER_PIN);
ADCModule pressureReader(PRESSURE_SENSOR_PIN);
Timer tempTimer;
Timer pressureTimer;

// Task Handlers
TaskHandle_t tempTaskHandle;
TaskHandle_t pressureTaskHandle;

// Function to monitor temperature, control LED and alarm system
void tempMonitoring(void *pvParameters) {
    (void) pvParameters;
    tempTimer.start(300000); // 5-minute interval
    while (1) {
        if (tempTimer.hasElapsed()) {
            float tempValue = dhtSensor.readTemperature();  // Fetch temperature from DHT sensor
            if (!isnan(tempValue)) {
                Serial.print("Temp Reading: ");
                Serial.print(tempValue);
                Serial.println(" °C");

                if (tempValue >= 38.0 && tempValue < 39.0) {
                    Serial.println("Notice: Mild fever detected. LED blinking every second.");
                    for (int i = 0; i < 10; i++) { // Blink for 10 seconds
                        ledControl.toggle();
                        vTaskDelay(1000 / portTICK_PERIOD_MS);  // 1-second delay
                    }
                    ledControl.setLow(); // Turn LED off after blinking
                } else if (tempValue >= 39.0 || tempValue < 35.0) {
                    Serial.println("CRITICAL: Extreme temperature detected! Buzzer activated.");
                    buzzerControl.setHigh();
                    vTaskDelay(1000 / portTICK_PERIOD_MS); // 1-second alarm
                    buzzerControl.setLow();
                } else {
                    Serial.println("Temp is within the normal range.");
                }
            } else {
                Serial.println("Temperature read failed.");
            }
            tempTimer.reset();
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);  // Yield control to FreeRTOS
    }
}

// Function to monitor bed occupancy and manage HELP alarm using button taps
void pressureMonitoring(void *pvParameters) {
    (void) pvParameters;
    pressureTimer.start(60000);  // 1-minute interval
    while (1) {
        if (pressureTimer.hasElapsed()) {
            int pressureValue = pressureReader.readValue();  // Read pressure sensor
            Serial.print("Pressure Value: ");
            Serial.println(pressureValue);

            if (pressureValue < 100) {
                Serial.println("Bed is not occupied.");
            } else {
                Serial.println("Bed is occupied.");
            }
            pressureTimer.reset();
        }

        // Handle delay after nurse call
        if (nurseCalledFlag && (millis() - nurseCallStart) < nurseDelay) {
            vTaskDelay(100 / portTICK_PERIOD_MS);  // Yield control and wait
            continue;
        } else if (nurseCalledFlag && (millis() - nurseCallStart) >= nurseDelay && !canResetAlarm) {
            canResetAlarm = true;  // Allow alarm reset after nurse call delay
            Serial.println("Alarm can now be reset.");
        }

        // Debounce and handle button press
        int btnReading = digitalRead(BUTTON_PIN);

        // Detect if button state changed
        if (btnReading != btnPrevState) {
            debounceStart = millis();  // Start debounce timer
        }

        // Check if the button press is stable beyond debounce time
        if ((millis() - debounceStart) > debounceInterval) {
            if (btnReading == LOW && !isButtonActive) {
                if (millis() - lastBtnPress > debounceInterval) {
                    lastBtnPress = millis();  // Update button press time
                    if (pressCount == 0) {
                        lastPressTime = millis();  // Start the tap window
                    }
                    pressCount++;
                    Serial.print("Tap registered. Total Taps: ");
                    Serial.println(pressCount);

                    if (pressCount == 5) {
                        Serial.println("ALERT: Nurse HELP Requested!");
                        isAlarmOn = true;
                        buzzerControl.setHigh();  // Trigger alarm
                        nurseCalledFlag = true;  // Start nurse delay
                        nurseCallStart = millis();  // Record nurse call time
                        canResetAlarm = false;  // Prevent immediate reset

                        pressCount = 0;  // Reset tap count after the nurse is called
                    }
                    isButtonActive = true;  // Lock the button press
                }
            } else if (btnReading == HIGH) {
                isButtonActive = false;  // Unlock button for next press
            }
        }

        // Save button state for next loop iteration
        btnPrevState = btnReading;

        // Reset press count if time exceeds press window
        if (pressCount > 0 && (millis() - lastPressTime) > pressWindow && pressCount < 5) {
            Serial.println("Time expired. Resetting tap count.");
            pressCount = 0;
        }

        // Reset the alarm if conditions are met
        if (isAlarmOn && canResetAlarm && digitalRead(BUTTON_PIN) == LOW) {
            Serial.println("Nurse arrived. Alarm deactivated.");
            buzzerControl.setLow();
            
            // Fetch the current temperature
            float tempValue = dhtSensor.readTemperature();
            if (!isnan(tempValue)) {
                Serial.print("Current Temperature: ");
                Serial.print(tempValue);
                Serial.println(" °C");
            } else {
                Serial.println("Failed to read temperature.");
            }
            
            isAlarmOn = false;
            canResetAlarm = false;  // Reset system for future alarms
            nurseCalledFlag = false; // Clear nurse call flag
        }

        vTaskDelay(100 / portTICK_PERIOD_MS);  // Yield control to FreeRTOS
    }
}

void setup() {
    Serial.begin(9600);
    while(!Serial);

    // Initialize sensors and outputs
    dhtSensor.begin();
    pressureReader.begin();
    ledControl.setLow();
    buzzerControl.setLow();

    // Setup button pin with internal pull-up
    pinMode(BUTTON_PIN, INPUT_PULLUP);  // Enable pull-up resistor

    Serial.println("System is ready.");

    // Create FreeRTOS tasks
    xTaskCreate(tempMonitoring, "Temp Monitoring", 1000, NULL, 1, &tempTaskHandle);
    xTaskCreate(pressureMonitoring, "Pressure Monitoring", 1000, NULL, 1, &pressureTaskHandle);

    // Start FreeRTOS scheduler
    vTaskStartScheduler();
}

void loop() {
    // Loop left empty as FreeRTOS handles task scheduling
}