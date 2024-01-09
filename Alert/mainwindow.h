#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStringListModel>
#include "../Common/udp.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
public slots:
    void MessageReceived(const QString& message);

private:
    Ui::MainWindow *ui;
    Udp udp;
    QStringList list;
    QStringListModel model;
};
#endif // MAINWINDOW_H
