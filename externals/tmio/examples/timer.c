/*==> MIZZI Computer Software  <=================================//

//=== Timing Functions ====================================//

//--- Version ---------------------------------------------------//

Version:  1.0 
Date:     November 2010

//--- Copyright --------------------------------------------------//

Copyright (C) 1992, 1993, 2006 by MIZZI Computer Software GmbH, 
Thomas Kihm, Mannheim, Germany.     

This software is furnished under a license and may be used    
and copied only in accordance with the terms of such          
license and with the inclusion of the above copyright         
notice.                                                       

This software or any other copies thereof may not be          
provided or otherwise made available to any other person.     
No title to and ownership of the software is hereby           
transferred.                                                  

The information in this software is subject to change         
without notice and should not be construed as a commitment 
by the holder of the copyright.                    

//----------------------------------------------------------------*/



/*=== Include =====================================================//

#include "timer.h"

//----------------------------------------------------------------*/

#include <time.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdio.h>

/*+++ Header +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/


/*=== Function ===================================================*/

double timer(double offset) 

/*--- Description ------------------------------------------------//

Returns the elapsed time minus a specified offset.

//----------------------------------------------------------------*/
{
  static long day,day0=0,sec,sec0=0,usec,usec0=0;
  struct timeval tv;
  gettimeofday(&tv,NULL);
  day=tv.tv_sec/86400;
  sec=tv.tv_sec%86400;
  usec=tv.tv_usec;
  if(day0==0) day0=day,sec0=sec,usec0=usec;
  double rc=(86400.0*(day-day0)+(sec-sec0)+(usec-usec0)/1.0e6)-offset;
  //if(rc>1e6)  printf("%lu %lu day %lu %lu sec %lu %lu usec %lu %lu \n",
  //   (unsigned long)tv.tv_sec,(unsigned long)tv.tv_usec,day,day0,sec,sec0,usec,usec0);
  return rc;
}

/*=== Function ===================================================*/

double systemtime(int what)

/*--- Description ------------------------------------------------//

Returns the current system in secs.

//----------------------------------------------------------------*/
{
  struct timeval tv;
  gettimeofday(&tv,NULL);
  switch(what)
  {
    case 1: return tv.tv_sec;
    case 2: return tv.tv_usec/1e6; 
  }
  return (tv.tv_sec+tv.tv_usec/1e6);
}



/*=== Function ===================================================*/

void udelay(unsigned int us)

/*--- Description ------------------------------------------------//

Delays execution for us mico seconds.  

//----------------------------------------------------------------*/
{
  double s=timer(0);
  while(timer(s)*1000000<us);
}

/*=== Function ===================================================*/

int waitinput(int msec)

/*--- Description ------------------------------------------------//

wait for msec milli seconds until input data is present on stdin. 
No data is read from input. 

If msec is 0 stdin is polled for data/ 

Returns > 0 if input is present on time out.    

//----------------------------------------------------------------*/

{
  int n;
  struct timeval tv;
  fd_set readmask;
  tv.tv_sec=msec/1000;
  tv.tv_usec=1000*msec%1000000;
  FD_ZERO(&readmask);
  FD_SET(0,&readmask);
  n=select(1,&readmask,0,0,&tv);
  if(n<0) return 0;
  return n;
}


/*
#ifdef   CLOCKS_PER_SEC
#if      CLOCKS_PER_SEC < 1000
#define  CLOCKTICKS  CLOCKS_PER_SEC
#endif
#endif
*/
// #ifndef  CLOCKTICKS
#include <sys/resource.h>
#include <unistd.h>
// #endif

/*=== Function ===================================================*/

double cputime(double offset)

/*--- Description ------------------------------------------------//

Returns the CPU time minus a specified offset.

//----------------------------------------------------------------*/

{

long sec,nsec;
double d;


#ifdef CLOCKTICKS
  long clock();
  long i=clock();        
  sec=(i/CLOCKTICKS);
  nsec=(i%CLOCKTICKS)*(1000000000/CLOCKTICKS);
#else
  long ts,tus;
  struct rusage info;
  getrusage(RUSAGE_SELF,&info);
  ts=info.ru_utime.tv_sec+info.ru_stime.tv_sec;
  tus=info.ru_utime.tv_usec+info.ru_stime.tv_usec;
  if(tus>=1000000) ts++,tus-=1000000;
  sec=ts;
  nsec=tus*1000;

#endif

d=sec+nsec/1e9;
return d-offset;
}


double twall = 0.0;
double tcpu = 0.0;


/*=== Function ===================================================*/

void init_benchmark_statistics(void)

/*--- Description ------------------------------------------------//

Initialises two timers (wall, cpu) for benchmarking.

//----------------------------------------------------------------*/

{
  twall = timer(0);
  tcpu = cputime(0);
}


/*=== Function ===================================================*/

void print_benchmark_statistics(long n_messages, long payload_size, long total_size)

/*--- Description ------------------------------------------------//

Prints statistics derived from two timers (wall, cpu).

//----------------------------------------------------------------*/

{
  twall = timer(twall);
  tcpu = cputime(tcpu);
  fprintf(stderr, "%6ld Byte/msg, %3.0f%% CPU, %7.0f msgs/s, %4.0f MByte/s\n",
          payload_size, 100.0 * tcpu / twall, n_messages / twall,
          (double) total_size / twall / 1.0e6);
}

/*+++ Header +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
#ifdef __cplusplus
}
#endif // __cplusplus
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
