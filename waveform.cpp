#include "waveform.h"
#include <QFile>
#include <QtEndian>
#include <QJsonDocument>
#include <QJsonArray>
#include <cmath>
#include <QDebug>

Waveform::Waveform(QObject *parent)
  :QLineSeries(parent)
{
  setUseOpenGL(true);
  setName("waveform");
}

void Waveform::open(QString filename)
{
  QFile file(filename);
  if (!file.open(QIODevice::ReadOnly))
    return;
 
  QFile meta(filename+".json");
  if(meta.open(QIODevice::ReadOnly|QFile::Text))
  {
    auto doc = QJsonDocument::fromJson(meta.readAll());
    if(doc["sample_rate"] != QJsonValue::Undefined)
      sample_rate_ = doc["sample_rate"].toDouble();
  }

  int16_t sample;
  while (file.read(reinterpret_cast<char *>(&sample), 2))
  {
    append(count()/sample_rate_, qFromBigEndian(sample)/float(std::numeric_limits<int16_t>::max()));
  }
}

const double& Waveform::sampleRate() const
{
  return sample_rate_;
}

void Waveform::setSampleRate(double sample_rate)
{
  sample_rate_ = sample_rate;
}

Waveform* Waveform::matchFilter(const Waveform* ping) const
{
  Waveform* ret = nullptr;

  if(ping && ping->count() > 0 && count() > 0)
  {
    ret = new Waveform;
    ret->setName(name()+" "+ping->name()+" matched");
    ret->sample_rate_ = sample_rate_;
    double power = ping->totalPower();
    auto samples = pointsVector();
    double zero_x = samples.front().x();
    double x_interval = (samples.back().x()-zero_x)/(count()-1.0);
    double half_ping_time = x_interval*ping->count()/2.0;
    for (int i = -ping->count(); i < count(); i++)
    {
      double y = 0.0;
      for(int j = 0; j < ping->count() && i+j < count(); j++)
        if(i+j >= 0)
          y += ping->at(j).y()*samples[i+j].y();
      ret->append(zero_x+half_ping_time+i*x_interval, y/power);
    }
  }

  return ret;
}

Waveform* Waveform::fourierTransform() const
{
  Waveform* ret = new Waveform;
  ret->setName("frequencies");
  ret->sample_rate_ = sample_rate_;

  if(count() > 0)
  {
    auto x = pointsVector();
    
    for(int k = 0; k < count()/2; k++)
    {
      double xk = 0.0;
      double xki = 0.0;
      double factor = k*2*M_PI/double(count());
      for(int n = 0; n < count(); n++)
      {
        xk += x[n].ry()*cos(factor*n);
        xki += x[n].ry()*sin(factor*n);
      }
      ret->append(k*sample_rate_/double(count()), xk*xk+xki*xki);
    }
  }

  return ret;
}

Waveform* Waveform::averagePower(int decimationFactor) const
{
  Waveform* ret = new Waveform;
  ret->setName(name()+" power");
  ret->sample_rate_ = sample_rate_/double(decimationFactor);
  double scale = 1.0/double(decimationFactor);
  auto samples = pointsVector();
  double zero_x = samples.front().x();
  double x_interval = (samples.back().x()-zero_x)/(count()-1.0);

  if(decimationFactor > 0)
  {
    for (int i = 0; i*decimationFactor < count(); i++)
    {
      double p = 0.0;
      for(int j = 0; j < decimationFactor && i*decimationFactor+j < count(); j++)
        p += samples[i*decimationFactor+j].y()*samples[i*decimationFactor+j].y()*scale;
      ret->append(zero_x+i*decimationFactor*x_interval, p);
    }
  }

  return ret;

}

Waveform* Waveform::maximumPower(int decimationFactor) const
{
  Waveform* ret = new Waveform;
  ret->setName(name()+" power");
  ret->sample_rate_ = sample_rate_/double(decimationFactor);

  if(decimationFactor > 0)
  {
    auto samples = pointsVector();
    double zero_x = samples.front().x();
    double x_interval = (samples.back().x()-zero_x)/(count()-1.0);
    for (int i = 0; i*decimationFactor < count(); i++)
    {
      double maxp = 0.0;
      for(int j = 0; j < decimationFactor && i*decimationFactor+j < count(); j++)
        maxp = std::max(maxp, samples[i*decimationFactor+j].y()*samples[i*decimationFactor+j].y());
      ret->append(zero_x+i*decimationFactor*x_interval, maxp);
    }
  }

  return ret;

}

Waveform* Waveform::dB() const
{
  Waveform* ret = new Waveform;
  ret->setName(name()+" dB");
  ret->sample_rate_ = sample_rate_;

  for(auto s: pointsVector())
    ret->append(s.rx(), 10*log10(s.ry()));

  return ret;
}

Waveform* Waveform::ranges(double sound_speed) const
{
  Waveform* ret = new Waveform;
  ret->setName(name());
  ret->sample_rate_ = sample_rate_;

  double factor = sound_speed/2.0; // two way travel time  

  for(auto s: pointsVector())
    ret->append(s.rx()*factor, s.ry());

  return ret;
}

Waveform* Waveform::xShift(double shift) const
{
  Waveform* ret = new Waveform;
  ret->setName(name());
  ret->sample_rate_ = sample_rate_;

  for(auto s: pointsVector())
    ret->append(s.rx()+shift, s.ry());

  return ret;
}

Waveform* Waveform::tvg20(double blanking_distance) const
{
  Waveform* ret = new Waveform;
  ret->setName(name());
  ret->sample_rate_ = sample_rate_;
  for(auto s: pointsVector())
  {
    auto y = s.ry();
    if(s.rx() >= blanking_distance)
      y += 20*log10(s.rx());
    ret->append(s.rx(), y);
  }
  return ret;
}

double Waveform::totalPower() const
{
  double ret = 0.0;
  for(auto p: pointsVector())
    ret += p.ry()*p.ry();
  return ret;
}

Waveform* Waveform::extractPing(double pulse_length) const
{
  Waveform* ret = nullptr;

  int sample_count = pulse_length*sample_rate_;

  int index = 0;
  int max_power = 0;
  for (int i = 0; i < count(); i++)
  {
    double p = 0.0;
    for(int j = 0; j < sample_count && i+j < count(); j++)
      p += at(i+j).y()*at(i+j).y();
    if (p > max_power)
    {
      index = i;
      max_power = p;
    }
  }

  qDebug() << "Max power: " << max_power << " at: " << index/sample_rate_;

  ret = new Waveform;
  ret->setName("ping");
  ret->sample_rate_ = sample_rate_;
  for (int i = index; i < index+sample_count; i++)
    if(i < count())
      ret->append(at(i));
    else
      ret->append(i/sample_rate_,0);
  return ret;
}

Waveform* Waveform::trimPing(int chunkSize, double powerThreshold) const
{
  Waveform* ret = new Waveform;
  ret->setName(name());
  ret->sample_rate_ = sample_rate_;

  std::vector<double> powers;
  double scale = 1.0/double(chunkSize);

  for(int i = 0; i < count(); i+= chunkSize)
  {
    double p = 0;
    for(int j = 0; j < chunkSize && i+j < count(); j++)
      p += pow(at(i+j).y(),2.0);
    powers.push_back(p*scale);
  }

  auto max_power = *std::max_element(powers.begin(),powers.end());

  int start = 0;
  while (start < powers.size() && powers[start]<max_power*powerThreshold)
    start++;

  int end = start;
  while (end < powers.size() && powers[end] >= max_power*powerThreshold)
    end++;

  for(int i = start*chunkSize; i < count() && i < end*chunkSize; i++)
    ret->append(at(i));

  return ret;
}
