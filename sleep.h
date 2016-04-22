#ifndef SLEEP
#define SLEEP
#include <QThread>

class SleeperThread : public QThread
{
    public:
        inline static void usleep(unsigned long microseconds)
        {
            QThread::usleep(microseconds);
        }
        inline static void msleep(unsigned long milliseconds)
        {
            QThread::msleep(milliseconds);
        }
        inline static void sleep(unsigned long seconds)
        {
            QThread::sleep(seconds);
        }
};

#endif // SLEEP

