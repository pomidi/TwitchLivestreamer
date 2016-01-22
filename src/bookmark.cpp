#include "bookmark.h"

Bookmark::Bookmark(const QUrl& url):
    mUrl(url)
{

}

QUrl Bookmark::url() const
{
    return mUrl;
}

void Bookmark::setUrl(const QUrl &url)
{
    mUrl = url;
}

QString Bookmark::streamName() const
{
    if (mUrl.path().length() < 5) return "N/A";
    {
        QStringList parts = mUrl.path().split('/');
        return (parts.length() > 1) ? parts[1] : "N/A";
    }
}

QIcon Bookmark::StatusIcon(bool status) const
{
    if(status)
    {
        return QIcon(":/online.png");
    }
    else
    {
        return QIcon(":/offline.png");
    }
}
