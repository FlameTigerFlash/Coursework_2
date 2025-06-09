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
    initFromXML();
    initLanguageMenu();

    QSettings settings;
    currentLang = settings.value("Language", "en_US").toString();
    loadLanguage(currentLang);
}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initLanguageMenu()
{
    langButtonGroup = new QButtonGroup(this);

    langButtonGroup->addButton(ui->radioButton, 0);
    langButtonGroup->addButton(ui->radioButton_2, 1);
    langButtonGroup->addButton(ui->radioButton_3, 2);

    if(currentLang == "ru_RU") {
        ui->radioButton->setChecked(true);
    } else if(currentLang == "en_US") {
        ui->radioButton_2->setChecked(true);
    } else if(currentLang == "de_DE") {
        ui->radioButton_3->setChecked(true);
    }

    connect(langButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(onLanguageChanged(int)));
}

void MainWindow::onLanguageChanged(int id)
{
    QString lang;
    switch(id) {
        case 0: lang = "ru_RU"; break;
        case 1: lang = "en_US"; break;
        case 2: lang = "de_DE"; break;
        default: lang = "en_US";
    }

    if(currentLang != lang) {
        loadLanguage(lang);
    }
}

void MainWindow::loadLanguage(const QString &lang)
{
    QApplication::removeTranslator(&appTranslator);
    QApplication::removeTranslator(&qtTranslator);

    if(appTranslator.load(TranslationsPath + lang + ".qm")) {
        QApplication::installTranslator(&appTranslator);
    }

    if(qtTranslator.load("qt_" + lang, QLibraryInfo::location(QLibraryInfo::TranslationsPath))) {
        QApplication::installTranslator(&qtTranslator);
    }

    currentLang = lang;

    QSettings settings;
    settings.setValue("Language", lang);

    retranslateUi();
}

void MainWindow::retranslateUi()
{
    ui->retranslateUi(this);
    setWindowTitle(tr("Humidity Monitor"));

    ui->radioButton->setText(tr("Русский"));
    ui->radioButton_2->setText(tr("English"));
    ui->radioButton_3->setText(tr("Deutsch"));

    QString currentMessage = ui->label_4->text();
    if(!currentMessage.isEmpty()) {
        if(currentMessage == "Некорректное значение нижней границы.") {
            ui->label_4->setText(tr("Некорректное значение нижней границы."));
        }
        else if(currentMessage == "Некорректное значение верхней границы.") {
            ui->label_4->setText(tr("Некорректное значение верхней границы."));
        }
        else if(currentMessage == "Указан некорректный диапазон.") {
            ui->label_4->setText(tr("Указан некорректный диапазон."));
        }
        else if(currentMessage == "Диапазон успешно обновлен.") {
            ui->label_4->setText(tr("Диапазон успешно обновлен."));
        }
    }
}

void MainWindow::changeEvent(QEvent *event)
{
    if(event->type() == QEvent::LanguageChange) {
        retranslateUi();
    }
    QMainWindow::changeEvent(event);
}

void MainWindow::initFromXML()
{
    QFile file(XMLPath);
    QDomDocument domDoc;
    QDomElement domElement;
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << tr("Failed to open file:") << file.errorString();
            return;
        }
    float window_width = 500, window_height = 500;
    if (domDoc.setContent(&file))
    {
        domElement = domDoc.documentElement();
        QMap<QString, void*> tagMap = {
            {"LowerThreshold", &lower},
            {"UpperThreshold", &upper},
            {"HumidityOffset", &humidity_offset},
            {"HumidityMultiplier", &humidity_multiplier},
            {"WindowWidth", &window_width},
            {"WindowHeight", &window_height}
        };
        readXmlValues(domElement, tagMap);
        ui->label_5->setText(QString::number(lower));
        ui->label_6->setText(QString::number(upper));
        resize((int)window_width, (int)window_height);
    }
    file.close();
    ui->label_5->setStyleSheet("color:grey");
    ui->label_6->setStyleSheet("color:grey");
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
    }
    //qDebug() << buffer;
    for (int i = 0; i < numRead; i++)
    {
        if (buffer[i] == '#')
        {
            if (input_buffer.size() > 0)
            {
                update_humidity(input_buffer);
            }
            input_buffer = "";
            continue;
        }
        input_buffer += buffer[i];
    }
}

void MainWindow::sendDeferred(QString str, int delay)
{
    for (int i = 0; i < str.size(); i++)
    {
        const char* out = ((QString)str[i]).toStdString().c_str();
        inner_port.write(out);
        inner_port.flush();
        QTime delayTime = QTime::currentTime().addMSecs(delay);
        while (QTime::currentTime() < delayTime) {
                QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
        }
    }
}

void MainWindow::update_humidity(QString inp)
{
    bool ok;
    float humidity = (inp.toFloat(&ok) + humidity_offset) * humidity_multiplier;
    if (humidity > 100 || humidity < 0)
    {
        return;
    }
    humidity = roundf(humidity);
    if (!ok)
    {
        return;
    }
    QFile file(XMLPath);
    QDomDocument domDoc;
    QDomElement domElement;
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qDebug() << tr("Failed to open file:") << file.errorString();
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
    }
    file.close();
    ui->lineEdit->setText(QString::number(humidity));


    QString output = QString::number((int)humidity/10);
    if (humidity > upper)
    {
        ui->lineEdit->setStyleSheet("color: red");
        output += "r";
    }
    else if (humidity < lower)
    {
        ui->lineEdit->setStyleSheet("color: blue");
        output += "b";
    }
    else
    {
        ui->lineEdit->setStyleSheet("color: green");
        output += "g";
    }
    sendDeferred(output, 210);
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
        ui->label_4->setText(tr("Некорректное значение нижней границы."));
        return;
    }
    upper = upper_val.toFloat(&ok);
    if (!ok)
    {
        ui->label_4->setText(tr("Некорректное значение верхней границы."));
        return;
    }
    if (lower > upper || lower < 0 || upper > 100)
    {
        ui->label_4->setText(tr("Указан некорректный диапазон."));
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
            qDebug() << tr("Failed to save file");
            return;
        }

        QTextStream stream(&file);
        stream.setCodec("UTF-8");
        domDoc.save(stream, 4);
        file.close();

    ui->label_4->setText(tr("Диапазон успешно обновлен."));
    ui->label_5->setText(QString::number(lower));
    ui->label_6->setText(QString::number(upper));
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
