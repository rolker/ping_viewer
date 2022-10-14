#include "echogram.h"
#include <QWheelEvent>
#include <QMenu>
#include <QDir>
#include "raw_ping.h"
#include "ping.h"
#include <QGraphicsPixmapItem>
#include <QDialog>
#include "ping_view.h"
#include <QHBoxLayout>

#include <QDebug>

Echogram::Echogram(QWidget *parent):
  QGraphicsView(parent)
{
  setScene(new QGraphicsScene(this));
  setDragMode(QGraphicsView::DragMode::ScrollHandDrag);
  setMouseTracking(true);
}

void Echogram::wheelEvent(QWheelEvent *event)
{
  if(event->angleDelta().y()<0)
    scale(.8,.8);
  if(event->angleDelta().y()>0)
    scale(1.25,1.25);
  event->accept();
}

void Echogram::mousePressEvent(QMouseEvent *event)
{
  switch(event->button())
  {
  case Qt::LeftButton:
    break;
  case Qt::RightButton:
    event->accept();
    break;
  case Qt::MiddleButton:
    break;
  default:
    break;
  }
  QGraphicsView::mousePressEvent(event);
}

void Echogram::mouseMoveEvent(QMouseEvent *event)
{
  QPointF transformedMouse = mapToScene(event->pos());
  QString posText;
  int ping_number = transformedMouse.rx()/horizontal_scale_;
  if (ping_number >= 0 && ping_number < pings_.size())
  {
    auto ping = pings_.begin();
    int i = 0;
    while(i < ping_number)
    {
      i++;
      ping++;
    }
    auto time = transformedMouse.ry()/ping->second->sampleRate();
    //posText += " time: " + QString::number(time);
    posText += " range: " + QString::number(time*1500.0/2.0);
    posText += " file: " + QString(ping->second->filename().c_str());
  }
  emit cursorInfo(posText);
  QGraphicsView::mouseMoveEvent(event);
}

void Echogram::mouseReleaseEvent(QMouseEvent *event)
{
  QGraphicsView::mouseReleaseEvent(event);
}

void Echogram::contextMenuEvent(QContextMenuEvent* event)
{
  QPointF transformedMouse = mapToScene(event->pos());
  int ping_number = transformedMouse.rx()/horizontal_scale_;
  if (ping_number >= 0 && ping_number < pings_.size())
  {
    auto ping = pings_.begin();
    int i = 0;
    while(i < ping_number)
    {
      i++;
      ping++;
    }

    QMenu menu(this);
    QAction *pingDetailsAction = menu.addAction("Ping Details");
    connect(pingDetailsAction, &QAction::triggered, [=]{ this->showDetails(ping->second->filename());});
    menu.exec(event->globalPos());
  }
  event->accept();
}

void Echogram::setDirectory(std::string dir)
{
  if(ping_loader_ && ping_loader_->isRunning())
  {
    ping_loader_->requestInterruption();
    while (ping_loader_->isRunning())
    {
      QThread::yieldCurrentThread();
    }
  }
  QDir qdir(dir.c_str(), "*.dat");
  for(auto file: qdir.entryList())
    pending_files_.push_back(dir+"/"+file.toStdString());
  qDebug() << pending_files_.size() << " pending files";
  ping_loader_ = new PingLoader();
  ping_loader_->setEchogram(this);
  ping_loader_->start();
}

void Echogram::newPings()
{
  {
    QMutexLocker lock(&new_pings_mutex_);
    while(!new_pings_.empty())
    {
      pings_[new_pings_.front().first] = new_pings_.front().second;
      new_pings_.pop_front();
    }
  }
  updateImage();
}

void Echogram::updateImage()
{
  std::size_t rows = 0;
  for(auto p: pings_)
    rows = std::max(rows, p.second->samples().size());
  
  std::size_t columns = pings_.size();
  std::size_t row_size = 4*columns;
  QImage image(columns, rows, QImage::Format_RGBA8888);
  std::size_t i = 0;

  double db_spread = max_db_ - min_db_;

  for(auto p: pings_)
  {
    for(std::size_t j = 0; j < rows && j < p.second->samples().size(); j++)
    {
      double db = p.second->samples()[j];
      double val = 0.0;
      if (db_spread != 0.0)
        val = std::min(1.0, std::max(0.0, (db-min_db_)/db_spread));
      std::size_t index = j*row_size+i*4;
      if(colormap_.isNull())
      {
        image.bits()[index] = val*255;
        image.bits()[index+1] = val*255;
        image.bits()[index+2] = val*255;
        image.bits()[index+3] = 255;
      }
      else
      {
        std::size_t cmindex = val*(colormap_.width()-1);
        image.setPixel(i,j,colormap_.pixel(cmindex,0));
      }
    }
    i++;
  }

  if(!pixmap_)
  {
    pixmap_ = scene()->addPixmap(QPixmap::fromImage(image));
    pixmap_->setTransform(QTransform().scale(horizontal_scale_, 1));
  }
  else
    pixmap_->setPixmap(QPixmap::fromImage(image));
}

void Echogram::setHorizontalScale(int scale)
{
  horizontal_scale_ = scale;
  if(pixmap_)
  {
    pixmap_->setTransform(QTransform().scale(scale, 1));
  }
}

void Echogram::setMinDB(double min)
{
  min_db_ = min;
  updateImage();
}

void Echogram::setMaxDB(double max)
{
  max_db_ = max;
  updateImage();
}

void Echogram::showDetails(std::string filename)
{
  qDebug() << "details: " << filename.c_str();
  if(!details_dialog_)
  {
    details_dialog_ = new QDialog(this);
    ping_details_ = new PingView(details_dialog_);
    QHBoxLayout* layout = new QHBoxLayout(details_dialog_);
    layout->addWidget(ping_details_);
    details_dialog_->setLayout(layout);
  }
  details_ping_ = std::make_shared<RawPing>(filename);
  ping_details_->setPing(*details_ping_);
  details_dialog_->show();
}

void Echogram::setColormap(QImage colormap)
{
  colormap_ = colormap;
  updateImage();
}

void PingLoader::setEchogram(Echogram* e)
{
  echogram_ = e;
}

void PingLoader::run()
{
  while(!isInterruptionRequested())
  {
    if(echogram_->pending_files_.empty())
      break;
    auto filename = echogram_->pending_files_.front();
    echogram_->pending_files_.pop_front();
    RawPing rp(filename);
    auto ping = std::make_shared<Ping>(rp);
    {
      QMutexLocker lock(&echogram_->new_pings_mutex_);
      echogram_->new_pings_.push_back(std::make_pair(filename, ping));
    }
    QMetaObject::invokeMethod(echogram_, "newPings", Qt::QueuedConnection);
  }
}
