#include "wave_view.h"
#include "waveform.h"
#include <QValueAxis>
#include <QDebug>

WaveView::WaveView(QWidget *parent)
  :QChartView(parent)
{
}

WaveView::~WaveView()
{

}

void WaveView::clear()
{
  chart()->removeAllSeries();
}

void WaveView::setWaveform(Waveform* waveform)
{
  chart()->removeAllSeries();
  chart()->addSeries(waveform);
  chart()->createDefaultAxes();
  updatePing();
}

void WaveView::addWaveform(Waveform* waveform)
{
  chart()->addSeries(waveform);
  chart()->createDefaultAxes();
  updatePing();
}

void WaveView::resetZoom()
{
  chart()->zoomReset();
}

void WaveView::resizeEvent(QResizeEvent* event)
{
  updateTicks(event->size().width());

  QChartView::resizeEvent(event);
}

void WaveView::updateTicks(int width)
{
  int tick_count = std::max(2, width / 150);
  if(!chart()->axes(Qt::Horizontal).empty())
  { 
    auto a = qobject_cast<QtCharts::QValueAxis*>(chart()->axes(Qt::Horizontal).front());
    if(a)
      a->setTickCount(tick_count);
  }
}

void WaveView::mousePressEvent(QMouseEvent *event)
{
  if(event->button() == Qt::LeftButton)
  {
    panning_ = true;
    last_mouse_x = event->x();
  }
}

void WaveView::mouseMoveEvent(QMouseEvent *event)
{
  if(panning_)
  {
    int deltax = event->x() - last_mouse_x;
    chart()->scroll(-deltax, 0);
    last_mouse_x = event->x();
    updatePing();
  }
}

void WaveView::mouseReleaseEvent(QMouseEvent *event)
{
  if(event->button() == Qt::LeftButton)
  {
    panning_ = false;
  }
}

void WaveView::wheelEvent(QWheelEvent *event)
{
  qreal factor;
  if ( event->delta() > 0 )
      factor = 1.5;
  else
      factor = 1/1.5;

  QPointF mousePos = mapFromGlobal(QCursor::pos());

  QRectF r = chart()->plotArea();

  if (mousePos.rx() <= 70)
  {
    //mousePos.ry() = r.center().ry();
    r.setHeight(r.height() / factor);
    mousePos.rx() = r.center().rx();
  }
  else
  {
    r.setWidth(r.width() / factor);
    mousePos.ry() = r.center().ry();
  }
  
  r.moveCenter(mousePos);

  chart()->zoomIn(r);
  QPointF delta = chart()->plotArea().center() -mousePos;
  chart()->scroll(delta.x(),-delta.y());
  updatePing();
}

void WaveView::zoomX(const QRectF& dataArea)
{
  auto pa = chart()->plotArea();
  auto tl = chart()->mapToValue(pa.topLeft());
  auto br = chart()->mapToValue(pa.bottomRight());
  qreal xfactor = dataArea.width()/(br.rx()-tl.rx());
  pa.setWidth(pa.width()*xfactor);
  chart()->zoomIn(pa);

  auto new_tl = chart()->mapToPosition(dataArea.topLeft());
  QPointF delta = chart()->plotArea().topLeft()-new_tl;
  chart()->scroll(-delta.x(), 0.0);
}

void WaveView::updatePing()
{
  auto pa = chart()->plotArea();
  auto tl = chart()->mapToValue(pa.topLeft());
  auto br = chart()->mapToValue(pa.bottomRight());
  QRectF dataArea(tl,br);
  updateTicks(pa.width());
  emit dataAreaChanged(dataArea);
}
