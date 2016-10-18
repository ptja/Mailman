#define F_CPU 8000000
#include <stdlib.h>

#include <avr/io.h>
#include <util/delay.h>
  
#include <avr/sleep.h>
#include <avr/interrupt.h>

// Routines to set and clear bits (used in the sleep code)
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#define LED 1

#define CO_ILE_POMIAR 100
volatile boolean f_watchdog = 1;       // ustawiane po zadziałaniu WDT
volatile boolean f_skrzynka = 0;       // ktoś otworzył skrzynkę. Podejrzany: listonosz.
volatile boolean f_slaba_bateria = 0;  // 1, jeśli napięcie baterii jest już niskie
volatile boolean f_zmierz_baterie = 0; // upłynął czas, zmierz napięcie baterii
volatile int     zaIlePomiar = 0;      // odliczanie do pomiaru napięcia baterii

void setup()  { 
    pinMode(LED,OUTPUT); 
    pinMode(0,INPUT_PULLUP); // switch w skrzynce
    // to avoid floating pins and save the supply current
    pinMode(2,INPUT_PULLUP);
    pinMode(3,INPUT_PULLUP);
    pinMode(4,INPUT_PULLUP);    
    pinMode(5,INPUT_PULLUP);
    pinMode(2,INPUT_PULLUP);
    pinMode(3,INPUT_PULLUP);
    pinMode(4,INPUT_PULLUP);    
    pinMode(5,INPUT_PULLUP);
    
    sbi(GIMSK,PCIE);   // przerwanie na zmianę stanu linii
    sbi(PCMSK,PCINT0); // zmiana na linii P0 zgłasza przerwanie

    delay(100);  // chwilka oddechu
    pinMode(LED, OUTPUT);
    blink(10);    
    blink(10);
    blink(10);
    // oszczędzamy prąd
    pinMode(LED, INPUT); //LED on Model B
    delay(3000);

    zaIlePomiar = 2;   // pierwszy raz zmierz od razu
    setup_watchdog(9); // co 8 sekund obudź się, a co CO_ILE_POMIAR zmierz napięcie zasilania.

    
/*    // "wymruganie" napięcia zasilania, do testów
      while (vcc) {
      if (vcc & 1) {
        blink(12);
      } else {
        blink(2);
      }
      delay(1000);
      vcc = vcc >> 1;
    }*/
    system_sleep();  // zaśnij
} 

void loop()  { 
  if (1 == f_skrzynka) { // ktoś gmerał przy skrzynce
    pinMode(LED, OUTPUT);
    blink(2);
    blink(2);
    blink(2);
    pinMode(LED, INPUT);
  }
  
  if (1 == f_zmierz_baterie) { // czas na pomiar
    long vcc = readVcc();
    if (vcc<3200) {
      f_slaba_bateria = 1;
      setup_watchdog(8); // mrugaj co ok. 4s
    }
    f_zmierz_baterie = 0; // zmierzyliśmy
  }
  if (f_slaba_bateria) { // mrugaj raz po wymianie baterii
    pinMode(LED, OUTPUT);
    blink(2);
    pinMode(LED, INPUT);
  }
  if (1 == f_watchdog) { // watchdog zadziałał
    f_watchdog=0;
    system_sleep();      // zaśnij znowu
  }
}

//zasypianie
void system_sleep() {
  
  cbi(ADCSRA,ADEN);                    // wyłącz ADC
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_enable();
  sleep_mode();
  sleep_disable();                     // obudził się
}

// 0=16ms, 1=32ms,2=64ms,3=128ms,4=250ms,5=500ms
// 6=1 sec,7=2 sec, 8=4 sec, 9= 8sec
void setup_watchdog(int mode) {
  byte bb;
  if (mode > 9 ) {
    mode=9;
  }
  bb = mode & 7;
  if (mode > 7) {
    bb |= (1<<5);
  }
  bb |= (1<<WDCE);

  MCUSR &= ~(1<<WDRF);
  // start timed sequence
  WDTCR |= (1<<WDCE) | (1<<WDE);
  // set new watchdog timeout value
  WDTCR = bb;
  WDTCR |= _BV(WDIE);
}
  
// WDT zadziałał
ISR(WDT_vect) {
  f_watchdog=1;
  if (!f_slaba_bateria) { // nie ma sensu mierzyć, jeśli już wiemy, że słaba
    zaIlePomiar--;
    if (zaIlePomiar<=0) { // czas na pomiar
      zaIlePomiar = CO_ILE_POMIAR; // następny pomiar za CO_ILE_POMIAR
      f_zmierz_baterie = 1; // znacznik dla loop(), że czas na pomiar
    }
  }
}

// zewnętrzne przerwanie
ISR(PCINT0_vect) {
  f_skrzynka=1;
  setup_watchdog(8); // mrugaj co kilka sekund (4s)
}


void blink(int time) {
    digitalWrite(1, HIGH);   // turn the LED on (HIGH is the voltage level)
    delay(100*time);               // wait for a second
    digitalWrite(1, LOW);    // turn the LED off by making the voltage LOW
    delay(100*time);               // wait for a second
}

long readVcc() {
  sbi(ADCSRA,ADEN);                    // switch Analog to Digitalconverter ON
  delay(10);
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif  
 
  delay(2); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring
 
  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH  
  uint8_t high = ADCH; // unlocks both
 
  cbi(ADCSRA,ADEN);                    // switch Analog to Digitalconverter OFF

  long result = (high<<8) | low;
  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result; // Vcc in millivolts
}
