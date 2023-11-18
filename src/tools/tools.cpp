#include <termios.h>
#include <sys/ioctl.h>
#include <iostream>
#include <iomanip>
#include "tools.hpp"

namespace tools
{

// check if a key (on keyboard) is pressed
bool kbhit()
{
    termios term;
    tcgetattr(0, &term);

    termios term2 = term;
    term2.c_lflag &= ~ICANON;
    tcsetattr(0, TCSANOW, &term2);

    int byteswaiting;
    ioctl(0, FIONREAD, &byteswaiting);

    tcsetattr(0, TCSANOW, &term);

    return byteswaiting > 0;
}

std::string to_string_exact(double x) 
{
  std::ostringstream os;
  os << std::fixed << std::setprecision(std::numeric_limits<double>::max_digits10) << x;
  return os.str();
}

}