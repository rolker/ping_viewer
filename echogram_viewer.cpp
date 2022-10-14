#include "echogram_viewer.h"

#include <QFileDialog>


EchogramViewer::EchogramViewer(QWidget *parent):
  QMainWindow(parent)
{
  ui_.setupUi(this);

  this->setCentralWidget(ui_.echogramViewer);

  connect(ui_.actionOpen, &QAction::triggered, this, &EchogramViewer::open);
  connect(ui_.scaleSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), ui_.echogram, &Echogram::setHorizontalScale);
  connect(ui_.minDBDoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), ui_.echogram, &Echogram::setMinDB);
  connect(ui_.maxDBDoubleSpinBox, QOverload<double>::of(&QDoubleSpinBox::valueChanged), ui_.echogram, &Echogram::setMaxDB);
  connect(ui_.echogram, &Echogram::cursorInfo, [=](const QString& message){ui_.statusbar->showMessage(message);});

  ui_.colormapComboBox->addItem("greyscale");
  QDir cmapdir(":/colormaps/");
  for(auto f: cmapdir.entryInfoList())
    ui_.colormapComboBox->addItem(f.baseName());

  connect(ui_.colormapComboBox, &QComboBox::currentTextChanged, this, &EchogramViewer::selectColormap);
}

EchogramViewer::~EchogramViewer()
{

}

void EchogramViewer::open()
{
  currentDirectory_ = QFileDialog::getExistingDirectory(this, "Select data directory");
  ui_.echogram->setDirectory(currentDirectory_.toStdString());
}

void EchogramViewer::selectColormap(const QString& colormap)
{
  QImage c(":/colormaps/"+colormap+".png");
  ui_.echogram->setColormap(c);
}

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    EchogramViewer viewer;
    viewer.show();

    return app.exec();
}
