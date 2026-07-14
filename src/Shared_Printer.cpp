#include <uhd/utils/safe_main.hpp>
#include <uhd/utils/thread.hpp>
#include <boost/program_options.hpp>
#include <mutex>
#include <thread>
#include <chrono>
#include <csignal>
#include <sstream>
#include "Shared_Printer.h"
void SharedPrinter::print(std::string message) 
{
        std::lock_guard<std::recursive_mutex> scoped_lock(this->mtx);
        std::cout << message << std::endl;
}
void SharedPrinter::lock() 
{
        this->mtx.lock();
}
void SharedPrinter::unlock() 
{
        this->mtx.unlock();
}
