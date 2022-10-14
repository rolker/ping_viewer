#ifndef PING_BASE_H
#define PING_BASE_H

#include <string>
#include <chrono>

class PingBase
{
public:
  const std::string& filename() const;

  using Timestamp = std::chrono::time_point<std::chrono::system_clock>;
  Timestamp timestamp() const;

protected:
  PingBase(std::string filename);
  
private:
  std::string filename_;
  Timestamp timestamp_;
};

#endif
