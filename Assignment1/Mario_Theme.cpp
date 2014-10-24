    #include "Arduino.h"
    #include "configuration.h"

void Supermario ( void )
{
  //Define notes
  #define C    1911
  #define C1    1804
  #define D    1703
  #define Eb    1607
  #define E    1517
  #define F    1432
  #define F1    1352
  #define G    1276
  #define Ab    1204
  #define A    1136
  #define Bb    1073
  #define B    1012
  #define c       955
  #define c1      902
  #define d       851
  #define eb      803
  #define e       758
  #define f       716
  #define f1      676
  #define g       638
  #define ab      602
  #define a       568
  #define bb      536
  #define b       506
  #define p       0  //pause
   
  // set up
  int speaker = 2;  // arduino pin
  long vel = 20000; // speed
  pinMode(speaker, OUTPUT);
  delay(2000);
  int melod[] = {e, e, e, c, e, g, G, c, G, E, A, B, Bb, A, G, e, g, a, f, g, e, c, d, B, c};
  int ritmo[] = {6, 12, 12, 6, 12, 24, 24, 18, 18, 18, 12, 12, 6, 12, 8, 8, 8, 12, 6, 12, 12, 6, 6, 6, 12};
   
      for (int i=0; i<24; i++) 
      {
          int tom = melod[i];
          int tempo = ritmo[i];
   
          long tvalue = tempo * vel;
   
          long tempo_gasto = 0;
            while (tempo_gasto < tvalue)
            {
              digitalWrite(speaker, HIGH);
              delayMicroseconds(tom / 2);
       
              digitalWrite(speaker, LOW);
              delayMicroseconds(tom/2);
       
              tempo_gasto += tom;
            }
   
          delayMicroseconds(1000); //pause the notes
      }   
      delay(1000);
return;
}


