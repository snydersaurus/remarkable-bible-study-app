#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickWindow>
#include <QTimer>
#include <QImage>
#include <QUrl>

#include "BibleData.h"

namespace {
QString option(const QStringList &args, const QString &name)
{
    const int index = args.indexOf(name);
    return index >= 0 && index + 1 < args.size() ? args.at(index + 1) : QString();
}
}

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("word-study"));

    const QStringList args = app.arguments();
    const QString shotPath = option(args, QStringLiteral("--shot"));
    const QString panel = option(args, QStringLiteral("--panel"));
    const bool landscape = option(args, QStringLiteral("--orient"))
                           == QStringLiteral("landscape");

    int panelW = 954;
    int panelH = 1696;
    const QStringList dimensions = panel.split(QLatin1Char('x'));
    if (dimensions.size() == 2) {
        panelW = dimensions.at(0).toInt();
        panelH = dimensions.at(1).toInt();
    }

    BibleData data;
    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty(QStringLiteral("feed"), &data);
    engine.rootContext()->setContextProperty(QStringLiteral("panelW"), panelW);
    engine.rootContext()->setContextProperty(QStringLiteral("panelH"), panelH);
    engine.rootContext()->setContextProperty(QStringLiteral("landscape"), landscape);
    engine.rootContext()->setContextProperty(QStringLiteral("shotMode"),
                                              !shotPath.isEmpty());

#if QT_VERSION >= QT_VERSION_CHECK(6, 5, 0)
    engine.loadFromModule("BibleStudy", "Preview");
#else
    engine.load(QUrl(QStringLiteral("qrc:/qt/qml/BibleStudy/Preview.qml")));
#endif
    if (engine.rootObjects().isEmpty())
        return -1;

    if (!shotPath.isEmpty()) {
        auto *window = qobject_cast<QQuickWindow *>(engine.rootObjects().first());
        if (!window)
            return -1;
        QTimer::singleShot(700, &app, [window, shotPath]() {
            const QImage image = window->grabWindow();
            if (image.isNull() || !image.save(shotPath)) {
                qWarning("could not write %s", qPrintable(shotPath));
                QGuiApplication::exit(1);
                return;
            }
            qInfo("wrote %s (%dx%d)", qPrintable(shotPath),
                  image.width(), image.height());
            QGuiApplication::quit();
        });
    }

    return app.exec();
}
