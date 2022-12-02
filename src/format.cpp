#include "format.h"

#include <string>

using std::string;
using std::to_string;

// TODO: Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
// REMOVE: [[maybe_unused]] once you define the function
string Format::ElapsedTime(long seconds) {
  int hr, min, sec;
  sec = seconds % 60;
  min = (seconds / 60) % 60;
  hr = seconds / (60 * 60);

  string hms;
  hms = ((hr < 10) ? "0" + to_string(hr) : to_string(hr)) + ":";
  hms += ((min < 10) ? "0" + to_string(min) : to_string(min)) + ":";
  hms += (sec < 10) ? "0" + to_string(sec) : to_string(sec);
  return hms;
}