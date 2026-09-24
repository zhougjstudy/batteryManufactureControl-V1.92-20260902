#ifndef BMCMAINWINDOW_H
#define BMCMAINWINDOW_H

#include "flashtobin.h"
#include <QMainWindow>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QFile>
#include <QFileDialog>
#include <QTimer>
#include <QLabel>
#include "dbthread.h" //线程头文件
#include "tableWidgetSystemStaWidget.h"

#define UNLOCKFLAG 0x0001
#define ZEROCALIFLAG 0x0002
#define TIMECALIFLAG 0x0004
#define VOLTDETECTFLAG 0x0008
#define VESIONDETECTFLAG 0x0010
#define VOLTDIFFFLAG 0x0020
#define CURCALIFLAG 0x0040
#define CURDETECTFLAG 0x0080
#define PARADETECTFLAG 0x0100



#define ReadlineAndGath() myText = in.readLine();\
                          strValName = myText.section('=',0,0);\
                          strLower = myText.section('=',1,1)
QT_BEGIN_NAMESPACE
namespace Ui { class BMCMainWindow; }
QT_END_NAMESPACE


typedef  enum // Upgrading Stage
{
   Idle_Stage=0, HandShake1_Stage, HandShake2_Stage,Wait_Stage, Start_Stage, Sending_Stage,Ending_Stage,Reset_Stage
}PackStatus_TypeDef;

typedef  enum // read Task Stage
{
   Idle_Stage_rT=0, Send_Read_Stage,Accept_Parse_Stage
}readTaskStatus_TypeDef;

typedef  enum // TEST ProcessTask Stage
{
   Waitting_Stage=0, PreTest_Stage,Unlock_Stage
}TESTProcessTaskStatus_TypeDef;

typedef struct _txStruct
{
    union
    {
       uint8_t alldat[72];
       struct
       {
           uint8_t Frame_Head;
           uint8_t Frame_Length;
           uint8_t Frame_Length_Check;
           uint8_t Frame_cmd;
           uint8_t Dat_Playload[66];
           uint8_t Check;
           uint8_t Frame_tail;
       }sFrame;
    }uFrame;

}txStruct;

typedef struct _txStruct_System_parameter
{
    union
    {
       uint8_t alldat[222];
       struct
       {
           uint8_t Frame_Head;
           uint8_t Frame_Length;
           uint8_t Frame_Length_Check;
           uint8_t Frame_cmd;
           uint8_t Dat_Playload[216];
           uint8_t Check;
           uint8_t Frame_tail;
       }sFrame;
    }uFrame;

}txStruct_System_parameter;

typedef struct binfor
{
  QString SWVersion;
  uint32_t Bat_Status;
  uint16_t Pack_Status;
  uint32_t TotalVolt;
  uint32_t VoltDiff;
  uint32_t CfgOption;
  int PackCurrent;
  uint32_t DC;
  uint32_t FCC;
  uint32_t RSOC;
  uint32_t RC;
  uint32_t BatIdCode;
  uint32_t CAN_ID;
  uint32_t MCI;
  uint32_t ShutDTime;
  uint32_t BalEndV;
  uint32_t ChgEndCur;
  uint32_t ChgEndVol;
  uint32_t ChgEndDel;
  uint32_t MNFDate;
  uint32_t SlfDsqBalV;
  uint32_t BalStartV;
  uint32_t BalDelay;
  uint32_t SlfDsgDel;
  uint32_t OtherState;
  float SampleRV;
  uint32_t CellNum;
  uint32_t OV;
  uint32_t OVR;
  uint32_t OVT;
  uint32_t UV;
  uint32_t UVR;
  uint32_t UVT;
  uint32_t BALV;
  uint32_t PREV;
  uint32_t LZV;
  uint32_t PFV;
  uint32_t OCD1V;
  uint32_t OCD1T;
  uint32_t OCD2V;
  uint32_t OCD2T;
  uint32_t SCV;
  uint32_t SCT;
  uint32_t OCCV;
  uint32_t OCCT;
  int EOTC;
  int EOTCR;
  int EUTC;
  int EUTCR;
  int EOTD;
  int EOTDR;
  int EUTD;
  int EUTDR;
  uint32_t SOCC;
  uint32_t SOCCD;
}infor_form_bms;

typedef struct _infor
{
     //gath from cfg txt
    QString ProdName;
    QString OperatorName;
    QString PONumber;
    QString WorkStationName;
    QString ProdDate;
    QString ScanTime;
    QString BMSERPCode;
    QString FirmwareERPCode;
    QString Version;
    QString CanBeUpdate;
    QString TestMeth;
    uint32_t VoltDetect_lower;
    uint32_t VoltDetect_upper;
    uint32_t CurVarify;
    uint32_t CurVarifyDelay=6;
//    uint32_t CurVarifyDelay;
    uint32_t CurDetect_lower;
    uint32_t CurDetect_upper;
    uint32_t VoltDiff_lower;
    uint32_t VoltDiff_upper;
    uint32_t SOCDetect_lower;
    uint32_t SOCDetect_upper;
    QString QualityStats;
    QString RepairTimes;
    QString DefectIssue;
    QString DefectName;
    bool EnFuse2;
    bool EnWriteSN=0;
    QString BATModel;

    QString CalbTime;
    QString ELECWORKSHOPSteps="110111111";
    QString PACKWORKSHOPSteps="001111001";
    QString WRAPWORKSHOPSteps="001111001";
    infor_form_bms BMSinfor;
}infor_form_txt;



typedef struct ainfor
{
    QString PONumber;
    QString ProdName;
    QString ProdDate;
    QString ScanTime;
    QString OperatorName;

    QString ProBarCode;
    QString InnerBarCode;
    QString BMSCode;
    QString BMSERPCode;
    QString FirmwareERPCode;

    QString QualityStats;
    QString RepairTimes;
    QString DefectIssue;
    QString DefectName;

    //gath from ui
    float TelnetMethord;
    QString BaudRate;
    QString WorkShop;
    QString LineName;
    QString WorkStationName;
    QString CanBeUpdate;
    QString CalbTime;

    infor_form_bms BMSinfor;
}infor_to_upload;
class DBThread;//前置声明
class BMCMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    BMCMainWindow(QWidget *parent = nullptr);
    ~BMCMainWindow();

    friend class DBThread;

    void ComUpdate(void);
    void Run();
    void initForm();
    void Get_Data(void);
    void BaseInfoParser(QByteArray buffer);
    void closeEvent(QCloseEvent *event);
    void keyPressEvent(QKeyEvent *keyValue);
    void loadPlugin(void);
    bool createSQLServerConnection(void);
    void loadStyle(const QString &qssFile);
    void ScanCFGFile(void);
    void AutoTest(void);
    void TestProcessTask(void);

    void dealDone(); //线程结束槽函数
    void stopThread(); //停止线程槽函数

    uint8_t SocCalcXor(QByteArray buf, uint8_t len)
    {
        uint8_t  i=0;
        //uint8_t *p = 0;
        char checksum = 0;
        //p = buf;
        checksum = buf[0];
       // p++;

        for(i=1; i<len; i++)
        {
            checksum ^= buf[i];
            //p++;
        }
        return checksum;
    }
    uint8_t SocCalcXor(uint8_t *buf, uint8_t len)
    {
        uint8_t  i=0;
        //uint8_t *p = 0;
        char checksum = 0;
        //p = buf;
        checksum = buf[0];
       // p++;

        for(i=1; i<len; i++)
        {
            checksum ^= buf[i];
            //p++;
        }
        return checksum;
    }

    uint8_t SocCalcSum(QByteArray buf, uint8_t len)
    {
        uint8_t  i=0;
        //uint8_t *p = 0;
        char checksum = 0;
        //p = buf;
        checksum = buf[0];
       // p++;

        for(i=1; i<len; i++)
        {
            checksum += buf[i];
            //p++;
        }
        return checksum;
    }

    void sendHandShake(uint8_t idx,uint8_t num_cmd);
    void sendBLCMD(uint8_t len,uint8_t FrameOrder,uint64_t DevID,uint8_t subCMD,uint32_t FlashAddr,QByteArray lData);
    void setTableStatus(uint8_t *bs,uint16_t ps);
    void setUiUpdateClose();
    void setUiUpdateOpen();
    void sendReadCMD(uint8_t *dat,uint8_t size);

    void ClearBaseInfo(void);
     void CheckIntoQueue(void);
    void BubbleSort(uint16_t* arr, uint16_t sz);
    void SystemInfoParser(QByteArray buffer);

    void TableNoSel(void);
    void setItemNoSel(uint8_t i,uint8_t j);

    void Upgrade_task(void);
    void BaseInfoReadTask(void);
    void ReadWriteToBIN(void);
    void DataBaseInsert(void);
    void BarCodeProcess(void);

    void CheckBoxStepsSel(QString str);

    void BmsConnectDetect(void);

    bool eventFilter(QObject *watched, QEvent *event) override;  // 重写事件过滤器函数
private:
    Ui::BMCMainWindow *ui;
    tableWidgetSystemStaWidget *systemStaWidget;

public:
    bool paraDetect_end;
    DBThread *Sthread; //线程对象
    QLabel *titleLabel = new QLabel;
   QVector <txStruct>  uartTxVector;
   QVector <txStruct_System_parameter>  uartTxVector_System_parameter;
   QTimer* my_time;
   QTimer* timer_paradect;
   QTimer* timer_CurCali;
   QTimer* timer_AutoCurCali;
   infor_form_txt Bms_cfg_infor;
   infor_to_upload Bms_infor_upload;
   QString barStr;
   QString curStr;
   QTimer timer_rt;

   uint16_t TestOption = 0;

   uint8_t BmsConnectDetectStatus = 0;
   uint16_t BmsConnectDetectTick = 0;
   uint8_t parameterReadIsOk = 0;
   uint8_t BaseInforReadIsOk = 0;
   bool AllowBmsConnectDetect = 1;
   uint16_t gTick=0;
   uint16_t readTick=0;
   uint16_t TestProcessTick = 0;
   uint16_t AutoTestTick = 0;
   uint32_t secondTick=0;
   uint8_t Second20=0;
   QSerialPort *serialport;
   QByteArray RArray;
   QByteArray RxArray;
   uint8_t RecFlag=0;
   uint16_t Buffer_time=70; //串口接收缓冲时间
   uint8_t Status=0;
   QList<QFileInfo> *CFGfileInfo;

   bool StartUpgrade=0;
   bool StartReadBase=0;
   bool StartPushtoQueue=0;
   bool Scaned=0;
   PackStatus_TypeDef UpgradeStatus=Idle_Stage;
   readTaskStatus_TypeDef readTaskStatus=Idle_Stage_rT;
   uint8_t TestProcessTaskState = 0;

   QString path;
   QFile file; //文件对象
   QString fileName; //文件名字
   uint32_t fileSize; //文件大小
   uint32_t sendSize; //已经发送文件的大小
   uint8_t FrameOrder;
   uint32_t FlashAddr;
   uint32_t BulkNum;
   uint32_t BulkNumSended;
   uint32_t LastBulkByte;
   QByteArray Filebuffer;
   uint32_t checksum;
   uint16_t checkxor;
   FlashtoBin fla2bin;
   FlashTOHex Flashtohex_ins;
   unsigned char parat[32];
   uint32_t HexBaseAddr;
   uint32_t HexOffsetAddr;
   bool suffix_is_hex=0;

   bool parameter_error=0;
   bool AllowSetRed = 0;
   uint8_t RTC_Year;
   uint8_t RTC_Month;
   uint8_t RTC_Day;
   uint8_t RTC_Hour;
   uint8_t RTC_Minute;
   uint8_t RTC_Second;
   uint8_t CurCali_timeout_count = 0;
//   uint32_t CurCali_timeout_count = 0;
   uint8_t  AutoTestStatus=0;
   bool BMSConnectIsOk = 0; //BMS连接成功
   bool DBConnectIsOk = 0;  //数据库连接成功

   bool ReadBaseInfoEnable=0; //发送读命令使能
   bool SendResumeEnable=0;//发送恢复命令使能
   bool SendResetEnable=0;//发送复位命令使能
   bool SendShutDownEnable=0;//发送关机命令使能
   bool SendPowerUpEnable=0;//发送关机命令使能
   bool StartStorageEnable=0;//开启仓储
   bool OpenHeaterEnable=0;//开启加热
   bool CloseHeaterEnable=0;//关闭加热
   bool AuthenticationEnable=0;//授权使能
   bool ZeroCurCaliEnable=0;//零电流校准使能
   bool VoltCheckEnable=0;//电压检测使能
   bool CurrentCaliEnable=0;//电流校准
   bool RTCCaliEnable=0;//时间校准
   bool IDWriteEnable=0;//ID录入
   bool CodesWriteEnable=0;//哑银码、内控码、BMS编码录入
   bool ReadSysPraEnable=0;//读取系统参数
   bool EFUSEEnable=0; //三端保险丝使能
   bool EsendSNRead=0;
   bool EsendSNWrite=0;
   bool TestProcessEnd = 0;
   bool TestProcessError = 0;

   bool BarCodeIsAccept = 0;
   bool Prohibit_Upload = 0;
   QString BarCodeStr = " ";    //码值
   QString BarCodeStr_old = " ";
   QString BarCodeStr2D;   //2D
   QString BarCodeStr8;   //8位内控码
   QString BarCodeStr13;  //13位产品序列号

   // 用于测试流程状态机的完成标志
   uint32_t scan_counter = 0;
   bool unlock_end=0;
   bool zerocurcali_end = 0;
   bool timecali_end=0;
   bool voltdetect_end=0;
   bool versiondetect_end=0;
   bool voltdiffdetect_end=0;
   bool Curcali_end = 0;
   bool curdetect_end=0;
   bool parameterdetect_end=0;

   bool AllowAutoTest = 0;
   bool btnStartCurCali=0;
   bool CurcaliDelayEnd = 0;
   bool resetok = 0;

   bool BaseInfo_Error = 0;

   bool TestProcessIsEnd = 0;

   bool AllowReadSysPara = 0;

   bool allowparread = 1;

   // BMS重启保护期相关
   bool BmsRebooting = false;      // BMS正在重启中标志
   uint32_t BmsRebootingTick = 0;  // 重启保护期计时器

   uint8_t Bat_Status[3];
   uint16_t Pack_Status;
   uint8_t Pack_Status2;

private slots:
   void on_btnComCheck_clicked();

   void processRet_PrName();
   void getCommData(void);
   void MyFunction_Timeout(void);
   void ParameterDetect_callback(void);
   void CurCali_callback(void);
   void AutoCurCali_check(void);
   void autoScroll(void);
   void on_btnComOpen_clicked();
   void on_comboBoxProductSel_currentIndexChanged(int index);
   void on_btnOpenFile_clicked();
   void on_btnStartUpgrade_clicked();
   void on_pushButtonUnlock_clicked();
   void on_pushButtonZeroCur_clicked();
   void on_pushButtonTimeCali_clicked();
   void on_pushButtonCurCali_2_clicked();
   void on_pushButtonCurCali_3_clicked();
   void on_pushButtonVoltDiffDetect_clicked();
   void on_pushButtonCurDetect_clicked();
   void on_pushButtonCurCali_clicked();
   void on_pushButtonParameterDetect_clicked();
   void on_comboBox_fac_currentIndexChanged(int index);
   void on_pushButtonStartTset_clicked();
   void on_pushButtonLock_clicked();
   void on_pushButtonunLock_clicked();
   void on_pushButtonSave_clicked();
   void on_pushButtonClear_clicked();
   void on_pushButtonConnectDB_clicked();
   void on_pushButtonResume_clicked();
   void on_pushButtonReset_clicked();
   void on_pushButton_StartCurCali_clicked();

   void on_pushButtonresumw2_clicked();
   void on_pushButtonSelProduct_clicked();
   void on_pushButtonResume_2_clicked();
   void on_pushButtonStartStorage_clicked();
   void on_pushButtonStart_clicked();
   void on_pushButtonStart_2_clicked();
   void on_pushButtonStartHeat_clicked();
   void on_pushButtonStopHeat_clicked();
   void on_lineEditMattSilverCode_returnPressed();
   void on_pushButtonSystemStaOpen_clicked();
};
#endif // BMCMAINWINDOW_H
