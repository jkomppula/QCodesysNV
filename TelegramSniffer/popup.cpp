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

#include "popup.h"
#include "ui_popup.h"

PopUp::PopUp(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PopUp)
{
    ui->setupUi(this);
}

PopUp::~PopUp()
{
    delete ui;
}

void PopUp::SetLabel1(QString label)
{
    ui->label1->setText(label);
}
void PopUp::SetLabel2(QString label)
{
    ui->label2->setText(label);
}
void PopUp::SetLabel3(QString label)
{
    ui->label3->setText(label);
}
void PopUp::SetLabel4(QString label)
{
    ui->label4->setText(label);
}
void PopUp::SetLabel5(QString label)
{
    ui->label5->setText(label);
}

 void PopUp::ClearLabel1(void)
 {
     ui->label1->clear();
 }
 void PopUp::ClearLabel2(void)
 {
     ui->label2->clear();
 }
 void PopUp::ClearLabel3(void)
 {
     ui->label3->clear();
 }
 void PopUp::ClearLabel4(void)
 {
     ui->label4->clear();
 }
 void PopUp::ClearLabel5(void)
 {
     ui->label5->clear();
 }

 void PopUp::ClearAll(void)
 {
     ClearLabel1();
     ClearLabel2();
     ClearLabel3();
     ClearLabel4();
     ClearLabel5();
 }
