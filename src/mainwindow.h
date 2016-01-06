#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <Qtcore>
#include <QtWidgets>
#include <QTimer>
#include "bookmark.h"
#include "download.h"

namespace Ui {
class TwitchStreamerApp;
}

enum class DataType {StreamStatus, Subscrition, NotRelevant,Error};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
private:
    void addBookmark(const Bookmark &bookmark, bool status);
    void removeBookmark(int index);
    void watchStream(const QUrl& url, const QString &player, const QString &quality);
    DataType DataTypeDecetection(const QJsonObject& jsonObject);
    void RefreshStreams(QString links);
    void UpdateSubscriptionList(const QJsonArray& array);
    void UpdateStreamStatus(QJsonValue& StatusValue);
    void UpdateViewerStatus(QJsonValue &ViewerValue, QJsonValue &StatusValue);
    QUrl getStreamerName();
    void SortItems();
private slots:
    void on_AddButton_clicked();

    void on_RemoveButton_pressed();

    void on_WatchButotn_clicked();

    void on_RefreshButton_clicked();

    void capture();

    void on_UpdateStatusButton_clicked();

    void on_actionExit_triggered();

    void on_checkBox_clicked(bool checked);

private:
    Ui::TwitchStreamerApp *ui;
    Download * m_download;
    QList<Bookmark> m_bookmarks;
    QList<QString> m_onlinePlayers_previous;
    QList<QString> m_onlinePlayers_now;
    QTimer * m_activateTimer;
};

#endif // MAINWINDOW_H
