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
    // Открываем диалог выбора аудиофайла
    QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("Open Audio File"),
        QDir::homePath(),
        tr("Audio Files (*.mp3 *.wav *.ogg *.flac)")
        );

    if (!filePath.isEmpty()) {
        // Выводим путь к файлу в консоль (можно заменить на нужную вам логику)
        qDebug() << "Selected audio file:" << filePath;
    }
}
