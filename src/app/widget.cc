#include "widget.h"
#include "widget.h"

#include <QStackedWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QLabel>
#include <QFileDialog>
#include <QDebug>

Widget::Widget(GeoViewWidget* m, QWidget *parent)
    : map(m), QWidget(parent) {
    setupUi();
}

Widget::~Widget() {}

void Widget::setupUi()
{
    auto *mainLayout = new QHBoxLayout(this);

    // Left bar
    auto *stepsLayout = new QVBoxLayout;
    auto *btn1 = new QPushButton("Робот");
    auto *btn2 = new QPushButton("ML сервис");
    auto *btn8 = new QPushButton("Карта");

    stepsLayout->addWidget(btn1);
    stepsLayout->addWidget(btn2);
    stepsLayout->addWidget(btn8);
    stepsLayout->addStretch();

    auto *stepsWidget = new QWidget(this);
    stepsWidget->setLayout(stepsLayout);

    // Right pages
    m_stack = new QStackedWidget(this);
    m_stack->addWidget(createRobotPage());    // 0
    m_stack->addWidget(createMLPage());       // 1
    m_stack->addWidget(map);    // 2

    mainLayout->addWidget(stepsWidget);
    mainLayout->addWidget(m_stack, 1);

    connect(btn1, &QPushButton::clicked, this, &Widget::onStepRobot);
    connect(btn2, &QPushButton::clicked, this, &Widget::onStepML);
    connect(btn8, &QPushButton::clicked, this, &Widget::onStepMap);
}




// ---------------- Create pages widgets

QWidget* Widget::createRobotPage()
{
    auto *w = new QWidget;
    auto *l = new QVBoxLayout(w);

    m_robotHostEdit = new QLineEdit("127.0.0.1");
    m_robotPortEdit = new QLineEdit("8000");

    auto *btnConn = new QPushButton("Подключится");
    auto *btnSpec = new QPushButton("Получить спецификацию");

    m_robotSpecView = new QPlainTextEdit;
    m_robotSpecView->setReadOnly(true);

    l->addWidget(new QLabel("Адрес:"));
    l->addWidget(m_robotHostEdit);
    l->addWidget(new QLabel("Порт:"));
    l->addWidget(m_robotPortEdit);
    l->addWidget(btnConn);
    l->addWidget(btnSpec);
    l->addWidget(new QLabel("Info:"));
    l->addWidget(m_robotSpecView, 1);

    connect(btnConn, &QPushButton::clicked, this, &Widget::onConnectRobotClicked);
    connect(btnSpec, &QPushButton::clicked, this, &Widget::onFetchSpecClicked);

    return w;
}

QWidget* Widget::createMLPage()
{
    auto *w = new QWidget;
    auto *l = new QVBoxLayout(w);

    m_mlHostEdit = new QLineEdit("127.0.0.1");
    m_mlPortEdit = new QLineEdit("8001");
    auto *btnSave = new QPushButton("Подключится");

    l->addWidget(new QLabel("Адрес:"));
    l->addWidget(m_mlHostEdit);
    l->addWidget(new QLabel("Порт:"));
    l->addWidget(m_mlPortEdit);
    l->addWidget(btnSave);
    l->addStretch();

    connect(btnSave, &QPushButton::clicked, this, &Widget::onConnectMLClicked);

    return w;
}

QWidget* Widget::createMapWidget() {
    return new GeoViewWidget();
}




// -------------------- Navigation

void Widget::onStepRobot()    { m_stack->setCurrentIndex(0); }
void Widget::onStepML()       { m_stack->setCurrentIndex(1); }
void Widget::onStepMap()      { m_stack->setCurrentIndex(2); }




// --------------- Slots

void Widget::onConnectRobotClicked()
{
    m_robotInterface.host = m_robotHostEdit->text();
    m_robotInterface.port = m_robotPortEdit->text().toUShort();

    emit ConnectRobotClicked(m_robotInterface.host, m_robotInterface.port);
}

void Widget::onFetchSpecClicked()
{
    emit FetchSpecClicked();
}

void Widget::onConnectMLClicked()
{
    m_mlInterface.host = m_mlHostEdit->text();
    m_mlInterface.port = m_mlPortEdit->text().toUShort();

    emit ConnectMLClicked(m_mlInterface.host, m_mlInterface.port);
}

//void Widget::onLoadContourClicked()
//{
//    const QString file = QFileDialog::getOpenFileName(this, "Выберите файл контура");
//    if (file.isEmpty())
//        return;
//
//    m_contourFilePath = file;
//    //log("Contour file selected: " + file);
//}
//
//void Widget::onBuildRouteClicked()
//{
//    //log("Build route signal (stub).");
//
//    // позже: вызов модуля построения маршрута
//}
//
//void Widget::onViewCommandsClicked()
//{
//    // заглушка json с командами
//    const QString dummy = R"({
//  "route_id": "123",
//  "commands": [
//    {"bla": "bla", "bla": 99},
//    {"bla": "bla", "bla": 99},
//    {"bla": "bla", "bla": 99},
//    {"bla": "bla", "bla": 99},
//    {"bla": "bla", "bla": 99},
//  ]
//})";
//
//    if (m_commandsView)
//        m_commandsView->setPlainText(dummy);
//}
//
//void Widget::onStartWorkClicked()
//{
//    //log("Start work signal (stub).");
//}
//
//void Widget::onRequestResultsClicked()
//{
//    // заглушка результатов
//    const QString dummy = R"({
//  "status": "ok",
//  "status": "ok",
//  "status": "ok",
//  "status": "ok",
//  "status": "ok",
//  "status": "ok",
//  "status": "ok",
//})";
//
//    if (m_resultsView)
//        m_resultsView->setPlainText(dummy);
//}
//
//

// --------------- Methods

void Widget::showRobotSpec(const QString &json) {
    m_robotSpecView->setPlainText(json);
}
