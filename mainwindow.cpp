#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QInputDialog>
#include <QSettings>
#include <QRandomGenerator>
#include <QTime>
#include <QIcon>
#include <QPainter>
#include <QSequentialAnimationGroup>
#include <QTableWidgetItem>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QTableWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),

    player(new QMediaPlayer(this)),
    audioOutput(new QAudioOutput(this)),
    musicCollection(new MusicCollection(this)),
    m_playbackTimer(new QTimer(this)),
    currentTrackIndex(-1),
    currentCollection(""),
    shuffleMode(false),
    playbackSpeed(1.0f),
    isSeeking(false),
    isFullscreen(false)
{
    ui->setupUi(this);


    setWindowFlags(Qt::FramelessWindowHint);
    resize(1000, 650);

    initUI();
    initConnections();
    applyStyles();
    setupAnimations();

    audioOutput->setVolume(0.7);
    player->setAudioOutput(audioOutput);
    player->setPlaybackRate(playbackSpeed);

    loadTrackList();
    updateCollectionsList();
    updatePlayerControls();
    m_playbackTimer->setInterval(1000);
    connect(m_playbackTimer, &QTimer::timeout,
            this, &MainWindow::updatePlaybackStatistics);
}




void MainWindow::setupAnimations()
{
    //Анимации кнопочек :)
    QList<QPushButton*> buttons = findChildren<QPushButton*>();
    foreach (QPushButton *btn, buttons) {
        if (btn->objectName() != "minimizeButton" &&
            btn->objectName() != "fullscreenButton" &&
            btn->objectName() != "closeButton") {

            QGraphicsOpacityEffect *effect = new QGraphicsOpacityEffect(btn);
            btn->setGraphicsEffect(effect);

            QPropertyAnimation *anim = new QPropertyAnimation(effect, "opacity", this);
            anim->setDuration(200);
            anim->setStartValue(0.8);
            anim->setEndValue(1.0);

            connect(btn, &QPushButton::pressed, [anim]() {
                anim->setDirection(QPropertyAnimation::Backward);
                anim->start();
            });

            connect(btn, &QPushButton::released, [anim]() {
                anim->setDirection(QPropertyAnimation::Forward);
                anim->start();
            });
        }
    }

    // Анимации списков
    QGraphicsOpacityEffect *listEffect = new QGraphicsOpacityEffect(ui->trackList);
    ui->trackList->setGraphicsEffect(listEffect);

    QPropertyAnimation *listAnim = new QPropertyAnimation(listEffect, "opacity", this);
    listAnim->setDuration(500);
    listAnim->setStartValue(0.0);
    listAnim->setEndValue(1.0);
    listAnim->start();
}
   //настройка и покраска виджетов
void MainWindow::initConnections()
{
    connect(ui->removeTrackButton, &QPushButton::clicked, this, &MainWindow::removeTrack);
    connect(ui->renameTrackButton, &QPushButton::clicked, this, &MainWindow::renameTrack);
    connect(ui->playButton, &QPushButton::clicked, this, &MainWindow::togglePlayPause);
    connect(ui->pauseButton, &QPushButton::clicked, this, &MainWindow::togglePlayPause);
    connect(ui->stopButton, &QPushButton::clicked, this, &MainWindow::stopPlayback);
    connect(ui->nextButton, &QPushButton::clicked, this, &MainWindow::playNextTrack);
    connect(ui->prevButton, &QPushButton::clicked, this, &MainWindow::playPreviousTrack);
    connect(ui->shuffleButton, &QPushButton::clicked, this, &MainWindow::toggleShuffle);
    connect(ui->speedButton, &QPushButton::clicked, this, &MainWindow::resetSpeed);

    connect(ui->minimizeButton, &QPushButton::clicked, this, &MainWindow::minimizeWindow);
    connect(ui->fullscreenButton, &QPushButton::clicked, this, &MainWindow::toggleFullscreen);
    connect(ui->closeButton, &QPushButton::clicked, this, &MainWindow::closeWindow);

    connect(ui->volumeSlider, &QSlider::valueChanged, this, &MainWindow::handleVolumeChange);
    connect(ui->speedSlider, &QSlider::valueChanged, this, &MainWindow::handleSpeedChange);
    connect(ui->progressSlider, &QSlider::sliderMoved, this, &MainWindow::seekTrack);

    connect(player, &QMediaPlayer::positionChanged, this, &MainWindow::updatePlaybackPosition);
    connect(player, &QMediaPlayer::durationChanged, this, [this](qint64 duration) {
        ui->progressSlider->setMaximum(static_cast<int>(duration / 1000));
    });
    connect(player, &QMediaPlayer::mediaStatusChanged, this, &MainWindow::handleMediaStatusChanged);

    connect(ui->openFileButton, &QPushButton::clicked, this, &MainWindow::openFile);
    connect(ui->openFolderButton, &QPushButton::clicked, this, &MainWindow::openFolder);
    connect(ui->createPlayListButton, &QPushButton::clicked, this, &MainWindow::createCollection);
    connect(ui->addToPlayListButton, &QPushButton::clicked, this, &MainWindow::addToCollection);
    connect(ui->renamePlayListButton, &QPushButton::clicked, this, &MainWindow::renameCollection);
    connect(ui->removeFromPlayListButton, &QPushButton::clicked, this, &MainWindow::removeFromCollection);
    connect(ui->removePlayListButton, &QPushButton::clicked, this, &MainWindow::removeCollection);

    connect(ui->playlistsList, &QListWidget::currentTextChanged,
            this, &MainWindow::updateCurrentCollection);
    connect(ui->trackList, &QListWidget::itemDoubleClicked,
            this, &MainWindow::playSelectedTrack);
    connect(ui->collectionTracksList, &QListWidget::itemDoubleClicked,
            this, &MainWindow::playSelectedCollectionTrack);

    connect(ui->searchEdit, &QLineEdit::textChanged, this, &MainWindow::filterTracks);
}
void MainWindow::initUI()
{
    ui->playButton->installEventFilter(this);
    ui->pauseButton->installEventFilter(this);
    ui->stopButton->installEventFilter(this);
    ui->prevButton->installEventFilter(this);
    ui->nextButton->installEventFilter(this);
    ui->trackList->installEventFilter(this);
    ui->collectionTracksList->installEventFilter(this);
    ui->playButton->setIcon(QIcon(":/assets/play.png"));
    ui->pauseButton->setIcon(QIcon(":/assets/pause.png"));
    ui->stopButton->setIcon(QIcon(":/assets/stop.png"));
    ui->prevButton->setIcon(QIcon(":/assets/prev.png"));
    ui->nextButton->setIcon(QIcon(":/assets/next.png"));
    ui->shuffleButton->setIcon(QIcon(":/assets/shuffle.png"));
    ui->speedButton->setIcon(QIcon(":/assets/speed.png"));
    ui->openFileButton->setIcon(QIcon(":/assets/add_file.png"));
    ui->openFolderButton->setIcon(QIcon(":/assets/add_folder.png"));
    ui->createPlayListButton->setIcon(QIcon(":/assets/add_playlist.png"));
    ui->addToPlayListButton->setIcon(QIcon(":/assets/add_box.png"));
    ui->renamePlayListButton->setIcon(QIcon(":/assets/edit.png"));
    ui->removeFromPlayListButton->setIcon(QIcon(":/assets/remove.png"));
    ui->removePlayListButton->setIcon(QIcon(":/assets/delete.png"));
    ui->settingsButton->setIcon(QIcon(":/assets/settings.png"));
    ui->removeTrackButton->setIcon(QIcon(":/assets/delete_song.png"));
    ui->renameTrackButton->setIcon(QIcon(":/assets/edit_tracks.png"));

    ui->minimizeButton->setIcon(QIcon(":/assets/minimize.png"));
    ui->fullscreenButton->setIcon(QIcon(":/assets/fullscreen.png"));
    ui->closeButton->setIcon(QIcon(":/assets/close.png"));

    ui->volumeSlider->setRange(0, 100);
    ui->volumeSlider->setValue(70);
    ui->speedSlider->setRange(50, 200);
    ui->speedSlider->setValue(100);
    ui->progressSlider->setRange(0, 100);

    ui->pauseButton->hide();
}

void MainWindow::applyStyles()
{
    QString styleSheet = R"(
        QMainWindow {
            background: #FFFFFF;
            border-radius: 8px;
            border: 1px solid #E0E0E0;
        }

        QWidget {
            background: transparent;
            color: #333333;
            font-family: 'Segoe UI';
        }

        QListWidget {
            background: #FAFAFA;
            border: 1px solid #E0E0E0;
            border-radius: 4px;
            padding: 5px;
            font-size: 12px;
        }

        QListWidget::item {
            padding: 8px;
            border-bottom: 1px solid #E0E0E0;
        }

        QListWidget::item:selected {
            background: #F0F0F0;
            color: #1DB954;
        }

        QSlider::groove:horizontal {
            height: 4px;
            background: #E0E0E0;
            border-radius: 2px;
        }

        QSlider::handle:horizontal {
            width: 12px;
            height: 12px;
            background: #1DB954;
            border-radius: 6px;
            margin: -4px 0;
        }

        QSlider::sub-page:horizontal {
            background: #1DB954;
            border-radius: 2px;
        }

        QPushButton {
            background: #F5F5F5;
            border: none;
            padding: 6px;
            border-radius: 4px;
            min-width: 24px;
            min-height: 24px;
        }

        QPushButton:hover {
            background: #E0E0E0;
        }

        QPushButton:pressed {
            background: #1DB954;
            color: white;
        }

        QLineEdit {
            background: #FAFAFA;
            border: 1px solid #E0E0E0;
            border-radius: 12px;
            padding: 6px 12px;
            color: #333333;
            font-size: 12px;
        }

        QLabel {
            color: #333333;
            font-size: 12px;
        }

        #playButton, #pauseButton {
            background: #1DB954;
            color: white;
            border-radius: 20px;
            min-width: 40px;
            min-height: 40px;
        }

        #trackInfoLabel {
            font-size: 14px;
            font-weight: bold;
            color: #1DB954;
        }

        #shuffleButton, #speedButton {
            background: #F5F5F5;
        }

        #shuffleButton:checked, #speedButton:checked {
            background: #1DB954;
            color: white;
        }

        #minimizeButton, #fullscreenButton, #closeButton {
            background: transparent;
            border-radius: 4px;
        }

        #minimizeButton:hover, #fullscreenButton:hover {
            background: #E0E0E0;
        }

        #closeButton:hover {
            background: #FF4444;
            color: white;
        }

        #titleBar {
            background: #F5F5F5;
            border-top-left-radius: 8px;
            border-top-right-radius: 8px;
            border-bottom: 1px solid #E0E0E0;
        }
    )";

    setStyleSheet(styleSheet);
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPosition().toPoint() - dragPosition);
        event->accept();
    }
}

void MainWindow::minimizeWindow()
{
    showMinimized();
}

bool MainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Enter && watched->isWidgetType()) {
        QWidget *widget = qobject_cast<QWidget*>(watched);
        if (widget && (widget->inherits("QPushButton") || widget->inherits("QListWidget"))) {
            QPropertyAnimation *anim = new QPropertyAnimation(widget, "geometry", this);
            anim->setDuration(150);
            anim->setStartValue(widget->geometry());
            anim->setEndValue(widget->geometry().adjusted(-2, -2, 2, 2));
            anim->start(QPropertyAnimation::DeleteWhenStopped);
        }
    }
    else if (event->type() == QEvent::Leave && watched->isWidgetType()) {
        QWidget *widget = qobject_cast<QWidget*>(watched);
        if (widget && (widget->inherits("QPushButton") || widget->inherits("QListWidget"))) {
            QPropertyAnimation *anim = new QPropertyAnimation(widget, "geometry", this);
            anim->setDuration(150);
            anim->setStartValue(widget->geometry());
            anim->setEndValue(widget->geometry().adjusted(2, 2, -2, -2));
            anim->start(QPropertyAnimation::DeleteWhenStopped);
        }
    }

    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::toggleFullscreen()
{
    if (isFullscreen) {
        showNormal();
        isFullscreen = false;
        ui->fullscreenButton->setIcon(QIcon(":/assets/fullscreen.png"));
    } else {
        showFullScreen();
        isFullscreen = true;
        ui->fullscreenButton->setIcon(QIcon(":/assets/normal.png"));
    }
}

void MainWindow::closeWindow()
{
    close();
}

void MainWindow::togglePlayPause()
{
    if (playlist.isEmpty()) return;

    if (player->playbackState() == QMediaPlayer::PlayingState) {
        player->pause();
        ui->pauseButton->hide();
        ui->playButton->show();
    } else {
        if (player->mediaStatus() == QMediaPlayer::NoMedia ||
            player->playbackState() == QMediaPlayer::StoppedState) {
            if (currentTrackIndex < 0 && !playlist.isEmpty()) {
                currentTrackIndex = 0;
            }
            playTrack(currentTrackIndex);
        } else {
            player->play();
        }
        ui->playButton->hide();
        ui->pauseButton->show();
    }
    updatePlayerControls();
}

 //---------------------РАБОТА С КОЛЛЕКЦИЯМИ!!!!!!!!!!!!!!---------------

void MainWindow::createCollection()
{
    QString name = QInputDialog::getText(this, "Создать коллекцию", "Введите название:");
    if (!name.isEmpty()) {
        musicCollection->addCollection(name);
        updateCollectionsList();
    }
}

void MainWindow::renameCollection()
{
    if (currentCollection.isEmpty()) return;

    QString newName = QInputDialog::getText(this, "Переименовать", "Новое название:",
                                            QLineEdit::Normal, currentCollection);
    if (!newName.isEmpty()) {
        musicCollection->renameCollection(currentCollection, newName);
        currentCollection = newName;
        updateCollectionsList();
    }
}

void MainWindow::removeCollection()
{
    if (currentCollection.isEmpty()) return;

    if (QMessageBox::question(this, "Удаление",
                              "Удалить коллекцию '" + currentCollection + "'?",
                              QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes) {
        musicCollection->removeCollection(currentCollection);
        currentCollection = "";
        updateCollectionsList();
        ui->collectionTracksList->clear();
    }
}

void MainWindow::addToCollection()
{
    if (currentCollection.isEmpty() || currentTrackIndex < 0) return;

    QString trackPath = playlist.at(currentTrackIndex);
    musicCollection->addTrackToCollection(currentCollection, trackPath);
    updateCurrentCollectionTracks();
}

void MainWindow::removeFromCollection()
{
    if (currentCollection.isEmpty() || !ui->collectionTracksList->currentItem()) return;

    QString trackPath = ui->collectionTracksList->currentItem()->data(Qt::UserRole).toString();
    musicCollection->removeTrackFromCollection(currentCollection, trackPath);
    updateCurrentCollectionTracks();
}

void MainWindow::paintEvent(QPaintEvent *event)
{
    QMainWindow::paintEvent(event);
}


void MainWindow::stopPlayback()
{
    player->stop();
    ui->progressSlider->setValue(0);
    ui->currentTimeLabel->setText("00:00");
    ui->playButton->show();
    ui->pauseButton->hide();
    updatePlayerControls();
    m_playbackTimer->stop();
}

void MainWindow::playNextTrack()
{
    if (playlist.isEmpty()) return;

    if (shuffleMode) {
        playRandomTrack();
    } else {
        int nextIndex = currentTrackIndex + 1;
        if (nextIndex < playlist.size()) {
            playTrack(nextIndex);
        } else {
            player->stop();
            currentTrackIndex = -1;
            updatePlayerControls();
        }
    }
}

void MainWindow::removeTrack()
{
    if (currentTrackIndex < 0 || currentTrackIndex >= playlist.size()) return;

    QString trackPath = playlist.at(currentTrackIndex);

    if (QMessageBox::question(this, "Удаление трека",
                              "Вы уверены, что хотите удалить этот трек из списка?",
                              QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes) {
        // Удаляем трек из всех коллекций
        QStringList collections = musicCollection->getCollectionNames();
        for (const QString &collection : collections) {
            if (musicCollection->getTracksInCollection(collection).contains(trackPath)) {
                musicCollection->removeTrackFromCollection(collection, trackPath);
            }
        }

        // Удаляем из основного списка (только одно вхождение)
        allTracks.removeOne(trackPath);
        playlist.removeAt(currentTrackIndex); // Удаляем по индексу
        trackStatistics.remove(trackPath);

        // Обновляем интерфейс
        loadTrackList();
        updateCurrentCollectionTracks();

        // Если удаляемый трек был текущим, останавливаем воспроизведение
        if (currentFilePath == trackPath) {
            stopPlayback();
            currentTrackIndex = -1;
        }

        saveTrackList();
    }
}

void MainWindow::renameTrack()
{
    if (currentTrackIndex < 0 || currentTrackIndex >= playlist.size()) return;

    QString oldPath = playlist.at(currentTrackIndex);
    QFileInfo fileInfo(oldPath);
    QString currentName = fileInfo.fileName();

    QString newName = QInputDialog::getText(this, "Переименовать трек",
                                            "Введите новое название:",
                                            QLineEdit::Normal,
                                            currentName.left(currentName.lastIndexOf('.')));

    if (!newName.isEmpty() && newName != currentName) {
        QString newPath = fileInfo.path() + "/" + newName + "." + fileInfo.suffix();

        if (QFile::rename(oldPath, newPath)) {
            // Обновляем пути во всех коллекциях
            updateTrackNameInCollections(oldPath, newPath);

            // Обновляем основной список
            int index = allTracks.indexOf(oldPath);
            if (index != -1) {
                allTracks.replace(index, newPath);
            }

            index = playlist.indexOf(oldPath);
            if (index != -1) {
                playlist.replace(index, newPath);
            }

            // Обновляем статистику
            if (trackStatistics.contains(oldPath)) {
                trackStatistics[newPath] = trackStatistics[oldPath];
                trackStatistics.remove(oldPath);
            }

            // Если переименовываем текущий трек
            if (currentFilePath == oldPath) {
                currentFilePath = newPath;
            }

            saveTrackList();
            loadTrackList();
            updateCurrentCollectionTracks();
        } else {
            QMessageBox::warning(this, "Ошибка", "Не удалось переименовать файл");
        }
    }
}

void MainWindow::updateTrackNameInCollections(const QString &oldPath, const QString &newPath)
{
    QStringList collections = musicCollection->getCollectionNames();
    for (const QString &collection : collections) {
        QStringList tracks = musicCollection->getTracksInCollection(collection);
        if (tracks.contains(oldPath)) {
            musicCollection->removeTrackFromCollection(collection, oldPath);
            musicCollection->addTrackToCollection(collection, newPath);
        }
    }
}

void MainWindow::playPreviousTrack()
{
    if (playlist.isEmpty()) return;

    if (player->position() > 3000) {
        player->setPosition(0);
    } else {
        if (currentTrackIndex > 0) {
            playTrack(currentTrackIndex - 1);
        } else {
            player->setPosition(0);
        }
    }
}

void MainWindow::seekTrack(int position)
{
    isSeeking = true;
    player->setPosition(position * 1000);
    isSeeking = false;
}

void MainWindow::updatePlaybackPosition(qint64 position)
{
    if (!isSeeking && !ui->progressSlider->isSliderDown()) {
        ui->progressSlider->setValue(static_cast<int>(position / 1000));
        updateTimeDisplay(position);
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
        QString fileName = fileInfo.fileName();
        fileName = fileName.left(fileName.lastIndexOf('.'));

        ui->trackInfoLabel->setText(QString("Now playing: %1").arg(fileName));

        for (int i = 0; i < ui->trackList->count(); ++i) {
            ui->trackList->item(i)->setIcon(QIcon());
        }

        if (currentTrackIndex >= 0 && currentTrackIndex < ui->trackList->count()) {
            QListWidgetItem* currentItem = ui->trackList->item(currentTrackIndex);
        }
    } else {
        ui->trackInfoLabel->setText("No track selected");
    }
}

void MainWindow::playTrack(int index)
{
    if (index >= 0 && index < playlist.size()) {
        isSeeking = false;
        currentTrackIndex = index;
        currentFilePath = playlist.at(index);

        player->setSource(QUrl::fromLocalFile(currentFilePath));
        player->play();

        updateTrackInfo();
        ui->trackList->setCurrentRow(index);
        ui->playButton->hide();
        ui->pauseButton->show();

        m_currentTrackStartTime = QDateTime::currentMSecsSinceEpoch();
        m_playbackTimer->start();

        trackStatistics[currentFilePath].playCount++;
        trackStatistics[currentFilePath].lastPlayed = QDateTime::currentDateTime();

        updatePlayerControls();
    }
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

void MainWindow::updatePlaybackStatistics()
{
    if (player->playbackState() == QMediaPlayer::PlayingState) {
        qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
        qint64 elapsed = currentTime - currentTrackStartTime;
        trackStatistics[currentFilePath].totalPlayTime += elapsed;
        currentTrackStartTime = currentTime;
    }
}

void MainWindow::updatePlayerControls()
{
    bool hasTracks = !playlist.isEmpty();
    bool isPlaying = player->playbackState() == QMediaPlayer::PlayingState;
    bool hasCurrentTrack = currentTrackIndex >= 0 && currentTrackIndex < playlist.size();

    ui->playButton->setEnabled(hasTracks);
    ui->pauseButton->setEnabled(hasTracks);
    ui->stopButton->setEnabled(isPlaying || player->playbackState() == QMediaPlayer::PausedState);
    ui->nextButton->setEnabled(hasTracks && (shuffleMode || hasCurrentTrack));
    ui->prevButton->setEnabled(hasTracks && hasCurrentTrack);

    ui->shuffleButton->setStyleSheet(shuffleMode ? "background-color: #1DB954; border-radius: 4px;" : "");
    ui->speedButton->setStyleSheet(playbackSpeed != 1.0 ? "background-color: #1DB954; border-radius: 4px;" : "");

    ui->removeTrackButton->setEnabled(hasCurrentTrack);
    ui->renameTrackButton->setEnabled(hasCurrentTrack);
}

void MainWindow::handleVolumeChange(int value)
{
    float volume = qBound(0.0f, value / 100.0f, 1.0f);
    audioOutput->setVolume(volume);
}

void MainWindow::handleSpeedChange(int value)
{
    playbackSpeed = value / 100.0f;
    player->setPlaybackRate(playbackSpeed);
    ui->speedLabel->setText(QString("Speed: %1x").arg(playbackSpeed, 0, 'f', 1));
}

void MainWindow::resetSpeed()
{
    playbackSpeed = 1.0f;
    ui->speedSlider->setValue(100);
    player->setPlaybackRate(playbackSpeed);
    ui->speedLabel->setText("Speed: 1.0x");
}

void MainWindow::toggleShuffle()
{
    shuffleMode = !shuffleMode;
    updatePlayerControls();
}

void MainWindow::openFile()
{
    QStringList filePaths = QFileDialog::getOpenFileNames(
        this,
        tr("Open Audio Files"),
        QDir::homePath(),
        tr("Audio Files (*.mp3 *.wav *.ogg *.flac)")
        );

    if (!filePaths.isEmpty()) {
        bool newFilesAdded = false;

        for (const QString &filePath : filePaths) {
            if (!allTracks.contains(filePath)) {
                allTracks.append(filePath);
                QListWidgetItem *item = new QListWidgetItem(QFileInfo(filePath).fileName());
                item->setData(Qt::UserRole, filePath);
                ui->trackList->addItem(item);
                newFilesAdded = true;
            }
        }

        if (newFilesAdded) {
            saveTrackList();
        }

        playlist = allTracks;
        currentTrackIndex = allTracks.indexOf(filePaths.first());
        currentFilePath = filePaths.first();
        playTrack(currentTrackIndex);
    }
}

void MainWindow::openFolder()
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

    for (const QString &file : audioFiles) {
        QString filePath = directory.filePath(file);
        if (!allTracks.contains(filePath)) {
            allTracks.append(filePath);
            QListWidgetItem *item = new QListWidgetItem(file);
            item->setData(Qt::UserRole, filePath);
            ui->trackList->addItem(item);
        }
    }

    playlist = allTracks;
    currentTrackIndex = 0;
    currentFilePath = playlist.first();
    playTrack(currentTrackIndex);
    saveTrackList();
}

void MainWindow::playSelectedTrack(QListWidgetItem *item)
{
    int index = ui->trackList->row(item);
    if (index >= 0 && index < playlist.size()) {
        playTrack(index);
    }
}

void MainWindow::playSelectedCollectionTrack(QListWidgetItem *item)
{
    if (!currentCollection.isEmpty()) {
        QStringList tracks = musicCollection->getTracksInCollection(currentCollection);
        int index = ui->collectionTracksList->row(item);

        if (index >= 0 && index < tracks.size()) {
            QString trackPath = tracks.at(index);
            currentTrackIndex = allTracks.indexOf(trackPath);
            if (currentTrackIndex >= 0) {
                currentFilePath = trackPath;
                playTrack(currentTrackIndex);
            }
        }
    }
}

void MainWindow::filterTracks(const QString &text)
{
    ui->trackList->clear();

    if (text.isEmpty()) {
        for (const QString &filePath : allTracks) {
            QListWidgetItem *item = new QListWidgetItem(QFileInfo(filePath).fileName());
            item->setData(Qt::UserRole, filePath);
            ui->trackList->addItem(item);
        }
        playlist = allTracks;
    } else {
        playlist.clear();
        for (const QString &filePath : allTracks) {
            QString fileName = QFileInfo(filePath).fileName();
            if (fileName.contains(text, Qt::CaseInsensitive)) {
                QListWidgetItem *item = new QListWidgetItem(fileName);
                item->setData(Qt::UserRole, filePath);
                ui->trackList->addItem(item);
                playlist.append(filePath);
            }
        }
    }

    if (currentTrackIndex >= playlist.size()) {
        currentTrackIndex = -1;
    }
    updatePlayerControls();
}

void MainWindow::updateCurrentCollection(const QString &collectionName)
{
    currentCollection = collectionName;
    updateCurrentCollectionTracks();
    updatePlayerControls();
}

void MainWindow::updateCurrentCollectionTracks()
{
    ui->collectionTracksList->clear();
    if (currentCollection.isEmpty()) return;

    QStringList tracks = musicCollection->getTracksInCollection(currentCollection);
    for (const QString &trackPath : tracks) {
        QListWidgetItem *item = new QListWidgetItem(QFileInfo(trackPath).fileName());
        item->setData(Qt::UserRole, trackPath);
        ui->collectionTracksList->addItem(item);
    }
}

void MainWindow::updateCollectionsList()
{
    ui->playlistsList->clear();
    QStringList collections = musicCollection->getCollectionNames();
    for (const QString &name : collections) {
        QListWidgetItem *item = new QListWidgetItem(QIcon(":/icons/playlist.png"), name);
        ui->playlistsList->addItem(item);
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
    ui->trackList->clear();
    for (const QString &filePath : allTracks) {
        QListWidgetItem *item = new QListWidgetItem(QFileInfo(filePath).fileName());
        item->setData(Qt::UserRole, filePath);
        ui->trackList->addItem(item);
    }

    if (!allTracks.isEmpty()) {
        playlist = allTracks;
        // Обновляем текущий индекс после удаления
        currentTrackIndex = allTracks.indexOf(currentFilePath);
        updatePlayerControls();
    } else {
        currentTrackIndex = -1;
        currentFilePath = "";
        updatePlayerControls();
    }
}

void MainWindow::handleMediaStatusChanged(QMediaPlayer::MediaStatus status)
{
    if (status == QMediaPlayer::EndOfMedia && !isSeeking) {
        if (currentTrackIndex < playlist.size() - 1) {
            playNextTrack();
        } else {
            player->stop();
            updatePlayerControls();
        }
    }
}

MainWindow::~MainWindow()
{
    saveTrackList();
    delete player;
    delete audioOutput;
    delete ui;
}
