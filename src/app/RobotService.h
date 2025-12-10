#ifndef ROBOT_SERVICE_H_
#define ROBOT_SERVICE_H_

#include "Http.h"

using namespace RobotNetwork;

class RobotService : public Http {
public:
    RobotService() = default;

    void getSpec() {
        getJson(QUrl("/spec/"));
    }
private:
};

#endif // ! ROBOT_SERVICE_H_