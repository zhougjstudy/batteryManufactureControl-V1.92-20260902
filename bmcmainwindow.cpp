#include "bmcmainwindow.h"
#include "ui_bmcmainwindow.h"
#include "QDebug"
#include <QStandardItemModel>
#include <QMessageBox>
#include <stdlib.h>
#include <QCloseEvent>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QPluginLoader>
#include <QMessageBox>
#include <QDateTime>
#include <QThread>

#include <QDesktopServices>
#include <QUrl>

#include "tableWidgetSystemStaWidget.h"


bool DBconnected = 0;


BMCMainWindow::BMCMainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::BMCMainWindow)
{
    ui->setupUi(this);
    initForm();
//    ui->lineEdit_password->setEchoMode(QLineEdit::Password);
//    ui->lineEdit_passwordok->setEchoMode(QLineEdit::Password);
    ui->btnShow_CurV->hide();
    ui->btnShow_UL->hide();
    ui->btnShow_TotalC->hide();
    ui->btnShow_ZV->hide();
    ui->btnShow_TV->hide();
    ui->btnShow_VC->hide();
    ui->btnShow_VerC->hide();
    ui->btnShow_VdC->hide();
    ui->btnShow_CurV->hide();
    ui->btnShow_CurC->hide();
    ui->btnShow_ParC->hide();
    ScanCFGFile();
    ui->label_14->setPixmap(QPixmap(":/myimage/qrc/image/LOGO6.png"));
    ui->label_comstate->setPixmap(QPixmap(":/myimage/qrc/image/red.png").scaled(20,20));
   // qDebug()<<"setPixmap ok";
    on_comboBox_fac_currentIndexChanged(0);

    my_time=new QTimer;
    connect(my_time, SIGNAL(timeout()), this, SLOT(MyFunction_Timeout()));
    my_time->start(2); //2ms
// qDebug()<<"timer_paradect ok";
    //分配空间
    Sthread = new DBThread();
    timer_paradect=new QTimer;
    connect(timer_paradect, SIGNAL(timeout()), this, SLOT(ParameterDetect_callback()));


    timer_CurCali=new QTimer;
    connect(timer_CurCali, SIGNAL(timeout()), this, SLOT(CurCali_callback()));

    connect(ui->textBrowserTestInfo, SIGNAL(cursorPositionChanged()), this, SLOT(autoScroll()));

    //亚银码录入
//    connect(ui->lineEditMattSilverCode, SIGNAL(returnPressed()), this, SLOT(on_lineEditMattSilverCode_returnPressed()));
    ui->lineEditMattSilverCode->installEventFilter(this);//注册监听器

   //connect(ui->lineEditPrName, SIGNAL(returnPressed()), this, SLOT(processRet_PrName()));
   //connect(ui->lineEditPrName, SIGNAL(returnPressed()), this, SLOT(processRet_PrName()));

    connect(Sthread, &DBThread::isDone, this, &BMCMainWindow::dealDone);

    //当按窗口右上角x时，窗口触发destroyed()
    connect(this, &BMCMainWindow::destroyed, this, &BMCMainWindow::stopThread);

    connect(serialport, SIGNAL(readyRead()), this, SLOT(getCommData()));
    connect(&timer_rt, &QTimer::timeout,
            [=]()
            {
                //关闭定时器
                timer_rt.stop();

                //Get data
                Get_Data();
            }
            );

    systemStaWidget = new tableWidgetSystemStaWidget(nullptr);
    systemStaWidget->setWindowTitle("系统状态");
    systemStaWidget->setWindowFlags(Qt::Window | Qt::WindowCloseButtonHint | Qt::WindowMinMaxButtonsHint);
    systemStaWidget->hide();

    systemStaWidget->getTableWidget()->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    systemStaWidget->getTableWidget()->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    QTableWidgetItem *item = systemStaWidget->getTableWidget()->item(0, 1);
    item->setToolTip("AFE看门狗溢出");// 设置固定提示内容
    item = systemStaWidget->getTableWidget()->item(0, 2);
    item->setToolTip("二次过压保护");// 设置固定提示内容
    item = systemStaWidget->getTableWidget()->item(0, 3);
    item->setToolTip("短路保护");// 设置固定提示内容
    item = systemStaWidget->getTableWidget()->item(0, 4);
    item->setToolTip("充电过流");// 设置固定提示内容
    item = systemStaWidget->getTableWidget()->item(0, 5);
    item->setToolTip("放电过流2");// 设置固定提示内容
    item = systemStaWidget->getTableWidget()->item(0, 6);
    item->setToolTip("放电过流1");// 设置固定提示内容
    item = systemStaWidget->getTableWidget()->item(0, 7);
    item->setToolTip("欠压保护");// 设置固定提示内容
    item = systemStaWidget->getTableWidget()->item(0, 8);
    item->setToolTip("过压保护");// 设置固定提示内容
    item = systemStaWidget->getTableWidget()->item(1, 5);
    item->setToolTip("放电高温保护");// 设置固定提示内容
    item = systemStaWidget->getTableWidget()->item(1, 6);
    item->setToolTip("放电低温保护");// 设置固定提示内容
    item = systemStaWidget->getTableWidget()->item(1, 7);
    item->setToolTip("充电高温保护");// 设置固定提示内容
    item = systemStaWidget->getTableWidget()->item(1, 8);
    item->setToolTip("充电低温保护");// 设置固定提示内容
    item = systemStaWidget->getTableWidget()->item(2, 4);
    item->setToolTip("读写EEP错误");// 设置固定提示内容
    item = systemStaWidget->getTableWidget()->item(2, 5);
    item->setToolTip("低压禁止充电标志");// 设置固定提示内容
    item = systemStaWidget->getTableWidget()->item(2, 6);
    item->setToolTip("预充电MOS状态（蓝色关闭）");// 设置固定提示内容
    item = systemStaWidget->getTableWidget()->item(2, 7);
    item->setToolTip("充电MOS状态（蓝色关闭）");// 设置固定提示内容
    item = systemStaWidget->getTableWidget()->item(2, 8);
    item->setToolTip("放电MOS状态（蓝色关闭）");// 设置固定提示内容
    item = systemStaWidget->getTableWidget()->item(3, 1);
    item->setToolTip("固件备份完成");// 设置固定提示内容
    item = systemStaWidget->getTableWidget()->item(3, 2);
    item->setToolTip("保险丝状态");// 设置固定提示内容
    item = systemStaWidget->getTableWidget()->item(3, 3);
    item->setToolTip("保险丝使能标志");// 设置固定提示内容
    item = systemStaWidget->getTableWidget()->item(3, 4);
    item->setToolTip("过流保护状态标志");// 设置固定提示内容
    item = systemStaWidget->getTableWidget()->item(3, 5);
    item->setToolTip("电池解锁标志（蓝色未解锁）");// 设置固定提示内容
    item = systemStaWidget->getTableWidget()->item(3, 6);
    item->setToolTip("电池内存加密标志（蓝色未加密）");// 设置固定提示内容
    item = systemStaWidget->getTableWidget()->item(3, 7);
    item->setToolTip("电流校准标志（蓝色未校准）");// 设置固定提示内容
    item = systemStaWidget->getTableWidget()->item(3, 8);
    item->setToolTip("零电流校准标志（蓝色未校准）");// 设置固定提示内容
    item = systemStaWidget->getTableWidget()->item(4, 1);
    item->setToolTip("软件充电过流");// 设置固定提示内容
    item = systemStaWidget->getTableWidget()->item(4, 2);
    item->setToolTip("容量更新倒计时");// 设置固定提示内容
    item = systemStaWidget->getTableWidget()->item(4, 3);
    item->setToolTip("上电AFE无响应");// 设置固定提示内容
    item = systemStaWidget->getTableWidget()->item(4, 4);
    item->setToolTip("充电中");// 设置固定提示内容
    item = systemStaWidget->getTableWidget()->item(4, 5);
    item->setToolTip("放电中");// 设置固定提示内容
    item = systemStaWidget->getTableWidget()->item(4, 6);
    item->setToolTip("开关机标志位（蓝色为关机）");// 设置固定提示内容
    item = systemStaWidget->getTableWidget()->item(4, 7);
    item->setToolTip("允许容量更新（蓝色为允许）");// 设置固定提示内容
    item = systemStaWidget->getTableWidget()->item(4, 8);
    item->setToolTip("电池满充（蓝色为满充）");// 设置固定提示内容
    item = systemStaWidget->getTableWidget()->item(5, 6);
    item->setToolTip("过温保险丝熔断");// 设置固定提示内容
    item = systemStaWidget->getTableWidget()->item(5, 7);
    item->setToolTip("过压保险丝熔断");// 设置固定提示内容
    item = systemStaWidget->getTableWidget()->item(5, 8);
    item->setToolTip("过流保险丝熔断");// 设置固定提示内容

//    Sthread->start();
}

BMCMainWindow::~BMCMainWindow()
{
    Status = 0;
    delete ui;
}
void BMCMainWindow::loadPlugin()
{
    QPluginLoader loader;
    // ODBC 驱动插件的路径
    loader.setFileName("/Users/Hula/Qt/5.11.2/clang_64/plugins/sqldrivers/libqsqlodbc.dylib");
    qDebug() << loader.load();
    qDebug() << loader.errorString();
}
bool BMCMainWindow::createSQLServerConnection()
{
    //loadPlugin();
qDebug() << "Available drivers:" << QSqlDatabase::drivers();
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
        return 0;
    }
    else
    {
      return 1;
        qDebug()<<"Connect Database OK";
        }



}

void BMCMainWindow::processRet_PrName()
{
    qInfo() << "processRet_dutsn";
    // 判断信号发出者
    QLineEdit* returnPressedSender = (QLineEdit*)sender();
    if (returnPressedSender == ui->lineEditPrName)
    {
        QString str = ui->comboBox_fac->currentText();
        if(str=="电子车间")
        ui->lineEdit2Dcode->setFocus();
    }
}


void BMCMainWindow::DataBaseInsert(void)
{
        QSqlQuery  query;
         bool  value;
         QString ProBarCode=" ";
         QString InnerBarCode=" ";
         QString BMSCode=" ";
        QString PONumber_ = ui->lineEditOrderNum->text();
        QString ProdName_ = ui->lineEditPrName->text();
        QString ProdDate_ = ui->dateTimeEditMNF->text();
        QDateTime current_date_time = QDateTime::currentDateTime();

        QString ScanTime_ =current_date_time.toString("yyyy-MM-dd hh:mm:ss");

        QString WorkShop = ui->comboBox_fac->currentText();
        QString LineName = ui->comboBox_cell->currentText();
        QString WorkStationName=ui->lineEditWorkStationName->text();
        QString OperatorName=ui->lineEditOperater->text();
        int ilen = BarCodeStr.length();
        if(ilen==8)
        {
             ProBarCode=" ";
             InnerBarCode=BarCodeStr8;
             BMSCode=" ";
        }
        else if(ilen==10)
        {
             ProBarCode=" ";
             InnerBarCode=" ";
             BMSCode=BarCodeStr2D;
        }
        else if(ilen>=13)
        {
            ProBarCode=BarCodeStr13;
            InnerBarCode=" ";
            BMSCode=" ";
        }

        QString QualityStats=" ";
        QString RepairTimes=" ";QString DefectName=" ";QString DefectIssue=" ";
        QString SWRevsion= Bms_infor_upload.BMSinfor.SWVersion;
        float TelnetMethord=(float)ui->cboxComunicatMenth->currentIndex();
        QString BaudRate=ui->cboxBaudrate->currentText();
        QString CanBeUpdate=ui->comboBox_upgrade->currentText();
        QString BMSERPCode=ui->lineEditBoardCode->text();QString FWERPCode=ui->lineEditSoftCode->text();


        QString CalbTime=ScanTime_;
        int TotalVolt = Bms_infor_upload.BMSinfor.TotalVolt;
        int DC = Bms_infor_upload.BMSinfor.DC;int FCC=Bms_infor_upload.BMSinfor.FCC;
        int RSOC=Bms_infor_upload.BMSinfor.RSOC;int RC=Bms_infor_upload.BMSinfor.RC;
        int BatIdCode=Bms_infor_upload.BMSinfor.BatIdCode;int CAN_ID=Bms_infor_upload.BMSinfor.CAN_ID;
        int MCI=Bms_infor_upload.BMSinfor.MCI;int ShutDTime=Bms_infor_upload.BMSinfor.ShutDTime;
        int BalEndV=Bms_infor_upload.BMSinfor.BalEndV;int ChgEndCur=Bms_infor_upload.BMSinfor.ChgEndCur;
        int ChgEndVol=Bms_infor_upload.BMSinfor.ChgEndVol;int ChgEndDel=Bms_infor_upload.BMSinfor.ChgEndVol;int MNFDate=Bms_infor_upload.BMSinfor.MNFDate;


       int SlfDsqBalV = Bms_infor_upload.BMSinfor.SlfDsqBalV;
       int BalStartV = Bms_infor_upload.BMSinfor.BalStartV;int BalDelay=Bms_infor_upload.BMSinfor.BalDelay;
       int OtherState=Bms_infor_upload.BMSinfor.OtherState;float SampleRV=Bms_infor_upload.BMSinfor.SampleRV;
       int CellNum=Bms_infor_upload.BMSinfor.CellNum;int OV=Bms_infor_upload.BMSinfor.OV;
       int OVR=Bms_infor_upload.BMSinfor.OVR;int OVT=Bms_infor_upload.BMSinfor.OVT;
       int UV=Bms_infor_upload.BMSinfor.UV;int UVR=Bms_infor_upload.BMSinfor.UVR;
       int UVT=Bms_infor_upload.BMSinfor.UVT;int BALV=Bms_infor_upload.BMSinfor.BALV;int PREV=Bms_infor_upload.BMSinfor.PREV;
        int LZV=Bms_infor_upload.BMSinfor.LZV;int PFV=Bms_infor_upload.BMSinfor.PFV;int OCD1V=Bms_infor_upload.BMSinfor.OCD1V;  int OCD1T=Bms_infor_upload.BMSinfor.OCD1T;

        int OCD2V = Bms_infor_upload.BMSinfor.OCD2V;
        int OCD2T = Bms_infor_upload.BMSinfor.OCD2T;int SCV=Bms_infor_upload.BMSinfor.SCV;
        int SCT=Bms_infor_upload.BMSinfor.SCT;int OCCV=Bms_infor_upload.BMSinfor.OCCV;
        int OCCT=Bms_infor_upload.BMSinfor.OCCT;int EOTC=Bms_infor_upload.BMSinfor.EOTC;
        int EOTCR=Bms_infor_upload.BMSinfor.EOTCR;int EUTC=Bms_infor_upload.BMSinfor.EUTC;
        int EUTCR=Bms_infor_upload.BMSinfor.EUTCR;int EOTD=Bms_infor_upload.BMSinfor.EOTD;
        int EOTDR=Bms_infor_upload.BMSinfor.EOTDR;int EUTD=Bms_infor_upload.BMSinfor.EUTD;int EUTDR=Bms_infor_upload.BMSinfor.EUTDR;
       int SOCC=Bms_infor_upload.BMSinfor.SOCC;int SOCCD=Bms_infor_upload.BMSinfor.SOCCD;


/*
       QString sql_insert = QString("insert into SoftData(PONumber,ProdName,ProdDate,ScanTime,WorkShop,LineName,WorkStationName,OperatorName,ProBarCode"
                                    ",InnerBarCode,BMSCode,QualityStats,RepairTimes,DefectName,DefectIssue,SWRevsion,TelnetMethord,BaudRate,CanBeUpdate"
                                    ")values('%1','%2','%3','%4','%5'"
                                    ",'%6','%7','%8','%9','%10','%11','%12','%13','%14','%15','%16','%17','%18',"
                                    ",'%19');").arg(PONumber_).arg(ProdName_).arg(ProdDate_).arg(ScanTime_)
                                    .arg(WorkShop).arg(LineName).arg(WorkStationName).arg(OperatorName).arg(ProBarCode).arg(InnerBarCode).arg(BMSCode)
               .arg(QualityStats).arg(RepairTimes).arg(DefectName).arg(DefectIssue).arg(SWRevsion).arg(TelnetMethord).arg(BaudRate)
               .arg(CanBeUpdate);
*/
//QString sql_insert = QString("insert into SoftData(PONumber,ProdName,ProdDate,ScanTime,WorkShop,LineName,WorkStationName,OperatorName,ProBarCode,InnerBarCode,BMSCode,QualityStats,RepairTimes,DefectName,DefectIssue,SWRevsion,TelnetMethord,BaudRate,CanBeUpdate,CalbTime,TotalVolt,DC,FCC,RSOC,RC,BatIdCode,CAN_ID,MCI,ShutDTime,BalEndV,ChgEndCur,ChgEndVol,ChgEndDel,MNFDate,SlfDsqBalV,BalStartV,BalDelay,OtherState,SampleRV,CellNum,OV,OVR,OVT,UV,UVR,UVT,BALV,PREV,LZV,PFV,OCD1V,OCD1T) values('%1','%2','%3','%4','%5','%6','%7','%8','%9','%10','%11','%12','%13','%14','%15','%16','%17','%18','%19','%20','%21','%22','%23','%24','%25','%26','%27','%28','%29','%30','%31','%32','%33','%34','%35','%36','%37','%38','%39','%40','%41','%42','%43','%44','%45','%46','%47','%48','%49','%50','%51','%52')").arg(PONumber_).arg(ProdName_).arg(ProdDate_).arg(ScanTime_).arg(WorkShop).arg(LineName).arg(WorkStationName).arg(OperatorName).arg(ProBarCode).arg(InnerBarCode).arg(BMSCode).arg(QualityStats).arg(RepairTimes).arg(DefectName).arg(DefectIssue).arg(SWRevsion).arg(TelnetMethord).arg(BaudRate).arg(CanBeUpdate).arg(CalbTime).arg(TotalVolt).arg(DC).arg(FCC).arg(RSOC).arg(RC).arg(BatIdCode).arg(CAN_ID).arg(MCI).arg(ShutDTime).arg(BalEndV).arg(ChgEndCur).arg(ChgEndVol).arg(ChgEndDel).arg(MNFDate).arg(SlfDsqBalV).arg(BalStartV).arg(BalDelay).arg(OtherState).arg(SampleRV).arg(CellNum).arg(OV).arg(OVR).arg(OVT).arg(UV).arg(UVR).arg(UVT).arg(BALV).arg(PREV).arg(LZV).arg(PFV).arg(OCD1V).arg(OCD1T);
// QString sql_insert = QString("insert into SoftData(PONumber,ProdName,ProdDate,ScanTime,WorkShop,LineName,WorkStationName,OperatorName,ProBarCode,InnerBarCode,BMSCode,QualityStats,RepairTimes,DefectName,DefectIssue,SWRevsion,TelnetMethord,BaudRate,CanBeUpdate,CalbTime,TotalVolt,DC,FCC,RSOC,RC,BatIdCode,CAN_ID,MCI,ShutDTime,BalEndV,ChgEndCur,ChgEndVol,ChgEndDel,MNFDate,SlfDsqBalV,BalStartV,BalDelay,OtherState,SampleRV,CellNum,OV,OVR,OVT,UV,UVR,UVT,BALV,PREV,LZV,PFV,OCD1V,OCD1T,OCD2V,OCD2T,SCV,SCT,OCCV,OCCT) values('%1','%2','%3','%4','%5','%6','%7','%8','%9','%10','%11','%12','%13','%14','%15','%16','%17','%18','%19','%20','%21','%22','%23','%24','%25','%26','%27','%28','%29','%30','%31','%32','%33','%34','%35','%36','%37','%38','%39','%40','%41','%42','%43','%44','%45','%46','%47','%48','%49','%50','%51','%52','%53','%54','%55','%56','%57','%58')").arg(PONumber_).arg(ProdName_).arg(ProdDate_).arg(ScanTime_).arg(WorkShop).arg(LineName).arg(WorkStationName).arg(OperatorName).arg(ProBarCode).arg(InnerBarCode).arg(BMSCode).arg(QualityStats).arg(RepairTimes).arg(DefectName).arg(DefectIssue).arg(SWRevsion).arg(TelnetMethord).arg(BaudRate).arg(CanBeUpdate).arg(CalbTime).arg(TotalVolt).arg(DC).arg(FCC).arg(RSOC).arg(RC).arg(BatIdCode).arg(CAN_ID).arg(MCI).arg(ShutDTime).arg(BalEndV).arg(ChgEndCur).arg(ChgEndVol).arg(ChgEndDel).arg(MNFDate).arg(SlfDsqBalV).arg(BalStartV).arg(BalDelay).arg(OtherState).arg(SampleRV).arg(CellNum).arg(OV).arg(OVR).arg(OVT).arg(UV).arg(UVR).arg(UVT).arg(BALV).arg(PREV).arg(LZV).arg(PFV).arg(OCD1V).arg(OCD1T).arg(OCD2V).arg(OCD2T).arg(SCV).arg(SCT).arg(OCCV).arg(OCCT);
//QString sql_insert = QString("insert into SoftData(PONumber,ProdName,ProdDate,ScanTime,WorkShop,LineName,WorkStationName,OperatorName,ProBarCode,InnerBarCode,BMSCode,QualityStats,RepairTimes,DefectName,DefectIssue,SWRevsion,TelnetMethord,BaudRate,CanBeUpdate,CalbTime,TotalVolt,DC,FCC,RSOC,RC,BatIdCode,CAN_ID,MCI,ShutDTime,BalEndV,ChgEndCur,ChgEndVol,ChgEndDel,MNFDate,SlfDsqBalV,BalStartV,BalDelay,OtherState,SampleRV,CellNum,OV,OVR,OVT,UV,UVR,UVT,BALV,PREV,LZV,PFV,OCD1V,OCD1T,OCD2V,OCD2T,SCV,SCT,OCCV,OCCT,EOTC,EOTCR,EUTC,EUTCR) values('%1','%2','%3','%4','%5','%6','%7','%8','%9','%10','%11','%12','%13','%14','%15','%16','%17','%18','%19','%20','%21','%22','%23','%24','%25','%26','%27','%28','%29','%30','%31','%32','%33','%34','%35','%36','%37','%38','%39','%40','%41','%42','%43','%44','%45','%46','%47','%48','%49','%50','%51','%52','%53','%54','%55','%56','%57','%58','%59','%60','%61','%62,)").arg(PONumber_).arg(ProdName_).arg(ProdDate_).arg(ScanTime_).arg(WorkShop).arg(LineName).arg(WorkStationName).arg(OperatorName).arg(ProBarCode).arg(InnerBarCode).arg(BMSCode).arg(QualityStats).arg(RepairTimes).arg(DefectName).arg(DefectIssue).arg(SWRevsion).arg(TelnetMethord).arg(BaudRate).arg(CanBeUpdate).arg(CalbTime).arg(TotalVolt).arg(DC).arg(FCC).arg(RSOC).arg(RC).arg(BatIdCode).arg(CAN_ID).arg(MCI).arg(ShutDTime).arg(BalEndV).arg(ChgEndCur).arg(ChgEndVol).arg(ChgEndDel).arg(MNFDate).arg(SlfDsqBalV).arg(BalStartV).arg(BalDelay).arg(OtherState).arg(SampleRV).arg(CellNum).arg(OV).arg(OVR).arg(OVT).arg(UV).arg(UVR).arg(UVT).arg(BALV).arg(PREV).arg(LZV).arg(PFV).arg(OCD1V).arg(OCD1T).arg(OCD2V).arg(OCD2T).arg(SCV).arg(SCT).arg(OCCV).arg(OCCT).arg(EOTC).arg(EOTCR).arg(EUTC).arg(EUTCR);
QString sql_insert = QString("insert into SoftData(PONumber,ProdName,ProdDate,ScanTime,WorkShop,LineName,WorkStationName,OperatorName,ProBarCode,InnerBarCode,BMSCode,QualityStats,RepairTimes,DefectName,DefectIssue,SWRevsion,TelnetMethord,BaudRate,CanBeUpdate,CalbTime,TotalVolt,DC,FCC,RSOC,RC,BatIdCode,CAN_ID,MCI,ShutDTime,BalEndV,ChgEndCur,ChgEndVol,ChgEndDel,MNFDate,SlfDsqBalV,BalStartV,BalDelay,OtherState,SampleRV,CellNum,OV,OVR,OVT,UV,UVR,UVT,BALV,PREV,LZV,PFV,OCD1V,OCD1T,OCD2V,OCD2T,SCV,SCT,OCCV,OCCT,EOTC,EOTCR,EUTC,BMSERPCode,FWERPCode,SOCC,SOCCD,EUTD,EUTDR,EOTD,EOTDR,EUTCR) values('%1','%2','%3','%4','%5','%6','%7','%8','%9','%10','%11','%12','%13','%14','%15','%16','%17','%18','%19','%20','%21','%22','%23','%24','%25','%26','%27','%28','%29','%30','%31','%32','%33','%34','%35','%36','%37','%38','%39','%40','%41','%42','%43','%44','%45','%46','%47','%48','%49','%50','%51','%52','%53','%54','%55','%56','%57','%58','%59','%60','%61','%62','%63','%64','%65','%66','%67','%68','%69','%70')").arg(PONumber_).arg(ProdName_).arg(ProdDate_).arg(ScanTime_).arg(WorkShop).arg(LineName).arg(WorkStationName).arg(OperatorName).arg(ProBarCode).arg(InnerBarCode).arg(BMSCode).arg(QualityStats).arg(RepairTimes).arg(DefectName).arg(DefectIssue).arg(SWRevsion).arg(TelnetMethord).arg(BaudRate).arg(CanBeUpdate).arg(CalbTime).arg(TotalVolt).arg(DC).arg(FCC).arg(RSOC).arg(RC).arg(BatIdCode).arg(CAN_ID).arg(MCI).arg(ShutDTime).arg(BalEndV).arg(ChgEndCur).arg(ChgEndVol).arg(ChgEndDel).arg(MNFDate).arg(SlfDsqBalV).arg(BalStartV).arg(BalDelay).arg(OtherState).arg(SampleRV).arg(CellNum).arg(OV).arg(OVR).arg(OVT).arg(UV).arg(UVR).arg(UVT).arg(BALV).arg(PREV).arg(LZV).arg(PFV).arg(OCD1V).arg(OCD1T).arg(OCD2V).arg(OCD2T).arg(SCV).arg(SCT).arg(OCCV).arg(OCCT).arg(EOTC).arg(EOTCR).arg(EUTC).arg(BMSERPCode).arg(FWERPCode).arg(SOCC).arg(SOCCD).arg(EUTD).arg(EUTDR).arg(EOTD).arg(EOTDR).arg(EUTCR);
       // QString sql_insert = QString("insert into SoftData(PONumber,ProdName,ProdDate,ScanTime,WorkShop,LineName,WorkStationName,OperatorName,ProBarCode,InnerBarCode,BMSCode,QualityStats,RepairTimes,DefectName,DefectIssue,SWRevsion,TelnetMethord,BaudRate,CanBeUpdate,CalbTime,TotalVolt,DC,FCC,RSOC,RC,BatIdCode,CAN_ID,MCI,ShutDTime,BalEndV,ChgEndCur,ChgEndVol,ChgEndDel,MNFDate,SlfDsqBalV,BalStartV,BalDelay,OtherState,SampleRV,CellNum,OV,OVR,OVT,UV,UVR,UVT,BALV,PREV,LZV,PFV,OCD1V,OCD1T,OCD2V,OCD2T,SCV,SCT,OCCV,OCCT,EOTC,EOTCR,EUTC,EUTCR,EOTD,EOTDR,EUTD,EUTDR,SOCC,SOCCD,BMSERPCode,FWERPCode) values('%1','%2','%3','%4','%5','%6','%7','%8','%9','%10','%11','%12','%13','%14','%15','%16','%17','%18','%19','%20','%21','%22','%23','%24','%25','%26','%27','%28','%29','%30','%31','%32','%33','%34','%35','%36','%37','%38','%39','%40','%41','%42','%43','%44','%45','%46','%47','%48','%49','%50','%51','%52','%53','%54','%55','%56','%57','%58','%59','%60','%61','%62,'%63','%64','%65','%66','%67','%68','%69','%70)").arg(PONumber_).arg(ProdName_).arg(ProdDate_).arg(ScanTime_).arg(WorkShop).arg(LineName).arg(WorkStationName).arg(OperatorName).arg(ProBarCode).arg(InnerBarCode).arg(BMSCode).arg(QualityStats).arg(RepairTimes).arg(DefectName).arg(DefectIssue).arg(SWRevsion).arg(TelnetMethord).arg(BaudRate).arg(CanBeUpdate).arg(CalbTime).arg(TotalVolt).arg(DC).arg(FCC).arg(RSOC).arg(RC).arg(BatIdCode).arg(CAN_ID).arg(MCI).arg(ShutDTime).arg(BalEndV).arg(ChgEndCur).arg(ChgEndVol).arg(ChgEndDel).arg(MNFDate).arg(SlfDsqBalV).arg(BalStartV).arg(BalDelay).arg(OtherState).arg(SampleRV).arg(CellNum).arg(OV).arg(OVR).arg(OVT).arg(UV).arg(UVR).arg(UVT).arg(BALV).arg(PREV).arg(LZV).arg(PFV).arg(OCD1V).arg(OCD1T).arg(OCD2V).arg(OCD2T).arg(SCV).arg(SCT).arg(OCCV).arg(OCCT).arg(EOTC).arg(EOTCR).arg(EUTC).arg(EUTCR).arg(EOTD).arg(EOTDR).arg(EUTD).arg(EUTDR).arg(SOCC).arg(SOCCD).arg(BMSERPCode).arg(FWERPCode);

/*   arg(PONumber_).arg(ProdName_).arg(ProdDate_).arg(ScanTime_).arg(WorkShop).arg(LineName).arg(WorkStationName).arg(OperatorName).arg(ProBarCode).arg(InnerBarCode).arg(BMSCode).arg(QualityStats).arg(RepairTimes).arg(DefectName).arg(DefectIssue).arg(SWRevsion).arg(TelnetMethord).arg(BaudRate).arg(CanBeUpdate)
        QString sql_insert = QString("insert into SoftData(PONumber,ProdName,ProdDate,ScanTime,WorkShop,LineName,WorkStationName,OperatorName,ProBarCode"                                                                                       arg(CalbTime).arg(TotalVolt).arg(DC).arg(FCC).arg(RSOC).arg(RC).arg(BatIdCode).arg(CAN_ID).arg(MCI).arg(ShutDTime).arg(BalEndV).arg(ChgEndCur).arg(ChgEndVol).arg(ChgEndDel).arg(MNFDate)
                                     ",InnerBarCode,BMSCode,QualityStats,RepairTimes,DefectName,DefectIssue,SWRevsion,TelnetMethord,BaudRate,CanBeUpdate"
                                     ",CalbTime,TotalVolt,DC,FCC,RSOC,RC,BatIdCode,CAN_ID,MCI,ShutDTime,BalEndV,ChgEndCur,ChgEndVol,ChgEndDel,MNFDate"
                                     ",SlfDsqBalV,BalStartV,BalDelay,OtherState,SampleRV,CellNum,OV,OVR,OVT,UV,UVR,UVT,BALV,PREV,LZV,PFV,OCD1V,OCD1T"
                                     ",OCD2V,OCD2T,SCV,SCT,OCCV,OCCT,EOTC,EOTCR,EUTC,EUTCR,EOTD,EOTDR,EUTD,EUTDR,SOCC,SOCCD,BMSERPCode,FWERPCode)values('%1','%2','%3','%4','%5'"
                                     ",'%6','%7','%8','%9','%10','%11','%12','%13','%14','%15','%16','%17','%18','%19','%20','%21','%22','%23','%24','%25','%26','%27','%28','%29','%30','%31','%32','%33','%34'"
                                     ",'%35','%36','%37','%38','%39','%40','%41','%42','%43','%44','%45','%46','%47','%48','%49','%50','%51','%52','%53','%54','%55','%56','%57','%58','%59','%60','%61','%62'"
                                     "'%63','%64','%65','%66','%67','%68','%69','%70');").arg(PONumber_).arg(ProdName_).arg(ProdDate_).arg(ScanTime_)
                                     .arg(WorkShop).arg(LineName).arg(WorkStationName).arg(OperatorName).arg(ProBarCode).arg(InnerBarCode).arg(BMSCode)
                                     .arg(QualityStats).arg(RepairTimes).arg(DefectName).arg(DefectIssue).arg(SWRevsion).arg(TelnetMethord).arg(BaudRate)
                                     .arg(CanBeUpdate).arg(CalbTime).arg(TotalVolt).arg(DC_).arg(FCC).arg(RSOC).arg(RC).arg(BatIdCode).arg(CAN_ID).arg(MCI).arg(ShutDTime).arg(BalEndV).arg(ChgEndCur).arg(ChgEndVol).arg(ChgEndDel).arg(MNFDate)
                                     .arg(RC).arg(BatIdCode).arg(CAN_ID).arg(MCI).arg(ShutDTime).arg(BalEndV).arg(ChgEndCur).arg(ChgEndVol).arg(ChgEndDel).arg(MNFDate)
                                     .arg(SlfDsqBalV).arg(BalStartV).arg(BalDelay).arg(OtherState).arg(SampleRV).arg(CellNum).arg(OV).arg(OVR).arg(OVT).arg(UV).arg(UVR).arg(UVT).arg(BALV).arg(PREV).arg(LZV).arg(PFV).arg(OCD1V).arg(OCD1T)
                                     .arg(UVR).arg(UVT).arg(BALV).arg(PREV).arg(LZV).arg(PFV).arg(OCD1V).arg(OCD1T).arg(OCD2V).arg(OCD2T).arg(SCV).arg(SCT).arg(OCCV).arg(OCCT).arg(EOTC).arg(EOTCR).arg(EUTC).arg(EUTCR).arg(EOTD).arg(EOTDR).arg(EUTD).arg(EUTDR).arg(SOCC).arg(SOCCD).arg(BMSERPCode).arg(FWERPCode)
                                     .arg(OCCT).arg(EOTC).arg(EOTCR).arg(EUTC).arg(EUTCR).arg(EOTD).arg(EOTDR)
                                      .arg(EUTD).arg(EUTDR).arg(SOCC).arg(SOCCD).arg(BMSERPCode).arg(FWERPCode);
*/
        value = query.exec(sql_insert);

//        value = query.exec("insert into SoftData(PONumber,ProdName,ProdDate,ScanTime)values(PONumber_,'自行车电池','2022-11-22','2022-11-22');");
         if(value)
         {
                 // QMessageBox::information（this，"notice","add sucessful!"）;
             //qDebug()<<"insert Database OK";
             ui->textBrowserTestInfo->append("上传成功！\n");
         }
         else
         {
                 //  QMessageBox::information（this，"notice","add failed!"）;
            // qDebug()<<"insert Database failed";
             ui->textBrowserTestInfo->append("上传失败！\n");
             DBConnectIsOk = 0;
             titleLabel->setText("离线");//label显示内容
             statusBar()->addWidget(titleLabel);//将label加到状态栏上
         }
         //query.first();
//         query.exec("SELECT * FROM SoftData;");
//         while(query.next())
//         {
//             qDebug()<<"PONumber ProdName ProdDate";
//                 QString name = query.value(0).toString();
//                 qDebug()<<name;
//                  name = query.value(1).toString();
//                 qDebug()<<name;
//                 name = query.value(2).toString();
//                qDebug()<<name;
//                name = query.value(3).toString();
//               qDebug()<<name;
//         }
}

void BMCMainWindow::loadStyle(const QString &qssFile)
{
    //加载样式表
    QString qss;
    QFile file(qssFile);
    if (file.open(QFile::ReadOnly)) {
        //用QTextStream读取样式文件不用区分文件编码 带bom也行
        QStringList list;
        QTextStream in(&file);
        //in.setCodec("utf-8");
        while (!in.atEnd()) {
            QString line;
            in >> line;
            list << line;
        }

        file.close();
        qss = list.join("\n");
        QString paletteColor = qss.mid(20, 7);
        qApp->setPalette(QPalette(paletteColor));
        qApp->setStyleSheet(qss);
    }
}

void BMCMainWindow::stopThread()
{
    //停止线程
    Sthread->quit();
    //等待线程处理完手头动作
    Sthread->wait();
}

void BMCMainWindow::dealDone()
{
    qDebug() << "it is over";
//    myTimer->stop(); //关闭定时器
        DBConnectIsOk=DBconnected;
        if(DBConnectIsOk)
        {
            //定义一个label
                titleLabel->setText("在线");//label显示内容
                statusBar()->addWidget(titleLabel);//将label加到状态栏上
        }
        else
        {

                titleLabel->setText("离线");//label显示内容
                statusBar()->addWidget(titleLabel);//将label加到状态栏上
        }
}
void BMCMainWindow::initForm()
{
    serialport = new QSerialPort();
    ui->progressBar->setRange(0,100);
//    ui->progressBarCom->setRange(0,100);
//    ui->progressBarCom->reset();
//    ui->progressBarCom->setOrientation(Qt::Horizontal);
//    ui->progressBarCom->setTextVisible(false);
    RecFlag = 0;
    Status = 0;
    ComUpdate();
    menuBar()->setVisible(false);  //隐藏菜单栏
    setWindowFlags(windowFlags()&~Qt::WindowMaximizeButtonHint);    // 禁止最大化按钮
//    setFixedSize(1020,760);                     // 禁止拖动窗口大小
    setFixedSize(1040,801);                     // 禁止拖动窗口大小
    //1082 831

    ui->progressBarSOC->setRange(0,100);
    ui->progressBarSOC->setFormat("%p%");
    ui->progressBarSOC->setValue(0);
    ui->progressBarSOC->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);

    ui->lineEdit2Dcode->setEnabled(true);
    ui->lineEditBarCode8->setEnabled(false);
    ui->lineEditBarCode13->setEnabled(false);

//    ui->pushButton_StartCurCali->setEnabled(false);
    btnStartCurCali = false; // <-- 确保在程序启动时它是 false

    ReadBaseInfoEnable=0; //发送读命令使能
    SendResumeEnable=0;//发送恢复命令使能
    SendResetEnable=0;//发送复位命令使能
    SendShutDownEnable=0;//发送关机命令使能
    SendPowerUpEnable=0;//发送关机命令使能
    StartStorageEnable=0;//开启仓储
    OpenHeaterEnable=0;//开启加热
    CloseHeaterEnable=0;//关闭加热
    AuthenticationEnable=0;//授权使能
    ZeroCurCaliEnable=0;//零电流校准使能
    VoltCheckEnable=0;//电压检测使能
    CurrentCaliEnable=0;//电流校准
    RTCCaliEnable=0;//时间校准
    IDWriteEnable=0;//ID录入
    CodesWriteEnable=0;//哑银码、内控码、BMS编码录入
    ReadSysPraEnable=0;//读取系统参数
    EFUSEEnable=0; //三端保险丝使能
    EsendSNRead=0;
    EsendSNWrite=0;
    TestProcessEnd = 0;
    TestProcessError = 0;

    BarCodeIsAccept = 0;
    Prohibit_Upload = 0;

    loadStyle(":/qss/flatgray.css"); //lightblue

    // 【修改1：补充初始化】
    Buffer_time = 70;

    ui->pushButtonZeroCur->setStyleSheet("color:blue;font-size:15px;font-family:宋体");
    ui->pushButtonUnlock->setStyleSheet("color:blue;font-size:15px;font-family:宋体");
    ui->pushButtonTimeCali->setStyleSheet("color:blue;font-size:15px;font-family:宋体");
    ui->pushButtonCurCali_2->setStyleSheet("color:blue;font-size:15px;font-family:宋体");

    ui->pushButtonCurCali_3->setStyleSheet("color:blue;font-size:15px;font-family:宋体");
    ui->pushButtonVoltDiffDetect->setStyleSheet("color:blue;font-size:15px;font-family:宋体");
    ui->pushButtonCurCali->setStyleSheet("color:blue;font-size:15px;font-family:宋体");
    ui->pushButtonCurDetect->setStyleSheet("color:blue;font-size:15px;font-family:宋体");

    ui->pushButtonParameterDetect->setStyleSheet("color:blue;font-size:15px;font-family:宋体");
}
void BMCMainWindow::closeEvent(QCloseEvent *event)
 {
    serialport->close();
    Status = 0;
//    QTime reachTime = QTime::currentTime().addMSecs(100);
//    while(QTime::currentTime() < reachTime);
    //delete ui;
    event->accept();
   // ~frmMain();
    //DeleteLater();
 }

void BMCMainWindow::ComUpdate(void)
{
    // 初始化下拉控件
    ui->cboxSerialNumber->clear();

    foreach(QSerialPortInfo info, QSerialPortInfo::availablePorts())
    {
        // 1. 获取串口的描述信息和制造商信息，统一转换为小写便于比较
        QString desc = info.description().toLower();
        QString manu = info.manufacturer().toLower();

        // 2. 核心筛选逻辑：如果描述或制造商中包含蓝牙相关的特征词，则跳过
        // 涵盖了中英文 Windows 系统下常见的蓝牙虚拟串口名称
        if (desc.contains("bluetooth") ||
            desc.contains("bthmodem") ||
            desc.contains("蓝牙") ||
            desc.contains("standard serial") || // 英文系统常显示为 "Standard Serial over Bluetooth link"
            desc.contains("标准串行"))          // 中文系统常显示为 "蓝牙链接标准串行"
        {
            continue; // 确认为蓝牙串口，直接跳过，不添加到列表
        }

        // 3. 将正常的串口名（如 COM1, COM3）添加到下拉框
        if(!info.isBusy())
        {
            ui->cboxSerialNumber->addItem(info.portName());
        }
    }
}
//void BMCMainWindow::ComUpdate()
//{
//    // 初始化下拉控件
//    ui->cboxSerialNumber->clear();
//    foreach(QSerialPortInfo info, QSerialPortInfo::availablePorts())
//    {
//        if(!info.isBusy())
//        {
//            ui->cboxSerialNumber->addItem(info.portName());
//        }
//    }
//}
void BMCMainWindow::ScanCFGFile(void)
{
//    QDesktopServices::openUrl(QUrl("file:///\\192.168.0.249/config/FinishGoods/"));
    QDir *dir=new QDir("./config/");
//    QDir *dir=new QDir("\\192.168.0.249/config/FinishGoods/");
    QStringList filter;
    filter<<"*.txt";
    dir->setNameFilters(filter);
    if(!dir->exists())
    {
     qDebug()<<"dir error";
     return;
     }
    CFGfileInfo=new QList<QFileInfo>(dir->entryInfoList(filter));
    for (int i = 0; i< CFGfileInfo->size(); i++)
    {
//        qDebug()<<fileInfo->at(i);
        QFileInfo txt1=CFGfileInfo->at(i);
        QString str = txt1.fileName();
        ui->comboBoxProductSel->addItem(str);
    }
}

void BMCMainWindow::keyPressEvent(QKeyEvent *keyValue)
{
    if(!ui->lineEditMattSilverCode->hasFocus())
    {
        if(keyValue->key() == Qt::Key_Return){//enter
            //curStr = ui->lineEdit->text();
    //        ui->lineEditBarCode13->clear();
    //        ui->lineEditBarCode8->clear();
    //        ui->lineEdit2Dcode->clear();
    //        QString str = "码值："+barStr;
    //        ui->textBrowserTestInfo->append(str);

    //        int idx = ui->comboBox_fac->currentIndex();
    //        if(idx==0)
    //        {
    //           ui->lineEdit2Dcode->setText(barStr);
    //        }
    //        else if(idx==1)
    //        {
    //            ui->lineEditBarCode8->setText(barStr);
    //        }
    //        else if(idx==2)
    //        {
    //            ui->lineEditBarCode13->setText(barStr);

    //        }
    //       int len = barStr.length();
    //       if(len==13)
    //       {
    //        //ui->lineEditBarCode13->setFocus();
    //        ui->lineEditBarCode13->setText(barStr);
    //       }
    //       else if(len==8)
    //       {
    //         //ui->lineEditBarCode8->setFocus();
    //         ui->lineEditBarCode8->setText(barStr);
    //       }
    //       else if(len==10)
    //       {
    //         //ui->lineEdit2Dcode->setFocus();
    //         ui->lineEdit2Dcode->setText(barStr);
    //       }
            BarCodeStr = barStr;
            BarCodeIsAccept = 1;
            barStr.clear();
            Scaned = 1;
            //DataBaseInsert();
        }else{
            barStr += keyValue->text();
        }
    //    qDebug() << "enter!";
    }
}

void BMCMainWindow::BarCodeProcess(void)
{
    // ===== 【优化：测试状态下的连接检测】 =====
    // 在自动测试过程中，如果通讯断开，也要显示断开状态
    if (TestProcessTaskState != 0)
    {
        if (BMSConnectIsOk)
        {
            // 测试中且连接正常
            ui->label_comstate->setPixmap(QPixmap(":/myimage/qrc/image/green.png").scaled(20,20));
            ui->label_connectState->setText("测试中...");
        }
        else
        {
            // 测试中但连接断开 - 显示红色断开状态
            ui->label_comstate->setPixmap(QPixmap(":/myimage/qrc/image/red.png").scaled(20,20));
            ui->label_connectState->setText("断开");
        }
    }
   else if(BMSConnectIsOk)
   {
    ui->label_comstate->setPixmap(QPixmap(":/myimage/qrc/image/green.png").scaled(20,20));
    ui->label_connectState->setText("通信中");
   }
   else
   {
    ui->label_comstate->setPixmap(QPixmap(":/myimage/qrc/image/red.png").scaled(20,20));
    ui->label_connectState->setText("断开");
   }
//   if(ui->lineEditMattSilverCode->hasFocus())
//   {
//        CodesWriteEnable = 1;
//       return;
//   }
//   else
//   {
       if(BarCodeIsAccept && Prohibit_Upload==0)
       {
           BarCodeIsAccept=0;
                   ui->lineEditBarCode13->clear();
                   ui->lineEditBarCode8->clear();
                   ui->lineEdit2Dcode->clear();
                   QString str = "码值："+BarCodeStr;
                   ui->textBrowserTestInfo->append(str);

                          int len = BarCodeStr.length();
                          if(len>=13) //产品序列号
                          {
                           //ui->lineEditBarCode13->setFocus();
                           ui->lineEditBarCode13->setText(BarCodeStr);
                           BarCodeStr13 = BarCodeStr;
                          }
                          else if(len==8) //内控码
                          {
                            //ui->lineEditBarCode8->setFocus();
                            ui->lineEditBarCode8->setText(BarCodeStr);
                            BarCodeStr8 = BarCodeStr;
                          }
                          else if(len==10) //保护板二维码
                          {
                            //ui->lineEdit2Dcode->setFocus();
                            ui->lineEdit2Dcode->setText(BarCodeStr);
                            BarCodeStr2D = BarCodeStr;
                          }

                  if(parameter_error)
                  {
                   ui->textBrowserTestInfo->append("参数出错,不能上传！\n");
                   return;
                  }
                  if(TestProcessError)
                  {
                   ui->textBrowserTestInfo->append("测试过程错误,不能上传！\n");
                   return;
                  }
    //              if(!BMSConnectIsOk)
    //              {
    //               ui->textBrowserTestInfo->append("保护板没连上,不能上传！\n");
    //               return;
    //              }
                  if(!DBConnectIsOk)
                  {
                   ui->textBrowserTestInfo->append("数据库没连上,不能上传！\n");
                   return;
                  }
                  if(BarCodeStr == BarCodeStr_old)
                  {
                      ui->textBrowserTestInfo->append("该码值已上传！\n");
                      return;
                  }
                  else
                  {

                     ui->label_barcode_old->setText("上一个扫码输入："+BarCodeStr_old);
                     BarCodeStr_old =  BarCodeStr;
                  }

                  if((Bms_infor_upload.BMSinfor.Pack_Status & 0x00000001) == 0)
                  {
                     ui->textBrowserTestInfo->append("没有零电流校准,不能上传！\n");
                     return;
                  }
                  if((Bms_infor_upload.BMSinfor.Pack_Status & 0x00000002) == 0)
                  {
                     ui->textBrowserTestInfo->append("没有电流校准,不能上传！\n");
                     return;
                  }
                  if((Bms_infor_upload.BMSinfor.Pack_Status & 0x00000008) == 0)
                  {
                     ui->textBrowserTestInfo->append("没有解锁,不能上传！\n");
                     return;
                  }
                  if(Bms_infor_upload.BMSinfor.Pack_Status & 0x00000010)
                  {
                     ui->textBrowserTestInfo->append("AE保护,不能上传！\n");
                     return;
                  }
                  if((Bms_infor_upload.BMSinfor.Pack_Status & 0x00000020)&&Bms_cfg_infor.EnFuse2)
                  {
                     ui->textBrowserTestInfo->append("没有开启二级保护,不能上传！\n");
                     return;
                  }
                  if(Bms_infor_upload.BMSinfor.Pack_Status & 0x00002000)
                  {
                     ui->textBrowserTestInfo->append("AFE芯片错误,不能上传！\n");
                     return;
                  }
    //              if(!resetok)
    //              {
    //                  ui->textBrowserTestInfo->append("没有按复位,不能上传！\n");
    //                  return;
    //              }


                  scan_counter++;
                  QString str1 = QString::number(scan_counter,10);
                  str1.insert(0,"数量：");
                  ui->textBrowserTestInfo->append(str1+"\n");
                   DataBaseInsert();
       }
//   }


}

void BMCMainWindow::on_btnComCheck_clicked()
{
   ComUpdate();
}

void BMCMainWindow::Get_Data()
{
    while(RArray.length() > 0)
    {
        if((uchar)RArray[0] == 0x68) // 校验帧头第一个字节 68
        {
            if (RArray.length() < 3) {
                return;
            }

            uint8_t temp2 = RArray[1];
            uint8_t temp3 = RArray[2];

            if(temp2 + temp3 == 0xff) // 判断长度字段校验
            {
                if (RArray.length() < temp2) {
                    return;
                }

                //到这里说明绝对已经完整接收了一帧
                for (uint8_t i=0; i<temp2; i++) {
                    RxArray[i] = RArray[i];
                }
                RecFlag = 1;

                //此处方便调试用
//                ui->textBrowserTestInfo->append(RxArray.toHex(' ').toUpper() + "\n");
//                ui->textBrowserTestInfo->append("\n");
                RArray.remove(0, temp2);
                return;
            }
            else
            {
                RArray.remove(0, 1);
            }
        }
        else
        {
            // 如果连 0x68 都不是，当成废数据直接抛弃
            RArray.remove(0, 1);
        }
    }
}
//void BMCMainWindow::Get_Data()
//{
//    while(RArray.length() >0)
//    {
//        if((uchar)RArray[0] == 0x68)//校验帧头第一个字节68
//        {

//               uint8_t temp2 = RArray[1];
//               uint8_t temp3 = RArray[2];
//            if(temp2+temp3==0xff)
//            {
//            for (uint8_t i=0;i<temp2;i++) {
//                RxArray[i] = RArray[i];
//            }

//            RecFlag = 1;
//            RArray.clear();
//            return;
//            }

//        }
//        else
//        {
//            RArray.remove(0, 1);
//        }
//    }

//}
// 接收数据
// 数据缓存时间
void BMCMainWindow::getCommData()
{

    QByteArray recvByteArray = serialport->readAll();


    int len = recvByteArray.length();
    if( len <=0) return ;
    RArray += recvByteArray;   //累积在缓冲区

    timer_rt.stop();
    timer_rt.start(Buffer_time);  //Buffer_time后接收数据

}

// 大循环
void BMCMainWindow:: Run()
{
    while(Status ==1)
    {
        QCoreApplication::processEvents();   //更新界面显示
        BmsConnectDetect();
        CheckIntoQueue();
        Upgrade_task();
        BaseInfoReadTask();
        TestProcessTask();
        if(!ui->lineEditMattSilverCode->hasFocus())
        {
            BarCodeProcess();
        }
        AutoTest();
    }
}
void BMCMainWindow::on_btnComOpen_clicked()
{
    if(ui->btnComOpen->text() == "打开")//打开串口之后，看选择的是什么通信方式
    {                             //根据后面的波特率和校验方式发送设置参数到通信工具
        // 配置串口，同时查看是否配置成功

        serialport->setPortName(ui->cboxSerialNumber->currentText());

        uint32_t baud = ui->cboxBaudrate->currentText().toInt();

        serialport->setBaudRate(9600); //9600
        serialport->setParity(  QSerialPort::NoParity);
        serialport->setDataBits(QSerialPort::Data8);
        serialport->setStopBits(QSerialPort::OneStop);

        if(serialport->open(QSerialPort::ReadWrite))
        {
            ui->btnComOpen->setText("关闭");
            QByteArray bt_Data;

            bt_Data[0] = 0x68;
            bt_Data[1] = 0x0c;
            bt_Data[2] = 0xff-bt_Data[1];
            if(ui->cboxComunicatMenth->currentText() == "UART")
            {bt_Data[3] = 1;}
            else if(ui->cboxComunicatMenth->currentText() == "CAN")
            {bt_Data[3] = 2;}
            else if(ui->cboxComunicatMenth->currentText() == "SMBUS")
            {bt_Data[3] = 3;}
            bt_Data[4] = 0x55; //命令
            bt_Data[5] = baud;   //扩展帧

            bt_Data[6] = baud>>8;

            bt_Data[7] = baud>>16;
            bt_Data[8] = baud>>24;
            bt_Data[9] = 0x55;
            bt_Data[10] = SocCalcXor(bt_Data,bt_Data[1]-2);
            bt_Data[11] = 0x16;
            serialport->write(bt_Data);
            //发送设置参数给通信工具
            StartReadBase = 1;
            StartPushtoQueue = 1;
            Status=1;
            Run();
        }
        else
        {
            ui->btnComOpen->setChecked(false);
         //   QMessageBox::critical(this, "错误", "串口打开失败", "确定");

         }
//        }
    }
    else
    {
        serialport->close();

        ui->btnComOpen->setText("打开");
        StartUpgrade = 0;
        StartReadBase = 0;
        StartPushtoQueue = 0;

        // ================= 【修复核心代码开始】 =================
        Status = 0;                     // 1. 退出 Run() 的 while 死循环，释放CPU
        timer_rt.stop();                // 2. 停止可能还在计时的串口接收定时器，防止幽灵回调
        RArray.clear();                 // 3. 清空串口接收积累的缓冲区
        RxArray.clear();                // 4. 清空正在处理的单帧数据
        RecFlag = 0;                    // 5. 复位接收标志位，防止 BaseInfoReadTask 误判
        readTaskStatus = Idle_Stage_rT; // 6. 读任务状态机强制复位
        // ================= 【修复核心代码结束】 =================

        // 手动关闭串口时，立刻将标志位置0，UI会瞬间变红
        BMSConnectIsOk = 0;
        ui->label_comstate->clear();

        // ================= 【修复核心代码开始】 =================
        // 因为退出了Run循环，主动调用一次UI刷新函数，确保状态立即变红且稳定
        BarCodeProcess();
        // ================= 【修复核心代码结束】 =================
    }
}
//void BMCMainWindow::ReadlineAndGath(QString *str1,QString *str2,QString *str3)
//{
////    myText = in.readLine();
////    strValName = myText.section('=',0,0);
////    strLower = myText.section('=',1,1);
//}


void BMCMainWindow::on_comboBoxProductSel_currentIndexChanged(int index)
{
  QFileInfo txt1=CFGfileInfo->at(index);
  QString path = txt1.filePath();
  qDebug()<<path;
  QFile file;
  //获取文件信息
  QFileInfo info(path);
  //指定文件的名字
  file.setFileName(path);

  bool isOk = file.open(QIODevice::ReadOnly);

  if(true == isOk)
  {
//      qDebug()<<"openisok";
    QTextStream in(&file);
    in.setCodec("UTF-8");
//    while (!in.atEnd())
//    {
//      QString myText = in.readLine();    //按行读一行文件
//      qDebug()<<myText;

//    }
    QString myText = in.readLine();
    QString strValName,strUpper,strLower;
    if(myText=="STARTCFG")
    {



     ReadlineAndGath();
     if(strValName=="BMSERPCode")
     {
      Bms_cfg_infor.BMSERPCode = strLower.section(' ',0,0);
      Bms_infor_upload.BMSERPCode = Bms_cfg_infor.BMSERPCode;
      ui->lineEditBoardCode->setText(Bms_cfg_infor.BMSERPCode);
     }

     ReadlineAndGath();
     if(strValName=="FirmwareERPCode")
     {
      Bms_cfg_infor.FirmwareERPCode = strLower.section(' ',0,0);
      Bms_infor_upload.FirmwareERPCode = Bms_cfg_infor.FirmwareERPCode;
      ui->lineEditSoftCode->setText(Bms_cfg_infor.FirmwareERPCode);
     }

     ReadlineAndGath();
     if(strValName=="SWRevsion")
     {
      Bms_cfg_infor.Version = strLower.section(' ',0,0);
//      Bms_infor_upload.BMSinfor.SWVersion = Bms_cfg_infor.Version;
      ui->lineEditVersion->setText(Bms_cfg_infor.Version);
     }

     ReadlineAndGath();
     if(strValName=="CanBeUpdate")
     {
      Bms_cfg_infor.CanBeUpdate = strLower.section(' ',0,0);
      Bms_infor_upload.CanBeUpdate = Bms_cfg_infor.CanBeUpdate;
      if(Bms_cfg_infor.CanBeUpdate=="Yes")
      ui->comboBox_upgrade->setCurrentIndex(0);
      else if(Bms_cfg_infor.CanBeUpdate=="No")
       ui->comboBox_upgrade->setCurrentIndex(1);
     }
     ReadlineAndGath();
     if(strValName=="TestMeth")
     {
      Bms_cfg_infor.TestMeth = strLower.section(' ',0,0);
      if(Bms_cfg_infor.TestMeth=="Manual")
      ui->comboBoxTestMode->setCurrentIndex(0);
      else if(Bms_cfg_infor.TestMeth=="Auto")
       ui->comboBoxTestMode->setCurrentIndex(1);
//      else if(Bms_cfg_infor.TestMeth=="Auto2")
//       ui->comboBoxTestMode->setCurrentIndex(2);
     }

     myText = in.readLine();
     strValName = myText.section('=',0,0);
     strLower = myText.section('=',1,1);
     strUpper = myText.section('=',2,2);
     if(strValName=="SOCDetect")
     {
      Bms_cfg_infor.SOCDetect_lower = strLower.toUInt();
      QString s=strUpper.section(' ',0,0);
      Bms_cfg_infor.SOCDetect_upper = s.toUInt();
     }

     myText = in.readLine();
     strValName = myText.section('=',0,0);
     strLower = myText.section('=',1,1);
     strUpper = myText.section('=',2,2);
     if(strValName=="VoltDetect")
     {
      Bms_cfg_infor.VoltDetect_lower = strLower.toUInt();
      QString s=strUpper.section(' ',0,0);
      Bms_cfg_infor.VoltDetect_upper = s.toUInt();
     }

     ReadlineAndGath();
     if(strValName=="CurVarify")
     {
       QString s =  strLower.section(' ',0,0);
      Bms_cfg_infor.CurVarify = s.toUInt();

     }

     ReadlineAndGath();
     if(strValName=="CurVarifyDelay")
     {
       QString s =  strLower.section(' ',0,0);
      Bms_cfg_infor.CurVarifyDelay = s.toUInt();

     }

     myText = in.readLine();
     strValName = myText.section('=',0,0);
     strLower = myText.section('=',1,1);
     strUpper = myText.section('=',2,2);
     if(strValName=="CurDetect")
     {
      Bms_cfg_infor.CurDetect_lower = strLower.toUInt();

      QString s=strUpper.section(' ',0,0);
      Bms_cfg_infor.CurDetect_upper = s.toUInt();

     }

     myText = in.readLine();
     strValName = myText.section('=',0,0);
     strLower = myText.section('=',1,1);
     strUpper = myText.section('=',2,2);
     if(strValName=="VoltDiff")
     {
      Bms_cfg_infor.VoltDiff_lower = strLower.toUInt();

      QString s=strUpper.section(' ',0,0);
      Bms_cfg_infor.VoltDiff_upper = s.toUInt();

     }

     ReadlineAndGath();
     if(strValName=="DC")
     {
       QString s =  strLower.section(' ',0,0);
      Bms_cfg_infor.BMSinfor.DC= s.toUInt();
     }

     ReadlineAndGath();
     if(strValName=="ShutDTime")
     {
       QString s =  strLower.section(' ',0,0);
      Bms_cfg_infor.BMSinfor.ShutDTime= s.toUInt();
     }

     ReadlineAndGath();
     if(strValName=="BalEndV")
     {
       QString s =  strLower.section(' ',0,0);
      Bms_cfg_infor.BMSinfor.BalEndV= s.toUInt();
     }

     ReadlineAndGath();
     if(strValName=="ChgEndCur")
     {
       QString s =  strLower.section(' ',0,0);
      Bms_cfg_infor.BMSinfor.ChgEndCur= s.toUInt();
     }

     ReadlineAndGath();
     if(strValName=="ChgEndVol")
     {
       QString s =  strLower.section(' ',0,0);
      Bms_cfg_infor.BMSinfor.ChgEndVol= s.toUInt();
     }

     ReadlineAndGath();
     if(strValName=="ChgEndDel")
     {
       QString s =  strLower.section(' ',0,0);
      Bms_cfg_infor.BMSinfor.ChgEndDel= s.toUInt();
     }

     ReadlineAndGath();
     if(strValName=="SlfDsqBalV")
     {
       QString s =  strLower.section(' ',0,0);
      Bms_cfg_infor.BMSinfor.SlfDsqBalV= s.toUInt();
     }

     ReadlineAndGath();
     if(strValName=="BalStartV")
     {
       QString s =  strLower.section(' ',0,0);
      Bms_cfg_infor.BMSinfor.BalStartV= s.toUInt();
     }

     ReadlineAndGath();
     if(strValName=="BalDelay")
     {
       QString s =  strLower.section(' ',0,0);
      Bms_cfg_infor.BMSinfor.BalDelay= s.toUInt();
     }

     ReadlineAndGath();
     if(strValName=="CellNum")
     {
       QString s =  strLower.section(' ',0,0);
      Bms_cfg_infor.BMSinfor.CellNum= s.toUInt();
     }

     ReadlineAndGath();
     if(strValName=="MNFDate")
     {
       QString s =  strLower.section(' ',0,0);
      Bms_cfg_infor.BMSinfor.MNFDate= s.toUInt();
     }

     ReadlineAndGath();
     if(strValName=="SlfDsgDel")
     {
       QString s =  strLower.section(' ',0,0);
      Bms_cfg_infor.BMSinfor.SlfDsgDel= s.toUInt();
     }

     ReadlineAndGath();
     if(strValName=="SampleRV")
     {
       QString s =  strLower.section(' ',0,0);
      Bms_cfg_infor.BMSinfor.SampleRV= s.toFloat();
     }

     ReadlineAndGath();
     if(strValName=="OV")
     {
       QString s =  strLower.section(' ',0,0);
      Bms_cfg_infor.BMSinfor.OV= s.toUInt();
     }

     ReadlineAndGath();
     if(strValName=="OVR")
     {
       QString s =  strLower.section(' ',0,0);
      Bms_cfg_infor.BMSinfor.OVR= s.toUInt();
     }

     ReadlineAndGath();
     if(strValName=="OVT")
     {
       QString s =  strLower.section(' ',0,0);
      Bms_cfg_infor.BMSinfor.OVT= s.toUInt();
     }

     ReadlineAndGath();
     if(strValName=="UV")
     {
       QString s =  strLower.section(' ',0,0);
      Bms_cfg_infor.BMSinfor.UV= s.toUInt();
     }

     ReadlineAndGath();
     if(strValName=="UVR")
     {
       QString s =  strLower.section(' ',0,0);
      Bms_cfg_infor.BMSinfor.UVR= s.toUInt();
     }

     ReadlineAndGath();
     if(strValName=="UVT")
     {
       QString s =  strLower.section(' ',0,0);
      Bms_cfg_infor.BMSinfor.UVT= s.toUInt();
     }

     ReadlineAndGath();
     if(strValName=="BALV")
     {
       QString s =  strLower.section(' ',0,0);
      Bms_cfg_infor.BMSinfor.BALV= s.toUInt();
     }

     ReadlineAndGath();
     if(strValName=="PREV")
     {
       QString s =  strLower.section(' ',0,0);
      Bms_cfg_infor.BMSinfor.PREV= s.toUInt();
     }

     ReadlineAndGath();
     if(strValName=="LZV")
     {
       QString s =  strLower.section(' ',0,0);
      Bms_cfg_infor.BMSinfor.LZV= s.toUInt();
     }
     ReadlineAndGath();
     if(strValName=="PFV")
     {
       QString s =  strLower.section(' ',0,0);
      Bms_cfg_infor.BMSinfor.PFV= s.toUInt();
     }
     ReadlineAndGath();
     if(strValName=="OCD1V")
     {
       QString s =  strLower.section(' ',0,0);
      Bms_cfg_infor.BMSinfor.OCD1V= s.toUInt();
     }
     ReadlineAndGath();
     if(strValName=="OCD1T")
     {
       QString s =  strLower.section(' ',0,0);
      Bms_cfg_infor.BMSinfor.OCD1T= s.toUInt();
     }
     ReadlineAndGath();
     if(strValName=="OCD2V")
     {
       QString s =  strLower.section(' ',0,0);
      Bms_cfg_infor.BMSinfor.OCD2V= s.toUInt();
     }
     ReadlineAndGath();
     if(strValName=="OCD2T")
     {
       QString s =  strLower.section(' ',0,0);
      Bms_cfg_infor.BMSinfor.OCD2T= s.toUInt();
     }

     ReadlineAndGath();
     if(strValName=="SCV")
     {
       QString s =  strLower.section(' ',0,0);
      Bms_cfg_infor.BMSinfor.SCV= s.toUInt();
     }

     ReadlineAndGath();
     if(strValName=="SCT")
     {
       QString s =  strLower.section(' ',0,0);
      Bms_cfg_infor.BMSinfor.SCT= s.toUInt();
     }

     ReadlineAndGath();
     if(strValName=="OCCV")
     {
       QString s =  strLower.section(' ',0,0);
      Bms_cfg_infor.BMSinfor.OCCV= s.toUInt();
     }

     ReadlineAndGath();
     if(strValName=="OCCT")
     {
       QString s =  strLower.section(' ',0,0);
      Bms_cfg_infor.BMSinfor.OCCT= s.toUInt();
     }

     ReadlineAndGath();
     if(strValName=="EOTC")
     {
       QString s =  strLower.section(' ',0,0);
      Bms_cfg_infor.BMSinfor.EOTC= s.toInt();
     }
     ReadlineAndGath();
     if(strValName=="EOTCR")
     {
       QString s =  strLower.section(' ',0,0);
      Bms_cfg_infor.BMSinfor.EOTCR= s.toInt();
     }
     ReadlineAndGath();
     if(strValName=="EUTC")
     {
       QString s =  strLower.section(' ',0,0);
      Bms_cfg_infor.BMSinfor.EUTC= s.toInt();
     }
     ReadlineAndGath();
     if(strValName=="EUTCR")
     {
       QString s =  strLower.section(' ',0,0);
      Bms_cfg_infor.BMSinfor.EUTCR= s.toInt();
     }
     ReadlineAndGath();
     if(strValName=="EOTD")
     {
       QString s =  strLower.section(' ',0,0);
      Bms_cfg_infor.BMSinfor.EOTD= s.toInt();
     }
     ReadlineAndGath();
     if(strValName=="EOTDR")
     {
       QString s =  strLower.section(' ',0,0);
      Bms_cfg_infor.BMSinfor.EOTDR= s.toInt();
     }
     ReadlineAndGath();
     if(strValName=="EUTD")
     {
       QString s =  strLower.section(' ',0,0);
      Bms_cfg_infor.BMSinfor.EUTD= s.toInt();
     }
     ReadlineAndGath();
     if(strValName=="EUTDR")
     {
       QString s =  strLower.section(' ',0,0);
      Bms_cfg_infor.BMSinfor.EUTDR= s.toInt();
     }
     ReadlineAndGath();
     if(strValName=="SOCC")
     {
       QString s =  strLower.section(' ',0,0);
      Bms_cfg_infor.BMSinfor.SOCC= s.toUInt();
     }

     ReadlineAndGath();
     if(strValName=="SOCCD")
     {
       QString s =  strLower.section(' ',0,0);
      Bms_cfg_infor.BMSinfor.SOCCD= s.toUInt();
     }

     ReadlineAndGath();
     if(strValName=="EFUSE")
     {
       QString s =  strLower.section(' ',0,0);
      Bms_cfg_infor.EnFuse2= s.toUInt();
     }

     ReadlineAndGath();
     if(strValName=="WriteSN")
     {
       QString s =  strLower.section(' ',0,0);
      Bms_cfg_infor.EnWriteSN= s.toUInt();
     }

     ReadlineAndGath();
     if(strValName=="ELECWORKSHOP")
     {
       QString s =  strLower.section(' ',0,0);
      Bms_cfg_infor.ELECWORKSHOPSteps= s;
     }

     ReadlineAndGath();
     if(strValName=="PACKWORKSHOP")
     {
       QString s =  strLower.section(' ',0,0);
      Bms_cfg_infor.PACKWORKSHOPSteps= s;
     }

     ReadlineAndGath();
     if(strValName=="WRAPWORKSHOP")
     {
       QString s =  strLower.section(' ',0,0);
      Bms_cfg_infor.WRAPWORKSHOPSteps= s;
     }


    }


  }
  file.close();
}


void BMCMainWindow::sendHandShake(uint8_t idx,uint8_t num_cmd)
{
    QByteArray bt_Data;

    bt_Data[0] = 0x68;
    bt_Data[1] = 0x0a;
    bt_Data[2] = 0xff-bt_Data[1];
    bt_Data[3] = idx;
    bt_Data[4] = 0x03;
    bt_Data[5] = 0x0c;
    bt_Data[6] = 0x02;
    bt_Data[7] = num_cmd;
    bt_Data[8] = SocCalcXor(bt_Data,bt_Data[1]-2);
    bt_Data[9] = 0x16;

    serialport->write(bt_Data);
}


void BMCMainWindow::sendBLCMD(uint8_t len,uint8_t FrameOrder,uint64_t DevID,uint8_t subCMD,uint32_t FlashAddr,QByteArray lData)
{
    QByteArray bt_Data;

    bt_Data[0] = 0x68;
    bt_Data[1] = len;
    bt_Data[2] = 0xff-len;
    bt_Data[3] = FrameOrder;
    bt_Data[4] = DevID>>32;
    bt_Data[5] = DevID>>24;
    bt_Data[6] = DevID>>16;
    bt_Data[7] = DevID>>8;
    bt_Data[8] = DevID>>0;
    bt_Data[9] = len-12;
    bt_Data[10] = 0x0c;
    bt_Data[11] = subCMD;
    bt_Data[12] = FlashAddr>>24;
    bt_Data[13] = FlashAddr>>16;
    bt_Data[14] = FlashAddr>>8;
    bt_Data[15] = FlashAddr;
    uint16_t llen = lData.length();
//    qDebug("llen1: %x",llen);
    for (uint16_t i=0;i<llen;i++) {
     bt_Data[16+i] = lData[i];
    }
    bt_Data[len-2] = SocCalcXor(bt_Data,len-2);
    bt_Data[len-1] = 0x16;
//    qDebug("llen2: %x",llen);


    serialport->write(bt_Data);
//    qDebug("llen3: %x",llen);
}

//0x68, 0x0c, 0xf3, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x97, 0x16
void BMCMainWindow::sendReadCMD(uint8_t *dat,uint8_t size)
{
    QByteArray bt_Data;
    uint8_t playload_length = dat[1]-6;
//    uint8_t size = sizeof (dat);
    bt_Data[0] = dat[0];
    bt_Data[1] = dat[1];
    bt_Data[2] = dat[2];
    bt_Data[3] = dat[3];
    for (uint8_t i=0;i<playload_length;i++) {
      bt_Data[4+i] = dat[4+i];
    }
    bt_Data[4+playload_length] = dat[size-2];
    bt_Data[5+playload_length] = dat[size-1];
//    bt_Data[4] = dat[4];
//    bt_Data[5] = dat[5];
//    bt_Data[6] = dat[6];
//    bt_Data[7] = dat[7];
//    bt_Data[8] = dat[8];
//    bt_Data[9] = dat[9];
//    bt_Data[10] = dat[10];
//    bt_Data[11] = dat[11];
//    ui->textBrowserTestInfo->insertHtml("<br>" + QString("<span style='color:blue;'>%1</span>").arg(QString::fromLatin1(bt_Data.toHex(' ').toUpper())) + "<br>");
    serialport->write(bt_Data);
}

void BMCMainWindow::ReadWriteToBIN(void)
{

    QString EnterResul="\n";
    uint32_t line_count = 0;

    QByteArray src1;
    Filebuffer.clear();
    line_count = 0;
    QTextStream in(&file);
//    in.readLine();
    HexBaseAddr = 0;
    HexOffsetAddr = 0;
    while (!in.atEnd())
    {
         unsigned int pptrsize=0;
         src1.clear();
         QString myText = in.readLine();    //按行读一行文件
         line_count++;
//         qDebug()<<myText;
         fla2bin.FormatHexjudge(&Flashtohex_ins,myText.toLatin1());           //分割  此处数据段后应该没有效验值
         QString type_back=Flashtohex_ins.FlashPartHead.mid(7,2);
         if(type_back=="00")// 是记录数据的
         {
             if(line_count==2)
             {
               QString offsetstr=Flashtohex_ins.FlashPartHead.mid(3,4);
               fla2bin.QString2intArray2(parat,offsetstr);
               HexOffsetAddr = parat[0];
               HexOffsetAddr <<= 8;
               HexOffsetAddr |=  parat[1];
               HexOffsetAddr &= 0x0000ffff;
             }
         pptrsize=(unsigned int)(Flashtohex_ins.FlashPartPartDate.size())/2; //计算数组的有效长度
         fla2bin.QString2intArray2(parat,Flashtohex_ins.FlashPartPartDate);
//         qDebug()<<Flashtohex_ins.FlashPartPartDate;
         for (uint32_t i=0;i<pptrsize-1;i++) {
           src1[i] = parat[i];
         }
         Filebuffer.append(src1);
         }
         else if(line_count==1&&type_back=="04")
         {
             pptrsize=(unsigned int)(Flashtohex_ins.FlashPartPartDate.size())/2; //计算数组的有效长度
             fla2bin.QString2intArray2(parat,Flashtohex_ins.FlashPartPartDate);
    //         qDebug()<<Flashtohex_ins.FlashPartPartDate;

             HexBaseAddr = parat[0];
             HexBaseAddr <<= 8;
             HexBaseAddr |=  parat[1];
             HexBaseAddr &= 0x0000ffff;
             HexBaseAddr <<= 16;
         }

    }//file.close();

    FlashAddr = HexBaseAddr + HexOffsetAddr;
  //  qDebug("FlashAddr: %x",FlashAddr);
}

void BMCMainWindow::on_btnOpenFile_clicked()
{
    path=QFileDialog::getOpenFileName(this,
                                                      "打开文件",
                                                      path);
    if(false == path.isEmpty()) //如果选择文件路径有效
    {
        fileName.clear();
        fileSize = 0;

        //获取文件信息
        QFileInfo info(path);
        fileName = info.fileName(); //获取文件名字
 //       fileSize = info.size(); //获取文件大小

        sendSize = 0; //发送文件的大小
        suffix_is_hex = 0;
        //只读方式打开文件
        //指定文件的名字
        file.setFileName(path);
        bool isOk = file.open(QIODevice::ReadOnly);

        if(false == isOk)
        {
            //file.close();
           // qDebug() << "只读方式打开文件失败 106";
            QMessageBox::information(NULL, "ERROR", "CAN NOT OPEN");
            return;

        }
        else if(("HEX"==info.suffix())||("hex"==info.suffix()))
        {

            suffix_is_hex = 1;
            ReadWriteToBIN();
            uint32_t lenght = Filebuffer.length();
            checksum = 0;

            checkxor = 0;
            fileSize = lenght;
  //          qDebug("lenght: %x",lenght);
           uint8_t tu = Filebuffer[0];
           uint8_t tu1 = Filebuffer[1];
           uint16_t  temp =0;
 //           qDebug("Filebuffer: %x",tu);
            for (uint32_t i=0;i<lenght;i+=2) {
             temp =0;
             tu = Filebuffer[i+1];  //QByteArray类要用8位的变量接收，否则高字节变为FF
             tu1 = Filebuffer[i];
             temp = tu;
             temp <<= 8;
             temp |= tu1;
             checksum += temp;
             checkxor ^= temp;
 //            qDebug("Filebuffer: %x",tu);
 //            qDebug("Filebuffer: %x",tu1);
 //            qDebug("Filebuffer: %x",temp);
            }
            //qDebug()<<"Filebuffer:";//<<Filebuffer;

           QString str1 = "SUM check:";
           QString str =  QString::number(checksum,16);//转换为十六进制
           str1.append(str);
           str1.append("  XOR check:");
           str = QString::number(checkxor,16);
           str1.append(str);
            ui->labelCheck->setText(str1);
        }

        file.close();

     ui->labelLocation->setText(path);
    }
    else
    {
      QMessageBox::information(NULL, "ERROR", "NULL");
    }
}

void BMCMainWindow::on_btnStartUpgrade_clicked()
{
    if(!BMSConnectIsOk)return;
    StartUpgrade = 1;
    StartReadBase = 0; //close base info read task
    StartPushtoQueue = 0;
    AllowBmsConnectDetect = 0;
    UpgradeStatus = Idle_Stage;
    ui->progressBar->setRange(0,100);
    ui->progressBar->setOrientation(Qt::Horizontal);
    ui->progressBar->reset();
    FlashAddr = HexBaseAddr + HexOffsetAddr;
}

void BMCMainWindow::BmsConnectDetect(void)   //升级时要停这个函数
{

  static uint32_t count = 0;
  static  txStruct txTemp;
  int sec = ui->comboBox_BeforTest->currentIndex();
  int Testmod = ui->comboBoxTestMode->currentIndex();
    if(!AllowBmsConnectDetect)return;





}

void BMCMainWindow::Upgrade_task(void)
{
    switch(UpgradeStatus)
        {
            case Idle_Stage:
            if((suffix_is_hex == 1)&&(readTaskStatus==Idle_Stage_rT))
            {
            if(StartUpgrade)
            {

             StartUpgrade = 0;
             if(ui->cboxComunicatMenth->currentText() == "UART")
             {Buffer_time = 15;}
             else
             {Buffer_time = 70;}

             sendHandShake(0,1);
             gTick = 0;
             RecFlag = 0;
             UpgradeStatus = HandShake1_Stage;
             RxArray.clear();
             ui->labelUpgradeState->setText("sendHandShake！");
            }
            }
            break;
            case HandShake1_Stage:
            if(gTick>=100)
            {
//                ui->labelUpgradeState->setText("TIME OUT！");
//                return;
                RxArray.clear();
                sendHandShake(1,2);
                gTick = 0;
                RecFlag = 0;
                UpgradeStatus = HandShake2_Stage;
            }
            else if(RecFlag==1)
            {
              uint8_t temp = RxArray[0];
              uint8_t len = RxArray[1];
              uint8_t check = RxArray[len-2];
              if((temp==0x68) &&(check==SocCalcXor(RxArray,len-2)))
              {
                //zgj 2025.10.15 Start
//                BMSConnectIsOk = 1;
                //zgj 2025.10.15 End
                temp = RxArray[3];
                if(temp==0x00)
                {
                    RxArray.clear();
                    sendHandShake(1,2);
                    gTick = 0;
                    RecFlag = 0;
                    UpgradeStatus = HandShake2_Stage;
                }
              }
            }
            break;
            case HandShake2_Stage:
            if(gTick>=150)
            {
                ui->labelUpgradeState->setText("TIME OUT！");
                StartReadBase = 1;
                AllowBmsConnectDetect = 1;
                UpgradeStatus = Idle_Stage;
                return;
            }
            else if(RecFlag==1)
            {
              uint8_t temp = RxArray[0];

              uint8_t len = RxArray[1];
              uint8_t check = RxArray[len-2];
              if((temp==0x68) &&(check==SocCalcXor(RxArray,len-2)))
              {
                temp = RxArray[3];
                if(temp==0x01)
                {
                    RxArray.clear();
                    //sendHandShake(1,2);
                    gTick = 0;
                    RecFlag = 0;
                    UpgradeStatus = Wait_Stage;
                    ui->labelUpgradeState->setText("HandShake OK！");
                    FrameOrder = 0;
                    ui->btnComOpen->setEnabled(false);
//                    ui->pushButtonResume->setEnabled(false);
//                    ui->pushButtonReset->setEnabled(false);
                }
              }
            }
            break;
            case Wait_Stage:
            if(gTick>=2800)
            {
             QByteArray nData;
             nData[0] = 0x55;
             nData[1] = 0x55;

             sendBLCMD(20,FrameOrder,0,0x02,FlashAddr,nData);
             gTick = 0;
             RecFlag = 0;
             FrameOrder ++;
             UpgradeStatus = Start_Stage;
             RxArray.clear();
//             fileSize = Filebuffer.length();
             LastBulkByte = fileSize%128;
             if(LastBulkByte==0)
             { BulkNum=(fileSize)/128; LastBulkByte = 128;}
             else{BulkNum=(fileSize)/128+1;}
             BulkNumSended = 0;
//             qDebug() << "Start_Stage 106"; //<<QString::number(BulkNum,16)
            }
            break;
            case Start_Stage:
            if(gTick>=150)
            {
                ui->labelUpgradeState->setText("HandShake ERROR！");
                UpgradeStatus = Idle_Stage;
                StartReadBase = 1;
                AllowBmsConnectDetect = 1;
                ui->btnComOpen->setEnabled(true);
//                ui->pushButtonResume->setEnabled(true);
//                ui->pushButtonReset->setEnabled(true);
                return;
            }
            else if(RecFlag==1)
            {
              uint8_t temp = RxArray[0];
              uint8_t len = RxArray[1];
              uint8_t check = RxArray[len-2];
//                              for (uint8_t i=0;i<len;i++) {
//                               uint8_t tu=RxArray[i];
//                              qDebug("RxArray: %x",tu);
//                             }
              if((temp==0x68) &&(check==SocCalcXor(RxArray,len-2)))
              {
                temp = RxArray[16];
                if(temp==0x55)//temp==0x55
                {




                        //每次发送数据的大小
                        QByteArray buf;

                        uint32_t j = 0;
                        for (uint32_t i=0;i<128;i++) {
                            j=BulkNumSended*128+i;
                            buf[i] = Filebuffer[j];
                        }
 //                       qDebug() << "Start_Stage 107";
                        sendBLCMD(146,FrameOrder,0,0x03,FlashAddr,buf);
//                    qDebug() << "Start_Stage 107";
//                    QByteArray nData;
//                    nData[0] = 0x55;
//                    nData[1] = 0x55;

//                    sendBLCMD(20,FrameOrder,0,0x02,FlashAddr,nData);
                        //发送的数据需要累积
                        BulkNumSended ++;
                        FrameOrder++;
                        FlashAddr += 128;
                        RxArray.clear();
                        gTick = 0;
                        RecFlag = 0;
                        UpgradeStatus = Sending_Stage;
                        ui->labelUpgradeState->setText("Updating...");
                        ui->progressBar->setValue(BulkNumSended*100/BulkNum);
//                      qDebug() << "Start_Stage 108";

                }
              }
            }
            break;
            case Sending_Stage:
            if(gTick>=600)
            {
                ui->labelUpgradeState->setText("Update defeat！");
                //file.close();
                UpgradeStatus = Idle_Stage;
                StartReadBase = 1;
                AllowBmsConnectDetect = 1;
                ui->btnComOpen->setEnabled(true);
//                ui->pushButtonResume->setEnabled(true);
//                ui->pushButtonReset->setEnabled(true);
                return;
            }
            else if(RecFlag==1)
            {
              uint8_t temp = RxArray[0];
              if(temp!=0x68)return;
              uint8_t len = RxArray[1];

              uint8_t check = RxArray[len-2];
              if((temp==0x68) &&(check==SocCalcXor(RxArray,len-2)))
              {
                temp = RxArray[16];
                if(temp==0x55)//temp==0x55
                {
                 RxArray.clear();
//                 qDebug() << "Start_Stage 109";
                 if(BulkNumSended<BulkNum)
                 {
                     QByteArray buf;
                     uint32_t j = 0;
                     for (uint32_t i=0;i<128;i++) {

                         buf[i] = 0;
                     }
                     if(BulkNumSended==BulkNum-1) //最后一块
                     {
                         for (uint32_t i=0;i<LastBulkByte;i++) {
                             j=BulkNumSended*128+i;
                             buf[i] = Filebuffer[j];
                         }
                     }
                     else
                     {
                         for (uint32_t i=0;i<128;i++) {
                             j=BulkNumSended*128+i;
                             buf[i] = Filebuffer[j];
                         }
                     }


                     sendBLCMD(146,FrameOrder,0,0x03,FlashAddr,buf);
                     //发送的数据需要累积
                     BulkNumSended ++;
                     FrameOrder++;
                     FlashAddr += 128;
                     RxArray.clear();
                     gTick = 0;
                     RecFlag = 0;
                     UpgradeStatus = Sending_Stage;
                     ui->progressBar->setValue(BulkNumSended*100/BulkNum);
                 }
                 else //
                 {
                  QByteArray buf;
                  buf[0] = ((fileSize/2)&0xffff)>>8;  //校验半字数
                  buf[1] = ((fileSize/2)&0xff)>>0;
                  buf[2] = checksum >> 24;  //校验和
                  buf[3] = checksum >> 16;
                  buf[4] = checksum >> 8;
                  buf[5] = checksum;
                  buf[6] = checkxor >> 8;  //校验异或
                  buf[7] = checkxor;
                  sendBLCMD(0x1A,FrameOrder,0,0x04,(HexBaseAddr+HexOffsetAddr),buf);
                  FrameOrder++;
                  RxArray.clear();
                  gTick = 0;
                  RecFlag = 0;
                  UpgradeStatus = Ending_Stage;
                 }
                }
              }
            }
            break;
            case Ending_Stage:
            if(gTick>=1000)
            {
                ui->labelUpgradeState->setText("verify defeat！");
                UpgradeStatus = Idle_Stage;
                StartReadBase = 1;
                AllowBmsConnectDetect = 1;
                ui->btnComOpen->setEnabled(true);
//                ui->pushButtonResume->setEnabled(true);
//                ui->pushButtonReset->setEnabled(true);
                return;
            }
            else if(RecFlag==1)
            {
              uint8_t temp = RxArray[0];
              if(temp!=0x68)return;
              uint8_t len = RxArray[1];
              uint8_t check = RxArray[len-2];
              if((temp==0x68) &&(check==SocCalcXor(RxArray,len-2)))
              {
                temp = RxArray[16];
                if(temp==0x55)//temp==0x55
                {
                    QByteArray buf;
                    buf[0] = 0x55;
                    buf[1] = 0x55;
                    sendBLCMD(0x14,FrameOrder,0,0x05,(HexBaseAddr+HexOffsetAddr),buf);
                    FrameOrder++;
                    RxArray.clear();
                    gTick = 0;
                    RecFlag = 0;
                    UpgradeStatus = Reset_Stage;

                }
              }
            }



            break;
            case Reset_Stage:
            if(gTick>=1000)
            {
                ui->labelUpgradeState->setText("reset defeat！");
                StartReadBase = 1;
                AllowBmsConnectDetect = 1;
                ui->btnComOpen->setEnabled(true);
//                ui->pushButtonResume->setEnabled(true);
//                ui->pushButtonReset->setEnabled(true);
                return;
            }
            else if(RecFlag==1)
            {
              uint8_t temp = RxArray[0];
              uint8_t len = RxArray[1];
              uint8_t check = RxArray[len-2];
              if((temp==0x68) &&(check==SocCalcXor(RxArray,len-2)))
              {
                temp = RxArray[16];
                if(temp==0x55)//temp==0x55
                {

                  ui->labelUpgradeState->setText("Update success！");
                  QMessageBox::information(NULL, "Completed", "Update success！");
                  UpgradeStatus = Idle_Stage;
                  StartReadBase = 1;
                  AllowBmsConnectDetect = 1;
                  readTaskStatus = Idle_Stage_rT;
                  StartPushtoQueue = 1;
                  ui->btnComOpen->setEnabled(true);
//                  ui->pushButtonResume->setEnabled(true);
//                  ui->pushButtonReset->setEnabled(true);
                }
              }
            }
            break;
    }
}

void BMCMainWindow::MyFunction_Timeout(void)
{

    gTick++;
    readTick++;
    TestProcessTick++;
    AutoTestTick++;
    BmsConnectDetectTick++;
    secondTick++;
    if(secondTick>=100)
    {
       secondTick = 0;
//       ReadBaseInfoEnable = 1;
//       if(allowparread)ReadSysPraEnable = 1;
       //zgj 2025.12.24 Start
       // 【新增】如果队列里已经有积压（超过4条），就先别加新的查询指令了！
      if(uartTxVector.size() < 3)
      {
          ReadBaseInfoEnable = 1;
          if(allowparread)ReadSysPraEnable = 1;
      }
      //zgj 2025.12.24 end
       if(Bms_cfg_infor.EnWriteSN) {EsendSNRead=1;}
       QDateTime current_date_time = QDateTime::currentDateTime();
       QString str_current_date =current_date_time.toString("yyyy-MM-dd hh:mm:ss");
       ui->dateTimeEditMNF->setText(str_current_date);
       Second20++;
       if(Second20>=20)
       {
           Second20 = 0;
           if(DBConnectIsOk==0)
           {
//             Sthread->start();
           }
       }
    }
//    if(deltaVolt>AllowdeltaVolt)
//    {

//        QPalette pa;
//        pa.setColor(QPalette::WindowText,Qt::red);
//        ui->labelDeltaVolt->setPalette(pa);
//      }
//    else
//    {
//        QPalette pa;
//        pa.setColor(QPalette::WindowText,QColor("#138008"));
//        ui->labelDeltaVolt->setPalette(pa);
//    }

}


void BMCMainWindow::ClearBaseInfo(void)
{
    QString str = " ";
    for (uint8_t i = 0;i < 16;i++) {

        ui->tableWidgetSystemPara->setItem(i,1,new QTableWidgetItem(str));
    }
    for (uint8_t i = 0;i < 16;i++) {

        ui->tableWidgetSystemPara->setItem(i,4,new QTableWidgetItem(str));
    }
    for (uint8_t i = 0;i < 16;i++) {

        ui->tableWidgetSystemPara->setItem(i,7,new QTableWidgetItem(str));
    }
    for (uint8_t i = 0;i < 17;i++) {

        ui->tableWidgetSystemPara->setItem(i,10,new QTableWidgetItem(str));
    }

    for (uint8_t i = 0;i < 8;i++) {

        ui->tableWidgetSystemPara->setItem(15,1+i,new QTableWidgetItem(str));
    }
    for (uint8_t i = 0;i < 8;i++) {

        ui->tableWidgetSystemPara->setItem(16,1+i,new QTableWidgetItem(str));
    }
}


void BMCMainWindow::BaseInfoReadTask(void)
{
    static  txStruct txTemp;
    txStruct_System_parameter  txTemp_System_parameter;
    switch(readTaskStatus)
        {
            case Idle_Stage_rT:
            if((uartTxVector.count()>0 || uartTxVector_System_parameter.count()>0) && StartReadBase) //队列中是否有数据
            {
                if(uartTxVector_System_parameter.count() > 0)
                {
                    // 1. 取出队列中的第一个包
                    txTemp_System_parameter = uartTxVector_System_parameter[0];
                    // 2. 从队列中移除
                    uartTxVector_System_parameter.remove(0, 1);

                    // 【关键修改】不要写死 222！而是读取这一帧实际设置的长度。
                    // 这样既能兼容旧的系统参数（长度222），也能兼容新的条码（长度约100多）
                    uint8_t realSize = txTemp_System_parameter.uFrame.sFrame.Frame_Length;

                    // 3. 发送实际长度的数据
                    // 注意：sendReadCMD 的第二个参数现在是动态的 realSize
                    sendReadCMD(txTemp_System_parameter.uFrame.alldat, realSize);

                    // 4. 更新 txTemp 的命令字，以便后续逻辑（如超时判断、进度条等）能知道刚才发了什么
                    txTemp.uFrame.sFrame.Frame_cmd = txTemp_System_parameter.uFrame.sFrame.Frame_cmd;
                }
                if(uartTxVector.count()>0)
                {
                    txTemp =  uartTxVector[0];
                    uartTxVector.remove(0,1);
                    sendReadCMD(txTemp.uFrame.alldat,72);
//                    sendReadCMD(txTemp.uFrame.alldat,102);
                }
                //zgj 2025.12.4start

//                if(uartTxVector.count()>0)
//                {
//                    txTemp =  uartTxVector[0];
//                    uartTxVector.remove(0,1);
//                    sendReadCMD(txTemp.uFrame.alldat,72);
////                    sendReadCMD(txTemp.uFrame.alldat,102);
//                }
//                //zgj 2025.12.4start
//                else if(uartTxVector_System_parameter.count() > 0)
//                {
//                    // 1. 取出队列中的第一个包
//                    txTemp_System_parameter = uartTxVector_System_parameter[0];
//                    // 2. 从队列中移除
//                    uartTxVector_System_parameter.remove(0, 1);

//                    // 【关键修改】不要写死 222！而是读取这一帧实际设置的长度。
//                    // 这样既能兼容旧的系统参数（长度222），也能兼容新的条码（长度约100多）
//                    uint8_t realSize = txTemp_System_parameter.uFrame.sFrame.Frame_Length;

//                    // 3. 发送实际长度的数据
//                    // 注意：sendReadCMD 的第二个参数现在是动态的 realSize
//                    sendReadCMD(txTemp_System_parameter.uFrame.alldat, realSize);

//                    // 4. 更新 txTemp 的命令字，以便后续逻辑（如超时判断、进度条等）能知道刚才发了什么
//                    txTemp.uFrame.sFrame.Frame_cmd = txTemp_System_parameter.uFrame.sFrame.Frame_cmd;
//                }
              Buffer_time = 70;
              if(txTemp.uFrame.sFrame.Frame_cmd==0) //如果是读命令
              {
               readTaskStatus = Send_Read_Stage;
               readTick = 0;
               RecFlag = 0;
               RxArray.clear();
//               ui->progressBarCom->setValue(0);
//               ui->label_comstate->clear();
              }
              else if(txTemp.uFrame.sFrame.Frame_cmd==0x0D)//如果是授权
              {
                  readTaskStatus = Send_Read_Stage;
                  readTick = 0;
                  RecFlag = 0;
                  RxArray.clear();
                  ui->textBrowserTestInfo->append("解锁...");
                  ui->btnShow_UL->hide();
              }
             else if(txTemp.uFrame.sFrame.Frame_cmd==0x03)//如果是电压检测
              {
                  readTaskStatus = Send_Read_Stage;
                  readTick = 0;
                  RecFlag = 0;
                  RxArray.clear();
                  ui->textBrowserTestInfo->append("发送电压检测命令！");
//                  ui->label_paraCheck->setText("－－－");
              }
              else if(txTemp.uFrame.sFrame.Frame_cmd==0x02)//如果是ID录入
               {
                   readTaskStatus = Send_Read_Stage;
                   readTick = 0;
                   RecFlag = 0;
                   RxArray.clear();
                   ui->textBrowserTestInfo->append("ID录入...");
//                   ui->textEdit->append("发送ID录入命令！");
//                   ui->label_ID->setText("－－－");
               }
              else if(txTemp.uFrame.sFrame.Frame_cmd==0x31)//如果是SN录入
               {
                   readTaskStatus = Send_Read_Stage;
                   readTick = 0;
                   RecFlag = 0;
                   RxArray.clear();
                   ui->textBrowserTestInfo->append("写产品序列号...");

               }
              else if(txTemp.uFrame.sFrame.Frame_cmd==0x04)//如果是电流校准
              {
                  readTaskStatus = Send_Read_Stage;
                  readTick = 0;
                  RecFlag = 0;
                  RxArray.clear();
                  ui->textBrowserTestInfo->append("电流校准...");
                  ui->btnShow_CurV->hide();
              }
              else if(txTemp.uFrame.sFrame.Frame_cmd==0x01)//如果是零电流校准
              {
                  readTaskStatus = Send_Read_Stage;
                  readTick = 0;
                  RecFlag = 0;
                  RxArray.clear();
                  ui->textBrowserTestInfo->append("零电流校准...");
                  ui->btnShow_ZV->hide();
              }
              else if(txTemp.uFrame.sFrame.Frame_cmd==0x11)
              {
               ui->textBrowserTestInfo->append("时间校准...");
               if(BMSConnectIsOk)
               {
                   ui->textBrowserTestInfo->append("成功！\n");
                   ui->btnShow_TV->show();
                   ui->btnShow_TV->setText("OK");
                   ui->btnShow_TV->setStyleSheet("color:#ffffff;font-size:10px;background:green");
                  // ui->label_timeverify->setPixmap(QPixmap(":/myimage/qrc/image/msg_info.png").scaled(25,25));
               }
               else
               {
                   ui->textBrowserTestInfo->append("失败！保护板未连接！\n");
                   ui->btnShow_TV->show();
                   ui->btnShow_TV->setText("NG");
                   ui->btnShow_TV->setStyleSheet("color:#ffffff;font-size:10px;background:red");
                  // ui->label_timeverify->setPixmap(QPixmap(":/myimage/qrc/image/msg_error1.png").scaled(25,25));
                   TestProcessError = 1;
               }
               timecali_end = 1;
              }
              else if(txTemp.uFrame.sFrame.Frame_cmd==0x05)
              {
//                 ui->textBrowserTestInfo->append("保护板复位...\n");
//                 ui->textBrowserTestInfo->append("保护板复位...resetok=1;\n");
                  ui->textBrowserTestInfo->append("保护板复位...\n");
                 // 【修改】确保复位也进入等待状态
//                  readTaskStatus = Send_Read_Stage;
//                  readTick = 0;
//                  RecFlag = 0;
//                  RxArray.clear();
                 resetok=1;
              }
              else if(txTemp.uFrame.sFrame.Frame_cmd==0x10)
              {
                 ui->textBrowserTestInfo->append("保护板恢复...\n");
              }
              else if(txTemp.uFrame.sFrame.Frame_cmd==0x06) //刷新系统参数
              {
                  readTaskStatus = Send_Read_Stage;
                  readTick = 0;
                  RecFlag = 0;
                  RxArray.clear();
                  Buffer_time = 350;

              }
              else if(txTemp.uFrame.sFrame.Frame_cmd==0x30)//如果是读序列号
              {
                  readTaskStatus = Send_Read_Stage;
                  readTick = 0;
                  RecFlag = 0;
                  RxArray.clear();
               }
              else if(txTemp.uFrame.sFrame.Frame_cmd==0x09)//如果是开机命令
               {
                   readTaskStatus = Send_Read_Stage;
                   readTick = 0;
                   RecFlag = 0;
                   RxArray.clear();
                   Buffer_time = 300;
                   ui->textBrowserTestInfo->append("发送开机命令!");
 //                  ui->label_paraCheck->setText("－－－");
               }
              else if(txTemp.uFrame.sFrame.Frame_cmd==0x0A)//如果是关机命令
               {
                   readTaskStatus = Send_Read_Stage;
                   readTick = 0;
                   RecFlag = 0;
                   RxArray.clear();
                   Buffer_time = 300;
                   ui->textBrowserTestInfo->append("发送关机命令!");
 //                  ui->label_paraCheck->setText("－－－");
               }
              else if(txTemp.uFrame.sFrame.Frame_cmd==0x20)//如果是开启仓储模式命令
               {
                   readTaskStatus = Send_Read_Stage;
                   readTick = 0;
                   RecFlag = 0;
                   RxArray.clear();
                   Buffer_time = 300;
                   ui->textBrowserTestInfo->append("发送开启仓储模式命令!");
 //                  ui->label_paraCheck->setText("－－－");
               }
              else if(txTemp.uFrame.sFrame.Frame_cmd==0x36)//如果是哑银码、内控码、BMS编码查询
               {
                   readTaskStatus = Send_Read_Stage;
                   readTick = 0;
                   RecFlag = 0;
                   RxArray.clear();
                   Buffer_time = 300;
//                   ui->textBrowserTestInfo->append("发送哑银码、内控码、BMS编码查询命令!");
//                  ui->label_paraCheck->setText("－－－");
               }
              else if(txTemp.uFrame.sFrame.Frame_cmd==0x37)//如果是哑银码、内控码、BMS编码录入
               {
                   readTaskStatus = Send_Read_Stage;
                   readTick = 0;
                   RecFlag = 0;
                   RxArray.clear();
//                   ui->textBrowserTestInfo->append("发送哑银码、内控码、BMS编码录入命令！");
//                   ui->labelCodeInput->setText("－－－");
               }
              else if(txTemp.uFrame.sFrame.Frame_cmd==0x15)//如果是开启加热模式命令
               {
                   readTaskStatus = Send_Read_Stage;
                   readTick = 0;
                   RecFlag = 0;
                   RxArray.clear();
                   Buffer_time = 300;
                   ui->textBrowserTestInfo->append("发送开启加热命令!");
 //                  ui->label_paraCheck->setText("－－－");
               }
              else if(txTemp.uFrame.sFrame.Frame_cmd==0x16)//如果是关闭加热模式命令
               {
                   readTaskStatus = Send_Read_Stage;
                   readTick = 0;
                   RecFlag = 0;
                   RxArray.clear();
                   Buffer_time = 300;
                   ui->textBrowserTestInfo->append("发送关闭加热命令!");
 //                  ui->label_paraCheck->setText("－－－");
               }


            }

            break;
            case Send_Read_Stage:
            if(readTick>=1000)//超时处理，现在是2秒
            {
                //清除显示内容

                if(txTemp.uFrame.sFrame.Frame_cmd==0x06||txTemp.uFrame.sFrame.Frame_cmd==0)ClearBaseInfo();

                if(txTemp.uFrame.sFrame.Frame_cmd==0x0D)
                {
                   ui->textBrowserTestInfo->append("失败！\n");  //解锁失败
                   ui->btnShow_UL->show();
                   ui->btnShow_UL->setText("NG");
                   ui->btnShow_UL->setStyleSheet("color:#ffffff;font-size:10px;background:red");
//                   ui->label_unlock->setPixmap(QPixmap(":/myimage/qrc/image/msg_error1.png").scaled(25,25));
                   TestProcessError = 1;
                   unlock_end = 1;
                }
                else if(txTemp.uFrame.sFrame.Frame_cmd==0x01)
                {
                  ui->textBrowserTestInfo->append("失败！\n"); //零电流校准失败
                  ui->btnShow_ZV->show();
                  ui->btnShow_ZV->setText("NG");
                  ui->btnShow_ZV->setStyleSheet("color:#ffffff;font-size:10px;background:red");
//                  ui->label_zeroverify->setPixmap(QPixmap(":/myimage/qrc/image/msg_error1.png").scaled(25,25));
                  TestProcessError = 1;
                  zerocurcali_end = 1;
                }
                else if(txTemp.uFrame.sFrame.Frame_cmd==0x04)
                {
                  ui->textBrowserTestInfo->append("失败！\n"); //电流校准
                  ui->btnShow_CurV->show();
                  ui->btnShow_CurV->setText("NG");
                  ui->btnShow_CurV->setStyleSheet("color:#ffffff;font-size:10px;background:red");
//                  ui->label_curverify->setPixmap(QPixmap(":/myimage/qrc/image/msg_error1.png").scaled(25,25));
                  TestProcessError = 1;
                  Curcali_end = 1;
                }
                RxArray.clear();
                readTaskStatus = Idle_Stage_rT;
                readTick = 0;
//                BMSConnectIsOk = 0;
//                ui->label_comstate->setPixmap(QPixmap(":/myimage/qrc/image/red.png").scaled(20,20));
                // 👇 【核心修改】：只有在非测试状态（空闲时），才允许判定断线亮红灯！
//                if (TestProcessTaskState == 0)
//                {
//                    BMSConnectIsOk = 0;
//                }
//                else
//                {
//                    // 如果正在自动测试中发生超时，我们给予“豁免”，不切断通讯
//                    // 可以在后台悄悄打印一句日志（可选）
//                    // ui->textBrowserTestInfo->append("提示：测试步骤响应较慢，通讯保持...");
//                }
                BMSConnectIsOk = 0;
            }
            else if(RecFlag==1)
            {

              //分析数据
                uint8_t temp = RxArray[0];
                uint8_t len = RxArray[1];
                uint8_t cmd = RxArray[3];
                uint8_t check = RxArray[len-2];
                if((temp==0x68)&&(check==SocCalcXor(RxArray,len-2)))
                {
                //zgj 2025.10.15 Start
                BMSConnectIsOk = 1;
                //zgj 2025.10.15 End
                 if(cmd==0) //read base information
                 {
                     BaseInfoParser(RxArray);
                     BaseInforReadIsOk++;
                     RxArray.clear();
                     readTaskStatus = Accept_Parse_Stage;
                     readTick = 0;
//                     ui->progressBarCom->setValue(100);
                     ui->label_comstate->setPixmap(QPixmap(":/myimage/qrc/image/green.png").scaled(20,20));
                 }
                 else if(cmd==0x06)//刷新系统参数
                 {
                  BMSConnectIsOk = 1;

                  SystemInfoParser(RxArray);
                     parameterReadIsOk++;
                     RxArray.clear();
                     readTaskStatus = Accept_Parse_Stage;
                     readTick = 0;

                 }
                 else if(cmd==0x0d) //auth cmd
                 {
                     uint32_t UID[3];
                     uint32_t AuthCode;
                     uint8_t buf[12];
                    //接收UID

                     UID[0] = (uint8_t)RxArray[7];
                     UID[0] <<= 8;
                     UID[0] |= (uint8_t)RxArray[6];
                     UID[0] <<= 8;
                     UID[0] |= (uint8_t)RxArray[5];
                     UID[0] <<= 8;
                     UID[0] |= (uint8_t)RxArray[4];

                     UID[1] = (uint8_t)RxArray[11];
                     UID[1] <<= 8;
                     UID[1] |= (uint8_t)RxArray[10];
                     UID[1] <<= 8;
                     UID[1] |= (uint8_t)RxArray[9];
                     UID[1] <<= 8;
                     UID[1] |= (uint8_t)RxArray[8];

                     UID[2] = (uint8_t)RxArray[15];
                     UID[2] <<= 8;
                     UID[2] |= (uint8_t)RxArray[14];
                     UID[2] <<= 8;
                     UID[2] |= (uint8_t)RxArray[13];
                     UID[2] <<= 8;
                     UID[2] |= (uint8_t)RxArray[12];
                     //计算并返回Auth code
                     AuthCode = (UID[0]+UID[1]+UID[2])/87;
                     buf[0] = 0x68;
                     buf[1] = 0x0c;
                     buf[2] = 0xff-0x0c;
                     buf[3] = 0x0e;
                     buf[4] = AuthCode;
                     buf[5] = AuthCode>>8;
                     buf[6] = AuthCode>>16;
                     buf[7] = AuthCode>>24;
                     buf[8] = 0;
                     buf[9] = 0;
                     buf[10] = SocCalcXor(buf,buf[1]-2);
                     buf[11] = 0x16;
                     sendReadCMD(buf,12);
                     RxArray.clear();
                     readTick = 0;
                     RecFlag = 0;
                     readTaskStatus = Send_Read_Stage;
//                     ui->textBrowserTestInfo->append("密码值OK！");
                     return;
                 }
                 else if(cmd==0x0e)
                 {
                   ui->textBrowserTestInfo->append("成功！\n");  //解锁成功
                   ui->btnShow_UL->show();
                   ui->btnShow_UL->setText("OK");
                   ui->btnShow_UL->setStyleSheet("color:#ffffff;font-size:10px;background:green");
//                   ui->label_unlock->setPixmap(QPixmap(":/myimage/qrc/image/msg_info.png").scaled(25,25));  //.scaled(243,67)
                   unlock_end = 1;
                   RxArray.clear();
                   readTaskStatus = Idle_Stage_rT;
                   readTick = 0;
                 }
                 else if(cmd==0x02)
                 {
                   ui->textBrowserTestInfo->append("ID录入成功！\n");
                   RxArray.clear();
                   readTaskStatus = Idle_Stage_rT;
                   readTick = 0;

                 }
                 else if(cmd==0x31)
                 {
                   ui->textBrowserTestInfo->append("序列号写入成功！\n");
                   RxArray.clear();
                   readTaskStatus = Idle_Stage_rT;
                   readTick = 0;

                 }
                 else if(cmd==0x37)// 写入编码的响应帧
                  {
                      uint8_t result = RxArray[4]; // 假设第5个字节是结果码 (1=成功, 0=失败)
                      if(result == 1){
                          QMessageBox::information(this, "成功", "编码录入成功！");
                          ui->textBrowserTestInfo->append("亚银码录入成功！");
//                          ui->labelCodeInput->setText("成功");
                          // 录入成功后，立即再次读取以确认
    //                         CodesReadEnable = 1;
                      } else {
                          QMessageBox::critical(this, "失败", "编码录入失败！");
                          ui->textBrowserTestInfo->append("亚银码录入失败！");
                      }
                      RxArray.clear();
                      readTaskStatus = Idle_Stage_rT;
                      readTick = 0;
                  }
                 else if(cmd==0x01)
                 {
                   uint8_t buf[12];
                   buf[0] = 0x68;
                   buf[1] = 0x0c;
                   buf[2] = 0xff-0x0c;
                   buf[3] = 0x08;
                   buf[4] = 0;
                   buf[5] = 0;
                   buf[6] = 0;
                   buf[7] = 0;
                   buf[8] = 0;
                   buf[9] = 0;
                   buf[10] = SocCalcXor(buf,buf[1]-2);
                   buf[11] = 0x16;
                   sendReadCMD(buf,12);
                   ui->textBrowserTestInfo->append("成功！\n");//零电流校准成功
                   ui->btnShow_ZV->show();
                   ui->btnShow_ZV->setText("OK");
                   ui->btnShow_ZV->setStyleSheet("color:#ffffff;font-size:10px;background:green");
//                   ui->label_zeroverify->setPixmap(QPixmap(":/myimage/qrc/image/msg_info.png").scaled(25,25));
                   zerocurcali_end = 1;
                   RxArray.clear();
                   readTaskStatus = Idle_Stage_rT;
                   readTick = 0;
                 }
                 else if(cmd==0x03)
                 {
                   ui->textBrowserTestInfo->append("电压检测成功！\n");
                   RxArray.clear();
                   readTaskStatus = Idle_Stage_rT;
                   readTick = 0;

                 }
                 else if(cmd==0x04)
                 {
                qDebug() << "成功收到电流校准(0x04)的回复！准备设置 Curcali_end = 1"; // <-- 新增
                   ui->textBrowserTestInfo->append("成功！\n"); //电流校准成功！
                   ui->btnShow_CurV->setText("OK");
                   ui->btnShow_CurV->setStyleSheet("color:#ffffff;font-size:10px;background:green");
                   ui->btnShow_CurV->show();
//                   ui->label_curverify->setPixmap(QPixmap(":/myimage/qrc/image/msg_info.png").scaled(25,25));
                   Curcali_end = 1;
                   RxArray.clear();
                   readTaskStatus = Idle_Stage_rT;
                   readTick = 0;
                 }
                 else if(cmd==0x30)
                 {
                     QByteArray SN;
                     QByteArray MODEL;
                     uint8_t le = (uint8_t)RxArray[1];
                     le -= 6;
                     if(le>66)return;
                    //接收SN
                     for (int i=0;i<32;i++) {
                      SN[i] = (uint8_t)RxArray[4+i+2];
                      MODEL[i] = (uint8_t)RxArray[4+i+32+2];
                     }
                     QString snStr=QString::fromLatin1(SN);
                     QString modelStr=QString::fromLatin1(MODEL);

                     QTableWidgetItem *item_ = new QTableWidgetItem;
                     item_->setText(snStr);
                     item_->setTextColor(Qt::blue);
                     item_->setFont(QFont("Microsoft Yahei", 8));
                     item_->setTextAlignment(Qt::AlignCenter);
                     ui->tableWidgetSystemPara->setItem(14,1,item_);
//                     ui->textEdit->append("SerialNumber:");
//                     ui->textEdit->append(snStr);
//                     ui->textEdit->append("Model：");
//                     ui->textEdit->append(modelStr);
                     //str = QString::number(temp32);
//                     ui->tableWidgetSystemPara->setItem(14,1,new QTableWidgetItem(snStr));//
//                     ui->tableWidgetSystemPara->setItem(15,1,new QTableWidgetItem(modelStr));//
                     RxArray.clear();
                     readTaskStatus = Idle_Stage_rT;
                     readTick = 0;
                 }
                 else
                 {
                     // 收到能过校验，但是未处理的命令（如 0x55 ACK 等）
                     RxArray.clear();
                     readTaskStatus = Idle_Stage_rT;
                     readTick = 0;
                 }
                }
                else
                {
              RxArray.clear();
              readTaskStatus = Idle_Stage_rT;
              readTick = 0;
                }
            }
            break;
            case Accept_Parse_Stage:
        //zgj 2025.12.24 Start 降低延时
//            if(readTick>=500)
            if(readTick>=25)
        //zgj 2025.12.24 End
            {
                readTick = 0;
               // qDebug() << "Idle_Stage_rT3";
                readTaskStatus = Idle_Stage_rT;
//                 ui->progressBarCom->setValue(0);
            }
            break;
         }

}


void BMCMainWindow::CheckIntoQueue()
{
    txStruct  txTemp;

    if(StartPushtoQueue==0) return;

    if(ReadBaseInfoEnable)
    {
     ReadBaseInfoEnable = 0;
     txTemp.uFrame.sFrame.Frame_Head = 0x68;
     txTemp.uFrame.sFrame.Frame_cmd = 0;   //read command
     txTemp.uFrame.sFrame.Frame_Length = 0x0c;
     txTemp.uFrame.sFrame.Frame_Length_Check = 0xFF-txTemp.uFrame.sFrame.Frame_Length;
     for (uint8_t i=0;i<6;i++) {
      txTemp.uFrame.sFrame.Dat_Playload[i]=0;
     }
     txTemp.uFrame.sFrame.Check = SocCalcXor(txTemp.uFrame.alldat,txTemp.uFrame.sFrame.Frame_Length-2);
     txTemp.uFrame.sFrame.Frame_tail = 0x16;

     uartTxVector.append(txTemp);
    }

    if(SendResumeEnable)
    {
     SendResumeEnable = 0;
     txTemp.uFrame.sFrame.Frame_Head = 0x68;
     txTemp.uFrame.sFrame.Frame_cmd = 0x10;  //resume command
     txTemp.uFrame.sFrame.Frame_Length = 0x0c;
     txTemp.uFrame.sFrame.Frame_Length_Check = 0xFF-txTemp.uFrame.sFrame.Frame_Length;
     for (uint8_t i=0;i<6;i++) {
      txTemp.uFrame.sFrame.Dat_Playload[i]=0;
     }
     txTemp.uFrame.sFrame.Check = SocCalcXor(txTemp.uFrame.alldat,txTemp.uFrame.sFrame.Frame_Length-2);
     txTemp.uFrame.sFrame.Frame_tail = 0x16;

     uartTxVector.append(txTemp);
    }

    if(SendResetEnable)
    {
     SendResetEnable = 0;
     txTemp.uFrame.sFrame.Frame_Head = 0x68;
     txTemp.uFrame.sFrame.Frame_cmd = 0x05;  //reset command
     txTemp.uFrame.sFrame.Frame_Length = 0x0c;
     txTemp.uFrame.sFrame.Frame_Length_Check = 0xFF-txTemp.uFrame.sFrame.Frame_Length;
     for (uint8_t i=0;i<6;i++) {
      txTemp.uFrame.sFrame.Dat_Playload[i]=0;
     }
     txTemp.uFrame.sFrame.Check = SocCalcXor(txTemp.uFrame.alldat,txTemp.uFrame.sFrame.Frame_Length-2);
     txTemp.uFrame.sFrame.Frame_tail = 0x16;

     uartTxVector.append(txTemp);
    }

    if(SendPowerUpEnable)
    {
     SendPowerUpEnable = 0;
     txTemp.uFrame.sFrame.Frame_Head = 0x68;
     txTemp.uFrame.sFrame.Frame_cmd = 0x09;  //开机 command
     txTemp.uFrame.sFrame.Frame_Length = 0x0c;
     txTemp.uFrame.sFrame.Frame_Length_Check = 0xFF-txTemp.uFrame.sFrame.Frame_Length;
     for (uint8_t i=0;i<6;i++) {
      txTemp.uFrame.sFrame.Dat_Playload[i]=0;
     }
     txTemp.uFrame.sFrame.Check = SocCalcXor(txTemp.uFrame.alldat,txTemp.uFrame.sFrame.Frame_Length-2);
     txTemp.uFrame.sFrame.Frame_tail = 0x16;

     uartTxVector.append(txTemp);
    }
    if(SendShutDownEnable)
    {
     SendShutDownEnable = 0;
     txTemp.uFrame.sFrame.Frame_Head = 0x68;
     txTemp.uFrame.sFrame.Frame_cmd = 0x0A;  //关机 command
     txTemp.uFrame.sFrame.Frame_Length = 0x0c;
     txTemp.uFrame.sFrame.Frame_Length_Check = 0xFF-txTemp.uFrame.sFrame.Frame_Length;
     for (uint8_t i=0;i<6;i++) {
      txTemp.uFrame.sFrame.Dat_Playload[i]=0;
     }
     txTemp.uFrame.sFrame.Check = SocCalcXor(txTemp.uFrame.alldat,txTemp.uFrame.sFrame.Frame_Length-2);
     txTemp.uFrame.sFrame.Frame_tail = 0x16;

     uartTxVector.append(txTemp);
    }
    if(OpenHeaterEnable)
    {
     OpenHeaterEnable = 0;
     txTemp.uFrame.sFrame.Frame_Head = 0x68;
     txTemp.uFrame.sFrame.Frame_cmd = 0x15;  //开加热 command
     txTemp.uFrame.sFrame.Frame_Length = 0x0c;
     txTemp.uFrame.sFrame.Frame_Length_Check = 0xFF-txTemp.uFrame.sFrame.Frame_Length;
     for (uint8_t i=0;i<6;i++) {
      txTemp.uFrame.sFrame.Dat_Playload[i]=0;
     }
     txTemp.uFrame.sFrame.Check = SocCalcXor(txTemp.uFrame.alldat,txTemp.uFrame.sFrame.Frame_Length-2);
     txTemp.uFrame.sFrame.Frame_tail = 0x16;

     uartTxVector.append(txTemp);
    }
    if(CloseHeaterEnable)
    {
     CloseHeaterEnable = 0;
     txTemp.uFrame.sFrame.Frame_Head = 0x68;
     txTemp.uFrame.sFrame.Frame_cmd = 0x16;  //关加热 command
     txTemp.uFrame.sFrame.Frame_Length = 0x0c;
     txTemp.uFrame.sFrame.Frame_Length_Check = 0xFF-txTemp.uFrame.sFrame.Frame_Length;
     for (uint8_t i=0;i<6;i++) {
      txTemp.uFrame.sFrame.Dat_Playload[i]=0;
     }
     txTemp.uFrame.sFrame.Check = SocCalcXor(txTemp.uFrame.alldat,txTemp.uFrame.sFrame.Frame_Length-2);
     txTemp.uFrame.sFrame.Frame_tail = 0x16;

     uartTxVector.append(txTemp);
    }
    if(StartStorageEnable)
    {
     StartStorageEnable = 0;
     txTemp.uFrame.sFrame.Frame_Head = 0x68;
     txTemp.uFrame.sFrame.Frame_cmd = 0x20;  //开仓储 command
     txTemp.uFrame.sFrame.Frame_Length = 0x0c;
     txTemp.uFrame.sFrame.Frame_Length_Check = 0xFF-txTemp.uFrame.sFrame.Frame_Length;
     for (uint8_t i=0;i<6;i++) {
      txTemp.uFrame.sFrame.Dat_Playload[i]=0;
     }
     txTemp.uFrame.sFrame.Check = SocCalcXor(txTemp.uFrame.alldat,txTemp.uFrame.sFrame.Frame_Length-2);
     txTemp.uFrame.sFrame.Frame_tail = 0x16;

     uartTxVector.append(txTemp);
    }
    if(AuthenticationEnable)
    {
     AuthenticationEnable = 0;
     txTemp.uFrame.sFrame.Frame_Head = 0x68;
     txTemp.uFrame.sFrame.Frame_cmd = 0x0D;  //授权 command
     txTemp.uFrame.sFrame.Frame_Length = 0x0c;
     txTemp.uFrame.sFrame.Frame_Length_Check = 0xFF-txTemp.uFrame.sFrame.Frame_Length;
     for (uint8_t i=0;i<6;i++) {
      txTemp.uFrame.sFrame.Dat_Playload[i]=0;
     }
     txTemp.uFrame.sFrame.Check = SocCalcXor(txTemp.uFrame.alldat,txTemp.uFrame.sFrame.Frame_Length-2);
     txTemp.uFrame.sFrame.Frame_tail = 0x16;

     uartTxVector.append(txTemp);
    }

    if(ZeroCurCaliEnable)
    {
     ZeroCurCaliEnable = 0;
     txTemp.uFrame.sFrame.Frame_Head = 0x68;
     txTemp.uFrame.sFrame.Frame_cmd = 0x01;  //零电流 command
     txTemp.uFrame.sFrame.Frame_Length = 0x0c;
     txTemp.uFrame.sFrame.Frame_Length_Check = 0xFF-txTemp.uFrame.sFrame.Frame_Length;
     for (uint8_t i=0;i<6;i++) {
      txTemp.uFrame.sFrame.Dat_Playload[i]=0;
     }
     txTemp.uFrame.sFrame.Check = SocCalcXor(txTemp.uFrame.alldat,txTemp.uFrame.sFrame.Frame_Length-2);
     txTemp.uFrame.sFrame.Frame_tail = 0x16;

     uartTxVector.append(txTemp);
    }

    if(VoltCheckEnable)
    {
     VoltCheckEnable = 0;
     txTemp.uFrame.sFrame.Frame_Head = 0x68;
     txTemp.uFrame.sFrame.Frame_cmd = 0x03;  //电压检测 command
     txTemp.uFrame.sFrame.Frame_Length = 0x0c;
     txTemp.uFrame.sFrame.Frame_Length_Check = 0xFF-txTemp.uFrame.sFrame.Frame_Length;
     for (uint8_t i=0;i<6;i++) {
      txTemp.uFrame.sFrame.Dat_Playload[i]=0;
     }
     txTemp.uFrame.sFrame.Check = SocCalcXor(txTemp.uFrame.alldat,txTemp.uFrame.sFrame.Frame_Length-2);
     txTemp.uFrame.sFrame.Frame_tail = 0x16;

     uartTxVector.append(txTemp);
    }

    if(CurrentCaliEnable)
    {
     CurrentCaliEnable = 0;
     int cur = -2000;//Bms_cfg_infor.CurVarify;//ui->lineEditCurCali1->text().toInt();
     txTemp.uFrame.sFrame.Frame_Head = 0x68;
     txTemp.uFrame.sFrame.Frame_cmd = 0x04;  //电流校准 command
     txTemp.uFrame.sFrame.Frame_Length = 0x0c;
     txTemp.uFrame.sFrame.Frame_Length_Check = 0xFF-txTemp.uFrame.sFrame.Frame_Length;
//    for (uint8_t i=0;i<6;i++) {

     txTemp.uFrame.sFrame.Dat_Playload[0]=cur;
     txTemp.uFrame.sFrame.Dat_Playload[1]=cur>>8;
     txTemp.uFrame.sFrame.Dat_Playload[2]=cur>>16;
     txTemp.uFrame.sFrame.Dat_Playload[3]=cur>>24;
     txTemp.uFrame.sFrame.Dat_Playload[4]=0;
     txTemp.uFrame.sFrame.Dat_Playload[5]=0;
//     }
     txTemp.uFrame.sFrame.Check = SocCalcXor(txTemp.uFrame.alldat,txTemp.uFrame.sFrame.Frame_Length-2);
     txTemp.uFrame.sFrame.Frame_tail = 0x16;

     uartTxVector.append(txTemp);
    }
    if(IDWriteEnable)
      {
       IDWriteEnable = 0;
       txTemp.uFrame.sFrame.Frame_Head = 0x68;
       txTemp.uFrame.sFrame.Frame_cmd = 0x02;  //id 记录 command
       txTemp.uFrame.sFrame.Frame_Length = 0x0e;
       txTemp.uFrame.sFrame.Frame_Length_Check = 0xFF-txTemp.uFrame.sFrame.Frame_Length;
       int id = ui->lineEditID->text().toInt();
  //     for (uint8_t i=0;i<8;i++) {
  //      txTemp.uFrame.sFrame.Dat_Playload[i]=0;
  //     }
       txTemp.uFrame.sFrame.Dat_Playload[0]=id;
       txTemp.uFrame.sFrame.Dat_Playload[1]=id>>8;
       txTemp.uFrame.sFrame.Dat_Playload[2]=id>>16;
       txTemp.uFrame.sFrame.Dat_Playload[3]=id>>24;
       id = ui->lineEditCANID->text().toInt();
       txTemp.uFrame.sFrame.Dat_Playload[4]=id;
       txTemp.uFrame.sFrame.Dat_Playload[5]=id>>8;
       txTemp.uFrame.sFrame.Dat_Playload[6]=id>>16;
       txTemp.uFrame.sFrame.Dat_Playload[7]=id>>24;
       txTemp.uFrame.sFrame.Check = SocCalcXor(txTemp.uFrame.alldat,txTemp.uFrame.sFrame.Frame_Length-2);
       txTemp.uFrame.sFrame.Frame_tail = 0x16;

       uartTxVector.append(txTemp);
      }
    if(EsendSNWrite)
    {
     EsendSNWrite = 0;
     txTemp.uFrame.sFrame.Frame_Head = 0x68;
     txTemp.uFrame.sFrame.Frame_cmd = 0x31;  //SN 记录 command
     txTemp.uFrame.sFrame.Frame_Length = 72;
     txTemp.uFrame.sFrame.Frame_Length_Check = 0xFF-txTemp.uFrame.sFrame.Frame_Length;

//     int id = ui->lineEditID->text().toInt();
     for (uint8_t i=0;i<66;i++) {
      txTemp.uFrame.sFrame.Dat_Playload[i]=0;
     }

     QString buf = ui->lineEditBarCode13->text();
     QString buf1 = ui->lineEditBatModel->text();

     QByteArray a = buf.toLocal8Bit().data();
     int lx = a.length();
     if(lx>32)return;

     for (int i=0;i<lx;i++) {
      txTemp.uFrame.sFrame.Dat_Playload[i+2] = a[i];
     }
     a = buf1.toLocal8Bit().data();
     lx = a.length();
     if(lx>32)return;
     for (int i=0;i<lx;i++) {
       txTemp.uFrame.sFrame.Dat_Playload[34+i] = a[i];
      }
     txTemp.uFrame.sFrame.Check = SocCalcXor(txTemp.uFrame.alldat,txTemp.uFrame.sFrame.Frame_Length-2);
     txTemp.uFrame.sFrame.Frame_tail = 0x16;

     uartTxVector.append(txTemp);
    }
    //zgj 2025.12.4start
    if(CodesWriteEnable)
    {
        CodesWriteEnable = 0;

        // 【关键修改】使用大结构体 txStruct_System_parameter
        txStruct_System_parameter txBigTemp;

        // 1. 初始化帧头和命令
        txBigTemp.uFrame.sFrame.Frame_Head = 0x68;
        txBigTemp.uFrame.sFrame.Frame_cmd = 0x37; // 编码录入命令

        // 2. 清空 Payload 区域 (使用大结构体的大小)
        memset(txBigTemp.uFrame.sFrame.Dat_Playload, 0, sizeof(txBigTemp.uFrame.sFrame.Dat_Playload));

        // 定义一个偏移量
        int currentOffset = 0;

        // --- Lambda 函数 (保持不变) ---
        auto packCode = [&](QLineEdit* edit) {
            QString textStr = edit->text().trimmed();
            QByteArray bytes = textStr.toLocal8Bit();
            uint8_t len = bytes.length();

            // 标志
            bool isValid = !textStr.isEmpty();
            txBigTemp.uFrame.sFrame.Dat_Playload[currentOffset++] = isValid ? 0x01 : 0x00;

            // 长度
            txBigTemp.uFrame.sFrame.Dat_Playload[currentOffset++] = len;

            // 内容
            if (len > 0) {
                // 安全检查：Dat_Playload 最大 216 字节
                int maxWrite = 216 - 1 - currentOffset;
                int copyLen = qMin((int)len, maxWrite);

                if (copyLen > 0) {
                    memcpy(&txBigTemp.uFrame.sFrame.Dat_Playload[currentOffset], bytes.constData(), copyLen);
                    currentOffset += copyLen;
                }
            }

            // 分号
            txBigTemp.uFrame.sFrame.Dat_Playload[currentOffset++] = 0x3B;
        };


        // --- 依次打包 ---
        packCode(ui->lineEditMattSilverCode);
        packCode(ui->lineEditInternalControlCode);
        packCode(ui->lineEditBMSCode);

        // --- 计算长度 ---
        txBigTemp.uFrame.sFrame.Frame_Length = currentOffset + 6;
        txBigTemp.uFrame.sFrame.Frame_Length_Check = 0xFF - txBigTemp.uFrame.sFrame.Frame_Length;

        // 【修正 BUG 开始】
        // 不能使用 txBigTemp.uFrame.sFrame.Check = ... 因为那会写到第 220 字节去！
        // 我们需要手动计算它们在 alldat 数组中的正确位置。

        // 1. 计算校验和 (范围：从帧头开始，到 Payload 结束)
        uint8_t calcCheck = SocCalcXor(txBigTemp.uFrame.alldat, txBigTemp.uFrame.sFrame.Frame_Length - 2);

        // 2. 手动写入校验和 (位置：倒数第2个字节)
        txBigTemp.uFrame.alldat[txBigTemp.uFrame.sFrame.Frame_Length - 2] = calcCheck;

        // 3. 手动写入帧尾 (位置：倒数第1个字节)
        txBigTemp.uFrame.alldat[txBigTemp.uFrame.sFrame.Frame_Length - 1] = 0x16;
        // 【修正 BUG 结束】

        // --- 加入大容量队列 ---
        uartTxVector_System_parameter.append(txBigTemp);

         ui->textBrowserTestInfo->append("发送亚银码录入命令！如无回复则可能失败");

         // 【新增调试信息】
         QString debugInfo = QString("发送亚银码录入命令！帧长：%1，校验和：0x%2，队列大小：%3")
                                .arg(txBigTemp.uFrame.sFrame.Frame_Length)
                                .arg(calcCheck, 2, 16, QLatin1Char('0'))
                                .arg(uartTxVector_System_parameter.size());
//         ui->textBrowserTestInfo->append(debugInfo);

         // 【新增：显示完整的帧内容】
//         QString frameHex;
//         for(int i = 0; i < txBigTemp.uFrame.sFrame.Frame_Length; i++) {
//             frameHex += QString("%1 ").arg((uint8_t)txBigTemp.uFrame.alldat[i], 2, 16, QLatin1Char('0'));
//         }
//         ui->textBrowserTestInfo->append("完整帧内容：" + frameHex.toUpper());

//         // 【新增：显示各编码内容】
//         ui->textBrowserTestInfo->append("亚银码：" + ui->lineEditMattSilverCode->text());
//         ui->textBrowserTestInfo->append("内控码：" + ui->lineEditInternalControlCode->text());
//         ui->textBrowserTestInfo->append("BMS编码：" + ui->lineEditBMSCode->text());

         // 【新增：立即发送队列中的第一条数据】
//         if(!uartTxVector_System_parameter.isEmpty()) {
//             // 这里需要调用实际的发送函数，假设是 SendSystemParameterData()
//             // SendSystemParameterData(uartTxVector_System_parameter.first());
//             // uartTxVector_System_parameter.removeFirst();
//             ui->textBrowserTestInfo->append("已尝试立即发送亚银码录入命令");
//         }
    }

    if(RTCCaliEnable)
    {
     RTCCaliEnable = 0;
//     int32_t cur = ui->lineEditCurCali1->text().toInt();
     QDateTime current_date_time = QDateTime::currentDateTime();
     QString str_current_date =current_date_time.toString("yyyy.MM.dd hh:mm:ss.zzz ddd");
     //current_date字符串结果为"2016.05.20 12:17:01.445 周五"
     int year = str_current_date.mid(0,4).toInt();
     int month = str_current_date.mid(5,2).toInt();
     int day = str_current_date.mid(8,2).toInt();
     int hour = str_current_date.mid(11,2).toInt();
     int min = str_current_date.mid(14,2).toInt();
     int sec = str_current_date.mid(17,2).toInt();
     txTemp.uFrame.sFrame.Frame_Head = 0x68;
     txTemp.uFrame.sFrame.Frame_cmd = 0x11;  //时间校准 command
     txTemp.uFrame.sFrame.Frame_Length = 0x0c;
     txTemp.uFrame.sFrame.Frame_Length_Check = 0xFF-txTemp.uFrame.sFrame.Frame_Length;
//    for (uint8_t i=0;i<6;i++) {

     txTemp.uFrame.sFrame.Dat_Playload[0]=sec;
     txTemp.uFrame.sFrame.Dat_Playload[1]=min;
     txTemp.uFrame.sFrame.Dat_Playload[2]=hour;
     txTemp.uFrame.sFrame.Dat_Playload[3]=day;
     txTemp.uFrame.sFrame.Dat_Playload[4]=month;
     txTemp.uFrame.sFrame.Dat_Playload[5]=year-2018;
//     }
     txTemp.uFrame.sFrame.Check = SocCalcXor(txTemp.uFrame.alldat,txTemp.uFrame.sFrame.Frame_Length-2);
     txTemp.uFrame.sFrame.Frame_tail = 0x16;

     uartTxVector.append(txTemp);
    }

    if(ReadSysPraEnable)
    {
     ReadSysPraEnable = 0;
     txTemp.uFrame.sFrame.Frame_Head = 0x68;
     txTemp.uFrame.sFrame.Frame_cmd = 0x06;  //读系统参数 command
     txTemp.uFrame.sFrame.Frame_Length = 0x0c;
     txTemp.uFrame.sFrame.Frame_Length_Check = 0xFF-txTemp.uFrame.sFrame.Frame_Length;
     for (uint8_t i=0;i<6;i++) {
      txTemp.uFrame.sFrame.Dat_Playload[i]=0;
     }
     txTemp.uFrame.sFrame.Check = SocCalcXor(txTemp.uFrame.alldat,txTemp.uFrame.sFrame.Frame_Length-2);
     txTemp.uFrame.sFrame.Frame_tail = 0x16;

     uartTxVector.append(txTemp);
    }
    if(EFUSEEnable)
    {
     EFUSEEnable = 0;
     txTemp.uFrame.sFrame.Frame_Head = 0x68;
     txTemp.uFrame.sFrame.Frame_cmd = 0x12;  //EFUSE command
     txTemp.uFrame.sFrame.Frame_Length = 0x0c;
     txTemp.uFrame.sFrame.Frame_Length_Check = 0xFF-txTemp.uFrame.sFrame.Frame_Length;
     for (uint8_t i=0;i<6;i++) {
      txTemp.uFrame.sFrame.Dat_Playload[i]=0;
     }
     txTemp.uFrame.sFrame.Check = SocCalcXor(txTemp.uFrame.alldat,txTemp.uFrame.sFrame.Frame_Length-2);
     txTemp.uFrame.sFrame.Frame_tail = 0x16;

     uartTxVector.append(txTemp);
    }
    if(EsendSNRead)
    {
     EsendSNRead = 0;
     txTemp.uFrame.sFrame.Frame_Head = 0x68;
     txTemp.uFrame.sFrame.Frame_cmd = 0x30;  //读序列号 command
     txTemp.uFrame.sFrame.Frame_Length = 0x0c;
     txTemp.uFrame.sFrame.Frame_Length_Check = 0xFF-txTemp.uFrame.sFrame.Frame_Length;
     for (uint8_t i=0;i<6;i++) {
      txTemp.uFrame.sFrame.Dat_Playload[i]=0;
     }
     txTemp.uFrame.sFrame.Check = SocCalcXor(txTemp.uFrame.alldat,txTemp.uFrame.sFrame.Frame_Length-2);
     txTemp.uFrame.sFrame.Frame_tail = 0x16;

     uartTxVector.append(txTemp);
    }

}


void BMCMainWindow::BubbleSort(uint16_t* arr, uint16_t sz)
{
    uint16_t i = 0;
    uint16_t j = 0;
    //共进行sz-1趟
    for (i = 0; i < sz-1; i++)
    {
        uint16_t flag = 1;//每一趟进来都假设有序
        // 每一趟
        for (j = 0; j < sz - 1 - i; j++)
        {
            if (arr[j] > arr[j + 1])
            {
                uint16_t tmp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = tmp;
                flag = 0;
            }
        }
        //若falg还是1，说明没有交换->已经有序了break退出
        if (flag == 1)
        {
            break;
        }
    }
}
void BMCMainWindow::BaseInfoParser(QByteArray buffer)
{
    uint16_t list1[30]={0},temp=0;
    uint32_t TotalVolt = 0;
    uint32_t temp32 = 0;
    uint8_t tu,tu1,tu2,tu3;
    QString str;

    uint8_t l= buffer[71];
    for (uint8_t i = 0;i<l;i++) {
        tu =  buffer[5+2*i];
        tu1 = buffer[4+2*i];
        temp = tu;
        temp <<= 8;
        temp |= tu1;
        list1[i] = temp;
        TotalVolt += temp;

        if(i<8)
        {
          str = QString::number(temp);
          QTableWidgetItem *item_ = new QTableWidgetItem;
          item_->setText(str);
          item_->setTextColor(Qt::blue);
          item_->setFont(QFont("Microsoft Yahei", 10));
          item_->setTextAlignment(Qt::AlignCenter);
          ui->tableWidgetSystemPara->setItem(15,i+1,item_);
        }
        else
        {
            str = QString::number(temp);
            QTableWidgetItem *item_ = new QTableWidgetItem;
            item_->setText(str);
            item_->setTextColor(Qt::blue);
            item_->setFont(QFont("Microsoft Yahei", 10));
            item_->setTextAlignment(Qt::AlignCenter);
            ui->tableWidgetSystemPara->setItem(16,i+1-8,item_);
        }
//        qDebug()<<temp;
        BaseInfo_Error = 0;
        if(temp<3000){

            BaseInfo_Error=1;
            str = "欠压,<3V";
            QTableWidgetItem *itemx = new QTableWidgetItem;
            itemx->setText(str);
            itemx->setTextColor(Qt::red);
            itemx->setFont(QFont("Microsoft Yahei", 10));
            itemx->setTextAlignment(Qt::AlignCenter);
            ui->tableWidgetSystemPara->setItem(13,7,itemx);
        }
        else
        {

            str = "ok";
            QTableWidgetItem *item_ = new QTableWidgetItem;
            item_->setText(str);
            item_->setTextColor(Qt::blue);
            item_->setFont(QFont("Microsoft Yahei", 10));
            item_->setTextAlignment(Qt::AlignCenter);
            ui->tableWidgetSystemPara->setItem(13,7,item_);
        }
  //cell volt
    }

    Bms_infor_upload.BMSinfor.TotalVolt = TotalVolt;
    str = QString::number(TotalVolt);
    QTableWidgetItem *item = new QTableWidgetItem;
    item->setText(str);
    item->setTextColor(Qt::blue);
    item->setFont(QFont("Microsoft Yahei", 10));
    item->setTextAlignment(Qt::AlignCenter);
    ui->tableWidgetSystemPara->setItem(1,7,item); //total voltage


    Bms_infor_upload.BMSinfor.CellNum = buffer[71];
    BubbleSort(list1,buffer[71]);

    temp = list1[Bms_infor_upload.BMSinfor.CellNum-1]-list1[0];
    Bms_infor_upload.BMSinfor.VoltDiff = temp;
    str = "压差：" + QString::number(temp)+"mV";
    ui->labelDeltaVolt->setText(str);


    signed short int SecdCurr = (signed short)(((uint8_t)buffer[85] << 8) | (uint8_t)buffer[84]);
    str = QString::number((signed char)SecdCurr);
    QTableWidgetItem *itemSecdCurr = new QTableWidgetItem;
    itemSecdCurr->setText(str);
    itemSecdCurr->setTextColor(Qt::blue);
    itemSecdCurr->setFont(QFont("Microsoft Yahei", 10));
    itemSecdCurr->setTextAlignment(Qt::AlignCenter);
    ui->tableWidgetSystemPara->setItem(17,7,itemSecdCurr);


    str = QString::number((signed char)buffer[36]);
    QTableWidgetItem *item1 = new QTableWidgetItem;
    item1->setText(str);
    if((signed char)buffer[36]>80)
     item1->setTextColor(Qt::red);
    else
     item1->setTextColor(Qt::blue);
    item1->setFont(QFont("Microsoft Yahei", 10));
    item1->setTextAlignment(Qt::AlignCenter);
    ui->tableWidgetSystemPara->setItem(10,7,item1); //temperature1

    str = QString::number((signed char)buffer[37]);
    QTableWidgetItem *item2 = new QTableWidgetItem;
    item2->setText(str);
    if((signed char)buffer[37]>80)
     item2->setTextColor(Qt::red);
    else
     item2->setTextColor(Qt::blue);
    item2->setFont(QFont("Microsoft Yahei", 10));
    item2->setTextAlignment(Qt::AlignCenter);
    ui->tableWidgetSystemPara->setItem(11,7,item2);//temperature2

    str = QString::number((signed char)buffer[38]);
    QTableWidgetItem *item3 = new QTableWidgetItem;
    item3->setText(str);
    if((signed char)buffer[38]>80)
     item3->setTextColor(Qt::red);
    else
     item3->setTextColor(Qt::blue);
    item3->setFont(QFont("Microsoft Yahei", 10));
    item3->setTextAlignment(Qt::AlignCenter);
    ui->tableWidgetSystemPara->setItem(12,7,item3);//temperature3


    tu =  buffer[40];
    tu1 = buffer[39];
    temp = tu;
    temp <<= 8;
    temp |= tu1;
    Bms_infor_upload.BMSinfor.DC = temp*10;
    str = QString::number(temp*10);
    QTableWidgetItem *item4 = new QTableWidgetItem;
    item4->setText(str);
    if(Bms_infor_upload.BMSinfor.DC==Bms_cfg_infor.BMSinfor.DC)
    item4->setTextColor(Qt::blue);
    else
    {BaseInfo_Error=1;item4->setTextColor(Qt::red);}
    item4->setFont(QFont("Microsoft Yahei", 10));
    item4->setTextAlignment(Qt::AlignCenter);
    ui->tableWidgetSystemPara->setItem(2,7,item4);//DesignCap

    tu =  buffer[42];
    tu1 = buffer[41];
    temp = tu;
    temp <<= 8;
    temp |= tu1;
    Bms_infor_upload.BMSinfor.FCC = temp*10;
    str = QString::number(temp*10);
    QTableWidgetItem *item5 = new QTableWidgetItem;
    item5->setText(str);
    item5->setTextColor(Qt::blue);
    item5->setFont(QFont("Microsoft Yahei", 10));
    item5->setTextAlignment(Qt::AlignCenter);
    ui->tableWidgetSystemPara->setItem(3,7,item5);//FCC


    tu =  buffer[44];
    tu1 = buffer[43];
    temp = tu;
    temp <<= 8;
    temp |= tu1;
    Bms_infor_upload.BMSinfor.RC = temp*10;
    str = QString::number(temp*10);
    QTableWidgetItem *item6 = new QTableWidgetItem;
    item6->setText(str);
    item6->setTextColor(Qt::blue);
    item6->setFont(QFont("Microsoft Yahei", 10));
    item6->setTextAlignment(Qt::AlignCenter);
    ui->tableWidgetSystemPara->setItem(5,7,item6);//RC


    tu =  buffer[46];
    tu1 = buffer[45];
    temp = tu;
    temp <<= 8;
    temp |= tu1;
    Bms_infor_upload.BMSinfor.RSOC = temp;
    str = QString::number(temp);
    QTableWidgetItem *item7 = new QTableWidgetItem;
    item7->setText(str);
    if(Bms_infor_upload.BMSinfor.RSOC>=Bms_cfg_infor.SOCDetect_lower &&Bms_infor_upload.BMSinfor.RSOC<=Bms_cfg_infor.SOCDetect_upper)
    item7->setTextColor(Qt::blue);
    else
    {BaseInfo_Error=1;item7->setTextColor(Qt::red);}
    item7->setFont(QFont("Microsoft Yahei", 10));
    item7->setTextAlignment(Qt::AlignCenter);
    ui->tableWidgetSystemPara->setItem(4,7,item7);//soc


    ui->progressBarSOC->setValue(Bms_infor_upload.BMSinfor.RSOC);

    tu = buffer[50];
    tu1 = buffer[49];
    tu2 = buffer[48];
    tu3 = buffer[47];
    temp32 = tu;
    temp32 <<= 8;
    temp32 |= tu1;
    temp32 <<= 8;
    temp32 |= tu2;
    temp32 <<= 8;
    temp32 |= tu3;
    int test1 = (int)temp32;
    Bms_infor_upload.BMSinfor.PackCurrent = test1;
    str = QString::number(test1);
    QTableWidgetItem *item8 = new QTableWidgetItem;
    item8->setText(str);
    item8->setTextColor(Qt::blue);
    item8->setFont(QFont("Microsoft Yahei", 10));
    item8->setTextAlignment(Qt::AlignCenter);
    ui->tableWidgetSystemPara->setItem(9,7,item8);//Current

    str = "电流：" + QString::number(test1)+"mA";
    ui->labelCurrent->setText(str);



    QString SWVersion = QString::number(buffer[55], 16);
    SWVersion.insert(1,'.');
    SWVersion.insert(0,'V');
    Bms_infor_upload.BMSinfor.SWVersion = SWVersion;
    QTableWidgetItem *item10 = new QTableWidgetItem;
    item10->setText(SWVersion);
    if(Bms_infor_upload.BMSinfor.SWVersion==Bms_cfg_infor.Version)
    item10->setTextColor(Qt::blue);
    else
     {BaseInfo_Error = 1;item10->setTextColor(Qt::red);}
    item10->setFont(QFont("Microsoft Yahei", 10));
    item10->setTextAlignment(Qt::AlignCenter);
    ui->tableWidgetSystemPara->setItem(0,7,item10);





    Bat_Status[0] = (uint8_t)buffer[62];
    Bat_Status[1] = (uint8_t)buffer[63];
    Bat_Status[2] = (uint8_t)buffer[64];
    Pack_Status = ((uint8_t)buffer[66] << 8) | (uint8_t)buffer[65];
    Pack_Status2 = (uint8_t)buffer[90];
    setTableStatus(Bat_Status, Pack_Status);


    Bms_infor_upload.BMSinfor.Bat_Status = buffer[64];
    Bms_infor_upload.BMSinfor.Bat_Status<<=8;
    Bms_infor_upload.BMSinfor.Bat_Status |= buffer[63];
    Bms_infor_upload.BMSinfor.Bat_Status<<=8;
    Bms_infor_upload.BMSinfor.Bat_Status |= buffer[62];
    if(Bms_infor_upload.BMSinfor.Bat_Status & 0x00000FFF){
        BaseInfo_Error=1;
        str = "异常";
        QTableWidgetItem *itemx = new QTableWidgetItem;
        itemx->setText(str);
        itemx->setTextColor(Qt::red);
        itemx->setFont(QFont("Microsoft Yahei", 10));
        itemx->setTextAlignment(Qt::AlignCenter);
        ui->tableWidgetSystemPara->setItem(14,7,itemx);
    }
    else
    {
        str = "正常";
        QTableWidgetItem *item_ = new QTableWidgetItem;
        item_->setText(str);
        item_->setTextColor(Qt::blue);
        item_->setFont(QFont("Microsoft Yahei", 10));
        item_->setTextAlignment(Qt::AlignCenter);
        ui->tableWidgetSystemPara->setItem(14,7,item_);
    }
    tu =  buffer[66];
    tu1 = buffer[65];
    temp = tu;
    temp <<= 8;
    temp |= tu1;
    Bms_infor_upload.BMSinfor.Pack_Status = temp;

    if(Bms_infor_upload.BMSinfor.Pack_Status&0x0800)
    {
        str = "放电";
        QTableWidgetItem *item_ = new QTableWidgetItem;
        item_->setText(str);
        item_->setTextColor(Qt::blue);
        item_->setFont(QFont("Microsoft Yahei", 10));
        item_->setTextAlignment(Qt::AlignCenter);
        ui->tableWidgetSystemPara->setItem(14,10,item_);
    }
    else if(Bms_infor_upload.BMSinfor.Pack_Status&0x1000)
    {
        str = "充电";
        QTableWidgetItem *item_ = new QTableWidgetItem;
        item_->setText(str);
        item_->setTextColor(Qt::blue);
        item_->setFont(QFont("Microsoft Yahei", 10));
        item_->setTextAlignment(Qt::AlignCenter);
        ui->tableWidgetSystemPara->setItem(14,10,item_);
    }
    else
    {
        str = " ";
        QTableWidgetItem *item_ = new QTableWidgetItem;
        item_->setText(str);
        item_->setTextColor(Qt::blue);
        item_->setFont(QFont("Microsoft Yahei", 10));
        item_->setTextAlignment(Qt::AlignCenter);
        ui->tableWidgetSystemPara->setItem(14,10,item_);
    }
    //固件备份 FBC状态
    if(Bms_infor_upload.BMSinfor.Pack_Status&0x0800)
    {
        str = "固件备份完成";
        QTableWidgetItem *item_ = new QTableWidgetItem;
        item_->setText(str);
        item_->setTextColor(Qt::blue);
        item_->setFont(QFont("Microsoft Yahei", 10));
        item_->setTextAlignment(Qt::AlignCenter);
        ui->tableWidgetSystemPara->setItem(17,4,item_);
    }
    else
    {
        str = "未备份";
        QTableWidgetItem *item_ = new QTableWidgetItem;
        item_->setText(str);
        item_->setTextColor(Qt::blue);
        item_->setFont(QFont("Microsoft Yahei", 10));
        item_->setTextAlignment(Qt::AlignCenter);
        ui->tableWidgetSystemPara->setItem(17,4,item_);
    }

    if(buffer[64]&0x01)
    {
        str = "开启";
        QTableWidgetItem *item_ = new QTableWidgetItem;
        item_->setText(str);
        item_->setTextColor(Qt::blue);
        item_->setFont(QFont("Microsoft Yahei", 10));
        item_->setTextAlignment(Qt::AlignCenter);
        ui->tableWidgetSystemPara->setItem(16,10,item_);
    }
    else
    {
        str = "关闭";
        QTableWidgetItem *item_ = new QTableWidgetItem;
        item_->setText(str);
        item_->setTextColor(Qt::blue);
        item_->setFont(QFont("Microsoft Yahei", 10));
        item_->setTextAlignment(Qt::AlignCenter);
        ui->tableWidgetSystemPara->setItem(16,10,item_);
    }

    if(buffer[64]&0x02)
    {
        str = "开启";
        QTableWidgetItem *item_ = new QTableWidgetItem;
        item_->setText(str);
        item_->setTextColor(Qt::blue);
        item_->setFont(QFont("Microsoft Yahei", 10));
        item_->setTextAlignment(Qt::AlignCenter);
        ui->tableWidgetSystemPara->setItem(15,10,item_);
    }
    else
    {
        str = "关闭";
        QTableWidgetItem *item_ = new QTableWidgetItem;
        item_->setText(str);
        item_->setTextColor(Qt::blue);
        item_->setFont(QFont("Microsoft Yahei", 10));
        item_->setTextAlignment(Qt::AlignCenter);
        ui->tableWidgetSystemPara->setItem(15,10,item_);
    }

//    str = QString::number(temp32);
//    ui->tableWidgetSBS->setItem(12,1,new QTableWidgetItem(str));

//    Bat_Addr = buffer[73];
//    str = QString::number(Bat_Addr);
//    ui->lineEditBatteryAddr->setText(str);

    RTC_Year = buffer[74];
    QString str00 = QString::number((int)RTC_Year+(int)2018);
    RTC_Month = buffer[75];
    QString str01 = QString::number(RTC_Month);
    if(RTC_Month<10) str01.insert(0,'0');
    RTC_Day = buffer[76];
    QString str02 = QString::number(RTC_Day);
    if(RTC_Day<10) str02.insert(0,'0');
    RTC_Hour = buffer[77];
    QString str03 = QString::number(RTC_Hour);
    if(RTC_Hour<10) str03.insert(0,'0');
    RTC_Minute = buffer[78];
    QString str04 = QString::number(RTC_Minute);
    if(RTC_Minute<10) str04.insert(0,'0');
    RTC_Second = buffer[79];
    QString str05 = QString::number(RTC_Second);
    if(RTC_Second<10) str05.insert(0,'0');
    str = str00+"-"+str01+"-"+str02+" "+str03+":"+str04+":"+str05;
    ui->labelRTC->setText(str);

    tu =  buffer[81];
    tu1 = buffer[80];
    temp = tu;
    temp <<= 8;
    temp |= tu1;
    Bms_infor_upload.BMSinfor.MCI = temp;
    str = QString::number(temp);
    QTableWidgetItem *item12 = new QTableWidgetItem;
    item12->setText(str);
    item12->setTextColor(Qt::blue);
    item12->setFont(QFont("Microsoft Yahei", 10));
    item12->setTextAlignment(Qt::AlignCenter);
    ui->tableWidgetSystemPara->setItem(8,7,item12);//mci

}


void BMCMainWindow::SystemInfoParser(QByteArray buffer)
{
   uint32_t temp32=0;
   QString str;
   uint32_t ovt_arr[] = {100,200,300,400,600,800,1000,2000,3000,
                        4000,6000,8000,10000,20000,30000,40000};
   temp32 = (uchar)buffer[7];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[6];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[5];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[4];
   parameter_error=0;
   Bms_infor_upload.BMSinfor.ShutDTime = temp32;
   str = QString::number(temp32);
   QTableWidgetItem *item12 = new QTableWidgetItem;
   item12->setText(str);
   if(Bms_infor_upload.BMSinfor.ShutDTime==Bms_cfg_infor.BMSinfor.ShutDTime)
   item12->setTextColor(Qt::blue);
   else
   {parameter_error=1;item12->setTextColor(Qt::red);}
   item12->setFont(QFont("Microsoft Yahei", 10));
   item12->setTextAlignment(Qt::AlignCenter);
   ui->tableWidgetSystemPara->setItem(0,1,item12);//ShutDownTime

   temp32 = (uchar)buffer[15];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[14];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[13];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[12];
   Bms_infor_upload.BMSinfor.BalEndV = temp32;
   str = QString::number(temp32);
   QTableWidgetItem *item13 = new QTableWidgetItem;
   item13->setText(str);
   if(Bms_infor_upload.BMSinfor.BalEndV==Bms_cfg_infor.BMSinfor.BalEndV)
   item13->setTextColor(Qt::blue);
   else
   {parameter_error=1; item13->setTextColor(Qt::red); }
   item13->setFont(QFont("Microsoft Yahei", 10));
   item13->setTextAlignment(Qt::AlignCenter);
   ui->tableWidgetSystemPara->setItem(1,1,item13);//BalEndV  均衡停止电压

   temp32 = (uchar)buffer[19];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[18];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[17];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[16];
   Bms_infor_upload.BMSinfor.ChgEndCur = temp32;
   str = QString::number(temp32);
   QTableWidgetItem *item14 = new QTableWidgetItem;
   item14->setText(str);
   if(Bms_infor_upload.BMSinfor.ChgEndCur==Bms_cfg_infor.BMSinfor.ChgEndCur)
   item14->setTextColor(Qt::blue);
   else
   {parameter_error=1; item14->setTextColor(Qt::red); }
   item14->setFont(QFont("Microsoft Yahei", 10));
   item14->setTextAlignment(Qt::AlignCenter);
   ui->tableWidgetSystemPara->setItem(2,1,item14);//ChgEndCur  充电截止电流


   temp32 = (uchar)buffer[23];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[22];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[21];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[20];
   Bms_infor_upload.BMSinfor.ChgEndVol = temp32;
   str = QString::number(temp32);
   QTableWidgetItem *item15 = new QTableWidgetItem;
   item15->setText(str);
   if(Bms_infor_upload.BMSinfor.ChgEndVol==Bms_cfg_infor.BMSinfor.ChgEndVol)
   item15->setTextColor(Qt::blue);
   else
   {parameter_error=1; item15->setTextColor(Qt::red); }
   item15->setFont(QFont("Microsoft Yahei", 10));
   item15->setTextAlignment(Qt::AlignCenter);
   ui->tableWidgetSystemPara->setItem(3,1,item15);//ChgEndVol  充电截止电压

   temp32 = (uchar)buffer[27];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[26];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[25];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[24];
   Bms_infor_upload.BMSinfor.ChgEndDel = temp32;
   str = QString::number(temp32);
   QTableWidgetItem *item16 = new QTableWidgetItem;
   item16->setText(str);
   if(Bms_infor_upload.BMSinfor.ChgEndDel==Bms_cfg_infor.BMSinfor.ChgEndDel)
   item16->setTextColor(Qt::blue);
   else
   {parameter_error=1; item16->setTextColor(Qt::red); }
   item16->setFont(QFont("Microsoft Yahei", 10));
   item16->setTextAlignment(Qt::AlignCenter);
   ui->tableWidgetSystemPara->setItem(4,1,item16);//ChgEndDel  充电截止延时


   temp32 = (uchar)buffer[31];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[30];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[29];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[28];
   Bms_infor_upload.BMSinfor.CfgOption = temp32;
   str = QString::number(temp32);
   QTableWidgetItem *item17 = new QTableWidgetItem;
   item17->setText(str);
   item17->setTextColor(Qt::blue);
   item17->setFont(QFont("Microsoft Yahei", 10));
   item17->setTextAlignment(Qt::AlignCenter);
   ui->tableWidgetSystemPara->setItem(5,1,item17);//CfgOption  配置信息

   temp32 = (uchar)buffer[39];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[38];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[37];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[36];
   Bms_infor_upload.BMSinfor.MNFDate = temp32;
   str = QString::number(temp32);
   QTableWidgetItem *item18 = new QTableWidgetItem;
   item18->setText(str);
   if(Bms_infor_upload.BMSinfor.MNFDate==Bms_cfg_infor.BMSinfor.MNFDate)
   item18->setTextColor(Qt::blue);
   else
   {parameter_error=1; item18->setTextColor(Qt::red); }
   item18->setFont(QFont("Microsoft Yahei", 10));
   item18->setTextAlignment(Qt::AlignCenter);
   ui->tableWidgetSystemPara->setItem(6,1,item18);//MNFDate  生产日期

   temp32 = (uchar)buffer[43];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[42];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[41];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[40];
   Bms_infor_upload.BMSinfor.SlfDsqBalV = temp32;
   str = QString::number(temp32);
   QTableWidgetItem *item19 = new QTableWidgetItem;
   item19->setText(str);
   if(Bms_infor_upload.BMSinfor.SlfDsqBalV==Bms_cfg_infor.BMSinfor.SlfDsqBalV)
   item19->setTextColor(Qt::blue);
   else
   {parameter_error=1; item19->setTextColor(Qt::red); }
   item19->setFont(QFont("Microsoft Yahei", 10));
   item19->setTextAlignment(Qt::AlignCenter);
   ui->tableWidgetSystemPara->setItem(7,1,item19);//SlfDsgBalV  自放电平衡电压

   temp32 = (uchar)buffer[47];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[46];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[45];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[44];
   Bms_infor_upload.BMSinfor.BalStartV = temp32;
   str = QString::number(temp32);
   QTableWidgetItem *item20 = new QTableWidgetItem;
   item20->setText(str);
   if(Bms_infor_upload.BMSinfor.BalStartV==Bms_cfg_infor.BMSinfor.BalStartV)
   item20->setTextColor(Qt::blue);
   else
   {parameter_error=1; item20->setTextColor(Qt::red); }
   item20->setFont(QFont("Microsoft Yahei", 10));
   item20->setTextAlignment(Qt::AlignCenter);
   ui->tableWidgetSystemPara->setItem(8,1,item20);//BalStartV  平衡启动阈值

   temp32 = (uchar)buffer[51];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[50];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[49];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[48];
   Bms_infor_upload.BMSinfor.BalDelay = temp32;
   str = QString::number(temp32);
   QTableWidgetItem *item21 = new QTableWidgetItem;
   item21->setText(str);
   if(Bms_infor_upload.BMSinfor.BalDelay==Bms_cfg_infor.BMSinfor.BalDelay)
   item21->setTextColor(Qt::blue);
   else
   {parameter_error=1; item21->setTextColor(Qt::red); }
   item21->setFont(QFont("Microsoft Yahei", 10));
   item21->setTextAlignment(Qt::AlignCenter);
   ui->tableWidgetSystemPara->setItem(9,1,item21);//BalDel  平衡延时

   temp32 = (uchar)buffer[55];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[54];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[53];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[52];
   Bms_infor_upload.BMSinfor.SlfDsgDel = temp32;
   str = QString::number(temp32);
   QTableWidgetItem *item22 = new QTableWidgetItem;
   item22->setText(str);
   if(Bms_infor_upload.BMSinfor.SlfDsgDel==Bms_cfg_infor.BMSinfor.SlfDsgDel)
   item22->setTextColor(Qt::blue);
   else
   {parameter_error=1; item22->setTextColor(Qt::red); }
   item22->setFont(QFont("Microsoft Yahei", 10));
   item22->setTextAlignment(Qt::AlignCenter);
   ui->tableWidgetSystemPara->setItem(10,1,item22);//SlfDsgDel  自放电延时

   temp32 = (uchar)buffer[59];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[58];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[57];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[56];
   Bms_infor_upload.BMSinfor.OtherState = temp32;
   str = QString::number(temp32,16);//转换为十六进制
   str.insert(0,"0x");
   QTableWidgetItem *item23 = new QTableWidgetItem;
   item23->setText(str);
   item23->setTextColor(Qt::blue);
   item23->setFont(QFont("Microsoft Yahei", 8));
   item23->setTextAlignment(Qt::AlignCenter);
   ui->tableWidgetSystemPara->setItem(11,1,item23);//OtherState  校准状态

   temp32 = (uchar)buffer[83];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[82];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[81];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[80];
   str = QString::number(temp32,16);//转换为十六进制
   str.insert(0,"0x");
   QTableWidgetItem *item24 = new QTableWidgetItem;
   item24->setText(str);
   item24->setTextColor(Qt::blue);
   item24->setFont(QFont("Microsoft Yahei", 10));
   item24->setTextAlignment(Qt::AlignCenter);
   ui->tableWidgetSystemPara->setItem(12,1,item24);//AuthCode  授权码
//   Bms_infor_upload.BMSinfor.AuthCode = temp32;
   temp32 = (uchar)buffer[99];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[98];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[97];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[96];
   float fl = *(float*)&temp32;
//   str = QString::number(temp32,16);
//   qDebug()<<"fl:"<<fl;
   Bms_infor_upload.BMSinfor.SampleRV = fl;
   str.setNum(fl,'f',3);
   QTableWidgetItem *item25 = new QTableWidgetItem;
   item25->setText(str);
   if(Bms_infor_upload.BMSinfor.SampleRV==Bms_cfg_infor.BMSinfor.SampleRV)
   item25->setTextColor(Qt::blue);
   else
   {parameter_error=1; item25->setTextColor(Qt::red); }
   item25->setFont(QFont("Microsoft Yahei", 10));
   item25->setTextAlignment(Qt::AlignCenter);
   ui->tableWidgetSystemPara->setItem(13,1,item25);//SampleRV  采样电阻
//   qDebug()<<"采样电阻:"<<fl<<"  +buffer:"<<buffer.toHex(' ').toUpper() + "\n";
//   QString bufferSting;
//   for(int i = 0; i < buffer.size(); i++)
//   {
//       bufferSting拼接buffer[0]
//   }

   temp32 = (uchar)buffer[103];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[102];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[101];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[100];
   Bms_infor_upload.BMSinfor.CellNum = temp32;
   str = QString::number(temp32);
   QTableWidgetItem *item26 = new QTableWidgetItem;
   item26->setText(str);
   if(Bms_infor_upload.BMSinfor.CellNum==Bms_cfg_infor.BMSinfor.CellNum)
   item26->setTextColor(Qt::blue);
   else
   {parameter_error=1; item26->setTextColor(Qt::red); }
   item26->setFont(QFont("Microsoft Yahei", 10));
   item26->setTextAlignment(Qt::AlignCenter);
   ui->tableWidgetSystemPara->setItem(0,4,item26);//CellNum  串数


   temp32 = (uchar)buffer[107];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[106];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[105];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[104];
   Bms_infor_upload.BMSinfor.OV=temp32*5;
   str = QString::number(temp32*5);
   QTableWidgetItem *item27 = new QTableWidgetItem;
   item27->setText(str);
   if(Bms_infor_upload.BMSinfor.OV==Bms_cfg_infor.BMSinfor.OV)
   item27->setTextColor(Qt::blue);
   else
   {parameter_error=1; item27->setTextColor(Qt::red); }
   item27->setFont(QFont("Microsoft Yahei", 10));
   item27->setTextAlignment(Qt::AlignCenter);
   ui->tableWidgetSystemPara->setItem(1,4,item27);//OV  过充电压

   temp32 = (uchar)buffer[111];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[110];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[109];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[108];
   Bms_infor_upload.BMSinfor.OVR = temp32*5;
   str = QString::number(temp32*5);
   QTableWidgetItem *item28 = new QTableWidgetItem;
   item28->setText(str);
   if(Bms_infor_upload.BMSinfor.OVR==Bms_cfg_infor.BMSinfor.OVR)
   item28->setTextColor(Qt::blue);
   else
   {parameter_error=1; item28->setTextColor(Qt::red); }
   item28->setFont(QFont("Microsoft Yahei", 10));
   item28->setTextAlignment(Qt::AlignCenter);
   ui->tableWidgetSystemPara->setItem(2,4,item28);//OVR  过充恢得电压

   temp32 = (uchar)buffer[115];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[114];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[113];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[112];
   Bms_infor_upload.BMSinfor.OVT = ovt_arr[temp32];
   str = QString::number(ovt_arr[temp32]);
   QTableWidgetItem *item29 = new QTableWidgetItem;
   item29->setText(str);
   if(Bms_infor_upload.BMSinfor.OVT==Bms_cfg_infor.BMSinfor.OVT)
   item29->setTextColor(Qt::blue);
   else
   {parameter_error=1; item29->setTextColor(Qt::red); }
   item29->setFont(QFont("Microsoft Yahei", 10));
   item29->setTextAlignment(Qt::AlignCenter);
   ui->tableWidgetSystemPara->setItem(3,4,item29);//OVT  过充延时

   temp32 = (uchar)buffer[119];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[118];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[117];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[116];
   Bms_infor_upload.BMSinfor.UV = temp32*20;
   str = QString::number(temp32*20);
   QTableWidgetItem *item30 = new QTableWidgetItem;
   item30->setText(str);
   if(Bms_infor_upload.BMSinfor.UV==Bms_cfg_infor.BMSinfor.UV)
   item30->setTextColor(Qt::blue);
   else
   {parameter_error=1; item30->setTextColor(Qt::red); }
   item30->setFont(QFont("Microsoft Yahei", 10));
   item30->setTextAlignment(Qt::AlignCenter);
   ui->tableWidgetSystemPara->setItem(4,4,item30);//UV  过充电压

   temp32 = (uchar)buffer[123];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[122];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[121];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[120];
   Bms_infor_upload.BMSinfor.UVR = temp32*20;
   str = QString::number(temp32*20);
   QTableWidgetItem *item31 = new QTableWidgetItem;
   item31->setText(str);
   if(Bms_infor_upload.BMSinfor.UVR==Bms_cfg_infor.BMSinfor.UVR)
   item31->setTextColor(Qt::blue);
   else
   {parameter_error=1; item31->setTextColor(Qt::red); }
   item31->setFont(QFont("Microsoft Yahei", 10));
   item31->setTextAlignment(Qt::AlignCenter);
   ui->tableWidgetSystemPara->setItem(5,4,item31);//UVR  过充恢得电压

   temp32 = (uchar)buffer[127];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[126];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[125];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[124];
   Bms_infor_upload.BMSinfor.UVT = ovt_arr[temp32];
   str = QString::number(ovt_arr[temp32]);
   QTableWidgetItem *item32 = new QTableWidgetItem;
   item32->setText(str);
   if(Bms_infor_upload.BMSinfor.UVT==Bms_cfg_infor.BMSinfor.UVT)
   item32->setTextColor(Qt::blue);
   else
   {parameter_error=1; item32->setTextColor(Qt::red); }
   item32->setFont(QFont("Microsoft Yahei", 10));
   item32->setTextAlignment(Qt::AlignCenter);
   ui->tableWidgetSystemPara->setItem(6,4,item32);//UVT  过充延时

   temp32 = (uchar)buffer[131];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[130];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[129];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[128];
   Bms_infor_upload.BMSinfor.BALV = temp32*20;
   str = QString::number(temp32*20);
   QTableWidgetItem *item33 = new QTableWidgetItem;
   item33->setText(str);
   if(Bms_infor_upload.BMSinfor.BALV==Bms_cfg_infor.BMSinfor.BALV)
   item33->setTextColor(Qt::blue);
   else
   {parameter_error=1; item33->setTextColor(Qt::red); }
   item33->setFont(QFont("Microsoft Yahei", 10));
   item33->setTextAlignment(Qt::AlignCenter);
   ui->tableWidgetSystemPara->setItem(7,4,item33);//BALV 平衡开启电压设置

   temp32 = (uchar)buffer[135];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[134];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[133];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[132];
   Bms_infor_upload.BMSinfor.PREV = temp32*20;
   str = QString::number(temp32*20);
   QTableWidgetItem *item34 = new QTableWidgetItem;
   item34->setText(str);
   if(Bms_infor_upload.BMSinfor.PREV==Bms_cfg_infor.BMSinfor.PREV)
   item34->setTextColor(Qt::blue);
   else
   {parameter_error=1; item34->setTextColor(Qt::red); }
   item34->setFont(QFont("Microsoft Yahei", 10));
   item34->setTextAlignment(Qt::AlignCenter);
   ui->tableWidgetSystemPara->setItem(8,4,item34);//PREV 预充电开启电压设置

   temp32 = (uchar)buffer[139];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[138];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[137];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[136];
   Bms_infor_upload.BMSinfor.LZV = temp32*20;
   str = QString::number(temp32*20);
   QTableWidgetItem *item35 = new QTableWidgetItem;
   item35->setText(str);
   if(Bms_infor_upload.BMSinfor.LZV==Bms_cfg_infor.BMSinfor.LZV)
   item35->setTextColor(Qt::blue);
   else
   {parameter_error=1; item35->setTextColor(Qt::red); }
   item35->setFont(QFont("Microsoft Yahei", 10));
   item35->setTextAlignment(Qt::AlignCenter);
   ui->tableWidgetSystemPara->setItem(9,4,item35);//LZV 低压禁止充电电压设置

   temp32 = (uchar)buffer[143];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[142];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[141];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[140];
   Bms_infor_upload.BMSinfor.PFV = temp32*20;
   str = QString::number(temp32*20);
   QTableWidgetItem *item36 = new QTableWidgetItem;
   item36->setText(str);
   if(Bms_infor_upload.BMSinfor.PFV==Bms_cfg_infor.BMSinfor.PFV)
   item36->setTextColor(Qt::blue);
   else
   {parameter_error=1; item36->setTextColor(Qt::red); }
   item36->setFont(QFont("Microsoft Yahei", 10));
   item36->setTextAlignment(Qt::AlignCenter);
   ui->tableWidgetSystemPara->setItem(10,4,item36);//PFV 二次过充电电压设置


   uint8_t ocd1v[] = {20,30,40,50,60,70,80,90,100,110,120,130,140,160,180,200};
   temp32 = (uchar)buffer[147];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[146];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[145];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[144];
   //str = QString::number(ocd1v[temp32]);
   float oc1_fl = ocd1v[temp32]/fl;
   str.setNum(oc1_fl,'f',3);
   Bms_infor_upload.BMSinfor.OCD1V = ocd1v[temp32];
   QTableWidgetItem *item37 = new QTableWidgetItem;
   item37->setText(str);
   if(Bms_infor_upload.BMSinfor.OCD1V==Bms_cfg_infor.BMSinfor.OCD1V)
   item37->setTextColor(Qt::blue);
   else
   {parameter_error=1; item37->setTextColor(Qt::red); }
   item37->setFont(QFont("Microsoft Yahei", 10));
   item37->setTextAlignment(Qt::AlignCenter);
   ui->tableWidgetSystemPara->setItem(11,4,item37);//OCD1V  放电过流1

   uint32_t ocd1t[] = {50,100,200,400,600,800,1000,2000,4000,6000,8000,10000,15000,20000,30000,40000};
   temp32 = (uchar)buffer[151];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[150];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[149];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[148];
   Bms_infor_upload.BMSinfor.OCD1T = ocd1t[temp32];
   str = QString::number(ocd1t[temp32]);
   QTableWidgetItem *item38 = new QTableWidgetItem;
   item38->setText(str);
   if(Bms_infor_upload.BMSinfor.OCD1T==Bms_cfg_infor.BMSinfor.OCD1T)
   item38->setTextColor(Qt::blue);
   else
   {parameter_error=1; item38->setTextColor(Qt::red); }
   item38->setFont(QFont("Microsoft Yahei", 10));
   item38->setTextAlignment(Qt::AlignCenter);
   ui->tableWidgetSystemPara->setItem(12,4,item38);//OCD1T  放电过流1延时


   uint16_t ocd2v[] = {30,40,50,60,70,80,90,100,120,140,160,180,200,300,400,500};
   temp32 = (uchar)buffer[155];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[154];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[153];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[152];
   Bms_infor_upload.BMSinfor.OCD2V = ocd2v[temp32];
   //str = QString::number(ocd2v[temp32]);
    oc1_fl = ocd2v[temp32]/fl;
   str.setNum(oc1_fl,'f',3);
   QTableWidgetItem *item39 = new QTableWidgetItem;
   item39->setText(str);
   if(Bms_infor_upload.BMSinfor.OCD2V==Bms_cfg_infor.BMSinfor.OCD2V)
   item39->setTextColor(Qt::blue);
   else
   {parameter_error=1; item39->setTextColor(Qt::red); }
   item39->setFont(QFont("Microsoft Yahei", 10));
   item39->setTextAlignment(Qt::AlignCenter);
   ui->tableWidgetSystemPara->setItem(13,4,item39);//OCD2V  放电过流2


   uint32_t ocd2t[] = {10,20,40,60,80,100,200,400,600,800,1000,2000,4000,8000,10000,20000};
   temp32 = (uchar)buffer[159];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[158];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[157];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[156];
   Bms_infor_upload.BMSinfor.OCD2T = ocd2t[temp32];
   str = QString::number(ocd2t[temp32]);
   QTableWidgetItem *item40 = new QTableWidgetItem;
   item40->setText(str);
   if(Bms_infor_upload.BMSinfor.OCD2T==Bms_cfg_infor.BMSinfor.OCD2T)
   item40->setTextColor(Qt::blue);
   else
   {parameter_error=1; item40->setTextColor(Qt::red); }
   item40->setFont(QFont("Microsoft Yahei", 10));
   item40->setTextAlignment(Qt::AlignCenter);
   ui->tableWidgetSystemPara->setItem(14,4,item40);//OCD2T  放电过流2延时


   uint32_t SCV[] = {50,80,110,140,170,200,230,260,290,320,350,400,500,600,800,1000};
   temp32 = (uchar)buffer[163];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[162];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[161];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[160];
   Bms_infor_upload.BMSinfor.SCV = SCV[temp32];
   //str = QString::number(SCV[temp32]);
   oc1_fl = SCV[temp32]/fl;
   str.setNum(oc1_fl,'f',3);
   QTableWidgetItem *item41 = new QTableWidgetItem;
   item41->setText(str);
   if(Bms_infor_upload.BMSinfor.SCV==Bms_cfg_infor.BMSinfor.SCV)
   item41->setTextColor(Qt::blue);
   else
   {parameter_error=1; item41->setTextColor(Qt::red); }
   item41->setFont(QFont("Microsoft Yahei", 10));
   item41->setTextAlignment(Qt::AlignCenter);
   ui->tableWidgetSystemPara->setItem(0,10,item41);//SCV  短路电流


   uint32_t SCVT[] = {0,64,128,192,256,320,384,448,512,576,640,704,768,832,896,960};
   temp32 = (uchar)buffer[167];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[166];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[165];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[164];
   Bms_infor_upload.BMSinfor.SCT = SCVT[temp32];
   str = QString::number(SCVT[temp32]);
   QTableWidgetItem *item42 = new QTableWidgetItem;
   item42->setText(str);
   if(Bms_infor_upload.BMSinfor.SCT==Bms_cfg_infor.BMSinfor.SCT)
   item42->setTextColor(Qt::blue);
   else
   {parameter_error=1; item42->setTextColor(Qt::red); }
   item42->setFont(QFont("Microsoft Yahei", 10));
   item42->setTextAlignment(Qt::AlignCenter);
   ui->tableWidgetSystemPara->setItem(1,10,item42);//SCVT  短路电流延时


   uint32_t OCCV[] = {20,30,40,50,60,70,80,90,100,110,120,130,140,160,180,200};
   temp32 = (uchar)buffer[171];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[170];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[169];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[168];
   Bms_infor_upload.BMSinfor.OCCV = OCCV[temp32];
   //str = QString::number(OCCV[temp32]);
   oc1_fl = OCCV[temp32]/fl;
   str.setNum(oc1_fl,'f',3);
   QTableWidgetItem *item43 = new QTableWidgetItem;
   item43->setText(str);
   if(Bms_infor_upload.BMSinfor.OCCV==Bms_cfg_infor.BMSinfor.OCCV)
   item43->setTextColor(Qt::blue);
   else
   {parameter_error=1; item43->setTextColor(Qt::red); }
   item43->setFont(QFont("Microsoft Yahei", 10));
   item43->setTextAlignment(Qt::AlignCenter);
   ui->tableWidgetSystemPara->setItem(2,10,item43);//OCCV  硬件充电过流值

   temp32 = (uchar)buffer[175];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[174];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[173];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[172];
   Bms_infor_upload.BMSinfor.OCCT = ocd2t[temp32];
   str = QString::number(ocd2t[temp32]);
   QTableWidgetItem *item44 = new QTableWidgetItem;
   item44->setText(str);
   if(Bms_infor_upload.BMSinfor.OCCT==Bms_cfg_infor.BMSinfor.OCCT)
   item44->setTextColor(Qt::blue);
   else
   {parameter_error=1; item44->setTextColor(Qt::red); }
   item44->setFont(QFont("Microsoft Yahei", 10));
   item44->setTextAlignment(Qt::AlignCenter);
   ui->tableWidgetSystemPara->setItem(3,10,item44);//OCCT  硬件充电过流延时

   temp32 = (uchar)buffer[179];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[178];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[177];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[176];
   Bms_infor_upload.BMSinfor.EOTC = *(int*)&temp32;
   str = QString::number(*(int*)&temp32);
   QTableWidgetItem *item45 = new QTableWidgetItem;
   item45->setText(str);
   if(Bms_infor_upload.BMSinfor.EOTC==Bms_cfg_infor.BMSinfor.EOTC)
   item45->setTextColor(Qt::blue);
   else
   {parameter_error=1; item45->setTextColor(Qt::red); }
   item45->setFont(QFont("Microsoft Yahei", 10));
   item45->setTextAlignment(Qt::AlignCenter);
   ui->tableWidgetSystemPara->setItem(4,10,item45);//ETOC  充电高温保护

   temp32 = (uchar)buffer[183];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[182];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[181];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[180];
   Bms_infor_upload.BMSinfor.EOTCR = *(int*)&temp32;
   str = QString::number(*(int*)&temp32);
   QTableWidgetItem *item46 = new QTableWidgetItem;
   item46->setText(str);
   if(Bms_infor_upload.BMSinfor.EOTCR==Bms_cfg_infor.BMSinfor.EOTCR)
   item46->setTextColor(Qt::blue);
   else
   {parameter_error=1; item46->setTextColor(Qt::red); }
   item46->setFont(QFont("Microsoft Yahei", 10));
   item46->setTextAlignment(Qt::AlignCenter);
   ui->tableWidgetSystemPara->setItem(5,10,item46);//ETOCR  充电高温保护释放

   temp32 = (uchar)buffer[187];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[186];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[185];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[184];
   Bms_infor_upload.BMSinfor.EUTC = *(int*)&temp32;
   str = QString::number(*(int*)&temp32);
   QTableWidgetItem *item47 = new QTableWidgetItem;
   item47->setText(str);
   if(Bms_infor_upload.BMSinfor.EUTC==Bms_cfg_infor.BMSinfor.EUTC)
   item47->setTextColor(Qt::blue);
   else
   {parameter_error=1; item47->setTextColor(Qt::red); }
   item47->setFont(QFont("Microsoft Yahei", 10));
   item47->setTextAlignment(Qt::AlignCenter);
   ui->tableWidgetSystemPara->setItem(6,10,item47);//EUTC  充电低温保护

   temp32 = (uchar)buffer[191];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[190];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[189];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[188];
   Bms_infor_upload.BMSinfor.EUTCR = *(int*)&temp32;
   str = QString::number(*(int*)&temp32);
   QTableWidgetItem *item48 = new QTableWidgetItem;
   item48->setText(str);
   if(Bms_infor_upload.BMSinfor.EUTCR==Bms_cfg_infor.BMSinfor.EUTCR)
   item48->setTextColor(Qt::blue);
   else
   {parameter_error=1; item48->setTextColor(Qt::red); }
   item48->setFont(QFont("Microsoft Yahei", 10));
   item48->setTextAlignment(Qt::AlignCenter);
   ui->tableWidgetSystemPara->setItem(7,10,item48);//EUTC  充电低温保护释放

   temp32 = (uchar)buffer[195];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[194];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[193];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[192];
   Bms_infor_upload.BMSinfor.EOTD = *(int*)&temp32;
   str = QString::number(*(int*)&temp32);
   QTableWidgetItem *item49 = new QTableWidgetItem;
   item49->setText(str);
   if(Bms_infor_upload.BMSinfor.EOTD==Bms_cfg_infor.BMSinfor.EOTD)
   item49->setTextColor(Qt::blue);
   else
   {parameter_error=1; item49->setTextColor(Qt::red); }
   item49->setFont(QFont("Microsoft Yahei", 10));
   item49->setTextAlignment(Qt::AlignCenter);
   ui->tableWidgetSystemPara->setItem(8,10,item49);//EOTD  放电高温保护

   temp32 = (uchar)buffer[199];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[198];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[197];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[196];
   Bms_infor_upload.BMSinfor.EOTDR = *(int*)&temp32;
   str = QString::number(*(int*)&temp32);
   QTableWidgetItem *item50 = new QTableWidgetItem;
   item50->setText(str);
   if(Bms_infor_upload.BMSinfor.EOTDR==Bms_cfg_infor.BMSinfor.EOTDR)
   item50->setTextColor(Qt::blue);
   else
   {parameter_error=1; item50->setTextColor(Qt::red); }
   item50->setFont(QFont("Microsoft Yahei", 10));
   item50->setTextAlignment(Qt::AlignCenter);
   ui->tableWidgetSystemPara->setItem(9,10,item50);//EOTDR  放电高温保护释放

   temp32 = (uchar)buffer[203];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[202];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[201];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[200];
   Bms_infor_upload.BMSinfor.EUTD = *(int*)&temp32;
   str = QString::number(*(int*)&temp32);
   QTableWidgetItem *item51 = new QTableWidgetItem;
   item51->setText(str);
   if(Bms_infor_upload.BMSinfor.EUTD==Bms_cfg_infor.BMSinfor.EUTD)
   item51->setTextColor(Qt::blue);
   else
   {parameter_error=1; item51->setTextColor(Qt::red); }
   item51->setFont(QFont("Microsoft Yahei", 10));
   item51->setTextAlignment(Qt::AlignCenter);
   ui->tableWidgetSystemPara->setItem(10,10,item51);//EUTD  放电低温保护

   temp32 = (uchar)buffer[207];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[206];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[205];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[204];
   Bms_infor_upload.BMSinfor.EUTDR = *(int*)&temp32;
   str = QString::number(*(int*)&temp32);
   QTableWidgetItem *item52 = new QTableWidgetItem;
   item52->setText(str);
   if(Bms_infor_upload.BMSinfor.EUTDR==Bms_cfg_infor.BMSinfor.EUTDR)
   item52->setTextColor(Qt::blue);
   else
   {parameter_error=1; item52->setTextColor(Qt::red); }
   item52->setFont(QFont("Microsoft Yahei", 10));
   item52->setTextAlignment(Qt::AlignCenter);
   ui->tableWidgetSystemPara->setItem(11,10,item52);//EUTDR  放电低温保护释放

   temp32 = (uchar)buffer[63];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[62];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[61];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[60];
   Bms_infor_upload.BMSinfor.SOCC = temp32;
   str = QString::number(temp32);
   QTableWidgetItem *item53 = new QTableWidgetItem;
   item53->setText(str);
   if(Bms_infor_upload.BMSinfor.SOCC==Bms_cfg_infor.BMSinfor.SOCC)
   item53->setTextColor(Qt::blue);
   else
   {parameter_error=1; item53->setTextColor(Qt::red); }
   item53->setFont(QFont("Microsoft Yahei", 10));
   item53->setTextAlignment(Qt::AlignCenter);
   ui->tableWidgetSystemPara->setItem(12,10,item53);//SOCC  软件充电过流

   temp32 = (uchar)buffer[67];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[66];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[65];
   temp32 <<= 8;
   temp32 |= (uchar)buffer[64];
   Bms_infor_upload.BMSinfor.SOCCD = temp32*1000/4;
   str = QString::number(temp32*1000/4);
   QTableWidgetItem *item54 = new QTableWidgetItem;
   item54->setText(str);
   if(Bms_infor_upload.BMSinfor.SOCCD==Bms_cfg_infor.BMSinfor.SOCCD)
   item54->setTextColor(Qt::blue);
   else
   {parameter_error=1; item54->setTextColor(Qt::red); }
   item54->setFont(QFont("Microsoft Yahei", 10));
   item54->setTextAlignment(Qt::AlignCenter);
   ui->tableWidgetSystemPara->setItem(13,10,item54);//SOCCD  软件充电过流


   }

void BMCMainWindow::AutoTest(void)
{
    int Testmod = ui->comboBoxTestMode->currentIndex();
    if(Testmod==0) return;

    // ===== BMS重启保护期处理 =====
    if (BmsRebooting)
    {
        BmsRebootingTick++;
        // 重启保护期为15秒（假设tick每2ms增加1，15000ms/2ms=7500）
        // 覆盖BMS重启时间(通常5-10秒) + 后续等待时间
        if (BmsRebootingTick >= 7500*4)//此处于2026.6.26临时*1.5改为*3，保护期约45秒
        {
            BmsRebooting = false;
            BmsRebootingTick = 0;
//            ui->textBrowserTestInfo->append("BMS重启保护期结束\n");
            ui->textBrowserTestInfo->append("测试完成，可以拔掉通讯线切入新的BMS板\n================================\n");
        }
    }

    switch (AutoTestStatus)
    {
     case 0:
        if(BMSConnectIsOk)
        {
            // ===== 修改：检测到连接时，先检查是否处于重启保护期 =====
            if (BmsRebooting)
            {
                // BMS正在重启，不认为是新保护板接入
                ui->textBrowserTestInfo->append("BMS重启中，忽略连接检测\n");
            }
            else if(Testmod==1&&AllowAutoTest)
            {
                AutoTestTick = 0;
                AutoTestStatus = 1;
            }
        }
        break;

     case 1:
        if(AutoTestTick>=100)
        {
            on_pushButtonStartTset_clicked();
            AllowAutoTest = 1;
            AutoTestStatus = 2;
            resetok = 0;
        }
        break;

     case 2:
        if(TestProcessEnd)
        {
         if(resetok)
         {
             AutoTestStatus = 3;
             AutoTestTick = 0;
         }
        }
        break;

     case 3:
        // ===== 修改：在等待期间也要检查BMS重启保护期 =====
        if (BmsRebooting)
        {
            // BMS重启中，重置等待计时，确保保护期结束后再等待10秒
            AutoTestTick = 0;
//            ui->textBrowserTestInfo->append("BMS重启中，暂停等待拔线检测\n");
        }
        else if(AutoTestTick>=1000)
        {
            AutoTestStatus = 4;
        }
        break;

    case 4:
        if(BMSConnectIsOk==0)
        {
            // ===== 修改：检测到断开时，先检查是否处于重启保护期 =====
            if (BmsRebooting)
            {
                // BMS正在重启，不认为是用户拔线
                ui->textBrowserTestInfo->append("BMS重启中，忽略断开检测\n");
            }
            else
            {
                AutoTestStatus = 0;
                ui->textBrowserTestInfo->append("重新连接可激活下一个.........\n");
            }
        }
        break;

    }
}


void BMCMainWindow::TestProcessTask(void)
{
    // 如果测试流程没有启动或已结束，则直接返回
//    if (TestProcessTaskState == 0 || TestProcessEnd == 1) {
//        return;
//    }
    // 如果测试流程没有启动或已结束，则直接返回
    if (TestProcessTaskState == 0) { // <-- 【修改】只检查这一项
        return;
    }

    // 【核心修改】
    // 注释掉这个全局错误检查。
    // 现在，即使 TestProcessError = 1，流程也会继续执行当前的 case，
    // 而不是立即跳转到 case 99。
    /*
    if (TestProcessError == 1 && TestProcessTaskState != 99 && TestProcessTaskState < 100) {
        TestProcessTaskState = 99; // 99作为错误状态
        TestProcessTick = 0;
    }
    */

    // 平滑延时的时间（单位：毫秒）
    const int smooth_delay_ms = 250;

    switch (TestProcessTaskState)
    {
        case 1: // 状态1: 初始化
            ui->textBrowserTestInfo->append("开始测试..............\n");
            // 初始化所有标志
            unlock_end = 0;
            zerocurcali_end = 0;
            timecali_end = 0;
            Curcali_end = 0;
            resetok = 0;

            // 隐藏所有状态指示灯
            ui->btnShow_UL->hide();
            ui->btnShow_ZV->hide();
            ui->btnShow_TV->hide();
            ui->btnShow_VC->hide();
            ui->btnShow_VerC->hide();
            ui->btnShow_VdC->hide();
            ui->btnShow_CurV->hide();
            ui->btnShow_CurC->hide();
            ui->btnShow_ParC->hide();

            //zgj 2025.12.25 start
            // 开始新测试时，隐藏 TotalC 按钮，防止显示上一次的结果
            ui->btnShow_TotalC->hide();
            //zgj 2025.12.25 end

            ui->label_testing->setPixmap(QPixmap(":/myimage/qrc/image/testing.png").scaled(70,70));

            // 检查是否需要预复位
            if (ui->radioButton_preTest->isChecked()) {
                ui->textBrowserTestInfo->append("--> 检测到预复位选项，正在执行复位...");
                SendResetEnable = 1; // 发送复位请求
                TestProcessTaskState = 10; // 【新】进入等待复位状态
            } else {
                TestProcessTaskState = 101; // 不需要复位，直接进入标准延时
            }
            TestProcessTick = 0;
            break;

        // --- 【新】步骤 1.5: 等待预复位完成 ---
        case 10:
            if (resetok) { // 等待 BaseInfoReadTask 设置 resetok = 1
                ui->textBrowserTestInfo->append("--> 预复位完成。");
                TestProcessTaskState = 101; // 复位成功，进入下一步延时
                TestProcessTick = 0;
            } else if (TestProcessTick > 5000) { // 给复位设置一个5秒的超时
                ui->textBrowserTestInfo->append("!!!!!! 预复位超时！");
                TestProcessError = 1;
                TestProcessTaskState = 101; // 即使超时也继续
                TestProcessTick = 0;
            }
            break;

        case 101: // 延时状态
            if (TestProcessTick >= smooth_delay_ms) {
                TestProcessTaskState = 2;
            }
            break;

        case 2:
            if (TestOption & UNLOCKFLAG) {
                AuthenticationEnable = 1; // 派发解锁指令
                TestProcessTaskState = 21; // 进入等待解锁状态
                TestProcessTick = 0;
            } else {
                TestProcessTaskState = 3; // 没勾选则直接跳到下一项
            }
            break;

        case 21: // 等待解锁完成
            if (unlock_end) {
                unlock_end = 0; // 收到成功标志，重置
                TestProcessTaskState = 22; // 进入强制延时
                TestProcessTick = 0;
            } else if (TestProcessTick > 4500) { // 约6秒超时 (3000 * 2ms)
                ui->textBrowserTestInfo->append("!!!!!! 解锁超时！");
                TestProcessError = 1;
                TestProcessTaskState = 22; // 即使超时也进入延时，防止影响下一项
                TestProcessTick = 0;
            }
            break;

        case 22: // 解锁后的独立物理延时 (300ms)
//            if (TestProcessTick >= smooth_delay_ms) {
            // 改为 25 (代表50ms)，原先是 smooth_delay_ms
            if (TestProcessTick >= 25) {
                TestProcessTaskState = 3; // 延时结束，进入第二项
            }
            break;

        // ==========================================
        // 【第二项】：零电流校准
        // ==========================================
        case 3:
            if (TestOption & ZEROCALIFLAG) {
                on_pushButtonZeroCur_clicked(); // 触发零电流校准
                TestProcessTaskState = 31;
                TestProcessTick = 0;
            } else {
                TestProcessTaskState = 35; // 没勾选则跳到下一项
            }
            break;

        case 31: // 等待零电流校准完成
            if (zerocurcali_end) {
                zerocurcali_end = 0;
                TestProcessTaskState = 32;
                TestProcessTick = 0;
            } else if (TestProcessTick > 3000) {
                ui->textBrowserTestInfo->append("!!!!!! 零电流校准超时！");
                TestProcessError = 1;
                TestProcessTaskState = 32;
                TestProcessTick = 0;
            }
            break;

        case 32: // 零电流校准后的独立物理延时 (300ms)
            if (TestProcessTick >= smooth_delay_ms) {
                TestProcessTaskState = 35; // 延时结束，进入第三项
            }
            break;

        // ==========================================
        // 【第三项】：时间校准
        // ==========================================
        case 35:
            if (TestOption & TIMECALIFLAG) {
                on_pushButtonTimeCali_clicked(); // 触发时间校准
                TestProcessTaskState = 36;
                TestProcessTick = 0;
            } else {
                TestProcessTaskState = 102; // 没勾选则直接进入过渡延时
            }
            break;

        case 36: // 等待时间校准完成
            if (timecali_end) {
                timecali_end = 0;
                TestProcessTaskState = 102;
                TestProcessTick = 0;
            } else if (TestProcessTick > 3000) {
                ui->textBrowserTestInfo->append("!!!!!! 时间校准超时！");
                TestProcessError = 1;
                TestProcessTaskState = 102;
                TestProcessTick = 0;
            }
            break;

        // ==========================================
        // 【过渡延时】：准备进入后续的同步检测项
        // ==========================================
        case 102: // 独立的 300ms 缓冲延时
            if (TestProcessTick >= smooth_delay_ms) {
                TestProcessTaskState = 4; // 无缝对接原有的 case 4 (同步检测任务)
            }
            break;

        // ==========================================
        // 【第四项】：电压检测 (本地同步判断)
        // ==========================================
        case 4:
            if (TestOption & VOLTDETECTFLAG) {
                on_pushButtonCurCali_2_clicked(); // 触发电压检测
                TestProcessTaskState = 41;        // 进入延时缓冲
                TestProcessTick = 0;
            } else {
                TestProcessTaskState = 42;        // 没勾选则跳过，测下一项
            }
            break;

        case 41: // 电压检测后的独立物理延时 (50ms)
            if (TestProcessTick >= 50) {
                TestProcessTaskState = 42;
            }
            break;

        // ==========================================
        // 【第五项】：软件版本检测 (本地同步判断)
        // ==========================================
        case 42:
            if (TestOption & VESIONDETECTFLAG) {
                on_pushButtonCurCali_3_clicked(); // 触发版本检测
                TestProcessTaskState = 43;        // 进入延时缓冲
                TestProcessTick = 0;
            } else {
                TestProcessTaskState = 44;        // 没勾选则跳过，测下一项
            }
            break;

        case 43: // 版本检测后的独立物理延时 (50ms)
            if (TestProcessTick >= 50) {
                TestProcessTaskState = 44;
            }
            break;

        // ==========================================
        // 【第六项】：压差检测 (本地同步判断)
        // ==========================================
        case 44:
            if (TestOption & VOLTDIFFFLAG) {
                on_pushButtonVoltDiffDetect_clicked(); // 触发压差检测
                TestProcessTaskState = 103;            // 检测完毕，进入过渡延时
                TestProcessTick = 0;
            } else {
                TestProcessTaskState = 103;            // 没勾选则直接进入过渡延时
            }
            break;

        // ==========================================
        // 【过渡延时】：准备进入后续的电流校准项 (case 5)
        // ==========================================
        case 103: // 独立的 50ms 缓冲延时
            if (TestProcessTick >= 50) {
                TestProcessTaskState = 5; // 无缝对接原有的 case 5 (异步电流校准任务)
            }
            break;

        case 5: // 状态5: 启动电流校准 (异步)
             if (TestOption & CURCALIFLAG) {
//                ui->textBrowserTestInfo->append("--> 步骤5: 电流校准...");
                on_pushButtonCurCali_clicked();
                TestProcessTaskState = 6;
                TestProcessTick = 0;
            } else {
                TestProcessTaskState = 7;
            }
            break;

        case 6: // 状态6: 等待电流校准完成 (无限等待)
            if (Curcali_end) {
                Curcali_end = 0;
//                ui->textBrowserTestInfo->append("--> 电流校准完成。");
                TestProcessTaskState = 104;
                TestProcessTick = 0;
            }
            break;

        case 104: // 延时状态
            if (TestProcessTick >= smooth_delay_ms) {
                TestProcessTaskState = 7;
            }
            break;

        case 7: // 状态 7: 执行电流检测 (同步)
            if (TestOption & CURDETECTFLAG) {
                on_pushButtonCurDetect_clicked();
            }
            TestProcessTaskState = 105;
            TestProcessTick = 0;
            break;

        case 105: // 延时状态
            if (TestProcessTick >= smooth_delay_ms) {
                TestProcessTaskState = 8;
            }
            break;

        case 8: // 状态 8: 启动参数检测 (异步)
            if (TestOption & PARADETECTFLAG) {
//                ui->textBrowserTestInfo->append("--> 步骤6: 参数检测...");
                on_pushButtonParameterDetect_clicked();
                TestProcessTaskState = 9;
                TestProcessTick = 0;
            } else {
                TestProcessTaskState = 100;
            }
            break;

        case 9: // 状态 9: 等待参数检测完成
            if (paraDetect_end) { // <-- 【修改】检查新标志位
                TestProcessTaskState = 100; // 检测完成，进入结束流程
            } else if (TestProcessTick > 5000) {
                ui->textBrowserTestInfo->append("!!!!!! 参数检测超时！");
                TestProcessError = 1;
                TestProcessTaskState = 100; // 超时，也进入结束流程
            }
            break;

        case 99: // 状态 99: 统一错误处理 (现在基本不会被调用了)
            ui->textBrowserTestInfo->append("\n!!!!!! 测试因错误中断 !!!!!!");
            TestProcessTaskState = 100;
            break;

        case 100: // 状态 100: 最终总结
            qDebug() << "到达case100";
            if (TestProcessError) {
                qDebug() << "到达case100if";
//                ui->textBrowserTestInfo->append("\n======== 测试完成，但存在错误项！ ========");
//                ui->label_testing->setPixmap(QPixmap(":/myimage/qrc/image/testerror.png").scaled(70,70));
            } else {
                qDebug() << "到达case100else";
//                ui->textBrowserTestInfo->append("\n======== 测试流程成功结束 ========");
//                ui->label_testing->setPixmap(QPixmap(":/myimage/qrc/image/testend.png").scaled(70,70));

                // 自动测试成功结束后，显示 TotalC 为绿色 OK
                ui->btnShow_TotalC->show();
                ui->btnShow_TotalC->setText("OK");
                ui->btnShow_TotalC->setStyleSheet("color:#ffffff;font-size:10px;background:green");
            }

            TestProcessTaskState = 200;
            break;

        // --- 测试后清理步骤 ---

        case 200: // 状态 200: 发送“恢复”指令
//            ui->textBrowserTestInfo->append("--> 发送恢复指令...");转到发送帧阶段
//            StartReadBase = 0;
            SendResumeEnable = 1;
            TestProcessTaskState = 201;
            TestProcessTick = 0;
            break;

        case 201: // 状态 201: 发送"复位"指令
            if (TestProcessTick >= 500)
            {
                SendResetEnable = 1;

                // ===== 关键修改：设置BMS重启保护期 =====
                BmsRebooting = true;
                BmsRebootingTick = 0;
//                ui->textBrowserTestInfo->append("--> 发送复位指令，启动BMS重启保护期\n");

                TestProcessTaskState = 202;
                TestProcessTick = 0;
            }
            break;

        case 202: // 状态 202: 结束流程
            if (TestProcessTick >= 500)
            {
                //三秒延时，用于等待恢复、复位执行
                QTimer::singleShot(3000, this, [=]() {
//                        ui->textBrowserTestInfo->append("全部测试结束！OK！\n");
                        ui->label_testing->setPixmap(QPixmap(":/myimage/qrc/image/testend.png").scaled(70,70));

                });
                //调试
//                ui->textBrowserTestInfo->append("全部测试结束！OK！已经发送TestProcessEnd = 1;\n");
                ui->textBrowserTestInfo->append("全部测试结束！\n");
                TestProcessEnd = 1;
                TestProcessTaskState = 0;
            }
            break;
    }
}
//void BMCMainWindow::TestProcessTask(void)
//{
//    // 如果测试流程没有启动或已结束，则直接返回
//    if (TestProcessTaskState == 0 || TestProcessEnd == 1) {
//        return;
//    }

//    // 在任何步骤中检测到错误，立即跳转到错误处理状态
////    if (TestProcessError == 1 && TestProcessTaskState != 99 && TestProcessTaskState < 100) {
////        // 【注意】只有在测试流程中才跳转，在结束流程中不再跳转
////        TestProcessTaskState = 99; // 99作为错误状态
////        TestProcessTick = 0;
////    }

//    // 平滑延时的时间（单位：毫秒）
//    const int smooth_delay_ms = 200;

//    switch (TestProcessTaskState)
//    {
//        case 1: // 状态1: 初始化
//            ui->textBrowserTestInfo->append("======== 开始测试流程 ========");
//            // 初始化所有标志
//            unlock_end = 0;
//            zerocurcali_end = 0;
//            timecali_end = 0;
//            Curcali_end = 0;

//            // 隐藏所有状态指示灯
//            ui->btnShow_UL->hide();
//            ui->btnShow_ZV->hide();
//            ui->btnShow_TV->hide();
//            ui->btnShow_VC->hide();
//            ui->btnShow_VerC->hide();
//            ui->btnShow_VdC->hide();
//            ui->btnShow_CurV->hide();
//            ui->btnShow_CurC->hide();
//            ui->btnShow_ParC->hide();

//            ui->label_testing->setPixmap(QPixmap(":/myimage/qrc/image/testing.png").scaled(70,70));

//            TestProcessTaskState = 101;
//            TestProcessTick = 0;
//            break;

//        case 101: // 延时状态
//            if (TestProcessTick >= smooth_delay_ms) {
//                TestProcessTaskState = 2;
//            }
//            break;

//        case 2: // 状态2: 派发初始异步任务
//            ui->textBrowserTestInfo->append("--> 派发初始任务(解锁、零电流、时间校准)...");
//            if (TestOption & UNLOCKFLAG) { AuthenticationEnable = 1; }
//            if (TestOption & ZEROCALIFLAG) { on_pushButtonZeroCur_clicked(); }
//            if (TestOption & TIMECALIFLAG) { on_pushButtonTimeCali_clicked(); }
//            TestProcessTaskState = 3;
//            TestProcessTick = 0;
//            break;

//        case 3: // 状态3: 等待初始异步任务完成
//        {
//            bool all_done = true;
//            if ((TestOption & UNLOCKFLAG) && !unlock_end) all_done = false;
//            if ((TestOption & ZEROCALIFLAG) && !zerocurcali_end) all_done = false;
//            if ((TestOption & TIMECALIFLAG) && !timecali_end) all_done = false;

//            if (all_done) {
//                ui->textBrowserTestInfo->append("--> 初始任务完成。");
//                TestProcessTaskState = 102;
//                TestProcessTick = 0;
//            } else if (TestProcessTick > 7000) {
//                ui->textBrowserTestInfo->append("!!!!!! 等待初始任务超时！请检查连接。");
//                TestProcessError = 1;
//                TestProcessTaskState = 102;
//                TestProcessTick = 0;
//            }
//            break;
//        }

//        case 102: // 延时状态
//            if (TestProcessTick >= smooth_delay_ms) {
//                TestProcessTaskState = 4;
//            }
//            break;

//        case 4: // 状态4: 执行同步检测任务
//            ui->textBrowserTestInfo->append("--> 执行同步检测项...");
//            if (TestOption & VOLTDETECTFLAG) { on_pushButtonCurCali_2_clicked(); }
//            if (TestOption & VESIONDETECTFLAG) { on_pushButtonCurCali_3_clicked(); }
//            if (TestOption & VOLTDIFFFLAG) { on_pushButtonVoltDiffDetect_clicked(); }
//            TestProcessTaskState = 103;
//            TestProcessTick = 0;
//            break;

//        case 103: // 延时状态
//            if (TestProcessTick >= smooth_delay_ms) {
//                TestProcessTaskState = 5;
//            }
//            break;

//        case 5: // 状态5: 启动电流校准 (异步)
//             if (TestOption & CURCALIFLAG) {
//                ui->textBrowserTestInfo->append("--> 步骤5: 电流校准...");
//                on_pushButtonCurCali_clicked();
//                TestProcessTaskState = 6;
//                TestProcessTick = 0;
//            } else {
//                TestProcessTaskState = 7;
//            }
//            break;

//        case 6: // 状态6: 等待电流校准完成 (无限等待)
//            if (Curcali_end) {
//                Curcali_end = 0;
//                ui->textBrowserTestInfo->append("--> 电流校准完成。");
//                TestProcessTaskState = 104;
//                TestProcessTick = 0;
//            }
//            break;

//        case 104: // 延时状态
//            if (TestProcessTick >= smooth_delay_ms) {
//                TestProcessTaskState = 7;
//            }
//            break;

//        case 7: // 状态 7: 执行电流检测 (同步)
//            if (TestOption & CURDETECTFLAG) {
//                on_pushButtonCurDetect_clicked();
//            }
//            TestProcessTaskState = 105;
//            TestProcessTick = 0;
//            break;

//        case 105: // 延时状态
//            if (TestProcessTick >= smooth_delay_ms) {
//                TestProcessTaskState = 8;
//            }
//            break;

//        case 8: // 状态 8: 启动参数检测 (异步)
//            if (TestOption & PARADETECTFLAG) {
//                ui->textBrowserTestInfo->append("--> 步骤6: 参数检测...");
//                on_pushButtonParameterDetect_clicked();
//                TestProcessTaskState = 9;
//                TestProcessTick = 0;
//            } else {
//                TestProcessTaskState = 100; // 不检测参数，直接跳到结束流程
//            }
//            break;

//        case 9: // 状态 9: 等待参数检测完成
//            qDebug() << "到达case9。当前 TestProcessTick:" << TestProcessTick << ", TestProcessEnd:" << TestProcessEnd;
//            if (TestProcessEnd) {
//                qDebug() << "到达case9if";
//                TestProcessTaskState = 100; // 检测完成，进入结束流程
//            } else if (TestProcessTick > 5000) {
//                qDebug() << "到达case9else";
//                ui->textBrowserTestInfo->append("!!!!!! 参数检测超时！");
//                TestProcessError = 1;
//                TestProcessTaskState = 100; // 超时，也进入结束流程
//            }
//            break;

//        case 99: // 【新】状态 99: 统一错误处理
//            // 只有在测试中途出错才会跳转到这里
//            ui->textBrowserTestInfo->append("\n!!!!!! 测试因错误中断 !!!!!!");
//            // 不设置 TestProcessEnd = 1，直接跳转到最终总结
//            TestProcessTaskState = 100;
//            break;

//        case 100: // 状态 100: 最终总结
//            qDebug() << "到达case100";
//            if (TestProcessError) {
//                qDebug() << "到达case100if";
//                ui->textBrowserTestInfo->append("\n======== 测试完成，但存在错误项！ ========");
//                ui->label_testing->setPixmap(QPixmap(":/myimage/qrc/image/testerror.png").scaled(70,70));
//            } else {
//                qDebug() << "到达case100else";
//                ui->textBrowserTestInfo->append("\n======== 测试流程成功结束 ========");
//                ui->label_testing->setPixmap(QPixmap(":/myimage/qrc/image/testend.png").scaled(70,70));
//            }

//            // 【新】进入恢复和复位流程
//            TestProcessTaskState = 200;
//            break;

//        // ---【新】测试后清理步骤 ---

//        case 200: // 状态 200: 发送“恢复”指令
//            ui->textBrowserTestInfo->append("--> 发送恢复指令...");
//            SendResumeEnable = 1; // 触发恢复
//            TestProcessTaskState = 201;
//            TestProcessTick = 0;
//            break;

//        case 201: // 状态 201: 发送“复位”指令 (在恢复后延时片刻)
//            if (TestProcessTick >= 500) // 延时 500ms
//            {
//                ui->textBrowserTestInfo->append("--> 发送复位指令...");
//                SendResetEnable = 1; // 触发复位
//                TestProcessTaskState = 202;
//                TestProcessTick = 0;
//            }
//            break;

//        case 202: // 状态 202: 最后的小延时，并结束流程
//            if (TestProcessTick >= 500) // 再延时 500ms，确保指令发出
//            {
//                ui->textBrowserTestInfo->append("--> 清理完成，请断开连接测试下一块板。");
//                TestProcessEnd = 1; // 标记本轮测试结束
//                TestProcessTaskState = 0; // 重置状态机，为下一次测试做准备
//            }
//            break;
//    }
//}
//void BMCMainWindow::TestProcessTask(void)
//{
//    // 如果测试流程没有启动或已结束，则直接返回
//    if (TestProcessTaskState == 0 || TestProcessEnd == 1) {
//        return;
//    }

//    // 【修改】移除了顶部的全局错误中断检查
//    // if (TestProcessError == 1 && TestProcessTaskState != 99) { ... }

//    // 平滑延时的时间（单位：毫秒）
//    const int smooth_delay_ms = 200;

//    switch (TestProcessTaskState)
//    {
//        case 1: // 状态1: 初始化
//            ui->textBrowserTestInfo->append("开始测试..............");
//            // 初始化所有标志
//            unlock_end = 0;
//            zerocurcali_end = 0;
//            timecali_end = 0;
//            Curcali_end = 0;

//            // 隐藏所有状态指示灯
//            ui->btnShow_UL->hide();
//            ui->btnShow_ZV->hide();
//            ui->btnShow_TV->hide();
//            ui->btnShow_VC->hide();
//            ui->btnShow_VerC->hide();
//            ui->btnShow_VdC->hide();
//            ui->btnShow_CurV->hide();
//            ui->btnShow_CurC->hide();
//            ui->btnShow_ParC->hide();

//            ui->label_testing->setPixmap(QPixmap(":/myimage/qrc/image/testing.png").scaled(70,70));

//            TestProcessTaskState = 101;
//            TestProcessTick = 0;
//            break;

//        case 101: // 延时状态
//            if (TestProcessTick >= smooth_delay_ms) {
//                TestProcessTaskState = 2;
//            }
//            break;

//        case 2: // 状态2: 派发初始异步任务
////            ui->textBrowserTestInfo->append("--> 派发初始任务(解锁、零电流、时间校准)...");
//            if (TestOption & UNLOCKFLAG) { AuthenticationEnable = 1; }
//            if (TestOption & ZEROCALIFLAG) { on_pushButtonZeroCur_clicked(); }
//            if (TestOption & TIMECALIFLAG) { on_pushButtonTimeCali_clicked(); }
//            TestProcessTaskState = 3;
//            TestProcessTick = 0;
//            break;

//        case 3: // 状态3: 等待初始异步任务完成
//        {
//            bool all_done = true;
//            if ((TestOption & UNLOCKFLAG) && !unlock_end) all_done = false;
//            if ((TestOption & ZEROCALIFLAG) && !zerocurcali_end) all_done = false;
//            if ((TestOption & TIMECALIFLAG) && !timecali_end) all_done = false;

//            if (all_done) {
////                ui->textBrowserTestInfo->append("--> 初始任务完成。");
//                TestProcessTaskState = 102;
//                TestProcessTick = 0;
//            } else if (TestProcessTick > 7000) {
//                ui->textBrowserTestInfo->append("!!!!!! 等待初始任务超时！请检查连接。");
//                TestProcessError = 1; // 【修改】只记录错误
//                TestProcessTaskState = 102; // 【修改】仍然继续到下一步
//                TestProcessTick = 0;
//            }
//            break;
//        }

//        case 102: // 延时状态
//            if (TestProcessTick >= smooth_delay_ms) {
//                TestProcessTaskState = 4;
//            }
//            break;

//        case 4: // 状态4: 执行同步检测任务
////            ui->textBrowserTestInfo->append("--> 执行同步检测项...");
//            if (TestOption & VOLTDETECTFLAG) { on_pushButtonCurCali_2_clicked(); }//电压检测
//            if (TestOption & VESIONDETECTFLAG) { on_pushButtonCurCali_3_clicked(); }//版本检测
//            if (TestOption & VOLTDIFFFLAG) { on_pushButtonVoltDiffDetect_clicked(); }//压差检测
//            TestProcessTaskState = 103;
//            TestProcessTick = 0;
//            break;

//        case 103: // 延时状态
//            if (TestProcessTick >= smooth_delay_ms) {
//                TestProcessTaskState = 5;
//            }
//            break;

//        case 5: // 状态5: 启动电流校准 (异步)
//             if (TestOption & CURCALIFLAG) {
////                ui->textBrowserTestInfo->append("--> 步骤5: 电流校准...");
//                on_pushButtonCurCali_clicked();
//                TestProcessTaskState = 6;
//                TestProcessTick = 0;
//            } else {
//                TestProcessTaskState = 7;
//            }
//            break;

//        case 6: // 状态6: 等待电流校准完成
//            if (Curcali_end) {
//                Curcali_end = 0;
////                ui->textBrowserTestInfo->append("--> 电流校准完成。");
//                TestProcessTaskState = 104;
//                TestProcessTick = 0;
//            }
//            //超时功能
////            else if (TestProcessTick > 10000) {
////                ui->textBrowserTestInfo->append("!!!!!! 电流校准超时！");
////                TestProcessError = 1; // 【修改】只记录错误
////                TestProcessTaskState = 104; // 【修改】仍然继续到下一步
////                TestProcessTick = 0;
////            }
//            break;

//        case 104: // 延时状态
//            if (TestProcessTick >= smooth_delay_ms) {
//                TestProcessTaskState = 7;
//            }
//            break;

//        case 7: // 状态 7: 执行电流检测 (同步)
//            if (TestOption & CURDETECTFLAG) {
//                on_pushButtonCurDetect_clicked();
//            }
//            TestProcessTaskState = 105;
//            TestProcessTick = 0;
//            break;

//        case 105: // 延时状态
//            if (TestProcessTick >= smooth_delay_ms) {
//                TestProcessTaskState = 8;
//            }
//            break;

//        case 8: // 状态 8: 启动参数检测 (异步)
//            if (TestOption & PARADETECTFLAG) {
////                ui->textBrowserTestInfo->append("--> 步骤6: 参数检测...");
//                on_pushButtonParameterDetect_clicked();
//                TestProcessTaskState = 9;
//                TestProcessTick = 0;
//            } else {
//                TestProcessTaskState = 100;
//            }
//            break;

//        case 9: // 状态 9: 等待参数检测完成

//            qDebug() << "到达case9。当前 TestProcessTick:" << TestProcessTick << ", TestProcessEnd:" << TestProcessEnd; // <-- 修改此行，立即打印状态
//            if (TestProcessEnd) {
//                qDebug() << "到达case9if";
//                 TestProcessTaskState = 100;
//            } else if (TestProcessTick > 5000) {
//                qDebug() << "到达case9else";
//                 ui->textBrowserTestInfo->append("!!!!!! 参数检测超时！");
//                 TestProcessError = 1; // 【修改】只记录错误
//                 TestProcessTaskState = 100; // 【修改】超时后直接进入最终总结状态
//            }
//            break;

//        // 【修改】移除了 case 99

//        case 100: // 【修改】统一的最终结束状态
//            qDebug() << "到达case100";
//            if (TestProcessError) {
//                qDebug() << "到达case100if";
//                // 如果在整个流程中，TestProcessError 被置为1过
//                ui->textBrowserTestInfo->append("\n======== 测试完成，但存在错误项！ ========");
//                ui->label_testing->setPixmap(QPixmap(":/myimage/qrc/image/testerror.png").scaled(70,70));
//            } else {
//                qDebug() << "到达case100else";
//                // 如果整个流程没有任何错误
//                ui->textBrowserTestInfo->append("\n======== 测试流程成功结束 ========");
//                ui->label_testing->setPixmap(QPixmap(":/myimage/qrc/image/testend.png").scaled(70,70));
//            }

//            TestProcessEnd = 1; // 标记本轮测试结束
//            TestProcessTaskState = 0; // 重置状态机，为下一次测试做准备
//            break;
//    }
//}
//void BMCMainWindow::TestProcessTask(void)
//{
//    // 如果测试流程没有启动或已结束，则直接返回
//    if (TestProcessTaskState == 0 || TestProcessEnd == 1) {
//        return;
//    }

//    // 在任何步骤中检测到错误，立即跳转到错误处理状态
//    if (TestProcessError == 1 && TestProcessTaskState != 99) {
//        TestProcessTaskState = 99; // 99作为错误状态
//        TestProcessTick = 0;
//    }

//    // 平滑延时的时间（单位：毫秒），您可以根据需要调整
//    const int smooth_delay_ms = 500; // 0.5秒

//    switch (TestProcessTaskState)
//    {
//        case 1: // 状态1: 初始化
//            ui->textBrowserTestInfo->append("======== 开始测试流程 ========");
//            // 初始化所有步骤的完成标志
//            unlock_end = 0;
//            zerocurcali_end = 0;
//            timecali_end = 0;
//            Curcali_end = 0;

//            // 隐藏所有状态指示灯
//            ui->btnShow_UL->hide();
//            ui->btnShow_ZV->hide();
//            ui->btnShow_TV->hide();
//            ui->btnShow_VC->hide();
//            ui->btnShow_VerC->hide();
//            ui->btnShow_VdC->hide();
//            ui->btnShow_CurV->hide();
//            ui->btnShow_CurC->hide();
//            ui->btnShow_ParC->hide();

//            ui->label_testing->setPixmap(QPixmap(":/myimage/qrc/image/testing.png").scaled(70,70));

//            TestProcessTaskState = 101; // 进入第一个延时状态
//            TestProcessTick = 0;
//            break;

//        // --- 延时状态 1: 初始化后延时 ---
//        case 101:
//            if (TestProcessTick >= smooth_delay_ms) {
//                TestProcessTaskState = 2; // 延时结束，进入批量派发
//            }
//            break;

//        case 2: // 状态2: 同时派发前三个异步任务
//            ui->textBrowserTestInfo->append("--> 派发初始任务(解锁、零电流、时间校准)...");
//            if (TestOption & UNLOCKFLAG) { AuthenticationEnable = 1; }
//            if (TestOption & ZEROCALIFLAG) { on_pushButtonZeroCur_clicked(); }
//            if (TestOption & TIMECALIFLAG) { on_pushButtonTimeCali_clicked(); }
//            TestProcessTaskState = 3; // 进入等待状态
//            TestProcessTick = 0;
//            break;

//        case 3: // 状态3: 等待前三个异步任务全部完成
//        {
//            bool all_done = true;
//            if ((TestOption & UNLOCKFLAG) && !unlock_end) all_done = false;
//            if ((TestOption & ZEROCALIFLAG) && !zerocurcali_end) all_done = false;
//            if ((TestOption & TIMECALIFLAG) && !timecali_end) all_done = false;

//            if (all_done) {
//                ui->textBrowserTestInfo->append("--> 初始任务完成。");
//                TestProcessTaskState = 102; // 全部完成，进入第二个延时状态
//                TestProcessTick = 0;
//            } else if (TestProcessTick > 7000) {
//                ui->textBrowserTestInfo->append("等待初始任务超时！请检查连接。");
//                TestProcessError = 1;
//            }
//            break;
//        }

//        // --- 延时状态 2: 初始任务完成后延时 ---
//        case 102:
//            if (TestProcessTick >= smooth_delay_ms) {
//                TestProcessTaskState = 4; // 延时结束，执行同步检测
//            }
//            break;

//        case 4: // 状态4: 执行除电流检测外的同步任务
//            ui->textBrowserTestInfo->append("--> 执行同步检测项...");
//            if (TestOption & VOLTDETECTFLAG) { on_pushButtonCurCali_2_clicked(); }
//            if (TestOption & VESIONDETECTFLAG) { on_pushButtonCurCali_3_clicked(); }
//            if (TestOption & VOLTDIFFFLAG) { on_pushButtonVoltDiffDetect_clicked(); }
//            // 注意：电流检测已移出此步骤
//            TestProcessTaskState = 103; // 完成后，进入延时
//            TestProcessTick = 0;
//            break;

//        // --- 延时状态 3: 同步任务后延时 ---
//        case 103:
//            if (TestProcessTick >= smooth_delay_ms) {
//                TestProcessTaskState = 5; // 延时结束，准备电流校准
//            }
//            break;

//        case 5: // 状态5: 启动电流校准 (异步)
//             if (TestOption & CURCALIFLAG) {
//                ui->textBrowserTestInfo->append("--> 步骤5: 电流校准...");
//                on_pushButtonCurCali_clicked();
//                TestProcessTaskState = 6; // 切换到等待状态
//                TestProcessTick = 0;
//            } else {
//                TestProcessTaskState = 7; // 如果不校准，直接跳过到电流检测
//            }
//            break;

//        case 6: // 状态6: 等待电流校准完成
//            if (Curcali_end) {
//                Curcali_end = 0;
//                ui->textBrowserTestInfo->append("--> 电流校准完成。");
//                TestProcessTaskState = 104; // 校准完成，进入延时
//                TestProcessTick = 0;
//            } else if (TestProcessTick > 10000) {
//                ui->textBrowserTestInfo->append("电流校准超时！");
//                TestProcessError = 1;
//            }
//            break;

//        // --- 延时状态 4: 电流校准后延时 ---
//        case 104:
//            if (TestProcessTick >= smooth_delay_ms) {
//                TestProcessTaskState = 7; // 延时结束，准备电流检测
//            }
//            break;

//        // ---【新】步骤 7: 执行电流检测 (同步) ---
//        case 7:
//            if (TestOption & CURDETECTFLAG) {
//                on_pushButtonCurDetect_clicked();
//            }
//            TestProcessTaskState = 105; // 完成后进入延时
//            TestProcessTick = 0;
//            break;

//        // ---【新】延时状态 5: 电流检测后延时 ---
//        case 105:
//            if (TestProcessTick >= smooth_delay_ms) {
//                TestProcessTaskState = 8; // 延时结束，准备参数检测
//            }
//            break;

//        case 8: // 状态8 (原7): 启动参数检测 (异步, 最后一步)
//            if (TestOption & PARADETECTFLAG) {
//                ui->textBrowserTestInfo->append("--> 步骤6: 参数检测...");
//                on_pushButtonParameterDetect_clicked();
//                TestProcessTaskState = 9; // 进入等待
//                TestProcessTick = 0;
//            } else {
//                TestProcessTaskState = 100; // 如果不检测，直接成功结束
//            }
//            break;

//        case 9: // 状态9 (原8): 等待参数检测完成
//            if (TestProcessEnd) {
//                 TestProcessTaskState = 100;
//            } else if (TestProcessTick > 5000) {
//                 ui->textBrowserTestInfo->append("参数检测超时！");
//                 TestProcessError = 1;
//            }
//            break;

//        case 99: // 状态99: 错误处理
//            ui->textBrowserTestInfo->append("!!!!!! 测试因错误中断 !!!!!!");
//            ui->label_testing->setPixmap(QPixmap(":/myimage/qrc/image/testerror.png").scaled(70,70));
//            TestProcessEnd = 1;
//            TestProcessTaskState = 0;
//            break;

//        case 100: // 状态100: 成功结束
//            ui->textBrowserTestInfo->append("======== 测试流程成功结束 ========");
//            ui->label_testing->setPixmap(QPixmap(":/myimage/qrc/image/testend.png").scaled(70,70));
//            TestProcessEnd = 1;
//            TestProcessTaskState = 0;
//            break;
//    }
//}
//void BMCMainWindow::TestProcessTask(void)
//{
//    // 如果测试流程没有启动或已结束，则直接返回
//    if (TestProcessTaskState == 0 || TestProcessEnd == 1) {
//        return;
//    }

//    // 在任何步骤中检测到错误，立即跳转到错误处理状态
//    if (TestProcessError == 1 && TestProcessTaskState != 99) {
//        TestProcessTaskState = 99; // 99作为错误状态
//        TestProcessTick = 0;
//    }

//    switch (TestProcessTaskState)
//    {
//        case 1: // 状态1: 开始测试流程的初始化
//            ui->textBrowserTestInfo->append("开始测试..............");
//            // 初始化所有步骤的完成标志
//            unlock_end = 0;
//            zerocurcali_end = 0;
//            timecali_end = 0;
//            voltdetect_end = 0;
//            versiondetect_end = 0;
//            voltdiffdetect_end = 0;
//            Curcali_end = 0;
//            curdetect_end = 0;

//            ui->btnShow_UL->hide();
//            ui->btnShow_ZV->hide();
//            ui->btnShow_TV->hide();
//            ui->btnShow_VC->hide();
//            ui->btnShow_VerC->hide();
//            ui->btnShow_VdC->hide();
//            ui->btnShow_CurV->hide();
//            ui->btnShow_CurC->hide();
//            ui->btnShow_ParC->hide();

//            ui->label_testing->setPixmap(QPixmap(":/myimage/qrc/image/testing.png").scaled(70,70));

//            TestProcessTaskState = 2; // 进入批量派发状态
//            TestProcessTick = 0;
//            break;

//        // --- 步骤 2: 同时派发前三个异步任务 ---
//        case 2:
////            ui->textBrowserTestInfo->append("--> 正在同时派发解锁、零电流校准、时间校准任务...");
//            if (TestOption & UNLOCKFLAG) {
////                AuthenticationEnable = 1;
//                on_pushButtonUnlock_clicked();
//            }
//            if (TestOption & ZEROCALIFLAG) {
//                on_pushButtonZeroCur_clicked(); // 内部会设置 ZeroCurCaliEnable
//            }
//            if (TestOption & TIMECALIFLAG) {
//                on_pushButtonTimeCali_clicked(); // 内部会设置 RTCCaliEnable
//            }
//            TestProcessTaskState = 3; // 进入等待状态
//            TestProcessTick = 0;
//            break;

//        // --- 步骤 3: 等待前三个异步任务全部完成 ---
//        case 3:
//        {
//            // 使用一个布尔变量来判断是否所有已勾选的任务都已完成
//            bool all_initial_tasks_done = true;
//            if ((TestOption & UNLOCKFLAG) && !unlock_end) {
//                all_initial_tasks_done = false;
//            }
//            if ((TestOption & ZEROCALIFLAG) && !zerocurcali_end) {
//                all_initial_tasks_done = false;
//            }
//            if ((TestOption & TIMECALIFLAG) && !timecali_end) {
//                all_initial_tasks_done = false;
//            }

//            if (all_initial_tasks_done) {
////                ui->textBrowserTestInfo->append("--> 初始异步任务全部完成。");
//                TestProcessTaskState = 4; // 所有任务完成，进入后续步骤
//            } else if (TestProcessTick > 7000) { // 设置一个更长的总超时（7秒），等待所有响应
//                ui->textBrowserTestInfo->append("等待初始任务超时！请检查连接。");
//                TestProcessError = 1; // 触发错误状态
//            }
//            break;
//        }

//        // --- 步骤 4: 执行所有同步检测任务 ---
//        case 4:
////            ui->textBrowserTestInfo->append("--> 开始执行同步检测项...");
//            if (TestOption & VOLTDETECTFLAG) {
//                on_pushButtonCurCali_2_clicked();
//            }
//            if (TestOption & VESIONDETECTFLAG) {
//                on_pushButtonCurCali_3_clicked();
//            }
//            if (TestOption & VOLTDIFFFLAG) {
//                on_pushButtonVoltDiffDetect_clicked();
//            }
//             if (TestOption & CURCALIFLAG) {
//                 ui->textBrowserTestInfo->append("--> 步骤5: 电流校准...");
//                on_pushButtonCurCali_clicked();
//            }
//             if (TestOption & CURDETECTFLAG) {
//                on_pushButtonCurDetect_clicked();
//            }
//            TestProcessTaskState = 6; // 直接进入下一个异步任务
//            break;

//        // --- 步骤 5: 电流校准 (异步) ---
//        case 5:
//             if (TestOption & CURCALIFLAG) {
////                ui->textBrowserTestInfo->append("--> 步骤5: 电流校准...");
//                on_pushButtonCurCali_clicked();
//                TestProcessTaskState = 6;
//                TestProcessTick = 0;
//            } else {
//                TestProcessTaskState = 7;
//            }
//            break;
//        case 6:
//        qDebug() << "状态机正在 case 6 等待 Curcali_end...";
//            if (Curcali_end) {
//                Curcali_end = 0;
//                TestProcessTaskState = 7;
//            } else if (TestProcessTick > 10000) { // 电流校准需要等待负载仪，延时较长
//                ui->textBrowserTestInfo->append("电流校准超时！");
//                TestProcessError = 1;
//            }
//            break;

//        // --- 步骤 7: 参数检测 (异步, 最后一步) ---
//        case 7:
//            if (TestOption & PARADETECTFLAG) {

//                ui->textBrowserTestInfo->append("--> 步骤6: 参数检测...");
//                on_pushButtonParameterDetect_clicked();
//                TestProcessTaskState = 8;
//                TestProcessTick = 0;
//            } else {
//                TestProcessTaskState = 100; // 如果不检测参数，直接成功结束
//            }
//            break;
//        case 8:
//            if (TestProcessEnd) {
//                 TestProcessTaskState = 100;
//            } else if (TestProcessTick > 5000) {
//                 ui->textBrowserTestInfo->append("参数检测超时！");
//                 TestProcessError = 1;
//            }
//            break;

//        case 99: // 状态99: 错误处理
////            ui->textBrowserTestInfo->append("!!!!!! 测试因错误中断 !!!!!!");
//            ui->label_testing->setPixmap(QPixmap(":/myimage/qrc/image/testerror.png").scaled(70,70));
//            TestProcessEnd = 1;
//            TestProcessTaskState = 0; // 重置状态机
//            break;

//        case 100: // 状态100: 所有流程成功结束
//            ui->textBrowserTestInfo->append("======== 测试流程成功结束 ========");
//            ui->label_testing->setPixmap(QPixmap(":/myimage/qrc/image/testend.png").scaled(70,70));
//            TestProcessEnd = 1;
//            TestProcessTaskState = 0; // 重置状态机
//            break;
//    }
//}
//void BMCMainWindow::TestProcessTask(void)
//{
//    // 如果测试流程没有启动或已结束，则直接返回
//    if (TestProcessTaskState == 0 || TestProcessEnd == 1) {
//        return;
//    }

//    // 在任何步骤中检测到错误，立即跳转到错误处理状态
//    if (TestProcessError == 1 && TestProcessTaskState != 99) {
//        TestProcessTaskState = 99; // 99作为错误状态
//        TestProcessTick = 0;
//    }

//    // 根据TestProcessTaskState这个状态变量，执行不同的测试步骤
//    switch (TestProcessTaskState)
//    {
//        case 1: // 状态1: 开始测试流程的初始化
//            ui->textBrowserTestInfo->append("======== 开始测试流程 ========");
//            // 初始化所有步骤的完成标志
//            unlock_end = 0;
//            zerocurcali_end = 0;
//            timecali_end = 0;
//            voltdetect_end = 0;
//            versiondetect_end = 0;
//            voltdiffdetect_end = 0;
//            Curcali_end = 0;
//            curdetect_end = 0;

//            ui->label_testing->setPixmap(QPixmap(":/myimage/qrc/image/testing.png").scaled(70,70));

//            TestProcessTaskState = 2; // 进入第一个实际测试步骤
//            TestProcessTick = 0;
//            break;

//        // --- 步骤 1: 解锁 (异步) ---
//        case 2:
//            if (TestOption & UNLOCKFLAG) {
//                ui->textBrowserTestInfo->append("--> 步骤1: 解锁电池...");
//                AuthenticationEnable = 1; // 派发指令
//                TestProcessTaskState = 3; // 进入等待状态
//                TestProcessTick = 0;
//            } else {
//                TestProcessTaskState = 4; // 跳过，进入下一个测试
//            }
//            break;
//        case 3:
//            if (unlock_end) {
//                unlock_end = 0; // 重置标志
//                TestProcessTaskState = 4; // 进入下一个测试
//            } else if (TestProcessTick > 3000) { // 增加3秒超时判断
//                ui->textBrowserTestInfo->append("解锁超时！");
//                TestProcessError = 1;
//            }
//            break;

//        // --- 步骤 2: 零电流校准 (异步) ---
//        case 4:
//            if (TestOption & ZEROCALIFLAG) {
//                ui->textBrowserTestInfo->append("--> 步骤2: 零电流校准...");
//                on_pushButtonZeroCur_clicked(); // 内部会设置 ZeroCurCaliEnable
//                TestProcessTaskState = 5;
//                TestProcessTick = 0;
//            } else {
//                TestProcessTaskState = 6;
//            }
//            break;
//        case 5:
//            if (zerocurcali_end) {
//                zerocurcali_end = 0;
//                TestProcessTaskState = 6;
//            } else if (TestProcessTick > 3000) {
//                ui->textBrowserTestInfo->append("零电流校准超时！");
//                TestProcessError = 1;
//            }
//            break;

//        // --- 步骤 3: 时间校准 (异步) ---
//        case 6:
//            if (TestOption & TIMECALIFLAG) {
//                ui->textBrowserTestInfo->append("--> 步骤3: 时间校准...");
//                on_pushButtonTimeCali_clicked(); // 内部会设置 RTCCaliEnable
//                TestProcessTaskState = 7;
//                TestProcessTick = 0;
//            } else {
//                TestProcessTaskState = 8;
//            }
//            break;
//        case 7:
//            if (timecali_end) {
//                timecali_end = 0;
//                TestProcessTaskState = 8; // 前三个异步操作已完成，进入后续步骤
//            } else if (TestProcessTick > 3000) {
//                ui->textBrowserTestInfo->append("时间校准超时！");
//                TestProcessError = 1;
//            }
//            break;

//        // --- 步骤 4: 执行所有同步检测任务 ---
//        case 8:
//            ui->textBrowserTestInfo->append("--> 开始执行同步检测项...");
//            if (TestOption & VOLTDETECTFLAG) {
//                on_pushButtonCurCali_2_clicked();
//            }
//            if (TestOption & VESIONDETECTFLAG) {
//                on_pushButtonCurCali_3_clicked();
//            }
//            if (TestOption & VOLTDIFFFLAG) {
//                on_pushButtonVoltDiffDetect_clicked();
//            }
//             if (TestOption & CURDETECTFLAG) {
//                on_pushButtonCurDetect_clicked();
//            }
//            // 因为这些都是同步执行，无需等待，直接进入下一个异步任务
//            TestProcessTaskState = 10;
//            break;

//        // --- 步骤 5: 电流校准 (异步) ---
//        case 10:
//             if (TestOption & CURCALIFLAG) {
//                ui->textBrowserTestInfo->append("--> 步骤5: 电流校准...");
//                on_pushButtonCurCali_clicked();
//                TestProcessTaskState = 11;
//                TestProcessTick = 0;
//            } else {
//                TestProcessTaskState = 12;
//            }
//            break;
//        case 11:
//            if (Curcali_end) {
//                Curcali_end = 0;
//                TestProcessTaskState = 12;
//            } else if (TestProcessTick > 10000) { // 电流校准需要更长延时
//                ui->textBrowserTestInfo->append("电流校准超时！");
//                TestProcessError = 1;
//            }
//            break;

//        // --- 步骤 6: 参数检测 (异步, 最后一步) ---
//        case 12:
//            if (TestOption & PARADETECTFLAG) {
//                ui->textBrowserTestInfo->append("--> 步骤6: 参数检测...");
//                on_pushButtonParameterDetect_clicked();
//                TestProcessTaskState = 13;
//                TestProcessTick = 0;
//            } else {
//                TestProcessTaskState = 100; // 如果不检测参数，直接成功结束
//            }
//            break;
//        case 13:
//            // 参数检测的回调函数会设置 TestProcessEnd = 1
//            if (TestProcessEnd) {
//                 TestProcessTaskState = 100; // 跳转到成功状态
//            } else if (TestProcessTick > 5000) { // 参数检测需要约2.5秒，设置5秒超时
//                 ui->textBrowserTestInfo->append("参数检测超时！");
//                 TestProcessError = 1;
//            }
//            break;

//        case 99: // 状态99: 错误处理
//            ui->textBrowserTestInfo->append("!!!!!! 测试因错误中断 !!!!!!");
//            ui->label_testing->setPixmap(QPixmap(":/myimage/qrc/image/testerror.png").scaled(70,70));
//            TestProcessEnd = 1;
//            TestProcessTaskState = 0; // 重置状态机，以便下次启动
//            break;

//        case 100: // 状态100: 所有流程成功结束
//            ui->textBrowserTestInfo->append("======== 测试流程成功结束 ========");
//            ui->label_testing->setPixmap(QPixmap(":/myimage/qrc/image/testend.png").scaled(70,70));
//            TestProcessEnd = 1;
//            TestProcessTaskState = 0; // 重置状态机
//            break;
//    }
//}
//void BMCMainWindow::TestProcessTask(void)
//{
//    // 只在状态为1（启动信号）时执行一次
//    if (TestProcessTaskState != 1) {
//        return;
//    }

//    // 标记为正在运行，防止重复派发
//    TestProcessTaskState = 2;

//    ui->textBrowserTestInfo->append("======== 开始派发所有测试指令 ========");
//    ui->label_testing->setPixmap(QPixmap(":/myimage/qrc/image/testing.png").scaled(70,70));

//    // 清空之前的UI状态
//    ui->btnShow_UL->hide();
//    ui->btnShow_ZV->hide();
//    // ... 将所有 btnShow_... 的按钮都 hide()

//    // 按顺序将所有选中的测试命令加入发送队列
//    if (TestOption & UNLOCKFLAG) {
////        ui->textBrowserTestInfo->append("--> 任务1: 解锁电池 [已派发]");
//        AuthenticationEnable = 1;
//    }
//    if (TestOption & ZEROCALIFLAG) {
////        ui->textBrowserTestInfo->append("--> 任务2: 零电流校准 [已派发]");
//        ZeroCurCaliEnable = 1;
//    }
//    if (TestOption & TIMECALIFLAG) {
////        ui->textBrowserTestInfo->append("--> 任务3: 时间校准 [已派发]");
//        RTCCaliEnable = 1;
//    }
//    if (TestOption & VOLTDETECTFLAG) {
////        ui->textBrowserTestInfo->append("--> 任务4: 电压检测 [已派发]");
//        // 注意：这个是立即执行的，不算异步命令
//        on_pushButtonCurCali_2_clicked();
//    }
//    if (TestOption & VESIONDETECTFLAG) {
////        ui->textBrowserTestInfo->append("--> 任务5: 版本检测 [已派发]");
//        on_pushButtonCurCali_3_clicked();
//    }
//    if (TestOption & VOLTDIFFFLAG) {
////        ui->textBrowserTestInfo->append("--> 任务6: 压差检测 [已派发]");
//        on_pushButtonVoltDiffDetect_clicked();
//    }
//    if (TestOption & CURCALIFLAG) {
////        ui->textBrowserTestInfo->append("--> 任务7: 电流校准 [已派发]");
//        on_pushButtonCurCali_clicked();
//    }
//    if (TestOption & CURDETECTFLAG) {
////        ui->textBrowserTestInfo->append("--> 任务8: 电流检测 [已派发]");
//        on_pushButtonCurDetect_clicked();
//    }
//    if (TestOption & PARADETECTFLAG) {
////        ui->textBrowserTestInfo->append("--> 任务9: 参数检测 [已派发]");
//        on_pushButtonParameterDetect_clicked();
//    }

//    // 所有命令已派发，现在只需要等待回调函数来判断是否全部完成
//    // 我们可以设置一个总的超时定时器来防止无限等待
//    // QTimer::singleShot(30000, this, &BMCMainWindow::checkAllTestsFinished); // 比如30秒总超时
//}
//void BMCMainWindow::TestProcessTask(void)
//{
//    // 如果测试流程没有启动或已结束，则直接返回
//    if (TestProcessTaskState == 0 || TestProcessEnd == 1) {
//        return;
//    }

//    // 如果在任何步骤中检测到错误，立即跳转到错误处理状态
//    if (TestProcessError == 1) {
//        TestProcessTaskState = 99; // 99作为错误状态
//    }

//    // 根据TestProcessTaskState这个状态变量，执行不同的测试步骤
//    switch (TestProcessTaskState)
//    {
//        case 1: // 状态1: 开始测试流程的初始化
//            ui->textBrowserTestInfo->append("======== 开始测试流程 ========");
//            // 初始化所有步骤的完成标志
//            unlock_end = 0;
//            zerocurcali_end = 0;
//            timecali_end = 0;
//            voltdetect_end = 0;
//            versiondetect_end = 0;
//            voltdiffdetect_end = 0;
//            Curcali_end = 0;
//            curdetect_end = 0;

//            ui->label_testing->setPixmap(QPixmap(":/myimage/qrc/image/testing.png").scaled(70,70)); // 假设您有这张图

//            TestProcessTaskState = 2; // 进入第一个实际测试步骤
//            break;

//        case 2: // 状态2: 执行解锁
//            if (TestOption & UNLOCKFLAG) {
//                ui->textBrowserTestInfo->append("--> 步骤1: 解锁电池...");
//                on_pushButtonUnlock_clicked();
//                TestProcessTaskState = 3; // 进入等待状态
//            } else {
//                TestProcessTaskState = 4; // 跳过，进入下一个测试
//            }
//            break;

//        case 3: // 状态3: 等待解锁完成
//            if (unlock_end) {
//                unlock_end = 0; // 重置标志
//                TestProcessTaskState = 4; // 进入下一个测试
//            }
//            break;

//        case 4: // 状态4: 执行零电流校准
//            if (TestOption & ZEROCALIFLAG) {
//                ui->textBrowserTestInfo->append("--> 步骤2: 零电流校准...");
//                on_pushButtonZeroCur_clicked();
//                TestProcessTaskState = 5;
//            } else {
//                TestProcessTaskState = 6;
//            }
//            break;

//        case 5: // 状态5: 等待零电流校准完成
//            if (zerocurcali_end) {
//                zerocurcali_end = 0;
//                TestProcessTaskState = 6;
//            }
//            break;

//        case 6: // 状态6: 执行时间校准
//            if (TestOption & TIMECALIFLAG) {
//                ui->textBrowserTestInfo->append("--> 步骤3: 时间校准...");
//                on_pushButtonTimeCali_clicked();
//                TestProcessTaskState = 7;
//            } else {
//                TestProcessTaskState = 8;
//            }
//            break;

//        case 7: // 状态7: 等待时间校准完成
//            if (timecali_end) {
//                timecali_end = 0;
//                TestProcessTaskState = 8;
//            }
//            break;

//        case 8: // 状态8: 执行电压检测
//            if (TestOption & VOLTDETECTFLAG) {
//                ui->textBrowserTestInfo->append("--> 步骤4: 电压检测...");
//                on_pushButtonCurCali_2_clicked(); // 注意：您UI里的电压检测按钮是这个
//                TestProcessTaskState = 9;
//            } else {
//                TestProcessTaskState = 10;
//            }
//            break;

//        case 9: // 状态9: 等待电压检测完成
//            if (voltdetect_end) {
//                voltdetect_end = 0;
//                TestProcessTaskState = 10;
//            }
//            break;

//        case 10: // 状态10: 执行版本检测
//            if (TestOption & VESIONDETECTFLAG) {
//                ui->textBrowserTestInfo->append("--> 步骤5: 版本检测...");
//                on_pushButtonCurCali_3_clicked(); // 注意：您UI里的版本检测按钮是这个
//                TestProcessTaskState = 11;
//            } else {
//                TestProcessTaskState = 12;
//            }
//            break;

//        case 11: // 状态11: 等待版本检测完成
//            if (versiondetect_end) {
//                versiondetect_end = 0;
//                TestProcessTaskState = 12;
//            }
//            break;

//        case 12: // 状态12: 执行压差检测
//            if (TestOption & VOLTDIFFFLAG) {
//                ui->textBrowserTestInfo->append("--> 步骤6: 压差检测...");
//                on_pushButtonVoltDiffDetect_clicked();
//                TestProcessTaskState = 13;
//            } else {
//                TestProcessTaskState = 14;
//            }
//            break;

//        case 13: // 状态13: 等待压差检测完成
//            if (voltdiffdetect_end) {
//                voltdiffdetect_end = 0;
//                TestProcessTaskState = 14;
//            }
//            break;

//        case 14: // 状态14: 执行电流校准
//            if (TestOption & CURCALIFLAG) {
//                ui->textBrowserTestInfo->append("--> 步骤7: 电流校准...");
//                on_pushButtonCurCali_clicked();
//                TestProcessTaskState = 15;
//            } else {
//                TestProcessTaskState = 16;
//            }
//            break;

//        case 15: // 状态15: 等待电流校准完成
//            if (Curcali_end) {
//                Curcali_end = 0;
//                TestProcessTaskState = 16;
//            }
//            break;

//        case 16: // 状态16: 执行电流检测
//            if (TestOption & CURDETECTFLAG) {
//                ui->textBrowserTestInfo->append("--> 步骤8: 电流检测...");
//                on_pushButtonCurDetect_clicked();
//                TestProcessTaskState = 17;
//            } else {
//                TestProcessTaskState = 18;
//            }
//            break;

//        case 17: // 状态17: 等待电流检测完成
//            if (curdetect_end) {
//                curdetect_end = 0;
//                TestProcessTaskState = 18;
//            }
//            break;

//        case 18: // 状态18: 执行参数检测 (最后一步)
//            if (TestOption & PARADETECTFLAG) {
//                ui->textBrowserTestInfo->append("--> 步骤9: 参数检测...");
//                on_pushButtonParameterDetect_clicked();
//                // 这个函数会异步地设置 TestProcessEnd = 1, 所以我们在这里等待
//                TestProcessTaskState = 19;
//            } else {
//                // 如果这是最后一个测试，直接跳到成功结束状态
//                TestProcessTaskState = 100;
//            }
//            break;

//        case 19: // 状态19: 等待参数检测完成
//            // 参数检测的回调函数 ParameterDetect_callback 会设置 TestProcessEnd = 1
//            // 我们的主循环检测到 TestProcessEnd = 1 后就会停止，所以这里可以等待
//            if (TestProcessEnd) {
//                 TestProcessTaskState = 100; // 跳转到成功状态
//            }
//            break;

//        case 99: // 状态99: 错误处理
//            ui->textBrowserTestInfo->append("!!!!!! 测试因错误中断 !!!!!!");
//            ui->label_testing->setPixmap(QPixmap(":/myimage/qrc/image/testerror.png").scaled(70,70)); // 假设您有这张图
//            TestProcessEnd = 1;
//            TestProcessTaskState = 0; // 重置
//            break;

//        case 100: // 状态100: 所有流程成功结束
//            ui->textBrowserTestInfo->append("======== 测试流程成功结束 ========");
//            ui->label_testing->setPixmap(QPixmap(":/myimage/qrc/image/testend.png").scaled(70,70));
//            TestProcessEnd = 1;
//            TestProcessTaskState = 0; // 重置
//            break;
//    }
//}

//void BMCMainWindow::TestProcessTask(void)
//{

//}

///////////////在参数测试之前先清0parameter_error=0;

void BMCMainWindow::on_pushButtonUnlock_clicked()
{
//    qDebug()<<"解锁电池，触发";
    if(BMSConnectIsOk)AuthenticationEnable = 1;
}

void BMCMainWindow::on_pushButtonZeroCur_clicked()
{

//    ui->label_zeroverify->clear();
//    if(!BMSConnectIsOk)
    if(!BMSConnectIsOk)
    {
    ui->textBrowserTestInfo->append("零电流校准...");

    ui->textBrowserTestInfo->append("NG！保护板未连接！\n");
    ui->btnShow_TotalC->show();
    ui->btnShow_TotalC->setText("NG");
    ui->btnShow_TotalC->setStyleSheet("color:#ffffff;font-size:10px;background:red");
//    ui->label_zeroverify->setPixmap(QPixmap(":/myimage/qrc/image/msg_error1.png").scaled(25,25));
    TestProcessError = 1;
    }
    else
    {
//        qDebug()<<"0电流校准，触发";
     int i1=0-1000,i2=1000;
     if(ui->comboBoxTestMode->currentIndex()==0){i1=0-10000;i2=10000;}
     else if(ui->comboBoxTestMode->currentIndex()==1){i1=0-1000;i2=1000;}
     if(Bms_infor_upload.BMSinfor.PackCurrent >= i1 && Bms_infor_upload.BMSinfor.PackCurrent <= i2)
       ZeroCurCaliEnable = 1;
     else
     {
         ui->textBrowserTestInfo->append("NG！电流太大！,请调节电流为0\n");
         ui->btnShow_TotalC->show();
         ui->btnShow_TotalC->setText("NG");
         ui->btnShow_TotalC->setStyleSheet("color:#ffffff;font-size:10px;background:red");
//         ui->label_zeroverify->setPixmap(QPixmap(":/myimage/qrc/image/msg_error1.png").scaled(25,25));
         TestProcessError = 1;
         zerocurcali_end = 1;
     }
    }
}

void BMCMainWindow::on_pushButtonTimeCali_clicked()
{
//    qDebug()<<"时间校准，触发";
    if(BMSConnectIsOk)RTCCaliEnable = 1;
}

void BMCMainWindow::on_pushButtonCurCali_2_clicked()
{
//    ui->label_voltdetect->clear();
    ui->textBrowserTestInfo->append("电压测试...");
    if(BMSConnectIsOk)
    {
//        qDebug()<<"电压检测，触发";
        if(Bms_infor_upload.BMSinfor.TotalVolt >= Bms_cfg_infor.VoltDetect_lower && Bms_infor_upload.BMSinfor.TotalVolt <= Bms_cfg_infor.VoltDetect_upper)
        {
            ui->textBrowserTestInfo->append("成功！\n");
            ui->btnShow_VC->show();
            ui->btnShow_VC->setText("OK");
            ui->btnShow_VC->setStyleSheet("color:#ffffff;font-size:10px;background:green");
//            ui->label_voltdetect->setPixmap(QPixmap(":/myimage/qrc/image/msg_info.png").scaled(25,25));
        }
        else
        {
            ui->textBrowserTestInfo->append("NG！电压超出范围！\n");
            ui->btnShow_VC->show();
            ui->btnShow_VC->setText("NG");
            ui->btnShow_VC->setStyleSheet("color:#ffffff;font-size:10px;background:red");
//            ui->label_voltdetect->setPixmap(QPixmap(":/myimage/qrc/image/msg_error1.png").scaled(25,25));
            TestProcessError = 1;
        }
    }
    else
    {
        ui->textBrowserTestInfo->append("NG！保护板未连接！\n");
        ui->btnShow_VC->show();
        ui->btnShow_VC->setText("NG");
        ui->btnShow_VC->setStyleSheet("color:#ffffff;font-size:10px;background:red");
//        ui->label_voltdetect->setPixmap(QPixmap(":/myimage/qrc/image/msg_error1.png").scaled(25,25));
        TestProcessError = 1;
    }
    voltdetect_end = 1;

}

void BMCMainWindow::on_pushButtonCurCali_3_clicked()
{
//    ui->label_versiondetect->clear();
    ui->textBrowserTestInfo->append("软件版本检测...");
    if(BMSConnectIsOk)
    {
//        qDebug()<<"版本检测，触发";
        if(Bms_infor_upload.BMSinfor.SWVersion == Bms_cfg_infor.Version)
        {
            ui->textBrowserTestInfo->append("成功！\n");
            ui->btnShow_VerC->show();
            ui->btnShow_VerC->setText("OK");
            ui->btnShow_VerC->setStyleSheet("color:#ffffff;font-size:10px;background:green");
//            ui->label_versiondetect->setPixmap(QPixmap(":/myimage/qrc/image/msg_info.png").scaled(25,25));
        }
        else
        {
            ui->textBrowserTestInfo->append("NG！软件版本错误！\n");
            ui->btnShow_VerC->show();
            ui->btnShow_VerC->setText("NG");
            ui->btnShow_VerC->setStyleSheet("color:#ffffff;font-size:10px;background:red");
//            ui->label_versiondetect->setPixmap(QPixmap(":/myimage/qrc/image/msg_error1.png").scaled(25,25));
            TestProcessError = 1;
        }
    }
    else
    {
        ui->textBrowserTestInfo->append("NG！保护板未连接！\n");
        ui->btnShow_VerC->show();
        ui->btnShow_VerC->setText("NG");
        ui->btnShow_VerC->setStyleSheet("color:#ffffff;font-size:10px;background:red");
//        ui->label_versiondetect->setPixmap(QPixmap(":/myimage/qrc/image/msg_error1.png").scaled(25,25));
        TestProcessError = 1;
    }
    versiondetect_end = 1;
}

void BMCMainWindow::on_pushButtonVoltDiffDetect_clicked()
{
//    ui->label_voltdiff->clear();
    ui->textBrowserTestInfo->append("压差检测...");
    if(BMSConnectIsOk)
    {
//        qDebug()<<"压差检测，触发";
     //   Bms_infor_upload.BMSinfor.VoltDiff = 10;
        if(Bms_infor_upload.BMSinfor.VoltDiff >= Bms_cfg_infor.VoltDiff_lower && Bms_infor_upload.BMSinfor.VoltDiff <= Bms_cfg_infor.VoltDiff_upper)
        {
            ui->textBrowserTestInfo->append("OK！\n");
            ui->btnShow_VdC->show();
            ui->btnShow_VdC->setText("OK");
            ui->btnShow_VdC->setStyleSheet("color:#ffffff;font-size:10px;background:green");
//            ui->label_voltdiff->setPixmap(QPixmap(":/myimage/qrc/image/msg_info.png").scaled(25,25));
        }
        else
        {
            ui->textBrowserTestInfo->append("NG！压差超出范围！\n");
            ui->btnShow_VdC->show();
            ui->btnShow_VdC->setText("NG");
            ui->btnShow_VdC->setStyleSheet("color:#ffffff;font-size:10px;background:red");
//            ui->label_voltdiff->setPixmap(QPixmap(":/myimage/qrc/image/msg_error1.png").scaled(25,25));
            TestProcessError = 1;
        }
    }
    else
    {
        ui->textBrowserTestInfo->append("NG！保护板未连接！\n");
        ui->btnShow_VdC->show();
        ui->btnShow_VdC->setText("NG");
        ui->btnShow_VdC->setStyleSheet("color:#ffffff;font-size:10px;background:red");
//        ui->label_voltdiff->setPixmap(QPixmap(":/myimage/qrc/image/msg_error1.png").scaled(25,25));
        TestProcessError = 1;
    }
    voltdiffdetect_end = 1;
}

void BMCMainWindow::on_pushButtonCurDetect_clicked()
{
//    ui->label_curdetect->clear();
    ui->textBrowserTestInfo->append("电流检测...");
    if(BMSConnectIsOk)
    {
//        qDebug()<<"电流检测，触发";
        if(Bms_infor_upload.BMSinfor.PackCurrent >= -2200 && Bms_infor_upload.BMSinfor.PackCurrent <= -1900)
        {
            ui->textBrowserTestInfo->append("OK！\n");
            ui->btnShow_CurC->show();
            ui->btnShow_CurC->setText("OK");
            ui->btnShow_CurC->setStyleSheet("color:#ffffff;font-size:10px;background:green");
//            ui->label_curdetect->setPixmap(QPixmap(":/myimage/qrc/image/msg_info.png").scaled(25,25));
        }
        else
        {
            ui->textBrowserTestInfo->append("NG！电流超出范围！\n");
            ui->btnShow_CurC->show();
            ui->btnShow_CurC->setText("NG");
            ui->btnShow_CurC->setStyleSheet("color:#ffffff;font-size:10px;background:red");
//            ui->label_curdetect->setPixmap(QPixmap(":/myimage/qrc/image/msg_error1.png").scaled(25,25));
            TestProcessError = 1;
        }
    }
    else
    {
        ui->textBrowserTestInfo->append("NG！保护板未连接！\n");
        ui->btnShow_CurC->show();
        ui->btnShow_CurC->setText("NG");
        ui->btnShow_CurC->setStyleSheet("color:#ffffff;font-size:10px;background:red");
//        ui->label_curdetect->setPixmap(QPixmap(":/myimage/qrc/image/msg_error1.png").scaled(25,25));
        TestProcessError = 1;
    }
    curdetect_end = 1;
}

void BMCMainWindow::on_pushButtonCurCali_clicked()
{
    if(BMSConnectIsOk)
    {
//        qDebug()<<"电流校准，触发";
//       ui->label_curverify->clear();
       //ui->textBrowserTestInfo->append("...");
        //此处不知道为什么，点击校准后，要延时很久才能真正校准
       ui->textBrowserTestInfo->append("电流校准前请将负载仪放电电流设置好！");
       CurCali_timeout_count = Bms_cfg_infor.CurVarifyDelay;
       timer_CurCali->start(1000);
       QString str = QString::number(CurCali_timeout_count,10);
       str.append("...");
       ui->textBrowserTestInfo->insertPlainText(str);
  }
}

void BMCMainWindow::on_pushButtonParameterDetect_clicked()
{
//   parameter_error=0;
    if(BMSConnectIsOk)
    {
//        qDebug()<<"参数检测，触发";
//   ui->label_paradetect->clear();
   ui->textBrowserTestInfo->append("参数检测...");
   timer_paradect->start(2500);
    }
}

void BMCMainWindow::ParameterDetect_callback(void)
{
    timer_paradect->stop();
    if(BMSConnectIsOk)
    {
    if(parameter_error)
    {
        ui->textBrowserTestInfo->append("NG！保护板参数有错，出错的参数在参数表里标记红色！\n");
        ui->btnShow_ParC->show();
        ui->btnShow_ParC->setText("NG");
        ui->btnShow_ParC->setStyleSheet("color:#ffffff;font-size:10px;background:red");
//        ui->label_paradetect->setPixmap(QPixmap(":/myimage/qrc/image/msg_error1.png").scaled(25,25));
        TestProcessError = 1;
    }
    else
    {
        ui->textBrowserTestInfo->append("OK!\n");
        ui->btnShow_ParC->show();
        ui->btnShow_ParC->setText("OK");
        ui->btnShow_ParC->setStyleSheet("color:#ffffff;font-size:10px;background:green");
//        ui->label_paradetect->setPixmap(QPixmap(":/myimage/qrc/image/msg_info.png").scaled(25,25));

    }
    }
    else
    {
        ui->textBrowserTestInfo->append("NG！保护板未连接！\n");
        ui->btnShow_ParC->show();
        ui->btnShow_ParC->setText("NG");
        ui->btnShow_ParC->setStyleSheet("color:#ffffff;font-size:10px;background:red");
//        ui->label_paradetect->setPixmap(QPixmap(":/myimage/qrc/image/msg_error1.png").scaled(25,25));
        TestProcessError = 1;
    }

//    TestProcessEnd = 1;

    qDebug() << "ParameterDetect_callback 已被调用，即将设置 paraDetect_end = 1";
    paraDetect_end = 1; // <-- 【修改】使用新的标志位
}

void BMCMainWindow::CurCali_callback(void)
{
    CurCali_timeout_count--;
    QString str = QString::number(CurCali_timeout_count, 10);
    str.append("...");
    ui->textBrowserTestInfo->insertPlainText(str);

    if (CurCali_timeout_count == 0)
    {
        timer_CurCali->stop(); // 停止倒计时

//        ui->textBrowserTestInfo->append("\n倒计时结束，请按【开始电流校准】按钮继续...");

        // 启用“开始电流校准”按钮
//        ui->pushButton_StartCurCali->setEnabled(true);

        // 设置标志位，表示我们现在可以开始电流校准了
        btnStartCurCali = true;
    }
}

//void BMCMainWindow::CurCali_callback(void)
//{

//    CurCali_timeout_count--;
//    QString str = QString::number(CurCali_timeout_count,10);
//    str.append("...");
//    ui->textBrowserTestInfo->insertPlainText(str);
//    if(CurCali_timeout_count==0)
//    {
//        timer_CurCali->stop();
//        CurcaliDelayEnd = 1;
////        if(Bms_infor_upload.BMSinfor.PackCurrent>=((int)-100) && Bms_infor_upload.BMSinfor.PackCurrent<=((int)100))
////        {
//        if(ui->pushButtonLock->text()=="锁定")
//        {
//                if(Bms_infor_upload.BMSinfor.PackCurrent==0)
//                  {
////                            qDebug()<<"电流未设置好";
//                   ui->textBrowserTestInfo->append("NG！电流未设置好\n");
//                   ui->btnShow_CurV->show();
//                   ui->btnShow_CurV->setText("NG");
//                   ui->btnShow_CurV->setStyleSheet("color:#ffffff;font-size:10px;background:red");
////                       ui->label_curverify->setPixmap(QPixmap(":/myimage/qrc/image/msg_error1.png").scaled(25,25));
//                   TestProcessError = 1;
//                   Curcali_end = 1;
//                  }
//                else
//                 {
////                        qDebug()<<"开启电流校准";
//                  ui->textBrowserTestInfo->append("开启电流校准...");
//                   CurrentCaliEnable=1;
//                 }
//        }

////       }
//    }
//}

void BMCMainWindow::autoScroll()
{

     QTextCursor cursor =  ui->textBrowserTestInfo->textCursor();

     cursor.movePosition(QTextCursor::End);

     ui->textBrowserTestInfo->setTextCursor(cursor);

}



void BMCMainWindow::CheckBoxStepsSel(QString str)
{
    if(str.at(0)=='1')
    ui->checkBox_unlock->setCheckState(Qt::Checked);
    else if(str.at(0)=='0')
    ui->checkBox_unlock->setCheckState(Qt::Unchecked);

    if(str.at(1)=='1')
    ui->checkBox_zerocali->setCheckState(Qt::Checked);
    else if(str.at(1)=='0')
    ui->checkBox_zerocali->setCheckState(Qt::Unchecked);

    if(str.at(2)=='1')
    ui->checkBox_timecali->setCheckState(Qt::Checked);
    else if(str.at(2)=='0')
    ui->checkBox_timecali->setCheckState(Qt::Unchecked);

    if(str.at(3)=='1')
    ui->checkBox_voltcheck->setCheckState(Qt::Checked);
    else if(str.at(3)=='0')
    ui->checkBox_voltcheck->setCheckState(Qt::Unchecked);

    if(str.at(4)=='1')
    ui->checkBox_versioncheck->setCheckState(Qt::Checked);
    else if(str.at(4)=='0')
    ui->checkBox_versioncheck->setCheckState(Qt::Unchecked);

    if(str.at(5)=='1')
    ui->checkBox_voltdiffdetect->setCheckState(Qt::Checked);
    else if(str.at(5)=='0')
    ui->checkBox_voltdiffdetect->setCheckState(Qt::Unchecked);

    if(str.at(6)=='1')
    ui->checkBox_curcali->setCheckState(Qt::Checked);
    else if(str.at(6)=='0')
    ui->checkBox_curcali->setCheckState(Qt::Unchecked);

    if(str.at(7)=='1')
    ui->checkBox_curdetect->setCheckState(Qt::Checked);
    else if(str.at(7)=='0')
    ui->checkBox_curdetect->setCheckState(Qt::Unchecked);

    if(str.at(8)=='1')
    ui->checkBox_paradetect->setCheckState(Qt::Checked);
    else if(str.at(8)=='0')
    ui->checkBox_paradetect->setCheckState(Qt::Unchecked);




}
void BMCMainWindow::on_comboBox_fac_currentIndexChanged(int index)
{
    if(index==0)  //电子车间
    {
       ui->lineEdit2Dcode->setEnabled(true);
       ui->lineEditBarCode8->setEnabled(false);
       ui->lineEditBarCode13->setEnabled(false);

       ui->comboBox_cell->clear();
       ui->comboBox_cell->addItem("电子A线");
       ui->comboBox_cell->addItem("电子B线");
       ui->comboBox_cell->addItem("电子C线");
       ui->comboBox_cell->addItem("电子D线");
       ui->comboBox_cell->addItem("电子E线");
       ui->comboBox_cell->addItem("电子F线");
       ui->comboBox_cell->addItem("电子G线");

       ui->checkBox_unlock->setEnabled(true);
       ui->checkBox_zerocali->setEnabled(true);
       ui->checkBox_timecali->setEnabled(true);
       ui->checkBox_voltcheck->setEnabled(true);
       ui->checkBox_versioncheck->setEnabled(true);
       ui->checkBox_curcali->setEnabled(true);
       ui->checkBox_curdetect->setEnabled(true);
       ui->checkBox_paradetect->setEnabled(true);
       ui->checkBox_voltdiffdetect->setEnabled(true);

       CheckBoxStepsSel(Bms_cfg_infor.ELECWORKSHOPSteps);
       ui->checkBox_unlock->setEnabled(false);
       ui->checkBox_zerocali->setEnabled(false);
       ui->checkBox_timecali->setEnabled(false);
       ui->checkBox_voltcheck->setEnabled(false);
       ui->checkBox_versioncheck->setEnabled(false);
       ui->checkBox_curcali->setEnabled(false);
       ui->checkBox_curdetect->setEnabled(false);
       ui->checkBox_paradetect->setEnabled(false);
       ui->checkBox_voltdiffdetect->setEnabled(false);

       ui->lineEditWorkStationName->setText("保护板测试");
    }
    else if(index==1) //pack车间
    {
        ui->lineEdit2Dcode->setEnabled(false);
        ui->lineEditBarCode8->setEnabled(true);
        ui->lineEditBarCode13->setEnabled(false);
        ui->comboBox_cell->clear();
        ui->comboBox_cell->addItem("动力A线");
        ui->comboBox_cell->addItem("动力B线");
        ui->comboBox_cell->addItem("动力C线");
        ui->comboBox_cell->addItem("动力D线");
        ui->comboBox_cell->addItem("动力E线");
        ui->comboBox_cell->addItem("动力F线");
        ui->comboBox_cell->addItem("动力G线");

        ui->checkBox_unlock->setEnabled(true);
        ui->checkBox_zerocali->setEnabled(true);
        ui->checkBox_timecali->setEnabled(true);
        ui->checkBox_voltcheck->setEnabled(true);
        ui->checkBox_versioncheck->setEnabled(true);
        ui->checkBox_curcali->setEnabled(true);
        ui->checkBox_curdetect->setEnabled(true);
        ui->checkBox_paradetect->setEnabled(true);
        ui->checkBox_voltdiffdetect->setEnabled(true);

        CheckBoxStepsSel(Bms_cfg_infor.PACKWORKSHOPSteps);
        ui->checkBox_unlock->setEnabled(false);
        ui->checkBox_zerocali->setEnabled(false);
        ui->checkBox_timecali->setEnabled(false);
        ui->checkBox_voltcheck->setEnabled(false);
        ui->checkBox_versioncheck->setEnabled(false);
        ui->checkBox_curcali->setEnabled(false);
        ui->checkBox_curdetect->setEnabled(false);
        ui->checkBox_paradetect->setEnabled(false);
        ui->checkBox_voltdiffdetect->setEnabled(false);

        ui->lineEditWorkStationName->setText("组装测试");
    }
    else if(index==2) //包装车间
    {
        ui->lineEdit2Dcode->setEnabled(false);
        ui->lineEditBarCode8->setEnabled(false);
        ui->lineEditBarCode13->setEnabled(true);
        ui->comboBox_cell->clear();
        ui->comboBox_cell->addItem("包装A线");
        ui->comboBox_cell->addItem("包装B线");
        ui->comboBox_cell->addItem("包装C线");
        ui->comboBox_cell->addItem("包装D线");
        ui->comboBox_cell->addItem("包装E线");
        ui->comboBox_cell->addItem("包装F线");
        ui->comboBox_cell->addItem("包装G线");
        ui->checkBox_unlock->setEnabled(true);
        ui->checkBox_zerocali->setEnabled(true);
        ui->checkBox_timecali->setEnabled(true);
        ui->checkBox_voltcheck->setEnabled(true);
        ui->checkBox_versioncheck->setEnabled(true);
        ui->checkBox_curcali->setEnabled(true);
        ui->checkBox_curdetect->setEnabled(true);
        ui->checkBox_paradetect->setEnabled(true);
        ui->checkBox_voltdiffdetect->setEnabled(true);
        CheckBoxStepsSel(Bms_cfg_infor.WRAPWORKSHOPSteps);
        ui->checkBox_unlock->setEnabled(false);
        ui->checkBox_zerocali->setEnabled(false);
        ui->checkBox_timecali->setEnabled(false);
        ui->checkBox_voltcheck->setEnabled(false);
        ui->checkBox_versioncheck->setEnabled(false);
        ui->checkBox_curcali->setEnabled(false);
        ui->checkBox_curdetect->setEnabled(false);
        ui->checkBox_paradetect->setEnabled(false);
        ui->checkBox_voltdiffdetect->setEnabled(false);

        ui->lineEditWorkStationName->setText("包装测试");
    }
}

//#define UNLOCKFLAG 0x0001
//#define ZEROCALIFLAG 0x0002
//#define TIMECALIFLAG 0x0004
//#define VOLTDETECTFLAG 0x0008
//#define VESIONDETECTFLAG 0x0010
//#define VOLTDIFFFLAG 0x0020
//#define CURCALIFLAG 0x0040
//#define CURDETECTFLAG 0x0080
//#define PARADETECTFLAG 0x0100
void BMCMainWindow::on_pushButtonStartTset_clicked()
{
    // ================= 【修复核心1：强行打断底层等待，优先执行测试】 =================
    uartTxVector.clear();           // 1. 清空队列里堆积的常规查询指令
    readTaskStatus = Idle_Stage_rT; // 2. 强制打断底层死等，不再等那4秒钟的超时
    readTick = 0;                   // 3. 超时计步器清零
    RxArray.clear();                // 4. 清空残余接收缓存
    // ==============================================================================


    // 【修改】在这里重置所有相关的标志位
    TestProcessEnd = 0;
    TestProcessError = 0;
    TestProcessTaskState = 1;
    paraDetect_end = 0;      // <-- 【新增】必须重置

    // 【新增】如果在电流校准中也遇到了同样的问题，
    // 最好把所有异步标志位都在这里统一重置
    unlock_end = 0;
    zerocurcali_end = 0;
    timecali_end = 0;
    Curcali_end = 0;
    btnStartCurCali = false; // 确保按钮状态也被重置

    AllowAutoTest = 0;
    BmsConnectDetectStatus = 0;
    TestProcessIsEnd = 0;
    allowparread = 1;
    //test end  ,connect broken
//    BMSConnectIsOk = 0;
//    ui->label_testing->setPixmap(QPixmap(":/myimage/qrc/image/testend.png").scaled(70,70));

    if(ui->checkBox_unlock->isChecked()==true)
    {
        TestOption |= UNLOCKFLAG;
    }
    else TestOption &= ~UNLOCKFLAG;

    if(ui->checkBox_zerocali->isChecked()==true) {
        TestOption |= ZEROCALIFLAG;
    }
    else TestOption &= ~ZEROCALIFLAG;

    if(ui->checkBox_timecali->isChecked()==true) {
       TestOption |= TIMECALIFLAG;
    }
    else TestOption &= ~TIMECALIFLAG;

    if(ui->checkBox_voltcheck->isChecked()==true) {
      TestOption |= VOLTDETECTFLAG;
    }
    else TestOption &= ~VOLTDETECTFLAG;
     if(ui->checkBox_versioncheck->isChecked()==true) {
         TestOption |= VESIONDETECTFLAG;
       }
       else TestOption &= ~VESIONDETECTFLAG;
    if(ui->checkBox_voltdiffdetect->isChecked()==true)  {
        TestOption |= VOLTDIFFFLAG;
      }
      else TestOption &= ~VOLTDIFFFLAG;
    if(ui->checkBox_curcali->isChecked()==true) {
        TestOption |= CURCALIFLAG;
      }
      else TestOption &= ~CURCALIFLAG;

    if(ui->checkBox_curdetect->isChecked()==true)  {
        TestOption |= CURDETECTFLAG;
      }
      else TestOption &= ~CURDETECTFLAG;
    if(ui->checkBox_paradetect->isChecked()==true) {
        TestOption |= PARADETECTFLAG;
      }
      else TestOption &= ~PARADETECTFLAG;
}

void BMCMainWindow::on_pushButtonLock_clicked()
{
    if(ui->pushButtonLock->text() == "锁定")
    {
        AllowAutoTest = 1;

        //zgj 2025.10.17 新增 在这里重置自动测试状态机
        AutoTestStatus = 0;
        BmsConnectDetectStatus = 0;
        TestProcessIsEnd = 0;
        //zgj 2025.10.17 新增 在这里重置自动测试状态机

        BmsConnectDetectStatus = 0;
        TestProcessIsEnd = 0;
        //AllowBmsConnectDetect = 1;
        AllowBmsConnectDetect = 1;
        ui->comboBoxProductSel->setEnabled(false);
        ui->comboBoxTestMode->setEnabled(false);
        ui->cboxBaudrate->setEnabled(false);
        ui->cboxComunicatMenth->setEnabled(false);
        ui->cboxSerialNumber->setEnabled(false);

        ui->comboBox_fac->setEnabled(false);
        ui->comboBox_cell->setEnabled(false);
        ui->comboBox_upgrade->setEnabled(false);
    //    lineEditPrName  lineEditOperater lineEditWorkStationName

        ui->lineEditPrName->setEnabled(false);
        ui->lineEditOperater->setEnabled(false);
        ui->lineEditWorkStationName->setEnabled(false);

        ui->lineEditOrderNum->setEnabled(false);
        ui->lineEditBatModel->setEnabled(false);

        ui->pushButtonUnlock->setEnabled(false);
        ui->pushButtonZeroCur->setEnabled(false);
        ui->pushButtonTimeCali->setEnabled(false);
        ui->pushButtonCurCali_2->setEnabled(false);
        ui->pushButtonCurCali_3->setEnabled(false);
        ui->pushButtonVoltDiffDetect->setEnabled(false);
        ui->pushButtonCurCali->setEnabled(false);
        ui->pushButtonCurDetect->setEnabled(false);
        ui->pushButtonParameterDetect->setEnabled(false);
        ui->pushButtonStartTset->setEnabled(false);
        ui->pushButtonunLock->setEnabled(false);
        ui->pushButtonLock->setText("取消锁定");
//        ui->pushButtonLock->setStyleSheet("color:red");

    }
   else  if(ui->pushButtonLock->text() == "取消锁定")
    {
        AllowAutoTest = 0;
        BmsConnectDetectStatus = 0;
        TestProcessIsEnd = 0;
        allowparread = 1;
        //test end  ,connect broken
//        BMSConnectIsOk = 0;
        ui->label_testing->setPixmap(QPixmap(":/myimage/qrc/image/testend.png").scaled(70,70));
        ui->pushButtonunLock->setEnabled(true);
        ui->comboBoxProductSel->setEnabled(true);
        ui->comboBoxTestMode->setEnabled(true);

        ui->cboxBaudrate->setEnabled(true);
        ui->cboxComunicatMenth->setEnabled(true);
        ui->cboxSerialNumber->setEnabled(true);

        ui->comboBox_fac->setEnabled(true);
        ui->comboBox_cell->setEnabled(true);
        ui->comboBox_upgrade->setEnabled(true);

        ui->lineEditPrName->setEnabled(true);
        ui->lineEditOperater->setEnabled(true);
        ui->lineEditWorkStationName->setEnabled(true);

        ui->lineEditOrderNum->setEnabled(true);
        ui->lineEditBatModel->setEnabled(true);

        ui->pushButtonUnlock->setEnabled(true);
        ui->pushButtonZeroCur->setEnabled(true);
        ui->pushButtonTimeCali->setEnabled(true);
        ui->pushButtonCurCali_2->setEnabled(true);
        ui->pushButtonCurCali_3->setEnabled(true);
        ui->pushButtonVoltDiffDetect->setEnabled(true);
        ui->pushButtonCurCali->setEnabled(true);
        ui->pushButtonCurDetect->setEnabled(true);
        ui->pushButtonParameterDetect->setEnabled(true);
        ui->pushButtonStartTset->setEnabled(true);
        ui->pushButtonLock->setText("锁定");
//        ui->pushButtonLock->setStyleSheet("color:green");
    }

}

void BMCMainWindow::on_pushButtonunLock_clicked()
{
   if(BMSConnectIsOk)
    { uartTxVector.clear();SendResetEnable = 1;resetok=0;}
}

void BMCMainWindow::on_pushButtonSave_clicked()
{
    QString path = QFileDialog::getSaveFileName(this, "save", "../", "TXT(*.txt)");
    if(path.isEmpty() == false)
    {
        QFile file; //创建文件对象
        //关联文件名字
        file.setFileName(path);

        //打开文件，只写方式
        bool isOk = file.open(QIODevice::WriteOnly);
        if(isOk == true)
        {

            //获取编辑区内容
            QString str = ui->textBrowserTestInfo->toPlainText();
            //写文件
            // QString -> QByteArray
            //file.write(str.toUtf8());

            //QString -> c++ string -> char *
            //file.write(str.toStdString().data());

            //转换为本地平台编码
            file.write(str.toLocal8Bit());
        }

        file.close();

    }
}

void BMCMainWindow::on_pushButtonClear_clicked()
{
    ui->textBrowserTestInfo->clear();
    scan_counter = 0;
}

void BMCMainWindow::on_pushButtonConnectDB_clicked()
{
    DBConnectIsOk=createSQLServerConnection();  //连接数据库
    if(DBConnectIsOk)
    {
        //定义一个label
            titleLabel->setText("在线");//label显示内容
            statusBar()->addWidget(titleLabel);//将label加到状态栏上
    }
    else
    {

            titleLabel->setText("离线");//label显示内容
            statusBar()->addWidget(titleLabel);//将label加到状态栏上
    }
//    Sthread->start();
}

void BMCMainWindow::on_pushButtonResume_clicked()
{
    // 创建一个question弹出对话框，添加两个按钮：Yes和No
        QMessageBox *msgBox = new QMessageBox(QMessageBox::Question, tr("注意"), tr("注意！此举将循环次数和FCC恢复出厂设置"),
                                              QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

        // 将原本显示“Yes”的按钮改为显示“是”
        msgBox->button(QMessageBox::Yes)->setText("是");
        // 将原本显示“No”的按钮改为显示“否”
        msgBox->button(QMessageBox::No)->setText("否");
        // 将原本显示“xx”的按钮改为显示“不要”
        msgBox->button(QMessageBox::Cancel)->setText("不要");

        // 弹出对话框
        int ret = msgBox->exec();


        switch (ret) {
        case QMessageBox::Yes:
            //qDebug() << "1";
            SendResumeEnable = 1;
            break;
        case QMessageBox::No:
            //qDebug() << "2";
            break;
        case QMessageBox::Cancel:
            //qDebug() << "3";
            break;
        }
}

void BMCMainWindow::on_pushButtonReset_clicked()
{
    // 创建一个question弹出对话框，添加两个按钮：Yes和No
        QMessageBox *msgBox = new QMessageBox(QMessageBox::Question, tr("注意"), tr("注意！复位后系统将重启！"),
                                              QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

        // 将原本显示“Yes”的按钮改为显示“是”
        msgBox->button(QMessageBox::Yes)->setText("是");
        // 将原本显示“No”的按钮改为显示“否”
        msgBox->button(QMessageBox::No)->setText("否");
        // 将原本显示“xx”的按钮改为显示“不要”
        msgBox->button(QMessageBox::Cancel)->setText("不要");

        // 弹出对话框
        int ret = msgBox->exec();


        switch (ret) {
        case QMessageBox::Yes:
            //qDebug() << "1";
            SendResetEnable = 1;
            break;
        case QMessageBox::No:
            //qDebug() << "2";
            break;
        case QMessageBox::Cancel:
            //qDebug() << "3";
            break;
        }
}

void BMCMainWindow::on_pushButton_StartCurCali_clicked()
{
    // 如果倒计时仍在进行，允许提前触发校准（不需要等到自减到0）
    if (timer_CurCali->isActive()) {
        timer_CurCali->stop();
        btnStartCurCali = true;
    }

    // 只有在标志位为 true 时，点击才有效
    if (!btnStartCurCali) {
        return;
    }

    // 点击后，立即重置标志位并禁用按钮，防止重复触发
    btnStartCurCali = false;
//    ui->pushButton_StartCurCali->setEnabled(false);

    // 检查电流是否已设置好 (电流值应该显著不为0)
    if (Bms_infor_upload.BMSinfor.PackCurrent > -100 && Bms_infor_upload.BMSinfor.PackCurrent < 100)
    {
       ui->textBrowserTestInfo->append("NG！电流未设置好或负载未打开！\n");
       ui->btnShow_CurV->show();
       ui->btnShow_CurV->setText("NG");
       ui->btnShow_CurV->setStyleSheet("color:#ffffff;font-size:10px;background:red");
       TestProcessError = 1;
       Curcali_end = 1;
    }
    else
    {
       // 电流正常，发送校准指令
       ui->textBrowserTestInfo->append("\n开启电流校准...");
       CurrentCaliEnable = 1;
    }
}
//void BMCMainWindow::on_pushButton_StartCurCali_clicked()
//{
//    btnStartCurCali=1;
//    uartTxVector.clear();
//}

void BMCMainWindow::on_pushButtonresumw2_clicked()
{
    uartTxVector.clear();
    SendResumeEnable = 1; //恢复
}

void BMCMainWindow::on_pushButtonSelProduct_clicked()
{
    QString str = ui->comboBox_fac->currentText();
    if(str=="电子车间")
    {
    path=QFileDialog::getOpenFileName(this,
                                                      "打开文件",
                                                      "\\\\192.168.0.249/config/PCBA/");
    }
    else
    {
        path=QFileDialog::getOpenFileName(this,
                                                          "打开文件",
                                                          "\\\\192.168.0.249/config/FinishGoods/");
    }
//     path="\\192.168.0.249/config/PCBA/3302010012-KH19288-077-config.txt";
//    if(false == path.isEmpty()) //如果选择文件路径有效
//    {
//        qDebug()<<"False";
//        return;
//}


//    QString path = txt1.filePath();
    qDebug()<<path;
    QFile file(path);
//    //获取文件信息
//    QFileInfo info(path);
//    //指定文件的名字
//    file.setFileName(path);

    bool isOk = file.open(QIODevice::ReadOnly);

    if(true == isOk)
    {
//        qDebug()<<"openisok";

      QString str1 = path.section('/',5,5);
      if(str=="电子车间")
      ui->lineEditSelProduct->setText("电子车间/"+str1);
      else
      ui->lineEditSelProduct->setText("pack成品/"+str1);
      ui->lineEditPrName->setText(str1.mid(22,11));
      QTextStream in(&file);
      in.setCodec("UTF-8");
  //    while (!in.atEnd())
  //    {
  //      QString myText = in.readLine();    //按行读一行文件
  //      qDebug()<<myText;

  //    }

      QString myText = in.readLine();
      QString strValName,strUpper,strLower;
      if(myText=="STARTCFG")
      {



       ReadlineAndGath();
       if(strValName=="BMSERPCode")
       {
        Bms_cfg_infor.BMSERPCode = strLower.section(' ',0,0);
        Bms_infor_upload.BMSERPCode = Bms_cfg_infor.BMSERPCode;
        ui->lineEditBoardCode->setText(Bms_cfg_infor.BMSERPCode);
       }

       ReadlineAndGath();
       if(strValName=="FirmwareERPCode")
       {
        Bms_cfg_infor.FirmwareERPCode = strLower.section(' ',0,0);
        Bms_infor_upload.FirmwareERPCode = Bms_cfg_infor.FirmwareERPCode;
        ui->lineEditSoftCode->setText(Bms_cfg_infor.FirmwareERPCode);
       }

       ReadlineAndGath();
       if(strValName=="SWRevsion")
       {
        Bms_cfg_infor.Version = strLower.section(' ',0,0);
  //      Bms_infor_upload.BMSinfor.SWVersion = Bms_cfg_infor.Version;
        ui->lineEditVersion->setText(Bms_cfg_infor.Version);
       }

       ReadlineAndGath();
       if(strValName=="CanBeUpdate")
       {
        Bms_cfg_infor.CanBeUpdate = strLower.section(' ',0,0);
        Bms_infor_upload.CanBeUpdate = Bms_cfg_infor.CanBeUpdate;
        if(Bms_cfg_infor.CanBeUpdate=="Yes")
        ui->comboBox_upgrade->setCurrentIndex(0);
        else if(Bms_cfg_infor.CanBeUpdate=="No")
         ui->comboBox_upgrade->setCurrentIndex(1);
       }
       ReadlineAndGath();
       if(strValName=="TestMeth")
       {
        Bms_cfg_infor.TestMeth = strLower.section(' ',0,0);
        if(Bms_cfg_infor.TestMeth=="Manual")
        ui->comboBoxTestMode->setCurrentIndex(0);
        else if(Bms_cfg_infor.TestMeth=="Auto")
         ui->comboBoxTestMode->setCurrentIndex(1);
  //      else if(Bms_cfg_infor.TestMeth=="Auto2")
  //       ui->comboBoxTestMode->setCurrentIndex(2);
       }

       myText = in.readLine();
       strValName = myText.section('=',0,0);
       strLower = myText.section('=',1,1);
       strUpper = myText.section('=',2,2);
       if(strValName=="SOCDetect")
       {
        Bms_cfg_infor.SOCDetect_lower = strLower.toUInt();
        QString s=strUpper.section(' ',0,0);
        Bms_cfg_infor.SOCDetect_upper = s.toUInt();
       }

       myText = in.readLine();
       strValName = myText.section('=',0,0);
       strLower = myText.section('=',1,1);
       strUpper = myText.section('=',2,2);
       if(strValName=="VoltDetect")
       {
        Bms_cfg_infor.VoltDetect_lower = strLower.toUInt();
        QString s=strUpper.section(' ',0,0);
        Bms_cfg_infor.VoltDetect_upper = s.toUInt();
       }

       ReadlineAndGath();
       if(strValName=="CurVarify")
       {
         QString s =  strLower.section(' ',0,0);
        Bms_cfg_infor.CurVarify = s.toUInt();

       }

       ReadlineAndGath();
       if(strValName=="CurVarifyDelay")
       {
         QString s =  strLower.section(' ',0,0);
        Bms_cfg_infor.CurVarifyDelay = s.toUInt();

       }

       myText = in.readLine();
       strValName = myText.section('=',0,0);
       strLower = myText.section('=',1,1);
       strUpper = myText.section('=',2,2);
       if(strValName=="CurDetect")
       {
        Bms_cfg_infor.CurDetect_lower = strLower.toUInt();

        QString s=strUpper.section(' ',0,0);
        Bms_cfg_infor.CurDetect_upper = s.toUInt();

       }

       myText = in.readLine();
       strValName = myText.section('=',0,0);
       strLower = myText.section('=',1,1);
       strUpper = myText.section('=',2,2);
       if(strValName=="VoltDiff")
       {
        Bms_cfg_infor.VoltDiff_lower = strLower.toUInt();

        QString s=strUpper.section(' ',0,0);
        Bms_cfg_infor.VoltDiff_upper = s.toUInt();
        qDebug()<<"Bms_cfg_infor.VoltDiff_lower="<<Bms_cfg_infor.VoltDiff_lower;
        qDebug()<<"Bms_cfg_infor.VoltDiff_upper="<<Bms_cfg_infor.VoltDiff_upper;
       }

       ReadlineAndGath();
       if(strValName=="DC")
       {
         QString s =  strLower.section(' ',0,0);
        Bms_cfg_infor.BMSinfor.DC= s.toUInt();
       }

       ReadlineAndGath();
       if(strValName=="ShutDTime")
       {
         QString s =  strLower.section(' ',0,0);
        Bms_cfg_infor.BMSinfor.ShutDTime= s.toUInt();
       }

       ReadlineAndGath();
       if(strValName=="BalEndV")
       {
         QString s =  strLower.section(' ',0,0);
        Bms_cfg_infor.BMSinfor.BalEndV= s.toUInt();
       }

       ReadlineAndGath();
       if(strValName=="ChgEndCur")
       {
         QString s =  strLower.section(' ',0,0);
        Bms_cfg_infor.BMSinfor.ChgEndCur= s.toUInt();
       }

       ReadlineAndGath();
       if(strValName=="ChgEndVol")
       {
         QString s =  strLower.section(' ',0,0);
        Bms_cfg_infor.BMSinfor.ChgEndVol= s.toUInt();
       }

       ReadlineAndGath();
       if(strValName=="ChgEndDel")
       {
         QString s =  strLower.section(' ',0,0);
        Bms_cfg_infor.BMSinfor.ChgEndDel= s.toUInt();
       }

       ReadlineAndGath();
       if(strValName=="SlfDsqBalV")
       {
         QString s =  strLower.section(' ',0,0);
        Bms_cfg_infor.BMSinfor.SlfDsqBalV= s.toUInt();
       }

       ReadlineAndGath();
       if(strValName=="BalStartV")
       {
         QString s =  strLower.section(' ',0,0);
        Bms_cfg_infor.BMSinfor.BalStartV= s.toUInt();
       }

       ReadlineAndGath();
       if(strValName=="BalDelay")
       {
         QString s =  strLower.section(' ',0,0);
        Bms_cfg_infor.BMSinfor.BalDelay= s.toUInt();
       }

       ReadlineAndGath();
       if(strValName=="CellNum")
       {
         QString s =  strLower.section(' ',0,0);
        Bms_cfg_infor.BMSinfor.CellNum= s.toUInt();
       }

       ReadlineAndGath();
       if(strValName=="MNFDate")
       {
         QString s =  strLower.section(' ',0,0);
        Bms_cfg_infor.BMSinfor.MNFDate= s.toUInt();
       }

       ReadlineAndGath();
       if(strValName=="SlfDsgDel")
       {
         QString s =  strLower.section(' ',0,0);
        Bms_cfg_infor.BMSinfor.SlfDsgDel= s.toUInt();
       }

       ReadlineAndGath();
       if(strValName=="SampleRV")
       {
         QString s =  strLower.section(' ',0,0);
        Bms_cfg_infor.BMSinfor.SampleRV= s.toFloat();
       }

       ReadlineAndGath();
       if(strValName=="OV")
       {
         QString s =  strLower.section(' ',0,0);
        Bms_cfg_infor.BMSinfor.OV= s.toUInt();
       }

       ReadlineAndGath();
       if(strValName=="OVR")
       {
         QString s =  strLower.section(' ',0,0);
        Bms_cfg_infor.BMSinfor.OVR= s.toUInt();
       }

       ReadlineAndGath();
       if(strValName=="OVT")
       {
         QString s =  strLower.section(' ',0,0);
        Bms_cfg_infor.BMSinfor.OVT= s.toUInt();
       }

       ReadlineAndGath();
       if(strValName=="UV")
       {
         QString s =  strLower.section(' ',0,0);
        Bms_cfg_infor.BMSinfor.UV= s.toUInt();
       }

       ReadlineAndGath();
       if(strValName=="UVR")
       {
         QString s =  strLower.section(' ',0,0);
        Bms_cfg_infor.BMSinfor.UVR= s.toUInt();
       }

       ReadlineAndGath();
       if(strValName=="UVT")
       {
         QString s =  strLower.section(' ',0,0);
        Bms_cfg_infor.BMSinfor.UVT= s.toUInt();
       }

       ReadlineAndGath();
       if(strValName=="BALV")
       {
         QString s =  strLower.section(' ',0,0);
        Bms_cfg_infor.BMSinfor.BALV= s.toUInt();
       }

       ReadlineAndGath();
       if(strValName=="PREV")
       {
         QString s =  strLower.section(' ',0,0);
        Bms_cfg_infor.BMSinfor.PREV= s.toUInt();
       }

       ReadlineAndGath();
       if(strValName=="LZV")
       {
         QString s =  strLower.section(' ',0,0);
        Bms_cfg_infor.BMSinfor.LZV= s.toUInt();
       }
       ReadlineAndGath();
       if(strValName=="PFV")
       {
         QString s =  strLower.section(' ',0,0);
        Bms_cfg_infor.BMSinfor.PFV= s.toUInt();
       }
       ReadlineAndGath();
       if(strValName=="OCD1V")
       {
         QString s =  strLower.section(' ',0,0);
        Bms_cfg_infor.BMSinfor.OCD1V= s.toUInt();
       }
       ReadlineAndGath();
       if(strValName=="OCD1T")
       {
         QString s =  strLower.section(' ',0,0);
        Bms_cfg_infor.BMSinfor.OCD1T= s.toUInt();
       }
       ReadlineAndGath();
       if(strValName=="OCD2V")
       {
         QString s =  strLower.section(' ',0,0);
        Bms_cfg_infor.BMSinfor.OCD2V= s.toUInt();
       }
       ReadlineAndGath();
       if(strValName=="OCD2T")
       {
         QString s =  strLower.section(' ',0,0);
        Bms_cfg_infor.BMSinfor.OCD2T= s.toUInt();
       }

       ReadlineAndGath();
       if(strValName=="SCV")
       {
         QString s =  strLower.section(' ',0,0);
        Bms_cfg_infor.BMSinfor.SCV= s.toUInt();
       }

       ReadlineAndGath();
       if(strValName=="SCT")
       {
         QString s =  strLower.section(' ',0,0);
        Bms_cfg_infor.BMSinfor.SCT= s.toUInt();
       }

       ReadlineAndGath();
       if(strValName=="OCCV")
       {
         QString s =  strLower.section(' ',0,0);
        Bms_cfg_infor.BMSinfor.OCCV= s.toUInt();
       }

       ReadlineAndGath();
       if(strValName=="OCCT")
       {
         QString s =  strLower.section(' ',0,0);
        Bms_cfg_infor.BMSinfor.OCCT= s.toUInt();
       }

       ReadlineAndGath();
       if(strValName=="EOTC")
       {
         QString s =  strLower.section(' ',0,0);
        Bms_cfg_infor.BMSinfor.EOTC= s.toInt();
       }
       ReadlineAndGath();
       if(strValName=="EOTCR")
       {
         QString s =  strLower.section(' ',0,0);
        Bms_cfg_infor.BMSinfor.EOTCR= s.toInt();
       }
       ReadlineAndGath();
       if(strValName=="EUTC")
       {
         QString s =  strLower.section(' ',0,0);
        Bms_cfg_infor.BMSinfor.EUTC= s.toInt();
       }
       ReadlineAndGath();
       if(strValName=="EUTCR")
       {
         QString s =  strLower.section(' ',0,0);
        Bms_cfg_infor.BMSinfor.EUTCR= s.toInt();
       }
       ReadlineAndGath();
       if(strValName=="EOTD")
       {
         QString s =  strLower.section(' ',0,0);
        Bms_cfg_infor.BMSinfor.EOTD= s.toInt();
       }
       ReadlineAndGath();
       if(strValName=="EOTDR")
       {
         QString s =  strLower.section(' ',0,0);
        Bms_cfg_infor.BMSinfor.EOTDR= s.toInt();
       }
       ReadlineAndGath();
       if(strValName=="EUTD")
       {
         QString s =  strLower.section(' ',0,0);
        Bms_cfg_infor.BMSinfor.EUTD= s.toInt();
       }
       ReadlineAndGath();
       if(strValName=="EUTDR")
       {
         QString s =  strLower.section(' ',0,0);
        Bms_cfg_infor.BMSinfor.EUTDR= s.toInt();
       }
       ReadlineAndGath();
       if(strValName=="SOCC")
       {
         QString s =  strLower.section(' ',0,0);
        Bms_cfg_infor.BMSinfor.SOCC= s.toUInt();
       }

       ReadlineAndGath();
       if(strValName=="SOCCD")
       {
         QString s =  strLower.section(' ',0,0);
        Bms_cfg_infor.BMSinfor.SOCCD= s.toUInt();
       }

       ReadlineAndGath();
       if(strValName=="EFUSE")
       {
         QString s =  strLower.section(' ',0,0);
        Bms_cfg_infor.EnFuse2= s.toUInt();
       }

       ReadlineAndGath();
       if(strValName=="WriteSN")
       {
         QString s =  strLower.section(' ',0,0);
        Bms_cfg_infor.EnWriteSN= s.toUInt();
       }

       ReadlineAndGath();
       if(strValName=="ELECWORKSHOP")
       {
         QString s =  strLower.section(' ',0,0);
        Bms_cfg_infor.ELECWORKSHOPSteps= s;
       }

       ReadlineAndGath();
       if(strValName=="PACKWORKSHOP")
       {
         QString s =  strLower.section(' ',0,0);
        Bms_cfg_infor.PACKWORKSHOPSteps= s;
       }

       ReadlineAndGath();
       if(strValName=="WRAPWORKSHOP")
       {
         QString s =  strLower.section(' ',0,0);
        Bms_cfg_infor.WRAPWORKSHOPSteps= s;
       }
      }
    }
    file.close();
}

void BMCMainWindow::on_pushButtonResume_2_clicked()
{
    // 创建一个question弹出对话框，添加两个按钮：Yes和No
    QMessageBox *msgBox = new QMessageBox(QMessageBox::Question, tr("注意"), tr("注意！此举将循环次数和FCC恢复出厂设置"),
                                          QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);

    // 将原本显示“Yes”的按钮改为显示“是”
    msgBox->button(QMessageBox::Yes)->setText("是");
    // 将原本显示“No”的按钮改为显示“否”
    msgBox->button(QMessageBox::No)->setText("否");
    // 将原本显示“xx”的按钮改为显示“不要”
    msgBox->button(QMessageBox::Cancel)->setText("不要");

    // 弹出对话框
    int ret = msgBox->exec();


    switch (ret) {
    case QMessageBox::Yes:
        //qDebug() << "1";
        SendResumeEnable = 1;
        break;
    case QMessageBox::No:
        //qDebug() << "2";
        break;
    case QMessageBox::Cancel:
        //qDebug() << "3";
        break;
    }
}

void BMCMainWindow::on_pushButtonStartStorage_clicked()
{
    StartStorageEnable = 1;
}

void BMCMainWindow::on_pushButtonStart_clicked()
{
    SendShutDownEnable = 1;
}

void BMCMainWindow::on_pushButtonStart_2_clicked()
{
    SendPowerUpEnable = 1;
}

void BMCMainWindow::on_pushButtonStartHeat_clicked()
{
    OpenHeaterEnable = 1;
}

void BMCMainWindow::on_pushButtonStopHeat_clicked()
{
    CloseHeaterEnable = 1;
}

void BMCMainWindow::on_lineEditMattSilverCode_returnPressed()
{
    CodesWriteEnable = 1;
}

bool BMCMainWindow::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->lineEditMattSilverCode) {
        if (event->type() == QEvent::MouseButtonPress) {
                QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
                // 检查是否是左键点击
                if (mouseEvent->button() == Qt::LeftButton) {
                    ui->lineEditMattSilverCode->clear();  // 清空内容
                    ui->lineEditInternalControlCode->clear();  // 清空内容
                    ui->lineEditBMSCode->clear();  // 清空内容
                    return true;  // 事件已处理，不再传递
                }
        }
    }

    return QMainWindow::eventFilter(watched, event);
}

void BMCMainWindow::on_pushButtonSystemStaOpen_clicked()
{
    if (systemStaWidget->isHidden())
    {
        systemStaWidget->show();
    }
    else
    {
        systemStaWidget->activateWindow();
        systemStaWidget->raise();
    }
}

void BMCMainWindow::setTableStatus(uint8_t *bs, uint16_t ps)
{
    setUiUpdateClose();
    QTableWidget *SystemSta = systemStaWidget->getTableWidget();
    uint8_t temp=0,laprecess;
    temp = 0x01;
    //通过temp <<= 1一直从00000001左移一位来进行对uint8_t数组bs的每位进行0、1的对比
    for (uint8_t i=0;i<8;i++) {
        if(bs[0]&temp)
        {
                SystemSta->item(0,8-i)->setBackground(QBrush(QColor(222, 81, 55)));

        }
        else
        {
                SystemSta->item(0,8-i)->setBackground(QBrush(QColor(247, 249, 251)));
        }
        temp <<= 1;
    }

    temp = 0x01;
    for (uint8_t i=0;i<4;i++) {
        if(bs[1]&temp)
        {
                SystemSta->item(1,8-i)->setBackground(QBrush(QColor(222, 81, 55)));
        }
        else
        {
                SystemSta->item(1,8-i)->setBackground(QBrush(QColor(247, 249, 251)));
        }

        temp <<= 1;
    }

    temp = 0x01;
    for (uint8_t i=0;i<3;i++) {
        if(bs[2]&temp)
        {
                SystemSta->item(2,8-i)->setBackground(QBrush(QColor(247, 249, 251)));
        }
        else
        {
                SystemSta->item(2,8-i)->setBackground(QBrush(QColor(1, 121, 227)));
        }

        temp <<= 1;
    }

    if(bs[2]&0x08)//LZV
    {
            SystemSta->item(2,5)->setBackground(QBrush(QColor(1, 121, 227)));
    }
    else
    {
            SystemSta->item(2,5)->setBackground(QBrush(QColor(247, 249, 251)));
    }
    if(bs[2]&0x10)//EEPR
    {
            SystemSta->item(2,4)->setBackground(QBrush(QColor(222, 81, 55)));
    }
    else
    {
            SystemSta->item(2,4)->setBackground(QBrush(QColor(247, 249, 251)));
    }

    laprecess = (uint8_t)ps;
    temp = 0x01;
    for (uint8_t i=0;i<8;i++) {
        if(laprecess&temp)
        {
            if((i<4) || (i==5))
            {
                    SystemSta->item(3,8-i)->setBackgroundColor(QColor(247, 249, 251));
            }else{
                    SystemSta->item(3,8-i)->setBackgroundColor(QColor(1, 121, 227));
            }
        }
        else
        {
            if((i<4) || (i==5)){
                    SystemSta->item(3,8-i)->setBackgroundColor(QColor(1, 121, 227));
            }else
            {
                    SystemSta->item(3,8-i)->setBackgroundColor(QColor(247, 249, 251));
            }
        }

        temp <<= 1;
    }

    laprecess = (uint8_t)(ps>>8);
    temp = 0x01;
    for (uint8_t i=0;i<8;i++) {
        if(laprecess&temp)
        {
              SystemSta->item(4,8-i)->setBackground(QBrush(QColor(1, 121, 227)));
        }
        else
        {
                SystemSta->item(4,8-i)->setBackground(QBrush(QColor(247, 249, 251)));
        }

        temp <<= 1;
    }

    setUiUpdateOpen();
}

void BMCMainWindow::setUiUpdateClose()
{
    this->setUpdatesEnabled(false);
    systemStaWidget->setUpdatesEnabled(false);
}

void BMCMainWindow::setUiUpdateOpen()
{
    systemStaWidget->setUpdatesEnabled(true);
    this->setUpdatesEnabled(true);
    this->update();
    systemStaWidget->update();
}
