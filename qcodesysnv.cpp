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

#include "qcodesysnv.h"
#include <QDataStream>
#include <QStringList>
#include <iostream>



const QCodesysNVType QCodesysNVType::BOOL = QCodesysNVType(0,"BOOL",1,QCodesysNVcmpType::_uint8);
const QCodesysNVType QCodesysNVType::BYTE = QCodesysNVType(1,"BYTE",1,QCodesysNVcmpType::_uint8);
const QCodesysNVType QCodesysNVType::WORD = QCodesysNVType(2,"WORD",2,QCodesysNVcmpType::_uint16);
const QCodesysNVType QCodesysNVType::DWORD = QCodesysNVType(3,"DWORD",4,QCodesysNVcmpType::_uint32);
const QCodesysNVType QCodesysNVType::SINT = QCodesysNVType(4,"SINT",1,QCodesysNVcmpType::_int8);
const QCodesysNVType QCodesysNVType::USINT = QCodesysNVType(5,"USINT",1,QCodesysNVcmpType::_uint8);
const QCodesysNVType QCodesysNVType::INT = QCodesysNVType(6,"INT",2,QCodesysNVcmpType::_int16);
const QCodesysNVType QCodesysNVType::UINT = QCodesysNVType(7,"UINT",2,QCodesysNVcmpType::_uint16);
const QCodesysNVType QCodesysNVType::DINT = QCodesysNVType(8,"DINT",4,QCodesysNVcmpType::_int32);
const QCodesysNVType QCodesysNVType::UDINT = QCodesysNVType(9,"UDINT",4,QCodesysNVcmpType::_uint32);
const QCodesysNVType QCodesysNVType::REAL = QCodesysNVType(10,"REAL",4,QCodesysNVcmpType::_float);
const QCodesysNVType QCodesysNVType::LREAL = QCodesysNVType(11,"LREAL",8,QCodesysNVcmpType::_double);

const QList<QCodesysNVType> QCodesysNVType::typeList ({QCodesysNVType::BOOL, QCodesysNVType::BYTE, QCodesysNVType::WORD, QCodesysNVType::DWORD, QCodesysNVType::SINT, QCodesysNVType::USINT, QCodesysNVType::INT, QCodesysNVType::UINT, QCodesysNVType::DINT, QCodesysNVType::UDINT, QCodesysNVType::REAL, QCodesysNVType::LREAL});


QCodesysNVSocket::QCodesysNVSocket(QHostAddress ip,quint16 localPort,QObject *parent) :
    QObject(parent)
{
    telegramIdentity.resize(4);
    telegramIdentity[3]='3';
    telegramIdentity[2]='S';
    telegramIdentity[1]='-';
    telegramIdentity[0]=0;

    this->ip=ip;
    port=localPort;
    // create a QUDP socket
    socket = new QUdpSocket(this);

    // The most common way to use QUdpSocket class is
    // to bind to an address and port using bind()
    // bool QAbstractSocket::bind(const QHostAddress & address,
    //     quint16 port = 0, BindMode mode = DefaultForPlatform)
    socket->bind(ip, localPort);

    connect(socket, SIGNAL(readyRead()), this, SLOT(readyRead()));

}

int QCodesysNVSocket::broadcastTelegram(QCodesysNVTelegram *telegram)
{
    if(telegram->isBroadcastable())
    {
        QByteArray tempData;
        QHostAddress address;
        quint16 port;
        telegram->readTelegram(tempData);
        address=telegram->readIP();
        port=telegram->readPort();

        if (socket->writeDatagram(tempData,address,port)>0)
        {
            return(0);
        }
        else
        {
            qDebug() << "QCodesysNVSocket::broadcastTelegram: Error in broadcasting telegram.";
            qDebug() << "QCodesysNVSocket::broadcastTelegram: QUdpSocket didn't broadcast any bytes.";
            return(1);
        }


    }
    else
    {
        qDebug() << "QCodesysNVSocket::broadcastTelegram: Error in broadcasting telegram.";
        qDebug() << "QCodesysNVSocket::broadcastTelegram: Telegram is defined to be unbroadcastable.";
        return(1);
    }


}

int QCodesysNVSocket::broadcastTelegram(int i)
{
    if(i >= 0 && telegramList.size()>i)
    {
        return(broadcastTelegram(telegramList[i]));
    }
    else
    {
        qDebug() << "QCodesysNVSocket::BroadcastTelegram: telegramList does not include element " << i;
        return(1);
    }

}

void QCodesysNVSocket::addTelegram(QCodesysNVTelegram *telegram)
{
    telegramList.append(telegram);
}

QCodesysNVTelegram *QCodesysNVSocket::getTelegram(int i)
{
    if(i >= 0 && telegramList.size()>i)
    {
        return(telegramList[i]);
    }
    else
    {
        return(NULL);
    }
}

int QCodesysNVSocket::telegramCount()
{
    return(telegramList.size());
}

void QCodesysNVSocket::clearTelegrams()
{
    telegramList.clear();
}

void QCodesysNVSocket::replaceTelegram(int i, QCodesysNVTelegram *telegram)
{
    if(i >= 0 && telegramList.size()>i)
    {
        telegramList[i]=telegram;
    }
}

void QCodesysNVSocket::removeTelegram(int i)
{
    if(i >= 0 && telegramList.size()>i)
    {
        telegramList.removeAt(i);
    }
}

void QCodesysNVSocket::readyRead()
{
    QByteArray buffer;
    buffer.resize(socket->pendingDatagramSize());

    QHostAddress senderIP;
    quint16 senderPort;

    while (socket->readDatagram(buffer.data(), buffer.size(),&senderIP, &senderPort)>0)
    {
        emit udpPackageReceived(senderIP.toString(),senderPort,buffer.length());

        bool CODESYStelegram=1;
        for (int i=0;i<telegramIdentity.size();i++)
        {
            if (telegramIdentity[i]!=buffer[i])
            {
                CODESYStelegram=0;
                break;
            }
        }

        if(CODESYStelegram==1)
        {
            quint16 incomingCobID;
            QCodesysNVConvert<quint16>::convertFromBytes(buffer.mid(8,2),QCodesysNVType::WORD,incomingCobID);
            emit codesysTelegramReceived(senderIP.toString(),senderPort,buffer.length(),incomingCobID);



            if(buffer.size()>20 && telegramList.size()>0)
            {

                for (int i=0;i<telegramList.size();i++)
                {
                    if(telegramList[i]->readCobId()==incomingCobID)
                    {
                        telegramList[i]->parseTelegram(buffer, senderIP, senderPort);
                        break;
                    }
                }

            }
        }
        buffer.clear();
        buffer.resize(socket->pendingDatagramSize());
    }
}

QCodesysNVTelegram::QCodesysNVTelegram(bool broadcastable, QObject *parent)
{
    dataUpdated=0;
    this->broadcastable = broadcastable;
    identity.resize(4);
    identity[0]='3';
    identity[1]='S';
    identity[2]='-';
    identity[3]=0;

    id=0;
    cob_id=65535;
    subindex=0;
    items=0;
    length=0;
    counter=0;
    flags=0;
    checksum=0;

}

int QCodesysNVTelegram::setVariableTypes(const QList<QCodesysNVType> &variables)
{
    if (variables.length()>0)
    {
        variableTypeList=variables;
        recalculateDataPositions();
        return(0);
    }
    else
    {
        qDebug() << "QCodesysNVTelegram::setVariableTypes: QList does not include any elements";
        return(1);
    }
}

int QCodesysNVTelegram::setVariableTypes(QString variables)
{
    variables = variables.simplified();
    variables.replace( " ", "" );

    QStringList variableStringList;
    variableStringList.clear();
    variableStringList=variables.split(",");
    QList<QCodesysNVType> variableList;

    //variableTypeList.clear();
    for(int i=0;i<variableStringList.length();i++)
    {
        bool isItem=0;
        for (int j=0; j<QCodesysNVType::typeList.size();j++)
        {
            if(QCodesysNVType::typeList[j].name.compare(variableStringList[i])== 0)
            {
                variableList.push_back(QCodesysNVType::typeList[j]);
                isItem=1;
                break;
            }
        }
        if(isItem==0)
        {
            qDebug() << "QCodesysNVTelegram::setVariableTypes: typo in variable string";
            return(1);
        }

    }
    return(setVariableTypes(variableList));
}


void QCodesysNVTelegram::clearVariableTypes()
{
    variableTypeList.clear();
}

QList<QCodesysNVType> QCodesysNVTelegram::readVariableTypes()
{
    return(variableTypeList);
}

void QCodesysNVTelegram::readVariableTypes(QString &variables)
{
    variables.clear();
    if(variableTypeList.size()>0)
    {
        for(int i=0;i<variableTypeList.size();i++)
        {
            for (int j=0; j<QCodesysNVType::typeList.size();j++)
            {
                if(variableTypeList[i] == QCodesysNVType::typeList[j])
                {
                    variables.append(QCodesysNVType::typeList[j].name);
                }
            }

            if (i<(variableTypeList.length()-1))
            {
                variables.append(", ");
            }
        }
    }
}

void QCodesysNVTelegram::readVariableTypes(QList<QCodesysNVType> &variables)
{
    variables=variableTypeList;
}

int QCodesysNVTelegram::variableCount()
{
    return(variableTypeList.size());
}



void QCodesysNVTelegram::recalculateDataPositions()
{
    totalDataLength=0;
    for (int i=0;i<variableTypeList.size();i++)
    {
        dataLengths.push_back(variableTypeList[i].size);
        dataPositions.push_back(totalDataLength);
        totalDataLength=totalDataLength+variableTypeList[i].size;
    }
    length=totalDataLength+20;
    items=variableTypeList.size();

    initializeTelegram();

}

void QCodesysNVTelegram::parseTelegram(QByteArray telegram, QHostAddress IPin, quint16 portIn)
{
    //qDebug() << "Something there";
    if(telegram.length()==(totalDataLength+20))
    {

        data=telegram;
        IP=IPin;
        port=portIn;
        dataUpdated=1;
        emit updated();
    }
    else
    {
        qDebug() << "QCodesysNVTelegram::parseTelegram: Telegram with cobID " << this->cob_id << "received, but length is " << telegram.length() << " instead of " <<  (totalDataLength+20);
    }

}

quint32 QCodesysNVTelegram::readId()
{
    quint32 temp;
    QCodesysNVConvert<quint32>::convertFromBytes(data.mid(4,4),QCodesysNVType::DWORD,temp);
    return(temp);
}

quint16 QCodesysNVTelegram::readCobId()
{
    quint16 temp;
    QCodesysNVConvert<quint16>::convertFromBytes(data.mid(8,2),QCodesysNVType::WORD,temp);
    return(temp);
}

quint16 QCodesysNVTelegram::readSubIndex()
{
    quint16 temp;
    QCodesysNVConvert<quint16>::convertFromBytes(data.mid(10,2),QCodesysNVType::WORD,temp);
    return(temp);
}

quint16 QCodesysNVTelegram::readItems()
{
    quint16 temp;
    QCodesysNVConvert<quint16>::convertFromBytes(data.mid(12.2),QCodesysNVType::WORD,temp);
    return(temp);
}

quint16 QCodesysNVTelegram::readLength()
{
    quint16 temp;
    QCodesysNVConvert<quint16>::convertFromBytes(data.mid(14,2),QCodesysNVType::WORD,temp);
    return(temp);
}

quint16 QCodesysNVTelegram::readCounter()
{
    quint16 temp;
    QCodesysNVConvert<quint16>::convertFromBytes(data.mid(16,2),QCodesysNVType::WORD,temp);
    return(temp);
}

quint8 QCodesysNVTelegram::readFlags()
{
    quint8 temp;
    QCodesysNVConvert<quint8>::convertFromBytes(data.mid(18,1),QCodesysNVType::BYTE,temp);
    return(temp);
}

quint8 QCodesysNVTelegram::readChecksum()
{
    quint8 temp;
    QCodesysNVConvert<quint8>::convertFromBytes(data.mid(19,1),QCodesysNVType::BYTE,temp);
    return(temp);
}

void QCodesysNVTelegram::readTelegram(QByteArray &dataOut)
{
    dataOut=data;
}

int QCodesysNVTelegram::setIP(QHostAddress IPset)
{
    if (isBroadcastable())
    {
        IP=IPset;
        initializeTelegram();
        return(0);
    }
    else
    {
        qDebug() << "QCodesysNVTelegram::setIP: IP can not be set";
        qDebug() << "QCodesysNVTelegram::setIP: Telegram type is not broadcastable";
        return(1);
    }
}

int QCodesysNVTelegram::setPort(quint16 portSet)
{
    if (isBroadcastable())
    {
        port=portSet;
        initializeTelegram();
        return(0);
    }
    else
    {
        qDebug() << "QCodesysNVTelegram::setPort: Port can not be set";
        qDebug() << "QCodesysNVTelegram::setPort: Telegram type is not broadcastable";
        return(1);
    }
}

int QCodesysNVTelegram::setId(quint32 IDset)
{
    if (isBroadcastable())
    {
        id=IDset;
        initializeTelegram();
        return(0);
    }
    else
    {
        qDebug() << "QCodesysNVTelegram::setId: Id can not be set";
        qDebug() << "QCodesysNVTelegram::setId: Telegram type is not broadcastable";
        return(1);
    }
}

void QCodesysNVTelegram::setCobId(quint16 cobID)
{
    this->cob_id=cobID;
        initializeTelegram();
}

int QCodesysNVTelegram::setSubIndex(quint16 subIndex)
{
    if (isBroadcastable())
    {
        this->subindex=subIndex;
        initializeTelegram();
        return(0);
    }
    else
    {
        qDebug() << "QCodesysNVTelegram::setSubIndex: SubIndex can not be set";
        qDebug() << "QCodesysNVTelegram::setSubIndex: Telegram type is not broadcastable";
        return(1);
    }

}

int QCodesysNVTelegram::setCounter(quint16 counter)
{
    if (isBroadcastable())
    {
        this->counter=counter;
        initializeTelegram();
        return(0);
    }
    else
    {
        qDebug() << "QCodesysNVTelegram::setCounter: Counter can not be set";
        qDebug() << "QCodesysNVTelegram::setCounter: Telegram type is not broadcastable";
        return(1);
    }

}

int QCodesysNVTelegram::setFlags(quint8 flags)
{
    if (isBroadcastable())
    {
        this->flags=flags;
        initializeTelegram();
        return(0);
    }
    else
    {
        qDebug() << "QCodesysNVTelegram::setFlags: Flags can not be set";
        qDebug() << "QCodesysNVTelegram::setFlags: Telegram type is not broadcastable";
        return(1);
    }

}

int QCodesysNVTelegram::setChecksum(quint8 checksum)
{
    if (isBroadcastable())
    {
        this->checksum=checksum;
        initializeTelegram();
        return(0);
    }
    else
    {
        qDebug() << "QCodesysNVTelegram::setChecksum: Checksum can not be set";
        qDebug() << "QCodesysNVTelegram::setChecksum: Telegram type is not broadcastable";
        return(1);
    }

}

bool QCodesysNVTelegram::isBroadcastable()
{
    return(broadcastable);
}


bool QCodesysNVTelegram::isUpdated()
{
    if(dataUpdated==1)
    {
        dataUpdated=0;
        return(1);
    }
    return(0);
}

QHostAddress QCodesysNVTelegram::readIP()
{
    return(IP);
}

quint16 QCodesysNVTelegram::readPort()
{
    return(port);
}

void QCodesysNVTelegram::dataVarToString(int index,QString& variableString)
{
    variableString.clear();
    if (index < variableTypeList.length())
    {
        if(variableTypeList[index].varType==QCodesysNVcmpType::_uint8 || variableTypeList[index].varType==QCodesysNVcmpType::_uint16 || variableTypeList[index].varType==QCodesysNVcmpType::_uint32 || variableTypeList[index].varType==QCodesysNVcmpType::_uint64)
        {
            quint64 tempData;
            readData(index,tempData);
            variableString.append(QString::number(tempData));
        }
        else if(variableTypeList[index].varType==QCodesysNVcmpType::_int8 || variableTypeList[index].varType==QCodesysNVcmpType::_int16 || variableTypeList[index].varType==QCodesysNVcmpType::_int32 || variableTypeList[index].varType==QCodesysNVcmpType::_int64)
        {
            qint64 tempData;
            readData(index,tempData);
            variableString.append(QString::number(tempData));
        }
        else if(variableTypeList[index].varType==QCodesysNVcmpType::_float || variableTypeList[index].varType==QCodesysNVcmpType::_double)
        {
            double tempData;
            readData(index,tempData);
            variableString.append(QString::number(tempData));
        }
        else
        {

            qDebug() << "QCodesysNVTelegram::dataVarToString: Wrong datatype (deep, unexpexted bug, in QCodesysNV* classes)";
        }
    }
    else
    {
        qDebug() << "QCodesysNVTelegram::dataVarToString: length of the variableList is only " << variableTypeList.length() << " but you are trying to handle index " << index;
    }
}

void QCodesysNVTelegram::stringToDataVar(int index,QString variableString)
{
    //variableString.clear();
    //qDebug() << "QCodesysNVTelegram::stringToDataVar: string" << variableString;
    if (index<variableTypeList.length())
    {
        if(variableTypeList[index].varType==QCodesysNVcmpType::_uint8 || variableTypeList[index].varType==QCodesysNVcmpType::_uint16 || variableTypeList[index].varType==QCodesysNVcmpType::_uint32 || variableTypeList[index].varType==QCodesysNVcmpType::_uint64)
        {
            quint64 tempData=variableString.toULongLong();
            setData(index,tempData);
        }
        else if(variableTypeList[index].varType==QCodesysNVcmpType::_int8 || variableTypeList[index].varType==QCodesysNVcmpType::_int16 || variableTypeList[index].varType==QCodesysNVcmpType::_int32 || variableTypeList[index].varType==QCodesysNVcmpType::_int64)
        {
            qint64 tempData=variableString.toLongLong();
            setData(index,tempData);
        }
        else if(variableTypeList[index].varType==QCodesysNVcmpType::_float || variableTypeList[index].varType==QCodesysNVcmpType::_double)
        {
            double tempData=variableString.toDouble();
            setData(index,tempData);
        }
        else
        {

            qDebug() << "QCodesysNVTelegram::stringToDataVar: Wrong datatype (deep, unexpexted bug, in QCodesysNV* classes)";
        }
    }
    else
    {
        qDebug() << "QCodesysNVTelegram::stringToDataVar: length of the variableList is only " << variableTypeList.length() << " but you are trying to handle index " << index;
    }
}

int QCodesysNVTelegram::stringToData(QString dataString){
    dataString= dataString.simplified();
    dataString.replace(" ", "");

    QStringList dataList;
    dataList.clear();
    dataList=dataString.split(",");
    //qDebug() << "QCodesysNVTelegram::stringToData: List items " << dataList.length() << dataList[0];
    if (variableTypeList.length()>0 && variableTypeList.length() == dataList.length())
    {
        for(unsigned int i=0;i<variableTypeList.length();i++)
        {
            stringToDataVar(i,dataList[i]);
        }
    }
    else
    {
        qDebug() << "QCodesysNVTelegram::StringListToData: QString includes wrong number of variables.";
        return(1);
    }

    return(0);
}

int QCodesysNVTelegram::stringListToData(QStringList &dataStringList)
{

    if (variableTypeList.length()>0 && variableTypeList.length()==dataStringList.length())
    {
        for(int i=0;i<variableTypeList.length();i++)
        {
            stringToDataVar(i,dataStringList[i]);
        }
    }
    else
    {
        qDebug() << "QCodesysNVTelegram::StringListToData: QStringList includes wrong number of variables.";
        return(1);
    }

    return(0);
}

void QCodesysNVTelegram::dataToString(QString& dataString)
{
    dataString.clear();
    if (variableTypeList.length()>0)
    {
        for(int i=0;i<variableTypeList.length();i++)
        {
            QString tempString;
            dataVarToString(i,tempString);
            dataString.append(tempString);

            if(i<(variableTypeList.length()-1))
            {
                dataString.append(", ");
            }


        }

    }
}

void QCodesysNVTelegram::dataToStringList(QStringList &dataStrings)
{
    dataStrings.clear();
    if (variableTypeList.length()>0)
    {
        for(int i=0;i<variableTypeList.length();i++)
        {
            QString tempString;
            dataVarToString(i,tempString);
            dataStrings.push_back(tempString);
        }

    }
}

void QCodesysNVTelegram::initializeTelegram(void)
{
    if(length<20)
    {
        data.resize(length);
    }
    else
    {
        data.resize(length);
    }
        QByteArray buffer;
        QDataStream bufferStream(identity);
        quint32 identityInt;
        bufferStream >> identityInt;


        QCodesysNVConvert<quint32>::convertToBytes(buffer, QCodesysNVType::DWORD, identityInt);
        data.replace(0,4,buffer);

        buffer.clear();
        QCodesysNVConvert<quint32>::convertToBytes(buffer, QCodesysNVType::DWORD, id);
        data.replace(4,4,buffer);

        buffer.clear();
        QCodesysNVConvert<quint16>::convertToBytes(buffer, QCodesysNVType::WORD, cob_id);
        data.replace(8,2,buffer);

        buffer.clear();
        QCodesysNVConvert<quint16>::convertToBytes(buffer, QCodesysNVType::WORD, subindex);
        data.replace(10,2,buffer);

        buffer.clear();
        QCodesysNVConvert<quint16>::convertToBytes(buffer, QCodesysNVType::WORD, items);
        data.replace(12,2,buffer);

        buffer.clear();
        QCodesysNVConvert<quint16>::convertToBytes(buffer, QCodesysNVType::WORD, length);
        data.replace(14,2,buffer);

        buffer.clear();
        QCodesysNVConvert<quint16>::convertToBytes(buffer, QCodesysNVType::WORD, counter);
        data.replace(16,2,buffer);

        buffer.clear();
        QCodesysNVConvert<quint8>::convertToBytes(buffer, QCodesysNVType::BYTE, flags);
        data.replace(18,1,buffer);

        buffer.clear();
        QCodesysNVConvert<quint8>::convertToBytes(buffer, QCodesysNVType::BYTE, checksum);
        data.replace(19,1,buffer);


}
