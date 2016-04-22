#ifndef LABELCLICK
#define LABELCLICK

#include <QLabel>
#include <QWidget>
#include <QMouseEvent>



class QExLabel : public QLabel

{

    Q_OBJECT

public:

    explicit QExLabel(QWidget *parent = 0) : QLabel(parent){};

signals:

    void clicked(QObject *sender);

protected:

    void mouseReleaseEvent(QMouseEvent *e)

    {

        if (e->button() == Qt::LeftButton)

        {
            emit clicked((QObject*)this);

        }

    }

};
#endif // LABELCLICK

