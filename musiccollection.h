#ifndef MUSICCOLLECTION_H
#define MUSICCOLLECTION_H

#include <QObject>
#include <QMap>
#include <QStringList>
#include <QSettings>

class MusicCollection : public QObject
{
    Q_OBJECT
public:
    explicit MusicCollection(QObject *parent = nullptr);

    QStringList getCollectionNames() const;
    QStringList getTracksInCollection(const QString &collectionName) const;

    void addCollection(const QString &name);
    void renameCollection(const QString &oldName, const QString &newName);
    void removeCollection(const QString &name);
    void addTrackToCollection(const QString &collectionName, const QString &trackPath);
    void removeTrackFromCollection(const QString &collectionName, const QString &trackPath);

private:
    QMap<QString, QStringList> collections;
    void saveCollections();
    void loadCollections();
};

#endif
