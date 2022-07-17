/* 
 * Для работы прошивки нужно поставить кварц на 16 МГц и заменить резистор R31 
 * с 1КОм на 500Ом (чтобы на PD6 (он же AIN0) стало 3.3В)
*/

#include <directADC.h>
byte adr = 153;
int max_speed = 60;                       // Макс. ШИМ на мотор
int counter = 0;                          // Переменная для плавного перехода от ускорения к остановке и обратно
int stop_time = 0;
int bits = 0;                             // Биты принимаемого пакета

void setup() {
  ACOMP_attachInterrupt(func, RISING_TRIGGER); 
  //ACOMP_setPositiveInput(ADC_1V1); // подключить +in к 1.1В
  ACOMP_setPositiveInput(ADC_AIN0); // подключить +in к D6 (3.3v)
  ACOMP_setNegativeInput(ADC_AIN1);
  pinMode(10, OUTPUT);    // ПРАВЫЙ ПОВ
  pinMode(9, OUTPUT);     // ЛЕВЫЙ ПОВ
  pinMode(1, OUTPUT);     // СТОПАРЬ
  pinMode(11, OUTPUT);    // МОТОР
}
void func() { 
  ACOMP_detachInterrupt();
  //delayMicroseconds(26900);
  delay(25);

  for(bits = 0; bits <= 7; bits++){
  bitWrite(adr, bits, !ACOMP_read());
  //delayMicroseconds(2000);
  delay(2);}

  ACOMP_attachInterrupt(func, RISING_TRIGGER);
}

void loop() {
    if(adr == 230){
      digitalWrite(1, HIGH);
      digitalWrite(11, LOW);
    }
    else if(adr == 153){
      digitalWrite(1, LOW); 
      analogWrite(11, max_speed);
    }
}





   
