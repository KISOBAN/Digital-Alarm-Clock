#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "pico/multicore.h"
#include "dummy.pio.h"
#include "time.h"
                            
volatile bool interrupt_flag = false;

//pins 
static int data = 27; // data pin on shift register
static int SRCLK = 26; // shift register clock
static int latch = 16; // latch pin for outputting the data
static int led1 = 18; // GPIO 18 
static int hourbutton = 11;
static int minutebutton = 12;
static uint set = 13;

int current_time[4] = {0,0,5,0};
                                // A,B,C,D,E,F,G,.
int sevenseg[24][8] =           { {1,1,1,1,1,1,0,0}, //0
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
void display(int num[8]){


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

bool repeating_timer_callback(struct repeating_timer *t) // essentially doing 4 digit addition by 1 each time it is called
{
  int carry = 0;
  int maxnum;
  if (current_time[3] + 1 >= 10){
     current_time[3] = 0; // we increment it later
    carry = 1;
  }
  else
    current_time[3]++;
    
  for (int i = 2; i>-1;i--){
  
    if (i == 2)
      maxnum = 6;
    else
      maxnum = 10;
      
      
    if (current_time[i] + carry >= maxnum){
      carry = 1;
      current_time[i] = 0;
   }else{
      current_time[i] = current_time[i] + carry;
      carry = 0;     
}     
} 
  return true;
}

int main() {

     stdio_init_all();
    //setup_default_uart();

    //Initialize the GPIO pins
    gpio_init(data);
    gpio_init(SRCLK);
    gpio_init(latch);
    struct repeating_timer timer;
    add_repeating_timer_ms(-1000, repeating_timer_callback, NULL, &timer);
    // Set the GPIO pin to an output direction
   // gpio_set_dir(LED_PIN, GPIO_OUT);
    gpio_set_dir(data, GPIO_OUT);
    gpio_set_dir(SRCLK,GPIO_OUT);
    gpio_set_dir(latch,GPIO_OUT);
    
    PIO pio;
    uint sm;
    uint offset;
  
    bool success = pio_claim_free_sm_and_add_program_for_gpio_range(&dummy_program, &pio, &sm, &offset, led1, 4, true);
    hard_assert(success);
    dummy_program_init(pio, sm, offset, led1); // initialize the state machine
    int num = 0;
    while(true){
        
    // displaying a different digit per led

    for (int i = 0; i<4; i++){
      num = current_time[i];
      pio_sm_put_blocking(pio, sm, (7<<i) | (7 >> (4-i))); //circular shifting to the left controls which led is turned on when displaying the character
      sleep_us(1);
      display(sevenseg[num]);
      sleep_us(1);
 
}



}
  pio_remove_program_and_unclaim_sm(&dummy_program, pio, sm, offset);
  return 0;
}
