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

    Capabilities caps() const override;

    void open(const QUrl& url) override;
    void open(const QString& url);
    void close() override;
    bool isOpen() const override; // always false

    qint64 send(const QByteArray& payload) override;
    QNetworkReply* get(const QUrl& url) override;
    QNetworkReply* post(const QUrl& url, const QByteArray& body, const QString& contentType) override;
    QNetworkReply* postJson(const QUrl& url, const QJsonValue& json);
    QNetworkReply* getJson(const QUrl& url);

signals:
    void jsonReceived(const QJsonDocument& doc);

    void replyReceived(QNetworkReply* reply, const QByteArray& data);
    void replyJsonReceived(QNetworkReply* reply, const QJsonDocument& doc);
    void replyError(QNetworkReply* reply, const QString& message);

private:
    QNetworkAccessManager manager_;
    QUrl baseUrl_;
    bool opened_ = false;

    void handleReply(QNetworkReply* reply);
};

}  // namespace RobotNetwork

#endif // ! ROBOT_NETWORK_HTTP_H_