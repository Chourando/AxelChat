#if defined(AXELCHAT_LIBRARY)
#include "shared_library_interface.hpp"
#else
#include <QApplication>
#include "constants.hpp"
#include <QQmlApplicationEngine>
#include "chathandler.hpp"
#include <QtWebEngine/QtWebEngine>
#include "githubapi.hpp"
#include "clipboardqml.hpp"
#include "qmlutils.hpp"
#include "i18n.hpp"
int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QApplication::setApplicationName(AxelChat::APPLICATION_NAME);
    QApplication::setOrganizationName(AxelChat::ORGANIZATION_NAME);
    QApplication::setOrganizationDomain(AxelChat::ORGANIZATION_DOMAIN);
    QApplication::setApplicationVersion(AxelChat::APPLICATION_VERSION);

    QSettings* settings = new QSettings();

    //QML Utils
    QMLUtils::declareQml();
    QMLUtils* qmlUtils = new QMLUtils(settings, "qml_utils");

    QApplication app(argc, argv);
    QtWebEngine::initialize();

    //Window icon
    app.setWindowIcon(QIcon(":/icon.ico"));

    //Translations
    I18n::declareQml();
    I18n* i18n = new I18n(settings, "i18n", nullptr, &app);
    Q_UNUSED(i18n);

    //Settings
    QString settingsPath = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);

    QDir dir(settingsPath);
    if (!dir.exists() && !settingsPath.isEmpty())
    {
        if (!dir.mkpath(settingsPath))
        {
            settingsPath = "";
        }
    }

    //ChatHandler
    ChatHandler::declareQml();
    ChatHandler* chatHandler = new ChatHandler(settings, "chat_handler");
    settings->setParent(chatHandler);

    //Update Checker
    GitHubApi::declareQml();
    GitHubApi* github = new GitHubApi(settings, "update_checker", chatHandler);

    QQmlApplicationEngine engine;
    qmlUtils->setParent(&engine);
    chatHandler->setParent(&engine);
    i18n->setQmlApplicationEngine(&engine);

    //Clipboard
    ClipboardQml::declareQml();
    ClipboardQml* clipboard = new ClipboardQml(&engine);

    engine.rootContext()->setContextProperty("i18n",               i18n);
    engine.rootContext()->setContextProperty("chatHandler",        chatHandler);
    engine.rootContext()->setContextProperty("youTubeInterceptor", chatHandler->youTubeInterceptor());
    engine.rootContext()->setContextProperty("outputToFile",       chatHandler->outputToFile());
    engine.rootContext()->setContextProperty("chatBot",            chatHandler->bot());
    engine.rootContext()->setContextProperty("updateChecker",      github);
    engine.rootContext()->setContextProperty("clipboard",          clipboard);
    engine.rootContext()->setContextProperty("qmlUtils",           qmlUtils);
    engine.rootContext()->setContextProperty("messagesModel",      chatHandler->messagesModel());

    const QUrl url(QStringLiteral("qrc:/main.qml"));
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
                     &app, [url](QObject *obj, const QUrl &objUrl) {
        if (!obj && url == objUrl)
            QCoreApplication::exit(-1);
    }, Qt::QueuedConnection);
    engine.load(url);

    int returnCode = app.exec();

    return returnCode;
}

#endif
