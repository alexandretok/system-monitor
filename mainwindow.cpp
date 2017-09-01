#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    this->timer = new QTimer();

    connect(this->timer, SIGNAL(timeout()), this, SLOT(executaTimer()));
    this->timer->setInterval(1000);
    this->timer->start();
}

void MainWindow::executaTimer(){
    QString comando = "bash -c";
    comando += " \"ps aux";
    if(!ui->filtro->text().isEmpty())
        comando += " | grep '" + ui->filtro->text() + "'";

    comando += "\"";

    QProcess process;
    process.start(comando);
    process.waitForFinished(-1); // will wait forever until finished

    QString stdout = process.readAllStandardOutput();
    QString stderr = process.readAllStandardError();

//    qDebug() << stdout;
//    qDebug() << stderr;

    QList<QString> linhas = stdout.split("\n");
    linhas.removeLast();

    if(ui->filtro->text().isEmpty()){
        linhas.removeFirst();
        linhas.removeLast();
    }
    else{
        linhas.removeLast();
        linhas.removeLast();
    }

//    qDebug() << linhas;


    ui->tabela_processos->clearContents();
    ui->tabela_processos->setRowCount(0);

    for(int i = 0; i < linhas.length(); i++){
         QList<QString> colunas = linhas[i].split(" ");
         colunas.removeAll("");
//         qDebug() << colunas;
//         USER       PID %CPU %MEM    VSZ   RSS TTY      STAT START   TIME COMMAND
         QString user = colunas[0];
         QString pid = colunas[1];
         QString cpu = colunas[2];
         QString memoria = colunas[3];
         QString status = colunas[7].at(0);
         QString processo = colunas[10];

         for(int j = 11; j < colunas.length();j++){
             processo += " " + colunas[j];
         }


         ui->tabela_processos->insertRow(i);

         ui->tabela_processos->setItem(i,0,new QTableWidgetItem(user));
         ui->tabela_processos->setItem(i,1,new QTableWidgetItem(pid));
         ui->tabela_processos->setItem(i,2,new QTableWidgetItem(cpu));
         ui->tabela_processos->setItem(i,3,new QTableWidgetItem(memoria));
         ui->tabela_processos->setItem(i,4,new QTableWidgetItem(status));
         ui->tabela_processos->setItem(i,5,new QTableWidgetItem(processo));


    }

    ui->tabela_processos->setHorizontalHeaderLabels(QString("Usuario;PID;CPU (%);Memoria (%);Status;Processo").split(";"));

}

MainWindow::~MainWindow()
{
    delete ui;
}
