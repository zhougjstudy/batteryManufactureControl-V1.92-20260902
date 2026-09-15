#ifndef DBTHREAD_H
#define DBTHREAD_H


#include <QThread>
#include "bmcmainwindow.h"
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
class DBThread : public QThread
{
    Q_OBJECT
public:
    explicit DBThread(QObject *parent = 0);

protected:
    //QThread的虚函数
    //线程处理函数
    //不能直接调用，通过start()间接调用
    void run();

signals:
    void isDone();

public slots:


public:
   // BMCMainWindow *bmc;


};

#endif // DBTHREAD_H
