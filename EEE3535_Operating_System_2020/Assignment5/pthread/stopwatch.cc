#include <iostream>
#include <cstring>
#include "stopwatch.h"

using namespace std;

stopwatch_t::stopwatch_t() : elapsed_time(0.0) {}
stopwatch_t::~stopwatch_t() {}

void stopwatch_t::start() { gettimeofday(&start_time, 0); }

void stopwatch_t::stop()  { gettimeofday(&end_time,   0); }

void stopwatch_t::reset() {
    elapsed_time = 0.0;
    memset(&start_time, 0, sizeof(timeval));
    memset(&end_time,   0, sizeof(timeval));
}

void stopwatch_t::display(stopwatch_unit m_stopwatch_unit) {
    elapsed_time += 
        ((end_time.tv_sec-start_time.tv_sec)*1e3 +
         (end_time.tv_usec-start_time.tv_usec)/1e3);

    if(!m_stopwatch_unit) {
             if(elapsed_time > 1e3)  { m_stopwatch_unit = sec;  }
        else if(elapsed_time < 1e-3) { m_stopwatch_unit = usec; }
    }

    switch(m_stopwatch_unit) {
        case sec: {     // time in seconds
            cout << "Elapsed time = " << elapsed_time/1e3 << " sec" << endl; break;
        }
        case usec: {    // time in microseconds
            cout << "Elapsed time = " << elapsed_time*1e3 << " usec" << endl; break;
        }
        default: {      // time in milliseconds
            cout << "Elapsed time = " << elapsed_time << " msec" << endl; break;
        }
    }
}

