// License - for Non-Commercial Research and Educational Use Only
//  
//  Copyright (c) 2019, Idiap research institute
//  
//  All rights reserved.
//  
//  Run, copy, study, change, improve and redistribute source and binary forms, with or without modification, are permitted for non-commercial research and educational use only provided that the following conditions are met:
//  
//  1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
//  
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//  
//  For permission to use for commercial purposes, please contact Idiap's Technology Transfer Office at tto@idiap.ch
// 
//  christian AT jaques DOT idiap DOT ch
// 
// INPUT/OUTPUTS
int blue = 11;
int green = 10;
int red = 9;
int white = 6;
int uv = 5;
int amber = 3;
int trigger = 2;

// which of the leds should be used ?
int leds_case = 0;

void setup() {
  // bit of a hack : below, we only use red green and blue variable names.
  // when using other leds, we just replace the red, green and blue led pin number by the other led's pins.
  switch(leds_case){
  case 0: // 'RGB'
    break;
  case 1: // 'RGUV'
    blue = uv;
    break;
   case 2: //'RUVB'
     green = uv;
     break;
   case 3: // 'UVAG' 
     red = uv;
     blue = green;
     green = amber;
     break;
   case 4: // 'BUVW'
     red = blue;
     green = uv;
     blue = white;
     break;
   case 5: // 'AGUV'
     red = amber;
     blue = uv;
     break;
  }
  // declare input/output
  pinMode(trigger,INPUT);
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT);
  // Debug prints
  //  Serial.begin(9600);
  digitalWrite(blue,LOW);
  digitalWrite(green,LOW);
  digitalWrite(red,LOW);
}

// vars
int led_time = 20;  // this var HAS to be equal to 1/3 of the camera's exposure time
int is_low = 1;
int counter = -1;

/* CALIBRATION SEQUENCE
 3 white images
 3 red_111 images
 3 red_110 images
 3 red_100 images
 3 green_111 images
 3 green_110 images
 3 green_100 images
 3 blue_111 images
 3 blue_110 images
 3 blue_100 images
 9 rgb_pattern_111 images

 TOTAL OF 45 images --> not all of them are required, but while the sample is still...
*/
void loop() {
  
  if( digitalRead(trigger) == 1)
    {
      if(is_low == 1) // rising edge?
      {// yes       
        // Increase counter
        counter++;
        
        if(counter < 3){                    // 3 white images
          digitalWrite(red,HIGH);
          digitalWrite(blue,HIGH);
          digitalWrite(green,HIGH);
          delay(3*led_time); // whole exp time
        }
        else if(counter < 6){               // red images 111 
          digitalWrite(red,HIGH);
          delay(3*led_time); // whole exp time
        }
        else if(counter < 9){               // red images 110 
          digitalWrite(red,HIGH);
          delay(2*led_time); // 2/3 exp time
        }
        else if(counter < 12){               // red images 100 
          digitalWrite(red,HIGH);
          delay(led_time); // 1/3 exp time
        }
        else if(counter < 15){               // green images 111 
          digitalWrite(green,HIGH);
          delay(3*led_time); // whole exp time
        }
        else if(counter < 18){               // green images 110 
          digitalWrite(green,HIGH);
          delay(2*led_time); // 2/3 exp time
        }
        else if(counter < 21){               // green images 100 
          digitalWrite(green,HIGH);
          delay(led_time); // 1/3 exp time
        }
        else if(counter < 24){               // blue images 111 
          digitalWrite(blue,HIGH);
          delay(3*led_time); // whole exp time
        }
        else if(counter < 27){               // blue images 110 
          digitalWrite(blue,HIGH);
          delay(2*led_time); // 2/3 exp time
        }
        else if(counter < 30){               // blue images 100 
          digitalWrite(blue,HIGH);
          delay(led_time); // 1/3 exp time
        }
        else if(counter < 36){
          delay(3*led_time);                // ALL LIGHTS OFF for 6 images
        }
        else if(counter < 45){
         // RGB pattern
          // first scarlet led
          digitalWrite(red,HIGH);
          delay(led_time);
          // then green led
          digitalWrite(red,LOW);
          digitalWrite(green,HIGH);
          delay(led_time);
          // then blue led
          digitalWrite(green,LOW);
          digitalWrite(blue,HIGH);
          delay(led_time);
          // switch blue led off
          digitalWrite(blue,LOW);
        }
        // switch all LEDs off
        digitalWrite(red,LOW);
        digitalWrite(blue,LOW);
        digitalWrite(green,LOW);
      }
      is_low = 0;
        
    }
   else
    {
      is_low = 1;
    }
}
