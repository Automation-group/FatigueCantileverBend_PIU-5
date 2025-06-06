#include <EEPROM.h>
#include <LiquidCrystal.h>
#include <Keypad.h>

// Initialize the library with the numbers of the interface pins
// For SafeDuino MEGA 2560
LiquidCrystal lcd(J6, J7, J2, J3, J4, J5);

// Keypad
const byte keypad_rows = 4; // количество строк на клавиатуре, 4
const byte keypad_cols = 3; // количество столбцов на клавиатуре, 3
char keymap[keypad_rows][keypad_cols] = {
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'},
  {'*', '0', '#'}
};
// Keypad pins
byte rPins[keypad_rows]= {A5,A0,A1,A3}; // строки с 0 по 3
byte cPins[keypad_cols]= {A4,A6,A2};    // столбцы с 0 по 2
// Keypad class init
Keypad kpd= Keypad(makeKeymap(keymap), rPins, cPins, keypad_rows, keypad_cols);

//int set_num = 0;
int num_rank = 0;
char setOfNumbers[12] = {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','\0'};
char setOfNumbersZero[12] = {' ',' ',' ',' ',' ',' ',' ',' ',' ',' ',' ','\0'};

// Clock timer
struct timer {
  unsigned int hour;
  unsigned int minut;
  unsigned int sec;
  unsigned int ticks;
} time;
char timerHMS[9] = "00:00:00"; // hour:min:sec
// 1Hz-30660, 10Hz-30640, 50Hz-30597, 100Hz-30560, 200Hz-30483, 1kHz-29906
// (02:08 +1)+(08:44 +2),  valueClockTimer(1-1/(524-128))
unsigned int valueClockTimer = 30597; // чиcло прерываний по clkI для отсчёта одной секунды
int counterInterruptSec = 0; // счётчик для обработки однократного перехода с 59 на 0 секунду

// ADC, amplitude
int adcMax = 0;
int adcMin = 1023;
int adcZero = 0;
long int adcSum = 0;
long int adcCounter = 0; // adc sps in sec
float adcAmp = 0.0;

// Frequency, number сycles
unsigned long int period = 0;
float adcFreq = 0.0;
unsigned long int numCycles = 0;
// delay aver counter
unsigned long int delayNew = 0;
unsigned long int delayOld = 0;
unsigned long int delaySum = 0;
unsigned int averCounter = 0;
unsigned int numberAverange = 1; // the value is calculated at the beginning of each oscillation period

// Menu
// 0 - main menu, 1 - experiment menu, 2 - calibration menu, 3 - information menu
int menuSelection = 0;
int expExitConfirmation = 0; // confirmation of exit from the experiment menu
// units of amplitude measurement
int unitsAmpMeas = 1; // 1 - ADC rms, 2 - V rms, 3 - L microns

// Linear approximation when calculating microns
// L_microns = ADCrms * coeff_k + coeff_b
float coeff_k = 0; // min 0.0001, max 9999.9999
float coeff_b = 0; // min -9999.9999, max 9999.9999
int setCoeff_k = 0; // calibration menu: 0 - set coeff_k off, 1 - set coeff_k on 
int setCoeff_b = 0; // calibration menu: 0 - set coeff_b off, 1 - set coeff_b on
int setNumbersOn = 0; // calibration menu: 0 - set coeff off, 1 - set coeff on

void setup() {
  // serial port
  //Serial.begin(115200);

  // ADC, 10-bit
  ADCSRA |= (1<<ADATE)|(1<<ADIE); // ADATA - auto start ADC, ADIE - ADC interrupt is activated
  // Division Factor, ADC SPS = clkI/13*128, 13 тактов МК на одно преобразование
  // Division Factor 16, SPS = 76 923 (max)
  //ADCSRA |= _BV(ADPS2);
  //ADCSRA &= ~(_BV(ADPS1)|_BV(ADPS0));
  // Division Factor 32, SPS = 38 461
  //ADCSRA |= _BV(ADPS2)|_BV(ADPS0);
  //ADCSRA &= ~_BV(ADPS1);
  // Division Factor 64, SPS = 19 230
  //ADCSRA |= _BV(ADPS2)|_BV(ADPS1);
  //ADCSRA &= ~_BV(ADPS0);
  // Division Factor 128, SPS = 9 615 (optimal)
  ADCSRA |= (1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);
  // Selecting and configuring the ADC input
  ADCSRB |= (1<<MUX5); // ADC8 - analog input
  //ADMUX &= ~((1<<MUX4)|(1<<MUX3)|(1<<MUX2)|(1<<MUX1)|(1<<MUX0)); // A8 - analog input
  DIDR2 |= (1<<ADC8D); // A8 - digital input A8 disable
  //DIDR0 = 0xff; // ADC0-ADC0 - digital input A0-A7 disable
  //ADMUX |= _BV(REFS1)|_BV(REFS0); // aref - 2.56V
  //ADMUX |= _BV(REFS1); // aref - 1.1V
  //ADMUX &= ~(_BV(REFS0)); // aref - 1.1V
  //ADMUX &= ~(_BV(REFS1)); // aref - Vcc (3...5V)
  //ADMUX |= _BV(REFS0); // aref - Vcc (3...5V)
  //ADMUX &= ~((1<<REFS1)|(1<<REFS0)); // aref - 4.096V
  //ADCSRA |= (1<<ADEN)|(1<<ADSC); // Start ADC
  ADCSRA &= ~((1<<ADEN)|(1<<ADSC)); // Stop ADC

  // Analog comparator (Interrupt)
  attachInterrupt(0, analogComparator, FALLING);
  // Analog comparator ATmega 2560
  /*ACSR =
  (0<<ACD) | // Analog Comparator: Enabled
  (0<<ACBG) | // Analog Comparator Bandgap Select: AIN0 is applied to the positive input
  (0<<ACO) | // Analog Comparator Output: Off
  (1<<ACI) | // Analog Comparator Interrupt Flag: Clear Pending Interrupt
  (1<<ACIE) | // Analog Comparator Interrupt: Enabled
  (0<<ACIC) | // Analog Comparator Input Capture: Disabled
  (1<<ACIS1) | (1<ACIS0); // Analog Comparator Interrupt Mode: Comparator Interrupt on Rising Output Edge
  ACSR |= (0<<ACD)|(0<<ACBG)|(0<<ACO)|(0<<ACI)|(1<<ACIE)|(1<<ACIC)|(1<<ACIS1)|(1<ACIS0);*/
  
  // Keypad and LCD display - timer 4
  TCCR4B |= (0<<CS42)|(1<<CS41)|(1<<CS40); // clkI/64
  TCCR4A = 0; // reset bit WGM21, WGM20
  TCCR4B |= (1<<WGM22);
  TIMSK4 |= (1<<OCIE4A);
  OCR4A = 25000; // 10 Hz

  // Hour timer initialization
  // clkI - clocking from microcontroller quartz 16 MHz
  TCCR2B = 0;
  ASSR = 0;
  TCCR2B |= (1<<CS20); // clkI/1
  TIMSK2 |= (1<<TOIE2); // enable overflow interrupt for clocking from clkI
  sei();

  // EEPROM read
  EEPROM.get(0, coeff_k);
  EEPROM.get(4, coeff_b);

  // Set up the LCD's number of columns and rows: 
  lcd.begin(20, 4);

  // LCD menu
  mainMenu();
}

void loop() {
}

// Keypad and LCD display
ISR(TIMER4_COMPA_vect) {
  // Experiment menu
  // Time (interruption every second)
  if (!counterInterruptSec and menuSelection == 1) {
    // Freq, amp
    if(period > 0 ) adcFreq = 1000000.0/(float)period;
    //adcAmp = (float)adcSum/(float)adcCounter; // Vrms (ADC)
    //adcAmp = (float)adcSum/(float)adcCounter*1.41421356; // Vamp (ADC)
    //adcAmp = (float)adcSum/(float)adcCounter*2.82842712; // Vpp (ADC)
    //adcAmp = (float)adcSum/(float)adcCounter*0.00442395; // aref - 4.096 V, amplitude (V)
    //adcAmp = (float)adcSum/(float)adcCounter*6.349171-3181.539392; // aref - 4.096 V, amplitude (micron)
    if(adcCounter > 0) {
      if (unitsAmpMeas == 1) adcAmp = (float)adcSum/(float)adcCounter; // Vrms (ADC)
      if (unitsAmpMeas == 2) adcAmp = (float)adcSum/(float)adcCounter*0.00442395; // aref - 4.096 V, amplitude (V)
      if (unitsAmpMeas == 3) adcAmp = (float)adcSum/(float)adcCounter*coeff_k-coeff_b; // aref - 4.096 V, amplitude (micron)
    }
    // Time (interruption every second)
    // seconds
    timerHMS[6] = '0' + time.sec/10;
    timerHMS[7] = '0' + time.sec%10;
    // minutes
    timerHMS[3] = '0' + time.minut/10;
    timerHMS[4] = '0' + time.minut%10;
    // hours
    timerHMS[0] = '0' + time.hour/10;
    timerHMS[1] = '0' + time.hour%10;
    // Time (hour:min:sec)
    lcd.setCursor(7, 0);
    lcd.print(timerHMS);
    //Serial.println(timerHMS);

    // Cycles
    lcd.setCursor(8, 1);
    lcd.print(numCycles);

    // Amplitude ADC, micron, Vrms(V)
    lcd.setCursor(5, 2);
    lcd.print("             ");
    lcd.setCursor(5, 2);
    if (unitsAmpMeas == 1) lcd.print(String(adcAmp,1)+" ADC");  // ADCrms (ADC)
    if (unitsAmpMeas == 2) lcd.print(String(adcAmp,3)+" Vrms"); // Vrms = ADCrms * coeff (V)
    if (unitsAmpMeas == 3) lcd.print(String(adcAmp,0)+" micron");// Lmicron = ADCrms * coeff_k - coeff_b (um)

    // ADC Zero 
    lcd.setCursor(5, 3);
    lcd.print("     ");
    lcd.setCursor(5, 3);
    lcd.print(adcZero-512); //Zero

    // ADC Frequency Hz
    lcd.setCursor(12, 3);
    lcd.print("        ");
    lcd.setCursor(12, 3);
    if (adcFreq < 9.999) lcd.print(String(adcFreq, 3)+" Hz");
    else if (adcFreq < 99.99) lcd.print(String(adcFreq, 2)+" Hz");
    else if (adcFreq < 999.9) lcd.print(String(adcFreq, 1)+" Hz");
    else lcd.print(String(adcFreq, 0)+" Hz");

    adcZero = adcMin+(adcMax-adcMin)/2;
    adcCounter = 0;
    adcSum = 0;
    adcMax = 0;
    adcMin = 1023;
    counterInterruptSec = 1; // interruption every second is enabled
  }
  
  // Keypad
  char keypressed = kpd.getKey();
  if (keypressed != NO_KEY) {
    // Main menu
    if (menuSelection == 0) {
      if (keypressed == '1' ) {
        menuSelection = 1; // experiment menu
        startADC();
        experimentMenu(); 
      }
      if (keypressed == '2' ) {
        menuSelection = 2; // calibration menu
        keypressed = "";
        calibrationMenu();
      }
      if (keypressed == '3' ) {
        menuSelection = 3; // information menu
        infoMenu();
      }
    }

    // Experiment menu
    if (menuSelection == 1) {
      if (!expExitConfirmation) {
        if (keypressed == '1') unitsAmpMeas = 1;
        if (keypressed == '2') unitsAmpMeas = 2;
        if (keypressed == '3') unitsAmpMeas = 3;
      } else {
        if (keypressed == '0') {
          expExitConfirmation = 0;
          lcd.setCursor(17, 0);
          lcd.print("   ");
        }
        if (keypressed == '1') {
          expExitConfirmation = 0;
          menuSelection = 0;
          stopADC();
          mainMenu(); 
        }
      }
      if (keypressed == '#' ) {
        lcd.setCursor(17, 0);
        lcd.print("1/0"); // Confirmation of exit from the experiment menu
        expExitConfirmation = 1;
      }  
    }

    // Calibration menu
    if (menuSelection == 2) {
      if (!setNumbersOn) {
        if (keypressed == '1') {
          setCoeff_k = 1;
          setCoeff_b = 0;
          setNumbersOn = 1;
          lcd.setCursor(7, 1);
          lcd.print("             ");
        }
        if (keypressed == '2') {
          setCoeff_k = 0;
          setCoeff_b = 1;
          setNumbersOn = 1;
          lcd.setCursor(7, 2);
          lcd.print("             ");
        }
        if (keypressed == '#') {
          setCoeff_k = 0;
          setCoeff_b = 0;
          menuSelection = 0;
          stopADC();
          mainMenu(); 
        }
      } else setCalibCoeff (keypressed);
    }

    // Information menu
    if (menuSelection == 3) {
      if (keypressed == '#' ) {
        menuSelection = 0;
        stopADC();
        mainMenu(); 
      }  
    }
  }
}

void setCalibCoeff (char keypressed) {
  if (num_rank > 9) num_rank = 9;
  // Save coeff
  if (keypressed == '#') {
    if (setCoeff_k) {
      coeff_k = atof(setOfNumbers);
      lcd.setCursor(7, 1);
      lcd.print("             ");
      lcd.setCursor(7, 1);
      lcd.print(coeff_k, 4);
      EEPROM.put(0, coeff_k);
    }
    if (setCoeff_b) {
      coeff_b = atof(setOfNumbers);
      lcd.setCursor(7, 2);
      lcd.print("             ");
      lcd.setCursor(7, 2);
      lcd.print(coeff_b, 4);
      EEPROM.put(4, coeff_b);
    }
    setNumbersOn = 0;
    setCoeff_k = 0;
    setCoeff_b = 0;
    num_rank = 0;
    for (int i=0; i<strlen(setOfNumbers)-1; i++) setOfNumbers[i] = setOfNumbersZero[i];
  }
  // Set coeff_k 
  if (setCoeff_k) {
    keypressed = convKeyPressed (keypressed);
    setOfNumbers[num_rank] = keypressed;
    lcd.setCursor(7, 1);
    lcd.print(setOfNumbers);
    num_rank++;
  }
  // Set coeff_b 
  if (setCoeff_b) {
    keypressed = convKeyPressed (keypressed);
    setOfNumbers[num_rank] = keypressed;
    lcd.setCursor(7, 2);
    lcd.print(setOfNumbers);
    num_rank++;
  }
}

char convKeyPressed (char keypressed) {
  /*if (keypressed == '0' ) set_num = 0;
  if (keypressed == '1' ) set_num = 1;
  if (keypressed == '2' ) set_num = 2;
  if (keypressed == '3' ) set_num = 3;
  if (keypressed == '4' ) set_num = 4;
  if (keypressed == '5' ) set_num = 5;
  if (keypressed == '6' ) set_num = 6;
  if (keypressed == '7' ) set_num = 7;
  if (keypressed == '8' ) set_num = 8;
  if (keypressed == '9' ) set_num = 9;*/
  if (num_rank == 0 and keypressed == '*') keypressed = '-';
  if (keypressed == '*') keypressed = '.';
  return keypressed;
}

// ADC
ISR (ADC_vect) {
  int code = ADC;// & 0x3FF;
  //Max, Min, Sum, ADC counter
  if (adcMax < code) adcMax = code;
  if (adcMin > code) adcMin = code;
  if (adcZero) {
    if (code > adcZero) adcSum += code - adcZero;
    else adcSum += adcZero - code;
    adcCounter++;
  }
}

void startADC () {
  ADCSRA |= (1<<ADEN)|(1<<ADSC); // Start ADC
}

void stopADC () {
  ADCSRA &= ~((1<<ADEN)|(1<<ADSC)); // Stop ADC
  adcMax = 0;
  adcMin = 1023;
  adcZero = 0;
  adcSum = 0;
  adcCounter = 0; // ADC SPS in one second
  adcAmp = 0.0;

  // Frequency, number сycles
  period = 0;
  adcFreq = 0.0;
  numCycles = 0;
  // Delay aver counter
  delayNew = 0;
  delayOld = 0;
  delaySum = 0;
  averCounter = 0;
  numberAverange = 1;

  // Timer off
  counterInterruptSec = 0;
  time.ticks = 0;
  time.sec = 0;
  time.minut = 0;
  time.hour = 0;
}

// Analog comparator LM211
void analogComparator () {
  // Measurement of the oscillation period
  unsigned long int time = micros();
  delayNew = time - delayOld;
  delayOld = time;
  
  // Сycles counter
  numCycles ++;
  
  // Calculation of the number of averages if averCounter = 0
  if (!averCounter) {
    numberAverange = 100000/delayNew; // 100 000 / 10 000 microsec = 10 averages
  }
  averCounter ++;
  delaySum += delayNew;
  // Calculation of the period based on a given number of averages
  if (averCounter > numberAverange or 200000/numberAverange < delayNew) {
    period = delaySum/(numberAverange+1);
    delaySum = 0;
    averCounter = 0;
  }
}

// Analog comparator ATmega2560
/*ISR(ANALOG_COMP_vect){
  unsigned long int a = micros();
  delayNew = a - delayOld;
  delayOld = a;
}*/

// Hour timer 
ISR(TIMER2_OVF_vect) {

  time.ticks++;
  if(time.ticks == valueClockTimer and menuSelection == 1) {
    time.ticks = 0;
    counterInterruptSec = 0;
    time.sec++;
    if(time.sec == 60){
      time.sec = 0;
      time.minut++;
    }
      if(time.minut == 60){
      time.minut = 0;
      time.hour++;
    }
  }
}

// Main menu
void mainMenu () {
  // LCD main menu
  lcd.clear();
  lcd.print("    Main menu");
  lcd.setCursor(0, 1);
  lcd.print(" 1) Experiment");
  lcd.setCursor(0, 2);
  lcd.print(" 2) Calibration");
  lcd.setCursor(0, 3);
  lcd.print(" 3) Info");
}

// Experiment menu
void experimentMenu () {
  lcd.clear();
  lcd.print("Timer: 00:00:00"); // hour:min:sec
  lcd.setCursor(0, 1);
  lcd.print("Cycles: 0");
  lcd.setCursor(0, 2);
  lcd.print("Amp: "+String(adcAmp,1)+" ADC");  // ADCrms (ADC)
  lcd.setCursor(0, 3);
  lcd.print("Zero:0    F:0.000 Hz");
  lcd.setCursor(0, 0);
}

// Calibration
void calibrationMenu () {
  lcd.clear();
  lcd.print("Calibration menu");
  lcd.setCursor(0, 1);
  lcd.print("1) k = "+String(coeff_k,4));
  lcd.setCursor(0, 2);
  lcd.print("2) b = "+String(coeff_b,4));
  lcd.setCursor(0, 3);
  lcd.print("micron = ADC*k + b");
}

// Information menu
void infoMenu () {
  lcd.clear();
  lcd.print("Information menu");
  lcd.setCursor(0, 1);
  lcd.print("1) Time 00:00:00");
  //rtc.isReset();
  //DateTime dt = rtc.getTime();
  //lcd.print("1) Time "+dt.hour+':'+dt.minute+':'+dt.second);
  //Serial.println(dt.hour+':'+dt.minute+':'+dt.second);
  lcd.setCursor(0, 2);
  lcd.print("2) D.M.Y 27.05.2025");
  //lcd.print("2) D.M.Y "+dt.day+'.'+dt.month+'.'+dt.year);
  lcd.setCursor(0, 3);
  lcd.print("Author: Vetrov D.N.");
}
