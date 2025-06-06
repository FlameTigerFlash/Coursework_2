/* Main.c file generated by New Project wizard
 *
 * Created:   �� ��� 6 2025
 * Processor: 80C31
 * Compiler:  SDCC for 8051
 */
 
#include <mcs51reg.h>
#include <stdint.h>
#include <8051.h>
#include <math.h>
#include <stdio.h>

char buffer[50];

 
 void delay_t(uint32_t ms){//��������
	for (uint32_t i = 0; i < ms; i++)
	   {
	      for (int j = 0; j < 85; j++);
	   }
}

void UART_Init() {
    TMOD = 0x20;
    TH1 = 0xFD;
    SCON = 0x50;
    TR1 = 1;
   TL1=0xFD;
}


char UART_Receive() {
    while (!RI); 
    RI = 0;
    return SBUF;
}

int UART_is_not_Empty(){
	return RI;
}

void putc(uint8_t input){
	SBUF = input;
	while (TI != 1){}
	TI = 0;
}

void send_string(const char *str) {
    while (*str) {
        putc(*str++);
    }
}

int calculate_humidity(unsigned char adc_value) {
    float val = adc_value;
   return (val - 41) / 12.0 * 100.0;
}

void main(void)
 { 
    P2=2;
    delay_t(100);
    P2=3;
    UART_Init();
   while (1)
   {
      float hum = calculate_humidity(P1);
      sprintf(buffer, "Humidity: %.d%%\r\n", (P1 - 41) * 100 / 12);
      send_string(buffer);
      delay_t(1000);
   }
 }