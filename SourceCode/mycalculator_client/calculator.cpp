#include <QtWidgets>

#include <cmath>

#include "button.h"
#include "calculator.h"

Calculator::Calculator(QWidget *parent)
    : QWidget(parent)
{

    socket = new QTcpSocket(this);
    connect(socket, SIGNAL(readyRead()), this, SLOT(sockReady()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(sockDisc()));
    socket->connectToHost("127.0.0.1", 5555);
    socket->waitForConnected(500);



    display = new QLineEdit("0");
    display->setReadOnly(true);
    display->setAlignment(Qt::AlignRight);
    display->setMaxLength(15);

    QFont font = display->font();
    font.setPointSize(font.pointSize() + 8);
    display->setFont(font);

    for (int i = 0; i < NumDigitButtons; ++i) {
        digitButtons[i] = createButton(QString::number(i), SLOT(digitClicked()));
    }

    Button *pointButton     = createButton(tr("."), SLOT(pointClicked()));
    Button *clearAllButton  = createButton(tr("Clear All"), SLOT(clearAll()));

    Button *divisionButton  = createButton(tr("/"), SLOT(multiplicativeOperatorClicked()));
    Button *timesButton     = createButton(tr("*"), SLOT(multiplicativeOperatorClicked()));
    Button *minusButton     = createButton(tr("-"), SLOT(additiveOperatorClicked()));
    Button *plusButton      = createButton(tr("+"), SLOT(additiveOperatorClicked()));

    Button *equalButton     = createButton(tr("="), SLOT(equalClicked()));

    QGridLayout *mainLayout = new QGridLayout;
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    mainLayout->addWidget(display, 0, 0, 1, 6);


    //mainLayout->addWidget(clearButton, 1, 1, 1, 2);
    mainLayout->addWidget(clearAllButton, 1, 1, 1, 5);

    for (int i = 1; i < NumDigitButtons; ++i) {
        int row    = ((9 - i) / 3) + 2;
        int column = ((i - 1) % 3) + 1;
        mainLayout->addWidget(digitButtons[i], row, column);
    }

    mainLayout->addWidget(digitButtons[0], 5, 1);
    mainLayout->addWidget(pointButton, 5, 2);
    //mainLayout->addWidget(changeSignButton, 5, 3);

    mainLayout->addWidget(divisionButton, 2, 4);
    mainLayout->addWidget(timesButton, 3, 4);
    mainLayout->addWidget(minusButton, 4, 4);
    mainLayout->addWidget(plusButton, 5, 3);

    mainLayout->addWidget(equalButton, 5, 4);
    setLayout(mainLayout);

    setWindowTitle(tr("Calculator"));
}

void Calculator::sockReady()
{

    if(socket->waitForConnected(500))
    {
        socket->waitForReadyRead(500);
        Data = socket->readAll();
        doc  = QJsonDocument::fromJson(Data, &docError);

        if(docError.errorString().toInt() == QJsonParseError::NoError)
        {
            if((doc.object().value("type").toString() == "connect") && (doc.object().value("status").toString() == "yes"))
            {
                qDebug() << "Соединение установлено";
            }
            else if ((doc.object().value("command").toString() == "calculate"))
            {
                line_calculate = doc.object().value("result").toString();
                display->clear();
                display->setText(line_calculate);
                line_calculate.clear();

            }
            else
            {
                qDebug() << "Соединение не установлено";
            }

        }
        else
        {
            qDebug() << "Соединение установлено Ошибка с форматом передачи данных " + docError.errorString();
        }
        Data.clear();
    }

}

void Calculator::sockDisc()
{
    socket->deleteLater();
}

void Calculator::digitClicked()
{
    Button *clickedButton = qobject_cast<Button *>(sender());
    int digitValue = clickedButton->text().toInt();
    if (display->text() == "0" && digitValue == 0.0)
        return;

    line_calculate += QString::number(digitValue);
    display->setText(line_calculate);
}



void Calculator::additiveOperatorClicked()
{
    Button *clickedButton   = qobject_cast<Button *>(sender());
    QString clickedOperator = clickedButton->text();
    line_calculate          += clickedOperator;
    display->setText(line_calculate);
}

void Calculator::multiplicativeOperatorClicked()
{
    Button *clickedButton   = qobject_cast<Button *>(sender());
    QString clickedOperator = clickedButton->text();

    line_calculate += clickedOperator;
    display->setText(line_calculate);

}

void Calculator::equalClicked()
{

    if(socket->isOpen())
    {
        // {"command" : "calculate", "data": "line_calculate"}
        QString data = "{\"command\":\"calculate\", \"value\":\"" + line_calculate + "\"}";
        QByteArray data_json;
        data_json.append(data);
        socket->write(data_json);
        socket->waitForBytesWritten(500);
    }
}

void Calculator::pointClicked()
{

    if (!display->text().contains('.'))
    {
        display->setText(display->text() + tr("."));
        line_calculate += tr(".");
    }
}




void Calculator::clearAll()
{

    display->setText("0");
    line_calculate.clear();
}


Button *Calculator::createButton(const QString &text, const char *member)
{
    Button *button = new Button(text);
    connect(button, SIGNAL(clicked()), this, member);
    return button;
}


