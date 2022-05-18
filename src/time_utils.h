#ifndef __TIME_UTILS_H__
#define __TIME_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus


int nsleep(double seconds);
double elapsed_time(double offset);
long utc_unix_to_gps(long utc_seconds);
long gps_unix_to_utc(long gps_seconds);
double timer_expired(double *t, const double interval);


#ifdef __cplusplus
}
#endif // __cplusplus

#endif // __TIME_UTILS_H__
