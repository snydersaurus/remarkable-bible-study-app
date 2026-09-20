#pragma once

#include <QObject>
#include <QByteArray>

class QSocketNotifier;

/* Backend side of AppLoad's SOCK_SEQPACKET frontend/backend link. */
class AppLoadLink : public QObject
{
    Q_OBJECT

public:
    static constexpr quint32 MsgTerminate      = 0xFFFFFFFFu;
    static constexpr quint32 MsgNewCoordinator = 0xFFFFFFFEu;
    static constexpr int MaxPacket = 10485760;

    explicit AppLoadLink(QObject *parent = nullptr);
    ~AppLoadLink() override;

    bool connectTo(const QString &socketPath);
    bool send(quint32 type, const QByteArray &payload);

signals:
    void messageReceived(quint32 type, const QByteArray &payload);
    void disconnected();

private:
    void readReady();

    int m_fd = -1;
    QSocketNotifier *m_notifier = nullptr;
};
