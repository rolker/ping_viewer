#ifndef PING_VIEW_H
#define PING_VIEW_H

#include <QWidget>
#include <ui_ping_view.h>

class RawPing;

class PingView: public QWidget
{
  Q_OBJECT

public:
  explicit PingView(QWidget *parent = nullptr);


public slots:
  void clear();
  void setRaw(Waveform* w);
  void setPing(RawPing& ping);

private slots:
  void updateDataAreas(const QRectF& dataArea);

private:
  Ui::PingView ui_;

};

#endif
