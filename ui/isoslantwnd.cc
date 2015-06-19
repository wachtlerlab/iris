
#include <QDebug>
#include <QDesktopWidget>
#include <QScreen>
#include <QMessageBox>
#include <QMetaEnum>

#include "isoslantwnd.h"
#include <iris.h>
#include <misc.h>

#include <numeric>
#include <algorithm>
#include <csv.h>
#include <fit.h>
#include <data.h>
#include <fs.h>

IsoslantWnd::IsoslantWnd(QWidget *parent) :
        QMainWindow(parent)
{
    setupUi(this);
    setGeometry(400, 250, 542, 390);

    setWindowTitle("iris isoslant");
    QMainWindow::statusBar()->clearMessage();
    customPlot->replot();
}

void IsoslantWnd::load_isodata(const std::string &path, QCustomPlot *plot) {
    fs::file fd(path);
    std::string raw = fd.read_all();
    iris::data::isodata input = iris::data::store::yaml2isodata(raw);

    QVector<double> x(input.samples.size());
    QVector<double> y(input.samples.size());
    std::transform(input.samples.cbegin(), input.samples.cend(), x.begin(),
                   [](const iris::data::isodata::sample &s) {
                       return s.stimulus  / M_PI * 180.0;
                   });

    std::transform(input.samples.cbegin(), input.samples.cend(), y.begin(),
                   [](const iris::data::isodata::sample &s) {
                       return s.response;
                   });

    QCPGraph *points = plot->addGraph();
    points->setData(x, y);
    points->setPen(QColor(30, 40, 255));
    points->setLineStyle(QCPGraph::lsNone);
    points->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 4));
    points->setName("Measured data");
    points->rescaleAxes();
    plot->axisRect()->setupFullAxesBox();
}

void IsoslantWnd::add_isoslant(const std::string &path) {
    load_isodata(path, customPlot);
    customPlot->replot();
}
