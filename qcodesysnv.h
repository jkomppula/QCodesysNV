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

#ifndef QCODESYSNV_H
#define QCODESYSNV_H

#include <QList>
#include <QVector>
#include <vector>
#include <QObject>
#include <QUdpSocket>
#include <QByteArray>
#include <type_traits>
#include <QDataStream>

using namespace std;

//Socket which handles QCodesysNVTelegrams
class QCodesysNVSocket;

//CoDeSys network variable telegram
class QCodesysNVTelegram;

//types of CoDeSys network variables
class QCodesysNVType;

//this class includes functions which convert
template <typename Type> class QCodesysNVConvert;

//variable types in the computer
enum class QCodesysNVcmpType {_int8,_uint8,_int16,_uint16,_int32,_uint32,_int64,_uint64,_float,_double};

//this class handles receiving and sending of QCodesysNVTelegrams
class QCodesysNVSocket : public QObject
{
    Q_OBJECT

public:

    explicit QCodesysNVSocket(QHostAddress ip,quint16 localPort,QObject *parent = 0);

    //telegram can be broadcasted by using pointer to telegram or element of telegramList
    //note that index is might not be constant if removeTelegram(int index) is used
    int broadcastTelegram(QCodesysNVTelegram* telegram);
    int broadcastTelegram(int i);

    //this adds the telegram to telegramList
    void addTelegram(QCodesysNVTelegram* telegram);

    //return pointer to telegram
    //note that index is might not be constant if removeTelegram(int index) is used
    QCodesysNVTelegram* getTelegram(int i);

    //number of telegrams
    int telegramCount();

    //clear telegramList
    void clearTelegrams();

    void replaceTelegram(int i,QCodesysNVTelegram* telegram);
    //removes telegram from teh list
    //note that index is might not be constant if removeTelegram(int index) is used
    void removeTelegram(int i);



signals:
    //these signals are emitted when UDP package or any codesys telegram is received
    void udpPackageReceived(QString IP, unsigned int port, unsigned int bytes);
    void codesysTelegramReceived(QString IP, unsigned int port, unsigned int bytes, unsigned int cobid);

public slots:
    //data available in UDP socket
    void readyRead();
private:

    //this is a list of pointers to telegrams. This class check if a CobId of incoming telegram corresponds to any of those in the list
    QList <QCodesysNVTelegram*> telegramList;

    //identity (first bytes) of the real CoDeSys telegram
    QByteArray telegramIdentity;

    //standard udp socket in QT
    QUdpSocket *socket;

    //IP address and port limitations for incoming telegrams
    QHostAddress ip;
    quint16 port;
};



//this is the telegram which is received or broadcasted via UDP
class QCodesysNVTelegram : public QObject
{
    Q_OBJECT

public:

    explicit QCodesysNVTelegram(bool telegramWritable = 0, QObject *parent = 0);

    int setVariableTypes(const QList<QCodesysNVType>& variables);
    int setVariableTypes(QString variables);

    void clearVariableTypes(void);

    QList<QCodesysNVType> readVariableTypes(void);
    void readVariableTypes(QString& variables);
    void readVariableTypes(QList<QCodesysNVType>& variables);

    int variableCount(void);

    int stringToData(QString dataString);
    void dataToString(QString& dataString);

    int stringListToData(QStringList& dataStringList);
    void dataToStringList(QStringList& dataStrings);

    void parseTelegram(QByteArray telegram, QHostAddress IP, quint16 port);


    //read properties of the telegram
    QHostAddress readIP(void);
    quint16 readPort(void);
    QString readIdentity(void);
    quint32 readId(void);
    quint16 readCobId(void);
    quint16 readSubIndex(void);
    quint16 readItems(void);
    quint16 readLength(void);
    quint16 readCounter(void);
    quint8 readFlags(void);
    quint8 readChecksum(void);
    void readTelegram(QByteArray& data);

    //set properties of the telegram
    //part of them can be set only if the telegram is writable
    int setIP(QHostAddress IP);
    int setPort(quint16 port);
    int setId(quint32 ID);
    void setCobId(quint16 cobID);
    int setSubIndex(quint16 subIndex);
    int setCounter(quint16 counter);
    int setFlags(quint8 flags);
    int setChecksum(quint8 checksum);

    //check if telegram is writable
    bool isBroadcastable(void);

    //check if telegram is updated
    bool isUpdated(void);

    //read data from variable
    //this SHOULD be safe so that you can use any Type
    template <typename Type>
    void readData(int index, Type &variable);

    //read data from variable
    //this SHOULD be safe so that you can use any variable Type
    template <typename Type>
    void setData(int index, Type variable);

private:
    void recalculateDataPositions();
    void initializeTelegram(void);

    QHostAddress IP;
    quint16 port;
    QByteArray identity;
    quint32 id;
    quint16 cob_id;
    quint16 subindex;
    quint16 items;
    quint16 length;
    quint16 counter;
    quint8 flags;
    quint8 checksum;

    quint8 totalDataLength;

    QList<int> dataPositions;
    QList<int> dataLengths;

    bool broadcastable;

    bool dataUpdated;

    QByteArray data;

    void dataVarToString(int index,QString& variableString);
    void stringToDataVar(int index,QString variableString);

    QList <QCodesysNVType> variableTypeList;

signals:
    void updated(void);
};


template <typename Type>
void QCodesysNVTelegram::readData(int index, Type &variable){
    if(index<variableTypeList.length())
    {
       QCodesysNVConvert<Type>::convertFromBytes(data.mid((20+dataPositions[index]),dataLengths[index]), variableTypeList[index],variable);
    }
    else
    {
        qDebug() << "QCodesysNVTelegram::readData: wrong index number";
    }
}
template <typename Type>
void QCodesysNVTelegram::setData(int index, Type variable){
    if(index<variableTypeList.length())
    {
        QByteArray buffer;
        QCodesysNVConvert<Type>::convertToBytes(buffer, variableTypeList[index], variable);
        data.replace((20+dataPositions[index]),dataLengths[index],buffer);
    }
    else
    {
        qDebug() << "QCodesysNVTelegram::readData: wrong index number";
    }


}

class QCodesysNVType {
public:

    //codesys variable types
    static const QCodesysNVType BOOL;
    static const QCodesysNVType BYTE;
    static const QCodesysNVType WORD;
    static const QCodesysNVType DWORD;
    static const QCodesysNVType SINT;
    static const QCodesysNVType USINT;
    static const QCodesysNVType INT;
    static const QCodesysNVType UINT;
    static const QCodesysNVType DINT;
    static const QCodesysNVType UDINT;
    static const QCodesysNVType REAL;
    static const QCodesysNVType LREAL;


    //list of all codesys varirable types
    static const QList <QCodesysNVType> typeList;

    //overloaded operators for comparing variable types
    friend bool operator== (const QCodesysNVType& lhs, const QCodesysNVType& rhs){
        return (lhs.index == rhs.index);
    }

    friend bool operator!= (const QCodesysNVType& lhs, const QCodesysNVType& rhs){
        return !(lhs == rhs);
    }

    quint32 index;
    quint32 size;
    QCodesysNVcmpType varType;
    QString name;

private:


    //variable type is defined as object which has following properties
    QCodesysNVType(quint32 i,QString name, quint8 size,QCodesysNVcmpType varType){
        this->index=i; //unique index number
        this->name=name; //string name for parsing types from string list
        this->size=size; //length in bytes
        this->varType=varType; //variable type in standard computer variables
    }
};


//Static class which convert variables to bytearray and vice versa
template <class Type>
class QCodesysNVConvert {
public:
    static void convertFromBytes(QByteArray bytes, QCodesysNVType netType, Type& variable);
    static void convertToBytes(QByteArray& bytes, QCodesysNVType netType, Type variable);
};

//convert bytearray from telegram to any variable type (by using casting if necessary)
template<typename Type>
void QCodesysNVConvert<Type>::convertFromBytes(QByteArray bytes, QCodesysNVType cmpType, Type& variable)
{
    QDataStream stream(bytes);
    stream.setByteOrder(QDataStream::LittleEndian);

    if(cmpType.varType==QCodesysNVcmpType::_uint8)
    {
        quint8 tempVar;
        stream >> tempVar;
        variable = (Type) tempVar;
    }
    else if(cmpType.varType==QCodesysNVcmpType::_int8)
    {
        qint8 tempVar;
        stream >> tempVar;
        variable = (Type) tempVar;
    }
    else if(cmpType.varType==QCodesysNVcmpType::_uint16)
    {
        quint16 tempVar;
        stream >> tempVar;
        variable = (Type) tempVar;
    }
    else if(cmpType.varType==QCodesysNVcmpType::_int16)
    {
        qint16 tempVar;
        stream >> tempVar;
        variable = (Type) tempVar;
    }
    else if(cmpType.varType==QCodesysNVcmpType::_uint32)
    {
        quint32 tempVar;
        stream >> tempVar;
        variable = (Type) tempVar;
    }
    else if(cmpType.varType==QCodesysNVcmpType::_int32)
    {
        qint32 tempVar;
        stream >> tempVar;
        variable = (Type) tempVar;
    }
    else if(cmpType.varType==QCodesysNVcmpType::_uint64)
    {
        quint64 tempVar;
        stream >> tempVar;
        variable = (Type) tempVar;
    }
    else if(cmpType.varType==QCodesysNVcmpType::_int64)
    {
        qint64 tempVar;
        stream >> tempVar;
        variable = (Type) tempVar;
    }
    else if(cmpType.varType==QCodesysNVcmpType::_float)
    {
        stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
        float tempVar;
        stream >> tempVar;
        variable = (Type) tempVar;
    }
    else if(cmpType.varType==QCodesysNVcmpType::_double)
    {
        stream.setFloatingPointPrecision(QDataStream::DoublePrecision);
        double tempVar;
        stream >> tempVar;
        variable = (Type) tempVar;
    }
}


//convert any variable type (by using casting if necessary) to QByteArray format which is suitable for Codesys UDP telegram
template <typename Type>
void QCodesysNVConvert<Type>::convertToBytes(QByteArray& bytes, QCodesysNVType cmpType, Type variable)
{
    bytes.clear();
    //Creates writeable QDataStream and changes byte order
    QDataStream stream(&bytes, QIODevice::WriteOnly);
    stream.setByteOrder(QDataStream::LittleEndian);

    //the conversion depends on the data type
    if(cmpType.varType==QCodesysNVcmpType::_uint8)
    {
        quint8 tempVar = (quint8) variable;
        stream << tempVar;
    }
    else if(cmpType.varType==QCodesysNVcmpType::_int8)
    {
        qint8 tempVar = (qint8) variable;
        stream << tempVar;
    }
    else if(cmpType.varType==QCodesysNVcmpType::_uint16)
    {
        quint16 tempVar = (quint16) variable;
        stream << tempVar;
    }
    else if(cmpType.varType==QCodesysNVcmpType::_int16)
    {
        qint16 tempVar = (qint16) variable;
        stream << tempVar;
    }
    else if(cmpType.varType==QCodesysNVcmpType::_uint32)
    {
        quint32 tempVar = (quint32) variable;
        stream << tempVar;
    }
    else if(cmpType.varType==QCodesysNVcmpType::_int32)
    {
        qint32 tempVar = (qint32) variable;
        stream << tempVar;
    }
    else if(cmpType.varType==QCodesysNVcmpType::_uint64)
    {
        quint64 tempVar = (quint64) variable;
        stream << tempVar;
    }
    else if(cmpType.varType==QCodesysNVcmpType::_int64)
    {
        qint64 tempVar = (qint64) variable;
        stream << tempVar;
    }
    else if(cmpType.varType==QCodesysNVcmpType::_float)
    {
        stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
        float tempVar = (float) variable;
        stream << tempVar;
    }
    else if(cmpType.varType==QCodesysNVcmpType::_double)
    {
        stream.setFloatingPointPrecision(QDataStream::DoublePrecision);
        double tempVar = (double) variable;
        stream << tempVar;
    }
}


#endif // QCODESYSNV_H
