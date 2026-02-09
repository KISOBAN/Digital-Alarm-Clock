#include <stdio.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/timer.h"
#include "pico/multicore.h"
#include "dummy.pio.h"
#include "time.h"
                              // A,B,C,D,E,F,G,.
volatile bool interrupt_flag = false;
static uint set = 12;
static int data = 27;
static int SRCLK = 26; // shift register clock
static int latch = 16; // latch pin for outputting the data
static int led1 = 18; // GPIO 18 

int sevenseg[23][8] = { {1,1,1,1,1,1,0,0}, //0
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
                                  };
                                  
//displays 1 character onto the seven
void display(int data,int SRCLK,int latch, int num[8]){

    gpio_put(latch,0);
    for (int i = 8;i>-1;i--){
      gpio_put(data,num[i]);
      sleep_us(0.0001);
      gpio_put(SRCLK,0);
      sleep_us(0.0001);
      gpio_put(SRCLK,1);
      sleep_us(0.0001);    
}
  sleep_us(0.0001);
  gpio_put(latch,1);
  sleep_us(0.0001);
}

void set_button_handler(uint gpio, uint32_t event_mask){
  interrupt_flag = true;
  }



int select_button(int button, int maxnum){
  if (get_core_num() == 1){
    button = multicore_fifo_pop_blocking();
    maxnum = multicore_fifo_pop_blocking();
    }
  stdio_init_all();
  gpio_set_irq_enabled(set,GPIO_IRQ_EDGE_FALL,false); // disable interrupt request since we are going to use it again         
  gpio_init(button);
  gpio_set_dir(button, GPIO_IN);
  gpio_pull_up(button);


  int count = 0;
  if (gpio_get(button))
    count++;
    
  if (count == maxnum)
    count = 0;
    
   return count; 
  
  }
  
void state_machine_init(int led1, int numofdigits, int digit){
  PIO pio;
  uint sm;
  uint offset;

  bool success = pio_claim_free_sm_and_add_program_for_gpio_range(&dummy_program, &pio, &sm, &offset, led1, 4, true);
  hard_assert(success);
  dummy_program_init(pio, sm, offset, led1);

  for (unsigned int i = 0; i<numofdigits; i++){
    pio_sm_put_blocking(pio, sm, ((7<<i) | (7>>(4-i)))); //circular shifting to the left
    sleep_us(0.01);
    if (i == 1)
      sevenseg[i][7] = 1;
    display(data,SRCLK,latch,sevenseg[19]);
}
}
  
int main() {
     stdio_init_all();
    //setup_default_uart();
     int hourbutton = 10;
     int minutebutton = 11;
     
    //Initialize the GPIO pins
    gpio_init(data);
    gpio_init(SRCLK);
    gpio_init(latch);
    
    gpio_init(set);
    gpio_set_dir(set,GPIO_IN);
    gpio_pull_up(set);

   // gpio_set_irq_enabled_with_callback(set,GPIO_IRQ_EDGE_FALL,true,&set_button_handler); // attach an interrupt request if set is pressed           
    
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
  dummy_program_init(pio, sm, offset, led1);
  
  while(true){
//unsigned int a[4] = {7,11,13,14};

//setting the time
/*  if (interrupt_flag){
    select_button(hourbutton,23);
    multicore_launch_core1(&select_button);
    //pushing arguments onto fifobuffer so that 
    multicore_fifo_push_blocking_inline (minutebutton);
    multicore_fifo_push_blocking_inline(59);
    }
  */  
    
// displaying a different digit per led
  for (unsigned int i = 0; i<4; i++){
    pio_sm_put_blocking(pio, sm, ((7<<i) | (7>>(4-i)))); //circular shifting to the left controls which led is turned on when displaying the character
    sleep_us(0.00001);
    display(data,SRCLK,latch,sevenseg[i]);

}
}
  pio_remove_program_and_unclaim_sm(&dummy_program, pio, sm, offset);
  return 0;
}
