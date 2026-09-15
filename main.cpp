#include "bmcmainwindow.h"
#include "tableWidgetSystemStaWidget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication::setDesktopSettingsAware(false);//不应用操作系统设置比如字体
#if (QT_VERSION >= QT_VERSION_CHECK(5,0,0))
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);  //Qt::AA_Use96Dpi  //自适应屏幕分辨率
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5,14,0))
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Floor);
#endif
    QApplication a(argc, argv);

    QFont font;
    font.setFamily("Microsoft Yahei");
    font.setPixelSize(10);
    a.setFont(font);

    BMCMainWindow w;
    w.setWindowTitle("动力电池生产测试系统(BMC) V1.92 beta");
    w.show();

//    tableWidgetSystemStaWidget t;
//    t.setWindowTitle("系统状态");
//    t.show();
    return a.exec();
}
