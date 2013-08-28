#ifndef DialogBare_H
#define DialogBare_H

#include <QWidget>
#include <QUrl>

namespace Ui {
    class DialogBare;
}
class QTimer;
class QextSerialPort;
class QextSerialEnumerator;
class QGCFirmwareUpgradeWorker;
class QNetworkRequest;
class QNetworkReply;

class DialogBare : public QWidget
{
    Q_OBJECT

public:
    explicit DialogBare(QWidget *parent = 0);
    ~DialogBare();

signals:
    void filenameSet(QString filename);
    void connectLinks();
    void disconnectLinks();

protected:
    void changeEvent(QEvent *e);
    void loadSettings();
    void storeSettings();
    void updateBoardId(const QString &fileName);

public slots:
    void onPortAddedOrRemoved();
    void onLoadStart();
    void onLoadFinished(bool success);
    void onUserAbort();
    void onDetectFinished(bool success, int board_id, const QString &boardName, const QString &bootLoader);
    void onFlashURL(const QString &url);
    void onDownloadProgress(qint64 curr, qint64 total);

private slots:
    void onPortNameChanged(const QString &name);
    void onFileSelectRequested();
    void onCancelButtonClicked();
    void onUploadButtonClicked();
    void onDetect();
    void onDownloadFinished();
    void onDownloadRequested(const QNetworkRequest &request);
    void onLinkClicked(const QUrl&);
    void onToggleAdvancedMode(bool enabled);

private:
    bool loading;
    Ui::DialogBare *ui;
    QTimer *timer;
    QextSerialPort *port;
    QextSerialEnumerator *enumerator;
    QGCFirmwareUpgradeWorker *worker;
    QWidget *boardFoundWidget;

    QString lastFilename;
};

#endif // DialogBare_H
