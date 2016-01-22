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
    int screenHeight = QApplication::desktop()->screenGeometry().height();
    int x1_sysTray = 0, y1_sysTray = 0, x2_sysTray = 0 , y2_sysTray = 0;
    int x1_menubar= 0,y1_menubar =0, x2_menubar = 0, y2_menubar = 0;
    int TrayWidth = x2_sysTray -x1_sysTray;

    sysTrayGeo.getCoords(&x1_sysTray, &y1_sysTray, &x2_sysTray, &y2_sysTray);

    x1_menubar = x1_sysTray + (TrayWidth / 2) - (this->width() / 2);
    x2_menubar = x1_menubar + this->width() - 1;

    if( y1_sysTray > (screenHeight / 2) )   //icon is on bottom
    {
        y1_sysTray-=10;
        y1_menubar = y1_sysTray - this->height();
        y2_menubar = y1_sysTray -1;
    }
    else    //icon is on top
    {
        y1_sysTray +=y2_sysTray;
        y1_menubar = y1_sysTray;
        y2_menubar = y1_menubar + this->height();
    }

    QRect newRect;
    newRect.setCoords( x1_menubar ,y1_menubar, x2_menubar,y2_menubar);
    return newRect;
}

void SystemTrayDialog::AddtoTree(const QString& stream,const QString& viewers)
{
    m_streamers << stream;
    QTreeWidgetItem * item = new QTreeWidgetItem((QTreeWidget*)0);
    item->setText(0,stream);
    item->setText(1,viewers);
    ui->treeWidget->addTopLevelItem(item);
    ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->treeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
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
    this->hide();
    emit OpenStream(indexes[0].data(Qt::DisplayRole).toString());
}

void SystemTrayDialog::on_ExitBtn_clicked()
{
    emit CloseDialog();
}

void SystemTrayDialog::on_AddBtn_clicked()
{
    this->hide();
    emit ShowMainWindow();
}
