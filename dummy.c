#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "hardware/ticks.h"
#include "pico/multicore.h"
#include "dummy.pio.h"
#include "time.h"
#include <string.h>

//flags                            
volatile bool interrupt_flag = false;
volatile bool decrementing_timer_fired = false; // flag for the decrementing the counter
volatile bool repeating_timer_fired = false;

//pins 
const int data = 27; // data pin on shift register
const int SRCLK = 26; // shift register clock
const int latch = 16; // latch pin for outputting the data
const int led1 = 18; // GPIO 18 
const int minutebutton = 12;
const int set = 13;
const int buzzer = 8; // pin for the buzzer
int current_time[4] = {1,2,0,9};
                                // A,B,C,D,E,F,G,.
const int sevenseg[24][8] =     { {1,1,1,1,1,1,0,0}, //0
                                  {0,1,1,0,0,0,0,0}, //1
                                  {1,1,0,1,1,0,1,0}, //2
                                  {1,1,1,1,0,0,1,0}, //3
                                  {0,1,1,0,0,1,1,0}, //4
                                  {1,0,1,1,0,1,1,0}, //5
                                  {1,0,1,1,1,1,1,0}, //6
                                  {1,1,1,0,0,0,0,0}, //7
                                  {1,1,1,1,1,1,1,0}, //8
                                  {1,1,1,0,0,1,1,0}, //9
                                  {1,1,1,0,1,1,1,0}, //A
                                  {0,0,1,1,1,1,1,0}, //b
                                  {1,0,0,1,1,1,0,0}, //C
                                  {0,1,1,1,1,0,1,0}, //d
                                  {1,0,0,1,1,1,1,0}, //E
                                  {1,0,0,0,1,1,1,0}, //F
                                  {0,1,1,0,1,1,1,0}, //H
                                  {0,1,1,1,0,0,0,0}, //J
                                  {0,0,0,1,1,1,0,0}, //L
                                  {0,0,1,0,1,0,1,0}, //n
                                  {1,1,0,0,1,1,1,0}, //P
                                  {0,0,0,1,1,1,1,0}, //t
                                  {0,1,1,1,1,1,0,0}, //U
                                  {0,0,0,0,0,0,0,1}
                                  };
                                  
//displays 1 character onto the seven segment display
void display(const int num[8]){


    for (int i = 7;i>-1;i--){
      gpio_put(data,num[i]);
      sleep_us(100);
      gpio_put(SRCLK,0);
      sleep_us(100);
      gpio_put(SRCLK,1);
      sleep_us(100);    
}
  sleep_us(0.00001);
  gpio_put(latch,1);
  sleep_us(0.00001);
  gpio_put(latch,0);
}

void set_button_handler(uint gpio, uint32_t event_mask){
        interrupt_flag = true;
}
  
void add(int array[4]){
  int carry = 0;
  int maxnum;
  if (array[3] + 1 >= 10){
     array[3] = 0;
    carry = 1;
  }
  else
    array[3]++;
    
  for (int i = 2; i>-1;i--){
  
    if (i == 2)
      maxnum = 6;
    else
      maxnum = 10;
      
      
  if (array[i] + carry >= maxnum){
      carry = 1;
      array[i] = 0;
   }else{
      array[i] = array[i] + carry;
      carry = 0;     
}     
}
}

void subtract(int array[4]){
  int carry = 0;
  int maxnum;
  if (array[3] - 1 < 0){
     array[3] = 9;
    carry = 1;
  }
  else
    array[3]--;
    
  for (int i = 2; i>-1;i--){
  
    if (i == 2)
      maxnum = 5;
    else
      maxnum = 9;
      
      
  if (array[i] - carry < 0){
      carry = 1;
      array[i] = maxnum;
   }else{
      array[i] = array[i] - carry;
      carry = 0;     
}     
}
}

bool repeating_timer_callback(struct repeating_timer *t) // essentially doing 4 digit addition by 1 each time it is called
{
  add(current_time);
  return true;
}

bool decrementing_timer_callback(struct repeating_timer *t) // essentially doing 4 digit subtraction by 1 each time it is called
{ 
  decrementing_timer_fired = true;
  return true;
}

void displaychars(PIO pio, uint sm, uint offset, int array[4]){
  int num;
  for (int i = 0; i<4; i++){
        num = array[i];                
        pio_sm_put_blocking(pio, sm, (7<<i) | (7 >> (4-i))); //circular shifting to the left controls which led is turned on when displaying the character
        sleep_us(1);

        if (i == 2)
          display(sevenseg[23]); //display the dot to separate the hours and minutes
        sleep_us(0.1);
        display(sevenseg[num]);
        sleep_us(1);
        }
        }
        
void beep(int seconds){

for (int i = 0; i<seconds;i++){
  gpio_put(buzzer,1);
  sleep_ms(500);
  gpio_put(buzzer,0);
  sleep_ms(500);
  }
  }
  
int main() {

     stdio_init_all();
    //setup_default_uart();
    struct repeating_timer timer;
    struct repeating_timer decrementingtimer;
    add_repeating_timer_ms(-60000, repeating_timer_callback, NULL, &timer);
    
    //Initialize the GPIO pins
    gpio_init(data);
    gpio_init(SRCLK);
    gpio_init(latch);
    gpio_init(set);
    gpio_init(minutebutton);
    gpio_init(buzzer);
    gpio_set_dir(buzzer,GPIO_OUT);

    // Set the GPIO pin to an output direction
   // gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_set_dir(data, GPIO_OUT);
    gpio_set_dir(SRCLK,GPIO_OUT);
    gpio_set_dir(latch,GPIO_OUT);
    gpio_set_dir(buzzer,GPIO_OUT);
    gpio_set_dir(set,GPIO_IN);
    gpio_set_dir(minutebutton,GPIO_IN);
    
    gpio_pull_up(minutebutton);
    gpio_pull_up(set);
    gpio_set_irq_enabled_with_callback(set, GPIO_IRQ_EDGE_FALL, true, &set_button_handler);
    
    PIO pio;
    uint sm;
    uint offset;
  
    bool success = pio_claim_free_sm_and_add_program_for_gpio_range(&dummy_program, &pio, &sm, &offset, led1, 4, true);
    hard_assert(success);
    dummy_program_init(pio, sm, offset, led1); // initialize the state machine
    int stopwatch[4] = {0,0,0,0};
    
    while(true){
      
      if (interrupt_flag){
        gpio_set_irq_enabled_with_callback(set, GPIO_IRQ_EDGE_FALL, false, &set_button_handler); // disable the interrupt request since we will call another interrupt
        
        while(!gpio_get(set)){
          if (!gpio_get(minutebutton))
            add(stopwatch);
          displaychars(pio,sm,offset,stopwatch);
       }
          add_repeating_timer_ms(-1000, decrementing_timer_callback, NULL, &decrementingtimer); //start the stopwatch timer to decrement the time
          
          while(stopwatch[0] || stopwatch[1] || stopwatch[2] || stopwatch[3]){
            if(decrementing_timer_fired){
              subtract(stopwatch);
              decrementing_timer_fired = false;
              }
            displaychars(pio,sm,offset,stopwatch);
              }
            
              cancel_repeating_timer(&decrementingtimer);
              sleep_us(1);
              pio_sm_put_blocking(pio, sm, 15); // turn of all leds
              sleep_us(1);
              beep(10); //beep the buzzer for 10 seconds after the timer is finished


          
          
          //set flags to false as we are exiting the interrupt
          decrementing_timer_fired = false;
          interrupt_flag = false;
          gpio_set_irq_enabled_with_callback(set, GPIO_IRQ_EDGE_FALL, true, &set_button_handler); // re-enable the interrupt request for this button
        } 
  
    // if you press the set button set up the stop watch
        displaychars(pio,sm,offset,current_time);
    
 
}


  pio_remove_program_and_unclaim_sm(&dummy_program, pio, sm, offset);
  return 0;
}
