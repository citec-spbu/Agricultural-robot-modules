#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QString>

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
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

private slots:
    void onStepRobot();
    void onStepML();
    void onStepContour();
    void onStepRoute();
    void onStepCommands();
    void onStepStart();
    void onStepResults();

    void onConnectRobotClicked();
    void onFetchSpecClicked();
    void onSaveMLClicked();
    void onLoadContourClicked();
    void onBuildRouteClicked();
    void onViewCommandsClicked();
    void onStartWorkClicked();
    void onRequestResultsClicked();

public:
    void showRobotSpec(const QString& json);

public:
signals:
    void ConnectRobotClicked(QString robotHost, ushort robotPort);
    void FetchSpecClicked();
    void SaveMLClicked();
    void LoadContourClicked();
    void BuildRouteClicked();
    void ViewCommandsClicked();
    void StartWorkClicked();
    void RequestResultsClicked();

private:
    void setupUi();

    QWidget* createRobotPage();
    QWidget* createMLPage();
    QWidget* createContourPage();
    QWidget* createRoutePage();
    QWidget* createCommandsPage();
    QWidget* createStartPage();
    QWidget* createResultsPage();

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
};

#endif // WIDGET_H
