#include <stdio.h>
#include <math.h>
#include <kilombo.h>

#ifdef SIMULATOR
#include <stdio.h> // for printf
#else
#include <avr/io.h>  // for microcontroller register defs
//  #define DEBUG          // for printf to serial port
//  #include "debug.h"
#endif

#define NO 0
#define YES 1
#define TIME_OFF 50

//
// declare motion variable type
typedef enum {
  STOP,
  FORWARD,
  LEFT,
  RIGHT
} motion_t;


typedef struct {
  uint8_t time_ticks;
  uint8_t random_motor;
  // Random motion of Reina et al. 2018
  uint32_t last_motion_ticks;
  uint8_t turning_ticks;
  uint8_t max_turning_ticks; /* constant to allow a maximum rotation of 180 degrees with \omega=\pi/5 */
  uint32_t max_straight_ticks;
  motion_t current_motion_type;

  // message variables
  // Message variables
  message_t transmit_msg;
  uint8_t new_message;
  distance_measurement_t dist;

  // gradient variables

  uint8_t new_leader;
  uint8_t my_gradient;
  uint8_t received_gradient;
  uint8_t i_saw_leader;
  uint8_t bot_saw_leader;
  uint32_t time_update;


} USERDATA;


REGISTER_USERDATA(USERDATA);

/*                   MOTION FUNCTIONS                  */

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
///////////////////////////////////////////////////////////////////


/*                        FUNCTION TO UPDATE COLOR          */
void update_color()
{
  // Set the LED color based on the gradient.
  if (mydata->my_gradient == 0)
  {
    //set_color(RGB(1, 1, 1)); // White
  }
  else if (mydata->my_gradient == 1)
  {
    set_color(RGB(2, 0, 1)); // Red
  }
  else if (mydata->my_gradient == 2)
  {
    set_color(RGB(0, 1, 2)); // Green
  }
  else if (mydata->my_gradient == 3)
  {
    set_color(RGB(0, 0, 3)); // Blue
  }
  else if (mydata->my_gradient == 4)
  {
    set_color(RGB(1, 0, 2)); // Magenta
  }
  else if (mydata->my_gradient >= 5)
  {
    set_color(RGB(2, 1, 0)); // Yellow
  }
  /*} else {
  set_color(RGB(3,3,3)); //white
}*/
}


///////////////////////////////////////////////////////

/*        MESSAGE RECEPTION AND TRANSMITTION FUNCTIONS        */

void message_rx(message_t *message, distance_measurement_t *d) {

  mydata->received_gradient = message->data[0];
  mydata->bot_saw_leader = message->data[1];
  mydata->dist = *d; // (^_^)
  mydata->new_message = 1;



}


message_t *message_tx()
{
  return &mydata->transmit_msg;
}

void update_message() {
  mydata->transmit_msg.data[0] = mydata->my_gradient;
  mydata->transmit_msg.data[1] = mydata->i_saw_leader;
  mydata->transmit_msg.crc = message_crc(&mydata->transmit_msg);
}


void activate(){
  if(kilo_ticks % 80 < 30){
    //update_color();
    if(mydata->my_gradient == 0){
      set_color(RGB(1,1,3));
    } else {
      set_color(RGB(0,2,3));
    }
    set_motion(RIGHT);
  } else {
    set_color(RGB(0,0,0));
    set_motion(STOP);
  }
}


//////////////////////////////////////////////////////////////////////////

void setup() {

  rand_seed(kilo_uid + 1);
  mydata->new_message = 0;
  mydata->transmit_msg.type = NORMAL;
  mydata->transmit_msg.crc = message_crc(&mydata->transmit_msg);
  mydata->last_motion_ticks = 0;
  mydata->turning_ticks = 0;
  mydata->max_turning_ticks = 180;
  mydata->max_straight_ticks = 500;
  mydata->current_motion_type = STOP;

  mydata->last_motion_ticks = rand_soft() % mydata->max_straight_ticks + 1;

  mydata->new_leader = 23;

  if(kilo_uid == mydata->new_leader){
    mydata->my_gradient = 0;
    mydata->i_saw_leader = YES;
  } else {
    mydata->my_gradient = 10;
    mydata->i_saw_leader = NO;
  }
  mydata->received_gradient = 0;
  update_message();
  mydata->time_update = kilo_ticks;

}


void loop() {


  if(mydata->my_gradient == 0){
    activate();
    update_message();
  }
  else{
    if(mydata->new_message == 1){
      mydata->i_saw_leader = mydata->bot_saw_leader;
      if(mydata->bot_saw_leader == YES){
        //mydata->i_saw_leader = YES;
        activate();
        update_message();
        mydata->new_message = 0;
        mydata->bot_saw_leader = NO;
      }

      if(mydata->my_gradient > mydata->received_gradient + 1){
        mydata->my_gradient = mydata->received_gradient + 1;
      }


    }

    if(kilo_ticks % 80 > 78){
      mydata->i_saw_leader = NO;
      set_color(RGB(0,0,0));
      set_motion(STOP);
      mydata->my_gradient = 10;
    }
  }

}
//*************************************** (^_^) ******************************1
#ifdef SIMULATOR
/* provide a text string for the simulator status bar about this bot */
static char botinfo_buffer[10000];
char *cb_botinfo(void)
{
  char *p = botinfo_buffer;
  p += sprintf (p, "My ID: %d\n", mydata->my_id);
  return botinfo_buffer;
}
#endif
//****************************************** (^_^) ****************************2


int16_t circle_barrier(double x, double y, double * dx, double * dy)
{
  double d = sqrt(x*x + y*y);

  if (d < 200.0)
  return 0;
  *dx = -x/d;
  *dy = -y/d;

  return 1;
}




int main() {

  SET_CALLBACK(obstacles, circle_barrier);
  /* Light sensing */
  kilo_init();
  // register message callbacks
  kilo_message_rx = message_rx;
  kilo_message_tx = message_tx;

  // register your program
  kilo_start(setup, loop);

  SET_CALLBACK(botinfo, cb_botinfo); // (^_^)
  SET_CALLBACK(reset, setup);


  return 0;
}
