#ifndef ML_SERVICE_H
#define ML_SERVICE_H

#include "Http.h"

using namespace RobotNetwork;

class MLService : public Http {
public:
    MLService() = default;

    QNetworkReply* postDetect(const QJsonObject& obj) {
        return postJson(QUrl("/detect/"), obj);
    }
private:
};

#endif // ! ML_SERVICE_H