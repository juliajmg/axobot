#include <kilombo.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

//
//#include "messages.h"


#ifdef SIMULATOR
#include <stdio.h> // for printf
#else
#include <avr/io.h>  // for microcontroller register defs
// #define DEBUG     // for printf to serial port
// #include "debug.h"
#endif


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

  // Message reception and transimition variables
  message_t transmit_msg;
  uint8_t new_message;
  uint8_t my_id;
  uint8_t received_id;
  uint32_t signals;
  uint32_t contador;
  uint8_t current_tick;

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


/*        MESSAGE RECEPTION AND TRANSMITTION FUNCTIONS        */

void message_rx(message_t *message, distance_measurement_t *d) {

  //mydata->received_id = message->data[0];
  mydata->new_message = 1;

}

message_t *message_tx()
{
  return &mydata->transmit_msg;
}

void update_message() {
  mydata->transmit_msg.crc = message_crc(&mydata->transmit_msg);
}



void setup()
{

//Message reception and transmittion
  mydata->new_message = 0;
  mydata->transmit_msg.type = NORMAL;
  mydata->transmit_msg.crc = message_crc(&mydata->transmit_msg);

  //rand_seed(kilo_uid + 1);
  mydata->current_motion_type = STOP;

  set_motion(STOP);


}

void loop()

{

if(mydata->new_message == 1){
  set_color(RGB(2, 0, 1));
  set_motion(RIGHT);
  if(kilo_ticks % 100 > 95){
    mydata->new_message = 0;
  }
} else {
  set_color(RGB(0,0,0));
  set_motion(STOP);
}

}






int main() {


    // initialize hardware
    kilo_init();

    SET_CALLBACK(botinfo, cb_botinfo); // (^_^)

    SET_CALLBACK(reset, setup);
    // register message callbacks
    kilo_message_rx = message_rx;
    kilo_message_tx = message_tx;

    // register your program
    kilo_start(setup, loop);

    return(0);
}
