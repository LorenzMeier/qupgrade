#ifndef QGC_H
#define QGC_H

#include <QDateTime>
#include <QThread>

class QGC
{
public:
    static quint64 groundTimeUsecs()
    {
        QDateTime time = QDateTime::currentDateTime();
        time = time.toUTC();
        /* Return seconds and milliseconds, in milliseconds unit */
        quint64 microseconds = time.toTime_t() * static_cast<quint64>(1000000);
        return static_cast<quint64>(microseconds + (time.time().msec()*1000));
    }

    static quint64 groundTimeMilliseconds()
    {
        QDateTime time = QDateTime::currentDateTime();
        time = time.toUTC();
        /* Return seconds and milliseconds, in milliseconds unit */
        quint64 seconds = time.toTime_t() * static_cast<quint64>(1000);
        return static_cast<quint64>(seconds + (time.time().msec()));
    }

    static qreal groundTimeSeconds()
    {
        QDateTime time = QDateTime::currentDateTime();
        time = time.toUTC();
        /* Return time in seconds unit */
        quint64 seconds = time.toTime_t();
        return static_cast<qreal>(seconds + (time.time().msec() / 1000.0));
    }
    
    class SLEEP : public QThread
    {
    public:
        /**
         * @brief Set a thread to sleep for seconds
         * @param s time in seconds to sleep
         **/
        static void sleep(unsigned long s) {
            QThread::sleep(s);
        }
        /**
         * @brief Set a thread to sleep for milliseconds
         * @param ms time in milliseconds to sleep
         **/
        static void msleep(unsigned long ms) {
            QThread::msleep(ms);
        }
        /**
         * @brief Set a thread to sleep for microseconds
         * @param us time in microseconds to sleep
         **/
        static void usleep(unsigned long us) {
            QThread::usleep(us);
        }
    };

};

#endif // QGC_H
