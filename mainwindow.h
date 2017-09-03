#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QDebug>
#include <QProcess>
#include <QList>
#include <QTableWidgetItem>
#include <QItemSelection>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void executaTimer();
    //void get_processo_selecionado(QTableWidgetItem* item);
    void get_mudanca_selecao_lista();
    void on_altera_core_button_clicked();

    void on_pushButton_clicked();
    void on_pushButton_2_clicked();

private:
    Ui::MainWindow *ui;
    QTimer * timer;
};

#endif // MAINWINDOW_H
