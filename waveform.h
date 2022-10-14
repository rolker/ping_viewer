#ifndef WAVEFORM_H
#define WAVWFORM_H

#include <QLineSeries>

class Waveform: public QtCharts::QLineSeries
{
  Q_OBJECT

public:
  Waveform(QObject *parent = nullptr);
  const double& sampleRate() const;
  void setSampleRate(double sample_rate);

  Waveform* matchFilter(const Waveform* ping) const;

  Waveform* fourierTransform() const;

  
  Waveform* extractPing(double pulse_length = 0.01) const;
  Waveform* trimPing(int chunkSize, double powerThreshold) const;

  Waveform* averagePower(int decimationFactor=1) const;
  Waveform* maximumPower(int decimationFactor=1) const;

  Waveform* dB() const;
  Waveform* ranges(double sound_speed) const;

  Waveform* xShift(double shift) const;

  Waveform* tvg20(double blanking_distance=1.0) const;

  double totalPower() const;

public slots:
  void open(QString filename);

private:
  double sample_rate_ = 125000.0;
};

#endif
