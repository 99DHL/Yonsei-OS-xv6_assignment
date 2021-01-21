#ifndef __STOPWATCH_H__
#define __STOPWATCH_H__

#include <sys/time.h>

enum stopwatch_unit { none = 0, sec, msec, usec };

class stopwatch_t {
public:
    stopwatch_t();
    virtual ~stopwatch_t();

    void start();
    void stop();
    void reset();
    void display(stopwatch_unit m_stopwatch_unit = none);

protected:
    double elapsed_time;
    timeval start_time;
    timeval end_time;
};

#endif

