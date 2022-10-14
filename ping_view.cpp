#include "ping_view.h"
#include "waveform.h"
#include "raw_ping.h"
#include <QDebug>

PingView::PingView(QWidget *parent):
  QWidget(parent)
{
  ui_.setupUi(this);
  connect(ui_.resetZoomPushButton, &QPushButton::clicked, ui_.shiftedRawWaveView, &WaveView::resetZoom);

  connect(ui_.shiftedRawWaveView, &WaveView::dataAreaChanged, this, &PingView::updateDataAreas);
}

void PingView::clear()
{
  ui_.shiftedRawWaveView->clear();
  //ui_.rawPowerWaveView->clear();
  ui_.pingWaveView->clear();
  //ui_.matchedWaveView->clear();
  ui_.matchedPowerWaveView->clear();
  ui_.powerDBWaveView->clear();
}

void PingView::setPing(RawPing& ping)
{
  clear();

  ui_.sampleRateLineEdit->setText(QString::number(ping.sampleRate()));

  Waveform* w = new Waveform;
  w->setSampleRate(ping.sampleRate());
  w->setName("raw");
  double start_time = ping.startTime();
  for(int i = 0; i < ping.waveform().size(); i++)
    w->append(start_time+i/ping.sampleRate(), ping.waveform()[i]);

  ui_.shiftedRawWaveView->setWaveform(w);

  w = new Waveform;
  w->setSampleRate(ping.sampleRate());
  w->setName("ping");
  auto ping_data = ping.ping();
  for(int i = 0; i < ping_data.size(); i++)
    w->append(i/ping.sampleRate(), ping_data[i]);

  ui_.pingWaveView->setWaveform(w);

  w = new Waveform;
  w->setSampleRate(ping.powerSampleRate());
  w->setName("matched power");
  for(int i = 0; i < ping.matchedPower().size(); i++)
    w->append(start_time+i/ping.powerSampleRate(), ping.matchedPower()[i]);

  ui_.matchedPowerWaveView->setWaveform(w);

  w = new Waveform;
  w->setSampleRate(ping.powerSampleRate());
  w->setName("tvg20");
  for(int i = 0; i < ping.tvg20().size(); i++)
    w->append(start_time+i/ping.powerSampleRate(), ping.tvg20()[i]);

  ui_.powerDBWaveView->setWaveform(w);
}

void PingView::setRaw(Waveform * w)
{
  clear();
  ui_.sampleRateLineEdit->setText(QString::number(w->sampleRate()));
  QApplication::processEvents();

  auto rough_ping = w->extractPing(0.01);
  auto ping = rough_ping->trimPing(10, 0.8);
  //auto ping = rough_ping;

  ui_.pingWaveView->setWaveform(ping);
  QApplication::processEvents();

  int decimationFactor = ping->count()/2;
  qDebug() << "decimation factor: " << decimationFactor;

  auto shifted = w->xShift(-ping->at(0).x())->ranges(1500);
  ui_.shiftedRawWaveView->setWaveform(shifted);
  QApplication::processEvents();

  // ui_.rawPowerWaveView->setWaveform(shifted->averagePower(10));
  // QApplication::processEvents();

  auto matched = shifted->matchFilter(ping);
  // ui_.matchedWaveView->setWaveform(matched);
  // QApplication::processEvents();

  auto maxPower = matched->maximumPower(decimationFactor);
  ui_.matchedPowerWaveView->setWaveform(maxPower);
  QApplication::processEvents();

  auto powerDB = maxPower->dB();
  ui_.powerDBWaveView->setWaveform(powerDB->tvg20(5.0));
}

void PingView::updateDataAreas(const QRectF& dataArea)
{
//  ui_.rawPowerWaveView->zoomX(dataArea);
  ui_.powerDBWaveView->zoomX(dataArea);
//  ui_.matchedWaveView->zoomX(dataArea);
  ui_.matchedPowerWaveView->zoomX(dataArea);
}
