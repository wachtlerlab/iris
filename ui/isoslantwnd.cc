
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
#include <misc.h>

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

    plot_isodata(input, plot);
}


QCPGraph *IsoslantWnd::plot_isodata(const iris::data::isodata &input, QCustomPlot *plot) {
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
}

QCPGraph *IsoslantWnd::plot_isoslant(const iris::data::isoslant &iso, QCustomPlot *plot) {

    int n_fg = 400;
    std::vector<double> xin = iris::linspace(-1.0 * M_2_PI, 2*M_PI, n_fg);

    QVector<double> y(n_fg);
    double offset = 0.66;
    std::transform(xin.begin(), xin.end(), y.begin(), [&offset, &iso](const double x){
       return offset + iso.dl * cos(x - iso.phi);
    });

    QVector<double> x(n_fg);
    std::transform(xin.begin(), xin.end(), x.begin(), [&offset, &iso](const double x){
        return x / M_PI * 180.0;
    });

    QCPGraph *points = plot->addGraph();
    points->setData(x, y);
    points->setPen(QColor(255, 0, 0));

    return points;
}

void IsoslantWnd::add_isoslant(const std::string &path) {
    load_isodata(path, customPlot);
    fs::file dir_fs(path);

    std::string root, ext;
    std::tie(root, ext) = dir_fs.splitext();

    fs::file isofd = dir_fs.parent().child(root + ".isoslant");
    if (isofd.exists()) {
        std::string data = isofd.read_all();
        iris::data::isoslant is = iris::data::store::yaml2isoslant(data);
        plot_isoslant(is, customPlot);

    }

    customPlot->rescaleAxes();
    customPlot->axisRect()->setupFullAxesBox();
    customPlot->yAxis->scaleRange(1.3, customPlot->yAxis->range().center());
    customPlot->xAxis->scaleRange(1.1, customPlot->xAxis->range().center());
    customPlot->replot();
}
