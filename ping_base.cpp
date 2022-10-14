#include "ping_base.h"

PingBase::PingBase(std::string filename):filename_(filename)
{
  //todo: extract timestamp
}

PingBase::Timestamp PingBase::timestamp() const
{
  return timestamp_;
}

const std::string& PingBase::filename() const
{
  return filename_;
}
