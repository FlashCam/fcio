/*==> FCIO FlashCam I/O system <========================//

//--- Version ---------------------------------------------------//

Version:  1.0 
Date:     2014

//----------------------------------------------------------------*/

/*=== General Information =======================================//

This Library is used to read and write messages with the FlashCam 
I/O system. 

The first part named FCIO structured I/O describes how to read FCIO 
data structures and items. 

The very simplified functional interface is well designed to read 
millions of short messages per sec as well it can handle on modern 
nodes (@2016) wire speed 10G ethernet tcp/ip messages with less than 
20% CPU usage. 

Data items are copied directly to a data structure which can be easily 
extended for further records and data items. The access of data items 
is managed by accessing them directly. This avoids additional overhead
by getter/setter functions and allows the maximal performance in speed. 
Data items must not be modified by other functions than those described 
here. 

The second part describes the basic low level message interface of 
FCIO. It is used to compose and transfer messages to files or to 
other nodes via tcp/ip. 

Please refer to the first part FCIO Structured I/O 
if you are reading FlashCam data only and skip the second part 
"low level message interface"

//----------------------------------------------------------------*/

/*+++ Header +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/


/*==> Include  <===================================================//

#include "fcio.h"

//----------------------------------------------------------------*/

// internal used only 

#include <stdio.h>
#include <stdlib.h>
#include "tmio.h"

static int debug=2; 

/*=== Function ===================================================*/

int FCIODebug(int level)

/*--- Description ------------------------------------------------//

Set up a a debug level for FCIO function calls 
Returns the old debug level.

0  = logging off
1  = errors on
2  = warning on 
3  = info on
>3 = debugging

If higher debugging is used you get a lot of output. Beware 
of turning it on during normal operation. 

The debug level is used for all further function calls
and may be set before initialization of a context structure. 

//----------------------------------------------------------------*/
{
  int old=debug;
  debug=level; 
  return old; 
}


///// Header /////////////////////////////////////////////////////// 

#define FCIOReadInt(x,i)        FCIORead(x,sizeof(int),&i) 
#define FCIOReadFloat(x,f)      FCIORead(x,sizeof(float),&f)
#define FCIOReadInts(x,s,i)     FCIORead(x,s*sizeof(int),(void*)(i))
#define FCIOReadFloats(x,s,f)   FCIORead(x,s*sizeof(float),(void*)(f))
#define FCIOReadUShorts(x,s,i)  FCIORead(x,s*sizeof(short int),(void*)(i))

////////////////////////////////////////////////////////////////////

///// Header /////////////////////////////////////////////////////// 

#define FCIOWriteInt(x,i)       ({ int data=(int)(i); FCIOWrite(x,sizeof(int),&data); })
#define FCIOWriteFloat(x,f)     ({ float data=(int)(f); FCIOWrite(x,sizeof(float),&data); })
#define FCIOWriteInts(x,s,i)    FCIOWrite(x,(s)*sizeof(int),(void*)(i))
#define FCIOWriteFloats(x,s,f)  FCIOWrite(x,(s)*sizeof(float),(void*)(f))
#define FCIOWriteUShorts(x,s,i) FCIOWrite(x,(s)*sizeof(short int),(void*)(i))

////////////////////////////////////////////////////////////////////


// helpers
/* used later 
static inline float **alloc2dfloats(int ny, int nx, float **data)
{
  if(data) { free(data[0]); free(data); } 
  if(ny<=0 || nx<=0 ) return 0; 
  int i;
  float **yp=(float**)calloc(ny,sizeof(void*));
  float *dp=(float*)calloc(nx*ny,sizeof(float));
  for(i=0; i<ny; i++) yp[i]= &dp[i*nx];
  return yp;
}
*/


/*=== FCIO Structured I/O =======================================//

Routines which know about the data format fo FCIO writers. 
Data reaping is done via a structure holding all possible 
data items. 

Use the following function calls to read data from an i/o stream/file
and transfer them to an internal buffer, which can be accessed by 
readers of the FCIO files/streams 

//----------------------------------------------------------------*/

/*--- Structures  -----------------------------------------------*/

#define FCIOMaxChannels 2304
#define FCIOMaxSamples  4000

typedef struct { // FlashCam envelope structure

  void *ptmio;                     // tmio stream
  int magic;                       // Magic number to validate structure

  struct {  // Readout configuration (typically once at start of run)

    int telid;                     // CTA-wide identifier of this camera
    int adcs;                      // Number of FADC channels
    int triggers;                  // Number of trigger channels
    int eventsamples;              // Number of FADC samples per trace
    int adcbits;                   // Number of bits per FADC sample
    int sumlength;                 // Number of samples of the FPGA integrator
    int blprecision;               // Precision of the FPGA baseline algorithm (1/LSB)
    int mastercards;               // Number of master cards
    int triggercards;              // Number of trigger cards
    int adccards;                  // Number of FADC cards
    int gps;                       // GPS mode flag (0: not used, 1: sync PPS and 10 MHz)

  } config;

  struct {    // Calibration data and results (typically once at start of run)

    int status;                    // Result of the calibration procedure (0: error, 1: success)
    int upsample;                  // Used upsampling factor
    int presamples;                // Used number of samples before pulse maxima
    float pulseramp;               // Used pulser amplitude (a.u.)
    float threshold;               // Used threshold for peak search (LSB)
    float pz[2304];                // Estimated deconvolution parameters
    float bl[2304];                // Average baselines during calibration (LSB)
    float pos[2304];               // Average peak positions of calibration pulses (samples)
    float max[2304];               // Average pulse heights of calibration pulses (LSB)
    float maxrms[2304];            // RMS of the pulse heights of the calib. pulses (LSB)
    float *traces[2304];           // Accessors for average traces
    float *ptraces[2304];          // Accessors for average de-convolved traces

    float tracebuf[2304 * 4002];   // internal tracebuffer don't use 
    float ptracebuf[2304 * 4002];  // internal ptracebuffer don't use 

  } calib;
 
  struct {  // Raw event

    int type;                       // 1: Generic event, 2: calibration event

    float pulser;                   // Used pulser amplitude in case of calibration event

    int timeoffset[10];             // [0] the offset in sec between the master and unix
                                    // [1] the offset in usec between master and unix 
                                    // [2] the calculated sec which must be added to the master
                                    // [3] the delta time between master and unix in usec
                                    // [4] the abs(time) between master and unix in usec 
                                    // [5-9] reserved for future use

    int deadregion[10];             // [0] start pps of the next dead window
                                    // [1] start ticks of the next dead window
                                    // [2] stop pps of the next dead window
                                    // [3] stop ticks of the next dead window
                                    // [4] maxticks of the dead window  
                                    // the values are updated by each event but 
                                    // stay at the previous value if no new dead region 
                                    // has been detected. The dead region window 
                                    // can define a window in the future  

    int timestamp[10];              // [0] Event no., [1] PPS, [2] ticks, [3] max. ticks 
                                    // [5-9] dummies reserved for future use

    unsigned short *trace[2304];    // Accessors for trace samples
    unsigned short *theader[2304];  // Accessors for traces incl. header bytes
                                    // (FPGA baseline, FPGA integrator)

    unsigned short traces[2304 * 4002];  // internal trace storage

  } event;

  struct {  // Readout status (~1 Hz, programmable)

    int status;         // 0: Errors occured, 1: no errors
    int statustime[5];  // fc250 seconds, microseconds, CPU seconds, microseconds, dummy
    int cards;          // Total number of cards (number of status data to follow)
    int size;           // Size of each status data

    // Status data of master card, trigger cards, and FADC cards (in that order)
    // the environment vars are: 
   
    // 5 Temps in mDeg
    // 5 Voltages in mV
    // 1 main current in mA
    // 1 humidity in o/oo
    // 2 Temps from adc cards in mDeg
   
    // links are int's which are used in an undefined manner
    // current adc links and trigger links contain: 
    // (one byte each MSB first) 
   
    // valleywidth bitslip wordslip(trigger)/tapposition(adc) errors 
   
    // these values should be used as informational content and can be 
    // changed in future versions  

    struct {

      unsigned int reqid, status, eventno, pps, ticks, maxticks, numenv,
                   numctilinks, numlinks, dummy;
      unsigned int totalerrors, enverrors, ctierrors, linkerrors, othererrors[5];
      int          environment[16];
      unsigned int ctilinks[4];
      unsigned int linkstates[256];

    } data[256];

  } status;

} FCIOData;


// valid record tags 
// all other tags are skipped

#define FCIOConfig 1 
#define FCIOCalib  2 
#define FCIOEvent  3 
#define FCIOStatus 4 

//----------------------------------------------------------------*/

// forward decls 
typedef void* FCIOStream; 
FCIOStream FCIOConnect(const char *name, int direction, int timeout, int buffer);
int FCIODisconnect(FCIOStream x);
int FCIOWriteMessage(FCIOStream x, int tag);
int FCIOWrite(FCIOStream x, int size, void *data);
int FCIOFlush(FCIOStream x);
int FCIOReadMessage(FCIOStream x);
int FCIORead(FCIOStream x, int size, void *data);

/*=== Function ===================================================*/

FCIOData *FCIOOpen(const char *name, int timeout, int buffer)

/*--- Description ------------------------------------------------//

Connects to a file, server or client for FCIO read data transfer.

name is the connection endpoint of the underlying TMIO/BUFIO
library. Please refer to the documentation of TMIO/BUFIO for 
more information. 

name can be: 

tcp://listen/port           to listen to port at all interfaces
tcp://listen/port/nodename  to listen to port at nodename interface
tcp://connect/port/nodename to connect to port and nodename

Any other name not starting with tcp: is treated as a file name.
 
timeout specifies the time to wait for a connection in milliseconds.
Specify 0 to return immediately (within the typical delays imposed by the
connection and OS) or -1 to block indefinitely.

buffer may be used to initialize the size (in kB) of the protocol buffers. If 0
is specified a default value will be used.

Returns a FCIOData structure or 0 on error. 

//----------------------------------------------------------------*/
{
  FCIOData *x=(FCIOData*)calloc(1,sizeof(FCIOData));
  if(!x) 
  {
    if(debug) fprintf(stderr,"FCIOInitData/ERROR: can not init structure\n"); 
    return 0; 
  }
  x->ptmio=(void*)FCIOConnect(name,'r',timeout,buffer);
  if(x->ptmio==0) 
  {
    if(debug) fprintf(stderr,"FCIOInitData: can not connect to data source %s \n",(name)?name:"(NULL)");
    free(x); 
    return 0; 
  }
  if(debug>2) fprintf(stderr,"FCIOInitData: io structure initialized, size %ld KB\n",(long)sizeof(FCIOData)/1024); 
  return x;  
}


/*=== Function ===================================================*/

int FCIOClose(FCIOData *x)

/*--- Description ------------------------------------------------//

Disconnects to any FCIOData source and closes any communication to 
the endpoint and frees all associated data. x becomes invalid 
after the function call.

returns 1 on success or 0 on error

//----------------------------------------------------------------*/
{
  FCIOStream xio=x->ptmio;   
  if(xio==0) return 0; 
  FCIODisconnect(xio);
  free(x);
  if(debug>2) fprintf(stderr,"FCIOClose: closed\n"); 
  return 1;
}

/*=== Function ===================================================*/

int FCIOGetRecord(FCIOData* x)

/*--- Description ------------------------------------------------//

Reads a record of data from remote peer or file. 
A record consist of a message tag and all data items stored under 
this tag. 

valid record tags are 

#define FCIOConfig 1 
#define FCIOCalib  2 
#define FCIOEvent  3 
#define FCIOStatus 4 

returns the tag (>0) on success or 0 on timeout and <0 on error. 

If a the data items are copied to the corresponding data structure 
FCIOData *x. You can access all items directly by the x pointer 
e.g.: x->config.adcs yields the number of adcs of camera. 

note: the structure is not complete up to now and will be extended by 
further items.  

//----------------------------------------------------------------*/
{
FCIOStream xio=x->ptmio; 
int tag=FCIOReadMessage(xio);
if(debug>4) fprintf(stderr,"FCIOGetRecord: got tag %d \n",tag);
switch(tag)
{
  case FCIOConfig:
  {
    int i; 
    FCIOReadInt(xio,x->config.adcs);
    FCIOReadInt(xio,x->config.triggers);
    FCIOReadInt(xio,x->config.eventsamples);  
    FCIOReadInt(xio,x->config.blprecision);      
    FCIOReadInt(xio,x->config.sumlength);      
    FCIOReadInt(xio,x->config.adcbits);  
    FCIOReadInt(xio,x->config.mastercards);   
    FCIOReadInt(xio,x->config.triggercards);   
    FCIOReadInt(xio,x->config.adccards);   
    FCIOReadInt(xio,x->config.gps);   
    if(debug>2) fprintf(stderr,"FCIOGetRecord: config %d/%d/%d adcs %d trigges %d samples %d adcbits %d blprec %d sumlength %d gps %d\n",
       x->config.mastercards, x->config.triggercards, x->config.adccards,
       x->config.adcs,x->config.triggers,x->config.eventsamples,x->config.adcbits,x->config.blprecision,x->config.sumlength,x->config.gps);   
    int traces=x->config.adcs+x->config.triggers;
    // check boundaries 
    for(i=0; i<traces; i++) x->event.trace[i]=&x->event.traces[2+i*(x->config.eventsamples+2)]; 
    for(i=0; i<traces; i++) x->event.theader[i]=&x->event.traces[i*(x->config.eventsamples+2)]; 
  }  
  break; 
  
  case FCIOCalib: 
  {  
    int i; 
    int adcs=x->config.adcs;
    FCIOReadInt(xio,x->calib.status);
    FCIOReadInt(xio,x->calib.upsample);
    FCIOReadInt(xio,x->calib.presamples);
    FCIOReadFloat(xio,x->calib.pulseramp); 
    FCIOReadFloat(xio,x->calib.threshold);
    FCIOReadFloats(xio,adcs,x->calib.pz);
    FCIOReadFloats(xio,adcs,x->calib.bl);
    FCIOReadFloats(xio,adcs,x->calib.pos);
    FCIOReadFloats(xio,adcs,x->calib.max);
    FCIOReadFloats(xio,adcs,x->calib.maxrms);
    int calchan=x->config.adcs;
    int calsamples=x->config.eventsamples*x->calib.upsample;
    // check the boundaries 
    FCIOReadFloats(xio,calchan*calsamples,x->calib.tracebuf);
    FCIOReadFloats(xio,calchan*calsamples,x->calib.ptracebuf); 
    for(i=0; i<adcs; i++) x->calib.traces[i]=&x->calib.tracebuf[i*calsamples]; 
    for(i=0; i<adcs; i++) x->calib.ptraces[i]=&x->calib.ptracebuf[i*calsamples]; 
    if(debug>2) fprintf(stderr,"FCIOGetRecord: calib adcs %d samples %d upsample %d\n",
      x->config.adcs,x->config.eventsamples,x->calib.upsample);   
  }
  break;

  case FCIOEvent: 
  { 
    FCIOReadInt(xio,x->event.type);   
    FCIOReadFloat(xio,x->event.pulser);   
    FCIOReadInts(xio,10,x->event.timeoffset);   
    FCIOReadInts(xio,10,x->event.timestamp);   
    FCIOReadUShorts(xio,2304*4002,x->event.traces);   
    FCIOReadInts(xio,10,x->event.deadregion);   

    if(debug>3) 
    {
      fprintf(stderr,"FCIOGetRecord: event type %d pulser %g, offset %d %d %d timestamp ",
          x->event.type,x->event.pulser,x->event.timeoffset[0],x->event.timeoffset[1],x->event.timeoffset[2]);
      int i; for(i=0;i<10;i++) fprintf(stderr," %d",x->event.timestamp[i]);
      fprintf(stderr,"\n");
    }
  }
  break;

  case FCIOStatus: 
  {
    int i,totalerrors=0;
    FCIOReadInt(xio,x->status.status); 
    FCIOReadInts(xio,5,x->status.statustime); 
    FCIOReadInt(xio,x->status.cards);  
    FCIOReadInt(xio,x->status.size);  
    for(i=0;i<x->status.cards;i++) FCIORead(xio,x->status.size,(void*)&x->status.data[i]);
    for(i=0;i<x->status.cards;i++) totalerrors+=x->status.data[i].totalerrors; 
    if(debug>2) fprintf(stderr,"FCIOGetRecord: overall status %d errors %d time pps %d ticks %d unix %d %d delta %d\n",
      x->status.status,totalerrors,x->status.statustime[0], x->status.statustime[1],x->status.statustime[2],
      x->status.statustime[3],x->status.statustime[4]);
    if(debug>2) for(i=0;i<x->status.cards;i++) 
    {
       fprintf(stderr,"FCIOGetRecord: card %d: status %d errors %d time %d %9d env ",i,
          x->status.data[i].status,x->status.data[i].totalerrors,x->status.data[i].pps,x->status.data[i].ticks);
       int i1; for (i1=0;i1<(int)x->status.data[i].numenv;i1++) fprintf(stderr,"%d ",(int)x->status.data[i].environment[i1]);  
       fprintf(stderr,"\n");
    }
  } 
  break;
}
return tag; 
}

/*=== Example reading a data with Structured I/O ==================//

// only a few items are accessed by this example 

char *fcio="datafile";
fprintf(stderr,"plot FC250b events FCIO format %s\n",fcio); 
int iotag;

FCIODebug(debug);
FCIOData *x=FCIOOpen(fcio,10000,0);
if(!x) exit(1); 

while((iotag=FCIOGetRecord(x))>0) 
{
  int i; 
  switch(iotag)
  {
    case FCIOConfig:  // a config record 
    // do something here 
    break; 

    case FCIOStatus:  // a status record 
    // do something here
    break; 

    case FCIOCalib:   // a calib record 
    // do something here 
    break; 
            
    case FCIOEvent:   // event record 
    // show some info 
    fprintf(stderr,"  adc       bl    isum-bl    tsum-bl   max-bl   pos\n");
    for(i=0;i<x->config.adcs;i++) 
    {
      // calculate baseline, integrator and trace integral  
      double bl=1.0*x->event.theader[i][0]/x->config.blprecision;
      double intsum=1.0*x->config.sumlength/x->config.blprecision*
         (x->event.theader[i][1]-x->event.theader[i][0]);
      double max=0; int imax=0; double tsum=0; int i1; 
      for(i1=0;i1<x->config.eventsamples;i1++) 
      { 
        double amp=x->event.trace[i][i1]-bl;
        if(amp>max) max=amp, imax=i1; 
        tsum+=amp;
      }
      if(max>0) fprintf(stderr,"%5d %8.2f %10g %10.2f %8.2f %5d\n",
         i,bl,intsum,tsum,max,imax);      
    }
    break; 
         
    default: 
    fprintf(stderr,"record tag %d... skipped \n",iotag); 
    break;
  }
} 

fprintf(stderr,"end of file \n");
FCIOClose(x);


//----------------------------------------------------------------*/



/*=== FCIO Low Level I/O functions ================================//

Functions for composing and transferring messages within the FCIO
stream based I/O system.  

Please refer to the first part FCIO Structured I/O 
if you are reading FlashCam data only and skip the rest of this document

//----------------------------------------------------------------*/


/*--- Structures  -----------------------------------------------*/

typedef void* FCIOStream; 

/*--- Description ------------------------------------------------//

An identifier for the FCIO connection.
This item is returned by any connection to a file or tcp/ip
stream and must be used in all further FCIO calls.  
 
//----------------------------------------------------------------*/


/*=== Function ===================================================*/

FCIOStream FCIOConnect(const char *name, int direction, int timeout, int buffer)

/*--- Description ------------------------------------------------//

Connects to a file, server or client for FCIO data transfer.

name is the connection endpoint of the underlying TMIO/BUFIO
library. Please refer to the documentation of TMIO/BUFIO for 
more information. 

Creates a connection or file, with name being a plain file name, "-" for
stdout, or

tcp://listen/port           to listen to port at all interfaces
tcp://listen/port/nodename  to listen to port at nodename interface
tcp://connect/port/nodename to connect to port and nodename

Any other name not starting with tcp: is treated as a file name.

direction must be an character 'r' or 'w' to specify the direction 
of read and write, 

timeout specifies the time to wait for a connection in milliseconds.
Specify 0 to return immediately (within the typical delays imposed by the
connection and OS) or -1 to block indefinitely.

buffer may be used to initialize the size (in kB) of the protocol buffers. If 0
is specified a default value will be used.

Returns a FCIOStream or 0 on error. 

//----------------------------------------------------------------*/
{
const char *proto="FlashCamV1";
if(name==0)  
{
  if(debug) fprintf(stderr,"FCIOConnect: endpoint not given, output will be discarded \n");
  return 0; 
}

tmio_stream *x=tmio_init(proto, timeout, buffer,0); 
if(x==0) 
{
   if(debug) fprintf(stderr,"FCIOConnect: error init TMIO structure\n");
   return 0;
} 

int rc=-1;
if(direction=='w') rc=tmio_create(x, name, timeout); 
else if(direction=='r') rc=tmio_open(x, name, timeout); 
if(rc<0)
{
  if(debug) fprintf(stderr,"FCIOConnect/ERROR: can not connect to stream %s, %s\n",
      name,tmio_status_str(x));
  tmio_delete(x); 
  return 0; 
} 

if(debug>2) fprintf(stderr,"FCIOConnect: %s connected, proto %s \n",name,proto); 
return (FCIOStream)x; 
}


/*=== Function ===================================================*/

int FCIODisconnect(FCIOStream x)

/*--- Description ------------------------------------------------//

Disconnects to any FCIOStream and closes any communication to 
the endpoint. 

//----------------------------------------------------------------*/
{

tmio_stream *xio=(tmio_stream *)x; 
if(xio==0) return 0; 
if(tmio_close(xio)<0)
{
  fprintf(stderr,"FCIODisconnect/ERROR: closing stream\n");
  return 0; 
} 

tmio_delete(xio);
if(debug>2) fprintf(stderr,"FCIODisconnect: stream closed\n"); 
return 1; 

}

/*=== Writing Messages ===========================================//

For getting the maximum speed during write messages will be composed 
on the fly. The following underlying function calls are used to 
composed FCIO messages. 

//----------------------------------------------------------------*/

/*=== Function ===================================================*/

int FCIOWriteMessage(FCIOStream x, int tag)

/*--- Description ------------------------------------------------//

starts a message with tag 
returns 1 on success or 0 on error

//----------------------------------------------------------------*/
{
tmio_stream *xio=(tmio_stream *)x;
if(xio==0) return 0; 
tmio_write_tag(xio,tag); 
if((tmio_status(xio)<0) && debug) fprintf(stderr,"FCIOWriteMessage/ERROR: writing tag %d \n",tag);
else if(debug>5)  fprintf(stderr,"FCIOWriteMessage: tag %d @ %lx \n",tag,(long)xio);
return 1; 
}


/*=== Function ===================================================*/

int FCIOWrite(FCIOStream x, int size, void *data)

/*--- Description ------------------------------------------------//

write a data item of size bytes length. 
data must point to the data buffer to transfer. 
returns 1 on success or 0 on error

//----------------------------------------------------------------*/
{
tmio_stream *xio=(tmio_stream *)x;
if(xio==0) return 0; 
tmio_write_data(xio, data, size);
if((tmio_status(xio)<0) && debug) fprintf(stderr,"FCIOWrite/ERROR: writing data of size %d\n",size);
else if(debug>5) fprintf(stderr,"FCIOWrite: size %d @ %lx \n",size,(long)xio);
return 1; 
}


/*=== Function ===================================================*/

int FCIOFlush(FCIOStream x)

/*--- Description ------------------------------------------------//

Flush all composed messages. 
 
//----------------------------------------------------------------*/
{
tmio_stream *xio=(tmio_stream *)x; 
if(xio==0) return 1; 
tmio_flush(xio); 
if(tmio_status(xio)<0)
{ 
  if(debug) fprintf(stderr,"FCIOFlush/ERROR: %s\n",tmio_status_str(xio));
  return 0; 
} 
return 1; 
}

/*=== Reading Messages ===========================================//

The following read function calls transfers written 
data messages to an user specified buffer. 

//----------------------------------------------------------------*/


/*=== Function ===================================================*/

int FCIOReadMessage(FCIOStream x)

/*--- Description ------------------------------------------------//

read message tag 
returns the tag (>0) on success or 0 on timeout and <0 on error.  

//----------------------------------------------------------------*/
{
tmio_stream *xio=(tmio_stream *)x; 
if(xio==0) return 0; 
int tag=tmio_read_tag(xio);
if(debug>5) fprintf(stderr,"FCIOReadMessage: got tag %d \n",tag);
return tag;
}


/*=== Function ===================================================*/

int FCIORead(FCIOStream x, int size, void *data)

/*--- Description ------------------------------------------------//

read a data item of size bytes length. 
data must point to the data buffer where data is copied to. 
returns 1 on success or 0 on error

//----------------------------------------------------------------*/
{
tmio_stream *xio=(tmio_stream *)x;
if(xio==0) return 0; 
tmio_read_data(xio, data, size);
if(tmio_status(xio)<0) fprintf(stderr,"FCIORead/ERROR: writing data of size %d\n",size);
else if(debug>5) fprintf(stderr,"FCIORead: size %d @ %lx \n",size,(long)xio);
return 1; 
}


/*+++ Header +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#ifdef __cplusplus
}
#endif // __cplusplus

/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/
