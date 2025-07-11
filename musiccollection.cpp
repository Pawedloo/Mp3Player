#include "musiccollection.h"
#include <QDebug>

MusicCollection::MusicCollection(QObject *parent) : QObject(parent)
{
    loadCollections();
}

QStringList MusicCollection::getCollectionNames() const
{
    return collections.keys();
}

QStringList MusicCollection::getTracksInCollection(const QString &collectionName) const
{
    return collections.value(collectionName);
}

void MusicCollection::addCollection(const QString &name)
{
    if (!collections.contains(name)) {
        collections.insert(name, QStringList());
        saveCollections();
    }
}

void MusicCollection::renameCollection(const QString &oldName, const QString &newName)
{
    if (collections.contains(oldName) && !collections.contains(newName)) {
        QStringList tracks = collections.take(oldName);
        collections.insert(newName, tracks);
        saveCollections();
    }
}

void MusicCollection::removeCollection(const QString &name)
{
    collections.remove(name);
    saveCollections();
}

void MusicCollection::addTrackToCollection(const QString &collectionName, const QString &trackPath)
{
    if (collections.contains(collectionName)) {
        QStringList &tracks = collections[collectionName];
        if (!tracks.contains(trackPath)) {
            tracks.append(trackPath);
            saveCollections();
        }
    }
}

void MusicCollection::removeTrackFromCollection(const QString &collectionName, const QString &trackPath)
{
    if (collections.contains(collectionName)) {
        collections[collectionName].removeAll(trackPath);
        saveCollections();
    }
}

void MusicCollection::saveCollections()
{
    QSettings settings;
    settings.beginWriteArray("Collections");
    int i = 0;
    for (auto it = collections.constBegin(); it != collections.constEnd(); ++it) {
        settings.setArrayIndex(i++);
        settings.setValue("name", it.key());
        settings.setValue("tracks", it.value());
    }
    settings.endArray();
}

void MusicCollection::loadCollections()
{
    QSettings settings;
    int size = settings.beginReadArray("Collections");
    for (int i = 0; i < size; ++i) {
        settings.setArrayIndex(i);
        QString name = settings.value("name").toString();
        QStringList tracks = settings.value("tracks").toStringList();
        collections.insert(name, tracks);
    }
    settings.endArray();
}
