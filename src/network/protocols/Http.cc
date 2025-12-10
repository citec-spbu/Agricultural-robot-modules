#include "Http.h"

namespace RobotNetwork {

Http::Http(QObject* parent) : BaseConnection(parent) {}

Capabilities Http::caps() const {
    return Capability::ByteStream;
}

void Http::open(const QUrl& url) {
    baseUrl_ = url;
    opened_  = baseUrl_.isValid();
    if (!opened_) {
        emit error("invalid base url");
        return;
    }
    emit connected();
}

void Http::open(const QString& url) {
    open(QUrl(url));
}

void Http::close() {
    opened_ = false;
    emit disconnected();
}

bool Http::isOpen() const {
    return opened_;
}

void Http::handleReply(QNetworkReply* reply) {
    connect(reply, &QNetworkReply::finished, this, [this, reply]() {
        QByteArray data = reply->readAll();

        if (reply->error() != QNetworkReply::NoError) {
            emit error(reply->errorString());
            reply->deleteLater();
            return;
        }
        emit received(data);

        QJsonParseError perr;
        QJsonDocument doc = QJsonDocument::fromJson(data, &perr);
        if (perr.error == QJsonParseError::NoError) {
            emit jsonReceived(doc);
        }

        reply->deleteLater();
    });
}

void Http::get(const QUrl& url) {
    QUrl u;
    if (url.isRelative() && baseUrl_.isValid()) {
        u = baseUrl_.resolved(url);
    }
    else {
        u = url;
    }

    if (!u.isValid()) {
        emit error("invalid url");
        return;
    }

    QNetworkRequest req(u);
    QNetworkReply* reply = manager_.get(req);
    handleReply(reply);
}

void Http::post(const QUrl& url, const QByteArray& body, const QString& contentType) {
    QUrl u;
    if (url.isRelative() && baseUrl_.isValid()) {
        u = baseUrl_.resolved(url);
    }
    else {
        u = url;
    }

    if (!u.isValid()) {
        emit error("invalid url");
        return;
    }

    QNetworkRequest req(u);
    if (!contentType.isEmpty()) {
        req.setHeader(QNetworkRequest::ContentTypeHeader, contentType);
    }

    QNetworkReply* reply = manager_.post(req, body);
    handleReply(reply);
}

qint64 Http::send(const QByteArray& payload) {
    if (!opened_ || !baseUrl_.isValid()) {
        emit error("HTTP not opened");
        return -1;
    }

    post(baseUrl_, payload, "application/octet-stream");
    return payload.size();
}

void Http::postJson(const QUrl& url, const QJsonValue& json) {
    QJsonDocument doc = json.isObject()
                        ? QJsonDocument(json.toObject())
                        : QJsonDocument::fromVariant(json.toVariant());
    QByteArray payload = doc.toJson(QJsonDocument::Compact);
    post(url, payload, "application/json");
}

void Http::getJson(const QUrl& url) {
    get(url);
}


}