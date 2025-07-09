#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDebug>
#include <QMessageBox>
#include <QDir>
#include <QTime>
#include <QInputDialog>
#include <QSettings>
#include <QRandomGenerator>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , player(new QMediaPlayer(this))
    , audioOutput(new QAudioOutput(this))
    , musicCollection(new MusicCollection(this))
    , currentTrackIndex(-1)
    , currentCollection("")
    , shuffleMode(false)
    , playbackSpeed(1.0f)
{
    ui->setupUi(this);


    audioOutput->setVolume(0.5f);
    ui->volumeSlider->setValue(50);
    player->setAudioOutput(audioOutput);
    player->setPlaybackRate(playbackSpeed);


    updatePlayerControls();


    connect(ui->openFileButton, &QPushButton::clicked, this, &MainWindow::on_openButton_clicked);
    connect(ui->playButton, &QPushButton::clicked, this, &MainWindow::on_playButton_clicked);
    connect(ui->pauseButton, &QPushButton::clicked, this, &MainWindow::on_pauseButton_clicked);
    connect(ui->stopButton, &QPushButton::clicked, this, &MainWindow::on_stopButton_clicked);
    connect(ui->nextButton, &QPushButton::clicked, this, &MainWindow::on_nextButton_clicked);
    connect(ui->prevButton, &QPushButton::clicked, this, &MainWindow::on_prevButton_clicked);
    connect(ui->shuffleButton, &QPushButton::clicked, this, &MainWindow::on_shuffleButton_clicked);
    connect(ui->speedButton, &QPushButton::clicked, this, &MainWindow::on_speedButton_clicked);
    connect(ui->openFolderButton, &QPushButton::clicked, this, &MainWindow::on_openFolderButton_clicked);
    connect(ui->createPlayListButton, &QPushButton::clicked, this, &MainWindow::on_createCollectionButton_clicked);
    connect(ui->renamePlayListButton, &QPushButton::clicked, this, &MainWindow::on_renameCollectionButton_clicked);
    connect(ui->removePlayListButton, &QPushButton::clicked, this, &MainWindow::on_removeCollectionButton_clicked);
    connect(ui->addToPlayListButton, &QPushButton::clicked, this, &MainWindow::on_addToCollectionButton_clicked);
    connect(ui->removeFromPlayListButton, &QPushButton::clicked, this, &MainWindow::on_removeFromCollectionButton_clicked);
    connect(ui->collectionTracksList, &QListWidget::itemDoubleClicked,
            this, &MainWindow::on_collectionTracksList_itemDoubleClicked);

    connect(ui->volumeSlider, &QSlider::valueChanged, this, &MainWindow::on_volumeSlider_valueChanged);
    connect(ui->speedSlider, &QSlider::valueChanged, this, &MainWindow::on_speedSlider_valueChanged);
    connect(ui->progressSlider, &QSlider::sliderMoved, this, &MainWindow::on_positionSlider_sliderMoved);
    connect(player, &QMediaPlayer::durationChanged, this, [this](qint64 duration) {
        ui->progressSlider->setMaximum(static_cast<int>(duration / 1000));
        updateTimeDisplay(duration);
    });
    connect(player, &QMediaPlayer::positionChanged, this, &MainWindow::on_positionChanged);
    connect(player, &QMediaPlayer::mediaStatusChanged, this, &MainWindow::on_mediaStatusChanged);
    connect(player, &QMediaPlayer::playbackStateChanged, this, [this]() {
        updatePlayerControls();
    });
    connect(ui->searchEdit, &QLineEdit::textChanged, this, &MainWindow::on_searchTextChanged);
    connect(ui->trackList, &QListWidget::itemDoubleClicked, this, &MainWindow::on_trackList_itemDoubleClicked);
    connect(ui->playlistsList, &QListWidget::currentTextChanged, this, &MainWindow::on_collectionsList_currentTextChanged);
    connect(ui->collectionTracksList, &QListWidget::itemDoubleClicked, this, &MainWindow::on_collectionTracksList_itemDoubleClicked);


    loadTrackList();
    updateCollectionsList();
}

MainWindow::~MainWindow()
{
    saveTrackList();
    delete player;
    delete audioOutput;
    delete ui;
}

void MainWindow::on_shuffleButton_clicked()
{
    shuffleMode = !shuffleMode;
    ui->shuffleButton->setStyleSheet(shuffleMode ? "background-color: #4CAF50;" : "");
    updatePlayerControls();
}

void MainWindow::on_speedButton_clicked()
{
    playbackSpeed = 1.0f;
    ui->speedSlider->setValue(100);
    applyPlaybackSpeed();
}

void MainWindow::on_speedSlider_valueChanged(int value)
{
    playbackSpeed = value / 100.0f;
    applyPlaybackSpeed();
}

void MainWindow::applyPlaybackSpeed()
{
    player->setPlaybackRate(playbackSpeed);
    ui->speedLabel->setText(QString("Speed: %1x").arg(playbackSpeed, 0, 'f', 1));
}

void MainWindow::playRandomTrack()
{
    if (playlist.isEmpty()) return;

    int newIndex;
    do {
        newIndex = QRandomGenerator::global()->bounded(playlist.size());
    } while (newIndex == currentTrackIndex && playlist.size() > 1);

    playTrack(newIndex);
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
        if (!allTracks.contains(filePath)) {
            allTracks.append(filePath);
            ui->trackList->addItem(QFileInfo(filePath).fileName());
            saveTrackList();
        }

        playlist = allTracks;
        currentTrackIndex = allTracks.indexOf(filePath);
        currentFilePath = filePath;
        playTrack(currentTrackIndex);
    }
}

void MainWindow::on_playButton_clicked()
{
    if (playlist.isEmpty()) return;

    if (player->playbackState() == QMediaPlayer::PlayingState) {
        player->pause();
    } else {
        if (player->mediaStatus() == QMediaPlayer::NoMedia ||
            player->playbackState() == QMediaPlayer::StoppedState) {
            playTrack(currentTrackIndex);
        } else {
            player->play();
        }
    }
}

void MainWindow::on_pauseButton_clicked()
{
    if (player->playbackState() == QMediaPlayer::PlayingState) {
        player->pause();
    }
}

void MainWindow::on_stopButton_clicked()
{
    player->stop();
    ui->progressSlider->setValue(0);
    ui->currentTimeLabel->setText("00:00");
    updatePlayerControls();
}

void MainWindow::on_volumeSlider_valueChanged(int value)
{
    float volume = qBound(0.0f, value / 100.0f, 1.0f);
    audioOutput->setVolume(volume);
}

void MainWindow::on_positionSlider_sliderMoved(int position)
{
    player->setPosition(position * 1000);
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

void MainWindow::on_collectionTracksList_itemDoubleClicked(QListWidgetItem *item)
{
    if (!currentCollection.isEmpty()) {
        QStringList tracks = musicCollection->getTracksInCollection(currentCollection);
        int index = ui->collectionTracksList->row(item);

        if (index >= 0 && index < tracks.size()) {
            QString trackPath = tracks.at(index);
            currentTrackIndex = allTracks.indexOf(trackPath);
            if (currentTrackIndex >= 0) {
                currentFilePath = trackPath;
                player->setSource(QUrl::fromLocalFile(currentFilePath));
                player->play();
                updateTrackInfo();
                updatePlayerControls();
            }
        }
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

    for (const QString &file : audioFiles) {
        QString filePath = directory.filePath(file);
        if (!allTracks.contains(filePath)) {
            allTracks.append(filePath);
            ui->trackList->addItem(file);
        }
    }

    playlist = allTracks;
    currentTrackIndex = 0;
    currentFilePath = playlist.first();
    playTrack(currentTrackIndex);
    saveTrackList();
}

void MainWindow::on_nextButton_clicked()
{
    if (playlist.isEmpty()) return;

    if (shuffleMode) {
        playRandomTrack();
    } else if (currentTrackIndex < playlist.size() - 1) {
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

void MainWindow::on_createCollectionButton_clicked()
{
    bool ok;
    QString name = QInputDialog::getText(this, "Create Collection",
                                         "Enter collection name:",
                                         QLineEdit::Normal,
                                         "", &ok);
    if (ok && !name.isEmpty()) {
        musicCollection->createCollection(name);
        updateCollectionsList();
    }
}

void MainWindow::on_renameCollectionButton_clicked()
{
    if (currentCollection.isEmpty()) return;

    bool ok;
    QString newName = QInputDialog::getText(this, "Rename Collection",
                                            "Enter new name:",
                                            QLineEdit::Normal,
                                            currentCollection, &ok);
    if (ok && !newName.isEmpty()) {
        musicCollection->renameCollection(currentCollection, newName);
        currentCollection = newName;
        updateCollectionsList();
        updateCurrentCollectionTracks();
    }
}

void MainWindow::on_removeCollectionButton_clicked()
{
    if (currentCollection.isEmpty()) return;

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Remove Collection",
                                  QString("Are you sure you want to remove collection '%1'?").arg(currentCollection),
                                  QMessageBox::Yes|QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        musicCollection->removeCollection(currentCollection);
        currentCollection = "";
        updateCollectionsList();
        updateCurrentCollectionTracks();
    }
}

void MainWindow::on_addToCollectionButton_clicked()
{
    if (currentCollection.isEmpty() || currentTrackIndex < 0) return;

    QString trackPath = playlist.at(currentTrackIndex);
    musicCollection->addTrackToCollection(currentCollection, trackPath);
    updateCurrentCollectionTracks();
}

void MainWindow::on_removeFromCollectionButton_clicked()
{
    if (currentCollection.isEmpty() || currentTrackIndex < 0) return;

    QString trackPath = playlist.at(currentTrackIndex);
    musicCollection->removeTrackFromCollection(currentCollection, trackPath);
    updateCurrentCollectionTracks();
}

void MainWindow::on_collectionsList_currentTextChanged(const QString &currentText)
{
    currentCollection = currentText;
    updateCurrentCollectionTracks();
    updatePlayerControls();
}

void MainWindow::on_mediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    if (status == QMediaPlayer::EndOfMedia) {
        if (currentTrackIndex < playlist.size() - 1) {
            on_nextButton_clicked();
        } else {
            player->stop();
            updatePlayerControls();
        }
    }
}

void MainWindow::on_positionChanged(qint64 position)
{
    if (!player->isPlaying()) return;

    ui->progressSlider->setValue(static_cast<int>(position / 1000));
    updateTimeDisplay(position);
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

void MainWindow::playTrack(int index)
{
    if (index >= 0 && index < playlist.size()) {
        currentTrackIndex = index;
        currentFilePath = playlist.at(index);
        player->setSource(QUrl::fromLocalFile(currentFilePath));
        player->play();
        updateTrackInfo();
        ui->trackList->setCurrentRow(index);
        updatePlayerControls();
    }
}

void MainWindow::updateTimeDisplay(qint64 position)
{
    QTime currentTime(0, 0, 0);
    currentTime = currentTime.addMSecs(position);

    QTime totalTime(0, 0, 0);
    totalTime = totalTime.addMSecs(player->duration());

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

void MainWindow::updatePlayerControls()
{
    bool hasTracks = !playlist.isEmpty();
    bool isPlaying = player->playbackState() == QMediaPlayer::PlayingState;
    bool isPaused = player->playbackState() == QMediaPlayer::PausedState;
    bool hasCurrentTrack = currentTrackIndex >= 0 && currentTrackIndex < playlist.size();

    ui->playButton->setEnabled(hasTracks);
    ui->playButton->setText(isPlaying ? "Pause" : "Play");
    ui->pauseButton->setEnabled(isPlaying);
    ui->stopButton->setEnabled(isPlaying || isPaused);
    ui->nextButton->setEnabled(hasTracks && (shuffleMode || (hasCurrentTrack && currentTrackIndex < playlist.size() - 1)));
    ui->prevButton->setEnabled(hasTracks && hasCurrentTrack && currentTrackIndex > 0);
    ui->addToPlayListButton->setEnabled(hasCurrentTrack && !currentCollection.isEmpty());
    ui->removeFromPlayListButton->setEnabled(hasCurrentTrack && !currentCollection.isEmpty());
    ui->renamePlayListButton->setEnabled(!currentCollection.isEmpty());
    ui->removePlayListButton->setEnabled(!currentCollection.isEmpty());
    ui->shuffleButton->setEnabled(hasTracks);
    ui->speedButton->setEnabled(hasTracks);
    ui->speedSlider->setEnabled(hasTracks);
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
    updatePlayerControls();
}

void MainWindow::updateCollectionsList()
{
    ui->playlistsList->clear();
    ui->playlistsList->addItems(musicCollection->getCollectionNames());
}

void MainWindow::updateCurrentCollectionTracks()
{
    ui->collectionTracksList->clear();
    if (currentCollection.isEmpty()) return;

    QStringList tracks = musicCollection->getTracksInCollection(currentCollection);
    for (const QString &trackPath : tracks) {
        ui->collectionTracksList->addItem(QFileInfo(trackPath).fileName());
    }
}

void MainWindow::saveTrackList()
{
    QSettings settings;
    settings.beginGroup("TrackList");
    settings.setValue("tracks", allTracks);
    settings.endGroup();
}

void MainWindow::loadTrackList()
{
    QSettings settings;
    settings.beginGroup("TrackList");
    allTracks = settings.value("tracks").toStringList();
    settings.endGroup();

    ui->trackList->clear();
    for (const QString &filePath : allTracks) {
        ui->trackList->addItem(QFileInfo(filePath).fileName());
    }

    if (!allTracks.isEmpty()) {
        playlist = allTracks;
        updatePlayerControls();
    }
}
