#ifndef WAVE_VIEW_H
#define WAVE_VIEW_H

#include <QChartView>

class Waveform;

class WaveView: public QtCharts::QChartView
{
  Q_OBJECT

public:
  WaveView(QWidget *parent = nullptr);
  ~WaveView();

  void setWaveform(Waveform* waveform);
  void addWaveform(Waveform* waveform);

signals:
  void dataAreaChanged(const QRectF& dataArea);

protected:
  void resizeEvent(QResizeEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  //void keyPressEvent(QKeyEvent *event) override;
  void wheelEvent(QWheelEvent* event) override;

public slots:
  void resetZoom();
  void updatePing();
  void zoomX(const QRectF& dataArea);
  void updateTicks(int width);
  void clear();

private:
  bool panning_ = false;
  int last_mouse_x;

};

#endif
