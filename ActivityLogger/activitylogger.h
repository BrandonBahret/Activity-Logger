#ifndef ACTIVITYLOGGER_H
#define ACTIVITYLOGGER_H

#include "ui_activitylogger.h"

#include <QMainWindow>
#include <QListWidget>

#include <QDebug>
#include <QFile>
#include <QDateTime>
#include <QTimer>
#include <QIcon>

#include <QListWidget>
#include <QFileDialog>
#include <QDialogButtonBox>

namespace Ui {
class activityLogger;
}

class activityLogger : public QMainWindow
{
    Q_OBJECT

public:
    qint64 epoch, timeAtPause, timeAfterPause, pause_offset;
    QList<QListWidgetItem> activities;
    QList<QStringList> entries;
    QString comments;

    explicit activityLogger(QWidget *parent = 0);
    ~activityLogger();

    void saveActivities();
    void loadActivities();
    void saveEntries();
    void loadEntries();

    void updateActivities(void);
    QString getTimeFromEpoch(qint64 epoch);
    QString getStartedTimeFromEntry(QString entry);
    qint64 getDurationFromEntry(QString entry);
    int countLeapYears(int year);

    int getDisplayDataIndex();
    int getItemIndexFromEntry(QString entry, int index);

    QString substring(QString inputString, int begin, int end);

    void setEnableFilters(bool doEnable);
    void filterData();

private slots:
    void onUpdate(void);
    void on_startActivity_clicked();
    void on_addActivity_clicked();
    void on_cancel_clicked();
    void on_removeActivity_clicked();
    void on_pause_clicked(bool checked);
    void on_finish_clicked();
    void on_activityList_2_clicked();
    void on_comment_cancel_clicked();
    void on_comment_submit_clicked();
    void on_editComment_clicked();
    void on_exportTimestamps_clicked();
    void on_exportText_clicked();
    void on_deleteEntry_clicked();
    void on_filterDate_dateChanged();
    void on_filterMonths_toggled();
    void on_filterYears_toggled();

private:
    Ui::activityLogger *ui;
};

#endif // ACTIVITYLOGGER_H
