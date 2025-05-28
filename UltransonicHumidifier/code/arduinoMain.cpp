// Define pins
#define BUTTON_PIN 2       // Button connected to digital pin 2 (PD2 / INT0)
#define HUMIDIFIER_PIN 3   // Humidifier control pin (digital pin 3 / PD3)
#define LED_PIN 4          // LED for status indication (digital pin 4 / PD4)

// Global variables
volatile uint8_t humidifier_state = 0;  // 0: OFF, 1: ON
volatile uint8_t button_pressed = 0;   // Flag to indicate button press

void setup() {
    // Initialize serial communication for Bluetooth
    Serial.begin(9600);

    // Set HUMIDIFIER_PIN and LED_PIN as output
    pinMode(HUMIDIFIER_PIN, OUTPUT);
    pinMode(LED_PIN, OUTPUT);

    // Set BUTTON_PIN as input with pull-up resistor
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    // Enable external interrupt on BUTTON_PIN (digital pin 2)
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), buttonISR, FALLING);
}

void loop() {
    // Check for Bluetooth commands
    if (Serial.available()) {
        char command = Serial.read();
        if (command == '1') {
            updateHumidifierState(1); // Turn humidifier ON
        } else if (command == '0') {
            updateHumidifierState(0); // Turn humidifier OFF
        }
    }

    // Check for button press (handled by ISR)
    if (button_pressed) {
        button_pressed = 0;  // Clear the flag
        // Toggle the humidifier state
        updateHumidifierState(!humidifier_state);
    }
}

// Update humidifier state (common function for both button and Bluetooth)
void updateHumidifierState(uint8_t state) {
    if (state) {
        digitalWrite(HUMIDIFIER_PIN, HIGH); // Turn humidifier ON
        digitalWrite(LED_PIN, HIGH);        // Turn LED ON
    } else {
        digitalWrite(HUMIDIFIER_PIN, LOW); // Turn humidifier OFF
        digitalWrite(LED_PIN, LOW);        // Turn LED OFF
    }
    humidifier_state = state; // Update global state
}

// Interrupt Service Routine for button press
void buttonISR() {
    delay(50); // Debounce delay
    if (digitalRead(BUTTON_PIN) == LOW) { // Check if button is still pressed
        button_pressed = 1; // Set the flag for main loop
    }
}
