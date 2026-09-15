#include "flashtobin.h"



FlashtoBin::FlashtoBin()
{

}


/****计算字符串长度*****/
int FlashtoBin::mystrlen(const char *StrDest)
{
    int i;
    i=0;
    while((*StrDest++)!='\0')
    {
        i++;
    }//这个循环体意思是从字符串第一个字符起计数，只遇到字符串结束标志'\0’才停止计数
    return i;
}


/****计算字符串长度*****/
unsigned char FlashtoBin::mystrlenchar(const char *StrDest)
{
    unsigned char i;
    i=0;
    while((*StrDest++)!='\0')
    {
        i++;
    }//这个循环体意思是从字符串第一个字符起计数，只遇到字符串结束标志'\0’才停止计数
    return i;
}

/****16进制转换10进制*****/
int FlashtoBin::HexToDec(char *src)
{
    //将src中的无符号十六进制字符串转为十进制数
    //如src="001A"，则返回26
    //字符串src需要以'\0'作为结束标志
    int value=0,srclen=mystrlen(src);
    int i;
    for(i=0;i<srclen;i++)
    {
        if(src[i]<='9'&&src[i]>='0')
        {
            value=value*16+(int)(src[i]-'0'); //'0'
        }
        else if(src[i]<='f'&&src[i]>='a')
        {
            value=value*16+(int)(src[i]-'a'+10);
        }
        else
        {
            value=value*16+(int)(src[i]-'A'+10);
        }
    }
    return value;//返回转换后的数值
}



/****16进制转换10进制*****/
unsigned char  FlashtoBin::HexToDecchar(char *src)
{
    //将src中的无符号十六进制字符串转为十进制数
    //如src="001A"，则返回26
    //字符串src需要以'\0'作为结束标志
    unsigned char value=0,srclen=mystrlenchar(src);
    int i;
    for(i=0;i<srclen;i++)
    {
        if(src[i]<='9'&&src[i]>='0')
        {
            value=value*16+(unsigned char)(src[i]-'0'); //'0'
        }
        else if(src[i]<='f'&&src[i]>='a')
        {
            value=value*16+(unsigned char)(src[i]-'a'+10);
        }
        else
        {
            value=value*16+(unsigned char)(src[i]-'A'+10);
        }
    }
    return value;//返回转换后的数值
}


/**************************************
 * 时间：06/17
 * 函数名：FlashTOHex FormatHexjudge(QByteArray src)
 * 返回值：分割后的字符串 头九个字节FlashPartHead(附带冒号),
 *                 和后边数据段FlashPartPartDate
 *
 * ***********************************/
void FlashtoBin::FormatHexjudge(FlashTOHex *Flashtohex,QByteArray src)
{
  QString dest;
  static QByteArray space=" ";
  //qDebug()<<src.size();   //大小带空格
  Flashtohex->FlashPartHead=src.left(9);

  dest=src.right(src.size()-9);
  //dst=src.right(src.size()-9).simplified();    //清除两边的空格

  Flashtohex->FlashPartPartDate=dest.remove(QRegExp("\\s"));      //清除中间多余的空格
  //qDebug()<<Flashtohex.FlashPartHead;
  //qDebug()<<Flashtohex.FlashPartPartDate;
 // return Flashtohex;
}

/*****将字符串转换成十进制存储到pptr数组中******************/
void FlashtoBin::QString2intArray2(unsigned char *pptr,QString src)
{
    unsigned char tranfer;
    int count=0;
    int coun=0;
        //qDebug()<<"Pcount"<<src.size();
    while(count<src.size())
    {
        tranfer=HexToDecchar(src.mid(count,2).toLatin1().data());  //先转换成16进制
        pptr[coun++]=tranfer;
        count+=2;
        //qDebug()<<"tranfer"<<tranfer;
    }
}



