#include "GeoViewToolbar.h"

#include <QVBoxLayout>

GeoViewToolbar::GeoViewToolbar(QObject* parent)
    : QObject(parent)
{
}

void GeoViewToolbar::setManualRouteMode(bool inMode)
{
    if (mManualRouteShortestAction)
        mManualRouteShortestAction->setEnabled(!inMode);
    if (mManualRouteInOrderAction)
        mManualRouteInOrderAction->setEnabled(!inMode);
    if (mManualRouteFinishAction)
        mManualRouteFinishAction->setEnabled(inMode);
    if (mManualRouteButton)
        mManualRouteButton->setText(inMode ? tr("Завершить построение маршрута") : tr("Указать маршрут вручную"));
}

QGroupBox* GeoViewToolbar::createOptionsList(QPushButton** manualRouteButton)
{
    QGroupBox* groupBox = new QGroupBox(tr("Управление"));
    groupBox->setLayout(new QVBoxLayout);

    {
        QPushButton* button = new QPushButton(tr("Разместить робота"));
        QMenu* menu = new QMenu(button);
        QAction* actMap = menu->addAction(tr("Указать на карте"));
        QAction* actFile = menu->addAction(tr("Из конфига"));
        connect(actMap, &QAction::triggered, this, &GeoViewToolbar::setInitialRobotPositionRequested);
        connect(actFile, &QAction::triggered, this, &GeoViewToolbar::setInitialRobotPositionFromFileRequested);
        button->setMenu(menu);
        groupBox->layout()->addWidget(button);
    }

    {
        QPushButton* button = new QPushButton(tr("Добавить контур (GeoJson)"));
        groupBox->layout()->addWidget(button);
        connect(button, &QPushButton::clicked, this, &GeoViewToolbar::addContourRequested);
    }

    {
        QPushButton* button = new QPushButton(tr("Построить параллельный маршрут"));
        QMenu* menu = new QMenu(button);
        QAction* actAuto = menu->addAction(tr("Автоматически"));
        QAction* actAngle = menu->addAction(tr("Угол вручную"));
        connect(actAuto, &QAction::triggered, this, &GeoViewToolbar::buildParallelRouteAutoRequested);
        connect(actAngle, &QAction::triggered, this, &GeoViewToolbar::buildParallelRouteWithAngleRequested);
        button->setMenu(menu);
        groupBox->layout()->addWidget(button);
    }

    {
        QPushButton* button = new QPushButton("Построить команды");
        groupBox->layout()->addWidget(button);
        connect(button, &QPushButton::clicked, this, &GeoViewToolbar::buildCommandsRequested);
    }

    {
        QPushButton* button = new QPushButton(tr("Указать маршрут вручную"));
        QMenu* menu = new QMenu(button);
        mManualRouteShortestAction = menu->addAction(tr("Кратчайший путь"));
        mManualRouteInOrderAction = menu->addAction(tr("По порядку указания"));
        mManualRouteFinishAction = menu->addAction(tr("Завершить построение маршрута"));
        mManualRouteFinishAction->setEnabled(false);
        connect(mManualRouteShortestAction, &QAction::triggered, this, &GeoViewToolbar::startManualRouteShortestRequested);
        connect(mManualRouteInOrderAction, &QAction::triggered, this, &GeoViewToolbar::startManualRouteInOrderRequested);
        connect(mManualRouteFinishAction, &QAction::triggered, this, &GeoViewToolbar::toggleManualRouteRequested);
        button->setMenu(menu);
        mManualRouteButton = button;
        groupBox->layout()->addWidget(button);
        if (manualRouteButton)
            *manualRouteButton = button;
    }

    {
        QPushButton* button = new QPushButton("Удалить контур");
        groupBox->layout()->addWidget(button);
        connect(button, &QPushButton::clicked, this, &GeoViewToolbar::removeContourRequested);
    }

    {
        QPushButton* button = new QPushButton("Очистить все");
        groupBox->layout()->addWidget(button);
        connect(button, &QPushButton::clicked, this, &GeoViewToolbar::clearAllRequested);
    }

    {
        QPushButton* button = new QPushButton(tr("Экспорт маршрута"));
        QMenu* menu = new QMenu(button);
        QAction* actGazebo = menu->addAction(tr("JSON для Gazebo"));
        QAction* actLatLon = menu->addAction(tr("Список координат"));
        connect(actGazebo, &QAction::triggered, this, &GeoViewToolbar::saveGazeboJsonRequested);
        connect(actLatLon, &QAction::triggered, this, &GeoViewToolbar::saveRouteLatLonRequested);
        button->setMenu(menu);
        groupBox->layout()->addWidget(button);
    }

    return groupBox;
}

QGroupBox* GeoViewToolbar::createInfoList(QListWidget*& outInfoList)
{
    QGroupBox* groupBox = new QGroupBox(tr("Info"));
    groupBox->setLayout(new QVBoxLayout);

    QListWidget* list = new QListWidget();
    list->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    list->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    list->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    groupBox->layout()->addWidget(list);

    outInfoList = list;
    return groupBox;
}
