#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::TwitchStreamerApp),
    m_download(new Download(this))
{
   //timer settings
   m_activateTimer = new QTimer(this);
   m_activateTimer->setInterval(60*1000);
   m_activateTimer->setSingleShot(false);
   m_activateTimer->start();

   ui->setupUi(this);
   ui->PathFile->setText("C:\\Program Files (x86)\\VideoLAN\\VLC\\vlc.exe");
   m_onlinePlayers_previous.clear();
   m_onlinePlayers_now.clear();

   //connect and slots definitions
   connect(m_download,SIGNAL(downloaded()),this,SLOT(capture()));
   connect(ui->actionAdd,SIGNAL(triggered()),this,SLOT(on_AddButton_clicked()));
   connect(ui->actionRemove,SIGNAL(triggered()),this,SLOT(on_RemoveButton_pressed()));
   connect(ui->treeWidget,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(on_WatchButotn_clicked()));
   connect(m_activateTimer,SIGNAL(timeout()),this,SLOT(on_UpdateStatusButton_clicked()));


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_AddButton_clicked()
{
    bool ok = false;
    QString txt = QInputDialog::getText(this, "Add Stream", "Stream URL (i.e. http://twitch.tv/some-stream):", QLineEdit::Normal, "http://", &ok);
    if (ok && !txt.isEmpty())
    {
        Bookmark newBookmark = Bookmark(QUrl(txt));
        foreach (Bookmark bookmark, m_bookmarks)
            if (bookmark == newBookmark)
                return;
        addBookmark(newBookmark,false);
    }
}

void MainWindow::addBookmark(const Bookmark &bookmark,bool status)
{
    m_bookmarks << bookmark;
    QStringList lst;
    if(status)
    {
        lst << bookmark.streamName() << "Live!";
    }
    else
    {
        lst << bookmark.streamName() << "Offline";
    }

    QTreeWidgetItem * item = new QTreeWidgetItem((QTreeWidget*)0, lst);
    item->setToolTip(0, bookmark.url().toString());
    item->setToolTip(1, bookmark.url().toString());
    item->setIcon(1,bookmark.StatusIcon(status));
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
    QVariant variant = indexes[0].data(Qt::DisplayRole);
    QString PlayerName = variant.toString();
    QString player = ui->PathFile->text();
    QString quality = ui->QualityBox->currentText();
    QUrl url = QUrl("http://twitch.tv/"+PlayerName);
    watchStream(url, player, quality);
}

void MainWindow::watchStream(const QUrl& url, const QString &player, const QString &quality)
{
    QStringList args;
    args << url.toString() << quality << "--player" << player;
    QProcess::startDetached("livestreamer", args);
}

void MainWindow::on_RefreshButton_clicked()
{
    //"https://api.twitch.tv/kraken/users/0midd/follows/channels"
    QString link_1 = "https://api.twitch.tv/kraken/users/";
    QString player = QInputDialog::getText(this, "Add User", "Your Twitch Username",QLineEdit::Normal,"0midd");
    QString link_2 = "/follows/channels";
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
    }
    else if(dataType == DataType::Subscrition)
    {
        QJsonValue ValueFollow = jsonObject["follows"];
        QJsonArray array = ValueFollow.toArray();
        UpdateSubscriptionList(array);
    }
    else if(dataType == DataType::Error)
    {
//        QMessageBox msgBox;
//        msgBox.setText("An Error occured getting one or more streamer's information\n");
//        msgBox.exec();
    }
    else
    {
        //do nothing
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
        addBookmark(newBookmark,false);
}

void MainWindow::on_UpdateStatusButton_clicked()
{
    m_onlinePlayers_previous.clear();
    m_onlinePlayers_previous = m_onlinePlayers_now;
    m_onlinePlayers_now.clear();
    QString urlString("https://api.twitch.tv/kraken/streams/");
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
}
