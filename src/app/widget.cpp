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

Widget::Widget(QWidget *parent)
    : QWidget(parent) {
    setupUi();
}

Widget::~Widget() {}

void Widget::setupUi()
{
    auto *mainLayout = new QHBoxLayout(this);

    // Left bar
    auto *stepsLayout = new QVBoxLayout;
    auto *btn1 = new QPushButton("1. Робот");
    auto *btn2 = new QPushButton("2. ML сервис");
    auto *btn3 = new QPushButton("3. Контур");
    auto *btn4 = new QPushButton("4. Маршрут");
    auto *btn5 = new QPushButton("5. Команды");
    auto *btn6 = new QPushButton("6. Старт");
    auto *btn7 = new QPushButton("7. Результаты");

    stepsLayout->addWidget(btn1);
    stepsLayout->addWidget(btn2);
    stepsLayout->addWidget(btn3);
    stepsLayout->addWidget(btn4);
    stepsLayout->addWidget(btn5);
    stepsLayout->addWidget(btn6);
    stepsLayout->addWidget(btn7);
    stepsLayout->addStretch();

    auto *stepsWidget = new QWidget(this);
    stepsWidget->setLayout(stepsLayout);

    // Right pages
    m_stack = new QStackedWidget(this);
    m_stack->addWidget(createRobotPage());    // 0
    m_stack->addWidget(createMLPage());       // 1
    m_stack->addWidget(createContourPage());  // 2
    m_stack->addWidget(createRoutePage());    // 3
    m_stack->addWidget(createCommandsPage()); // 4
    m_stack->addWidget(createStartPage());    // 5
    m_stack->addWidget(createResultsPage());  // 6

    mainLayout->addWidget(stepsWidget);
    mainLayout->addWidget(m_stack, 1);

    connect(btn1, &QPushButton::clicked, this, &Widget::onStepRobot);
    connect(btn2, &QPushButton::clicked, this, &Widget::onStepML);
    connect(btn3, &QPushButton::clicked, this, &Widget::onStepContour);
    connect(btn4, &QPushButton::clicked, this, &Widget::onStepRoute);
    connect(btn5, &QPushButton::clicked, this, &Widget::onStepCommands);
    connect(btn6, &QPushButton::clicked, this, &Widget::onStepStart);
    connect(btn7, &QPushButton::clicked, this, &Widget::onStepResults);
}




// ---------------- Pages

QWidget* Widget::createRobotPage()
{
    auto *w = new QWidget;
    auto *l = new QVBoxLayout(w);

    m_robotHostEdit = new QLineEdit;
    m_robotPortEdit = new QLineEdit;

    auto *btnConn = new QPushButton("Подключиться");
    auto *btnSpec = new QPushButton("Получить спецификацию");

    m_robotSpecView = new QPlainTextEdit;
    m_robotSpecView->setReadOnly(true);

    l->addWidget(new QLabel("Адрес робота:"));
    l->addWidget(m_robotHostEdit);
    l->addWidget(new QLabel("Порт:"));
    l->addWidget(m_robotPortEdit);
    l->addWidget(btnConn);
    l->addWidget(btnSpec);
    l->addWidget(new QLabel("Спецификация (JSON):"));
    l->addWidget(m_robotSpecView, 1);

    connect(btnConn, &QPushButton::clicked, this, &Widget::onConnectRobotClicked);
    connect(btnSpec, &QPushButton::clicked, this, &Widget::onFetchSpecClicked);

    return w;
}

QWidget* Widget::createMLPage()
{
    auto *w = new QWidget;
    auto *l = new QVBoxLayout(w);

    m_mlHostEdit = new QLineEdit;
    m_mlPortEdit = new QLineEdit;
    auto *btnSave = new QPushButton("Сохранить");

    l->addWidget(new QLabel("Адрес ML сервиса:"));
    l->addWidget(m_mlHostEdit);
    l->addWidget(new QLabel("Порт:"));
    l->addWidget(m_mlPortEdit);
    l->addWidget(btnSave);
    l->addStretch();

    connect(btnSave, &QPushButton::clicked, this, &Widget::onSaveMLClicked);

    return w;
}

QWidget* Widget::createContourPage()
{
    auto *w = new QWidget;
    auto *l = new QVBoxLayout(w);

    auto *btnLoad = new QPushButton("Загрузить файл контура…");
    l->addWidget(btnLoad);
    l->addStretch();

    connect(btnLoad, &QPushButton::clicked, this, &Widget::onLoadContourClicked);

    return w;
}

QWidget* Widget::createRoutePage()
{
    auto *w = new QWidget;
    auto *l = new QVBoxLayout(w);

    auto *btn = new QPushButton("Построить маршрут");
    l->addWidget(btn);
    l->addStretch();

    connect(btn, &QPushButton::clicked, this, &Widget::onBuildRouteClicked);

    return w;
}

QWidget* Widget::createCommandsPage()
{
    auto *w = new QWidget;
    auto *l = new QVBoxLayout(w);

    auto *btn = new QPushButton("Показать команды");
    m_commandsView = new QPlainTextEdit;
    m_commandsView->setReadOnly(true);

    l->addWidget(btn);
    l->addWidget(m_commandsView, 1);

    connect(btn, &QPushButton::clicked, this, &Widget::onViewCommandsClicked);

    return w;
}

QWidget* Widget::createStartPage()
{
    auto *w = new QWidget;
    auto *l = new QVBoxLayout(w);

    auto *btn = new QPushButton("Начать работу");
    l->addWidget(btn);
    l->addStretch();

    connect(btn, &QPushButton::clicked, this, &Widget::onStartWorkClicked);

    return w;
}

QWidget* Widget::createResultsPage()
{
    auto *w = new QWidget;
    auto *l = new QVBoxLayout(w);

    auto *btn = new QPushButton("Запросить результаты");
    m_resultsView = new QPlainTextEdit;
    m_resultsView->setReadOnly(true);

    l->addWidget(btn);
    l->addWidget(m_resultsView, 1);

    connect(btn, &QPushButton::clicked, this, &Widget::onRequestResultsClicked);

    return w;
}



// -------------------- Navigation

void Widget::onStepRobot()    { m_stack->setCurrentIndex(0); }
void Widget::onStepML()       { m_stack->setCurrentIndex(1); }
void Widget::onStepContour()  { m_stack->setCurrentIndex(2); }
void Widget::onStepRoute()    { m_stack->setCurrentIndex(3); }
void Widget::onStepCommands() { m_stack->setCurrentIndex(4); }
void Widget::onStepStart()    { m_stack->setCurrentIndex(5); }
void Widget::onStepResults()  { m_stack->setCurrentIndex(6); }




// --------------- Logic

void Widget::onConnectRobotClicked()
{
    m_robotInterface.host = m_robotHostEdit->text();
    m_robotInterface.port = m_robotPortEdit->text().toUShort();

    emit ConnectRobotClicked(m_robotHostEdit->text(), m_robotPortEdit->text().toUShort());
}

void Widget::onFetchSpecClicked()
{
    emit FetchSpecClicked();
}

void Widget::onSaveMLClicked()
{
    m_mlInterface.host = m_mlHostEdit->text();
    m_mlInterface.port = m_mlPortEdit->text().toUShort();

    //log("ML interface: " + m_mlInterface.host
       // + ":" + QString::number(m_mlInterface.port));
}

void Widget::onLoadContourClicked()
{
    const QString file = QFileDialog::getOpenFileName(this, "Выберите файл контура");
    if (file.isEmpty())
        return;

    m_contourFilePath = file;
    //log("Contour file selected: " + file);
}

void Widget::onBuildRouteClicked()
{
    //log("Build route signal (stub).");

    // позже: вызов модуля построения маршрута
}

void Widget::onViewCommandsClicked()
{
    // заглушка json с командами
    const QString dummy = R"({
  "route_id": "123",
  "commands": [
    {"bla": "bla", "bla": 99},
    {"bla": "bla", "bla": 99},
    {"bla": "bla", "bla": 99},
    {"bla": "bla", "bla": 99},
    {"bla": "bla", "bla": 99},
  ]
})";

    if (m_commandsView)
        m_commandsView->setPlainText(dummy);
}

void Widget::onStartWorkClicked()
{
    //log("Start work signal (stub).");
}

void Widget::onRequestResultsClicked()
{
    // заглушка результатов
    const QString dummy = R"({
  "status": "ok",
  "status": "ok",
  "status": "ok",
  "status": "ok",
  "status": "ok",
  "status": "ok",
  "status": "ok",
})";

    if (m_resultsView)
        m_resultsView->setPlainText(dummy);
}




void Widget::showRobotSpec(const QString &json) {
    m_robotSpecView->setPlainText(json);
}
