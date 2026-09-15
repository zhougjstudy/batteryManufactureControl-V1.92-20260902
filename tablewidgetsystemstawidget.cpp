#include "tablewidgetsystemstawidget.h"
#include "ui_tablewidgetsystemstawidget.h"
#include <QCloseEvent>

#include <QStyledItemDelegate>
#include <QPainter>


class ColorDelegate : public QStyledItemDelegate {
public:
    explicit ColorDelegate(QObject *parent = nullptr) : QStyledItemDelegate(parent) {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override {
        // 先拷贝一份 option 配置
        QStyleOptionViewItem opt = option;

        //核心修复：对 QSS 撒谎
        // 抹除悬浮状态，防止触发 flatgray.css 里的 :hover 背景变灰
        opt.state &= ~QStyle::State_MouseOver;

        // 抹除选中状态，防止鼠标点击这个格子时颜色变化
        opt.state &= ~QStyle::State_Selected;

        // 抹除焦点状态，防止点击后出现虚线框
        opt.state &= ~QStyle::State_HasFocus;


        // 1. 取出在 bmcmainwindow.cpp 里设置的背景色 (红/蓝)
        QVariant bgData = index.data(Qt::BackgroundRole);
        if (bgData.isValid() && bgData.canConvert<QBrush>()) {
            QBrush brush = bgData.value<QBrush>();
            // 2. 无视 QSS，强行把格子涂成我们的颜色
            painter->fillRect(option.rect, brush);
        }

        // 3. 把背景设为空白，然后让 Qt 引擎继续画文字和网格线
        opt.backgroundBrush = QBrush(Qt::NoBrush);
        QStyledItemDelegate::paint(painter, opt, index);
    }
};


tableWidgetSystemStaWidget::tableWidgetSystemStaWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::tableWidgetSystemStaWidget)
{
    ui->setupUi(this);

    //把这个委托挂载到表格上
    ui->tableWidgetSystemSta->setItemDelegate(new ColorDelegate(this));
}

tableWidgetSystemStaWidget::~tableWidgetSystemStaWidget()
{
    delete ui;
}

void tableWidgetSystemStaWidget::closeEvent(QCloseEvent *event)
{
    hide();
    event->ignore();
}

QTableWidget* tableWidgetSystemStaWidget::getTableWidget()
{
    return ui->tableWidgetSystemSta;
}
