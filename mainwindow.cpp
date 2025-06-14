#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include <QDir>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , player(nullptr)
    , audioOutput(nullptr)
{
    ui->setupUi(this);

    player = new QMediaPlayer;
    audioOutput = new QAudioOutput();
    audioOutput -> setVolume(100);

    player->setAudioOutput(audioOutput);
    ui->playButton->setEnabled(false);
    connect(player, &QMediaPlayer::durationChanged, this, [this](int duration){
        qDebug()<<duration;
        int durationInSeconds=duration/1000;
        int durationMinutes = durationInSeconds/60;
        int durationSeconds=durationInSeconds - durationMinutes * 60;
        ui->horizontalSlider_2->setMaximum(durationInSeconds);
        ui->label->setText(QString::number(durationMinutes) + ":" + QString::number(durationSeconds));
    });

}

MainWindow::~MainWindow()
{
    delete player;
    delete audioOutput;
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
        currentFilePath = filePath;
        qDebug() << "Selected audio file:" << filePath;

        ui->playButton->setEnabled(true);
        ui->playButton->setText("Play");
    }
}
void MainWindow::on_playButton_clicked()
{
    if (player->isPlaying()) {
        player->pause();
        ui->playButton->setText("Play");
    } else {
        player->setSource(QUrl::fromLocalFile(currentFilePath));
        player->play();
        ui->playButton->setText("Pause");
    }
}

void MainWindow::on_horizontalSlider_valueChanged(int value)
{
    audioOutput->setVolume(value);
}

