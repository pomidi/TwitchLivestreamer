#include "download.h"

Download::Download(QObject *parent) : QObject(parent)
{
    m_WebCtrl = new QNetworkAccessManager(this);
    connect(m_WebCtrl,SIGNAL(finished(QNetworkReply*)),
            this,SLOT(fileDonwloaded(QNetworkReply*)));
}

Download::~Download(){delete m_WebCtrl;}

void Download::DownloadFromUrl(QUrl Url)
{
    QNetworkRequest request(Url);
    m_WebCtrl->get(request);
}

void Download::fileDonwloaded(QNetworkReply *pReply)
{
    m_DownloadedData = pReply->readAll();
    pReply->deleteLater();
    emit downloaded();
}

QByteArray Download::downloadedData()
{
    QByteArray temp = m_DownloadedData;
    m_DownloadedData.clear();
    return temp;
}
