#include <16F877A.h>
#device adc=10

#FUSES NOWDT                    //No Watch Dog Timer
#FUSES HS                       //High speed Osc (> 4mhz for PCM/PCH) (>10mhz for PCD)
#FUSES NOPUT                    //No Power Up Timer
#FUSES NOPROTECT                //Code not protected from reading
#FUSES NODEBUG                  //No Debug mode for ICD
#FUSES NOBROWNOUT               //No brownout reset
#FUSES NOLVP                    //No low voltage prgming, B3(PIC16) or B5(PIC18) used for I/O
#FUSES NOCPD                    //No EE protection
#FUSES NOWRT                    //Program memory not write protected
#FUSES RESERVED                 //Used to set the reserved FUSE bits

#use delay(clock=20000000)
#use rs232(baud=115200,parity=N,xmit=PIN_C6,rcv=PIN_C7,bits=8,stream=Wireless)

#ifndef lcd_enable
#define lcd_enable     pin_E1      // pino enable do LCD
#define lcd_rs         pin_E2      // pino rs do LCD
//#define lcd_rw      pin_e2      // pino rw do LCD
#define lcd_d4         pin_d4      // pino de dados d4 do LCD
#define lcd_d5         pin_d5      // pino de dados d5 do LCD
#define lcd_d6         pin_d6      // pino de dados d6 do LCD
#define lcd_d7         pin_d7      // pino de dados d7 do LCD
#endif
#include "mod_lcd.c"


signed int16 somatorioPID = 0.0, 
proporcional = 0.0, integrativo = 0.0, 
derivativo = 0.0, tempRef = 0.0, erro = 0.0,rpmCooler;

int16 contador = 0;
int16  ultimaTemperatura,tempLM35;
#INT_RTCC
void  RTCC_isr(void) 
{
   if(contador <= somatorioPID) {
      output_high(PIN_C5);
   }
   else {
      output_low(PIN_C5);
   }
  
   contador++;
   
   if(contador >= 19) {
      contador = 0;
   }
}

void main()
{  
   
   setup_adc_ports(AN0_AN1_AN2_AN3_AN4);
   
   lcd_ini();  
   delay_ms(10);
   
   setup_adc(ADC_CLOCK_DIV_2);
   setup_timer_0(RTCC_INTERNAL|RTCC_DIV_1|RTCC_8_bit);      //51,2 us overflow
   setup_timer_2(T2_DIV_BY_16,255,1);      //819 us overflow, 819 us interrupt

   setup_ccp1(CCP_PWM);
   set_pwm1_duty(512);

   enable_interrupts(INT_RTCC);
   enable_interrupts(GLOBAL);

   char strTempLM35[20];
   char strTempRef[20];
   char strErro[20];
  
   while(TRUE)
   {
      set_adc_channel(0); // Canal do pot da temperatura.
      delay_us(10);

      
      tempRef = (read_adc() * 0.0469208211143695) + 28; // Transforma o retorno do pot para valor em temperatura.
      
      set_adc_channel(2); // Multiplexa do conversor AD para ouvir o sensor LM35
      delay_us(10);

      tempLM35 =  read_adc() * 0.488758553;
      
      set_adc_channel(1); // Multiplexa do conversor AD para ouvir o pot que controla o Coooler.
      delay_us(10);
      /*
        ======================Cooler==========================
      */
      rpmCooler = read_adc(); // 0 ---- 1023
      set_pwm1_duty(rpmCooler); // PWM 50% duty = 512  || ADC 10 Bits 0 --- 1023

      erro = tempRef - tempLM35;
      proporcional = erro * 10;                                  // kp = 10;
      integrativo += erro * 0.1;                                 // ki = 0.1;
      derivativo = (ultimaTemperatura - tempLM35) * 0.1;         // kd = 0.1
      
      ultimaTemperatura = tempLM35;

      somatorioPID = proporcional + integrativo + derivativo;


      printf(lcd_escreve, "\fRef=%Ld  Erro=%Ld\n\rTemp=%Ld", tempRef, erro, tempLM35);
     
      
      delay_ms(100);
      
      sprintf(strTempLM35, "%ld", ultimaTemperatura); 
      puts(strTempLM35);
     // delay_ms(100);
  
      
      sprintf(strErro, "%ld", erro); 
      puts(strErro);
      
    

      sprintf(strTempRef, "%ld", tempRef); 
      puts(strTempRef);
   
   }
}
