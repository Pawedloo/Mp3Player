#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_openButton_clicked()
{  
    QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("Open Audio File"),
        QDir::homePath(),
        tr("Audio Files (*.mp3 *.wav *.ogg *.flac)")
        );

    if (!filePath.isEmpty()) { 
        qDebug() << "Selected audio file:" << filePath;
    }
}
