#include <stdio.h>
//#include <kilolib.h>
#include <math.h>
#include <kilombo.h>
#include "random_motion_clustering.h"

#ifdef SIMULATOR
#include <stdio.h> // for printf
#else
#include <avr/io.h>  // for microcontroller register defs
//  #define DEBUG          // for printf to serial port
//  #include "debug.h"
#endif


REGISTER_USERDATA(USERDATA)


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

  mydata->received_id = message->data[1];

  mydata->dist = *d; // (^_^)

  mydata->new_message = 1;

  mydata->kilo_saw_bot0 = message->data[2];

  mydata->received_quality = message->data[3];

  mydata->received_status = message->data[4];


}


message_t *message_tx()
{
  return &mydata->transmit_msg;
}

void update_message() {
  mydata->transmit_msg.data[0] = mydata->my_gradient;
  mydata->transmit_msg.data[1] = mydata->my_id;
  mydata->transmit_msg.data[2] = mydata->i_saw_bot0;
  mydata->transmit_msg.data[3] = mydata->saved_quality;
  mydata->transmit_msg.data[4] = mydata->status;

  mydata->transmit_msg.crc = message_crc(&mydata->transmit_msg);
}

/*              RANDOM MOTION FUNCTION              */

void random_walk(){
   switch(mydata->current_motion_type) {
   case LEFT:
   case RIGHT:
      if(kilo_ticks > mydata->last_motion_ticks + mydata->turning_ticks) {
         /* start moving forward */
         mydata->last_motion_ticks = kilo_ticks;
         set_motion(FORWARD);
      }
      break;
   case FORWARD:
      if( kilo_ticks > mydata->last_motion_ticks + mydata->max_straight_ticks) {
         /* perform a radnom turn */
         mydata->last_motion_ticks = kilo_ticks;
         if(rand_soft()%2) {
            set_motion(LEFT);
         }
         else {
            set_motion(RIGHT);
         }
         mydata->turning_ticks = rand_soft()%mydata->max_turning_ticks + 1;
      }
      break;
   case STOP:
   default:
      set_motion(FORWARD);
   }
}


/* Función para seguir la luz a partir de mejorar tu gradiente */
void follow_light() {

  uint32_t d0;
  uint32_t d1;

  // Mide luz del ambiente
  mydata->current_light = get_ambientlight();
  if(mydata->current_light < RADIUS && mydata->signals_lower_gradient < 10){
    //set_color(RGB(2,0,1));
    if(kilo_ticks % (mydata->turning_ticks*2 + 1) == 1) {
      //¿Cuál será el mínimo de ticks para la vuelta?
      mydata->turning_ticks = rand_soft()%mydata->max_turning_ticks + 32;
      mydata->random_turn = rand_soft() % 2;
      mydata->previous_light = mydata->current_light;
    }


    if(mydata->current_light <= mydata->previous_light){
      set_motion(FORWARD);

    }
    else {
      // Aquí hay que probar si es más eficiente un solo ángulo de giro, o random.
      if(kilo_ticks % (mydata->turning_ticks*2 + 1) < mydata->turning_ticks + 1) {

        if(mydata->random_turn == 1){
          set_motion(RIGHT);
          //set_color(RGB(0,2,2));
        } else
        {
          set_motion(LEFT);
          //set_color(RGB(3,1,2));
        }

      } else if(kilo_ticks % (mydata->turning_ticks*2 + 1) > mydata->turning_ticks + 1 && kilo_ticks % (mydata->turning_ticks*2 + 1) < (mydata->turning_ticks + 31)){
        set_motion(FORWARD);
        //set_color(RGB(2,2,0));
      } else {
          d0 = mydata->current_light;
          d1 = get_ambientlight();
          if(d1 >= d0) {
            if(mydata->random_turn == 1){
              set_motion(RIGHT);
              //set_color(RGB(2,2,2));
            } else {
              set_motion(LEFT);
              //set_color(RGB(0,0,0));
            }
          } else {
            set_motion(FORWARD);
          //  set_color(RGB(1,0,0));
          }
        }
      }
    //if(kilo_ticks % (mydata->turning_ticks*2 + 1) == 1){

    //}
  }


}



//////////////////////////////////////////////////////////////////////////

void setup() {

  mydata->new_message = 0;
  mydata->transmit_msg.type = NORMAL;
  mydata->transmit_msg.crc = message_crc(&mydata->transmit_msg);

  rand_seed(kilo_uid + 1);
  mydata->last_motion_ticks = 0;
  mydata->turning_ticks = 0;
  mydata->max_turning_ticks = 180;
  mydata->max_straight_ticks = 500;
  mydata->current_motion_type = STOP;
  mydata->signals_lower_gradient = 0;
  mydata->signals_higher_gradient = 0;

  mydata->last_motion_ticks = rand_soft() % mydata->max_straight_ticks + 1;

  /* Light sensing */
  mydata->current_light = 0;
  mydata->previous_light =  RADIUS + 10;



  // Quality of sources
  if(kilo_uid == 0) { // HIGH QUALITY SOURCE
    mydata->saved_quality = HIGH;
    mydata->status = FOUND1;
    set_color(RGB(3,0,0));
    mydata->my_id = kilo_uid;
    mydata->my_gradient = 0;
    set_motion(STOP);
    mydata->update_time = 0;
    mydata->i_saw_bot0 = YES;
  }

  if(kilo_uid == 1) { // LOW QUALITY SOURCE
    mydata->saved_quality  = LOW;
    mydata->status = FOUND2;
    set_color(RGB(0,0,3));
    mydata->my_id = kilo_uid;
    mydata->my_gradient = 0;
    set_motion(STOP);
    mydata->update_time = 0;
    mydata->i_saw_bot0 = YES;
    }

  if(kilo_uid > 1 && kilo_uid <= NIDO) {
      mydata->saved_quality = UNKNOWN;
      mydata->status = FOUND2;  //They start knowing their state
      mydata->my_id = kilo_uid;
      mydata->my_gradient = 10; //ponía 7
      mydata->i_saw_bot0 = NO;
      set_motion(STOP);
      mydata->update_time = LONG;
      update_color();
  }
  if(kilo_uid > NIDO) {
      mydata->saved_quality = HIGH;
      mydata->status = LOST;
      set_color(RGB(2,0,2));
      mydata->my_id = kilo_uid;
      mydata->my_gradient = 10; //ponía 7
      mydata->i_saw_bot0 = YES;
      mydata->update_time = EXPLORER;
  }

}

void loop() {
  if(mydata->new_message == 1) {
    mydata->new_message = 0;

    if(mydata->my_id > 1) {
      if(mydata->received_id == 0) {
         mydata->status = FOUND1;
         if(mydata->my_id > NIDO){
            mydata->i_saw_bot0 = YES;
            //set_color(RGB(2,0,2));
         }
      }
      else{ //<NEW> ANIDADO para evitar que partículas con up_t=1000 se acumulen en la fuente LOW hay que hacer que vuelvan a LOST cuando no tocan un cluster (abajo)
         if(mydata->received_id == 1) {
            mydata->status = FOUND2;
            //mydata->i_saw_bot0 = YES;
         }
         else{
            /*copia el estado que haya encontrado el bot*/
            if(mydata->received_status != LOST) {
               mydata->status = mydata->received_status;
            }
         }
      }
      //if(mydata->my_id <= NIDO){
      /* Si sabes que hay calidad alta y encuentras fuente con calidad baja*/
         if(mydata->status == FOUND2 && mydata->saved_quality == HIGH) {
        /* Disminuye tu tiempo de actualización */
            if(mydata->my_id <= NIDO){
               mydata->update_time = SHORT;
            }
            if(mydata->my_id > NIDO){
               if(mydata->update_time == LONG){
                  mydata->update_time = SHORT;
               }
            }
         }

         if(mydata->status == FOUND1) {
           set_color(RGB(0,2,0));
            if(mydata->my_id > NIDO && mydata->signals_lower_gradient > 15 && mydata->my_gradient > 7 && mydata->my_gradient < 10){
               mydata->update_time = LONG; //Here explorers are turned into nest individuals
               set_color(RGB(4,4,0));
            }
            else{
               if(mydata->my_id <= NIDO){
                  mydata->update_time = LONG;
               }
            }
         }
      //}

      if(mydata->received_quality == HIGH && mydata->kilo_saw_bot0 == YES){
        mydata->saved_quality = mydata->received_quality;

      }
      /* Gradientes */

      if(mydata->received_gradient < mydata->my_gradient) {
        /*Aquí poner numero de señales necesarias para detenerse*/
        mydata->signals_lower_gradient = mydata->signals_lower_gradient + 1;
        /*Código para cambiar gradiente */
        if(estimate_distance(&mydata->dist)<40 && mydata->signals_lower_gradient > 15){ // <NEW> AQUI CREO QUE IRÍA LA RESTA (^:_:^) pero es mejor entenderlo primero
        //if(estimate_distance(&mydata->dist)<40){
          mydata->my_gradient = mydata->received_gradient + 1;
          set_motion(STOP);
          //update_color();
        }
      }
      /*NUEVO: No solo encontrar la fuente, sino el cluster*/
      /*Corregir para que sólo suceda esto si estás en el cluster con fuente alta */
      if(mydata->status == FOUND1 && mydata->kilo_saw_bot0 == YES) { //<NEW> CON ESTAS CONDICIONES CREO QUE YA ES SUFICIENTE, found1 significa que estoy en el cluster
          //if(mydata->received_gradient < 2 && mydata->signals_lower_gradient > (8 - mydata->my_gradient)){
          if(mydata->received_gradient < 5){  // <NEW> condición que indica que estoy cerca del nucleo, la otra condición que hablamos (la resta) creo que no iría aquí
            //mydata->status = FOUND1;
            //mydata->i_saw_bot0 = YES;
            //set_color(RGB(2,0,2));
          }
      }
    }
  }
  update_message();
  if(kilo_ticks > 2000){ // Initial stabilization time
  //update_color();

  /* Light sensing */
    if(mydata->my_id > 1 && mydata->my_id <= NIDO && mydata->status != FOUND1 && mydata->my_gradient > 9) {
      follow_light();
    }

  /*-----------------------------------------*/

  if(mydata->my_gradient > 9) {
    if(mydata->signals_lower_gradient > 10) {
      set_motion(STOP);
    }
    else {
      if(mydata->saved_quality == HIGH) {
        if(mydata->current_light > RADIUS || mydata->my_id > NIDO){
         random_walk();
       }
      }
       mydata->status = LOST;
    }
  }


  if(mydata->my_id > 1){
    if(kilo_ticks % mydata->update_time == 1) {
      mydata->signals_lower_gradient = 0;
      if(mydata->my_gradient < 10) { //ponía 7
        mydata->my_gradient = mydata->my_gradient + 1;
      }
      // Sacar actualización de gradiente del tiempo de actualización? <NEW> Sí, creo que hay que considerarlo dentro
    }
    if(mydata->saved_quality == HIGH) {
      if(mydata->status == FOUND1) {
        mydata->i_saw_bot0 = YES;
      }
        //set_color(RGB(2,0,2));
    }
      else {
      //set_color(RGB(0,2,2));
        if(mydata->my_id <= NIDO){
           //set_color(RGB(2,2,0));
           //set_color(RGB(0,0,1));

        }
      }
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
  //p += sprintf (p, "My Gradient: %d\n", mydata->my_gradient);
  p += sprintf (p, "Current_light: %d\n", mydata->current_light);
  p += sprintf (p, "previous_light: %d\n", mydata->previous_light);
  p += sprintf (p, " i i_saw_bot0: %d\n", mydata->i_saw_bot0);

  //p += sprintf (p, "Update time: %d\n", mydata->update_time);
  //p += sprintf (p, "Neighbor Gradient: %d\n", mydata->received_gradient);
  //p += sprintf (p, "Signals received: %d\n", mydata->signals_lower_gradient);


  return botinfo_buffer;
}
#include <jansson.h>
json_t *json_state();
#endif
//****************************************** (^_^) ****************************2


int16_t circle_barrier(double x, double y, double * dx, double * dy)
{
  double d = sqrt(x*x + y*y);

  if (d < 750.0)
    return 0;

  *dx = -x/d;
  *dy = -y/d;

  return 1;
 }

/* Light sensing */

int16_t light(double x, double y){
  double x0 = 350.0;
  double y0 = -350.0;

  double r = sqrt((x-x0)*(x-x0) + (y-y0)*(y-y0));

  int16_t gradient;

  if(r < RADIUS) {
    gradient = (int16_t) r;
  } else {
    gradient = RADIUS + 10;
  }
  return gradient;
}



int main() {

    SET_CALLBACK(obstacles, circle_barrier);
    /* Light sensing */
    SET_CALLBACK(lighting, light);
    // initialize hardware
    kilo_init();
    // register message callbacks
    kilo_message_rx = message_rx;
    kilo_message_tx = message_tx;

    // register your program
    kilo_start(setup, loop);

    SET_CALLBACK(botinfo, cb_botinfo); // (^_^)
    SET_CALLBACK(reset, setup);
    SET_CALLBACK(json_state, json_state);


    return 0;
}
