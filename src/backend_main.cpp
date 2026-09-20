#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>

#include "AppLoadLink.h"
#include "BibleData.h"

namespace {
constexpr quint32 MsgState = 101;
constexpr quint32 MsgHello = 1;
constexpr quint32 MsgSelectWord = 2;
constexpr quint32 MsgNavigate = 3;
constexpr quint32 MsgGeometry = 4;
constexpr quint32 MsgSelectStrong = 5;
constexpr quint32 MsgGoBack = 6;
constexpr quint32 MsgSearchVerse = 7;
constexpr quint32 MsgNavigateTo = 9;
constexpr quint32 MsgOccurrencePageSize = 10;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("word-study-backend"));

    if (argc < 2) {
        qWarning("usage: entry <appload-socket>");
        return 2;
    }

    AppLoadLink link;
    if (!link.connectTo(QString::fromLocal8Bit(argv[1])))
        return 1;

    BibleData data;
    const auto push = [&]() {
        const QByteArray json = QJsonDocument(
            QJsonObject::fromVariantMap(data.state()))
            .toJson(QJsonDocument::Compact);
        link.send(MsgState, json);
    };

    QObject::connect(&data, &BibleData::stateChanged, &app, push);
    QObject::connect(&link, &AppLoadLink::messageReceived, &app,
                     [&](quint32 type, const QByteArray &payload) {
        switch (type) {
        case AppLoadLink::MsgNewCoordinator:
        case MsgHello:
            push();
            break;
        case MsgSelectWord:
            data.selectIndex(payload.trimmed().toInt());
            break;
        case MsgSelectStrong:
            data.selectStrongId(QString::fromLatin1(payload.trimmed()));
            break;
        case MsgGoBack:
            data.goBack();
            break;
        case MsgSearchVerse:
            data.searchVerse(QString::fromUtf8(payload));
            break;
        case MsgNavigateTo:
            data.navigateTo(QString::fromUtf8(payload));
            break;
        case MsgNavigate:
            data.navigate(QString::fromLatin1(payload.trimmed()));
            break;
        case MsgOccurrencePageSize:
            data.setOccurrencePageSize(payload.trimmed().toInt());
            break;
        case MsgGeometry:
            qInfo("frontend window: %s", payload.constData());
            break;
        case AppLoadLink::MsgTerminate:
            app.quit();
            break;
        default:
            break;
        }
    });
    QObject::connect(&link, &AppLoadLink::disconnected,
                     &app, &QCoreApplication::quit);

    push();
    return app.exec();
}
