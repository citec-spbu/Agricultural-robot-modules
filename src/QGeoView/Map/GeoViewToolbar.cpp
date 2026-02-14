#include "GeoViewToolbar.h"

#include <QVBoxLayout>

GeoViewToolbar::GeoViewToolbar(QObject* parent)
    : QObject(parent)
{
}

QGroupBox* GeoViewToolbar::createOptionsList(QPushButton** manualRouteButton)
{
    QGroupBox* groupBox = new QGroupBox(tr("Управление"));
    groupBox->setLayout(new QVBoxLayout);

    {
        QPushButton* button = new QPushButton("Задать начальную позицию робота");
        groupBox->layout()->addWidget(button);
        connect(button, &QPushButton::clicked, this, &GeoViewToolbar::setInitialRobotPositionRequested);
    }

    {
        QPushButton* button = new QPushButton("Добавить контур (GeoJson)");
        groupBox->layout()->addWidget(button);
        connect(button, &QPushButton::clicked, this, &GeoViewToolbar::addContourRequested);
    }

    {
        QPushButton* button = new QPushButton("Построить команды");
        groupBox->layout()->addWidget(button);
        connect(button, &QPushButton::clicked, this, &GeoViewToolbar::buildCommandsRequested);
    }

    {
        QPushButton* button = new QPushButton("Указать маршрут вручную");
        groupBox->layout()->addWidget(button);
        if (manualRouteButton)
            *manualRouteButton = button;
        connect(button, &QPushButton::clicked, this, &GeoViewToolbar::toggleManualRouteRequested);
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
        QPushButton* button = new QPushButton("Сгенерировать json для симуляции в Gazebo");
        groupBox->layout()->addWidget(button);
        connect(button, &QPushButton::clicked, this, &GeoViewToolbar::saveGazeboJsonRequested);
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
