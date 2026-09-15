#ifndef TABLEWIDGETSYSTEMSTAWIDGET_H
#define TABLEWIDGETSYSTEMSTAWIDGET_H

#include <QWidget>
#include <QTableWidget>

namespace Ui {
class tableWidgetSystemStaWidget;
}

class tableWidgetSystemStaWidget : public QWidget
{
    Q_OBJECT

public:
    explicit tableWidgetSystemStaWidget(QWidget *parent = nullptr);
    ~tableWidgetSystemStaWidget();

    QTableWidget* getTableWidget();

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::tableWidgetSystemStaWidget *ui;
};

#endif // TABLEWIDGETSYSTEMSTAWIDGET_H