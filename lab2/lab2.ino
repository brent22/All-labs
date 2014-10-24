/*
 * Mechatronics Lab 2.
 *
 * Copyright 2014 edited by Brent N. Law       Brent.n.law@uscga.edu
 * Copyright 2014 created by Aaron P. Dahlen   APDahlen@gmail.com
 *
 * This program is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with this program.
 * If not, see <http://www.gnu.org/licenses/>.
 */



/** @Warning - there is a bug somewhere between the ISR and the Arduino analogWrite
 * function.  If you need to use the PWM from within the ISR then use:
 *
 * http://arduino.cc/en/Tutorial/SecretsOfArduinoPWM
 * 
 *    void setup(){
 *        .
 *        .
 *        .
 *        pinMode(3, OUTPUT);
 *        pinMode(11, OUTPUT);
 *        TCCR2A = _BV(COM2A1) | _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
 *        TCCR2B = _BV(CS22);                                               // This register controls PWM frequency
 *        OCR2A = 0;                                                        // Arduino I/O pin # 11 duty cycle
 *        OCR2B = 0;                                                        // Arduino I/O pin # 3 duty cycle
 *        .
 *        .
 *        .
 *    }
 *
 *    void loop(){
 *        .
 *        .
 *        .
 *        OCR2A = pin_11_PWM_DC;
 *        OCR2B = pin_3_PWM_DC;
 *        .
 *        .
 *        .
 *    }
 */
 
 
// AVR GCC libraries for more information see:
//      http://www.nongnu.org/avr-libc/user-manual/modules.html
//      https://www.gnu.org/software/libc/manual/

    #include <avr/io.h>
    #include <avr/interrupt.h>
    #include <stdint.h>
    #include <string.h>
    #include <ctype.h>

// Arduino libraries: see http://arduino.cc/en/Reference/Libraries

    #include <LiquidCrystal.h>

// Project specific includes

    #include "configuration.h"
    #include "USART.h"
    #include "line_parser.h"

// Global variables

    int k1 = 3 ;
    int k2 = 6;

    char line[BUF_LEN];
    
    // Yes, this is a variable!
    LiquidCrystal lcd (LCD_RS,  LCD_E, LCD_D4,  LCD_D5, LCD_D6, LCD_D7);   

    // Used to ensure some loops only run once
    int set = 0 ;
    
void setup()
{
    // Declare the pin
    pinMode(3, OUTPUT);
    pinMode(6, OUTPUT);
  
    // Usart Initialization
    USART_init(F_CLK, BAUD_RATE);               
    USART_set_terminator(LINE_TERMINATOR);
    
    // Initialize LCD
    lcd.begin(16, 2);                         
  
    // Ensure Relays K1 and are deenergized
    // K1 = pin 3, k2 = pin 6
    digitalWrite(k1,LOW);
    digitalWrite(k2,LOW);
    
    // Print to serial monitor and lcd
    sprintf(line, "This lab demonstrates step start control over DC motors!\n");
    USART_puts(line);
    lcd.setCursor(0, 0);  
    lcd.print("Step Start Lab");
    lcd.setCursor(0, 1);                            
    lcd.print("Brent Law");
   
}


/*********************************************************************************
 *  ______  ____   _____   ______  _____  _____    ____   _    _  _   _  _____
 * |  ____|/ __ \ |  __ \ |  ____|/ ____||  __ \  / __ \ | |  | || \ | ||  __ \
 * | |__  | |  | || |__) || |__  | |  __ | |__) || |  | || |  | ||  \| || |  | |
 * |  __| | |  | ||  _  / |  __| | | |_ ||  _  / | |  | || |  | || . ` || |  | |
 * | |    | |__| || | \ \ | |____| |__| || | \ \ | |__| || |__| || |\  || |__| |
 * |_|     \____/ |_|  \_\|______|\_____||_|  \_\ \____/  \____/ |_| \_||_____/
 *
 ********************************************************************************/

ISR(USART_RX_vect){

 /**
 * @note This Interrupt Service Routine is called when a new character is received by the USART.
 * Idealy it would have been placed in the USART.cpp file but there is a error "multiple definition 
 * of vector_18".  Apparently Arduino detects when an ISR is in the main sketch.  If you place it 
 * somewhere else it is missed and replaced with the Arduino handler.  This is the source of the 
 * multiple definitions error -  * see discussion @ http://forum.arduino.cc/index.php?topic=42153.0
 */
    USART_handle_ISR();
}

/*********************************************************************************
 *  ____            _____  _  __ _____  _____    ____   _    _  _   _  _____
 * |  _ \    /\    / ____|| |/ // ____||  __ \  / __ \ | |  | || \ | ||  __ \
 * | |_) |  /  \  | |     | ' /| |  __ | |__) || |  | || |  | ||  \| || |  | |
 * |  _ <  / /\ \ | |     |  < | | |_ ||  _  / | |  | || |  | || . ` || |  | |
 * | |_) |/ ____ \| |____ | . \| |__| || | \ \ | |__| || |__| || |\  || |__| |
 * |____//_/    \_\\_____||_|\_\\_____||_|  \_\ \____/  \____/ |_| \_||_____/
 *
 ********************************************************************************/

void loop()
{

    // 3 Seconds
    if ( millis() > 3000  && set == 0)
    {
      // clear lcd
      lcd.clear() ;
      
      // Print to serial monitor and lcd
      sprintf(line, "Standby, a motor sequence has been initiated!\n");
      USART_puts(line);
      lcd.setCursor(0, 0);  
      lcd.print("Caution!");
      lcd.setCursor(0, 1);                            
      lcd.print("Motor Starting");
      
      // Beep 3 times over 2.1 seconds
      tone(BUZ_PIN, 554);   
      delay(700);
      tone(BUZ_PIN, 659);    
      delay(700);
      tone(BUZ_PIN, 784);   
      delay(700);
      noTone(BUZ_PIN);
      
      // Ensure code runs once
      set =1;
    }
    
    // 6 Seconds
    while ( millis() > 6000 && set == 1)
    {
    // clear lcd
    lcd.clear() ;
    
    // Print to serial monitor and lcd
    sprintf(line, "Motor is accelerating!\n");
    USART_puts(line);
    lcd.setCursor(0, 0);  
    lcd.print("Motor Ramping");
      
    // Eergize relay K2
    digitalWrite(k2,HIGH);

    // Ensure code runs once
    set = 2;
    }
    
        // 9 Seconds
    while ( millis() > 9000 && set == 2)
    {
    // clear lcd
    lcd.clear() ;
    
    // Print to serial monitor and lcd
    sprintf(line, "Full power engaged.\n");
    USART_puts(line);
    lcd.setCursor(0, 0);  
    lcd.print("Motor Running");
      
    // Eergize relay K1
    digitalWrite(k1,HIGH);        
    // De-ergize relay K2
    digitalWrite(k2,LOW);

    // Ensure code runs once
    set = 3;
    }
    
    // 12 Seconds
    while ( millis() > 12000 && set == 3)
    {
    // clear lcd
    lcd.clear() ;
    
    // Print to serial monitor and lcd
    sprintf(line, "Motor Secured and coasting to a stop.\n");
    USART_puts(line);
    lcd.setCursor(0, 0);  
    lcd.print("Motor Secured");
      
    // De-ergize relay K1
    digitalWrite(k1,LOW);        
    // De-ergize relay K2
    digitalWrite(k2,LOW);

    // Ensure code runs once
    set = 4;
    }
    
}

