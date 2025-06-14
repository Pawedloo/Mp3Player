#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QDir>
#include <QLineEdit>
#include <QListWidget>

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
    void on_openButton_clicked();
    void on_playButton_clicked();
    void on_volumeSlider_valueChanged(int value);
    void on_openFolderButton_clicked();
    void on_mediaStatusChanged(QMediaPlayer::MediaStatus status);
    void on_positionChanged(qint64 position);
    void on_nextButton_clicked();
    void on_prevButton_clicked();
    void on_positionSlider_sliderMoved(int position);
    void on_searchTextChanged(const QString &text);
    void on_trackList_itemDoubleClicked(QListWidgetItem *item);

private:
    Ui::MainWindow *ui;
    QMediaPlayer *player;
    QAudioOutput *audioOutput;
    QString currentFilePath;
    QStringList playlist;
    QStringList allTracks;
    int currentTrackIndex;
    void loadFolder(const QString &folderPath);
    void playTrack(int index);
    void updateTimeDisplay(qint64 duration);
    void updateTrackInfo();
    void filterTracks(const QString &filter);
};
#endif // MAINWINDOW_H
