/* 
 * Для работы прошивки нужно поставить кварц на 16 МГц и кинуть провод
 * с PD7 на PD2 (на нем есть аппаратное прерывание)
*/

#include <Arduino_FreeRTOS.h>
#include <directADC.h>
#include <semphr.h>

int PWM = 0;                              // ШИМ на поворотники
int bits = 0;                             // Биты принимаемого пакета
int adr = 0;                              // Для записи сигнала светофора
int max_speed = 60;                       // Макс. ШИМ на мотор
SemaphoreHandle_t interruptSemaphore;

void TaskBlink1( void *pvParameters );
void TaskBlink2( void *pvParameters );
void TaskBlink3( void *pvParameters );
void TaskBlink4( void *pvParameters );

void setup() {
  pinMode(1, OUTPUT);     //стопы
  pinMode(9, OUTPUT);     // левый пов
  pinMode(10, OUTPUT);    // правый пов
  pinMode(11, OUTPUT);    // мотор
  pinMode(7, INPUT);     // IR
  pinMode(2, INPUT);


  //ACOMP_attachInterrupt(func, RISING_TRIGGER); 
  //ACOMP_setPositiveInput(ADC_AIN0);
  //ACOMP_setNegativeInput(ADC_AIN1);

  interruptSemaphore = xSemaphoreCreateBinary();              // Создаем семафор
  if (interruptSemaphore != NULL) {                           // Если создался
    //ACOMP_attachInterrupt(interruptHandler, RISING_TRIGGER);
    attachInterrupt(digitalPinToInterrupt(2), interruptHandler, RISING); // прерывание по пину 2
    }
  
  xTaskCreate(TaskBlink1,"Task1",128,NULL,4,NULL);    // 4 - макс. приоритет
  xTaskCreate(TaskBlink2,"Task2",128,NULL,1,NULL);    // иначе не будет 
  xTaskCreate(TaskBlink3,"Task3",128,NULL,1,NULL);    // считывать адрес
  xTaskCreate(TaskBlink4,"Task4",128,NULL,1,NULL);    // до конца
  vTaskStartScheduler();
}

void loop() {
}

void interruptHandler() {                         // Прерывание будет выкидывать сюда
  xSemaphoreGiveFromISR(interruptSemaphore, NULL);  // Тут мы можем включить семафор
}

void TaskBlink1(void *pvParameters)  {
    while(xSemaphoreTake(interruptSemaphore, portMAX_DELAY) == pdPASS) // Если семафор включен - считываем сигнал светофора
    {
      //vTaskDelay( 1 / portTICK_PERIOD_MS );
      //delay(1);
        //if (xSemaphoreTake(interruptSemaphore, portMAX_DELAY) == pdPASS) {
         //ACOMP_detachInterrupt();
          detachInterrupt(digitalPinToInterrupt(2)); // Отключим прерывание пока считываем адрес

          delay(26);               // Лучше использовать delay(), чтоб не привязываться к таймингу ядра ОС

          for(bits = 0; bits <= 7; bits++){
            bitWrite(adr, bits, !ACOMP_read());
            //delayMicroseconds(2000);
            //vTaskDelay( 2 / portTICK_PERIOD_MS );
            delay(2);
            }
            
      attachInterrupt(digitalPinToInterrupt(2), interruptHandler, RISING); // Включаем прерывание обратно
         
        
          //ACOMP_attachInterrupt(interruptHandler, RISING_TRIGGER);
        //}
    }
}

void TaskBlink2(void *pvParameters)  {
    while(1)
    {
      //vTaskDelay( 1 / portTICK_PERIOD_MS );
      if(adr == 230){             // Если сигнал "красный" - стоим, стопы не горят
        digitalWrite(1, HIGH);
        digitalWrite(11, LOW);
        vTaskDelay( 100 / portTICK_PERIOD_MS );
        }
      else if(adr == 153){        // Если сигнал "зеленый" - гасим стопы и едем
        digitalWrite(1, LOW);   
        analogWrite(11, max_speed); // со скоростью max_speed
        vTaskDelay( 100 / portTICK_PERIOD_MS );
        }
    }
}

void TaskBlink3(void *pvParameters)  { // Моргаем правым поворотником плавно
     while(1){
       for(PWM = 5; PWM <= 255; PWM=PWM+21){
          analogWrite(10, PWM);                
          vTaskDelay( 100 / portTICK_PERIOD_MS );   
          }
       for(PWM = 255; PWM >= 5; PWM=PWM-21){
          analogWrite(10, PWM);                
          vTaskDelay( 100 / portTICK_PERIOD_MS );   
        }
     }
}

void TaskBlink4(void *pvParameters)  {  // Левым правым поворотником резко
     while(1){
       digitalWrite(9, HIGH);
       vTaskDelay( 200 / portTICK_PERIOD_MS );
       digitalWrite(9, LOW);
       vTaskDelay( 200 / portTICK_PERIOD_MS );
     }
}
