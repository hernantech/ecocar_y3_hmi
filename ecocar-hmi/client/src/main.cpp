#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQuickStyle>
#include <QScreen>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    
    // Set application properties
    app.setOrganizationName("EcoCar");
    app.setOrganizationDomain("ecocar.org");
    app.setApplicationName("EcoCar HMI");

    // Use the Material style for better touch support
    QQuickStyle::setStyle("Material");

    QQmlApplicationEngine engine;

    // Set the target screen resolution
    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        screen->setVirtualGeometry(QRect(0, 0, 1280, 720));
    }

    const QUrl url(u"qrc:/EcocarHMI/src/qml/main.qml"_qs);
    
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreated,
        &app, [url](QObject *obj, const QUrl &objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        }, Qt::QueuedConnection);

    engine.load(url);

    return app.exec();
}