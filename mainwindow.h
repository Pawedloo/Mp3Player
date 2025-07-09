#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QDir>
#include <QListWidget>
#include "musiccollection.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Основные слоты
    void on_openButton_clicked();
    void on_openFolderButton_clicked();
    void on_playButton_clicked();
    void on_pauseButton_clicked();
    void on_stopButton_clicked();
    void on_nextButton_clicked();
    void on_prevButton_clicked();
    void on_shuffleButton_clicked();
    void on_speedButton_clicked();

    // Управление коллекциями
    void on_createCollectionButton_clicked();
    void on_renameCollectionButton_clicked();
    void on_removeCollectionButton_clicked();
    void on_addToCollectionButton_clicked();
    void on_removeFromCollectionButton_clicked();
    void on_collectionsList_currentTextChanged(const QString &currentText);
    void on_collectionTracksList_itemDoubleClicked(QListWidgetItem *item);

    // Управление воспроизведением
    void on_volumeSlider_valueChanged(int value);
    void on_speedSlider_valueChanged(int value);
    void on_positionSlider_sliderMoved(int position);
    void on_mediaStatusChanged(QMediaPlayer::MediaStatus status);
    void on_positionChanged(qint64 position);

    // Поиск и навигация
    void on_searchTextChanged(const QString &text);
    void on_trackList_itemDoubleClicked(QListWidgetItem *item);

private:
    Ui::MainWindow *ui;
    QMediaPlayer *player;
    QAudioOutput *audioOutput;
    MusicCollection *musicCollection;

    QString currentFilePath;
    QStringList playlist;
    QStringList allTracks;
    QString currentCollection;
    int currentTrackIndex;
    bool shuffleMode;
    float playbackSpeed;

    // Вспомогательные методы
    void loadFolder(const QString &folderPath);
    void playTrack(int index);
    void playRandomTrack();
    void updateTimeDisplay(qint64 duration);
    void updateTrackInfo();
    void filterTracks(const QString &filter);
    void updateCollectionsList();
    void updateCurrentCollectionTracks();
    void saveTrackList();
    void loadTrackList();
    void updatePlayerControls();
    void applyPlaybackSpeed();
};

#endif
