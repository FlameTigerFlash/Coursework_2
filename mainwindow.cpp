#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    inner_port.setPortName("COM1");
    inner_port.setBaudRate(QSerialPort::Baud9600);
    inner_port.setDataBits(QSerialPort::Data8);
    inner_port.setStopBits(QSerialPort::OneStop);
    inner_port.open(QSerialPort::ReadWrite);
    connect(&inner_port, SIGNAL(readyRead()), this, SLOT(on_innerMessage()));
}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::load_stats(const QDomNode& node)
{
    QDomElement min_threshold_element = node.firstChildElement("LowerThreshold");
    QDomElement max_threshold_element = node.firstChildElement("UpperThreshold");
    lower = min_threshold_element.firstChild().nodeValue().toFloat();
    upper = max_threshold_element.firstChild().nodeValue().toFloat();
}

void MainWindow::save_stats(const QDomNode& node)
{
    QDomElement min_threshold_element = node.firstChildElement("LowerThreshold");
    QDomElement max_threshold_element = node.firstChildElement("UpperThreshold");
    min_threshold_element.firstChild().setNodeValue(QString::number(lower));
    max_threshold_element.firstChild().setNodeValue(QString::number(upper));
}

void MainWindow::on_innerMessage()
{
    char buffer[100];
    int numRead = 0;
    if (inner_port.bytesAvailable() > 0)
    {
        numRead = inner_port.read(buffer, 100);
        //outer_port.flush();
    }
    QString inp = buffer;
    bool ok;
    float humidity = inp.toFloat(&ok);
    if (!ok)
    {
        return;
    }
    QFile file(XMLPath);
    QDomDocument domDoc;
    QDomElement domElement;
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << "Failed to open file:" << file.errorString();
            return;
        }
    if (domDoc.setContent(&file))
    {
        domElement = domDoc.documentElement();
        load_stats(domElement);
    }
    file.close();
    ui->lineEdit->setText(inp);
    if (humidity > upper)
    {
        ui->lineEdit->setStyleSheet("color: red");
    }
    else if (humidity < lower)
    {
        ui->lineEdit->setStyleSheet("color: blue");
    }
    else
    {
        ui->lineEdit->setStyleSheet("color: green");
    }
    return;
}

void MainWindow::on_pushButton_clicked()
{
    QString lower_val = ui->lineEdit_2->text();
    QString upper_val = ui->lineEdit_3->text();
    bool ok;
    lower = lower_val.toFloat(&ok);
    if (!ok)
    {
        ui->label_4->setText("Некорректное значение нижней границы.");
        return;
    }
    upper = upper_val.toFloat(&ok);
    if (!ok)
    {
        ui->label_4->setText("Некорректное значение верхней границы.");
        return;
    }
    if (lower > upper)
    {
        ui->label_4->setText("Указан некорректный диапазон.");
        return;
    }
    QFile file(XMLPath);
    QDomDocument domDoc;
    QDomElement domElement;
    if (file.open(QIODevice::ReadWrite))
    {
        if (domDoc.setContent(&file))
        {
            domElement = domDoc.documentElement();
            save_stats(domElement);
        }
    }
    file.close();
    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
            qDebug() << "Failed to save file";
            return;
        }

        QTextStream stream(&file);
        stream.setCodec("UTF-8");
        domDoc.save(stream, 4);  // 4 - отступ для форматирования
        file.close();

    ui->label_4->setText("Диапазон успешно обновлен.");

    float humidity = ui->lineEdit->text().toFloat();
    if (humidity > upper)
    {
        ui->lineEdit->setStyleSheet("color: red");
    }
    else if (humidity < lower)
    {
        ui->lineEdit->setStyleSheet("color: blue");
    }
    else
    {
        ui->lineEdit->setStyleSheet("color: green");
    }
}
