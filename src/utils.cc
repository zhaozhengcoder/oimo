#include <ctime>
#include <string>
#include "utils.h"

namespace Oimo {

std::string getCurrentTime() {
    time_t now = time(nullptr);
    tm *ltm = localtime(&now);
    std::string time(30, '\0');
    strftime(&time[0], time.size(), "%Y-%m-%d %H:%M:%S", ltm);
    return time;
}

}