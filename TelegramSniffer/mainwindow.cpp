/***************************************************************************
**                                                                        **
**  QCodesysNV, a QT C++ library for communication                        **
**  with CODESYS network variable UDP protocol.                           **
**                                                                        **
**  Copyright (C) 2015-2016 Jani Komppula                                 **
**                                                                        **
**  This program is free software: you can redistribute it and/or modify  **
**  it under the terms of the GNU General Public License as published by  **
**  the Free Software Foundation, either version 3 of the License, or     **
**  (at your option) any later version.                                   **
**                                                                        **
**  This program is distributed in the hope that it will be useful,       **
**  but WITHOUT ANY WARRANTY; without even the implied warranty of        **
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         **
**  GNU General Public License for more details.                          **
**                                                                        **
**  You should have received a copy of the GNU General Public License     **
**  along with this program.  If not, see http://www.gnu.org/licenses/.   **
**                                                                        **
****************************************************************************
**           Author: Jani Komppula                                        **
**                   jani.komppula@gmail.com                              **
**             Date: 11.02.2016                                           **
**          Version: 0.6                                                  **
****************************************************************************/

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
using namespace std;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //makes pointers to NULL
    //remember this!
    socket=NULL;
    logFile = NULL;
    logStream = NULL;
    trafficLogFile = NULL;
    trafficLogStream = NULL;

    //initializes telegrams
    receivableTelegram=new QCodesysNVTelegram(0);
    broadcastableTelegram = new QCodesysNVTelegram(1);
    //connects update signal of receivableTelegram to dataAvailable() slot of this
    connect(receivableTelegram,SIGNAL(updated()),this,SLOT(dataAvailable()));


    //initializes counters
    CODESYScounter=0;
    UDPcounter=0;
    TELEGRAMcounter=0;

    //initializes timers for counters, datalogger and heart beat
    counterTimer = new QTimer(this);
    connect(counterTimer, SIGNAL(timeout()), this, SLOT(updateCounters()));
    counterTimer->start(rateTimeValues[ui->comboBox_rateTime->currentIndex()]*1000);

    dataloggerTimer = new QTimer(this);
    connect(dataloggerTimer, SIGNAL(timeout()), this, SLOT(updateDatalogger()));
    dataloggerTimer->start(dataloggerTimeValues[ui->comboBox_dataloggerTimestep->currentIndex()]*1000);

    heartBeatTimer = new QTimer(this);
    connect(heartBeatTimer, SIGNAL(timeout()), this, SLOT(updateHearBeat()));
    heartBeatTimer->start(ui->spinBox_hearBeatInterval->value());
    heartBeatValue=0;

    //set values for comboxes of time intervals of data logger and counters
    rateTimeValues.push_back(1);
    rateTimeValues.push_back(5);
    rateTimeValues.push_back(10);
    rateTimeValues.push_back(60);
    rateTimeValues.push_back(600);

    dataloggerTimeValues.push_back(1);
    dataloggerTimeValues.push_back(5);
    dataloggerTimeValues.push_back(10);
    dataloggerTimeValues.push_back(60);
    dataloggerTimeValues.push_back(600);

    //sets max size of sniffer text box
    ui->plainTextEdit_messageLog->setMaximumBlockCount(500);

}

MainWindow::~MainWindow()
{
    delete popupWindow;
    delete ui;
}

//writes data to logfile if file is open and check box is checked
//function call in MainWindow::dataAvailable()
//see connection in MainWindow::MainWindow
void MainWindow::updateDatalogger()
{
    if(ui->checkBox_LogToFile->isChecked() && logFile != NULL)
    {
        if(logFile->isOpen())
        {
            const QDateTime now = QDateTime::currentDateTime();
            QString line;
            QStringList dataList;
            receivableTelegram->dataToStringList(dataList);

            *logStream << QString::number((now.toMSecsSinceEpoch()-startingTime.toMSecsSinceEpoch())/1000.0,'f',3);

            for(int i=0;i<dataList.length();i++)
            {
                *logStream << " " << dataList[i];
            }

            *logStream << "\n";
        }
    }
}

//this function (which is actually SLOT) is executed when heartBeatTimer is timeOut()
//see connection in MainWindow::MainWindow
void MainWindow::updateHearBeat()
{
    if(ui->checkBox_hearBeat->isChecked())
    {
        if (socket == NULL || broadcastableTelegram->variableCount()==0)
        {
            on_pushButton_broadcast_clicked();
        }
        else if (broadcastableTelegram->variableCount() > ui->spinBox_hearBeatVariable->value())
        {
            heartBeatValue =! heartBeatValue;

            broadcastableTelegram->setData(ui->spinBox_hearBeatVariable->value(),heartBeatValue);

            socket->broadcastTelegram(broadcastableTelegram);

            if (heartBeatValue==1)
            {
                ui->widget_heartBeatMarker->setStyleSheet("background-color:black;");
            }
            else
            {
                ui->widget_heartBeatMarker->setStyleSheet("background-color:white;");
            }
        }
        else
        {
            qDebug() << "HearBeat fail!";
        }
    }

}

//this function (which is actually SLOT) is executed when counterTimer is timeOut()
//see connection in MainWindow::MainWindow
void MainWindow::updateCounters(){

    tempCounter++;

    //updates labels in user interface
    ui->label_UDPrate->setText(QString::number(((double) UDPcounter) / ((double) rateTimeValues[ui->comboBox_rateTime->currentIndex()])));
    ui->label_codesysrate->setText(QString::number((double) CODESYScounter / (double) rateTimeValues[ui->comboBox_rateTime->currentIndex()]));
    ui->label_telegramrate->setText(QString::number((double) TELEGRAMcounter / (double) rateTimeValues[ui->comboBox_rateTime->currentIndex()]));

    CODESYScounter=0;
    UDPcounter=0;
    TELEGRAMcounter=0;
}

//this function (which is actually SLOT) is executed when codesys socket emit CODESYSreceived() signal
//see connection in MainWindow::on_pushButton_listen_clicked()
void MainWindow::CODESYSreceived(QString IP, unsigned int port, unsigned int bytes, unsigned int cobid)
{

    CODESYScounter++;

    //writes text to messageLog if it is selected from comboBox_UDPtrafficLogCondition
    if( ui->comboBox_UDPtrafficLogCondition->currentIndex()==1)
    {
        const QDateTime now = QDateTime::currentDateTime();
        QString line;
        line.append(now.toString(QLatin1String("hh:mm:ss.zzz")));

        line.append(" CobID: ");
        line.append(QString::number(cobid));
        line.append(", ");
        line.append(IP);
        line.append(":");
        line.append(QString::number(port));
        line.append(" ");
        line.append(QString::number(bytes));
        line.append(" bytes\n");

        ui->plainTextEdit_messageLog->insertPlainText(line);

        if(trafficLogFile != NULL && ui->checkBox_logUDPtraffic->isChecked())
        {
            if(trafficLogFile->isOpen())
            {
                *trafficLogStream << line;
            }
        }
    }

}

//this function (which is actually SLOT) is executed when codesys socket emit UDPreceived() signal
//see connection in MainWindow::on_pushButton_listen_clicked()
void MainWindow::UDPreceived(QString IP, unsigned int port, unsigned int bytes)
{
    UDPcounter++;

    //writes text to messageLog if it is selected from comboBox_UDPtrafficLogCondition
    if(ui->comboBox_UDPtrafficLogCondition->currentIndex()==0)
    {
        const QDateTime now = QDateTime::currentDateTime();
        QString line;
        line.append(now.toString(QLatin1String("hh:mm:ss.zzz")));
        line.append(" ");
        line.append(IP);
        line.append(":");
        line.append(QString::number(port));
        line.append(" ");
        line.append(QString::number(bytes));
        line.append(" bytes\n");

        ui->plainTextEdit_messageLog->insertPlainText(line);

        if(trafficLogFile != NULL && ui->checkBox_logUDPtraffic->isChecked())
        {
            if(trafficLogFile->isOpen())
            {
                *trafficLogStream << line;
            }
        }
    }
}

//this function (which is actually SLOT) is executed when receivableTelegram emits signal updated
//see connection in MainWindow::MainWindow
void MainWindow::dataAvailable()
{
    //writes values to logBox
    TELEGRAMcounter++;
    ui->logBox->clear();
    const QDateTime now = QDateTime::currentDateTime();
    ui->logBox->insertPlainText(now.toString(QLatin1String("yyyy_MM_dd-hh:mm:ss.zzz")));
    ui->logBox->insertPlainText("\n");

    ui->logBox->insertPlainText("Telegram from: ");
    ui->logBox->insertPlainText(receivableTelegram->readIP().toString());
    ui->logBox->insertPlainText(":");
    ui->logBox->insertPlainText(QString::number(receivableTelegram->readPort()));
    ui->logBox->insertPlainText("\n");

    QStringList DataStrings;
    receivableTelegram->dataToStringList(DataStrings);
    for(int i=0;i<DataStrings.length();i++)
    {
         QString line;
         line.append(variableList[i]);
         line=line.leftJustified(13,' ');
         line.append(DataStrings[i]);
         line.append("\n");
         ui->logBox->insertPlainText(line);
    }

    //writes values to logFile if all data available data want to be logged
    //if(ui->checkBox_LogToFile->isChecked() && ui->comboBox_dataloggerTimestep->currentIndex()==0)
    //{
    //    updateDatalogger();
    //}
}

// opens log file if checkBox_LogToFile is Checked
void MainWindow::on_checkBox_LogToFile_clicked()
{

    if(ui->checkBox_LogToFile->isChecked())
    {
        if(logFile!=NULL)
        {
            if(logFile->isOpen())
            {
                logFile->close();
            }
            delete logFile;
            logFile=NULL;
            delete logStream;
            logStream=NULL;
        }

        QString filename;
        const QDateTime now = QDateTime::currentDateTime();
        filename.append(ui->file_prefix->text());
        filename.append(now.toString(QLatin1String("_yyyyMMdd_hhmmss")));
        filename.append(".txt");

        logFile = new QFile(filename);
        if(logFile->open(QIODevice::WriteOnly))
        {
            logStream= new QTextStream(logFile);
            startingTime = QDateTime::currentDateTime();
        }
        else
        {
            delete logFile;
            logFile=NULL;
        }


    }
    else
    {
        if(logFile->isOpen())
        {
            logFile->close();
        }
        delete logFile;
        logFile=NULL;
        delete logStream;
        logStream=NULL;
    }
}

//sets time interval of counters
void MainWindow::on_comboBox_rateTime_currentIndexChanged(int index)
{
    counterTimer->setInterval(rateTimeValues[ui->comboBox_rateTime->currentIndex()]*1000);
}

//sets datalogger timeintervals
void MainWindow::on_comboBox_dataloggerTimestep_currentIndexChanged(int index)
{
    if (ui->comboBox_dataloggerTimestep->currentIndex()==0)
    {
        dataloggerTimer->stop();
    }
    else
    {
        if(dataloggerTimer->isActive()!= 1)
        {
            dataloggerTimer->start(dataloggerTimeValues[ui->comboBox_dataloggerTimestep->currentIndex()]*1000);
        }
        else
        {
            dataloggerTimer->setInterval(dataloggerTimeValues[ui->comboBox_dataloggerTimestep->currentIndex()]*1000);
        }
    }
}

//Open log file for UDP traffic if checBox is Checked
void MainWindow::on_checkBox_logUDPtraffic_clicked()
{
    if(ui->checkBox_logUDPtraffic->isChecked())
    {
        if(trafficLogFile!=NULL)
        {
            if(trafficLogFile->isOpen())
            {
                trafficLogFile->close();
            }
            delete trafficLogFile;
            trafficLogFile=NULL;
            delete trafficLogStream;
            trafficLogStream=NULL;
        }

        QString filename;
        const QDateTime now = QDateTime::currentDateTime();
        filename.append(ui->file_prefix->text());
        filename.append(now.toString(QLatin1String("_yyyyMMdd_hhmmss")));
        filename.append(".txt");

        trafficLogFile = new QFile(filename);
        if(trafficLogFile->open(QIODevice::WriteOnly))
        {
            trafficLogStream= new QTextStream(trafficLogFile);
            trafficLogStartingTime = QDateTime::currentDateTime();
        }
        else
        {
            delete trafficLogFile;
            trafficLogFile=NULL;
        }


    }
    else
    {
        if(trafficLogFile->isOpen())
        {
            trafficLogFile->close();
        }
        delete trafficLogFile;
        trafficLogFile=NULL;
        delete trafficLogStream;
        trafficLogStream=NULL;
    }
}

//Broadcast telegram
void MainWindow::on_pushButton_broadcast_clicked()
{
    if(socket == NULL)
    {

        setupSocket();
    }

    broadcastableTelegram->setVariableTypes(ui->lineEdit_broadcast_variables->text());
    broadcastableTelegram->setCobId(ui->spinBox_broadcast_cobid->value());
    broadcastableTelegram->setPort(ui->spinBox_broadcast_port->value());
    if(ui->radioButton_broadcast_IP_any->isChecked())
    {
        broadcastableTelegram->setIP(QHostAddress::Broadcast);
    }
    else
    {
        broadcastableTelegram->setIP(QHostAddress(ui->lineEdit_broadcast_IP->text()));
    }
    broadcastableTelegram->stringToData(ui->lineEdit_broadcast_values->text());

    for (int i=(broadcastableTelegram->variableCount()-1); i>-1;i--)
    {
        qint64 temp;
        broadcastableTelegram->readData(i,temp);
        qDebug() << temp;
    }

    socket->broadcastTelegram(broadcastableTelegram);

}


//Start to listen receivableTelegram
void MainWindow::on_pushButton_listen_clicked()
{
    QString variableText = ui->variables->text();
    variableList.clear();
    variableList=variableText.split(",");

    receivableTelegram->setCobId(ui->CobIdBox->value());
    if(receivableTelegram->setVariableTypes(variableText))
    {
        popupWindow = new PopUp(); // Be sure to destroy you window somewhere
        popupWindow->ClearAll();
        popupWindow->SetLabel1("Typo in the variable list!");
        popupWindow->show();
    }
    else
    {
        setupSocket();
        socket->addTelegram(receivableTelegram);
    }

}

//Opens codesys socket
void MainWindow::setupSocket()
{
    QHostAddress address;

    if(ui->specialIP->isChecked())
    {
        address=QHostAddress(ui->IP->text());
    }
    else
    {
        address=QHostAddress::Any;
    }

    uint16_t port = ui->portBox->value();

    if(socket != NULL){
        delete socket;
    }
    socket = new QCodesysNVSocket(address,port);
    connect(socket,SIGNAL(udpPackageReceived(QString,uint,uint)),this,SLOT(UDPreceived(QString, unsigned int, unsigned int)));
    connect(socket,SIGNAL(codesysTelegramReceived(QString,uint,uint,uint)),this,SLOT(CODESYSreceived(QString, unsigned int, unsigned int, unsigned int)));
}


void MainWindow::on_checkBox_hearBeat_clicked()
{
    if(socket == NULL && ui->checkBox_hearBeat->isChecked())
    {
        setupSocket();
    }
}

void MainWindow::on_spinBox_hearBeatInterval_valueChanged(int arg1)
{
    heartBeatTimer->setInterval(arg1);
}
