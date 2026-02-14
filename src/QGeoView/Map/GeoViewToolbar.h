#pragma once

#include <QGroupBox>
#include <QListWidget>
#include <QObject>
#include <QPushButton>

/**
 * Панель управления: кнопки и информационный список.
 * Испускает сигналы при нажатии; обработка — в GeoViewWidget.
 */
class GeoViewToolbar : public QObject
{
    Q_OBJECT

public:
    explicit GeoViewToolbar(QObject* parent = nullptr);

    /** Создаёт группу с кнопками управления. manualRouteButton — опционально, чтобы виджет мог менять текст при переключении режима. */
    QGroupBox* createOptionsList(QPushButton** manualRouteButton = nullptr);

    /** Создаёт группу с информационным списком. outInfoList — указатель для обновления из виджета. */
    QGroupBox* createInfoList(QListWidget*& outInfoList);

signals:
    void setInitialRobotPositionRequested();
    void addContourRequested();
    void buildCommandsRequested();
    void toggleManualRouteRequested();
    void removeContourRequested();
    void clearAllRequested();
    void saveGazeboJsonRequested();
};
