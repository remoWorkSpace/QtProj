#include "codestreamtranslator.h"

#include <QApplication>
#include <QLocale>
#include <QTranslator>

#include <QStyleFactory>//窗口风格头文件

int main(int argc, char *argv[])
{
    //修改窗口展示风格为 Fusion 风格
    QApplication::setStyle(QStyleFactory::create("Fusion"));
    //See also ("windows") Style, ("windowsvista") Style, ("macOS") Style, ("fusion") Style

    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "CodeStreamTranslator_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    CodeStreamTranslator w;
    w.setWindowTitle("微波消融仪软件消息解析工具");

    w.setWindowFlags(w.windowFlags() & ~Qt::WindowMaximizeButtonHint);//括号里设置的都是要起作用的，不想最大化，所以最大化要取反拿掉
    w.setWindowFlags(Qt::MSWindowsFixedSizeDialogHint);//设置窗口为固定大小
    w.show();
    return a.exec();
}
