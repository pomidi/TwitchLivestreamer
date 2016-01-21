#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtCore>
#include <QtWidgets>
#include <QTimer>
#include "bookmark.h"
#include "download.h"
#include "systemtraydialog.h"

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
    void closeEvent (QCloseEvent *event);

    void addBookmark(const Bookmark &bookmark);

    void removeBookmark(int index);

    void watchStream(const QUrl& url, const QString &player, const QString &quality);

    DataType DataTypeDecetection(const QJsonObject& jsonObject);

    void RefreshStreams(QString links);

    void UpdateSubscriptionList(const QJsonArray& array);

    void UpdateStreamStatus(QJsonValue& StatusValue);

    void UpdateViewerStatus(QJsonValue &ViewerValue, QJsonValue &StatusValue);

    QUrl getStreamerName();

    void SortItems();

    void SaveSettings();

    void LoadSettings();

    void UpdateIcon();

private slots:
    void on_AddButton_clicked();

    void on_RemoveButton_pressed();

    void on_WatchButotn_clicked();

    void on_GetStream_clicked();

    void capture();

    void on_UpdateStatusButton_clicked();

    void on_actionExit_triggered();

    void on_checkBox_clicked(bool checked);

    void on_BrowseButton_clicked();

    void UpdateTimerText();

    void ShowWindow(QSystemTrayIcon::ActivationReason reason);

    void WatchFromMenu(QString stream);

    void RealClose();

    void updateDialogContent();

private:

    Ui::TwitchStreamerApp *ui;

    Download * m_download;

    QList<Bookmark> m_bookmarks;

    QList<QString> m_onlinePlayers_previous;

    QList<QString> m_onlinePlayers_now;

    QTimer * m_activateTimer;

    QSystemTrayIcon * m_sysTray;

    QMenu * m_menubar;

    QString m_username;

    QStringList m_urls;

    int m_timer;

    bool m_fromMenubar;

    SystemTrayDialog * m_dialog;

};

#endif // MAINWINDOW_H
