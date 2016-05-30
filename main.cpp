#include "widget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    int cur_x,cur_y;
    QApplication a(argc, argv);   
    Widget w;


    w.setWindowTitle("Управление роботом RS10N");
    //QDesktopWidget d;

    //cur_x =d.width();     // returns desktop width
   // cur_y = d.height();    // returns desktop height

   //QApplication::desktop().screenGeometry();
  //  w.setGeometry(950,450,938,565);
    //w.setGeometry(10,27,938,565);
    w.show();

    return a.exec();
}
