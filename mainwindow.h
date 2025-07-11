#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include "musiccollection.h"
#include <QListWidgetItem>
#include <QMouseEvent>
#include <QDateTime>
#include <QMap>
#include <QTimer>
#include <QTableWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    bool eventFilter(QObject *obj, QEvent *event) override;

private slots:
    void togglePlayPause();
    void updatePlaybackStatistics();
    void stopPlayback();
    void playNextTrack();
    void playPreviousTrack();
    void toggleShuffle();
    void resetSpeed();

    void minimizeWindow();
    void toggleFullscreen();
    void closeWindow();

    void openFile();
    void openFolder();


    void handleVolumeChange(int value);
    void handleSpeedChange(int value);
    void seekTrack(int position);

    void createCollection();
    void renameCollection();
    void removeCollection();
    void addToCollection();
    void removeFromCollection();
    void updateCurrentCollection(const QString &collectionName);


    void updatePlaybackPosition(qint64 position);
    void handleMediaStatusChanged(QMediaPlayer::MediaStatus status);
    void updateTimeDisplay(qint64 position);
    void updateTrackInfo();
    void updatePlayerControls();
    void removeTrack();
    void renameTrack();
    void playSelectedTrack(QListWidgetItem *item);
    void playSelectedCollectionTrack(QListWidgetItem *item);
    void filterTracks(const QString &text);

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
    bool isSeeking;
    bool isFullscreen;
    QPoint dragPosition;

    void initUI();
    void initConnections();
    void applyStyles();
    void setupAnimations();
    void loadFolder(const QString &folderPath);
    void playTrack(int index);
    void playRandomTrack();
    void updateTrackNameInCollections(const QString &oldPath, const QString &newPath);
    void updateCollectionsList();
    void updateCurrentCollectionTracks();
    void saveTrackList();
    void loadTrackList();

    struct TrackStats {
        int playCount = 0;
        qint64 totalPlayTime = 0;
        QDateTime lastPlayed;
    };

    QMap<QString, TrackStats> trackStatistics;

    QTimer* m_playbackTimer;
    qint64 currentTrackStartTime = 0;

    void updateTrackStats(const QString& filePath, qint64 duration);// Переименовываем для ясности
    qint64 m_currentTrackStartTime = 0;
};

#endif // MAINWINDOW_H
