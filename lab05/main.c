#include <msp430.h>
#define LEDRED BIT0
#define LEDGREEN BIT6
#define B1 BIT3

void timer_ACLK_VLO(int timer, int ms);
void timer_SMCLK_DCO(int timer, int ms);
void timer_stop(int timer);
void timer_ACLK_VLO_trigger_mode(int timer, int ms);
void timer_SMCLK_DCO_trigger_mode(int timer, int ms);
void timer_clear(int timer);
void timer_interrupt_enable(int timer);
void timer_interrupt_clear(int timer);

void led_init(int led_bit);
void led_set(int led_bit, int state);

void button_init();
void button_interrupt_enable();
void button_interrupt_set(int high_to_low);
void button_interrupt_clear();

void adc_temp_rsc(); // repeat single channel
void adc_temp_scsc(); // single channel single conversion
void adc_temp_scsc_sc(); // single channel single conversion start conversion

/*
// basic 1
volatile unsigned int led_sel, led_on, f1, f2, b_place, ta1_isr_func_on, port1_func_on, adc10_isr_func_on, lpm_mode;
volatile double avg, last_avg, vol;

#pragma vector = TIMER1_A0_VECTOR
__interrupt void TA1_ISR (void){
	timer_interrupt_clear(1);
	timer_clear(1);

	ta1_isr_func_on = 1;

	_BIC_SR_IRQ(lpm_mode + GIE);
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void) {
	button_interrupt_clear();

	port1_func_on = 1;

	_BIC_SR_IRQ(lpm_mode + GIE);
}

#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void) {
	adc10_isr_func_on = 1;

	_BIC_SR_IRQ(lpm_mode + GIE);
}

void ta1_isr_func() {
	if(led_on == 0) {
		led_on = 1;
		led_set(led_sel, 1);
		timer_ACLK_VLO(1, f1);
	}
	else {
		led_on = 0;
		led_set(LEDRED, 0);
		led_set(LEDGREEN, 0);
		timer_ACLK_VLO(1, f2);

		led_sel = (led_sel == LEDGREEN) ? LEDRED : LEDGREEN;
	}
}

void port1_func() {
	if (b_place == 1) {
		button_interrupt_clear();
		button_interrupt_set(0);

		b_place = 0;

		timer_ACLK_VLO_trigger_mode(0, 250);
	}
	else {
		button_interrupt_clear();
		button_interrupt_set(1);

		b_place = 1;

		timer_stop(0);
	}
}

#define SN 4
void adc10_isr_func() {
	volatile static int count = 0, init = 3;
	volatile int i;
	volatile double temp[SN], total;

	if(count < SN) {
		vol = 1.5 * (double)ADC10MEM/(double)1024;
		temp[count] = (vol - 0.986) / 0.00355;
		count++;
		return;
	}
	count = 0;
	total = 0;
	for(i = 0; i < SN; i++) {
		total += temp[i];
	}

	if(init) {
		avg = total / SN;
		init--;
		return;
	}

	last_avg = total / SN;
	if(avg + 0.1 < last_avg) {
		f1 = 300;
		f2 = 200;
		avg = last_avg;

		lpm_mode = LPM0_bits;
	}

	if(last_avg < avg - 0.1) {
		f1 = 300;
		f2 = 700;
		avg = last_avg;

		lpm_mode = LPM3_bits;
	}
}

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    led_init(LEDRED);
    led_init(LEDGREEN);
    button_init();
    button_interrupt_enable();
    button_interrupt_set(1);

    adc_temp_rsc();

	led_sel = LEDGREEN;
	led_set(led_sel, 1);
	led_on = 1;
	b_place = 1;

	timer_ACLK_VLO(1, 300);
	timer_interrupt_enable(1);

	f1 = 300;
	f2 = 700;

	lpm_mode = LPM3_bits;

	__enable_interrupt();
	while(1) {
		_BIS_SR(lpm_mode + GIE);

		if(ta1_isr_func_on == 1) {
			ta1_isr_func();
			ta1_isr_func_on = 0;
		}

		if(port1_func_on == 1){
			port1_func();
			port1_func_on = 0;
		}

		if(adc10_isr_func_on == 1) {
			adc10_isr_func();
			adc10_isr_func_on = 0;
		}
	}

	return 0;
}
*/


// basic 2
volatile unsigned int led_sel, led_on, f1, f2, b_place, count, total_clock, n;
double total_time;

void show_time() {
	n++;
	return;
}

void calculate_time() {
	total_clock += TA1R;
	total_time = total_clock / 1500.0;
	n++;

	if(n > 20) {
		show_time();
	}
}

#pragma vector = TIMER0_A0_VECTOR
__interrupt void TA0_ISR (void){
	calculate_time();

	timer_interrupt_clear(1);
	timer_clear(1);

	if(led_on == 0) {
		led_on = 1;
		led_set(led_sel, 1);
		timer_ACLK_VLO(0, f1);
	}
	else {
		led_on = 0;
		led_set(led_sel, 0);
		timer_ACLK_VLO(0, f2);

		count++;
		if(count == 3) {
			led_sel = LEDRED;
			f1 = 200;
			f2 = 800;
		}
	}

	_BIC_SR_IRQ(LPM3_bits);
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void) {
	calculate_time();

	button_interrupt_clear();

	if (b_place == 1) {
		button_interrupt_clear();
		button_interrupt_set(0);

		led_sel = LEDGREEN;
		led_set(led_sel, 1);
		led_on = 1;
		count = 0;
		b_place = 0;

		f1 = 300;
		f2 = 700;

		timer_ACLK_VLO_trigger_mode(0, 300);
		timer_interrupt_enable(0);
	}
	else {
		button_interrupt_clear();
		button_interrupt_set(1);

		led_set(LEDRED, 0);
		led_set(LEDGREEN, 0);
		b_place = 1;

		timer_stop(0);
	}

	_BIC_SR_IRQ(LPM3_bits);
}

int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    led_init(LEDRED);
    led_init(LEDGREEN);
    button_init();
    button_interrupt_enable();
    button_interrupt_set(1);

	b_place = 1;
	total_clock = 0;
	n = 0;

	while(1) {
		timer_ACLK_VLO(1, 10 * 1000);
		_BIS_SR(LPM3_bits + GIE);
	}
}

// utilities
void timer_ACLK_VLO(int timer, int ms) {
	BCSCTL3 |= LFXT1S_2;

	switch (timer) {
	case 0:
		TA0CCR0 = ms * 1500 / 1000;
		TA0CTL = MC_1|ID_3|TASSEL_1|TACLR;
		break;
	case 1:
		TA1CCR0 = ms * 1500 / 1000;
		TA1CTL = MC_1|ID_3|TASSEL_1|TACLR;
		break;
	default:
		break;
	}
}

void timer_SMCLK_DCO(int timer, int ms) {
	BCSCTL1 = CALBC1_1MHZ;
	DCOCTL = CALDCO_1MHZ;
	BCSCTL2 &= ~SELS;
	BCSCTL2 |= DIVS_3;

	switch (timer) {
	case 0:
		TA0CCR0 = ms * 15625 / 1000;
		TA0CTL = MC_1|ID_3|TASSEL_2|TACLR;
		break;
	case 1:
		TA1CCR0 = ms * 15625 / 1000;
		TA1CTL = MC_1|ID_3|TASSEL_2|TACLR;
		break;
	default:
		break;
	}
}

void timer_stop(int timer) {
	switch (timer) {
	case 0:
		TA0CTL = 0;
		break;
	case 1:
		TA1CTL = 0;
		break;
	default:
		break;
	}
}

void timer_ACLK_VLO_trigger_mode(int timer, int ms) {
	timer_ACLK_VLO(timer, ms);
	switch (timer) {
	case 0:
		TA0CCTL1 = OUTMOD_3;
		TA0CCR1 = TA0CCR0 - 1;
		break;
	case 1:
		TA1CCTL1 = OUTMOD_3;
		TA1CCR1 = TA0CCR0 - 1;
		break;
	default:
		break;
	}
}

void timer_SMCLK_DCO_trigger_mode(int timer, int ms) {
	timer_SMCLK_DCO(timer, ms);
	switch (timer) {
	case 0:
		TA0CCTL1 = OUTMOD_3;
		TA0CCR1 = TA0CCR0 - 1;
		break;
	case 1:
		TA1CCTL1 = OUTMOD_3;
		TA1CCR1 = TA0CCR0 - 1;
		break;
	default:
		break;
	}
}

void timer_clear(int timer) {
	switch (timer) {
	case 0:
		TA0CTL |= TACLR;
		break;
	case 1:
		TA1CTL |= TACLR;
		break;
	default:
		break;
	}
}

void timer_interrupt_enable(int timer) {
	switch (timer) {
	case 0:
		TA0CCTL0 |= CCIE;
		break;
	case 1:
		TA1CCTL0 |= CCIE;
		break;
	default:
		break;
	}
}

void timer_interrupt_clear(int timer) {
	switch (timer) {
	case 0:
		TA0CTL &= ~TAIFG;
		break;
	case 1:
		TA1CTL &= ~TAIFG;
		break;
	default:
		break;
	}
}

void led_init(int led_bit) {
	P1DIR |= led_bit;
	P1OUT &= ~led_bit;
}

void led_set(int led_bit, int state) {
	switch (state) {
	case 1:
		P1OUT |= led_bit;
		break;
	case 0:
		P1OUT &= ~led_bit;
		break;
	default:
		break;
	}
}

void button_init() {
	P1OUT |= BIT3;
	P1REN |= BIT3;
}

void button_interrupt_enable() {
	P1IFG &= ~BIT3;            // P1.4 IFG cleared
	P1IE |= BIT3;              // P1.4 interrupt enabled
}

void button_interrupt_set(int high_to_low){
	if(high_to_low) {
		P1IES |= BIT3;             // P1.4 High/Low edge
	}
	else {
		P1IES &= ~BIT3;            // P1.4 Low/High edge
	}
}

void button_interrupt_clear() {
	P1IFG &= ~B1;            // P1.4 IFG cleared
}

void adc_temp_rsc() { // repeat single channel
	volatile int wait = 3000;

	ADC10CTL0 = ADC10SHT_3 | ADC10ON | ADC10IE | SREF_1 | REFON;
	ADC10CTL1 = INCH_10 | CONSEQ_2 | SHS_1;    // Input from temperature sensor
	while(wait--);
	ADC10CTL0 |= ENC; // Start sampling
}

void adc_temp_scsc() { // single channel single conversion
	ADC10CTL0 = ADC10SHT_3 | ADC10ON | ADC10IE | SREF_1 | REFON;
	ADC10CTL1 = INCH_10 | ADC10DIV_0 | CONSEQ_0;    // Input from temperature sensor
}

void adc_temp_scsc_sc() { // single channel single conversion start conversion
	ADC10CTL0 |= ENC + ADC10SC; // Start sampling
}
