#ifndef ECHOGRAM_H
#define ECHOGRAM_H

#include <QGraphicsView>
#include <memory>
#include <QThread>
#include <QMutex>

class Ping;
class RawPing;
class PingLoader;
class PingView;

class Echogram: public QGraphicsView
{
  Q_OBJECT
public:
  explicit Echogram(QWidget *parent = 0);

public slots:
  void setDirectory(std::string dir);
  void newPings();
  void updateImage();
  void setHorizontalScale(int scale);
  void setMinDB(double min);
  void setMaxDB(double max);
  void setColormap(QImage colormap);

signals:
  void cursorInfo(const QString& message);

protected:
  void wheelEvent(QWheelEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void contextMenuEvent(QContextMenuEvent *event) override;

private slots:
  void showDetails(std::string filename);

private:
  std::map<std::string, std::shared_ptr<Ping> > pings_;

  QGraphicsPixmapItem* pixmap_ = nullptr;

  double min_db_ = -100;
  double max_db_ = -0;

  int horizontal_scale_ = 1;

  PingLoader* ping_loader_=nullptr;
  std::list<std::string> pending_files_;

  std::list<std::pair<std::string, std::shared_ptr<Ping> > > new_pings_;
  QMutex new_pings_mutex_;

  friend class PingLoader;

  QDialog* details_dialog_ = nullptr;
  PingView* ping_details_ = nullptr;
  std::shared_ptr<RawPing> details_ping_;

  QImage colormap_;
};

class PingLoader: public QThread
{
  Q_OBJECT
  void run() override;
  public:
    void setEchogram(Echogram* e);
  private:
    Echogram* echogram_;
};

#endif
