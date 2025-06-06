#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
// of SafeDuino
LiquidCrystal lcd(13, 12, 11, 10, 8, 9);

// Encoder
const int pin_A = A1;       // pin A1
const int pin_B = A2;       // pin A2
const int pin_D = A0;       // pin A0
unsigned char encoder_A;
unsigned char encoder_B;
unsigned char encoder_D;
unsigned char encoder_A_prev=0;
int buttonEncoderD = 0;
int buttonOn = 0;

// Button PWM on/off
const int pin_E = A3;       // pin A3
unsigned char button_onOff;
int buttonPWM = 0;
int buttonPWMon = 0;

// Menu data
int lcd_print_on = 1;
unsigned int time_counter = 0; // time counter
int set_options = 0; // 0 - freq, 1 - duty, 2 - phase
unsigned int freq_oscill = 500; // step = 0.5 Hz, min=0.5 Hz, max=200.0 Hz
unsigned int duty_oscill = 50; // step = 0.5 %, min=0.5 %., max=50.0 %

// PWM
unsigned long int pwm_counter = 0;
bool resetTimerOn = 0; // Сброс таймера при перенастройке частоты или скважности

void setup() {
  // LCD read/write (SafeDuino v1 only!)
  pinMode(2, OUTPUT);	
  digitalWrite(2, LOW);

  // Encoder pins
  pinMode(pin_A, INPUT);
  pinMode(pin_B, INPUT);
  pinMode(pin_D, INPUT);
  pinMode(pin_E, INPUT);  

  // PWM
  DDRD |= (1<<PD5)|(1<<PD6)|(1<<PD7); // config pin output
  PORTB |= (1<<PD5)|(1<<PD6); // init level LOW
  PORTD &= ~(1<<PD7); // PWM logic off

  // Timer 1 init (16 bit)
  cli();
  TCCR1A = 0;   // установить регистры в 0
  TCCR1B = 0;   // установить регистры в 0
  TCCR1B |= (0<<CS12)|(0<<CS11)|(1<<CS10); // clkI/1
  //TCCR1B |= (0<<CS12)|(1<<CS11)|(0<<CS10); // clkI/8
  //TCCR1B |= (0<<CS12)|(1<<CS11)|(1<<CS10); // clkI/64
  //TCCR1B |= (1<<CS12)|(0<<CS11)|(0<<CS10); // clkI/256
  //TCCR1B |= (1<<CS12)|(0<<CS11)|(1<<CS10); // clkI/1024
  TCCR1B |= (1<<WGM12); // прерываение по совпадению c OCR4A
  TIMSK1 |= (1<<OCIE1A);
  OCR1A = (40000/freq_oscill)*20; // Частота по умолчанию 80 кГц, 65535 - max

  // Timer 2 init (8 bit)
  TCCR2A = 0; // reset bit WGM21, WGM20
  TCCR2B = 0;
  //TCCR2B |= (0<<CS22)|(0<<CS21)|(1<<CS20); // clkI/1
  //TCCR2B |= (0<<CS22)|(1<<CS21)|(0<<CS20); // clkI/8
  //TCCR2B |= (0<<CS22)|(1<<CS21)|(1<<CS20); // clkI/64
  //TCCR2B |= (1<<CS22)|(0<<CS21)|(0<<CS20); // clkI/256
  TCCR2B |= (1<<CS22)|(0<<CS21)|(1<<CS20); // clkI/1024
  TCCR2B |= (1<<WGM22); // прерываение по совпадению c OCR4A
  TIMSK2 |= (1<<OCIE2A);
  OCR2A = 125; // 125 Гц
  sei();

  // set up the LCD's number of columns and rows: 
  lcd.begin(20, 4);
  // LCD freq.
  lcd.setCursor(0, 0);
  lcd.print("Freq:      Hz");
  lcd.setCursor(6, 0);
  lcd.print(freq_oscill/10.0, 1);
  // LCD duty cycle
  lcd.setCursor(0, 1);
  lcd.print("Duty:      %");
  lcd.setCursor(6, 1);
  lcd.print(duty_oscill/2.0, 1);
  // LCD cursor
  lcd.setCursor(5, 0);
  lcd.print("*");
  // LCD pwm on/off
  lcd.setCursor(0, 2);
  lcd.print("PWM: off");
}

void loop() {
}

// Генератор шим на 16-ти битном таймере
// Генерация сигнала  ШИМ с регулировкой скважности и фазы
ISR(TIMER1_COMPA_vect) {
  if (buttonPWMon) {
    if (!resetTimerOn) {
      if (pwm_counter == 0) PORTD |= (1<<PD5); // начало импульса для вывода PD5
      if (pwm_counter == 99) PORTD |= (1<<PD6); // начало импульса для вывода PD6

      if (pwm_counter == duty_oscill-1) PORTD &= ~(1<<PD5); // конец импульса для вывода PD5
      if (pwm_counter == 98+duty_oscill) PORTD &= ~(1<<PD6); // конец импульса для вывода PD6

      pwm_counter++;
      if(pwm_counter >= 199) pwm_counter = 0; // сброс счётчика в конце одного периода
    } else {
      // Сброс счётчика во время вывода на LCD дисплей
      // Если этого не сделать то таймер будет выдавать импульсы двойной длительности
      // во время вывода информации на LCD дисплей
      PORTD &= ~((1<<PD5)|(1<<PD6));
      pwm_counter++;
      if(pwm_counter >= 99) {
        pwm_counter = 0;
        resetTimerOn = 0;
      }
    }
  } else PORTD &= ~((1<<PD5)|(1<<PD6));
}

// Задание частоты
void freq_set() {
  if (freq_oscill <= 200) { // от 20 Гц и ниже
    TCCR1B |= (1<<CS11); // clkI/8
    TCCR1B &= ~(1<<CS10); // clkI/8
    OCR1A = (10000/freq_oscill)*10;
  }
  if (freq_oscill > 200) { // выше 20 Гц
    TCCR1B &= ~(1<<CS11); // clkI/1
    TCCR1B |= (1<<CS10); // clkI/1
    OCR1A = (40000/freq_oscill)*20;
  }
}

// Сброс таймера во время вывода на LCD дисплей
void resetTimer () {
  resetTimerOn = 1;
  pwm_counter = 0;
  PORTD &= ~((1<<PD5)|(1<<PD6));
}

// Работа с энкодером на прерывании основанном на переполнении таймера 2
ISR(TIMER2_COMPA_vect) {
    encoder_A = digitalRead(pin_A);     // считываем состояние выхода А энкодера 
    encoder_B = digitalRead(pin_B);     // считываем состояние выхода B энкодера    
    encoder_D = digitalRead(pin_D);     // считываем состояние выхода D энкодера
    button_onOff = digitalRead(pin_E);  // считываем состояние выхода E энкодера

    // Включить/выключить ШИМ
    if(button_onOff) buttonPWM = 1;
    else {
      lcd_print_on = 1;
      if (buttonPWM) {
        buttonPWM = 0;
        if(buttonPWMon) {
          buttonPWMon = 0;
          pwm_counter = 0;
          PORTD &= ~(1<<PD7);
        }
        else {
          buttonPWMon = 1;
          PORTD |= (1<<PD7);
        }
      }
    }

    // Выбрать частоту/скважность
    if(encoder_D) buttonEncoderD = 1;
    else {
      lcd_print_on = 1;
      if (buttonEncoderD) {
        buttonEncoderD = 0;
        if(buttonOn) {
          buttonOn = 0;
          set_options++;
          if(set_options >= 2) set_options = 0;
        } else {
          set_options++;
          if(set_options >= 2) set_options = 0;
          buttonOn = 1;
        }
      }
    }

    // Задать частоту/скважность
    if((!encoder_A) && (encoder_A_prev)){    // если состояние изменилось с положительного на ноль
      if(encoder_B) {
        lcd_print_on = 1;
        // Вращение по часовой стрелке
        if(!set_options){
          if (freq_oscill - 1 >= 5) {
            freq_oscill -= 5;
            freq_set();
          }
        }
        if(set_options == 1 and duty_oscill - 1 >= 1) duty_oscill -= 1;
      }   
      else {
        // Вращение против часовой стрелки
        lcd_print_on = 1;
        if(!set_options){
          if(freq_oscill + 1 <= 2000) {
            freq_oscill += 5;
            freq_set();
          }
        }
        if(set_options == 1 and duty_oscill + 1 <= 100) duty_oscill += 1;
      }   
    }   
    encoder_A_prev = encoder_A;     // сохраняем значение А для следующего цикла

    if(time_counter == 120 && lcd_print_on) lcd_print_data ();  // выводим данные на LCD каждую секунду
    time_counter++;
    if(time_counter > 125) time_counter = 0;
}

// Вывод данных на LCD дисплей
void lcd_print_data () {
  resetTimer();
  lcd.setCursor(5, 0);
  lcd.print(" ");
  lcd.setCursor(5, 1);
  lcd.print(" ");
      
  if(!set_options) {
    resetTimer();
    lcd.setCursor(5, 0);
    lcd.print("*     ");
    lcd.setCursor(6, 0);
    lcd.print(freq_oscill/10.0, 1);
  } 
  if(set_options == 1) {
    resetTimer();
    lcd.setCursor(5, 1);
    lcd.print("*    ");
    lcd.setCursor(6, 1);
    lcd.print(duty_oscill/2.0, 1);
  }
  if(buttonPWMon) {
    resetTimer();
    lcd.setCursor(5, 2);
    lcd.print("on ");
  } else {
    resetTimer();
    lcd.setCursor(5, 2);
    lcd.print("off");
  }
  lcd_print_on = 0;
}
