#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <string>
#include <vector>

using std::stof;
using std::string;
using std::to_string;
using std::vector;

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
float LinuxParser::MemoryUtilization() {
  string line;
  int memTotal, memFree, buffers, cached;  // slab;

  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  while (getline(stream, line)) {
    string key;
    int value;
    std::istringstream ss(line);
    ss >> key >> value;
    if (key == "MemTotal:")
      memTotal = value;
    else if (key == "MemFree:")
      memFree = value;
    else if (key == "Buffers:")
      buffers = value;
    else if (key == "Cached:")
      cached = value;
    // else if (key == "Slab:") slab = value;
  }
  double memUsed;
  // memUsed = memTotal - memFree - buffers - cached - slab;
  memUsed = (memTotal - memFree - buffers - cached);
  return (memUsed / memTotal);
}

// TODO: Read and return the system uptime
long LinuxParser::UpTime() {
  long uptime;
  string line;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  while (getline(stream, line)) {
    std::istringstream ss(line);
    ss >> uptime;
  }
  return uptime;
}

// TODO: Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { return 0; }

// TODO: Read and return the number of active jiffies for a PID
// REMOVE: [[maybe_unused]] once you define the function
long LinuxParser::ActiveJiffies(int pid [[maybe_unused]]) { return 0; }

// TODO: Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() { return 0; }

// TODO: Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() { return 0; }

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
        while (getline(ss, jiffy, ' ')) {
          cpuinfo.emplace_back(jiffy);
        }
      }
    }
  }
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
      }
    }
  }
  return totalProcesses;
}

// TODO: Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  int procsRunning = 0;
  string line;
  std::ifstream stream(kProcDirectory + kStatFilename);
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
  return procsRunning;
}

// TODO: Read and return the command associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Command(int pid) {
  string cmdline;
  std::ifstream stream(kProcDirectory + to_string(pid) + kCmdlineFilename);
  if (stream.is_open()) {
    getline(stream, cmdline);
  }
  return cmdline;
}

// TODO: Read and return the memory used by a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::Ram(int pid) {
  string line;
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatusFilename);
  while (getline(stream, line)) {
    string key;
    size_t memsize;
    std::istringstream ss(line);
    ss >> key >> memsize;
    if (key == "VmSize:") {
      // size already in KB, return size in MB
      return to_string(memsize / 1024);
    }
  }
  return string();
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
  return string();
}

// TODO: Read and return the user associated with a process
// REMOVE: [[maybe_unused]] once you define the function
string LinuxParser::User(int pid) {
  string line;
  string username;
  string uid{LinuxParser::Uid(pid)};
  std::ifstream stream(kPasswordPath);

  if (stream.is_open()) {
    while (getline(stream, line)) {
      string token;
      vector<string> userinfo;
      std::istringstream ss(line);

      // each line in /etc/password file is tokenized by ':' delimiter
      while (getline(ss, token, ':')) {
        userinfo.emplace_back(token);
      }
      if (userinfo[2] == to_string(pid)) {
        return userinfo[0];
      }
    }
  }
  return string();
}

// TODO: Read and return the uptime of a process
// REMOVE: [[maybe_unused]] once you define the function
#define STARTTIME 21
long LinuxParser::UpTime(int pid) {
  string line;
  vector<string> procstat;
  std::ifstream stream(kProcDirectory + to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    getline(stream, line);
    std::istringstream ss(line);
    string token;
    while (getline(ss, token, ' ')) {
      procstat.emplace_back(token);
    }
  }
  long procUptime = stol(procstat[STARTTIME]) / sysconf(_SC_CLK_TCK);
  return procUptime;
}
