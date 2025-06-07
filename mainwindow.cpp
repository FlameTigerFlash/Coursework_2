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
    init_XML();
}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::init_XML()
{
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
        QMap<QString, void*> tagMap = {
            {"LowerThreshold", &lower},
            {"UpperThreshold", &upper},
            {"HumidityOffset", &humidity_offset},
            {"HumidityMultiplier", &humidity_multiplier}
        };
        readXmlValues(domElement, tagMap);
    }
    file.close();
}

void MainWindow::readXmlValues(const QDomNode& node, const QMap<QString, void*>& tag_var)
{
       QDomNode cur_node = node.firstChildElement();
        while (!cur_node.isNull()) {
            if (cur_node.isElement()) {
                QDomElement element = cur_node.toElement();
                QString tagName = element.tagName();

                if (tag_var.contains(tagName)) {
                    void* variablePtr = tag_var[tagName];
                    QString valueStr = element.text();
                    //qDebug() << valueStr;
                    if (auto floatPtr = static_cast<float*>(variablePtr)) {
                        *floatPtr = valueStr.toFloat();
                    }
                    else if (auto stringPtr = static_cast<QString*>(variablePtr)) {
                        *stringPtr = valueStr;
                    }
            }
        }
        cur_node = cur_node.nextSibling();
    }
}

void writeXmlValues(QDomDocument& doc, QDomNode& parentNode, const QMap<QString, QVariant>& values)
{
    for (auto it = values.constBegin(); it != values.constEnd(); ++it) {
        const QString& tagName = it.key();
        const QVariant& value = it.value();

        QDomElement element;
        QDomNodeList elements = parentNode.toElement().elementsByTagName(tagName);

        if (elements.isEmpty()) {
            element = doc.createElement(tagName);
            parentNode.appendChild(element);
        } else {
            element = elements.at(0).toElement();
        }

        QDomText textNode = doc.createTextNode(value.toString());
        if (element.firstChild().isNull()) {
            element.appendChild(textNode);
        } else {
            element.replaceChild(textNode, element.firstChild());
        }
    }
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
    for (int i = 0; i < numRead; i++)
    {
        if (buffer[i] == '#')
        {
            if (input_buffer.size() > 0)
            {
                update_humidity(input_buffer);
                //qDebug() << input_buffer;
            }
            input_buffer = "";
            continue;
        }
        input_buffer += buffer[i];
    }
}

void MainWindow::update_humidity(QString inp)
{
    bool ok;
    float humidity = (inp.toFloat(&ok) + humidity_offset) * humidity_multiplier;
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
        QMap<QString, void*> tagMap = {
            {"LowerThreshold", &lower},
            {"UpperThreshold", &upper}
        };
        readXmlValues(domElement, tagMap);
        //load_stats(domElement);
    }
    file.close();
    ui->lineEdit->setText(QString::number(humidity));
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
            QMap<QString, QVariant> values = {
                {"LowerThreshold", lower},
                {"UpperThreshold", upper}
            };
            writeXmlValues(domDoc, domElement, values);
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
