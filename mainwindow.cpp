#include "mainwindow.h"
#include <QFileDialog>
#include <QDebug>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , player(nullptr)
    , audioOutput(nullptr)
{
    ui->setupUi(this);


    player = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);
    player->setAudioOutput(audioOutput);


    audioOutput->setVolume(0.5);


    ui->playButton->setEnabled(false);
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
        player->setSource(QUrl::fromLocalFile(filePath));
        qDebug() << "Selected audio file:" << filePath;


        ui->playButton->setEnabled(true);
        ui->playButton->setText("Play");
    }
}

void MainWindow::on_playButton_clicked()
{
    if (player->playbackState() == QMediaPlayer::PlayingState) {

        player->pause();
        ui->playButton->setText("Play");
    } else {

        player->play();
        ui->playButton->setText("Pause");
    }
}
