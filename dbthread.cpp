#include "dbthread.h"

extern bool DBconnected;
DBThread::DBThread(QObject *parent) : QThread(parent)
{
 // bmc = new BMCMainWindow;
}

void DBThread::run()
{
 //  bmc->createSQLServerConnection();  //连接数据库


    QString strHost = "192.168.0.249";
    int port = 3306;
    QString strDbName = "EsApp1";
    QString strUserName = "OA_Admin";
    QString strUserPwd = "SP188988";
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");
    QString strconn = QString("Driver={sql server};SERVER=%1;PORT=%2;DATABASE=%3;UID=%4;PWD=%5;")
                          .arg(strHost)
                          .arg(port)
                          .arg(strDbName)
                          .arg(strUserName)
                          .arg(strUserPwd);
    db.setDatabaseName(strconn);
    if (!db.open())
    {
        qDebug() <<"error_SqlServer:" << db.lastError().text();
        DBconnected = 0;
    }
    else
    {
      DBconnected = 1;
        qDebug()<<"Connect Database OK";
        }
    emit isDone();
}
