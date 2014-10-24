/*
 * Mechatronics Assignment 1.
 *
 * Copyright edit 2014 Brent N. Law    Brent.n.law@uscga.edu
 * Copyright 2014 Aaron P. Dahlen       APDahlen@gmail.com
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

    //Beta
    float Beta;

    int Base;
    float base_current;
    float volts_base_supply;
    float volts_base;
    char base_curr_str[10];
    
    int Collector;
    float collector_current;
    float volts_collector_supply;
    float volts_collector;
    char collector_curr_str[10];

    char line[BUF_LEN];
    
    float base_current_array[100];
    float base_current_total;
    float base_current_average ;
    
    float collector_current_array[100];
    float collector_current_total;
    float collector_current_average;

    LiquidCrystal lcd (LCD_RS,  LCD_E, LCD_D4,  LCD_D5, LCD_D6, LCD_D7);   // Yes, this is a variable!

void setup()
{

    USART_init(F_CLK, BAUD_RATE);                   // The USART code must be placed in your Arduino sketchbook
    USART_set_terminator(LINE_TERMINATOR);

    lcd.begin(16, 2);                               // Define LCD as a 2-line by 16 char device
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
    // Think of the Arduino loop as a function - use static to remember between loops...
    static uint32_t next_LCD_update_time = 0;       
    
    // current time
    uint32_t current_time;

    // clear
    lcd.clear();
    
    // Current through Base
    lcd.setCursor(0, 0);
    volts_base= analogRead(A0);
    volts_base = 5*(volts_base/1023);
    volts_base_supply = (Base*5.0)/255;
    base_current = (volts_base_supply-volts_base)/10000;
    dtostre(base_current , base_curr_str,2,0x00);
    lcd.print(base_curr_str);
    //lcd.print(volts_base);

    
    // BETA variable
    Beta = collector_current/base_current;
    lcd.setCursor(10,0);
    lcd.print(Beta);
 
    // Current through Collector  
    lcd.setCursor(0, 1);
    volts_collector= analogRead(A1);
    volts_collector = 5*(volts_collector/1023);
    volts_collector_supply = (Collector/255.0)*5;
    collector_current = (volts_collector_supply - volts_collector)/220;
    dtostre(collector_current , collector_curr_str,2,0x00);
    lcd.print(collector_curr_str); 
    //lcd.print(volts_collector);

    // ====================================================================
    // Average
    // ====================================================================
    // Base
    // 100 points
/*
    for (int i = 0; i < 100; i++)
    {
      if ( i = 0)
        base_current_array[i] = base_current ;
      
      else 
      {
        base_current_array[i] = base_current_array[i-1] ;
      } 
    }
    
    // total number
    for (int i = 0; i < 100; i++)
    {
      base_current_total = base_current_total+base_current_array[i];
    }
    
    // average
    base_current_average = base_current_total/100;
    
    // Collector =================================================
    // 100 points
    
    for (int i = 0; i < 100; i++)
    {
      if ( i = 0)
        collector_current_array[i] = collector_current ;
      
      else 
      {
        collector_current_array[i] = collector_current_array[i-1] ;
      } 
    }
    
    // total number
    for (int i = 0; i < 100; i++)
    {
      collector_current_total = collector_current_total+collector_current_array[i];
    }
    
    // Average
    collector_current_average = collector_current_total/100;
*/

    // Respond to serial commands
    if(USART_is_string())
    {
        USART_gets(line);
        USART_puts(line);
        
        line_parser(line, ',');
        get_field(line, 1);
        
        // set base voltage
        if(!strncmp(line, "DB", 2))
        {
            get_field(line, 2);
            Base = strtol(line , NULL, 10);
            analogWrite(10, Base);
        }
        // set collector voltage
        else if(!strncmp(line, "DC", 2))
        {
            get_field(line, 2);
            Collector = strtol(line , NULL, 10);
            analogWrite(9, Collector);
        }
        
        // base current
        else if(!strncmp(line, "?IB", 3))
        {
          volts_base = analogRead(A0);
          volts_base = 5*(volts_base/1023);
          volts_base_supply = (Base/255.0)*5;
          base_current = (volts_base_supply-volts_base)/10000;
          dtostre(base_current , base_curr_str,2,0x00);
          USART_puts("\n");
          USART_puts(base_curr_str);
          //snprintf(line,BUF_LEN,"B=%s",base_curr_str);
        }
        // collector current
        else if(!strncmp(line, "?IC", 3))
        {
          volts_collector= analogRead(A1);
          volts_collector = 5*(volts_collector/1023);
          volts_collector_supply = (Collector/255.0)*5;
          collector_current = (volts_collector_supply - volts_collector)/220;
          dtostre(collector_current , collector_curr_str,2,0x00);
          USART_puts("\n");
          USART_puts(collector_curr_str);
          //snprintf(line,BUF_LEN,"B=%s",collector_curr_str);
        }

        USART_puts("\n");
    }
}
