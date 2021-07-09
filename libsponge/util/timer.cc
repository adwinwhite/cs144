
#include "timer.hh"

void Timer::start(const size_t expire_duration) {
  started_ = true;
  expire_ = expire_duration;
  passed_ = 0;
}

bool Timer::running() const {
  return started_;
}

bool Timer::expired() const {
  //Since it starts from 0, when passed_ equals expire_, it's regarded as expired
  return passed_ >= expire_;
}

void Timer::stop() {
  started_ = false;
}

void Timer::tick(const size_t duration) {
  if (started_) {
    passed_ += duration;
  }
}

void Timer::reset() {
  started_ = false;
  expire_ = 0;
  passed_ = 0;
}
