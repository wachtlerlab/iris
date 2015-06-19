
#include <QApplication>
#include "isoslantwnd.h"

int main(int argc, char **argv) {

    QApplication app(argc, argv);
    IsoslantWnd w;
    w.show();

    w.add_isoslant(argv[1]);

    return app.exec();
}
