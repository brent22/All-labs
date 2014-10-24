/*
 * Mechatronics Lab 1.
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

    char line[BUF_LEN];

    LiquidCrystal lcd (LCD_RS,  LCD_E, LCD_D4,  LCD_D5, LCD_D6, LCD_D7);   // Yes, this is a variable!

void setup(){

    USART_init(F_CLK, BAUD_RATE);                   // The USART code must be placed in your Arduino sketchbook
    USART_set_terminator(LINE_TERMINATOR);

    sprintf(line, "Hello!\n");
    USART_puts(line);

    lcd.begin(16, 2);                               // Define LCD as a 2-line by 16 char device
    lcd.setCursor(0, 0);                            // Point to the LCD line 1 upper right character position
    lcd.print("Brent Law");
    lcd.setCursor(0, 1);                            // Point to LCD line 2 left character position
    lcd.print("Aldie,VA");
    delay(2000);
    
    // 16 spaces on the lcd display
    for (int Posit = 0; Posit < 16; Posit++)
    {
      // scroll one position right:
      lcd.scrollDisplayRight();
      // wait a bit:
      delay(31);
    }  

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
    double count = 0;
    int zero2nine = 0;

void loop(){

    USART_puts(line);
    lcd.begin(16, 2);
    lcd.setCursor(0, 0); // Point to the LCD line 1 upper right character position
    lcd.print(zero2nine);
    lcd.setCursor(0, 1);
    lcd.print(millis());
    lcd.setCursor(8,1);
    lcd.print(count);
    lcd.setCursor(0, 0);


    if (count < millis() )
    {
      if ( zero2nine < 10 )
      {
        zero2nine = zero2nine +1;
        count = millis() + 500;
      }
      else if ( zero2nine > 9 )
      {
        zero2nine = 0;
        tone(BUZ_PIN, 554); // C 
        delay (250);
        noTone(BUZ_PIN);
        count = millis() + 250;
      }
        
      lcd.print(zero2nine);
      
    }

}

