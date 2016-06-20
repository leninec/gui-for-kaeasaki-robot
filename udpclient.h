#ifndef UDPCLIENT_H
#define UDPCLIENT_H
#include <QUdpSocket>
#include <QtCore>
#include <Windows.h>
#include <stddef.h>
#include <conio.h>
#include <QObject>
#include "struct.h"
#include "fazus.h"
#include <io.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <share.h>
#include "sleep.h"
#include <QTimer>


#define SERVERADDR "192.168.0.2"
#define PORTADDR 49152

//#define SERVERADDR "192.168.1.57"
//#define SERVERADDR "127.0.0.1"
//#define PORTADDRSEND 5824
//#define PORTADDRRECV PORTADDRSEND+1


class UdpClient : public QObject
{
    Q_OBJECT
public:

    explicit UdpClient(QObject *parent = 0);
    ~UdpClient();
    QByteArray ReadMessage();
    QString  ReadStatus();

    QByteArray qbDataSend;
    QString sError;
    QString sAnswer;
    bool bSend;
    bool bSdvigOK;
    bool bControl;
    void GetPointer(fazus &pointer);
    void AddComand( rs10nComand );
    int ParametrsRs10N(float *angle,int *speed,int *mesh);

    void SetWorkSpaceINpc(float,float,float,float,float,float);
    QTimer *timer;
    bool Get_bLevelVanna(){return this->bLevelVanna;}
    bool Get_bLevelBak(){return this->bLevelBak;}
    bool Get_bStatusSliv(){return this->bStatSliv;}
    bool Get_bDef(){return this->bDef;}

    float Get_OinstrShift(){return this->fOinstrShift;};


signals:
    void finished();
    void error(QString err);
    void answer(); // сигнал сообщает нам о том что есть новые прочитанные данные
    void progres(int step); // сигнал для прогресс бара

public slots:
     void Stop(){brun=false;}
     int Process();
   //  void ReadyRead();
     int SendData(QByteArray data, int timeout = 2000,int nSend = 2);
     int ReadData();
     void EventTimer();

private:
     char readPipe;
     BOOL   fConnected;
    // Идентификатор канала Pipe
    HANDLE hNamedPipe;

   // LPCWSTR  lpszPipeName = L"\\\\.\\pipe\\$MyPipe$";
    LPCWSTR  lpszPipeName;

    // Буфер для передачи данных через канал
    char   szBuf[512];

    // Количество байт данных, принятых через канал
    DWORD  cbRead;

    // Количество байт данных, переданных через канал
    DWORD  cbWritten;
    int fh; // для файла с координатами
    fazus *pFazus;
    char infoScan[128];
    stCoord stC;

    float fXShift;
    float fYShift;
    float fZShift;
    float fOShift;
    float fAShift;
    float fTShift;

    float fOinstrShift; // на макетном образце рамка с датчиками расположена под другими углом - нужно смещние

    float fWZx1,fWZx2,fWZy1,fWZy2,fWZz1,fWZz2;  // для хранения информации о границах перемещения

    int iSpeed;
    int iNumPoint;
    int iCurrentPoint;
    int iMaxZ;
    int iMesh;
    int flagCircle;   // создаем флаг для контроля перемещения по дуге. Как только мы изменили высоту по Z
                       // сразу  сбрасываем этот флаг и при движении по дуге созадем таректорию заново
    float fStepAngle;
    float rHub;
    bool bDef;  // флаг контроль идет
    bool brun;
    bool bcheck;
    bool bLevelVanna;
    bool bLevelBak;
    bool bStatSliv;

    bool bPipeOpen;
    int iFlagNastr;
    QUdpSocket *socket;
    QHostAddress qha;

  //  QVector <QByteArray>vPriemCoordinat;
    QVector <rs10nComand> vectorComand;
    QMutex vectorComand_mutex;
    QVector <QByteArray>vPpriemMessage;
    QMutex vPpriemMessage_mutex;
    QVector <stCoord> vectorC;
    QByteArray qbaTrajectory;
    void TestPipe();
    void ClearData();
   // void SaveC(QString comment);
    void SaveC(QByteArray comment);
    void Sdvig();
    int SendCommand(QByteArray baCommand,QString sAnswer, QString sError, int par,int timeout = 2000,int nSend = 2); // par если 1 высылаем ответ в интрфейс если 0 удаляем
    int SendCommand(QByteArray baCommand,int timeout = 2000,int nSend = 2);
    void sendInPipe(QByteArray);
    void createPipe(QString pipeName);
    int getPointCoordint (int, QString timeoutText, float *x,float *y,float *z,float *o,float *a,float *t);
    int getRS10parametr();
    int UploadShift();
    int OpenFileT(QString);
    int HereShift();
    int Here();
    int Here(float *x,float *y,float *z);
    int Here(float *x,float *y,float *z,float *o,float *a,float *t);
    int TestHere();
    int ClearStep();
    int DeletePoint();
    int DownloadPoint();
    int Orientation180(int del =1); // есть 1 удаялем созданную точку
    int Calibration(int napr);
    int DeleteFileDef(QString name);
    int MoveX(float value);
    int MoveY(float value);
    int MoveZ(float value);
    int WaitingMoveFinish();
    int SetSpeed(float value);
    int SetMech(float value);
    int StartControl();
    int GoHome();
    int GetErrorRobot();
    int ResetError();
    int MotorOn();
    int ContinueWork();
    int ResumeAfterError();
    int TestMotor();
    int GetDigitalInput();
    int DeleteShift();
    int SetWorkSpace(float x1,float y1,float z1,float x2,float y2,float z2);
    int GetWorkSpace(float *x1,float *y1,float *z1,float *x2,float *y2,float *z2);
    QString SearhIncreace(QString nameNastr, int ampLow,int ampMax); // путь к папке где лежат настройки
    float Round(float);
    int Init();
    int SetCircle(float r,float step);
    int ChangeMoveMode(int mode);
    int SavePoint();
    int Move2step(float step,int napr);
    int Move2degree(float degree,int napr);

};

#endif // UDPCLIENT_H
