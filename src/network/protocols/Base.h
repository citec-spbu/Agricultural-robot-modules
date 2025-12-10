#ifndef ROBOT_NETWORK_BASE_TRANSPORT_H_
#define ROBOT_NETWORK_BASE_TRANSPORT_H_

#include <QObject>
#include <QUrl>

namespace RobotNetwork {

enum class Capability : quint32 {
    None       = 0,
    ByteStream = 1u<<0,
    Datagram   = 1u<<1,
    Http       = 1u<<2
};
Q_DECLARE_FLAGS(Capabilities, Capability)
Q_DECLARE_OPERATORS_FOR_FLAGS(Capabilities)


class BaseConnection : public QObject {
    Q_OBJECT
public:
    using QObject::QObject;
    virtual ~BaseConnection() = default;

    virtual qint64 send(const QByteArray& payload) {
        Q_UNUSED(payload);
        return -1;
    };

    virtual void get(const QUrl& url) {
        emit error("GET not supported");
    }

    virtual void post(const QUrl& url, const QByteArray& body, const QString& contentType) {
        emit error("POST not supported");
    }

    virtual Capabilities caps() const = 0;

    virtual void open(const QUrl& url) = 0;
    virtual void close() = 0;
    virtual bool isOpen() const = 0;

signals:
    void connected();
    void disconnected();
    void received(QByteArray data);
    void error(QString msg);
};

}

#endif // ! ROBOT_NETWORK_BASE_TRANSPORT_H_