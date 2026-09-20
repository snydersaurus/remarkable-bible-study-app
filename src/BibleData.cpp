#include "BibleData.h"

#include <QCoreApplication>
#include <QDataStream>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRegularExpression>
#include <QVariantList>

namespace {

constexpr int maxOccurrencePageSize = 12;

QVariantMap lexicon(const QString &id,
                    const QString &language,
                    const QString &lemma,
                    const QString &transliteration,
                    const QString &pronunciation,
                    const QString &gloss,
                    const QString &definition,
                    const QString &morphology,
                    const QStringList &related)
{
    QVariantMap entry;
    entry[QStringLiteral("strongId")] = id;
    entry[QStringLiteral("language")] = language;
    entry[QStringLiteral("lemma")] = lemma;
    entry[QStringLiteral("transliteration")] = transliteration;
    entry[QStringLiteral("pronunciation")] = pronunciation;
    entry[QStringLiteral("gloss")] = gloss;
    entry[QStringLiteral("definition")] = definition;
    entry[QStringLiteral("morphology")] = morphology;
    entry[QStringLiteral("partOfSpeech")] = morphology;
    entry[QStringLiteral("derivation")] = QString();
    entry[QStringLiteral("usageOutline")] = gloss;

    QVariantList relatedIds;
    for (const QString &relatedId : related)
        relatedIds.append(relatedId);
    entry[QStringLiteral("relatedIds")] = relatedIds;
    return entry;
}

QString readableTransliteration(QString value)
{
    value = value.normalized(QString::NormalizationForm_D);
    QString result;
    for (const QChar character : value) {
        const ushort code = character.unicode();
        if (character.category() == QChar::Mark_NonSpacing
            || character.category() == QChar::Mark_SpacingCombining
            || character.category() == QChar::Mark_Enclosing)
            continue;
        if (code == 0x02BC || code == 0x02BB || code == 0x2018
            || code == 0x2019 || code == 0x2032) {
            result.append(QLatin1Char('\''));
        } else if (code < 128) {
            result.append(character);
        }
    }
    return result.simplified();
}

QString normalizedBookName(QString value)
{
    value = value.trimmed().toLower();
    value.remove(QRegularExpression(QStringLiteral("[^a-z0-9]")));
    return value;
}

const QHash<QString, QStringList> &bookAliases()
{
    static const QHash<QString, QStringList> aliases = [] {
        QHash<QString, QStringList> result;
        result.insert(QStringLiteral("genesis"), {QStringLiteral("ge"), QStringLiteral("gen"), QStringLiteral("gn")});
        result.insert(QStringLiteral("exodus"), {QStringLiteral("ex"), QStringLiteral("exo")});
        result.insert(QStringLiteral("leviticus"), {QStringLiteral("le"), QStringLiteral("lev")});
        result.insert(QStringLiteral("numbers"), {QStringLiteral("nu"), QStringLiteral("num")});
        result.insert(QStringLiteral("deuteronomy"), {QStringLiteral("de"), QStringLiteral("deu"), QStringLiteral("dt")});
        result.insert(QStringLiteral("joshua"), {QStringLiteral("jos")});
        result.insert(QStringLiteral("judges"), {QStringLiteral("jdg"), QStringLiteral("judg")});
        result.insert(QStringLiteral("ruth"), {QStringLiteral("ru")});
        result.insert(QStringLiteral("1samuel"), {QStringLiteral("1sa"), QStringLiteral("1sam")});
        result.insert(QStringLiteral("2samuel"), {QStringLiteral("2sa"), QStringLiteral("2sam")});
        result.insert(QStringLiteral("1kings"), {QStringLiteral("1ki"), QStringLiteral("1kgs")});
        result.insert(QStringLiteral("2kings"), {QStringLiteral("2ki"), QStringLiteral("2kgs")});
        result.insert(QStringLiteral("1chronicles"), {QStringLiteral("1ch"), QStringLiteral("1chr")});
        result.insert(QStringLiteral("2chronicles"), {QStringLiteral("2ch"), QStringLiteral("2chr")});
        result.insert(QStringLiteral("ezra"), {QStringLiteral("ezr")});
        result.insert(QStringLiteral("nehemiah"), {QStringLiteral("ne"), QStringLiteral("neh")});
        result.insert(QStringLiteral("esther"), {QStringLiteral("es"), QStringLiteral("est")});
        result.insert(QStringLiteral("job"), {QStringLiteral("jb")});
        result.insert(QStringLiteral("psalms"), {QStringLiteral("ps"), QStringLiteral("psa"), QStringLiteral("psalm")});
        result.insert(QStringLiteral("proverbs"), {QStringLiteral("pr"), QStringLiteral("prv"), QStringLiteral("prov")});
        result.insert(QStringLiteral("ecclesiastes"), {QStringLiteral("ec"), QStringLiteral("ecc")});
        result.insert(QStringLiteral("songofsolomon"), {QStringLiteral("ss"), QStringLiteral("sos"), QStringLiteral("song"), QStringLiteral("cant")});
        result.insert(QStringLiteral("isaiah"), {QStringLiteral("isa")});
        result.insert(QStringLiteral("jeremiah"), {QStringLiteral("jer")});
        result.insert(QStringLiteral("lamentations"), {QStringLiteral("la"), QStringLiteral("lam")});
        result.insert(QStringLiteral("ezekiel"), {QStringLiteral("eze"), QStringLiteral("ezek")});
        result.insert(QStringLiteral("daniel"), {QStringLiteral("da"), QStringLiteral("dan")});
        result.insert(QStringLiteral("hosea"), {QStringLiteral("ho")});
        result.insert(QStringLiteral("joel"), {QStringLiteral("joe")});
        result.insert(QStringLiteral("amos"), {QStringLiteral("am")});
        result.insert(QStringLiteral("obadiah"), {QStringLiteral("ob")});
        result.insert(QStringLiteral("jonah"), {QStringLiteral("jon")});
        result.insert(QStringLiteral("micah"), {QStringLiteral("mic")});
        result.insert(QStringLiteral("nahum"), {QStringLiteral("na")});
        result.insert(QStringLiteral("habakkuk"), {QStringLiteral("hab")});
        result.insert(QStringLiteral("zephaniah"), {QStringLiteral("zep"), QStringLiteral("zeph")});
        result.insert(QStringLiteral("haggai"), {QStringLiteral("hag")});
        result.insert(QStringLiteral("zechariah"), {QStringLiteral("zec"), QStringLiteral("zech")});
        result.insert(QStringLiteral("malachi"), {QStringLiteral("mal")});
        result.insert(QStringLiteral("matthew"), {QStringLiteral("mt"), QStringLiteral("mat")});
        result.insert(QStringLiteral("mark"), {QStringLiteral("mk"), QStringLiteral("mrk")});
        result.insert(QStringLiteral("luke"), {QStringLiteral("lu"), QStringLiteral("lk")});
        result.insert(QStringLiteral("john"), {QStringLiteral("jn"), QStringLiteral("joh")});
        result.insert(QStringLiteral("acts"), {QStringLiteral("ac"), QStringLiteral("act")});
        result.insert(QStringLiteral("romans"), {QStringLiteral("ro"), QStringLiteral("rom")});
        result.insert(QStringLiteral("1corinthians"), {QStringLiteral("1co"), QStringLiteral("1cor")});
        result.insert(QStringLiteral("2corinthians"), {QStringLiteral("2co"), QStringLiteral("2cor")});
        result.insert(QStringLiteral("galatians"), {QStringLiteral("ga"), QStringLiteral("gal")});
        result.insert(QStringLiteral("ephesians"), {QStringLiteral("eph")});
        result.insert(QStringLiteral("philippians"), {QStringLiteral("php"), QStringLiteral("phi")});
        result.insert(QStringLiteral("colossians"), {QStringLiteral("col"), QStringLiteral("co")});
        result.insert(QStringLiteral("1thessalonians"), {QStringLiteral("1th"), QStringLiteral("1thes")});
        result.insert(QStringLiteral("2thessalonians"), {QStringLiteral("2th"), QStringLiteral("2thes")});
        result.insert(QStringLiteral("1timothy"), {QStringLiteral("1ti"), QStringLiteral("1tim")});
        result.insert(QStringLiteral("2timothy"), {QStringLiteral("2ti"), QStringLiteral("2tim")});
        result.insert(QStringLiteral("titus"), {QStringLiteral("tit")});
        result.insert(QStringLiteral("philemon"), {QStringLiteral("phm"), QStringLiteral("philem")});
        result.insert(QStringLiteral("hebrews"), {QStringLiteral("heb")});
        result.insert(QStringLiteral("james"), {QStringLiteral("jas"), QStringLiteral("jam")});
        result.insert(QStringLiteral("1peter"), {QStringLiteral("1pe"), QStringLiteral("1pet")});
        result.insert(QStringLiteral("2peter"), {QStringLiteral("2pe"), QStringLiteral("2pet")});
        result.insert(QStringLiteral("1john"), {QStringLiteral("1jn"), QStringLiteral("1joh")});
        result.insert(QStringLiteral("2john"), {QStringLiteral("2jn"), QStringLiteral("2joh")});
        result.insert(QStringLiteral("3john"), {QStringLiteral("3jn"), QStringLiteral("3joh")});
        result.insert(QStringLiteral("jude"), {QStringLiteral("jud")});
        result.insert(QStringLiteral("revelation"), {QStringLiteral("re"), QStringLiteral("rev")});
        return result;
    }();
    return aliases;
}

} // namespace

BibleData::BibleData(QObject *parent) : QObject(parent)
{
    const bool corpusLoaded = loadCorpus();
    const bool lexiconLoaded = loadLexicon();
    m_usingImportedCorpus = corpusLoaded;

    if (!corpusLoaded)
        loadDemoCorpus();

    auto addRelated = [this](const QString &id, const QStringList &related) {
        auto it = m_lexicon.find(id);
        if (it == m_lexicon.end())
            return;
        QVariantList ids;
        for (const QString &relatedId : related)
            ids.append(relatedId);
        it->insert(QStringLiteral("relatedIds"), ids);
    };
    addRelated(QStringLiteral("H7225"), {QStringLiteral("H7218")});
    addRelated(QStringLiteral("H7218"), {QStringLiteral("H7225")});
    addRelated(QStringLiteral("G746"), {QStringLiteral("G757")});
    addRelated(QStringLiteral("G757"), {QStringLiteral("G746")});
    addRelated(QStringLiteral("G3056"), {QStringLiteral("G3004")});
    addRelated(QStringLiteral("G3004"), {QStringLiteral("G3056")});

    Q_UNUSED(lexiconLoaded);
    rebuildState();
}

QString BibleData::dataPath(const QString &fileName) const
{
    const QString relative = QStringLiteral("/data/") + fileName;
    QStringList candidates;
    const QString configured = qEnvironmentVariable("WORD_STUDY_DATA_DIR");
    if (!configured.isEmpty())
        candidates.append(QDir(configured).filePath(fileName));

    const QString appDir = QCoreApplication::applicationDirPath();
    candidates.append(appDir + QStringLiteral("/../data/") + fileName);
    candidates.append(appDir + relative);
    candidates.append(QDir::currentPath() + relative);

    for (const QString &candidate : candidates) {
        const QFileInfo info(candidate);
        if (info.exists() && info.isFile())
            return info.absoluteFilePath();
    }
    return QString();
}

bool BibleData::loadCorpus()
{
    const QString path = dataPath(QStringLiteral("bible.bin"));
    if (path.isEmpty())
        return false;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    QDataStream stream(&file);
    stream.setByteOrder(QDataStream::LittleEndian);

    char magic[4] = {};
    if (stream.readRawData(magic, 4) != 4
        || QByteArray(magic, 4) != QByteArrayLiteral("WSB1"))
        return false;

    auto readString = [&stream](QString *result) {
        quint32 length = 0;
        stream >> length;
        if (stream.status() != QDataStream::Ok || length > 1'000'000)
            return false;
        QByteArray bytes;
        bytes.resize(static_cast<int>(length));
        if (length > 0 && stream.readRawData(bytes.data(), bytes.size()) != bytes.size())
            return false;
        *result = QString::fromUtf8(bytes);
        return true;
    };

    quint16 version = 0;
    quint16 bookCount = 0;
    quint32 verseCount = 0;
    stream >> version >> bookCount >> verseCount;
    if (stream.status() != QDataStream::Ok || version != 1 || bookCount == 0
        || verseCount == 0)
        return false;

    m_books.clear();
    m_books.reserve(bookCount);
    for (quint16 i = 0; i < bookCount; ++i) {
        BookRecord book;
        if (!readString(&book.name))
            return false;

        quint16 chapterCount = 0;
        stream >> chapterCount;
        if (stream.status() != QDataStream::Ok)
            return false;
        book.chapters.reserve(chapterCount);
        for (quint16 chapter = 0; chapter < chapterCount; ++chapter) {
            quint16 number = 0;
            stream >> number;
            book.chapters.append(number);
        }
        m_books.append(book);
    }

    m_verses.clear();
    m_verses.reserve(static_cast<int>(verseCount));
    for (quint32 i = 0; i < verseCount; ++i) {
        quint16 bookIndex = 0;
        quint16 chapter = 0;
        quint16 number = 0;
        stream >> bookIndex >> chapter >> number;
        if (stream.status() != QDataStream::Ok || bookIndex >= m_books.size())
            return false;

        VerseRecord record;
        record.bookIndex = bookIndex;
        record.book = m_books.at(bookIndex).name;
        record.chapter = chapter;
        record.verse = number;
        record.reference = QStringLiteral("%1 %2:%3")
                               .arg(record.book)
                               .arg(record.chapter)
                               .arg(record.verse);
        if (!readString(&record.text))
            return false;

        quint32 wordCount = 0;
        stream >> wordCount;
        if (stream.status() != QDataStream::Ok || wordCount > 1000)
            return false;
        record.words.reserve(static_cast<int>(wordCount));
        for (quint32 wordIndex = 0; wordIndex < wordCount; ++wordIndex) {
            TokenRecord word;
            if (!readString(&word.text))
                return false;
            quint8 strongCount = 0;
            stream >> strongCount;
            for (quint8 strongIndex = 0; strongIndex < strongCount; ++strongIndex) {
                QString strongId;
                if (!readString(&strongId))
                    return false;
                word.strongIds.append(strongId);
            }
            record.words.append(word);
        }
        m_verses.append(record);
    }

    return stream.status() == QDataStream::Ok && !m_verses.isEmpty();
}

bool BibleData::loadLexicon()
{
    const QString path = dataPath(QStringLiteral("strongs.json"));
    if (path.isEmpty())
        return false;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly))
        return false;

    const QJsonObject root = QJsonDocument::fromJson(file.readAll()).object();
    if (root.isEmpty())
        return false;

    for (auto it = root.begin(); it != root.end(); ++it) {
        const QJsonObject source = it.value().toObject();
        const QString id = it.key();
        const QString language = id.startsWith(QLatin1Char('G'))
                                     ? QStringLiteral("Greek")
                                     : QStringLiteral("Hebrew");
        const QString definition = source.value(QStringLiteral("def")).toString();
        QString gloss = source.value(QStringLiteral("kjv")).toString();
        if (gloss.isEmpty())
            gloss = definition;

        QVariantMap entry = lexicon(
            id,
            language,
            source.value(QStringLiteral("lemma")).toString(),
            readableTransliteration(source.value(QStringLiteral("translit")).toString()),
            readableTransliteration(source.value(QStringLiteral("pron")).toString()),
            gloss,
            definition,
            QStringLiteral("Strong's lexical entry"),
            {});

        QString partOfSpeech = source.value(QStringLiteral("partOfSpeech")).toString();
        if (partOfSpeech.isEmpty())
            partOfSpeech = source.value(QStringLiteral("pos")).toString();
        entry[QStringLiteral("partOfSpeech")]
            = partOfSpeech.isEmpty()
                  ? QStringLiteral("Not included in this dictionary")
                  : readableTransliteration(partOfSpeech);
        entry[QStringLiteral("derivation")]
            = readableTransliteration(source.value(QStringLiteral("derivation")).toString());
        entry[QStringLiteral("usageOutline")] = gloss;
        m_lexicon.insert(id, entry);
    }
    return !m_lexicon.isEmpty();
}

void BibleData::loadDemoCorpus()
{
    auto add = [this](const QString &id,
                      const QString &language,
                      const QString &lemma,
                      const QString &transliteration,
                      const QString &pronunciation,
                      const QString &gloss,
                      const QString &definition,
                      const QString &morphology,
                      const QStringList &related) {
        if (!m_lexicon.contains(id))
            m_lexicon.insert(id, lexicon(id, language, lemma, transliteration,
                                         pronunciation, gloss, definition,
                                         morphology, related));
    };

    add(QStringLiteral("H7225"), QStringLiteral("Hebrew"), QStringLiteral("רֵאשִׁית"),
        QStringLiteral("re'shiyth"), QStringLiteral("ray-sheeth"),
        QStringLiteral("beginning; first"),
        QStringLiteral("the beginning, first in place, time, order or rank"),
        QStringLiteral("feminine noun"), {QStringLiteral("H7218")});
    add(QStringLiteral("H7218"), QStringLiteral("Hebrew"), QStringLiteral("רֹאשׁ"),
        QStringLiteral("ro'sh"), QStringLiteral("roshe"),
        QStringLiteral("head; chief; top"),
        QStringLiteral("the head or highest part; figuratively the first or chief"),
        QStringLiteral("masculine noun"), {QStringLiteral("H7225")});
    add(QStringLiteral("H430"), QStringLiteral("Hebrew"), QStringLiteral("אֱלֹהִים"),
        QStringLiteral("'elohiym"), QStringLiteral("el-o-heem"),
        QStringLiteral("God; gods; judges"),
        QStringLiteral("the plural form commonly used for the one true God"),
        QStringLiteral("masculine plural noun"), {});
    add(QStringLiteral("H1254"), QStringLiteral("Hebrew"), QStringLiteral("בָּרָא"),
        QStringLiteral("bara'"), QStringLiteral("baw-raw"),
        QStringLiteral("to create; shape; form"),
        QStringLiteral("to create, especially in the sense of bringing into existence"),
        QStringLiteral("verb"), {});
    add(QStringLiteral("H8064"), QStringLiteral("Hebrew"), QStringLiteral("שָׁמַיִם"),
        QStringLiteral("shamayim"), QStringLiteral("shaw-mah-yim"),
        QStringLiteral("heaven; sky"),
        QStringLiteral("the visible heavens, sky, or dwelling place of God"),
        QStringLiteral("masculine plural noun"), {});
    add(QStringLiteral("H776"), QStringLiteral("Hebrew"), QStringLiteral("אֶרֶץ"),
        QStringLiteral("'erets"), QStringLiteral("eh-rets"),
        QStringLiteral("land; earth; country"),
        QStringLiteral("land or earth, including a region or country"),
        QStringLiteral("feminine noun"), {});
    add(QStringLiteral("G746"), QStringLiteral("Greek"), QStringLiteral("ἀρχή"),
        QStringLiteral("archē"), QStringLiteral("ar-khay"),
        QStringLiteral("beginning; origin; rule"),
        QStringLiteral("a beginning, origin, or first place; also a ruler or authority"),
        QStringLiteral("feminine noun"), {QStringLiteral("G757")});
    add(QStringLiteral("G757"), QStringLiteral("Greek"), QStringLiteral("ἄρχω"),
        QStringLiteral("archō"), QStringLiteral("ar-kho"),
        QStringLiteral("to rule; begin"),
        QStringLiteral("to be first, to rule, or to begin"), QStringLiteral("verb"),
        {QStringLiteral("G746")});
    add(QStringLiteral("G3056"), QStringLiteral("Greek"), QStringLiteral("λόγος"),
        QStringLiteral("logos"), QStringLiteral("log-os"),
        QStringLiteral("word; message; account"),
        QStringLiteral("a word, message, account, or reason"),
        QStringLiteral("masculine noun"), {QStringLiteral("G3004")});
    add(QStringLiteral("G3004"), QStringLiteral("Greek"), QStringLiteral("λέγω"),
        QStringLiteral("legō"), QStringLiteral("leg-o"),
        QStringLiteral("to say; speak; tell"),
        QStringLiteral("to say or speak; the lexical family associated with logos"),
        QStringLiteral("verb"), {QStringLiteral("G3056")});
    add(QStringLiteral("G2316"), QStringLiteral("Greek"), QStringLiteral("θεός"),
        QStringLiteral("theos"), QStringLiteral("theh-os"),
        QStringLiteral("God; deity"), QStringLiteral("God or a deity"),
        QStringLiteral("masculine noun"), {});
    add(QStringLiteral("G4314"), QStringLiteral("Greek"), QStringLiteral("πρός"),
        QStringLiteral("pros"), QStringLiteral("pros"),
        QStringLiteral("toward; with; near"),
        QStringLiteral("a preposition expressing motion toward or personal relation with"),
        QStringLiteral("preposition"), {});
    add(QStringLiteral("G2258"), QStringLiteral("Greek"), QStringLiteral("ἦν"),
        QStringLiteral("ēn"), QStringLiteral("ane"), QStringLiteral("was; existed"),
        QStringLiteral("an imperfect form of to be, describing continuing existence"),
        QStringLiteral("verb, imperfect"), {});

    m_books = {
        {QStringLiteral("Genesis"), {1}},
        {QStringLiteral("John"), {1}},
    };
    m_verses = {
        {0, QStringLiteral("Genesis"), 1, 1, QStringLiteral("Genesis 1:1"),
         QStringLiteral("In the beginning God created the heaven and the earth."),
         {{QStringLiteral("In"), {QStringLiteral("H7225")}},
          {QStringLiteral("the"), {QStringLiteral("H7225")}},
          {QStringLiteral("beginning"), {QStringLiteral("H7225")}},
          {QStringLiteral("God"), {QStringLiteral("H430")}},
          {QStringLiteral("created"), {QStringLiteral("H1254")}},
          {QStringLiteral("the"), {}},
          {QStringLiteral("heaven"), {QStringLiteral("H8064")}},
          {QStringLiteral("and"), {}},
          {QStringLiteral("the"), {}},
          {QStringLiteral("earth."), {QStringLiteral("H776")}}}},
        {1, QStringLiteral("John"), 1, 1, QStringLiteral("John 1:1"),
         QStringLiteral("In the beginning was the Word, and the Word was with God, and the Word was God."),
         {{QStringLiteral("In"), {QStringLiteral("G746")}},
          {QStringLiteral("the"), {QStringLiteral("G746")}},
          {QStringLiteral("beginning"), {QStringLiteral("G746")}},
          {QStringLiteral("was"), {QStringLiteral("G2258")}},
          {QStringLiteral("the"), {}},
          {QStringLiteral("Word,"), {QStringLiteral("G3056")}},
          {QStringLiteral("and"), {}},
          {QStringLiteral("the"), {}},
          {QStringLiteral("Word"), {QStringLiteral("G3056")}},
          {QStringLiteral("was"), {QStringLiteral("G2258")}},
          {QStringLiteral("with"), {QStringLiteral("G4314")}},
          {QStringLiteral("God,"), {QStringLiteral("G2316")}},
          {QStringLiteral("and"), {}},
          {QStringLiteral("the"), {}},
          {QStringLiteral("Word"), {QStringLiteral("G3056")}},
          {QStringLiteral("was"), {QStringLiteral("G2258")}},
          {QStringLiteral("God."), {QStringLiteral("G2316")}}}},
    };
}

QVariantMap BibleData::lexiconEntry(const QString &strongId) const
{
    return m_lexicon.value(strongId);
}

QVariantMap BibleData::wordState(const TokenRecord &word) const
{
    QVariantMap result;
    result[QStringLiteral("text")] = word.text;
    result[QStringLiteral("tagged")] = !word.strongIds.isEmpty();
    result[QStringLiteral("strongId")]
        = word.strongIds.isEmpty() ? QString() : word.strongIds.first();

    QVariantList ids;
    for (const QString &id : word.strongIds)
        ids.append(id);
    result[QStringLiteral("strongIds")] = ids;
    return result;
}

QVariantMap BibleData::verseState(const VerseRecord &record) const
{
    QVariantMap result;
    result[QStringLiteral("book")] = record.book;
    result[QStringLiteral("chapter")] = record.chapter;
    result[QStringLiteral("verse")] = record.verse;
    result[QStringLiteral("reference")] = record.reference;
    result[QStringLiteral("text")] = record.text;

    QVariantList words;
    for (const TokenRecord &word : record.words)
        words.append(wordState(word));
    result[QStringLiteral("words")] = words;
    return result;
}

QVariantMap BibleData::state() const
{
    return m_state;
}

QVariantList BibleData::occurrencesFor(const QString &strongId, int page) const
{
    QVariantList occurrences;
    if (strongId.isEmpty())
        return occurrences;

    const int offset = qMax(0, page) * m_occurrencePageSize;
    int matched = 0;
    for (const VerseRecord &current : m_verses) {
        for (const TokenRecord &word : current.words) {
            if (!word.strongIds.contains(strongId))
                continue;

            if (matched++ < offset)
                continue;
            if (occurrences.size() >= m_occurrencePageSize)
                return occurrences;

            QVariantMap occurrence;
            occurrence[QStringLiteral("reference")] = current.reference;
            occurrence[QStringLiteral("text")] = word.text;
            occurrence[QStringLiteral("verseText")] = current.text;
            occurrences.append(occurrence);
        }
    }
    return occurrences;
}

int BibleData::occurrenceCountFor(const QString &strongId) const
{
    if (strongId.isEmpty())
        return 0;

    int count = 0;
    for (const VerseRecord &current : m_verses) {
        for (const TokenRecord &word : current.words) {
            if (word.strongIds.contains(strongId))
                ++count;
        }
    }
    return count;
}

int BibleData::indexForReference(const QString &reference) const
{
    const QString wanted = reference.trimmed();
    if (wanted.isEmpty())
        return -1;

    for (int i = 0; i < m_verses.size(); ++i) {
        if (m_verses.at(i).reference.compare(wanted, Qt::CaseInsensitive) == 0)
            return i;
    }

    static const QRegularExpression pattern(
        QStringLiteral("^\\s*(.+?)\\s+(\\d+)(?:\\s*[:.]\\s*(\\d+))?\\s*$"),
        QRegularExpression::CaseInsensitiveOption);
    const QRegularExpressionMatch match = pattern.match(wanted);
    if (!match.hasMatch())
        return -1;

    const QString requestedBook = normalizedBookName(match.captured(1));
    const int requestedChapter = match.captured(2).toInt();
    const bool hasVerse = !match.captured(3).isEmpty();
    const int requestedVerse = hasVerse ? match.captured(3).toInt() : 1;
    int requestedBookIndex = -1;

    for (int i = 0; i < m_books.size(); ++i) {
        const QString canonical = normalizedBookName(m_books.at(i).name);
        if (canonical == requestedBook
            || bookAliases().value(canonical).contains(requestedBook)) {
            requestedBookIndex = i;
            break;
        }
    }
    if (requestedBookIndex < 0)
        return -1;

    for (int i = 0; i < m_verses.size(); ++i) {
        const VerseRecord &candidate = m_verses.at(i);
        if (candidate.bookIndex != requestedBookIndex
            || candidate.chapter != requestedChapter)
            continue;
        if (!hasVerse || candidate.verse == requestedVerse)
            return i;
    }
    return -1;
}

void BibleData::rebuildState()
{
    if (m_verses.isEmpty())
        return;
    m_verseIndex = qBound(0, m_verseIndex, m_verses.size() - 1);

    const VerseRecord &current = m_verses.at(m_verseIndex);
    const QVariantMap currentState = verseState(current);
    const QVariantList words = currentState.value(QStringLiteral("words")).toList();

    QVariantMap selected;
    if (m_selectedIndex >= 0 && m_selectedIndex < words.size())
        selected = words.at(m_selectedIndex).toMap();
    else if (!m_selectedStrongId.isEmpty()) {
        selected[QStringLiteral("text")] = QStringLiteral("Related entry");
        selected[QStringLiteral("strongId")] = m_selectedStrongId;
        selected[QStringLiteral("tagged")] = true;
    }

    const QString strongId = selected.value(QStringLiteral("strongId")).toString();
    if (!strongId.isEmpty()) {
        const QVariantMap entry = lexiconEntry(strongId);
        for (auto it = entry.cbegin(); it != entry.cend(); ++it)
            selected.insert(it.key(), it.value());

        QVariantList related;
        for (const QVariant &idValue : entry.value(QStringLiteral("relatedIds")).toList()) {
            const QString relatedId = idValue.toString();
            QVariantMap relatedEntry = lexiconEntry(relatedId);
            if (!relatedEntry.isEmpty()) {
                relatedEntry[QStringLiteral("label")]
                    = relatedId + QStringLiteral(" · ")
                    + relatedEntry.value(QStringLiteral("transliteration")).toString();
                related.append(relatedEntry);
            }
        }
        selected[QStringLiteral("relatedEntries")] = related;
    } else {
        selected[QStringLiteral("language")] = QStringLiteral("—");
        selected[QStringLiteral("lemma")] = QStringLiteral("Not tagged");
        selected[QStringLiteral("gloss")] = QStringLiteral("Function word");
        selected[QStringLiteral("definition")]
            = QStringLiteral("This word has no Strong's entry in the KJV corpus.");
        selected[QStringLiteral("relatedEntries")] = QVariantList();
    }

    QVariantList chapterVerses;
    for (const VerseRecord &candidate : m_verses) {
        if (candidate.bookIndex != current.bookIndex || candidate.chapter != current.chapter)
            continue;

        QVariantMap readingVerse;
        readingVerse[QStringLiteral("reference")] = candidate.reference;
        readingVerse[QStringLiteral("verse")] = candidate.verse;
        readingVerse[QStringLiteral("text")] = candidate.text;
        chapterVerses.append(readingVerse);
    }

    QVariantList books;
    for (const BookRecord &book : m_books) {
        QVariantMap bookState;
        bookState[QStringLiteral("name")] = book.name;
        QVariantList chapters;
        for (int chapter : book.chapters)
            chapters.append(chapter);
        bookState[QStringLiteral("chapters")] = chapters;
        books.append(bookState);
    }

    m_state = currentState;
    m_state[QStringLiteral("loaded")] = true;
    m_state[QStringLiteral("translation")] = QStringLiteral("KJV");
    m_state[QStringLiteral("corpusStatus")]
        = m_usingImportedCorpus
              ? QStringLiteral("FULL KJV · %1 VERSES").arg(m_verses.size())
              : QStringLiteral("DEMO CORPUS · %1 VERSES").arg(m_verses.size());
    m_state[QStringLiteral("verseIndex")] = m_verseIndex;
    m_state[QStringLiteral("verseCount")] = m_verses.size();
    m_state[QStringLiteral("canPrev")] = m_verseIndex > 0;
    m_state[QStringLiteral("canNext")] = m_verseIndex + 1 < m_verses.size();

    int chapterStart = m_verseIndex;
    while (chapterStart > 0) {
        const VerseRecord &previous = m_verses.at(chapterStart - 1);
        if (previous.bookIndex != current.bookIndex
            || previous.chapter != current.chapter)
            break;
        --chapterStart;
    }
    int chapterEnd = m_verseIndex;
    while (chapterEnd + 1 < m_verses.size()) {
        const VerseRecord &next = m_verses.at(chapterEnd + 1);
        if (next.bookIndex != current.bookIndex
            || next.chapter != current.chapter)
            break;
        ++chapterEnd;
    }
    m_state[QStringLiteral("canPrevChapter")] = chapterStart > 0;
    m_state[QStringLiteral("canNextChapter")] = chapterEnd + 1 < m_verses.size();
    m_state[QStringLiteral("selectedIndex")] = m_selectedIndex;
    m_state[QStringLiteral("selectedWord")] = selected;
    m_state[QStringLiteral("canGoBack")] = !m_selectionHistory.isEmpty();
    m_state[QStringLiteral("chapterVerses")] = chapterVerses;
    m_state[QStringLiteral("books")] = books;
    const int occurrenceTotal = occurrenceCountFor(strongId);
    const int occurrencePageCount = occurrenceTotal > 0
                                        ? (occurrenceTotal + m_occurrencePageSize - 1)
                                              / m_occurrencePageSize
                                        : 0;
    m_occurrencePage = qBound(0, m_occurrencePage,
                              qMax(0, occurrencePageCount - 1));
    m_state[QStringLiteral("occurrences")]
        = occurrencesFor(strongId, m_occurrencePage);
    m_state[QStringLiteral("occurrenceTotal")] = occurrenceTotal;
    m_state[QStringLiteral("occurrencePage")] = m_occurrencePage;
    m_state[QStringLiteral("occurrencePageCount")] = occurrencePageCount;
    m_state[QStringLiteral("canPrevOccurrencePage")] = m_occurrencePage > 0;
    m_state[QStringLiteral("canNextOccurrencePage")]
        = m_occurrencePage + 1 < occurrencePageCount;
    m_state[QStringLiteral("searchStatus")] = m_searchStatus;
    emit stateChanged();
}

void BibleData::selectIndex(int index)
{
    if (m_verses.isEmpty())
        return;
    const int count = m_verses.at(m_verseIndex).words.size();
    if (count == 0)
        return;
    const int clamped = qBound(0, index, count - 1);
    if (clamped == m_selectedIndex && m_selectedStrongId.isEmpty())
        return;
    m_selectionHistory.clear();
    m_selectedStrongId.clear();
    m_selectedIndex = clamped;
    m_occurrencePage = 0;
    rebuildState();
}

void BibleData::selectStrongId(const QString &strongId)
{
    if (!m_lexicon.contains(strongId))
        return;
    if (m_selectedIndex == -1 && m_selectedStrongId == strongId)
        return;
    m_selectionHistory.append({m_selectedIndex, m_selectedStrongId});
    m_selectedIndex = -1;
    m_selectedStrongId = strongId;
    m_occurrencePage = 0;
    rebuildState();
}

void BibleData::goBack()
{
    if (m_selectionHistory.isEmpty())
        return;

    const Selection previous = m_selectionHistory.takeLast();
    m_selectedIndex = previous.index;
    m_selectedStrongId = previous.strongId;
    m_occurrencePage = 0;
    rebuildState();
}

void BibleData::setOccurrencePageSize(int size)
{
    const int bounded = qBound(1, size, maxOccurrencePageSize);
    if (bounded == m_occurrencePageSize)
        return;
    m_occurrencePageSize = bounded;
    m_occurrencePage = 0;
    rebuildState();
}

void BibleData::navigate(const QString &direction)
{
    if (m_verses.isEmpty())
        return;

    if (direction == QStringLiteral("nextOccurrencePage")
        || direction == QStringLiteral("prevOccurrencePage")) {
        const int pageCount = m_state.value(QStringLiteral("occurrencePageCount")).toInt();
        const int delta = direction == QStringLiteral("nextOccurrencePage") ? 1 : -1;
        const int nextPage = m_occurrencePage + delta;
        if (pageCount <= 0 || nextPage < 0 || nextPage >= pageCount)
            return;
        m_occurrencePage = nextPage;
        rebuildState();
        return;
    }

    int next = m_verseIndex;
    if (direction == QStringLiteral("next")) {
        ++next;
    } else if (direction == QStringLiteral("prev")) {
        --next;
    } else if (direction == QStringLiteral("nextChapter")) {
        while (next + 1 < m_verses.size()
               && m_verses.at(next + 1).bookIndex == m_verses.at(m_verseIndex).bookIndex
               && m_verses.at(next + 1).chapter == m_verses.at(m_verseIndex).chapter)
            ++next;
        if (next + 1 >= m_verses.size())
            return;
        ++next;
    } else if (direction == QStringLiteral("prevChapter")) {
        while (next > 0
               && m_verses.at(next - 1).bookIndex == m_verses.at(m_verseIndex).bookIndex
               && m_verses.at(next - 1).chapter == m_verses.at(m_verseIndex).chapter)
            --next;
        if (next == 0)
            return;
        --next;
        while (next > 0
               && m_verses.at(next - 1).bookIndex == m_verses.at(next).bookIndex
               && m_verses.at(next - 1).chapter == m_verses.at(next).chapter)
            --next;
    } else {
        return;
    }

    next = qBound(0, next, m_verses.size() - 1);
    if (next == m_verseIndex)
        return;
    m_verseIndex = next;
    m_selectedIndex = 0;
    m_selectedStrongId.clear();
    m_selectionHistory.clear();
    m_searchStatus.clear();
    m_occurrencePage = 0;
    rebuildState();
}

void BibleData::searchVerse(const QString &query)
{
    const QString wanted = query.trimmed();
    int match = indexForReference(wanted);

    if (match < 0 && !wanted.isEmpty()) {
        const QString textQuery = wanted.toLower();
        for (int i = 0; i < m_verses.size(); ++i) {
            if (m_verses.at(i).text.toLower().contains(textQuery)) {
                match = i;
                break;
            }
        }
    }

    if (match < 0) {
        m_searchStatus = wanted.isEmpty()
                              ? QStringLiteral("Type a reference, e.g. Genesis 1:1")
                              : QStringLiteral("No match in the KJV corpus");
        rebuildState();
        return;
    }

    m_verseIndex = match;
    m_selectedIndex = 0;
    m_selectedStrongId.clear();
    m_selectionHistory.clear();
    m_searchStatus.clear();
    m_occurrencePage = 0;
    rebuildState();
}

void BibleData::navigateTo(const QString &reference)
{
    const int match = indexForReference(reference);
    if (match < 0) {
        searchVerse(reference);
        return;
    }

    m_verseIndex = match;
    m_selectedIndex = 0;
    m_selectedStrongId.clear();
    m_selectionHistory.clear();
    m_searchStatus.clear();
    m_occurrencePage = 0;
    rebuildState();
}
