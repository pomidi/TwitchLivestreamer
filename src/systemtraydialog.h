#ifndef SYSTEMTRAYDIALOG_H
#define SYSTEMTRAYDIALOG_H

#include <QDialog>

namespace Ui {
class SystemTrayDialog;
}

class SystemTrayDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SystemTrayDialog(QWidget *parent = 0);

    QRect newGeometry(const QRect &sysTrayGeo);

    void AddtoTree(const QString &stream, const QString &viewers);

    void InitializeTree();

    void closeEvent (QCloseEvent *event);

    ~SystemTrayDialog();

signals:
    void CloseDialog();

    void RefreshDialog();

    void OpenStream(QString);

    void ShowMainWindow();

private slots:

    void on_RefreshBtn_clicked();

    void ConnectToMainWindow();

    void on_ExitBtn_clicked();

    void on_AddBtn_clicked();

private:

    Ui::SystemTrayDialog *ui;

    QStringList m_streamers;
};

#endif // SYSTEMTRAYDIALOG_H
