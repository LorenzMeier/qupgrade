#ifndef DIALOG_H
#define DIALOG_H

#include <QWidget>
#include <QSerialPort>
#include <QUrl>

namespace Ui {
    class Dialog;
}
class QTimer;
class QextSerialPort;
class QextSerialEnumerator;
class QGCFirmwareUpgradeWorker;
class QNetworkRequest;
class QNetworkReply;

class Dialog : public QWidget
{
    Q_OBJECT

public:
    explicit Dialog(QWidget *parent = 0);
    ~Dialog();

signals:
    void filenameSet(QString filename);

protected:
    void changeEvent(QEvent *e);
    void loadSettings();
    void storeSettings();
    void updateBoardId(const QString &fileName);

public slots:
    void onPortAddedOrRemoved();
    void onLoadStart();
    void onLoadFinished(bool success);
    void onDownloadProgress(qint64 curr, qint64 total);

private slots:
    void onPortNameChanged(const QString &name);
    void onFileSelectRequested();
    void onCancelButtonClicked();
    void onUploadButtonClicked();
    void onDownloadFinished();
    void onDownloadRequested(const QNetworkRequest &request);
    void onHomeRequested();
    void onLinkClicked(const QUrl&);
    void onToggleAdvancedMode(bool enabled);

private:
    bool loading;
    Ui::Dialog *ui;
    QTimer *timer;
    QSerialPort *port;
    QGCFirmwareUpgradeWorker *worker;

    QString lastFilename;
};

#endif // DIALOG_H
