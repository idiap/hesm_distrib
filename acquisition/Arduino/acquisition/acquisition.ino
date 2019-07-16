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
  case 6: // 'AGB'
    red = amber;
    break;
  }
  // declare input/output
  pinMode(trigger, INPUT);
  pinMode(red, OUTPUT);
  pinMode(green, OUTPUT);
  pinMode(blue, OUTPUT);
  pinMode(amber, OUTPUT);
  pinMode(uv, OUTPUT);
  pinMode(white, OUTPUT);
  // Debug prints
  //  Serial.begin(9600);
  digitalWrite(blue, LOW);
  digitalWrite(green, LOW);
  digitalWrite(red, LOW);
  digitalWrite(amber, LOW);
  digitalWrite(uv, LOW);
  digitalWrite(white, LOW);  

}

// vars
int led_time = 10;  // this var HAS to be equal to 1/3 of the camera's exposure time
int is_low = 1;
int force_ON = 0;  // 0 = HESM, 1 = RGB, 2 = just white

void loop() 
{
  if(force_ON == 1) // this is useful in case you want to keep the light ON to set up the sample under the microscope 
  {
    digitalWrite(red, HIGH);
    digitalWrite(green, HIGH);
    digitalWrite(blue, HIGH);  
    delay(3*led_time);
  }
  else if (force_ON == 2)
  {
    digitalWrite(white, HIGH);
  }
  else if ( digitalRead(trigger) == 1)
  {
    if (is_low == 1) // rising edge?
    { // yes
        // first led
        digitalWrite(red, HIGH);
        delay(led_time);
        // second led
        digitalWrite(green, HIGH); // switch new led ON before switching off
        digitalWrite(red, LOW);
        delay(led_time);
        // third led
        digitalWrite(blue, HIGH); // switch new led ON before switching off
        digitalWrite(green, LOW);
        delay(led_time);
        // switch aqua led off
        digitalWrite(blue, LOW);
    }
    is_low = 0;
  }
  else
  {
    is_low = 1;
  }
}
