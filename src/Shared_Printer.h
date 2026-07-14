#pragma once

#include <uhd/utils/safe_main.hpp>
#include <uhd/utils/thread.hpp>
#include <boost/program_options.hpp>
#include <mutex>
#include <thread>
#include <chrono>
#include <csignal>
#include <sstream>
class SharedPrinter {
  private:
    std::recursive_mutex mtx;
  public:
    void print(std::string message);
    void lock();
    void unlock();
};
