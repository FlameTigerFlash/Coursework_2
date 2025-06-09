#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QDebug>
#include <QFile>
#include <QtXml>
#include <QMap>
#include <QDomDocument>
#include <QTime>
#include <QCoreApplication>

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
    float humidity_offset = 0;
    float humidity_multiplier = 0;
    QString input_buffer = "";
    void initFromXML();
    void load_stats(const QDomNode& node);
    void save_stats(const QDomNode& node);
    void readXmlValues(const QDomNode& node, const QMap<QString, void*>& tag_var);
    void update_humidity(QString inp);
    QDomElement loadXML();
    void saveXML();
    void sendDeferred(QString str, int delay);
private slots:
    void on_innerMessage();
    void on_pushButton_clicked();
};
#endif // MAINWINDOW_H
