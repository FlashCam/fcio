
/* examples/timer.h

Generated automatically on Tue Jan 26 12:44:11 CET 2016 from examples/timer.c.
*/

#ifndef __TIMER_H__
#define __TIMER_H__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus

double timer(double offset) 
;
double systemtime(int what)
;
void udelay(unsigned int us)
;
int waitinput(int msec)
;
double cputime(double offset)
;
void init_benchmark_statistics(void)
;
void print_benchmark_statistics(long n_messages, long payload_size, long total_size)
;
#ifdef __cplusplus
}
#endif // __cplusplus

#endif  // __TIMER_H__
