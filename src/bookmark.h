#ifndef BOOKMARK_H
#define BOOKMARK_H

#include <QtCore>
#include <QUrlQuery>
#include <QIcon>

class Bookmark
{
public:
    Bookmark(const QUrl &url);

    enum Status
    {
        Live = 0,
        Offline
    };

    QUrl url() const;
    void setUrl(const QUrl &url);
    QString streamName() const;
    QIcon StatusIcon(bool status) const;

    inline bool operator ==(const Bookmark &other)
    {
        return (url() == other.url());
    }
private:
    QUrl mUrl;
};

#endif // BOOKMARK_H
