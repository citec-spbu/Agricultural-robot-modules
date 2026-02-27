#pragma once

#include <QAction>
#include <QGroupBox>
#include <QListWidget>
#include <QMenu>
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

    void setManualRouteMode(bool inMode);

signals:
    void setInitialRobotPositionRequested();
    void setInitialRobotPositionFromFileRequested();
    void addContourRequested();
    void buildParallelRouteAutoRequested();
    void buildParallelRouteWithAngleRequested();
    void buildCommandsRequested();
    void startManualRouteShortestRequested();
    void startManualRouteInOrderRequested();
    void toggleManualRouteRequested();
    void removeContourRequested();
    void clearAllRequested();
    void saveGazeboJsonRequested();

private:
    QPushButton* mManualRouteButton = nullptr;
    QAction* mManualRouteShortestAction = nullptr;
    QAction* mManualRouteInOrderAction = nullptr;
    QAction* mManualRouteFinishAction = nullptr;

public:
    QGroupBox* createInfoList(QListWidget*& outInfoList);
};
