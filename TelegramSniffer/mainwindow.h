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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QTimer>
#include <QVector>
#include "../qcodesysnv.h"
#include "popup.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

    //slot for incoming data from receivableTelegram
    void dataAvailable();

    //slot for datagrams received by socket
    void CODESYSreceived(QString IP, unsigned int port, unsigned int bytes, unsigned int cobid);
    void UDPreceived(QString IP, unsigned int port, unsigned int bytes);

    //slots for timers
    void updateCounters();
    void updateDatalogger();
    void updateHearBeat();

    //slots for different buttons and other user interface objects
    void on_comboBox_rateTime_currentIndexChanged(int index);
    void on_checkBox_LogToFile_clicked();
    void on_comboBox_dataloggerTimestep_currentIndexChanged(int index);
    void on_checkBox_logUDPtraffic_clicked();
    void on_pushButton_broadcast_clicked();
    void on_pushButton_listen_clicked();
    void on_checkBox_hearBeat_clicked();
    void on_spinBox_hearBeatInterval_valueChanged(int arg1);

private:
    void setupSocket();

    Ui::MainWindow *ui;
    PopUp *popupWindow;

    //socket and telegrams
    QCodesysNVSocket *socket;
    QCodesysNVTelegram *receivableTelegram;
    QCodesysNVTelegram *broadcastableTelegram;

    QStringList variableList;

    //objects for data logs in Listen page
    QFile *logFile;
    QTextStream *logStream;
    QDateTime startingTime;

    //objects for traffic logs in Sniffer page
    QFile *trafficLogFile;
    QTextStream *trafficLogStream;
    QDateTime trafficLogStartingTime;

    //timers for counters, datalogger and hearbeat
    QTimer *counterTimer;
    QTimer *dataloggerTimer;
    QTimer *heartBeatTimer;
    bool heartBeatValue;

    //comboBox_rateTime values (indexes) to seconds
    QVector <int> rateTimeValues;
    QVector <int> dataloggerTimeValues;


    //counters for different datagramtypes
    int UDPcounter;
    int CODESYScounter;
    int TELEGRAMcounter;

    int tempCounter;
};

#endif // MAINWINDOW_H
