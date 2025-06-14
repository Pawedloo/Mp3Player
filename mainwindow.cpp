#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include <QDir>
#include <QTime>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , player(nullptr)
    , audioOutput(nullptr)
    , currentTrackIndex(-1)
{
    ui->setupUi(this);

    player = new QMediaPlayer(this);
    audioOutput = new QAudioOutput(this);
    audioOutput->setVolume(50);
    ui->volumeSlider->setValue(50);

    player->setAudioOutput(audioOutput);
    ui->playButton->setEnabled(false);
    ui->nextButton->setEnabled(false);
    ui->prevButton->setEnabled(false);

    // Подключаем сигналы
    connect(player, &QMediaPlayer::durationChanged, this, [this](qint64 duration) {
        ui->positionSlider->setMaximum(static_cast<int>(duration / 1000));
        updateTimeDisplay(duration);
    });

    connect(player, &QMediaPlayer::positionChanged, this, &MainWindow::on_positionChanged);
    connect(player, &QMediaPlayer::mediaStatusChanged, this, &MainWindow::on_mediaStatusChanged);
    connect(ui->searchEdit, &QLineEdit::textChanged, this, &MainWindow::on_searchTextChanged);
    connect(ui->trackList, &QListWidget::itemDoubleClicked, this, &MainWindow::on_trackList_itemDoubleClicked);
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
        playlist.clear();
        allTracks.clear();
        playlist.append(filePath);
        allTracks.append(filePath);
        currentTrackIndex = 0;
        currentFilePath = filePath;

        ui->playButton->setEnabled(true);
        ui->playButton->setText("Play");
        ui->nextButton->setEnabled(false);
        ui->prevButton->setEnabled(false);
        updateTrackInfo();

        ui->trackList->clear();
        ui->trackList->addItem(QFileInfo(filePath).fileName());
    }
}

void MainWindow::on_openFolderButton_clicked()
{
    QString folderPath = QFileDialog::getExistingDirectory(
        this,
        tr("Open Music Folder"),
        QDir::homePath(),
        QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks
        );

    if (!folderPath.isEmpty()) {
        loadFolder(folderPath);
    }
}

void MainWindow::loadFolder(const QString &folderPath)
{
    QDir directory(folderPath);
    QStringList audioFiles = directory.entryList(
        {"*.mp3", "*.wav", "*.ogg", "*.flac"},
        QDir::Files,
        QDir::Name
        );

    if (audioFiles.isEmpty()) {
        QMessageBox::information(this, "No Audio Files", "No supported audio files found in the selected folder.");
        return;
    }

    playlist.clear();
    allTracks.clear();
    ui->trackList->clear();

    for (const QString &file : audioFiles) {
        QString filePath = directory.filePath(file);
        playlist.append(filePath);
        allTracks.append(filePath);
        ui->trackList->addItem(file);
    }

    currentTrackIndex = 0;
    currentFilePath = playlist.first();
    ui->playButton->setEnabled(true);
    ui->playButton->setText("Play");
    ui->nextButton->setEnabled(playlist.size() > 1);
    ui->prevButton->setEnabled(false);
    updateTrackInfo();
}

void MainWindow::filterTracks(const QString &filter)
{
    ui->trackList->clear();

    if (filter.isEmpty()) {
        for (const QString &filePath : allTracks) {
            ui->trackList->addItem(QFileInfo(filePath).fileName());
        }
        playlist = allTracks;
    } else {
        playlist.clear();
        for (const QString &filePath : allTracks) {
            QString fileName = QFileInfo(filePath).fileName();
            if (fileName.contains(filter, Qt::CaseInsensitive)) {
                ui->trackList->addItem(fileName);
                playlist.append(filePath);
            }
        }
    }


    if (currentTrackIndex >= playlist.size()) {
        currentTrackIndex = -1;
    }
    ui->playButton->setEnabled(!playlist.isEmpty());
    ui->nextButton->setEnabled(playlist.size() > 1);
    ui->prevButton->setEnabled(false);
}

void MainWindow::playTrack(int index)
{
    if (index >= 0 && index < playlist.size()) {
        currentTrackIndex = index;
        currentFilePath = playlist.at(index);
        player->setSource(QUrl::fromLocalFile(currentFilePath));
        player->play();
        ui->playButton->setText("Pause");
        ui->nextButton->setEnabled(index < playlist.size() - 1);
        ui->prevButton->setEnabled(index > 0);
        updateTrackInfo();

        ui->trackList->setCurrentRow(index);
    }
}

void MainWindow::on_playButton_clicked()
{
    if (playlist.isEmpty()) return;

    if (player->isPlaying()) {
        player->pause();
        ui->playButton->setText("Play");
    } else {
        if (player->mediaStatus() == QMediaPlayer::NoMedia ||
            player->playbackState() == QMediaPlayer::StoppedState) {
            playTrack(currentTrackIndex);
        } else {
            player->play();
            ui->playButton->setText("Pause");
        }
    }
}

void MainWindow::on_volumeSlider_valueChanged(int value)
{
    float volume = qBound(0.0f, value / 100.0f, 1.0f);
    audioOutput->setVolume(volume);
}

void MainWindow::on_mediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    if (status == QMediaPlayer::EndOfMedia) {
        if (currentTrackIndex < playlist.size() - 1) {
            on_nextButton_clicked();
        }
    }
}

void MainWindow::on_positionChanged(qint64 position)
{
    if (!player->isPlaying()) return;

    ui->positionSlider->setValue(static_cast<int>(position / 1000));
    updateTimeDisplay(position);
}

void MainWindow::on_nextButton_clicked()
{
    if (currentTrackIndex < playlist.size() - 1) {
        playTrack(currentTrackIndex + 1);
    }
}

void MainWindow::on_prevButton_clicked()
{
    if (player->position() > 3000) {
        player->setPosition(0);
    } else if (currentTrackIndex > 0) {
        playTrack(currentTrackIndex - 1);
    }
}

void MainWindow::on_positionSlider_sliderMoved(int position)
{
    player->setPosition(position * 1000);
}

void MainWindow::on_searchTextChanged(const QString &text)
{
    filterTracks(text);
}

void MainWindow::on_trackList_itemDoubleClicked(QListWidgetItem *item)
{
    int index = ui->trackList->row(item);
    if (index >= 0 && index < playlist.size()) {
        playTrack(index);
    }
}

void MainWindow::updateTimeDisplay(qint64 position)
{
    QTime currentTime((position / 3600000) % 60,
                      (position / 60000) % 60,
                      (position / 1000) % 60);
    QTime totalTime((player->duration() / 3600000) % 60,
                    (player->duration() / 60000) % 60,
                    (player->duration() / 1000) % 60);

    QString timeFormat = "mm:ss";
    if (totalTime.hour() > 0) {
        timeFormat = "hh:mm:ss";
    }

    ui->currentTimeLabel->setText(currentTime.toString(timeFormat));
    ui->totalTimeLabel->setText(totalTime.toString(timeFormat));
}

void MainWindow::updateTrackInfo()
{
    if (!playlist.isEmpty()) {
        QFileInfo fileInfo(currentFilePath);
        ui->trackInfoLabel->setText(QString("Now playing: %1").arg(fileInfo.fileName()));
    } else {
        ui->trackInfoLabel->setText("No track selected");
    }
}
