#include "AppLoadLink.h"

#include <QDebug>
#include <QSocketNotifier>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>

namespace {
struct MessageHeader {
    quint32 type;
    quint32 length;
};
}

AppLoadLink::AppLoadLink(QObject *parent) : QObject(parent) {}

AppLoadLink::~AppLoadLink()
{
    if (m_fd >= 0)
        ::close(m_fd);
}

bool AppLoadLink::connectTo(const QString &socketPath)
{
    m_fd = ::socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if (m_fd < 0) {
        qWarning("appload: socket() failed: %s", strerror(errno));
        return false;
    }

    sockaddr_un address{};
    address.sun_family = AF_UNIX;
    const QByteArray path = socketPath.toUtf8();
    if (path.size() >= int(sizeof(address.sun_path))) {
        qWarning("appload: socket path too long");
        return false;
    }
    std::memcpy(address.sun_path, path.constData(), size_t(path.size()));

    if (::connect(m_fd, reinterpret_cast<sockaddr *>(&address), sizeof(address)) != 0) {
        qWarning("appload: connect(%s) failed: %s", path.constData(), strerror(errno));
        return false;
    }

    m_notifier = new QSocketNotifier(m_fd, QSocketNotifier::Read, this);
    connect(m_notifier, &QSocketNotifier::activated,
            this, &AppLoadLink::readReady);
    return true;
}

bool AppLoadLink::send(quint32 type, const QByteArray &payload)
{
    if (m_fd < 0)
        return false;

    const MessageHeader header{type, quint32(payload.size())};
    if (::send(m_fd, &header, sizeof(header), 0) < 0) {
        qWarning("appload: send header type=%u failed: %s", type, strerror(errno));
        return false;
    }
    if (!payload.isEmpty()
        && ::send(m_fd, payload.constData(), payload.size(), 0) < 0) {
        qWarning("appload: send payload failed: %s", strerror(errno));
        return false;
    }
    return true;
}

void AppLoadLink::readReady()
{
    MessageHeader header{};
    const ssize_t got = ::recv(m_fd, &header, sizeof(header), 0);
    if (got < 1) {
        m_notifier->setEnabled(false);
        emit disconnected();
        return;
    }

    if (header.length > quint32(MaxPacket)) {
        qWarning("appload: oversized message (%u)", header.length);
        emit disconnected();
        return;
    }

    // AppLoad emits a second datagram for the payload, including an empty one.
    QByteArray payload(int(header.length), Qt::Uninitialized);
    const ssize_t gotPayload = ::recv(m_fd, payload.data(), header.length, 0);
    if (gotPayload < 0 || (gotPayload < 1 && header.length != 0)) {
        emit disconnected();
        return;
    }
    emit messageReceived(header.type, payload);
}
