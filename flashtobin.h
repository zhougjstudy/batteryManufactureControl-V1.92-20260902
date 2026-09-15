#ifndef FLASHTOBIN_H
#define FLASHTOBIN_H
#include <QCoreApplication>
#include <stdio.h>
#include <stdlib.h>
#include<QDebug>
#include<QString>
#include<QByteArray>
#include<QFile>
#include<QDataStream>

typedef struct falshpart
{
    QString  FlashPartHead;
    QString  FlashPartPartDate;

}FlashTOHex;

class FlashtoBin
{
public:
    FlashtoBin();
    int mystrlen(const char *StrDest);
    unsigned char mystrlenchar(const char *StrDest);
    int HexToDec(char *src);
    unsigned char  HexToDecchar(char *src);
    void FormatHexjudge(FlashTOHex *Flashtohex,QByteArray src);
    QString AnalyseHEX(QByteArray dest);
    void QString2intArray2(unsigned char *pptr,QString src);
    void ReadWrite(QString filenameread,QString filenameWrite);
//    void ReadWriteToBIN(QString filenameread);


};

#endif // FLASHTOBIN_H
