#include "ping.h"
#include "raw_ping.h"

Ping::Ping(const RawPing& raw):PingBase(raw)
{
  sample_rate_ = raw.powerSampleRate();
  samples_ = std::vector<double>(raw.tvg20().begin()+raw.pingPowerStart(), raw.tvg20().end());
}

const double& Ping::sampleRate() const
{
  return sample_rate_;
}

const std::vector<double>& Ping::samples() const
{
  return samples_;
}