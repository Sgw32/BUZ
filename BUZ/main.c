/* ---------------------------------------------------------------------
 * PWM control for ATtiny13.
 * Datasheet for ATtiny13: http://www.atmel.com/images/doc2535.pdf
 * 
 * Pin configuration -
 * PB1/OC0B: 
 * PB4: input
 *
 * -------------------------------------------------------------------*/
 
// 4.8 MHz, built in resonator
#define F_CPU 4800000
#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define PWMCTL PORTB3
#define IGN_OUT PORTB4 
 
//1100 1550
 
//period 3413
//0.3222 0.4541 (x255)
//82 115
#define KEY_OPEN_PWM 70
#define KEY_CLOSE_PWM 100
#define KEY_TIMESTEP 30 //(1000/9)

//910 2110
//period 3413
//0.2666 0.6182 (x255)
//68 157
#define TCNT0_US_PER_TICK 1000/56

uint8_t state = 0;
uint8_t pwmint = 0;

ISR(PCINT0_vect)
{
	if (((PINB & (1<<PWMCTL)))) // начался фронт ШИМа 00000000000......11
	{
		pwmint = TCNT0; //254
	}
	if (!((PINB & (1<<PWMCTL))))// кончился +pulse ШИМа 1111111111111....0
	{
		
		long lpwm;
		pwmint = TCNT0-pwmint;//140-254
		lpwm=(long)pwmint*TCNT0_US_PER_TICK;
		if (lpwm>800 && lpwm<2200)
		{
			if (lpwm>1500)
				state = state<30 ? state+1 : 30;
			else
				state = state>0 ? state-1 : 0;
			if (state>15)
				PORTB|=(1<<IGN_OUT);
			else
				PORTB&=~(1<<IGN_OUT);	
		}
	}
}
 
 
void gpio_setup (void)
{
      
}
 
int gpio_read (void)
{
    return PINB&(1<<PWMCTL);
}
 
void pwm_setup (void)
{
    // Set Timer 0 prescaler to clock/64.
    // At 4.8 MHz this is 75 kHz.
    // See ATtiny13 datasheet, Table 11.9.
    TCCR0B |= (1 << CS01) | (1 << CS00);
 
    // Set to 'Fast PWM' mode
    TCCR0A |= (1 << WGM01) | (1 << WGM00);
 
    // Clear OC0B output on compare match, upwards counting.
    TCCR0A |= (1 << COM0A1) | (1 << COM0B1);
	// F~~293
}

void setupint()
{
	 GIMSK |= (1<<PCIE); // Разрешаем внешние прерывания PCINT0.
	 PCMSK |= (1<<PCINT3); // Разрешаем по маске прерывания на ногак кнопок (PCINT3, PCINT4)
	 sei(); // Разрешаем прерывания глобально: SREG |= (1<<SREG_I)
}
 
int main (void)
{
    // PWMOUT is an output.
    DDRB |= (1 << IGN_OUT);
	// PWMCTL is an input  
	DDRB &= ~(1 << PWMCTL);  
	state=0;
    gpio_setup();
    pwm_setup();
	setupint();
	
    while (1) {
  
    }
}