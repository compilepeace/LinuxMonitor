#include "process.h"

#include <unistd.h>

#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

// TODO: Return this process's ID
int Process::Pid() const { return pid_; }

// TODO: Return this process's CPU utilization
// https://stackoverflow.com/questions/16726779/how-do-i-get-the-total-cpu-usage-of-an-application-from-proc-pid-stat/16736599#16736599
float Process::CpuUtilization() const {
  long procActiveTime = LinuxParser::ActiveJiffies(Pid());
  long procUptime = LinuxParser::UpTime(Pid());
  return (float)procActiveTime / (float)procUptime;
}

// TODO: Return the command that generated this process
string Process::Command() {
  if (cmdline_.empty()) {
    cmdline_ = LinuxParser::Command(Pid());
  }
  return cmdline_;
}

// TODO: Return this process's memory utilization
string Process::Ram() { return LinuxParser::Ram(Pid()); }

// TODO: Return the user (name) that generated this process
string Process::User() {
  if (username_.empty()) {
    username_ = LinuxParser::User(Pid());
  }
  return username_;
}

// TODO: Return the age of this process (in seconds)
long int Process::UpTime() { return LinuxParser::UpTime(Pid()); }

// TODO: Overload the "less than" comparison operator for Process objects
bool Process::operator<(Process const& a) const {
  return (Process::CpuUtilization() < a.CpuUtilization());
}