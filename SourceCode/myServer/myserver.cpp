#include "myserver.h"

myserver::myserver(){}
myserver::~myserver(){}

std::string myserver::rpn(const std::string origin)
{
    stack<char> operation;              // стек хранения операций
    string result;                      // строка в обр.польской нотацие
    string::const_iterator it, it_copy; // итератор для прохода по строке

    for (it = origin.begin(); it<origin.end(); ++it) //выполняется проход
    {
        //если прочитанный символ - цифра, прибавляем к result
        if (isdigit(*it))       
        {
            result += *it;
            it_copy = it + 1;
            if (it_copy >= origin.begin() && it_copy < origin.end() && (*(it_copy) == '.'))
            {
                result  += *it_copy;
                it_copy += 1;
                while (isdigit(*it_copy))
                {
                    result += *it_copy++;
                }
                it = it_copy - 1;
            }
        }
        // если сивол - не цифра, то проверяем, является ли символ операцией
        else  
        {
            int currentPr = priority(*it); //при неизвестном символе возвращается 0

            // если введен некорректный символ, пропускаем его и переходим к следующему символу в строке origin
            if (currentPr)
            {
                // для удобства последующих преобразований после каждых прочитаных
                // числа или операции ставим пробел в result
                result += " ";

                if (!operation.empty())
                {
                    // если приоритет текущей прочитаной операции не больше приоритета предыдущих
                    // прочитаных операций, выталкиваем со стека в result все те операции,
                    // для которых выполняется условие неравенства
                    while (!operation.empty() && currentPr <= priority(operation.top()))
                    {
                        char value_str = operation.top();
                        result = result + value_str;
                        if(value_str != '.')
                            result += " ";
                        operation.pop();
                    }
                }
                operation.push(*it); // запихиваем в стек текущую операцию
            }
        }
    }
    // выталкиваем в result все, что осталось в стеке
    while (!operation.empty())
    {
        result = result + " " + operation.top();
        operation.pop();
    }
    return result;
}

int myserver::priority(const char symbol)
{
    if (symbol == '+' || symbol == '-')
        return 1;                               //низкий приоритет операций
    else if (symbol == '*' || symbol == '/')
        return 2;                               //высокий приоритет операций
    else if (symbol == '.')
        return 3;
    else
        throw runtime_error("unrecognized symbol\n");
}

double myserver::calculate(const std::string rpn_st)
{
    stack<double> numbers; //стек чисел
    string temp; //временная строка для хранения числа
    string::const_iterator it;
    //double result = 0;
    int flag = 0;
    for (it = rpn_st.begin(); it<rpn_st.end(); ++it)
    {
        // если прочитанный символ - цифра, прибавляем к temp
        if (isdigit(*it) || *it == '.')         
        {
            temp += *it;
            flag = 0;
        }
        // если встречаем пробел - запихиваем число в стек
        else if (*it == ' ')                    
        {
            if (flag == 0)
            {
                numbers.push(atof(temp.c_str()));
                temp.clear();
            }
        }
        // выполнение арифметических действий
        else 
        {
            flag = 0;
            double temp1 = numbers.top();
            numbers.pop();
            double temp2 = numbers.top();
            numbers.pop();
            double result = 0;
            
            // "*" - умножение
            if (*it == '*') 
                result += (temp2 * temp1);
            // "/" - деление
            else if (*it == '/') 
            {
                if (temp1 == 0) // проверка деления на ноль
                    throw runtime_error("attempted to divide by zero\n");
                else
                    result += (temp2 / temp1);
            }
            // "+" - сумма
            else if (*it == '+') 
                result += (temp2 + temp1);
            // "-" - вычитание
            else if (*it == '-') 
                result += (temp2 - temp1);

            numbers.push(result);
            flag = 1;
            //++it; // пропускаем пробел, идущий после знака операции в строке end
        }
    }
    return numbers.top();
}

void myserver::startServer()
{
    if(this->listen(QHostAddress::Any, 5555))
    {
        qDebug() << "Listening";
    }
    else {
        qDebug() << "Not listening";
    }
}

void myserver::incomingConnection(qintptr socketDescriptor)
{
    socket = new QTcpSocket(this);
    socket->setSocketDescriptor(socketDescriptor);
    connect(socket, SIGNAL(readyRead()), this, SLOT(sockReady()));
    connect(socket, SIGNAL(disconnected()), this,SLOT(sockDisk()));

    qDebug() << socketDescriptor << "Client connected";

    // "key": "value" or value
    // "type": "connect", "status": "yes"
    socket->write("{\"type\": \"connect\", \"status\":\"yes\"}");
    qDebug() << "Send client connect status - YES";
}

void myserver::sockReady()
{
    Data = socket->readAll();
    qDebug() << "Data: " << Data;

    doc = QJsonDocument::fromJson(Data, &docError);

    // {"command" : "calculate", "data": "line_calculate"}
    if (docError.errorString().toInt() == QJsonParseError::NoError)
    {
        if ((doc.object().value("command").toString() == "calculate"))
        {
            data_calculate = doc.object().value("value").toString().toStdString();
            string begin   = data_calculate;
            string rpn_st;

            QString result;
            double res;
            try
            {
                rpn_st  = rpn(begin);
                res     = calculate(rpn_st);
                cout << "\n\n" << rpn_st << " = " << res << endl;
                result  = "{\"command\":\"calculate\", \"result\":\"" + QString::number(res) + "\"}";

            }
            catch (runtime_error& ex)
            {
                rpn_st = ex.what();
                cout << "\nException occured :  " << ex.what() << endl;
                result = "{\"command\":\"calculate\", \"result\":\"" + QString::fromStdString(rpn_st) + "\"}";
            }

            qDebug() << endl << result << endl;
            QByteArray res_json;
            res_json.append(result);
            socket->write(res_json);
            socket->waitForBytesWritten(500);

        }
    }
}

void myserver::sockDisk()
{
    qDebug() << "Disconnect";
    socket->deleteLater();
}
