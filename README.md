QCodesysNV
==========
QCodesysNV is a QT C++ library for communication with CODESYS network variable UDP protocol. It has been tested with QT versions 5.1.1 (Microsoft Windows), 5.2.1 (Ubuntu Linux)  and 5.4.2 (Red Hat Enterprise Linux)

How to use?
-----------
1. Create a new QT project
2. Copy *qcodesysnv.h* and *qcodesysnv.cpp* files to your project folder
3. Add the files to your project by right click on the root entry of your project in the left sidebar and choose Add Existing Files...
4. Add following lines to your .pro file
    QT       += network
    CONFIG   += c++11
5. Some ubuntu versions require also following line to your .pro file:
    QMAKE_CXXFLAGS += -std=c++11

Changelog
---------
###0.6
* The first public version



