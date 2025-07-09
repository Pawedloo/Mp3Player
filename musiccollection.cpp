#include "musiccollection.h"
#include <QDebug>

MusicCollection::MusicCollection(QObject *parent) : QObject(parent)
{
    loadCollections();
}

void MusicCollection::createCollection(const QString &name)
{
    if (name.isEmpty()) {
        qWarning() << "Cannot create collection with empty name";
        return;
    }

    if (!m_collections.contains(name)) {
        m_collections.insert(name, QStringList());
        emit collectionCreated(name);
        saveCollections();
    }
}

void MusicCollection::renameCollection(const QString &oldName, const QString &newName)
{
    if (newName.isEmpty()) {
        qWarning() << "Cannot rename collection to empty name";
        return;
    }

    if (m_collections.contains(oldName) && !m_collections.contains(newName)) {
        QStringList tracks = m_collections.value(oldName);
        m_collections.remove(oldName);
        m_collections.insert(newName, tracks);
        emit collectionRenamed(oldName, newName);
        saveCollections();
    }
}

void MusicCollection::removeCollection(const QString &name)
{
    if (m_collections.remove(name) > 0) {
        emit collectionRemoved(name);
        saveCollections();
    }
}

QStringList MusicCollection::getCollectionNames() const
{
    return m_collections.keys();
}

void MusicCollection::addTrackToCollection(const QString &collectionName, const QString &trackPath)
{
    if (!m_collections.contains(collectionName)) {
        qWarning() << "Collection" << collectionName << "does not exist";
        return;
    }

    if (trackPath.isEmpty() || !QFile::exists(trackPath)) {
        qWarning() << "Invalid track path:" << trackPath;
        return;
    }

    QStringList &tracks = m_collections[collectionName];
    if (!tracks.contains(trackPath)) {
        tracks.append(trackPath);
        emit trackAddedToCollection(collectionName, trackPath);
        saveCollections();
    }
}

void MusicCollection::removeTrackFromCollection(const QString &collectionName, const QString &trackPath)
{
    if (m_collections.contains(collectionName)) {
        QStringList &tracks = m_collections[collectionName];
        if (tracks.removeAll(trackPath) > 0) {
            emit trackRemovedFromCollection(collectionName, trackPath);
            saveCollections();
        }
    }
}

QStringList MusicCollection::getTracksInCollection(const QString &collectionName) const
{
    return m_collections.value(collectionName, QStringList());
}

void MusicCollection::saveCollections(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "Failed to open file for writing:" << filePath;
        return;
    }

    QTextStream out(&file);
    for (auto it = m_collections.constBegin(); it != m_collections.constEnd(); ++it) {
        out << "[" << it.key() << "]\n";
        for (const QString &track : it.value()) {
            out << track << "\n";
        }
        out << "\n";
    }
    file.close();
}

void MusicCollection::loadCollections(const QString &filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open file for reading:" << filePath;
        return;
    }

    m_collections.clear();
    QTextStream in(&file);
    QString currentCollection;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.startsWith('[') && line.endsWith(']')) {
            currentCollection = line.mid(1, line.length() - 2);
            m_collections.insert(currentCollection, QStringList());
        }
        else if (!line.isEmpty() && !currentCollection.isEmpty()) {
            if (QFile::exists(line)) {
                m_collections[currentCollection].append(line);
            } else {
                qWarning() << "Track file not found:" << line;
            }
        }
    }

    file.close();
    emit collectionsLoaded();
}
