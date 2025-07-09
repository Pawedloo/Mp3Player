#ifndef MUSICCOLLECTION_H
#define MUSICCOLLECTION_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QFile>
#include <QTextStream>

class MusicCollection : public QObject
{
    Q_OBJECT

public:
    explicit MusicCollection(QObject *parent = nullptr);

    void createCollection(const QString &name);
    void renameCollection(const QString &oldName, const QString &newName);
    void removeCollection(const QString &name);
    QStringList getCollectionNames() const;

    void addTrackToCollection(const QString &collectionName, const QString &trackPath);
    void removeTrackFromCollection(const QString &collectionName, const QString &trackPath);
    QStringList getTracksInCollection(const QString &collectionName) const;

    void saveCollections(const QString &filePath = "collections.mc");
    void loadCollections(const QString &filePath = "collections.mc");

signals:
    void collectionCreated(const QString &name);
    void collectionRenamed(const QString &oldName, const QString &newName);
    void collectionRemoved(const QString &name);
    void trackAddedToCollection(const QString &collectionName, const QString &trackPath);
    void trackRemovedFromCollection(const QString &collectionName, const QString &trackPath);
    void collectionsLoaded();

private:
    QMap<QString, QStringList> m_collections;
};

#endif
