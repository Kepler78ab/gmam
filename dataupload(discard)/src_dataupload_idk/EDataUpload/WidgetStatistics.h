#ifndef WIDGETSTATISTICS_H
#define WIDGETSTATISTICS_H

#include <QWidget>
#include <QTimer>
#include <QDirIterator>
#include <QRegularExpression>
#include <QStringList>
#include <QPair>
namespace Ui {
class WidgetStatistics;
}

class WidgetStatistics : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetStatistics(const QString &dir,QWidget *parent = nullptr);
   virtual ~WidgetStatistics();
private:
    QString getPrefixByStringCompare(const QString& dirPath, const QString& startDay, const QString& endDay, const QString& suffix);
    void refreshScreen();
public slots:

private:
    Ui::WidgetStatistics *ui;
    QString m_dir;
    QTimer *m_timer;

};

#endif // WIDGETSTATISTICS_H
