#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStandardItemModel>
#include "../Common/udp.h"
#include <QJsonArray>
#include "qsomodel.h"

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
    void MessageReceived(QString message);
    void FreqChange(int freq);
private:
    Ui::MainWindow *ui;
    Udp udp;
    QJsonArray array;
    void ReadDxccJson2();
    QString FindCountry(QString& call);
    int FindEntity(QString& call);
    QsoModel model;
    QJsonObject object;
    QString band;
 };
#endif // MAINWINDOW_H
