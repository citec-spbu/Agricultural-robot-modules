#ifndef LOGMANAGER_H
#define LOGMANAGER_H

#include <QString>

namespace LogMsg {
const QString MANAGER_NEW_SPEC        = "Saving spec...";
const QString MANAGER_NEW_DATA        = "New DATA point...";
const QString MANAGER_FAILED_POINT    = "Failed to create point";
const QString MANAGER_FAILED_OBS      = "Failed to create observation";
const QString MANAGER_ML_BEFORE_OBS   = "ML result received before any observation!";
const QString MANAGER_UNKNOWN_TYPE    = "Unknown manager message type:";
}

#endif // LOGMANAGER_H
