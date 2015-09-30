#include "activitylogger.h"

activityLogger::activityLogger(QWidget *parent) : QMainWindow(parent), ui(new Ui::activityLogger){
    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(onUpdate()));
    timer->start(10);
    this->setWindowIcon(QIcon(":/icon/icon.ico"));
    ui->setupUi(this);
    pause_offset=0;
    epoch=-1;
    loadActivities();
    loadEntries();
    foreach(QListWidgetItem item,activities){
        ui->activityList->addItem(new QListWidgetItem(item.text()));
    }
    ui->activityList_2->setModel(ui->activityList->model());
    ui->filterDate->setDate(QDate::currentDate());
    setEnableFilters(false);
}

void activityLogger::setEnableFilters(bool doEnable){
    ui->filterMonths->setEnabled(doEnable);
    ui->filterYears->setEnabled(doEnable);
    ui->filterDate->setEnabled(doEnable);
    ui->totalTime->setText("");
}

activityLogger::~activityLogger(){
    delete ui;
}

QString activityLogger::getTimeFromEpoch(qint64 epoch){
    //Note: due to integer division, dividing and then multipling by the same number doesn't
    //neccessarly mean the result will be the original value.
    epoch=(QDateTime::currentMSecsSinceEpoch()/1000.0-pause_offset)-epoch;
    int hours=epoch/3600;
    epoch-=hours*3600;
    int minutes=epoch/60;
    epoch-=minutes*60;
    int seconds=epoch;
    return QString::asprintf("Hours: %d,Minutes: %d,Seconds: %d",hours,minutes,seconds);
}

void activityLogger::onUpdate(){
    if(ui->filterMonths->isEnabled()){
        //Filters are enabled
        ui->filterDate->setEnabled(ui->filterMonths->checkState() || ui->filterYears->checkState());
    }
    if(ui->stackedWidget->currentIndex()==1 && !ui->pause->isChecked()){
        QString TimeString=getTimeFromEpoch(epoch);
        QStringList Time=TimeString.split(",");
        ui->hoursString->setText(Time[0]);
        ui->minutesString->setText(Time[1]);
        ui->secondsString->setText(Time[2]);
    }

}

void activityLogger::updateActivities(){
    activities.clear();
    int len=ui->activityList->count();
    for(int i=0;i<len;i++){
        QListWidgetItem *item=ui->activityList->item(i);
        activities.append(*item);
    }
}

void activityLogger::saveActivities(){
    QFile save("activities.txt");
    if(!save.open(QIODevice::WriteOnly|QIODevice::Text))
        return;
    QString write;
    foreach(QListWidgetItem item,activities){
        write.append(item.text()+"[NEWENT]");
    }
    save.write(write.toStdString().data());
    save.close();
}

void activityLogger::loadActivities(){
    activities.clear();
    QFile load("activities.txt");
    if(!load.open(QIODevice::ReadOnly|QIODevice::Text))
        return;
    while(!load.atEnd()){
        QString line=QString::fromStdString(load.readLine().toStdString());
        QStringList foo=line.split("[NEWENT]");
        foreach(QString name,foo){
            if(name.length()>0)
                activities.append(QListWidgetItem(name));
        }
    }
    load.close();
}

void activityLogger::saveEntries(){
    if(entries.length()>0){
        QFile save("entries.txt");
        if(!save.open(QIODevice::WriteOnly|QIODevice::Text))
            return;
        QString write;
        bool doWrite=false;
        foreach(QStringList lst,entries){
            if(lst.length()<1)
                continue;
            write.append("[NEWLST]"+lst[0]+"[NEWENT]");
            lst.pop_front();
            foreach(QString item,lst){
                if(item.length()<1)
                    continue;
                write.append(item+"[NEWENT]");
                doWrite=true;
            }
        }
        write=substring(write,7,write.length());
        if(doWrite)
            save.write(write.toStdString().data());
        save.close();
    }
}

void activityLogger::loadEntries(){
    entries.clear();
    QFile load("entries.txt");
    if(!load.open(QIODevice::ReadOnly|QIODevice::Text))
        return;
    QString line = QString::fromStdString(load.readAll().toStdString());
    QStringList arrayEntries=line.split("[NEWLST]");
    for(int i=0;i<arrayEntries.length();i++){
        QString item=arrayEntries[i];
        QStringList lst=item.split("[NEWENT]");
        for(int i=0;i<lst.length();i++){
            if(lst.at(i).length()<1)
                lst.removeAt(i);
        }
        if(lst.length()>0)
            entries.append(lst);
    }
    load.close();
}

void activityLogger::filterData(){
    ui->displayData->clear();
    QString month=ui->filterDate->date().longMonthName(ui->filterDate->date().month());
    QString year=QString::asprintf("%d",ui->filterDate->date().year());
    QString itemName=ui->activityList_2->currentIndex().model()->data(ui->activityList_2->currentIndex(), Qt::DisplayRole).toString();
    QStringList lst;
    for(int i=0; i<entries.length();i++){
        lst=entries[i];
        if(lst.length()>0 && lst[0]==itemName){
            lst.removeAt(0);
            break;
        }
    }
    qint64 total=0;
    foreach(QString entry,lst){
        if((entry.indexOf(month)!=-1 || ui->filterMonths->checkState()==false) &&
                (entry.indexOf(year)!=-1 || ui->filterYears->checkState()==false)){
            ui->displayData->addItem(entry);
            total+=getDurationFromEntry(entry);
        }
    }
    int hours=total/3600;
    total-=hours*3600;
    int minutes=total/60;
    total-=minutes*60;
    int seconds=total;
    ui->totalTime->setText(QString::asprintf("Total = Hours: %d Minutes: %d Seconds: %d",hours,minutes,seconds));
}

int activityLogger::getDisplayDataIndex(){
    QModelIndex cIndex=ui->activityList_2->currentIndex();
    QString activityName=cIndex.model()->data(cIndex, Qt::DisplayRole).toString();
    //Get activity index in the entries object
    for(int i=0; i<entries.length();i++){
        QStringList lst=entries[i];
        if(lst.length()>0 && lst[0]==activityName){
            return i;
        }
    }
    return -1;
}

int activityLogger::getItemIndexFromEntry(QString entry, int index){
    for(int i=0;i<entries[index].length();i++){
        if(entries[index][i]==entry)
            return i;
    }
    return -1;
}

int activityLogger::countLeapYears(int year){
    int count=0;
    for(int i=1970;i<year;i++){
        count+=QDate::isLeapYear(i);
    }
    return count;
}

QString activityLogger::getStartedTimeFromEntry(QString entry){
    QString time_date   = substring(entry,entry.indexOf('-')+1,entry.indexOf("Duration")-1);
    qint64 startSeconds = QDateTime::fromMSecsSinceEpoch(0).secsTo(QDateTime::fromString(time_date,"MMMM/dd/yyyy h:mm:ss AP"));
    if(startSeconds>0)
        return QString::asprintf("%lld",startSeconds);
    return "-1";
}

qint64 activityLogger::getDurationFromEntry(QString entry){
    qint64 durationSeconds=0;
    QString substr=substring(entry,entry.indexOf("["),entry.indexOf("]"));
    if(substr.count(",")==2){
        QStringList durationTime=substr.split(',');
        durationSeconds+=durationTime[0].replace("Hours: ","").toInt()*3600;
        durationSeconds+=durationTime[1].replace("Minutes: ","").toInt()*60;
        durationSeconds+=durationTime[2].replace("Seconds: ","").toInt();
        return durationSeconds;
    }
    return 0;
}
