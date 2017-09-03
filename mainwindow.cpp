#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <signal.h> // definição dos sinais de interrupções
#include <unistd.h> // para: pegar numero de processadores
#include <sched.h> // para escolher CPU

int selected_pid = 0;
QString selected_status = "";
QList<QString> lista_processos;
bool mudando_core = false;

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);
    this->timer = new QTimer();

    connect(ui->tabela_processos, SIGNAL(itemSelectionChanged()),
                this, SLOT(get_mudanca_selecao_lista()));

    // add lista com numero de cores para comboBox
    int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    QStringList cores_list;
    for (int i = 1; i <= num_cores; ++i) {
       cores_list << QString::number(i);
    }
    qDebug() << cores_list;
    ui->comboBox->addItems(cores_list);

    connect(this->timer, SIGNAL(timeout()), this, SLOT(executaTimer()));
    this->timer->setInterval(1000);
    this->timer->start();
    this->executaTimer();

    ui->tabela_processos->setColumnWidth(0, 60);
    ui->tabela_processos->setColumnWidth(1, 50);
    ui->tabela_processos->setColumnWidth(2, 60);
    ui->tabela_processos->setColumnWidth(3, 80);
    ui->tabela_processos->setColumnWidth(4, 50);
    ui->tabela_processos->setColumnWidth(5, 40);
}

void::MainWindow::get_mudanca_selecao_lista(){

    int index_changed = ui->tabela_processos->currentRow();
    qDebug() << "pid changed: " << ui->tabela_processos->item(index_changed,1)->text().toInt();
    selected_pid = ui->tabela_processos->item(index_changed,1)->text().toInt();
    selected_status = ui->tabela_processos->item(index_changed,4)->text();

    // atualiza o core usado no combo box
    if(selected_pid > 0 && !mudando_core){
        int core = ui->tabela_processos->item(index_changed,5)->text().toInt();
        ui->comboBox->setCurrentIndex(core-1);
    }

    if(selected_status != "T")
        ui->pushButton_2->setText("Pausar");
    else
        ui->pushButton_2->setText("Continuar");
}

void MainWindow::executaTimer(){
    QString comando = "bash -c";
    comando += " \"ps -axo user,pid,pcpu,pmem,stat,psr,args";
    if(!ui->filtro->text().isEmpty())
        comando += " | grep '" + ui->filtro->text() + "'";

    comando += "\"";

    QProcess process;
    process.start(comando);
    process.waitForFinished(-1); // will wait forever until finished

    QString stdout = process.readAllStandardOutput();

    QList<QString> linhas = stdout.split("\n");
    if(linhas.length())
        linhas.removeLast(); // ultima linha sempre fica vazia

    // remove o cabeçalho e o proprio processo
    if(ui->filtro->text().isEmpty()){
        linhas.removeFirst();
        if(linhas.length() > 2){
            linhas.removeLast();
        }
    } else {
        if(linhas.length() > 2){
            linhas.removeLast();
//            linhas.removeLast();
        }
    }


    ui->tabela_processos->clearContents();
    ui->tabela_processos->setRowCount(linhas.length());

    for(int i = 0; i < linhas.length(); i++){
         QList<QString> colunas = linhas[i].split(" ");
         colunas.removeAll("");
        //user,pid,pcpu,pmem,stat,psr,comm
         QString user = colunas[0];
         QString pid = colunas[1];
         QString cpu = colunas[2];
         QString memoria = colunas[3];
         QString status = colunas[4].at(0);
         QString core = QString::number(colunas[5].toInt() + 1);
         QString processo = colunas[6];

         // concatena os parametros do processo
         for(int j = 7; j < colunas.length();j++){
             processo += " " + colunas[j];
         }

         ui->tabela_processos->setItem(i, 0, new QTableWidgetItem(user));
         ui->tabela_processos->setItem(i, 1, new QTableWidgetItem(pid));
         ui->tabela_processos->setItem(i, 2, new QTableWidgetItem(cpu));
         ui->tabela_processos->setItem(i, 3, new QTableWidgetItem(memoria));
         ui->tabela_processos->setItem(i, 4, new QTableWidgetItem(status));
         ui->tabela_processos->setItem(i, 5, new QTableWidgetItem(core));
         ui->tabela_processos->setItem(i, 6, new QTableWidgetItem(processo));

         if(selected_pid == pid.toInt()){
            ui->tabela_processos->setAutoScroll(false);
            ui->tabela_processos->selectRow(i);
            ui->tabela_processos->setAutoScroll(true);
         }
    }

    ui->tabela_processos->setHorizontalHeaderLabels(QString("Usuario;PID;CPU (%);Memoria (%);Status;Core;Processo").split(";"));
}

void MainWindow::on_altera_core_button_clicked(){
    if(selected_pid == 0){
        qDebug() << "nenhum processo selecionado";
    } else {
        QString core_selecionado = ui->comboBox->currentText();
        qDebug() << "muda processo " << selected_pid << " para core " << core_selecionado;

        cpu_set_t mask;

        CPU_ZERO( &mask );

        CPU_SET( core_selecionado.toInt()-1, &mask );

        /* sched_setaffinity returns 0 in success */
        if( sched_setaffinity(selected_pid, sizeof(mask), &mask ) == -1 ){
            qDebug() << "CPU Affinity não funcionou...";
        }
        qDebug() << "Rodando no core " << core_selecionado;
        mudando_core = false;
    }

}

void MainWindow::on_pushButton_clicked() {

    if(selected_pid == 0){
        qDebug() << "nenhum processo selecionado";
    } else {
        kill(selected_pid, SIGKILL);
        qDebug() << "matou processo " << selected_pid;
    }

}

void MainWindow::on_pushButton_2_clicked() {
    if(selected_pid == 0){
        qDebug() << "nenhum processo selecionado";
    } else {
        if(selected_status != "T"){
            kill(selected_pid, SIGSTOP);
            selected_status = "T";
            ui->pushButton_2->setText("Continuar");
        } else {
            kill(selected_pid, SIGCONT);
            selected_status = "";
            ui->pushButton_2->setText("Pausar");
        }
    }
}

void MainWindow::on_comboBox_activated(int index)
{
    mudando_core = true;
    qDebug() << "muda combobox para " << index;
}

MainWindow::~MainWindow(){
    delete ui;
}

