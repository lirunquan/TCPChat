#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::logOutput(QString log)
{
    QDateTime date(QDateTime::currentDateTime());
    QTime time(QTime::currentTime());
    QString current = QString("%1 %2 : ").arg(date.toString("dd.MM.yyyy")).arg(time.toString("hh:mm:ss"));
    QFile logFile("log.txt");
    if(logFile.open(QIODevice::WriteOnly|QIODevice::Append)){
        QTextStream output(&logFile);
        output << current << log << "\n";
    }
    logFile.close();
}
