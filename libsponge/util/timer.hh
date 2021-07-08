#ifndef SPONGE_LIBSPONGE_TIMER_HH
#define SPONGE_LIBSPONGE_TIMER_HH


#include <string>

class Timer {
  private:
    bool started_{false};
    std::size_t expire_{0};
    std::size_t passed_{0};

  public:
      void start(const std::size_t expire_duration);

      bool running() const;

      bool expired() const;

      void stop();

      void tick(const size_t duration);

      void reset();

};







#endif
