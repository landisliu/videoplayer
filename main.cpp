#include "mainwindow.h"
#include <QApplication>

extern "C"
{
#include "SDL.h"
}

#ifdef __MINGW32__
#undef main /* Prevents SDL from overriding main() */
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();

    return a.exec();
}
