#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qglobal.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TwitchStreamerApp),
    m_download(new Download(this)),
    m_activateTimer(new QTimer(this)),
    m_sysTray(new QSystemTrayIcon(this)),
    m_menubar(new QMenu(this)),
    m_dialog(new SystemTrayDialog(this))
{
   //timer settings
   m_activateTimer->setInterval(1000);
   m_activateTimer->setSingleShot(false);
   m_activateTimer->start();
   m_timer = 0;
   m_fromMenubar = false;
   m_enable_notifications = true;
   ui->setupUi(this);

#ifdef WIN32
   ui->PathFile->setText("C:\\Program Files (x86)\\VideoLAN\\VLC\\vlc.exe");
#else
   ui->PathFile->setText("vlc");
#endif

   LoadSettings();
   m_onlinePlayers_previous.clear();
   m_onlinePlayers_now.clear();

   m_sysTray->setIcon(QIcon("://logo.png"));
   m_sysTray->show();

   m_dialog->setModal(true);

   m_dialog->setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
   connect(m_download,SIGNAL(downloaded()),this,SLOT(capture()));
   connect(ui->actionAdd,SIGNAL(triggered()),this,SLOT(on_AddButton_clicked()));
   connect(ui->actionRemove,SIGNAL(triggered()),this,SLOT(on_RemoveButton_pressed()));
   connect(ui->treeWidget,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(on_WatchButotn_clicked()));
   connect(m_activateTimer,SIGNAL(timeout()),this,SLOT(UpdateTimerText()));
   connect(m_sysTray,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,SLOT(ShowWindow(QSystemTrayIcon::ActivationReason)));
   connect(m_dialog,SIGNAL(CloseDialog()),this,SLOT(RealClose()));
   connect(m_dialog,SIGNAL(RefreshDialog()),this,SLOT(on_UpdateStatusButton_clicked()));
   connect(m_dialog,SIGNAL(OpenStream(QString)),this,SLOT(WatchFromMenu(QString)));
   connect(m_dialog,SIGNAL(ShowMainWindow()),this,SLOT(Show()));
}

MainWindow::~MainWindow()
{
    SaveSettings();
    delete m_download;
    delete m_activateTimer;
    delete ui;
    delete m_sysTray;

}


void MainWindow::RealClose()
{
    m_fromMenubar = true;
    this->close();
}

void MainWindow::closeEvent (QCloseEvent *event)
{

    if(m_fromMenubar)
    {
        m_fromMenubar = false;
        event->accept();
    }
    else
    {
        event->ignore();
        this->hide();
        if(m_enable_notifications)
            m_sysTray->showMessage("Info","App is still running. If you want to close it, you should do it from the Menubar",QSystemTrayIcon::Information,2000);
    }


}

void MainWindow::WatchFromMenu(QString stream)
{
    QUrl streamUrl = QUrl("http://twitch.tv/" + stream);
    QString player = ui->PathFile->text();
    QString quality = ui->QualityBox->currentText();
    watchStream(streamUrl,player,quality);
}

void MainWindow::ShowWindow(QSystemTrayIcon::ActivationReason)
{
    QRect rect = m_dialog->newGeometry(m_sysTray->geometry());
    m_dialog->setGeometry(rect);
    if(m_dialog->isActiveWindow())
    {
        m_dialog->hide();
    }
    else
    {
         m_dialog->show();
         m_dialog->activateWindow();
    }

    updateDialogContent();
}

void MainWindow::updateDialogContent()
{
    m_dialog->InitializeTree();
    for(int index = 0; index < m_bookmarks.size();++index)
    {
       QTreeWidgetItem * item = ui->treeWidget->topLevelItem(index);
       if(m_onlinePlayers_now.contains(item->text(0)))
       {
            m_dialog->AddtoTree(item->text(0),item->text(2));
       }
    }
}

void MainWindow::Show()
{
    this->showNormal();
    this->activateWindow();
}

void MainWindow::on_AddButton_clicked()
{
    bool ok = false;
    QString txt = QInputDialog::getText(this, "Add Stream", "Stream URL (i.e. http://twitch.tv/some-stream):", QLineEdit::Normal, "http://twitch.tv/", &ok);
    txt = txt.toLower();
    if (ok && !txt.isEmpty())
    {
        Bookmark newBookmark = Bookmark(QUrl(txt));
        foreach (Bookmark bookmark, m_bookmarks)
            if (bookmark == newBookmark)
                return;
        addBookmark(newBookmark);
    }
}

void MainWindow::addBookmark(const Bookmark &bookmark)
{
    m_bookmarks << bookmark;
    QStringList lst;
    lst << bookmark.streamName() << "Offline";

    QTreeWidgetItem * item = new QTreeWidgetItem((QTreeWidget*)0, lst);
    item->setToolTip(0, bookmark.url().toString());
    item->setToolTip(1, bookmark.url().toString());
    item->setIcon(1,bookmark.StatusIcon(false));
    item->setText(2,"-");
    ui->treeWidget->addTopLevelItem(item);
}

void MainWindow::on_RemoveButton_pressed()
{
    QModelIndexList indexes = ui->treeWidget->selectionModel()->selectedRows();

    foreach (QModelIndex index, indexes)
    {
        if (index.row() < 0 || index.row() >= m_bookmarks.length())
            continue;

        removeBookmark(index.row());
    }
}

void MainWindow::removeBookmark(int index)
{
    m_bookmarks.removeAt(index);
    delete ui->treeWidget->topLevelItem(index);
}

void MainWindow::on_WatchButotn_clicked()
{
    QModelIndexList indexes = ui->treeWidget->selectionModel()->selectedRows();
    if(indexes.isEmpty())
    {
        QMessageBox msgBox;
        msgBox.setText("First select a channel to watch!");
        msgBox.exec();
        return;
    }
    QVariant variant = indexes[0].data(Qt::DisplayRole);
    QString PlayerName = variant.toString();
    QString player = ui->PathFile->text();
    QString quality = ui->QualityBox->currentText();
    QUrl url = QUrl("http://twitch.tv/"+PlayerName);
    watchStream(url, player, quality);
}

void MainWindow::watchStream(const QUrl& url, const QString &player, const QString &quality)
{
    if(player.isEmpty())
    {
        QMessageBox msgBox;
        msgBox.setText("No player selected!");
        msgBox.exec();
        return;
    }
    QStringList args;
    args << url.toString() << quality << "--player" << player;
    QString livestreamer;

    if(ui->LivestreamerPath->text().isEmpty() ||  (ui->LivestreamerPath->text() == "livestreamer")){
        livestreamer = "livestreamer";
    }
    else
    {
        livestreamer = ui->LivestreamerPath->text();
    }
    if(!QProcess::startDetached(livestreamer, args))
    {
        QMessageBox msgBox;
        msgBox.setText("Could not find livestreamer on your computer.\n");
        msgBox.exec();
    }
}

void MainWindow::on_GetStream_clicked()
{
    QString link_1 = "https://api.twitch.tv/kraken/users/";
    QString player;
    if(m_username.isEmpty())
    {
        player = QInputDialog::getText(this, "Add User", "Your Twitch Username",QLineEdit::Normal,"");
    }
    else
    {
        player = QInputDialog::getText(this, "Add User", "Your Twitch Username",QLineEdit::Normal,m_username);
    }

    QString link_2 = "/follows/channels";
    m_username = player;
    QUrl url(link_1+player+link_2);
    m_download->DownloadFromUrl(url);

}

void MainWindow::capture()
{
    QString Jstring = m_download->downloadedData();
    QJsonDocument JResponse = QJsonDocument::fromJson(Jstring.toUtf8());
    QJsonObject jsonObject = JResponse.object();
    DataType dataType = DataTypeDecetection(jsonObject);

    if(dataType == DataType::StreamStatus)
    {
        QJsonValue ValueFollow = jsonObject["stream"];
        QJsonObject StatusObject = ValueFollow.toObject();
        QJsonValue StatusValue = StatusObject["channel"];
        QJsonValue ViewerValue = StatusObject["viewers"];
        UpdateStreamStatus(StatusValue);
        UpdateViewerStatus(ViewerValue,StatusValue);
        return;
    }
    if(dataType == DataType::Subscrition)
    {
        QJsonValue ValueFollow = jsonObject["follows"];
        QJsonArray array = ValueFollow.toArray();
        UpdateSubscriptionList(array);
        return;
    }
}

DataType MainWindow::DataTypeDecetection(const QJsonObject& jsonObject)
{
    if(jsonObject.contains("follows"))
    {
        return DataType::Subscrition;
    }
    else if(jsonObject.contains("stream"))
    {
        QJsonValue value = jsonObject["stream"];
        QJsonObject object = value.toObject();
        if(object.contains("_id"))
            return DataType::StreamStatus;
        else
            return DataType::NotRelevant;
    }
    else
    {
        return DataType::Error;
    }
}

void MainWindow::UpdateSubscriptionList(const QJsonArray& array)
{
    for(int i = 0; i < array.size();i++)
    {
        QJsonObject linkArray = array[i].toObject();
        QJsonValue channelValue = linkArray["channel"];

        QJsonObject ChannelObject = channelValue.toObject();
        QJsonValue UrlValue = ChannelObject["url"];

        QString link = UrlValue.toString();
        RefreshStreams(link);
    }
    emit(on_UpdateStatusButton_clicked());
}

void MainWindow::UpdateStreamStatus(QJsonValue &StatusValue)
{
    QJsonObject statusObject = StatusValue.toObject();
    QJsonValue arrayValue = statusObject["name"];
    QString name = arrayValue.toString();
    if(!m_onlinePlayers_now.contains(name))
        m_onlinePlayers_now << name;
    for(int index = 0; index < m_bookmarks.size(); ++index)
    {
        QTreeWidgetItem * item = ui->treeWidget->topLevelItem(index);
        if(item->text(0) == name)
        {
            item->setText(1,"Live!");
            item->setIcon(1,m_bookmarks[index].StatusIcon(true));
        }
        else
        {
            if(m_onlinePlayers_previous.contains(item->text(0)) && !m_onlinePlayers_now.contains(item->text(0)))
            {
                item->setText(1,"Offline");
                item->setIcon(1,m_bookmarks[index].StatusIcon(false));
                item->setText(2,"-");
            }
        }
    }
    UpdateIcon();
    SortItems();
}

void MainWindow::UpdateViewerStatus(QJsonValue &ViewerValue,QJsonValue &StatusValue)
{
    QJsonObject statusObject = StatusValue.toObject();
    QJsonValue nameValue = statusObject["name"];
    QString name = nameValue.toString();
    QString viewer = QString::number(ViewerValue.toInt());
    for(int index = 0; index < m_bookmarks.size(); ++index)
    {
        QTreeWidgetItem * item = ui->treeWidget->topLevelItem(index);
        if(item->text(0) == name)
        {
            item->setText(2,viewer);
        }
    }
    updateDialogContent();
}

void MainWindow::SortItems()
{
    ui->treeWidget->sortItems(1,Qt::SortOrder::AscendingOrder);
}

void MainWindow::RefreshStreams(QString link)
{
        Bookmark newBookmark = Bookmark(QUrl(link));
        foreach (Bookmark bookmark, m_bookmarks)
            if (bookmark == newBookmark)
                return;
        addBookmark(newBookmark);
}

void MainWindow::on_UpdateStatusButton_clicked()
{
    m_timer = 0;
    UpdateIcon();

    m_onlinePlayers_previous.clear();
    m_onlinePlayers_previous = m_onlinePlayers_now;
    m_onlinePlayers_now.clear();

    QString urlString("https://api.twitch.tv/kraken/streams/");
    ui->UpdateStatusText->setText("Loading...");
    for(int index = 0; index < m_bookmarks.size();index++)
    {
        QTreeWidgetItem * item = ui->treeWidget->topLevelItem(index);
        QString urlNameString = item->text(0);
        QUrl url = QUrl(urlString+urlNameString);
        m_download->DownloadFromUrl(url);
    }
}

void MainWindow::on_actionExit_triggered()
{
    this->close();
}

void MainWindow::on_checkBox_clicked(bool checked)
{
     for(int index = 0; index < m_bookmarks.size();index++)
     {
          QTreeWidgetItem * item = ui->treeWidget->topLevelItem(index);
          if(item->text(1) == "Offline")
          {
              item->setHidden(checked);
          }
     }
}

void MainWindow::on_BrowseButton_clicked()
{
    QString  fileName = QFileDialog::getOpenFileName(this);
    ui->PathFile->setText(fileName);
    if(fileName.isEmpty())
    {
#ifdef WIN32
        ui->PathFile->setText("C:\\Program Files (x86)\\VideoLAN\\VLC\\vlc.exe");
#else
        ui->PathFile->setText("vlc");
#endif
    }
}

void MainWindow::SaveSettings()
{
    QSettings setting(QSettings::IniFormat,QSettings::UserScope,"TwitchLivestreamer");

    setting.remove("bookmarks");
    setting.remove("VLCPath");
    setting.remove("username");
    setting.remove("Enable_notification");
    setting.setValue("VLCPath",ui->PathFile->text());
    setting.setValue("username",m_username);
    setting.setValue("LivestreamerPath",ui->LivestreamerPath->text());
    if(ui->NotificationCheck->isChecked())
    {
        setting.setValue("Enable_notification","true");
    }
    else
    {
        setting.setValue("Enable_notification","false");
    }

    m_urls.clear();

    foreach(Bookmark bookmark,m_bookmarks){
        if(!m_urls.contains( bookmark.url().toString()))
            m_urls << bookmark.url().toString();
    }

    setting.setValue("bookmarks",m_urls);
}

void MainWindow::LoadSettings()
{
    QSettings setting(QSettings::IniFormat,QSettings::UserScope,"TwitchLivestreamer");
    m_username = setting.value("username").toString();
    QString path = setting.value("VLCPath").toString();
    if(setting.value("Enable_notification").toString() == "true")
    {
        ui->NotificationCheck->setChecked(true);
        m_enable_notifications = false;
    }
    else
    {
        ui->NotificationCheck->setChecked(false);
        m_enable_notifications = true;
    }

    if(path.isEmpty())
    {
#ifdef WIN32
        ui->PathFile->setText("C:\\Program Files (x86)\\VideoLAN\\VLC\\vlc.exe");
#else
        ui->PathFile->setText("vlc");
#endif
    }
    else
    {
        ui->PathFile->setText(path);
    }

    m_urls = setting.value("bookmarks").toStringList();

    for(int i =0 ; i < m_urls.size();i++)
    {
        addBookmark(Bookmark(QUrl(m_urls[i])));
    }

    QString livestreamerPath = setting.value("LivestreamerPath").toString();

    if(livestreamerPath.isEmpty())
    {
        ui->LivestreamerPath->setText("livestreamer");
    }
    else
    {
        ui->LivestreamerPath->setText(livestreamerPath);
    }

    on_UpdateStatusButton_clicked();
}

void MainWindow::UpdateTimerText()
{
    m_timer++;
    QString time = QString::number(61-m_timer);
    ui->UpdateStatusText->setText("updating the streams in " + time + " s");

    if(m_timer == 60)
    {
        on_UpdateStatusButton_clicked();
    }
}

void MainWindow::UpdateIcon()
{
     int nPlayers = m_onlinePlayers_now.size();
     if(nPlayers >= 9)
     {
         nPlayers = 9;
     }
     QString sPlayer = QString::number(nPlayers);
     m_sysTray->setIcon(QIcon("://" + sPlayer + ".png"));
}

void MainWindow::on_NotificationCheck_clicked(bool checked)
{
    if(checked)
    {
        m_enable_notifications = false;
    }
    else
    {
        m_enable_notifications = true;
    }
}
