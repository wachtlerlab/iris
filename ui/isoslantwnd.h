
#ifndef IRIS_UI_ISOSLANTWND_H
#define IRIS_UI_ISOSLANTWND_H

#include <QMainWindow>
#include <QTimer>
#include "qcustomplot.h"

#include <iris.h>
#include <data.h>

namespace Ui {
class IsoslantWnd;
}

#include <ui_isoslantwnd.h>

class IsoslantWnd : public QMainWindow, Ui::IsoslantWnd
{
  Q_OBJECT
  
public:
  explicit IsoslantWnd(QWidget *parent = 0);

  static void load_isodata(const std::string &path,  QCustomPlot *plot);
  static QCPGraph *plot_isoslant(const iris::data::isoslant &iso, QCustomPlot *plot);
  void add_isoslant(const std::string &path);

};

#endif // IRIS_UI_ISOSLANTWND_H
