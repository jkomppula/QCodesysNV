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
#include <QList>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //Initializes telegram which is not broadcastable (parameter 0). A broadcastable telegram requires parameter 1.
    telegramListen = new QCodesysNVTelegram(0);
    //CobID of the telegram
    telegramListen->setCobId(477);

    //Determines variable types of the telegram
    QList <QCodesysNVType> variablesListen;
    variablesListen << QCodesysNVType::REAL;
    variablesListen << QCodesysNVType::REAL;
    variablesListen << QCodesysNVType::REAL;
    variablesListen << QCodesysNVType::REAL;
    variablesListen << QCodesysNVType::REAL;
    variablesListen << QCodesysNVType::REAL;
    variablesListen << QCodesysNVType::REAL;
    variablesListen << QCodesysNVType::REAL;
    variablesListen << QCodesysNVType::BYTE;
    telegramListen->setVariableTypes(variablesListen);

    //index of QList variables which will be plotted
    plotableVariable=4;




    //Initializes socket which listen telegrams from any IP address to port 1202
    socket = new QCodesysNVSocket(QHostAddress::Any,1202);

    //This adds the telegram to the telegram list in the socket. The socket monitors CobIDs of incoming telegrams and checks if the cobID of
    //incoming telegram correspond to any in telegram list. If a telegram is found from the telegram list socket sends byte array to
    //the corresponding telegram.
    socket->addTelegram(telegramListen);

    //Connect the update signal of the telegram to the telegramReceived slot
    connect(telegramListen,SIGNAL(updated()),this,SLOT(telegramReceived()));







    //The broadcastable telegram
    telegramBroadcast= new QCodesysNVTelegram(1);
    //CobID of the telegram
    telegramBroadcast->setCobId(477);

    //Telegram contains two variables: heart beat and counter
    QList <QCodesysNVType> variablesBroadcast;
    variablesBroadcast << QCodesysNVType::BOOL;
    variablesBroadcast << QCodesysNVType::DWORD;
    telegramBroadcast->setVariableTypes(variablesListen);

    //initializes values
    heartbeat=0;
    counter=0;

    telegramBroadcast->setData(0,heartbeat);
    telegramBroadcast->setData(1,counter);

    ui->label_counter->setText(QString::number(counter));


    //Initiliazes timeinterval of the plotter
    on_comboBox_timeinterval_currentIndexChanged(0);


    //This setup plotter
    //See details from http://qcustomplot.com/index.php/demos/realtimedatademo
    ui->plotter->addGraph();
    ui->plotter->graph(0)->setPen(QPen(Qt::blue));
    ui->plotter->graph(0)->setBrush(QBrush(QColor(240, 255, 200)));
    ui->plotter->graph(0)->setAntialiasedFill(false);

    ui->plotter->xAxis->setTickLabelType(QCPAxis::ltDateTime);
    ui->plotter->xAxis->setAutoTickStep(false);
    ui->plotter->xAxis->axisRect()->setupFullAxesBox();
}

MainWindow::~MainWindow()
{
    delete ui;
}


//This changes time interval when the value of comboBox_timeinterval is changed and sets settings of the x-axis
void MainWindow::on_comboBox_timeinterval_currentIndexChanged(int index)
{
    if(ui->comboBox_timeinterval->currentIndex()==0)
    {
        timeInterval=2;
        ui->plotter->xAxis->setDateTimeFormat("hh:mm:ss");
        ui->plotter->xAxis->setTickStep(1);
    }
    else if(ui->comboBox_timeinterval->currentIndex()==1)
    {
        timeInterval=5;
        ui->plotter->xAxis->setDateTimeFormat("hh:mm:ss");
        ui->plotter->xAxis->setTickStep(1);
    }
    else if(ui->comboBox_timeinterval->currentIndex()==2)
    {
        timeInterval=15;
        ui->plotter->xAxis->setDateTimeFormat("hh:mm:ss");
        ui->plotter->xAxis->setTickStep(5);
    }
    else if(ui->comboBox_timeinterval->currentIndex()==3)
    {
        timeInterval=60;
        ui->plotter->xAxis->setDateTimeFormat("hh:mm:ss");
        ui->plotter->xAxis->setTickStep(15);
    }
    else if(ui->comboBox_timeinterval->currentIndex()==4)
    {
        timeInterval=300;
        ui->plotter->xAxis->setDateTimeFormat("hh:mm");
        ui->plotter->xAxis->setTickStep(60);
    }
    else if(ui->comboBox_timeinterval->currentIndex()==5)
    {
        timeInterval=900;
        ui->plotter->xAxis->setDateTimeFormat("hh:mm");
        ui->plotter->xAxis->setTickStep(60);
    }
    else
    {
        timeInterval=3600;
        ui->plotter->xAxis->setDateTimeFormat("hh:mm");
        ui->plotter->xAxis->setTickStep(300);
    }


}

//this add data to plotter when telegram is received
void MainWindow::telegramReceived()
{
    //timestamp of current time
    double timeStamp = QDateTime::currentDateTime().toMSecsSinceEpoch()/1000.0;

    //This reads value from telgram variable number plotableVariable which has been set in MainWindow::MainWindow
    double value;
    telegramListen->readData(plotableVariable,value);

    //this adds data to the graph, removes unnecessary data points etc.
    //for example, see details from http://qcustomplot.com/index.php/demos/realtimedatademo
    ui->plotter->graph(0)->addData(timeStamp,value);
    ui->plotter->graph(0)->removeDataBefore(timeStamp-timeInterval);
    ui->plotter->graph(0)->rescaleValueAxis();
    ui->plotter->xAxis->setRange(timeStamp+0.25, timeInterval, Qt::AlignRight);
    ui->plotter->replot();
}

void MainWindow::on_pushButton_counter_clicked()
{
    counter++;

    telegramBroadcast->readData(0,heartbeat);
    telegramBroadcast->setData(0,!heartbeat);
    telegramBroadcast->setData(1,counter);
    socket->broadcastTelegram(telegramBroadcast);

    ui->label_counter->setText(QString::number(counter));

}
