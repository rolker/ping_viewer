#include "raw_ping.h"
#include <fstream>
#include <cmath>
#include <algorithm>

RawPing::RawPing(std::string filename):PingBase(filename)
{
  std::ifstream in(filename, std::ios::binary | std::ios::ate);

  auto size = in.tellg();
  in.seekg(0, std::ios::beg);

  std::vector<char> data(size);
  in.read(data.data(), size);

  for(std::size_t i = 0; i+1 < size; i+=2)
  {
    char temp = data[i];
    data[i] = data[i+1];
    data[i+1] = temp;
  }

  auto sample_count = size/2;

  raw_.reserve(sample_count);
  for(std::size_t i = 0; i < sample_count; i++)
    raw_.push_back(reinterpret_cast<int16_t*>(data.data())[i]/float(std::numeric_limits<int16_t>::max()));

  calculatePower();
  findPing();
  calculateMatched();
}

void RawPing::calculatePower()
{
  for (int i = 0; i*power_sample_count_ < raw_.size(); i++)
  {
    double p = 0.0;
    for(int j = 0; j < power_sample_count_ && i* power_sample_count_+j < raw_.size(); j++)
      p += pow(raw_[i*power_sample_count_+j], 2.0);
    power_.push_back(p);
  }
}

void RawPing::findPing()
{
  std::size_t sample_count = ping_length_*sample_rate_;
  std::size_t index = 0;
  double max_power = 0.0;

  for(std::size_t i = 0; i < raw_.size(); i+=power_sample_count_)
  {
    ping_start_ = i;
    ping_end_ = std::min(ping_start_+sample_count, raw_.size());
    double p = pingTotalPower();
    if(p > max_power)
    {
      max_power = p;
      index = i;
    }
  }

  ping_start_ = std::min(index, raw_.size());
  ping_end_ = std::min(ping_start_+sample_count, raw_.size());

  //now trim the ping

  if(ping_end_ > ping_start_)
  {
    max_power = *std::max_element(&power_[pingPowerStart()], &power_[pingPowerEnd()]);

    auto start = pingPowerStart();
    while (start < pingPowerEnd() && power_[start]<max_power*ping_power_threshold_)
    {
      start++;
    }

    auto end = pingPowerEnd()-1;
    while (end > start && power_[end] < max_power*ping_power_threshold_)
    {
      end--;
    }

    ping_start_ = std::min(start*power_sample_count_, raw_.size());
    ping_end_ = std::min(end*power_sample_count_, raw_.size());
  }
}

void RawPing::calculateMatched()
{
  if(ping_end_ > ping_start_)
  {
    double ping_power = pingTotalPower(true);
    auto ping_sample_count = ping_end_-ping_start_;
    auto half_ping_sample_count = (ping_end_-ping_start_)/2;
    for(std::size_t i = 0; i < raw_.size(); i++)
    {
      double sum = 0.0;
      for(std::size_t j = 0; j < ping_sample_count; j++)
        if(i+j-half_ping_sample_count >= 0 && i+j-half_ping_sample_count < raw_.size())
          sum += raw_[i+j-half_ping_sample_count]*raw_[ping_start_+j];
      matched_.push_back(sum/ping_power);
    }

    if(power_sample_count_ > 0)
    {
      for (int i = 0; i*power_sample_count_ < matched_.size(); i++)
      {
        double p = 0.0;
        for(int j = 0; j < power_sample_count_ && i* power_sample_count_+j < raw_.size(); j++)
          p = std::max(p, pow(matched_[i*power_sample_count_+j], 2.0));
        matched_power_.push_back(p);
      }

      for(int i = 0; i < matched_power_.size(); i++)
      {
        double db = 10.0*log10(matched_power_[i]);
        double range = indexToRange(i*power_sample_count_);
        double tvg = 0.0;
        if (range > blanking_distance_)
          tvg = 20*log10(range);
        tvg20_.push_back(db+tvg);
      }
    }
  }
}

std::size_t RawPing::pingStart() const
{
  return std::max(std::size_t(0), std::min(ping_start_, raw_.size()));
}

std::size_t RawPing::pingPowerStart() const
{
  return std::max(std::size_t(0), std::min(ping_start_/power_sample_count_, power_.size()));
}

std::size_t RawPing::pingPowerEnd() const
{
  return 1+(ping_end_-1)/power_sample_count_;
}

double RawPing::pingTotalPower(bool exact) const
{
  double ret = 0.0;
  if(exact)
    for(auto i = ping_start_; i < ping_end_; i++)
      ret += pow(raw_[i],2);
  else
    for(auto i = pingPowerStart(); i < pingPowerEnd(); i++)
      ret += power_[i];
  return ret;
}

double RawPing::startTime() const
{
  if(ping_start_ < raw_.size())
  {
    double ret = ping_start_/sample_rate_;
    return -ret;
  }
  return 0.0;
}

double RawPing::indexToTime(std::size_t i) const
{
  return startTime()+i/sample_rate_;
}

double RawPing::indexToRange(std::size_t i) const
{
  return indexToTime(i)*sound_speed_/2.0; // two way travel time
}

double RawPing::sampleRate() const
{
  return sample_rate_;
}

double RawPing::powerSampleRate() const
{
  return sample_rate_/double(power_sample_count_);
}

const std::vector<double>& RawPing::waveform() const
{
  return raw_;
}

std::vector<double> RawPing::ping() const
{
  std::vector<double> ret;
  for(auto i = ping_start_; i < ping_end_; i++)
    ret.push_back(raw_[i]);
  return ret;
}

const std::vector<double>& RawPing::power() const
{
  return power_;
}

const std::vector<double>& RawPing::matched() const
{
  return matched_;
}

const std::vector<double>& RawPing::matchedPower() const
{
  return matched_power_;
}

const std::vector<double>& RawPing::tvg20() const
{
  return tvg20_;
}

