#pragma once

#include <QObject>
#include <QHash>
#include <QList>
#include <QString>
#include <QStringList>
#include <QVariantList>
#include <QVariantMap>
#include <QVector>

class BibleData : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap state READ state NOTIFY stateChanged)

public:
    explicit BibleData(QObject *parent = nullptr);

    QVariantMap state() const;

    Q_INVOKABLE void selectIndex(int index);
    Q_INVOKABLE void selectStrongId(const QString &strongId);
    Q_INVOKABLE void goBack();
    Q_INVOKABLE void navigate(const QString &direction);
    Q_INVOKABLE void setOccurrencePageSize(int size);
    Q_INVOKABLE void searchVerse(const QString &query);
    Q_INVOKABLE void navigateTo(const QString &reference);

signals:
    void stateChanged();

private:
    struct TokenRecord {
        QString text;
        QStringList strongIds;
    };

    struct VerseRecord {
        int bookIndex = 0;
        QString book;
        int chapter = 0;
        int verse = 0;
        QString reference;
        QString text;
        QVector<TokenRecord> words;
    };

    struct BookRecord {
        QString name;
        QVector<int> chapters;
    };

    QVariantMap lexiconEntry(const QString &strongId) const;
    QVariantList occurrencesFor(const QString &strongId, int page) const;
    int occurrenceCountFor(const QString &strongId) const;
    int indexForReference(const QString &reference) const;
    QString dataPath(const QString &fileName) const;
    bool loadCorpus();
    bool loadLexicon();
    void loadDemoCorpus();
    QVariantMap wordState(const TokenRecord &word) const;
    QVariantMap verseState(const VerseRecord &verse) const;
    void rebuildState();

    struct Selection {
        int index = 0;
        QString strongId;
    };

    QVector<VerseRecord> m_verses;
    QVector<BookRecord> m_books;
    QHash<QString, QVariantMap> m_lexicon;
    QVariantMap m_state;
    int m_verseIndex = 0;
    int m_selectedIndex = 0;
    QString m_selectedStrongId;
    QList<Selection> m_selectionHistory;
    QString m_searchStatus;
    int m_occurrencePage = 0;
    int m_occurrencePageSize = 4;
    bool m_usingImportedCorpus = false;
};
