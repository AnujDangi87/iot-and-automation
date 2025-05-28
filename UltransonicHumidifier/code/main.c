#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

// Define pins
#define BUTTON_PIN PD2       // Button connected to PD2
#define HUMIDIFIER_PIN PD3   // Humidifier control pin (connected to MOSFET or relay)
#define LED_PIN PD4          // LED for status indication

// UART settings
#define F_CPU 16000000UL
#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1

// Function prototypes
void UART_init(unsigned int ubrr);
void UART_transmit(unsigned char data);
unsigned char UART_receive(void);
void init_pins(void);
void update_humidifier_state(uint8_t state);

// Global variables
volatile uint8_t humidifier_state = 0;  // 0: OFF, 1: ON
volatile uint8_t button_pressed = 0;   // Flag to indicate button press

int main(void) {
    // Initialize UART, pins, and interrupts
    UART_init(MYUBRR);
    init_pins();
    
    // Enable external interrupt on INT0 (PD2) for button
    EICRA |= (1 << ISC01);  // Falling edge of INT0 triggers interrupt
    EIMSK |= (1 << INT0);   // Enable INT0
    sei();                  // Enable global interrupts

    while (1) {
        // Check for Bluetooth commands
        if (UCSR0A & (1 << RXC0)) { // Data received via UART
            unsigned char command = UART_receive();
            if (command == '1') {
                update_humidifier_state(1); // Turn humidifier ON
            } else if (command == '0') {
                update_humidifier_state(0); // Turn humidifier OFF
            }
        }

        // Check for button press (handled by ISR)
        if (button_pressed) {
            button_pressed = 0;  // Clear the flag
            // Toggle the humidifier state
            update_humidifier_state(!humidifier_state);
        }
    }
}

// UART initialization
void UART_init(unsigned int ubrr) {
    // Set baud rate
    UBRR0H = (unsigned char)(ubrr >> 8);
    UBRR0L = (unsigned char)ubrr;

    // Enable receiver and transmitter
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);

    // Set frame format: 8 data bits, 1 stop bit
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);
}

// UART transmit function
void UART_transmit(unsigned char data) {
    while (!(UCSR0A & (1 << UDRE0))); // Wait for empty transmit buffer
    UDR0 = data;                     // Put data into buffer
}

// UART receive function
unsigned char UART_receive(void) {
    return UDR0; // Return received data
}

// Initialize pins
void init_pins(void) {
    // Set HUMIDIFIER_PIN and LED_PIN as output
    DDRD |= (1 << HUMIDIFIER_PIN) | (1 << LED_PIN);

    // Set BUTTON_PIN as input with pull-up resistor
    DDRD &= ~(1 << BUTTON_PIN);
    PORTD |= (1 << BUTTON_PIN);
}

// Update humidifier state (common function for both button and Bluetooth)
void update_humidifier_state(uint8_t state) {
    if (state) {
        PORTD |= (1 << HUMIDIFIER_PIN); // Turn humidifier ON
        PORTD |= (1 << LED_PIN);        // Turn LED ON
    } else {
        PORTD &= ~(1 << HUMIDIFIER_PIN); // Turn humidifier OFF
        PORTD &= ~(1 << LED_PIN);        // Turn LED OFF
    }
    humidifier_state = state; // Update global state
}

// Interrupt Service Routine for INT0 (Button press)
ISR(INT0_vect) {
    _delay_ms(50); // Debounce delay
    if (!(PIND & (1 << BUTTON_PIN))) { // Check if button is still pressed
        button_pressed = 1; // Set the flag for main loop
    }
}
