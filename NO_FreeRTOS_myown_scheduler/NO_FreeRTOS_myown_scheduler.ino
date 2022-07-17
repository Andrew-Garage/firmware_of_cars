/* 
 * Для работы прошивки нужно поставить кварц на 16 МГц и заменить резистор R31 
 * с 1КОм на 500Ом (чтобы на PD6 (он же AIN0) стало 3.3В)
*/

#include <StaticThreadController.h>
#include <Thread.h>
#include <ThreadController.h>
#include <directADC.h>

byte adr = 153;
int max_speed = 60;                       // Макс. ШИМ на мотор
int counter = 0;                          // Переменная для плавного перехода от ускорения к остановке и обратно
int PWM = 0;                              // ШИМ на правый поворотник
int semaphore = 0;                        // 1/0 - уменьшаем/увеличиваем свечение правого поворотника
int bits = 0;                             // Биты принимаемого пакета

Thread Left = Thread();
Thread Right = Thread();
Thread MotorTHR = Thread();

void setup() {
  ACOMP_attachInterrupt(func, RISING_TRIGGER); 
  ACOMP_setPositiveInput(ADC_AIN0); // подключить +in к D6 (3.3v)
  ACOMP_setNegativeInput(ADC_AIN1);

  Left.onRun(ledLeft);        // Какую функцию запускаем на этом потоке
  Left.setInterval(300);      // С каким периодом

  Right.onRun(ledRight);      // Какую функцию запускаем на этом потоке
  Right.setInterval(20);      // С каким периодом

  MotorTHR.onRun(motor);      // Какую функцию запускаем на этом потоке
  MotorTHR.setInterval(20);   // С каким периодом
  
  pinMode(10, OUTPUT);    // ПРАВЫЙ ПОВ
  pinMode(9, OUTPUT);     // ЛЕВЫЙ ПОВ
  pinMode(1, OUTPUT);     // СТОПАРЬ
  pinMode(11, OUTPUT);    // МОТОР
}

void func() {                 // Прерывание вызовется когда вызовется, ЛУП его выполнения не прерывает
  ACOMP_detachInterrupt();
  delay(25);

  for(bits = 0; bits <= 7; bits++){
  bitWrite(adr, bits, !ACOMP_read());
  delay(2);}

  ACOMP_attachInterrupt(func, RISING_TRIGGER);
}

void loop() {                 // ЛУП крутится бесконечно и запускает функции с периодом ПОТОК.setInterval(ИНТЕРВАЛ)
if (Left.shouldRun())         
        Left.run();
if (Right.shouldRun())
        Right.run();
if (MotorTHR.shouldRun())
        MotorTHR.run();
}

void motor(){
  if(adr == 230){
    digitalWrite(1, HIGH);
    digitalWrite(11, LOW);
    }
  else if(adr == 153){
    digitalWrite(1, LOW); 
    analogWrite(11, max_speed);
    }
}

void ledRight(){
  if(semaphore == 0){         // Увеличиваем ШИМ
      PWM = PWM + 10;
      
      if(PWM>=255){
        semaphore = 1;
        PWM = 255;}
        
      analogWrite(10, PWM);}
  
  if(semaphore == 1){         // Уменьшаем ШИМ
    
       PWM = PWM - 10;
       
       if(PWM<=0){
         semaphore = 0;
         PWM = 0;}
         
    analogWrite(10, PWM);}
}

void ledLeft(){
  digitalWrite(9, !digitalRead(9));
}



   
