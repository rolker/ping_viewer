#include "ping_viewer.h"
#include <QFileDialog>
#include <QMessageBox>
#include "waveform.h"
#include <QtEndian>
#include <QJsonDocument>
#include <QJsonArray>
#include <QDebug>
#include "raw_ping.h"

PingViewer::PingViewer(QWidget *parent):
  QMainWindow(parent)
{
  ui_.setupUi(this);

  this->setCentralWidget(ui_.pingViewer);

  connect(ui_.actionOpen, &QAction::triggered, this, &PingViewer::open);

  connect(ui_.fileListWidget, &QListWidget::currentItemChanged, this, &PingViewer::fileSelected);
}

PingViewer::~PingViewer()
{

}

void PingViewer::open()
{
  currentDirectory_ = QFileDialog::getExistingDirectory(this, "Select data directory");
  QDir dir(currentDirectory_, "*.dat");
  ui_.fileListWidget->clear();
  for(auto file: dir.entryList())
    ui_.fileListWidget->insertItem(ui_.fileListWidget->count(),file);
}

void PingViewer::fileSelected(QListWidgetItem* current, QListWidgetItem* previous)
{
  auto ping = pings_[qvariant_cast<QString>(current->data(Qt::UserRole))];

  if(!ping)
  {
    qDebug() << "currentDirectory_: " << currentDirectory_;
    QString fileName = currentDirectory_+"/"+current->text();
    qDebug() << "fileName: " << fileName;
    QFile file(fileName);
    currentFile_ = current->text();
    if (!file.open(QIODevice::ReadOnly)) {
      QMessageBox::warning(this, "Warning", "Cannot open file: " + file.errorString());
      return;
    }
    ping = std::make_shared<RawPing>(fileName.toStdString());
    current->setData(Qt::UserRole, fileName);
    pings_[fileName] = ping;
  }

  ui_.pingView->setPing(*ping);
}

void PingViewer::updatePixmap()
{
  std::size_t depth_steps = 2048;
  double range = 1000.0;
  double step_size = range/depth_steps;
  if(!pings_.empty())
  {
    std::size_t columns = pings_.size();
    std::size_t row_size = 4*columns;
    QImage image(columns, depth_steps, QImage::Format_RGBA8888);
    std::size_t i = 0;

    for(auto echo: pings_)
    {
      if(echo.second)
      {
        for(std::size_t j = 0; j < depth_steps; j++)
        {
          double db;// = echo.second->db(j*step_size);
          uint8_t val = 0;
          if (db > -100.0)
            val = 255*std::min(1.0, (db+100)/100.0);
          std::size_t index = j*row_size+i*4;
          image.bits()[index] = val;
          image.bits()[index+1] = val;
          image.bits()[index+2] = val;
          image.bits()[index+3] = 255;
        }
      }
      i++;
    }
   
    //pixmap_item_->setPixmap(QPixmap::fromImage(image));
  }
}
