#ifndef MYSERVER_H
#define MYSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QFile>

#include <iostream>
using std::cout;
using std::cin;
using std::endl;
using std::system;

#include <stack>
using std::stack;

#include <string>
using std::string;

#include <cstdlib>
using std::atof;

#include <cctype>
using std::isdigit;

#include <stdexcept>
using std::runtime_error;

class myserver: public QTcpServer
{
    Q_OBJECT
public:
    myserver();
    ~myserver();
    QTcpSocket* socket;
    QByteArray Data;
    string data_calculate;

    QJsonDocument doc;
    QJsonParseError docError;

private:
    string rpn(const string);       // ф-ия преобразования в обр.польскую нотацию
    int priority(const char);       // ф-ия определения приоритета операции
    double calculate(const string); // ф-ия вычисления результата


public slots:
    void startServer();
    void incomingConnection(qintptr socketDescriptor);
    void sockReady();
    void sockDisk();
};

#endif // MYSERVER_H
