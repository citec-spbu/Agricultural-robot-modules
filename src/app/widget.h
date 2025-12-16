#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QString>

#include "GeoViewWidget.h"

class QStackedWidget;
class QLineEdit;
class QPlainTextEdit;

struct InterfaceConfig {
    QString host{};
    quint16 port{};
};

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(GeoViewWidget* map, QWidget *parent = nullptr);
    ~Widget();

private slots:
    void onStepRobot();
    void onStepML();
    void onStepMap();

    void onConnectRobotClicked();
    void onFetchSpecClicked();
    void onConnectMLClicked();

public:
    void showRobotSpec(const QString& json);

public:
signals:
    void ConnectRobotClicked(QString robotHost, ushort robotPort);
    void FetchSpecClicked();
    void ConnectMLClicked(QString MLHost, ushort MLPort);

private:
    void setupUi();

    QWidget* createRobotPage();
    QWidget* createMLPage();
    QWidget* createMapWidget();

private:

    QStackedWidget* m_stack = nullptr;

    // Robot page
    QLineEdit* m_robotHostEdit = nullptr;
    QLineEdit* m_robotPortEdit = nullptr;
    QPlainTextEdit* m_robotSpecView = nullptr;

    // ML page
    QLineEdit* m_mlHostEdit = nullptr;
    QLineEdit* m_mlPortEdit = nullptr;

    // Commands page
    QPlainTextEdit* m_commandsView = nullptr;

    // Results page
    QPlainTextEdit* m_resultsView = nullptr;

    // State
    InterfaceConfig m_robotInterface;
    InterfaceConfig m_mlInterface;
    QString m_contourFilePath;

    // Map widget
    GeoViewWidget* map;
};

#endif // WIDGET_H
