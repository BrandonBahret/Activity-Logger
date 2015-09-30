#include "activitylogger.h"

void activityLogger::on_startActivity_clicked(){
    ui->displayData->clear();
    if(ui->activityList->currentItem()){
        comments="";
        QListWidgetItem *item=ui->activityList->currentItem();
        ui->stackedWidget->setCurrentIndex(1);
        ui->currentActivity->setText(item->text());
        epoch=QDateTime::currentMSecsSinceEpoch()/1000.0;
        ui->startTime->setText("Started At: "+QTime::currentTime().toString("h:mm:ss AP"));
        onUpdate();
    }
}

void activityLogger::on_addActivity_clicked(){
    foreach(QListWidgetItem act,activities){
        if(act.text()==ui->newActivityEdit->text()){
            ui->newActivityEdit->setText("");
            return;
        }
    }
    QString activityName=ui->newActivityEdit->text().replace("[NEWLST]","").replace("[NEWENT]","").replace("'","");
    if(ui->newActivityEdit->text().length()>0){
        QListWidgetItem *item=new QListWidgetItem(activityName);
        ui->activityList->addItem(item);
        updateActivities();
        QStringList lst(item->text());
        entries.append(lst);
        saveEntries();
        saveActivities();
    }
    ui->newActivityEdit->setText("");
}


void activityLogger::on_addComments_clicked(){
    ui->stackedWidget->setCurrentIndex(2);
}

void activityLogger::on_comment_cancel_clicked(){
        ui->stackedWidget->setCurrentIndex(epoch!=-1); //If we arn't doing an activity go to page 0 else 1
}

void activityLogger::on_comment_submit_clicked(){
    if(epoch!=-1){
        comments=ui->commentEditor->toPlainText().replace("[NEWLST]","").replace("[NEWENT]","");
        ui->stackedWidget->setCurrentIndex(1);
    }
    else{
        QString entry=ui->displayData->currentItem()->text();
        int index = getDisplayDataIndex();
        if(index!=-1){
            int itemIndex = getItemIndexFromEntry(entry, index);
            QString entryText=entries[index][itemIndex];
            QStringList items=entryText.split("Comments:");
            entries[index][itemIndex]=items[0]+"Comments: "+ui->commentEditor->toPlainText().replace("[NEWLST]","").replace("[NEWENT]","");
            ui->stackedWidget->setCurrentIndex(0);
            on_activityList_2_clicked();
            saveEntries();
        }
    }
}

void activityLogger::on_cancel_clicked(){
    ui->commentEditor->setText("");
    ui->pause->setChecked(false);
    ui->stackedWidget->setCurrentIndex(0);
    pause_offset=0;
    epoch=-1;
}

void activityLogger::on_removeActivity_clicked(){
    if(ui->activityList->currentItem()){
        QDialog dialog;
        QVBoxLayout layout(&dialog);
        QLabel message("\nAre you sure you want to remove '"+ui->activityList->currentItem()->text()+"'\nand all of it's entries.\n");
        layout.addWidget(&message);
        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

        connect(buttonBox, SIGNAL(accepted()), &dialog, SLOT(accept()));
        connect(buttonBox, SIGNAL(rejected()), &dialog, SLOT(reject()));

        layout.addWidget(buttonBox);
        dialog.setLayout(&layout);
        dialog.setWindowIcon(QIcon(":/icon/icon.ico"));
        dialog.setWindowFlags(Qt::WindowCloseButtonHint);
        dialog.setWindowTitle("Remove '"+ui->activityList->currentItem()->text()+"'");
        if(dialog.exec() == QDialog::Accepted){
            ui->displayData->clear();
            QListWidgetItem *item=ui->activityList->currentItem();
            QStringList lst;
            for(int i=0; i<entries.length();i++){
                lst=entries[i];
                if(lst.length()>0 && lst[0]==item->text()){
                    entries.removeAt(i);
                    break;
                }
            }
            delete item;
            updateActivities();
            saveActivities();
            saveEntries();
        }
    }
}

void activityLogger::on_pause_clicked(bool checked){
    timeAtPause= QDateTime::currentMSecsSinceEpoch()/1000.0;
    if(checked!=true){
        pause_offset+=timeAfterPause-timeAtPause;
    }
}

void activityLogger::on_finish_clicked(){
    ui->commentEditor->setText("");
    QString activity=ui->activityList->currentItem()->text();
    int len=entries.length();
    for(int i=0;i<len;i++){
        if(entries[i].length()>0 && entries[i][0]==activity){
            time_t time=epoch;
            QDateTime startingDateTime=QDateTime::fromTime_t(time);
            QString start_time=ui->startTime->text().replace("Started At: ","");
            QString date=startingDateTime.date().toString("MMMM/dd/yyyy");
            QString Total=getTimeFromEpoch(epoch).replace("[NEWENT]"," ");
            QString Final=QString::asprintf("%s - %s %s\nDuration[%s]\nComments: %s",activity.toStdString().data(),
                                            date.toStdString().data(),start_time.toStdString().data(),Total.toStdString().data(),
                                            comments.toStdString().data());
            entries[i].append(Final);
            break;
        }
    }
    pause_offset=0;
    ui->pause->setChecked(false);
    ui->stackedWidget->setCurrentIndex(0);
    saveEntries();
    epoch=-1;
}

void activityLogger::on_activityList_2_clicked(){
    ui->filterMonths->setEnabled(true);
    ui->filterYears->setEnabled(true);
    filterData();
}

void activityLogger::on_editComment_clicked(){
    if(ui->displayData->currentItem()){
        QString entry=ui->displayData->currentItem()->text();
        QStringList items=entry.split("Comments:");
        //Set up the comment editor for the entry.
        QString text=items[1];
        text.remove(0,1);
        ui->commentEditor->setText(text);
        ui->stackedWidget->setCurrentIndex(2);
    }
}

QString activityLogger::substring(QString inputString,int begin,int end){
    QString substr;
    for(int i=begin+1;i<end;i++){
        substr.append(inputString[i]);
    }
    return substr;
}


void activityLogger::on_exportTimestamps_clicked(){
    if(ui->activityList_2->currentIndex().model() && entries[getDisplayDataIndex()].length()>1){
        QString filename=QFileDialog::getSaveFileName(0,"Saving timestamps of: "+entries[getDisplayDataIndex()][0],entries[getDisplayDataIndex()][0],"*.csv");
        if(filename.length()>0){
            if(!filename.endsWith(".csv"))
                filename.append(".csv");
            QFile writeFile(filename);
            if(!writeFile.open(QIODevice::WriteOnly|QIODevice::Text))
                return;

            QStringList lst=entries[getDisplayDataIndex()];
            QString output;
            if(ui->filterMonths->checkState() || ui->filterYears->checkState()){
                output="Entries in "+ui->filterDate->text()+"\n";
            }
            lst.removeAt(0);
            output.append("Started at,Duration\n");

            //For filtering events
            QString month=ui->filterDate->date().longMonthName(ui->filterDate->date().month());
            QString year=QString::asprintf("%d",ui->filterDate->date().year());

            foreach(QString entry,lst){
                if((entry.indexOf(month)!=-1 || ui->filterMonths->checkState()==false) &&
                        (entry.indexOf(year)!=-1 || ui->filterYears->checkState()==false)){
                    //Filter out entries
                    QString substr=substring(entry,entry.indexOf("["),entry.indexOf("]"));
                    if(substr.length()>0){
                        output.append(getStartedTimeFromEntry(entry)+",");
                        output.append(QString::asprintf("%lld\n",getDurationFromEntry(entry)));
                    }
                }
            }
            writeFile.write(output.toStdString().data());
            writeFile.close();
        }
    }
}

void activityLogger::on_exportText_clicked(){
    if(ui->activityList_2->currentIndex().model() && entries[getDisplayDataIndex()].length()>1){
        QString filename=QFileDialog::getSaveFileName(0,"Saving text of: "+entries[getDisplayDataIndex()][0],entries[getDisplayDataIndex()][0],"*.txt");
        if(filename.length()>0){
            if(!filename.endsWith(".txt"))
                filename.append(".txt");
            QFile writeFile(filename);
            if(!writeFile.open(QIODevice::WriteOnly|QIODevice::Text))
                return;
            QString output;

            QString month=ui->filterDate->date().longMonthName(ui->filterDate->date().month());
            QString year=QString::asprintf("%d",ui->filterDate->date().year());
            foreach(QString entry,entries[getDisplayDataIndex()]){
                if((entry.indexOf(month)!=-1 || ui->filterMonths->checkState()==false) &&
                        (entry.indexOf(year)!=-1 || ui->filterYears->checkState()==false)){
                    output.append(entry+"\n\n");
                }
            }
            writeFile.write(output.toStdString().data());
            writeFile.close();
        }
    }
}

void activityLogger::on_deleteEntry_clicked(){
    if(ui->displayData->currentItem()){
        int index=getDisplayDataIndex();
        QString entry=ui->displayData->currentItem()->text();
        int itemIndex=getItemIndexFromEntry(entry,index);
        entries[getDisplayDataIndex()].removeAt(itemIndex);
        saveEntries();
        on_activityList_2_clicked();
    }
}


void activityLogger::on_filterDate_dateChanged(){
    if(ui->filterMonths->checkState() || ui->filterYears->checkState())
    filterData();
}

void activityLogger::on_filterMonths_toggled(){
    filterData();
}

void activityLogger::on_filterYears_toggled(){
    filterData();
}
