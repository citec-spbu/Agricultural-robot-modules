#ifndef ROBOT_NETWORK_TCP_H_
#define ROBOT_NETWORK_TCP_H_

#include <QObject>
#include <QTcpSocket>

#include "Base.h"

namespace RobotNetwork {

class Tcp : public BaseConnection {
    Q_OBJECT
public:
    explicit Tcp(QObject* parent = nullptr);

    Capabilities caps() const override;

    void open(const QUrl& target) override;
    void open(const QString& host, quint16 port);
    void close() override;
    bool isOpen() const override;
    qint64 send(const QByteArray& bytes) override;

private:
    QTcpSocket sock_{};
    QString host_{};
    quint16 port_{};
};

}

#endif // ! ROBOT_NETWORK_TCP_H_