#ifndef ROBOT_NETWORK_HTTP_H_
#define ROBOT_NETWORK_HTTP_H_

#include <QHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QObject>
#include <QPair>
#include <QPointer>
#include <QSslError>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>

#include "Base.h"

namespace RobotNetwork {

class Http : public BaseConnection {
    Q_OBJECT
public:
    explicit Http(QObject* parent = nullptr);

    qint64 send(const QByteArray& payload) override;

    void get(const QUrl& url) override;
    void post(const QUrl& url, const QByteArray& body, const QString& contentType) override;

    Capabilities caps() const override;

    void open(const QUrl& url) override;
    void open(const QString& url);
    void close() override;
    bool isOpen() const override; // always false

    void postJson(const QUrl& url, const QJsonValue& json);
    void getJson(const QUrl& url);

signals:
    void jsonReceived(const QJsonDocument& doc);

private:
    QNetworkAccessManager manager_;
    QUrl baseUrl_;
    bool opened_ = false;

    void handleReply(QNetworkReply* reply);
};

}  // namespace RobotNetwork

#endif // ! ROBOT_NETWORK_HTTP_H_