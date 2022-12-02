#include "processor.h"

#include "linux_parser.h"

// TODO: Return the aggregate CPU utilization
// 1. EXTRACT the system CPU utilization data from /procfs
// 2. figure out how to USE extracted data to calculate CPU utilization
float Processor::Utilization() {
  long totalJiffies = LinuxParser::Jiffies();
  long activeJiffies = LinuxParser::ActiveJiffies();
  // CPU utilization is calculated as the time duration in which
  // the CPU remains active. Duration is represented as a jiffy.
  float aggregateUtilization = (float)activeJiffies / (float)totalJiffies;
  return aggregateUtilization;
}