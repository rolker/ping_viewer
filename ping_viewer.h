#ifndef PING_VIEWER_H
#define PING_VIEWER_H

#include <QMainWindow>
#include <ui_ping_viewer.h>
#include <QLineSeries>
#include <memory>

class PingViewer: public QMainWindow
{
  Q_OBJECT

public:
  explicit PingViewer(QWidget *parent = nullptr);
  ~PingViewer();

private slots:
  void open();
  void fileSelected(QListWidgetItem* current, QListWidgetItem* previous);
  void updatePixmap();

private:
  Ui::PingViewer ui_;
  QString currentDirectory_;
  QString currentFile_;
  std::map<QString, std::shared_ptr<RawPing> > pings_;

};

#endif
