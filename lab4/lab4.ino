/*
 * Finite State Machine (FSM) and Interrupt Service Routine (ISR) template.
 *
 * Copyright 2014 Aaron P. Dahlen 
 *
 *     APDahlen@gmail.com
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

/** @Warning - there is a bug between the timer ISR and the Arduino analogWrite
 * function.  If you need to use the PWM from within the ISR then use:
 *
 * VOLATILE
 *C's volatile keyword is a qualifier that is applied to a variable when it is declared. 
 *It tells the compiler that the value of the variable may change at any time--without any
 *action being taken by the code the compiler finds nearby.
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
//     http://www.nongnu.org/avr-libc/user-manual/modules.html
//     https://www.gnu.org/software/libc/manual/

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



// Global variables

    char line[BUF_LEN];

    LiquidCrystal lcd (LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);   // Yes, this is a variable!

    volatile uint8_t start_flag = 0;

    typedef enum{initial, stop, idle, fullpower} e_states;

    volatile static e_states state = initial;

    // current time
    double STime = 0;
    
    // Tachometer variable
    double Tachometer = 0 ;
    
    // String
    char Stop_String[] = "Please type start to initiate a motor start.";
    char Idle_String[] = "CAUTION! Motor Starting!";
    char FullPower_String[] = "By your command - full power engaged!";
    
    //my personal timer
    double mytime = 0;

void setup(){

    digitalWrite(K1_PIN, LOW);
    digitalWrite(K2_PIN, LOW);
    pinMode(K1_PIN, OUTPUT);
    pinMode(K2_PIN, OUTPUT);

    pinMode(LED_PIN, OUTPUT);

    USART_init(F_CLK, BAUD_RATE);                   // The USART code must be placed in your Arduino sketchbook
    USART_set_terminator(LINE_TERMINATOR);

    init_timer_1_CTC(100);                          // Enable the timer ISR

    lcd.begin(16, 2);                               // Initialize LCD display


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




ISR(TIMER1_COMPA_vect){

/** @brief This Interrupt Service Routine (ISR) serves as the 100 Hz heartbeat 
 * for the Arduino.  See the companion function init_timer_1_CTC for additional 
 * information.
 *
 * @ Note:
 *    1) Compiler generated code pushes status register and any used registers to stack.
 *    2) Calling a subroutine from the ISR causes compiler to save all 32 registers; a 
 *       slow operation (fact check). 
 *    3) Status and used registers are popped by compiler generated code.
 */

    static uint8_t LED_flag;
    static uint16_t time_in_state = 0;
    
    // Have blinker off
    digitalWrite(13,LOW);
    
    // ONE SECOND LED CODE - blinker on for every second.
    if ( STime < millis() )
    {
      digitalWrite(13, HIGH);
      STime = millis() + 1000 ;// time+1 second
    }


    switch(state){

       case initial:
          if(++time_in_state > 100){
              state = stop;
              time_in_state = 0;
          }
          break;



        case stop:
            digitalWrite(K1_PIN, LOW);
            digitalWrite(K2_PIN, LOW);

            if (start_flag){
                start_flag = 0;
                state = idle;
                time_in_state = 0;
                digitalWrite(K1_PIN, HIGH);
                digitalWrite(K2_PIN, LOW);
            }
            break;



        case idle:

            if (analogRead(JOY_VERT) > 600){
                state = fullpower;
                time_in_state = 0;
                digitalWrite(K1_PIN, HIGH);
                digitalWrite(K2_PIN, HIGH);
            }
            break;



        case fullpower:

            if (++time_in_state > 400){
                state = stop;
                time_in_state = 0;
            }
            break;



        default:
            state = stop;
            break; 
    }
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

void loop(){

    char buf[50];
    static uint8_t last_state = !initial;
    
    // Tachometer Always
    Tachometer = analogRead(A2);
    lcd.setCursor(11, 1);
    lcd.print(Tachometer);
    
    if ( mytime < millis() )
    {
       sprintf(line, "%2d", Tachometer);
       USART_puts(strcat(line, "\n"));
       mytime = millis() + 2000; 
    }

/* This section of code is used to receive messages from the USART.  If a messages
 * is received a flag is set signaling the ISR to perform the operation.  This construct
 * was used to keep the ISR code small and fast.
 */

    switch(state){

        case stop: // Motor stop
           if(USART_is_string())
           { 
              USART_gets(buf);
              if(strcmp(buf,"start") == 0)// Zero indicates both strings are equal
              {       
                  start_flag = 1;
                  USART_puts(strcat(buf, "\n"));  // echo
              }  
              else if(strcmp(buf,"start"))// Zero indicates both strings are equal
              {       
                  USART_puts(strcat(buf, "\n"));  // echo
                  USART_puts(strcat("Improper command. Please enter \"start.\"", "\n"));
              }  
           }
            
            break;


        case idle: // Motor Idle
            break;


        case fullpower: //Motor Full Power
            break;


        default:
            break;

    }


/* This section of code executes once on change of state.  It is useful for
 * the LCD display and to send messages via the USART.
 */
    if(state != last_state){
        lcd.clear();
        last_state = state;

        switch(state){

            case initial:

                lcd.clear();
                lcd.print("Mecha FSM lab");
                lcd.setCursor(0, 1);
                lcd.print("23 Sep 14");

                USART_puts("This lab demonstrates serial control of the Arduino. It also changes motor operating states based on feedback from a tachometer. Type “start” then strike enter to initiate a motor start sequence.\n");
                USART_puts("\n");

                tone(BUZ_PIN, 554);    // C                      // Make a happy noise
                delay(200);
                tone(BUZ_PIN, 659);    // E
                delay(100);
                tone(BUZ_PIN, 784);    // G
                delay(100);
                tone(BUZ_PIN, 1047);   // C
                delay(200);
                noTone(BUZ_PIN);

                break;



            case stop: // stop STATE

                // Serial Monitor
                USART_puts("Type \"start\" to initiate a startup sequence.\n");
                
                tone(BUZ_PIN, 1047);   // C
                delay(200);
                noTone(BUZ_PIN);
                
                // LCD
                lcd.clear();
                lcd.print("Please type start to initiate a motor start.");
                lcd.setCursor(0, 1);
                lcd.print("Type start- ");

                for (int Posit = 1; Posit < 29; Posit++)
                { 
                  for (int i =0; i < 16; i++)
                  { 
                    lcd.setCursor(i, 0); 
                    lcd.print(Stop_String[Posit+i]);
                  }
                
                // slight pause
                  delay(120);
                }

                break;


            case idle: // Idle

                USART_puts("Standby, a motor start sequence has been initiated.\n");
                USART_puts("Joystick up for next state.\n");

                tone(BUZ_PIN, 1047);   // C
                delay(200);
                noTone(BUZ_PIN);
                
                lcd.clear();
                lcd.print("CAUTION! Motor Starting!");
                lcd.setCursor(0, 1);
                lcd.print("JoystickUp- ");

                for (int Posit = 1; Posit < 9; Posit++)
                { 
                  for (int i =0; i < 16; i++)
                  { 
                    lcd.setCursor(i, 0); 
                    lcd.print(Idle_String[Posit+i]);
                  }
                
                // slight pause
                  delay(120);
                }

                break;


            case fullpower: // Full Power

                USART_puts("By your command - full power engaged!\n");

                tone(BUZ_PIN, 1047);   // C
                delay(200);
                noTone(BUZ_PIN);
                
                lcd.clear();
                lcd.print("By your command - full power engaged!");
                lcd.setCursor(0, 1);
                lcd.print("Wait- ");

                for (int Posit = 1; Posit < 22; Posit++)
                { 
                  for (int i =0; i < 16; i++)
                  { 
                    lcd.setCursor(i, 0); 
                    lcd.print(FullPower_String[Posit+i]);
                  }
                
                // slight pause
                  delay(120);
                }
                
                USART_puts("Motor secured and coasting to a stop.");
                lcd.clear();
                lcd.print("Motor Secured");
                lcd.setCursor(0, 1);
                lcd.print("Wait- ");

                break;


            default:
                lcd.print("unknown state");
                break;
        }
    }
}



void init_timer_1_CTC(long desired_ISR_freq){
/**
 * @brief Configure timer #1 to operate in Clear Timer on Capture Match (CTC Mode)
 *
 *      desired_ISR_freq = (F_CLK / prescale value) /  Output Compare Registers
 *
 *   For example:
 *        Given an Arduino Uno: F_clk = 16 MHz
 *        let prescale                = 64
 *        let desired ISR heartbeat   = 100 Hz
 *
 *        if follows that OCR1A = 2500
 *
 * @param desired_ISR_freq is the desired operating frequency of the ISR
 * @param F_CLK must be defined globally e.g., #define F_CLK 16000000L
 *
 * @return void
 *
 * @note The prescale value is set manually in this function.  Refer to ATMEL ATmega48A/PA/88A/PA/168A/PA/328/P datasheet for specific settings.
 *
 * @warning There are no checks on the desired_ISR_freq parameter.  Use this function with caution.
 *
 * @warning Use of this code will break the Arduino Servo() library.
 */
    cli();                                          // Disable global
    TCCR1A = 0;                                     // Clear timer counter control registers.  The initial value is zero but it appears Arduino code modifies them on startup...
    TCCR1B = 0;
    TCCR1B |= (1 << WGM12);                         // Timer #1 using CTC mode
    TIMSK1 |= (1 << OCIE1A);                        // Enable CTC interrupt
    TCCR1B |= (1 << CS10)|(1 << CS11);              // Prescale: divide by F_CLK by 64.  Note SC12 already cleared
    OCR1A = (F_CLK / 64L) / desired_ISR_freq;       // Interrupt when TCNT1 equals the top value of the counter specified by OCR
    sei();                                          // Enable global
}
