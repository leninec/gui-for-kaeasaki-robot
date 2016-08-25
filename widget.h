#ifndef WIDGET_H
#define WIDGET_H

#include <QtWidgets/QMainWindow>

#include "fazus.h"
#include "UdpClient.h"
#include <QListWidgetItem>
#include <QMessageBox>



namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:

    explicit Widget(QWidget *parent = 0);
    ~Widget();
    QString comment;
    QByteArray Get_qbEncodeComment(){return this->encodedComment;}


signals:
    void processStart();
    void quit();

public slots:
    void ErrorHandlerF(QString serr);
    void ErrorHandlerUDP(QString serr);
    int  GetNastr();
    void ShowScan();
    void SendComand();
    void ReadAnswer();
    void Sliv();
    void Pump();
    void Stop();
    void StopE();
    int StartControl();
    void OneShot1();
    void Ascan();
    void AscanStop();
    void OpenFileT(QString name = "0");
    void HereShift();
    void Here();
    void ClearStep();
    void DeletePoint();
    void DownloadPoint();
    void ShowProgress(int step);
    void CreateHub();
    void Orientation();
    void Calibration();
    void WriteComment();
    void StartFazus();
    void Continue();
    void StartMaxim();
    void SetSpeed();
    void SetMech();
    void SetWorkSpace();
    void GoHome();
    void OpenHelp();
    void GoFirstPoint();
    void ResetandResume();
    void ShowService();
    void HideTab();
    void StopFazus();
    void TestPipe();

    void xPmove();
    void xMmove();
    void yPmove();
    void yMmove();
    void zPmove();
    void zMmove();
    void stepDegreeP();
    void stepDegreeM();
    void stepMmPlus();
    void stepMmMinus();
    void moveDegree();
    void J1Pmove();
    void J1Mmove();
    void J2Pmove();
    void J2Mmove();
    void J3Pmove();
    void J3Mmove();
    void J4Pmove();
    void J4Mmove();
    void J5Pmove();
    void J5Mmove();
    void J6Pmove();
    void J6Mmove();

    void SetSpinX(double);
    void SetSpinY(double);
    void SetSpinZ(double);

    void NextPageButton();

    void GetSpeedMech();

    //    void OneShot();

private slots:
    void on_pushButtonComment_clicked();




   // void on_tabWidget_currentChanged(int index);

    int on_listWidget_currentRowChanged(int currentRow);

    void on_stackedWidget_currentChanged(int arg1);

    void TextChanged();
private:
    bool bReadyControl;
    bool bMassivButton[13];
    bool bFlagMoveMode;
    QString nameFolder ;
    QByteArray encodedComment;
    bool bRepeatControl;  // флаг чтобы правильно открывать и закрывать кнопки при повторном контроле
    QDateTime data;
    QString fileNameNastr;
    QProcess    *processFazus;
    QProcess    *processMaxim;
    QThread     *threadF ;
    QThread     *threadUDP;
    fazus       *fazusD;
    fazus       *pF;
    UdpClient  *udpClient;
    rs10nComand rComand;

    QListWidgetItem  *ItemTitle;
    QListWidgetItem *ItemReg;
    QListWidgetItem *ItemTraect;
    QListWidgetItem *ItemOrient;
    QListWidgetItem *ItemNastrUzk;
    QListWidgetItem *ItemStartPoint;
    QListWidgetItem *ItemControl;
    QListWidgetItem *ItemVspomag;
    QListWidgetItem *ItemManualMove;
    QListWidgetItem *ItemService;

    Ui::Widget *ui;
    int i_flag_button[12];

    QString chtosyQt(QString str);
    int parser(const char *find, const char *source, char *dest);
    int parser (QByteArray *find,QByteArray *source,QByteArray *dest);
    void CloseButtonMotion();
    void OpenButtonMotion();
    void SetButtonControl();
    void SetLabelConrol();
    void InitRobot();
    void ShowTab();
    void RepeatControl(); // подготовка к повтроному контролю - сброс счетчика шагов траекторию не удаялем
    void InitSystem(); // проконтролировал втулку или произошла ошибка - все сбросил в начало
    void GetWorkSpace(float *x1,float *y1,float *z1,float *x2,float *y2,float *z2);
    void writeTrace(const char *chFileName, 		// Название файла
                    float 	d_H,					// Внешений диаметр
                    float 	d_L,					// Внутренний диаметр
                    float	H,						// Высота
                    float 	stepscan				// Шаг сканирования по внешнему диаметру
                    );

};

#endif // WIDGET_H
