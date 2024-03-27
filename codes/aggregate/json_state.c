/* Saving bot state as json. Not for use in the real bot, only in the simulator. */
#include <kilombo.h>

#ifdef SIMULATOR

#include <jansson.h>
#include <stdio.h>
#include <string.h>

#include "aggregate.h"




json_t *json_state()
{
  //create the state object we return
  json_t* state = json_object();

  // store the status value
  json_t* b = json_integer(mydata->i_saw_bot0);
  json_object_set(state, "Agreggated", b);
  json_t* g = json_integer(mydata->saved_quality);
  json_object_set(state, "status", g);
  json_t* m = json_integer(kilo_ticks);
  json_object_set(state, "TIME", m);



  return state;
}

#endif
