#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QDebug>
#include <QFile>
#include <QtXml>
#include <QDomDocument>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QSerialPort inner_port;
    QString XMLPath = "C:/QTProjects/FinalProject/data.xml";
    float lower = 0;
    float upper = 0;
    void load_stats(const QDomNode& node);
    void save_stats(const QDomNode& node);
    QDomElement loadXML();
    void saveXML();
private slots:
    void on_innerMessage();
    void on_pushButton_clicked();
};
#endif // MAINWINDOW_H
