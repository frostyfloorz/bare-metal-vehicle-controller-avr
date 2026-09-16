#include <avr/io.h>
#include <stdbool.h>
#include <stdint.h>
#define F_CPU 16000000
#include <util/delay.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include <stdlib.h>


//shared global varibles 
volatile uint16_t adcx =0;
volatile uint16_t adcy =0;
volatile bool current_channel; /* keeps track of which ADC channel is being read */



/* switch between the X and Y joystick channels after each conversion */
ISR(ADC_vect)
{
    if (current_channel){
        adcx = ADC;
        current_channel = false;
        ADMUX &= ~(1U << MUX0); // switch to ADC0
        ADCSRA |= (1U << ADSC); // start next conversion


    }
    else{
        adcy = ADC;
        current_channel = true;
        ADMUX |= (1U << MUX0); // switch back to ADC1
        ADCSRA |= (1U << ADSC);



    }
   
}

void uart_init(void){

     UCSR0B = (1U << TXEN0); // enable transmitter
     UBRR0H = 0;
    UBRR0L = 16; // sets baud rate
    UCSR0A = (1U << U2X0);      // double-speed UART mode
     UCSR0C =
        (1U << UCSZ01) |
        (1U << UCSZ00);         // 8-bit characters


}



uint16_t servo_pulse(int16_t scale){
     uint16_t pulse = 1500 + 5 * (scale);
     return pulse;

}
void motor_pwm_init(void)
{
    DDRB |= (1U << DDB4);       // PB4 / D10 / OC2A as output

    TCCR2A =
        (1U << COM2A1) |        // Timer2 controls OC2A pin
        (1U << WGM21)  |
        (1U << WGM20);          // Fast PWM mode

    TCCR2B = (1U << CS21);      // Timer2 prescaler = /8

    OCR2A = 0;                  // PWM duty value, 0–255
}

void servo_init(void)
{
    DDRB |= (1U << DDB5);     // D11 / OC1A output

    TCCR1A = (1U << COM1A1) | (1U << WGM11);  // Timer1 controls OC1A pin
    TCCR1B = (1U << WGM13)  | (1U << WGM12) | (1U << CS11); // Fast PWM, ICR1 TOP, prescaler /8

    ICR1 = 39999;             // 20 ms period
    OCR1A = 3000;             // start centered
}

/* stop small joystick noise near the center from causing movement */
 void deadzone(int16_t *pScale){

    if (*pScale <5 && *pScale > -5){

        *pScale = 0;
    }


 }
 // scale joystick from -100 to 100
int16_t scale_joystick(int16_t centred)
{
    int16_t scale = ((int32_t)centred*100)/512;
    
    if (scale>100){
        scale = 100;
    }
    else if (scale<-100){

        scale = -100;
    }
    return scale;
}

int16_t centeradc(uint16_t raw){
    int16_t center = (int16_t) raw - 512;
    return center;



}

void adc_init(void)
{
   ADMUX |= (1U << REFS0) ;  // use AVcc as ADC reference voltage
    ADMUX |= (1U << MUX0);   // select ADC as first channel
    current_channel = true;
    

    ADCSRA =
        (1U << ADEN)  |              // enables ADC
        (1U << ADPS2) |
        (1U << ADPS1) |
        (1U << ADPS0) |             // ADC clock prescaler = /128
        (1U<<ADIE);                  // enable ADC-complete interrupt


        ADCSRA |= (1U << ADSC);     // start first ADC conversion
}

void uart_tx_char(char c)
{

    while (!(UCSR0A & (1U<<UDRE0))){
        
    }
    UDR0 = c;
}


void uart_tx_string(char *c)
{
    while (*c != '\0'){
        uart_tx_char(*c);
         c++;



    }
}

void uart_tx_int(int16_t value)
{
    char buffer[8];

    itoa(value, buffer, 10);
    uart_tx_string(buffer);
}

/* send useful controller values to the PC for testing/debugging */
void uart_tx_telemetry(int16_t steer,
                       int16_t throttle,
                       uint8_t pwm,
                       bool safe)
{
    uart_tx_string("STEER: ");
    uart_tx_int(steer);
    uart_tx_string("THROTTLE: ");
    uart_tx_int(throttle);
    uart_tx_string("PWM:");
    uart_tx_int(pwm);
    uart_tx_string("SAFE: ");
    uart_tx_int(safe);
    uart_tx_string("\r\n");
    
    
}

typedef struct{
         uint8_t pwm;
    bool direction;
    } data;

//returns raw magintude
data motory(int16_t scaley){
    data dat;
if (scaley > 0) {
    int16_t speed = scaley;
    dat.direction = true;
     dat.pwm = (speed*255) /100;
    return dat;
} else if (scaley < 0) {
    int16_t speed = -scaley;
    dat.direction = false;
     dat.pwm = (speed*255) /100;
    return dat;
} else {
    dat.direction = true;
     dat.pwm = 0;
    return dat;
}


}



int main(void){
 

DDRB &= ~(1U << DDB6); 
PORTB |= (1U<< PORTB6); 
DDRA |= (1U << DDA0);
DDRA |= (1U << DDA1);
adc_init();
servo_init();
motor_pwm_init();
uart_init();
sei();

uart_tx_string("\r\n");
int telementryCounter =0;

while(1){
    
if (!(PINB & (1U<< PINB6))){

    uint16_t rawx;
    uint16_t rawy;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
{
     rawx = adcx;
    rawy= adcy;
} 
    int16_t centredx = centeradc(rawx);
    int16_t centredy = centeradc(rawy);
    int16_t scalex = scale_joystick(centredx);
    int16_t scaley = scale_joystick(centredy);
    deadzone(&scalex);
    deadzone(&scaley);
    int16_t pulse = servo_pulse(scalex);
    data dat = motory(scaley);
    if (telementryCounter >= 1000UL){
        uart_tx_telemetry(scalex, scaley, dat.pwm, false);
        telementryCounter = 0;


    }

    if (dat.pwm ==0){
        PORTA &= ~ (1U<<PA1);
        PORTA &= ~ (1U<<PA0);

    }
    else if (dat.direction == true){
       PORTA |= (1U<<PA0);
        PORTA &= ~(1U<<PA1);
    }
    else{

        PORTA &= ~(1U<<PA0);
        PORTA |= (1U<<PA1);

    }
    OCR1A = pulse*2;
    OCR2A = dat.pwm;
}

else{OCR1A = 3000;
    OCR2A = 0;
    PORTA &= ~ (1U<<PA1);
    PORTA &= ~ (1U<<PA0);
      if (telementryCounter >= 1000UL)
    {
        uart_tx_telemetry(0, 0, 0, true);
        telementryCounter = 0;
    }}

   
    telementryCounter++;

}

}



  




