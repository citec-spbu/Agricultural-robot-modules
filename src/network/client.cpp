#include <QCoreApplication>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include "protocols/Base.h"
#include "protocols/Http.h"
#include "protocols/Tcp.h"
#include "protocols/Udp.h"

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);

    // Check Http
    RobotNetwork::Http http;
    http.open(QUrl("http://127.0.0.1:8000"));

    QJsonObject root;
    QByteArray imgData = "4AAQSkZJRgABAQAAAQABA";
    QByteArray base64 = imgData.toBase64();
    root["image"] = QString::fromUtf8(base64);
    QJsonObject meta;
    meta["user"] = "test";
    meta["other_info"] = 123;
    root["metadata"] = meta;

    http.postJson(QUrl("/detect/"), root);

    // One line way
    //http.post(QUrl("http://127.0.0.1:8000/detect/"), payload, "application/json");

    QObject::connect(&http, &RobotNetwork::BaseConnection::received, [](const QByteArray& data) {
        qInfo() << data;
    });





    /* Check Qt http connection
    QNetworkAccessManager manager;

    QObject::connect(&manager, &QNetworkAccessManager::finished,
                     [](QNetworkReply* reply) {
        qDebug().noquote() << reply->readAll();
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray resp = reply->readAll();
            qInfo() << "Good";
        } else {
            qWarning() << "Network error:" << reply->error()
                       << reply->errorString();
            reply->deleteLater();
            return;
        }
        reply->deleteLater();
    });

    QJsonObject root;
    QByteArray imgData = "4AAQSkZJRgABAQAAAQABA";
    QByteArray base64 = imgData.toBase64();
    root["image"] = QString::fromUtf8(base64);
    QJsonObject meta;
    meta["user"] = "test";
    meta["other_info"] = 123;
    root["metadata"] = meta;

    QByteArray payload = QJsonDocument(root).toJson();

    QNetworkRequest req(QUrl("http://127.0.0.1:8000/detect/"));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    manager.post(req, payload);
    */




    /* Check Udp
    RobotNetwork::Udp udp;
    udp.open("127.0.0.1", 12345);
    udp.send("Hello, Udp!");
    */



    /* Check Tcp
    RobotNetwork::Tcp tcp;
    tcp.open("127.0.0.1", 12345);
    if (!tcp.isOpen()) {
        return 1;
    }
    tcp.send("Hello world!");
    */

    return app.exec();
}

