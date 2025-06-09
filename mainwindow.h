#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QDebug>
#include <QFile>
#include <QtXml>
#include <QMap>
#include <QDomDocument>
#include <QTranslator>
#include <QButtonGroup>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void changeEvent(QEvent *event) override;

private:
    Ui::MainWindow *ui;
    QSerialPort inner_port;
    QString XMLPath = "C:\\Users\\Andrey270\\Desktop\\Final_Project_2\\data.xml";
    QString TranslationsPath = "C:\\Users\\Andrey270\\Desktop\\Final_Project_2\\lang_";
    float lower = 0;
    float upper = 0;
    float humidity_offset = 0;
    float humidity_multiplier = 0;
    QString input_buffer = "";

    QTranslator appTranslator;
    QTranslator qtTranslator;
    QString currentLang = "en_US";
    QButtonGroup *langButtonGroup;

    void init_XML();
    void load_stats(const QDomNode& node);
    void save_stats(const QDomNode& node);
    void readXmlValues(const QDomNode& node, const QMap<QString, void*>& tag_var);
    void update_humidity(QString inp);
    QDomElement loadXML();
    void saveXML();

    void loadLanguage(const QString &lang);
    void retranslateUi();
    void initLanguageMenu();

private slots:
    void on_innerMessage();
    void on_pushButton_clicked();
    void onLanguageChanged(int id);
};
#endif // MAINWINDOW_H
