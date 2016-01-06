#ifndef FILEDOWNLOAD_H
#define FILEDOWNLOAD_H

#include <QObject>
#include <QByteArray>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrl>
#include <QFile>
#include <QBuffer>


class Download : public QObject
{
    Q_OBJECT
public:
    explicit Download(QObject *parent = 0);
    virtual ~Download();
    void DownloadFromUrl(QUrl Url);
    QByteArray downloadedData();
signals:
    void downloaded();
private:
    QNetworkAccessManager * m_WebCtrl;
    QByteArray m_DownloadedData;
private slots:
    void fileDonwloaded(QNetworkReply* pReply);
};

#endif // FILEDOWNLOAD_H
