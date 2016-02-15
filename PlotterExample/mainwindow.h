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
#include "qcustomplot.h"
#include "../qcodesysnv.h"

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
    void on_comboBox_timeinterval_currentIndexChanged(int index);

    //Slot which is connected to updated() signal of telegramPlotData
    void plotDataReceived(void);

    //Slot which is connected to updated() signal of telegramReceivedCounter
    void counterReceived(void);

    void on_pushButton_counter_clicked();

private:
    Ui::MainWindow *ui;

    //Codesys network variable socket
    QCodesysNVSocket *socket;

    //Codesys network variable telegram, which is listened
    QCodesysNVTelegram *telegramPlotData;

    //Codesys network variable telegram, which is listened
    QCodesysNVTelegram *telegramReceivedCounter;

    //Codesys network variable telegram, which is broadcasted
    QCodesysNVTelegram *telegramBroadcastedCounter;

    //time interval of the plotter
    double timeInterval;

    //an index of the variable which is plotted
    int plotableVariable;

    //counter value
    int counter;

    //heart beat value
    bool heartbeat;
};

#endif // MAINWINDOW_H
