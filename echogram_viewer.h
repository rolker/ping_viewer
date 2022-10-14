#ifndef ECHOGRAM_VIEWER_H
#define ECHOGRAM_VIEWER_H

#include <QMainWindow>
#include <ui_echogram_viewer.h>

class EchogramViewer: public QMainWindow
{
  Q_OBJECT

public:
  explicit EchogramViewer(QWidget *parent = nullptr);
  ~EchogramViewer();

private slots:
  void open();
  void selectColormap(const QString& colormap);

private:
  Ui::EchogramViewer ui_;
  QString currentDirectory_;
};

#endif
