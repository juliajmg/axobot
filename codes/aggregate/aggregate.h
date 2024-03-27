
#define LOST 0
#define FOUND 3
#define FOUND1 1
#define FOUND2 2

#define NIDO AQUI

#define NO 0
#define YES 1


#define UNKNOWN 0
#define LOW 1
#define HIGH 2

#define EXPLORER 50
#define SHORT 50
#define LONG 2000

// Light sensing
#define RADIUS 500

typedef enum {
    STOP,
    FORWARD,
    LEFT,
    RIGHT
} motion_t;


typedef struct {
  message_t transmit_msg;
  uint8_t new_message;
  uint8_t my_gradient; // 0 for leader. Others get it
  uint8_t received_gradient;
  uint8_t received_id;
  uint8_t received_status;
  uint8_t my_id;
  uint8_t i_am_bot0;
  distance_measurement_t dist;
  // For random walk
  //uint8_t time_ticks;
  //uint8_t random_motor;
  // Random motion of Reina et al. 2018
  uint32_t last_motion_ticks;
  uint32_t turning_ticks;
  uint32_t max_turning_ticks; /* constant to allow a maximum rotation of 180 degrees with \omega=\pi/5 */
  uint32_t max_straight_ticks;
  motion_t current_motion_type;

  // For saving state: FOUND/LOST
  uint8_t status;
  //uint8_t num_of_signals;

  // For resting time
  uint8_t signals_lower_gradient; // antes num_of_signals
  uint8_t signals_higher_gradient;

  // For quality assesment
  uint8_t saved_quality;
  uint8_t current_quality;
  uint8_t sent_quality;
  uint8_t received_quality;
  uint8_t i_saw_bot0;
  uint8_t kilo_saw_bot0;
  uint8_t kilo_sees_bot0;
  uint32_t update_time;

  //Light sensing
  uint32_t current_light;
  uint32_t previous_light;
  // for binary random in light sensing
  uint8_t random_turn;

} USERDATA;

extern USERDATA *mydata;
//REGISTER_USERDATA(USERDATA);
