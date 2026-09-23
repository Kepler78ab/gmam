#include "WidgetStatistics.h"
#include "ui_WidgetStatistics.h"
#include <QTimer>
#include<QDir>
#include<QTextStream>
#include<QDebug>
#include<QDateTime>
WidgetStatistics::WidgetStatistics(const QString &dir,QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::WidgetStatistics)
    ,m_dir(dir)
{
    ui->setupUi(this);
    // this->setAttribute(Qt::WA_DeleteOnClose);
    this->setWindowFlags(Qt::CustomizeWindowHint|Qt::WindowTitleHint);
    this->setWindowTitle("统计界面");
    ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());
    ui->dateTimeEdit_2->setDateTime(QDateTime::currentDateTime());
    m_timer=new QTimer();
    connect(ui->pushButton,&QPushButton::clicked,[this](){
        refreshScreen();
    });
    connect(ui->pushButton_2,&QPushButton::clicked,[this](){
        refreshScreen();
    });
    // connect(m_timer,&QTimer::timeout,[this](){
    //     refreshScreen();
    // });
    // m_timer->start(10000);
    refreshScreen();
}

WidgetStatistics::~WidgetStatistics()
{
    qDebug()<<"widget close";
    disconnect();
    if(m_timer!=nullptr)
        delete m_timer;
    delete ui;
     qDebug()<<"widget destroy";
}
void WidgetStatistics::refreshScreen()
{
    ui->plainTextEdit->clear();
    ui->plainTextEdit_2->clear();
    QString myPattern = R"(^([^_]+_[^_]+)_(\d{14})_p(_r)?\.xls$)";

    // 调用函数
    int start=ui->dateTimeEdit->date().toString("yyyyMMdd").toInt();
    int end=ui->dateTimeEdit_2->date().toString("yyyyMMdd").toInt();
    // qDebug()<<start<<end;
    if(end<start)
    {
        return;
    }
    // qDebug()<<"1234";
    // qDebug()<<        getPrefixByStringCompare(m_dir,
    //                                      ui->dateTimeEdit->date().toString("yyyyMMdd"),
    //                                      ui->dateTimeEdit_2->date().toString("yyyyMMdd"),
    //                                      "_p_r");
    ui->plainTextEdit->setPlainText(
        getPrefixByStringCompare(m_dir,
                                 ui->dateTimeEdit->date().toString("yyyyMMdd"),
                                 ui->dateTimeEdit_2->date().toString("yyyyMMdd"),
                                 "_p"));
    ui->plainTextEdit_2->setPlainText(
        getPrefixByStringCompare(m_dir,
                                 ui->dateTimeEdit->date().toString("yyyyMMdd"),
                                 ui->dateTimeEdit_2->date().toString("yyyyMMdd"),
                                 "_p_r"));

}


QString WidgetStatistics::getPrefixByStringCompare(const QString& dirPath, const QString& startDay, const QString& endDay, const QString& suffix) {
    // 1. 直接构造字符串边界
    QString minStr = startDay + "000000";
    QString maxStr = endDay + "999999";

    QStringList buffer;
    QString fullSuffix = suffix + ".xls";
    int suffixLen = fullSuffix.length();

    // 2. 迭代器初筛
    QDirIterator it(dirPath, QStringList() << "*" + fullSuffix, QDir::Files);

    while (it.hasNext()) {
        it.next();
        QString name = it.fileName();

        // 剥离后缀：得到 xxxx_xxxx_20260313120000
        QString baseName = name.left(name.length() - suffixLen);

        if (baseName.length() >= 14) {
            //截取最后 14 位
            QString fileTimeStr = baseName.right(14);

            // 字典序比较 (String Comparison)
            // 只要位数相同，字符串比大小逻辑上等同于数值比大小
            if (fileTimeStr >= minStr && fileTimeStr <= maxStr) {

                // 6. 截取前缀：去掉末尾的 14 位数字和它前面的那个下划线 (_)
                // 这样剩下的就是“第二个下划线前”的所有内容
                if (baseName.length() > 15) {
                    QString prefix = baseName.left(baseName.length() - 15);
                    // buffer << prefix;
                    buffer << baseName;
                }
            }
        }
    }

    return buffer.join("\n");
}
