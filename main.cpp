#include "mainwindow.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include "qsurfaceformat.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "QT_DIPLOM_3D_Paint_texture_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    QSurfaceFormat format;
    format.setSamples(16);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setVersion(3, 3);
    QSurfaceFormat::setDefaultFormat(format);
    QCoreApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
    MainWindow w;
    w.show();
    return a.exec();
}
