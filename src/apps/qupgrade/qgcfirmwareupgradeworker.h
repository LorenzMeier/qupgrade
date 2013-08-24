#ifndef QGCFirmwareUpgradeWorker_H
#define QGCFirmwareUpgradeWorker_H

#include <QObject>
#include <qextserialport.h>

class QGCFirmwareUpgradeWorker : public QObject
{
    Q_OBJECT
public:
    explicit QGCFirmwareUpgradeWorker(QObject *parent = 0);
    static QGCFirmwareUpgradeWorker* putWorkerInThread(const QString &filename, const QString &port="", int boardId=0);
    static QGCFirmwareUpgradeWorker* putDetectorInThread();

signals:
    void detectionStatusChanged(const QString& status);
    void upgradeStatusChanged(const QString& status);
    void upgradeProgressChanged(int percent);
    void validPortFound(const QString& portName);
    void loadFinished(bool success);
    void detectFinished(bool success, int boardid);
    void finished();
    
public slots:

    /**
     * @brief Continously scan for bootloaders
     * @return
     */
    void startContinousScan();

    /**
     * @brief Aborts a currently running upload
     */
    void abortUpload();

    /**
     * @brief Set firmware filename
     * @param filename
     */
    void setFilename(const QString &filename);

    /**
     * @brief Load firmware to board
     */
    void loadFirmware();

    /**
     * @brief Detect boards
     */
    void detectBoards();

    /**
     * @brief Set the board ID this uploader only accepts
     * @param id
     */
    void setBoardId(int id);

    /**
     * @brief Set a fixed port name, do not perform automatic scanning
     * @param port
     */
    void setPort(const QString &port);

    /**
     * @brief Abort upgrade worker
     */
    void abort();

protected:
    bool exitThread;

private:
    bool _abortUpload;
    int _filterBoardId;
    QString _fixedPortName;
    QextSerialPort *port;
    QString filename;
};

#endif // QGCFIRMWAREUPGRADEWORKER_H
