#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <string>
#include <vector>

using std::stof;
using std::string;
using std::to_string;
using std::vector;

// HELPER FUNCTION
void logfile(string fname, string msg) {
  std::fstream stream(fname, std::ios::out);
  if (stream.is_open()) {
    stream << msg;
  }
  stream.close();
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  filestream.close();
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, version, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  stream.close();
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// TODO: Read and return the system memory utilization
// Previously calculated by : memUsed = memTotal - memFree - buffers - cached -
// slab;
float LinuxParser::MemoryUtilization() {
  string line, title, usage;
  vector<string> memVec;
  float memTotal, memFree;
  int idx = 0;
  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      linestream >> title >> usage;
      if (title == "MemTotal:") {
        memTotal = std::stof(usage);
        ++idx;
      } else if (title == "MemFree:") {
        memFree = std::stof(usage);
        ++idx;
      }
      if (idx == 2) break;
    }
    float memUsage = memTotal - memFree;
    return memUsage / memTotal;
  }
  stream.close();
  return 0;
}

// TODO: Read and return the system uptime
long LinuxParser::UpTime() {
  long uptime;
  string line;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    while (getline(stream, line)) {
      std::istringstream ss(line);
      ss >> uptime;
    }
  }
  stream.close();
  return uptime;
}

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() {
  long totaljiffies = 0;
  vector<string> jiffies = LinuxParser::CpuUtilization();

  for (size_t i = 0; i < jiffies.size(); ++i) {
    if (i == CPUStates::kGuest_ || i == CPUStates::kGuestNice_) {
      // usertime = usertime - guest
      // nicetime = nicetime - guestnice
      totaljiffies -= stol(jiffies[i]);
    } else {
      // sum all jiffies (from all CPUStates)
      totaljiffies += stol(jiffies[i]);
    }
  }
  return totaljiffies;
}

// TODO: Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) {
  vector<string> procstat;
  string line;
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    while (getline(stream, line)) {
      std::istringstream ss(line);
      string token;
      while (ss >> token) {
        procstat.emplace_back(token);
      }
    }
  }
  /*
    // debug - SIGSEGV
    string result;
    for (string s: procstat)
      result += s + "~";
    logfile("/tmp/activejiffies", result);
  */
  // totalTime (spent by the process)
  // startTime (the time the process started after system boot)
  // systemUptime (the time duration since system boot)
  long totalTime = stol(procstat[ProcessStates::kUTime_]) +
                   stol(procstat[ProcessStates::kSTime_]) +
                   stol(procstat[ProcessStates::kCuTime_]) +
                   stol(procstat[ProcessStates::kCsTime_]);
  totalTime /= sysconf(_SC_CLK_TCK);  // time in seconds
  stream.close();
  return totalTime;
}

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  vector<string> cpuinfo{LinuxParser::CpuUtilization()};
  long totalActiveJiffies = 0;
  for (size_t i = 0; i < cpuinfo.size(); ++i) {
    if (i != CPUStates::kIdle_ && i != CPUStates::kIOwait_)
      totalActiveJiffies += stol(cpuinfo[i]);
  }
  return totalActiveJiffies;
}

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  vector<string> cpuinfo{LinuxParser::CpuUtilization()};
  long totalIdleJiffies = 0;
  for (size_t i = 0; i < cpuinfo.size(); ++i) {
    if (i == CPUStates::kIdle_ || i == CPUStates::kIOwait_)
      totalIdleJiffies += stol(cpuinfo[i]);
  }
  return totalIdleJiffies;
}

// TODO: Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() {
  vector<string> cpuinfo;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    string line;
    while (getline(stream, line)) {
      string key, jiffy;
      std::istringstream ss(line);
      ss >> key;
      if (key == "cpu") {
        while (ss >> jiffy) {
          cpuinfo.emplace_back(jiffy);
        }
      }
    }
  }
  stream.close();
  return cpuinfo;
}

// TODO: Read and return the total number of processes
int LinuxParser::TotalProcesses() {
  int totalProcesses;
  string line;
  string key;
  int value;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (getline(stream, line)) {
      std::stringstream ss(line);
      ss >> key >> value;
      if (key == "processes") {
        totalProcesses = value;
        break;
      }
    }
  }
  stream.close();
  return totalProcesses;
}

// TODO: Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  int procsRunning = 0;
  string line;
  std::ifstream stream(kProcDirectory + kStatFilename);
  if (stream.is_open()) {
    while (getline(stream, line)) {
      std::istringstream ss(line);
      string key;
      int value;
      ss >> key >> value;
      if (key == "procs_running") {
        procsRunning = value;
        break;
      }
    }
  }
  stream.close();
  return procsRunning;
}

// TODO: Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  string cmdline;
  std::ifstream stream(kProcDirectory + to_string(pid) + kCmdlineFilename);
  if (stream.is_open()) {
    getline(stream, cmdline);
  }
  stream.close();
  return cmdline;
}

// TODO: Read and return the memory used by a process
string LinuxParser::Ram(int pid) {
  string line;
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (stream.is_open()) {
    while (getline(stream, line)) {
      string key;
      long memsize;
      std::istringstream ss(line);
      ss >> key >> memsize;
      if (key == "VmSize:") {
        // size already in KB, return size in MB
        return to_string(memsize / 1024);
      }
    }
  }
  stream.close();
  return "0";
}

// TODO: Read and return the user ID associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Uid(int pid) {
  string line;
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatusFilename);
  if (stream.is_open()) {
    while (getline(stream, line)) {
      string key;
      string uid;
      std::istringstream ss(line);
      ss >> key >> uid;
      if (key == "Uid:") return uid;
    }
  }
  stream.close();
  return "UNK";
}

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid) {
  string line;
  std::ifstream stream(kPasswordPath);
  if (stream.is_open()) {
    while (getline(stream, line)) {
      string username, x, uid;
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream ss(line);
      while (ss >> username >> x >> uid) {
        if (uid == LinuxParser::Uid(pid)) {
          return username;
        }
      }
    }
  }
  stream.close();
  return "UNK";
}

// TODO: Read and return the uptime of a process
#define STARTTIME 21
long LinuxParser::UpTime(int pid) {
  string line;
  vector<string> procstat;
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    getline(stream, line);
    std::istringstream ss(line);
    string token;
    while (ss >> token) {
      procstat.emplace_back(token);
    }
  }
  long procUptime = LinuxParser::UpTime() -
                    (stol(procstat[STARTTIME]) / sysconf(_SC_CLK_TCK));
  stream.close();
  return procUptime;
}