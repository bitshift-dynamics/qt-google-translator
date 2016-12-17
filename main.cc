#include <QtWidgets/QApplication>

#include "mainwindow.h"
#include "gtranslator.h"

#include "apitoken.h"


int main(int argc, char** argv)
{
    bool isConnected = false;
    Q_UNUSED(isConnected);

    QApplication app(argc, argv);


    // Setup the uber translator.
    auto translator = new(std::nothrow) GTranslator;
    Q_CHECK_PTR(translator);

    translator->setApiToken(::GoogleApiToken);
    app.installTranslator(translator);


    // Create the main window and show it.
    auto window = new MainWindow;
    Q_CHECK_PTR(window);

    isConnected = QObject::connect(window,
                                   SIGNAL(languageRequested(QString)),
                                   translator,
                                   SLOT(setLanguage(QString)));
    Q_ASSERT(isConnected == true);

    isConnected = QObject::connect(
                translator,
                SIGNAL(languageListChanged(QHash<QString,QString>)),
                window,
                SLOT(setLanguages(QHash<QString,QString>)));
    Q_ASSERT(isConnected == true);

    translator->requestLanguageList();
    window->show();


    // Run the event queue and clean up afterwards.
    int returnCode = app.exec();

    app.removeTranslator(translator);
    delete translator;
    translator = nullptr;

    delete window;
    window = nullptr;

    return returnCode;
}
