/*
 A C-program for MT19937-64 (2004/9/29 version).
 Coded by Takuji Nishimura and Makoto Matsumoto.

 This is a 64-bit version of Mersenne Twister pseudorandom number
 generator.

 Before using, initialize the state by using init_genrand64(seed)
 or init_by_array64(init_key, key_length).

 Copyright (C) 2004, Makoto Matsumoto and Takuji Nishimura,
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions
 are met:

 1. Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.

 2. Redistributions in binary form must reproduce the above copyright
 notice, this list of conditions and the following disclaimer in the
 documentation and/or other materials provided with the distribution.

 3. The names of its contributors may not be used to endorse or promote
 products derived from this software without specific prior written
 permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 References:
 T. Nishimura, ``Tables of 64-bit Mersenne Twisters''
 ACM Transactions on Modeling and
 Computer Simulation 10. (2000) 348--357.
 M. Matsumoto and T. Nishimura,
 ``Mersenne Twister: a 623-dimensionally equidistributed
 uniform pseudorandom number generator''
 ACM Transactions on Modeling and
 Computer Simulation 8. (Jan. 1998) 3--30.

 Any feedback is very welcome.
 http://www.math.hiroshima-u.ac.jp/~m-mat/MT/emt.html
 email: m-mat @ math.sci.hiroshima-u.ac.jp (remove spaces)
 */

/* Author:  G. Jungman */

#ifndef __GSL_SF_H__
#define __GSL_SF_H__

#include <gsl/gsl_sf_airy.h>
#include <gsl/gsl_sf_bessel.h>
#include <gsl/gsl_sf_clausen.h>
#include <gsl/gsl_sf_coupling.h>
#include <gsl/gsl_sf_coulomb.h>
#include <gsl/gsl_sf_dawson.h>
#include <gsl/gsl_sf_debye.h>
#include <gsl/gsl_sf_dilog.h>
#include <gsl/gsl_sf_elementary.h>
#include <gsl/gsl_sf_ellint.h>
#include <gsl/gsl_sf_elljac.h>
#include <gsl/gsl_sf_erf.h>
#include <gsl/gsl_sf_exp.h>
#include <gsl/gsl_sf_expint.h>
#include <gsl/gsl_sf_fermi_dirac.h>
#include <gsl/gsl_sf_gamma.h>
#include <gsl/gsl_sf_gegenbauer.h>
#include <gsl/gsl_sf_hyperg.h>
#include <gsl/gsl_sf_laguerre.h>
#include <gsl/gsl_sf_lambert.h>
#include <gsl/gsl_sf_legendre.h>
#include <gsl/gsl_sf_log.h>
#include <gsl/gsl_sf_mathieu.h>
#include <gsl/gsl_sf_pow_int.h>
#include <gsl/gsl_sf_psi.h>
#include <gsl/gsl_sf_synchrotron.h>
#include <gsl/gsl_sf_transport.h>
#include <gsl/gsl_sf_trig.h>
#include <gsl/gsl_sf_zeta.h>

#endif /* __GSL_SF_H__ */
#include <stdio.h>
#include <time.h>
#include <math.h>

#define NN 312
#define MM 156
#define PI 3.14159265
#define MATRIX_A 0xB5026F5AA96619E9ULL
#define UM 0xFFFFFFFF80000000ULL /* Most significant 33 bits */
#define LM 0x7FFFFFFFULL /* Least significant 31 bits */

//**************************************************************************************************************
//**                                                  RANDOM GENERATOR                                        **
//**************************************************************************************************************

/* The array for the state vector */
static unsigned long long mt[NN];
/* mti==NN+1 means mt[NN] is not initialized */
static int mti=NN+1;

/* initializes mt[NN] with a seed */
void init_genrand64(unsigned long long seed)
{
    mt[0] = seed;
    for (mti=1; mti<NN; mti++)
        mt[mti] =  (6364136223846793005ULL * (mt[mti-1] ^ (mt[mti-1] >> 62)) + mti);
}

/* initialize by an array with array-length */
/* init_key is the array for initializing keys */
/* key_length is its length */
void init_by_array64(unsigned long long init_key[],
					 unsigned long long key_length)
{
    unsigned long long i, j, k;
    init_genrand64(19650218ULL);
    i=1; j=0;
    k = (NN>key_length ? NN : key_length);
    for (; k; k--) {
        mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 62)) * 3935559000370003845ULL))
		+ init_key[j] + j; /* non linear */
        i++; j++;
        if (i>=NN) { mt[0] = mt[NN-1]; i=1; }
        if (j>=key_length) j=0;
    }
    for (k=NN-1; k; k--) {
        mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 62)) * 2862933555777941757ULL))
		- i; /* non linear */
        i++;
        if (i>=NN) { mt[0] = mt[NN-1]; i=1; }
    }

    mt[0] = 1ULL << 63; /* MSB is 1; assuring non-zero initial array */
}

/* generates a random number on [0, 2^64-1]-interval */
unsigned long long genrand64_int64(void)
{
    int i;
    unsigned long long x;
    static unsigned long long mag01[2]={0ULL, MATRIX_A};

    if (mti >= NN) { /* generate NN words at one time */

        /* if init_genrand64() has not been called, */
        /* a default initial seed is used     */
        if (mti == NN+1)
            init_genrand64(5489ULL);

        for (i=0;i<NN-MM;i++) {
            x = (mt[i]&UM)|(mt[i+1]&LM);
            mt[i] = mt[i+MM] ^ (x>>1) ^ mag01[(int)(x&1ULL)];
        }
        for (;i<NN-1;i++) {
            x = (mt[i]&UM)|(mt[i+1]&LM);
            mt[i] = mt[i+(MM-NN)] ^ (x>>1) ^ mag01[(int)(x&1ULL)];
        }
        x = (mt[NN-1]&UM)|(mt[0]&LM);
        mt[NN-1] = mt[MM-1] ^ (x>>1) ^ mag01[(int)(x&1ULL)];

        mti = 0;
    }

    x = mt[mti++];

    x ^= (x >> 29) & 0x5555555555555555ULL;
    x ^= (x << 17) & 0x71D67FFFEDA60000ULL;
    x ^= (x << 37) & 0xFFF7EEE000000000ULL;
    x ^= (x >> 43);

    return x;
}

/* generates a random number on [0, 2^63-1]-interval */
long long genrand64_int63(void)
{
    return (long long)(genrand64_int64() >> 1);
}

/* generates a random number on [0,1]-real-interval */
double genrand64_real1(void)
{
    return (genrand64_int64() >> 11) * (1.0/9007199254740991.0);
}

/* generates a random number on [0,1)-real-interval */
double genrand64_real2(void)
{
    return (genrand64_int64() >> 11) * (1.0/9007199254740992.0);
}

/* generates a random number on (0,1)-real-interval */
double genrand64_real3(void)
{
    return ((genrand64_int64() >> 12) + 0.5) * (1.0/4503599627370496.0);
}
//**********************************************************************************************************
//**********************************************************************************************************


//**********************************************************************************************************
//**                                                MAIN PROGRAM                                          **
//**********************************************************************************************************

//FUNCTIONS
double gsl_sf_ellint_F (double phi, double k, gsl_mode_t mode);

//DECLARATION OF FILES
//FILE *gaussian;

//MAIN FUNCTION
int main(void)
{
//gaussian = fopen("gaussian.sh", "w");

int i;
unsigned long long seed;
double dir, xpos, ypos, comp;
double a,r;
int nido = AQUI1; //kilobots en el nido
double nido_r = AQUI2; //it has to be 2(one kilobot)*(nido), that would be the area
int outside;

seed = time(NULL);
init_genrand64(seed);


printf("{\n");
printf("  #bot_states#: [\n");
printf("    {\n");
printf("      #ID#: 0,\n");
dir = 2*PI*genrand64_real2();
printf("      #direction#: %0.16f,\n",dir);
printf("      #x_position#: 500.00000000000000,\n");
printf("      #y_position#: -500.00000000000000\n");
printf("    },\n");
printf("    {\n");
printf("      #ID#: 1,\n");
dir = 2*PI*genrand64_real2();
printf("      #direction#: %0.16f,\n",dir);
printf("      #x_position#: -500.00000000000000,\n");
printf("      #y_position#: 500.00000000000000\n");
printf("    },\n");


for (i=2 ; i<= nido; i++){
printf("    {\n");
printf("      #ID#: %d,\n",i);
dir = 2*PI*genrand64_real2();
printf("      #direction#: %0.16f,\n",dir);
a = 2*PI*genrand64_real2();
r = nido_r*sqrt(genrand64_real2());
ypos = r*sin(a);
xpos = r*cos(a);
//xpos = 2*175*genrand64_real1() - 175;
printf("      #x_position#: %0.14f,\n",xpos-500);
//ypos = 2*175*genrand64_real1() - 175;
printf("      #y_position#: %0.14f\n",ypos+500);
printf("    },\n");
}


for (i=nido+1; i<149; i++){
printf("    {\n");
printf("      #ID#: %d,\n",i);
dir = 2*PI*genrand64_real2();
printf("      #direction#: %0.16f,\n",dir);
outside = 0;
while(outside < 1){
a = 2*PI*genrand64_real2();
r = 750*sqrt(genrand64_real2());
ypos = r*sin(a);
xpos = r*cos(a);
comp = sqrt(pow(xpos+500.0,2) + pow(ypos-500.0,2));
//if(fabs(xpos+nido_r) < 175 && fabs(ypos-nido_r) < 175){ //for a rectangular region
if(comp < nido_r){
//it is inside unwanted region
//if(xpos > 0 || ypos > 0){ //checking to see second quadrant
}
else{
outside = 1;
}
}
printf("      #x_position#: %0.14f,\n",xpos);
printf("      #y_position#: %0.14f\n",ypos);
printf("    },\n");
}
printf("    {\n");
printf("      #ID#: 149,\n"); //Last value
dir = 2*PI*genrand64_real2();
printf("      #direction#: %0.16f,\n",dir);
outside = 0;
while(outside < 1){
a = 2*PI*genrand64_real2();
r = 750*sqrt(genrand64_real2());
ypos = r*sin(a);
xpos = r*cos(a);
if(fabs(xpos)+500.0 < 175 && fabs(ypos)-500.0 < 175){
//it is inside unwanted region
}
else{
outside = 1;
}
}
printf("      #x_position#: %0.14f,\n",xpos);
printf("      #y_position#: %0.14f\n",ypos);
printf("    }\n"); //There is no (,)
printf("  ]\n");
printf("}\n");


//fclose(gaussian);
return 0;
}
