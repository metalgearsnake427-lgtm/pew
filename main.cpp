#include <QApplication>
#include <QMessageBox>
#include <cstdlib>  
#include "MainWindow.hpp"
#include <cstdlib>
#include <exception>
int main(int argc, char *argv[])
{
    QApplication::setAttribute(
        Qt::AA_EnableHighDpiScaling
    );

    QApplication::setAttribute(
        Qt::AA_UseHighDpiPixmaps
    );

    QApplication app(argc, argv);

    QApplication::setApplicationName("VIDM");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setOrganizationName("VIDM");

    try
    {
        MainWindow window;

        if (argc > 1)
        {
            QString mediaPath =
                QString::fromLocal8Bit(argv[1]);

            window.openMedia(mediaPath);
        }

        window.show();

        return app.exec();
    }
    catch (const std::exception& e)
    {
        QMessageBox::critical(
            nullptr,
            "VIDM Fatal Error",
            e.what()
        );

        return EXIT_FAILURE;
    }
    catch (...)
    {
        QMessageBox::critical(
            nullptr,
            "VIDM Fatal Error",
            "Unknown fatal error."
        );

        return EXIT_FAILURE;
    }
}