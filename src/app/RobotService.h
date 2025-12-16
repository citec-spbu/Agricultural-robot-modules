#ifndef ROBOT_SERVICE_H_
#define ROBOT_SERVICE_H_

#include "Http.h"

using namespace RobotNetwork;

class RobotService : public Http {
    Q_OBJECT
public:
    RobotService() = default;

    QNetworkReply* getSpecification() {
        return get(QUrl("/spec/"));
    }

    QNetworkReply* getCollect() {
        return get(QUrl("/collect/"));
    }

    QNetworkReply* sendCommand(const QJsonValue& val) {
        return postJson(QUrl("/command/"), val);
    }

    QNetworkReply* postStop() {
        return post(QUrl("/stop/"), {}, {});
    }

};

#endif // ! ROBOT_SERVICE_H_