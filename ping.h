#ifndef PING_H
#define PING_H

#include "ping_base.h"
#include <vector>

class RawPing;

class Ping: public PingBase
{
public:
  Ping(const RawPing &raw);
  const double& sampleRate() const;
  const std::vector<double>& samples() const;

private:
  double sample_rate_ = 125000.0;
  double sound_speed_ = 1500.0;

  std::vector<double> samples_;
};

#endif