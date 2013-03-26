#ifndef QGCFirmwareUpgradeWorker_H
#define QGCFirmwareUpgradeWorker_H

#include <QObject>

#include <SerialLink.h>

class QGCFirmwareUpgradeWorker : public QObject
{
    Q_OBJECT
public:
    explicit QGCFirmwareUpgradeWorker(QObject *parent = 0);
    static QGCFirmwareUpgradeWorker* putWorkerInThread(QObject *parent);

signals:
    void detectionStatusChanged(const QString& status);
    void upgradeStatusChanged(const QString& status);
    void upgradeProgressChanged(int percent);
    void validPortFound(const QString& portName);
    
public slots:

    /**
     * @brief Continously scan for bootloaders
     * @return
     */
    void startContinousScan();

    /**
     * @brief Detect connected PX4 bootloaders
     *
     * If a bootloader was found, the link will be opened to this
     * bootloader and ready for flashing when returning from the call.
     *
     * @return true if found on one link, false else
     */
    void detect();

    /**
     * @brief Upgrade the firmware using the currently connected link
     * @param filename file name / path of the firmware file
     */
    void upgrade();

    /**
     * @brief Load firmware from disk into memory
     * @param filename
     */
    void loadFirmware(const QString &filename);

    /**
     * @brief Receive bytes from a link
     * @param link
     * @param b
     */
    void receiveBytes(LinkInterface* link, QByteArray b);

    /**
     * @brief Abort upgrade worker
     */
    void abort();

protected:
    bool exitThread;

private:
    SerialLink *link;
    bool insync;
    //QJsonDocument firmware;
};

#endif // QGCFirmwareUpgradeWorker_H
