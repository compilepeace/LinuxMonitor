#include "ncurses_display.h"
#include "system.h"

#include <string>
#include <fstream>
#include <vector>
using std::vector;
using std::string;
using std::to_string;

int main() {
  System system;

  string ps;
  vector<Process> pv = system.Processes();
  std::fstream stream("/tmp/debugprocs", std::ios::out);
  if (stream.is_open()){
    for (auto p: pv){
      ps = to_string(p.Pid()) + " ~ " + p.Command() + " ~ " +  p.User() + "\n";
      ps += "Ram: " + p.Ram() + ", CpuUtilization: " + to_string(p.CpuUtilization()) 
      + ", Uptime: " + to_string(p.UpTime());
      ps += "\n\n";
      stream << ps;
    }
  }
  stream.close();

  NCursesDisplay::Display(system);
  //NCursesDisplay::Display(system, system.Processes().size());
}