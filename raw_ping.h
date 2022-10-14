#ifndef RAW_PING_H
#define RAW_PING_H

#include "ping_base.h"
#include <vector>

class RawPing: public PingBase
{
public:
  RawPing(std::string filename);

  const std::vector<double>& waveform() const;
  std::vector<double> ping() const;
  const std::vector<double>& power() const;
  const std::vector<double>& matched() const;
  const std::vector<double>& matchedPower() const;
  const std::vector<double>& tvg20() const;

  std::size_t pingStart() const;
  std::size_t pingPowerStart() const;
  double sampleRate() const;
  double powerSampleRate() const;
  double startTime() const;

private:
  std::size_t pingPowerEnd() const;
  double indexToTime(std::size_t i) const;
  double indexToRange(std::size_t i) const;
  void findPing();
  void calculatePower();
  void calculateMatched();
  double pingTotalPower(bool exact=false) const;

private:
  double sample_rate_ = 125000.0;
  double ping_length_ = 0.01;
  double sound_speed_ = 1500.0;

  std::size_t power_sample_count_ = 15;
  
  double ping_power_threshold_ = 0.8;

  std::vector<double> raw_;
  std::size_t ping_start_ = 0;
  std::size_t ping_end_ = 0;


  std::vector<double> power_;

  std::vector<double> matched_;
  std::vector<double> matched_power_;

  std::vector<double> tvg20_;
  double blanking_distance_ = 5.0;

};

#endif
