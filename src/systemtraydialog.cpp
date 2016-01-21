#include "systemtraydialog.h"
#include "ui_systemtraydialog.h"
#include <QDebug>
#include <QDesktopWidget>
#include <QCloseEvent>

SystemTrayDialog::SystemTrayDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SystemTrayDialog)
{
    ui->setupUi(this);
    connect(ui->treeWidget,SIGNAL(doubleClicked(QModelIndex)),this,SLOT(ConnectToMainWindow()));
}

QRect SystemTrayDialog::newGeometry(const QRect& sysTrayGeo)
{
    QRect ScreenGeometry = QApplication::desktop()->screenGeometry();
    int screenHeight = ScreenGeometry.height();
    int x = 0,y = 0;
    int trayWidth = 0 , trayHeight = 0;
    sysTrayGeo.getCoords(&x, &y, &trayWidth, &trayHeight);

    int width = this->width();
    int height = this->height();

    QRect newRect;
    int x1= 0,y1 =0, y2 = 0;
    trayWidth -= x;

    x1 = x + (trayWidth/2) - (width/2);
    if( y > screenHeight/2)
    {
        y-=10;
        y1 = y-height;
        y2 = y -1;
        //icon is on bottom
    }
    else
    {
        y +=trayHeight;
        y1 = y;//+height;
        y2 = y1+height;
    }

    newRect.setCoords( x1 ,y1, x1 + width-1,y2);
    return newRect;
}

void SystemTrayDialog::AddtoTree(const QString& stream,const QString& viewers)
{
    m_streamers << stream;
    QTreeWidgetItem * item = new QTreeWidgetItem((QTreeWidget*)0);
    item->setText(0,stream);
    item->setText(1,viewers);
    item->setIcon(0,QIcon("://online.jpg"));
    ui->treeWidget->addTopLevelItem(item);
}

void SystemTrayDialog::InitializeTree()
{
    ui->treeWidget->clear();
    m_streamers.clear();
}

void SystemTrayDialog::closeEvent(QCloseEvent *event)
{
    if(event){
        event->ignore();
        this->hide();
    }
}

SystemTrayDialog::~SystemTrayDialog()
{
    delete ui;
}


void SystemTrayDialog::on_RefreshBtn_clicked()
{
    emit RefreshDialog();
}

void SystemTrayDialog::ConnectToMainWindow()
{
    QModelIndexList indexes = ui->treeWidget->selectionModel()->selectedRows();
    emit OpenStream(indexes[0].data(Qt::DisplayRole).toString());
}

void SystemTrayDialog::on_ExitBtn_clicked()
{
    emit CloseDialog();
}

void SystemTrayDialog::on_AddBtn_clicked()
{
    emit ShowMainWindow();
}
