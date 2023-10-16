#include "ping_base.h"
#include <sstream>
#include "date.h"

PingBase::PingBase(std::string filename):filename_(filename)
{
  auto time_start = filename.rfind('_');
  if(time_start != std::string::npos)
  {
    time_start += 1;
    auto time_string = filename.substr(time_start, filename.rfind('.')-time_start);
    std::stringstream time_ss(time_string);
    time_ss >> date::parse("%Y-%m-%dT%H.%M.%S", timestamp_);
  }
  
}

PingBase::Timestamp PingBase::timestamp() const
{
  return timestamp_;
}

const std::string& PingBase::filename() const
{
  return filename_;
}
