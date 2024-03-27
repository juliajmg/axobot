#include <kilombo.h>
#include <time.h>
#include <stdio.h>
#include <math.h>


#ifdef SIMULATOR
#include <stdio.h> // for printf
#else
#include <avr/io.h>  // for microcontroller register defs
//  #define DEBUG     // for printf to serial port
//  #include "debug.h"
#endif



/* synchronization constants */

// mydata->periodo de tiempo en el que harán un comportamiento.

#define PERIOD 50
// Affects the size of the reset time adjustment for every discrepancy
// with a neighbor. A larger value means smaller adjustments. As a rule of thumb
// this value should increase with the average number of neighbors each robot has.
#define RESET_TIME_ADJUSTMENT_DIVIDER 120

// A cap on the absolute value of the reset time adjustment.

#define RESET_TIME_ADJUSTMENT_MAX 30

#define LOOP_01 10000

// declare motion variable type
typedef enum {
    STOP,
    FORWARD,
    LEFT,
    RIGHT
} motion_t;


typedef struct {

  // Random motion of Reina et al. 2018
  uint32_t last_motion_ticks;
  uint8_t turning_ticks;
  uint8_t max_turning_ticks; /* constant to allow a maximum rotation of 180 degrees with \omega=\pi/5 */
  uint32_t max_straight_ticks;
  motion_t current_motion_type;

  // Message variables
  message_t transmit_msg;
  uint8_t new_message;
  distance_measurement_t dist;

  uint32_t reset_time;
  uint32_t last_reset;
  uint8_t reset_time_adjustment;

  uint8_t bot_timer;
  uint32_t my_timer;
  uint32_t timer_discrepancy;

  uint32_t ind_reset_time_adjustment;
  uint8_t rand_start;
  uint32_t period;
  uint8_t random_speed;
  uint16_t rand_active;
  uint16_t rand_rwalk;
  uint32_t ticks_update;
} USERDATA;

REGISTER_USERDATA(USERDATA);


void smooth_set_motors(uint8_t ccw, uint8_t cw)
{
  // OCR2A = ccw;  OCR2B = cw;
#ifdef KILOBOT
  uint8_t l = 0, r = 0;
  if (ccw && !OCR2A) // we want left motor on, and it's off
    l = 0xff;
  if (cw && !OCR2B)  // we want right motor on, and it's off
    r = 0xff;
  if (l || r)        // at least one motor needs spin-up
    {
      set_motors(l, r);
      delay(15);
    }
#endif
  // spin-up is done, now we set the real value
  set_motors(ccw, cw);
}

void set_motion(motion_t new_motion)
{
  if(mydata->current_motion_type != new_motion) {
    switch(new_motion) {
    case STOP:
    default:
      smooth_set_motors(0,0);
      break;
    case FORWARD:
      smooth_set_motors(kilo_straight_left, kilo_straight_right);
      break;
    case LEFT:
      smooth_set_motors(kilo_turn_left, 0);
      break;
    case RIGHT:
      smooth_set_motors(0, kilo_turn_right);
      break;
    }
    mydata->current_motion_type = new_motion;
  }
}
void random_walk(){
   switch(mydata->current_motion_type) {
   case LEFT:
   case RIGHT:
      if(kilo_ticks > mydata->last_motion_ticks + mydata->turning_ticks) {
         /* start moving forward */
         mydata->last_motion_ticks = kilo_ticks;
         set_motion(FORWARD);
         //set_color(RGB(0,1,0));
      }
      break;
   case FORWARD:
      if( kilo_ticks > mydata->last_motion_ticks + mydata->max_straight_ticks) {
         /* perform a radnom turn */
         mydata->last_motion_ticks = kilo_ticks;
         if( rand_soft() % 2) {
            set_motion(LEFT);
            //set_color(RGB(0,0,1));
         }
         else {
            set_motion(RIGHT);
            //set_color(RGB(1,0,0));
         }
         mydata->turning_ticks = rand_soft()%mydata->max_turning_ticks + 1;
      }
      break;
   case STOP:
   default:
      set_motion(FORWARD);
   }
}


message_t *message_tx()
{
  mydata->transmit_msg.crc = message_crc(&mydata->transmit_msg);
  return &mydata->transmit_msg;


}

void update_message(){
  mydata->transmit_msg.data[0] = mydata->my_timer;

  if((kilo_ticks - mydata->last_reset) < 255){
    mydata->transmit_msg.crc = message_crc(&mydata->transmit_msg);
  } else {
    mydata->transmit_msg.crc = 0;
  }

}


void message_rx(message_t *message, distance_measurement_t *d) {
  mydata->dist = *d; // (^_^)
  mydata->new_message = 1;
  mydata->bot_timer = message->data[0];

}

void calculate_delay(){
  // Calcula el momento del periodo en el que estás
  mydata->my_timer = kilo_ticks - mydata->last_reset;
  // Resta el tiempo del periodo en el que está el bot, al tuyo
  mydata->timer_discrepancy = mydata->my_timer - mydata->bot_timer;

  // Reset la variable que indica la discrepancia de itempo con el bot del que recibiste mensaje
  mydata->ind_reset_time_adjustment = 0;

  // Si yo boy adelantado:
  if(mydata->timer_discrepancy > 0){
    // Si el vecino va atrasado por más de la mitad del mydata->periodo, mueve el tiempo de reset hacia adelante
  if(mydata->timer_discrepancy < (mydata->period / 2)){
      mydata->ind_reset_time_adjustment = mydata->timer_discrepancy;
    }
    // Si el vecino va adelantado: mueve el tiempo de reset hacia atrás (reset antes)
    else {
      mydata->ind_reset_time_adjustment = - (mydata->period - mydata->timer_discrepancy) % mydata->period;
    }
  }
  else if(mydata->timer_discrepancy < 0){
  // Si el vecino va por delante: mueve el tiempo de reset hacia atrás
    if(- mydata->timer_discrepancy < (mydata->period / 2)){
      mydata->ind_reset_time_adjustment = mydata->timer_discrepancy;
    }
    else {
      mydata->ind_reset_time_adjustment = (mydata->period + mydata->timer_discrepancy) % mydata->period;
    }
  }
  mydata->reset_time_adjustment = mydata->reset_time_adjustment + mydata->ind_reset_time_adjustment;
}


void autosync_routine(uint16_t max, uint16_t async_max){
  if(kilo_ticks % max < async_max){
    calculate_delay();

    // Si ya pasaste el tiempo del periodo P + el tiempo de ajuste después de promediar:
    if(kilo_ticks >= mydata->reset_time){
      mydata->reset_time_adjustment = (mydata->reset_time_adjustment / RESET_TIME_ADJUSTMENT_DIVIDER); 

      if(mydata->reset_time_adjustment < - RESET_TIME_ADJUSTMENT_MAX){
        mydata->reset_time_adjustment = - RESET_TIME_ADJUSTMENT_MAX;
      }
      else if(mydata->reset_time_adjustment  > RESET_TIME_ADJUSTMENT_MAX) {
        mydata->reset_time_adjustment = RESET_TIME_ADJUSTMENT_MAX;
      }

      mydata->last_reset = kilo_ticks;
      mydata->reset_time = kilo_ticks + mydata->period + mydata->reset_time_adjustment;

      mydata->reset_time_adjustment = 0;

      /* Que bailen cantidades distintas de bots */
      if(mydata->period == PERIOD) {
        set_color(RGB(2,0,3));
      } else {
        set_color(RGB(1,1,1));
      }
      set_motion(FORWARD);

    }
    else if(kilo_ticks > (mydata->last_reset + 4)){
      set_color(RGB(0,0,0));
      set_motion(STOP);
    }

    update_message();
  }
  else {
    if(kilo_ticks > mydata->ticks_update + (max - async_max)){
      mydata->ticks_update = kilo_ticks;

      mydata->rand_active = rand_soft() % 8;
      mydata->rand_rwalk = rand_soft() % 40;
  }
    if(mydata->rand_rwalk < mydata->rand_active){
      random_walk();
      set_color(RGB(0,0,1));

    } else {
      set_motion(STOP);
      set_color(RGB(0,0,0));
    }

    }
}
void setup(){

  rand_seed(kilo_uid + 1);

  mydata->random_speed = rand_soft() % 2;
  if(mydata->random_speed == 1){
    mydata->period = PERIOD;
  }
  else {
    mydata->period = PERIOD/2;
  }
  // Set the message.
  mydata->transmit_msg.type = NORMAL;
  mydata->transmit_msg.data[0] = 0;
  mydata->transmit_msg.crc = message_crc(&mydata->transmit_msg);


  mydata->last_motion_ticks = 0;
  mydata->turning_ticks = 0;
  mydata->max_turning_ticks = 192;
  mydata->max_straight_ticks = 60;
  mydata->current_motion_type = FORWARD;

  mydata->last_motion_ticks = rand_soft() % mydata->max_straight_ticks + 1;

  // Initialize timer variables to 0
  mydata->reset_time = mydata->last_reset = mydata->reset_time_adjustment = 0;
  mydata->bot_timer = mydata->my_timer = 0;
  mydata->ind_reset_time_adjustment = 0;

  mydata->rand_active = rand_soft() % 5;
  mydata->rand_rwalk = rand_soft() % 40;
  // Introduce a random delay so the robots don't become instantly
  // synchronized by the run signal.
  set_color(RGB(1, 0, 0));
  mydata->rand_start = rand_hard() % 10;
  if(kilo_ticks  == mydata->rand_start){
    set_color(RGB(0, 0, 0));
  }
}

void loop(){

  autosync_routine(3000, 1500);
}


//*************************************** (^_^) ******************************1
#ifdef SIMULATOR
/* provide a text string for the simulator status bar about this bot */
static char botinfo_buffer[10000];
char *cb_botinfo(void)
{
  char *p = botinfo_buffer;
  p += sprintf(p, "my timer: %d\n", mydata->rand_rwalk);
  p += sprintf(p, "bot state: %d\n",   mydata->rand_active);
  p += sprintf(p, "random : %u\n", mydata->ticks_update);

  p += sprintf (p, "My ID: %d\n", kilo_uid);


  return botinfo_buffer;
}

#endif

int16_t circle_barrier(double x, double y, double * dx, double * dy)
{
  double d = sqrt(x*x + y*y);

  //if (d < 200.0)
  if(d <300)
    return 0;

  *dx = -x/d;
  *dy = -y/d;

  return 1;
 }



int main() {

    SET_CALLBACK(obstacles, circle_barrier);
    // initialize hardware
    kilo_init();

    // register message callbacks
    kilo_message_rx = message_rx;
    kilo_message_tx = message_tx;
    // register your program
    kilo_start(setup, loop);

    SET_CALLBACK(botinfo, cb_botinfo); // (^_^)
    SET_CALLBACK(reset, setup);

    return(0);
}
