#include "widget.h"
#include "ui_widget.h"
#include <QFileDialog>

#include <QProcess>

#include "labelclick.h"

// дефектоскоп и прием данных в разных потоках потокам
// поток приема данных не правильно завершается

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    this->processMaxim = new QProcess();
    if (QFile::exists(".\\PipeDefMultiWin.exe"))
    {
        // this->processMaxim->setProgram("E:\\FAZUS_DLL\\fazus-N\\PipeDefMultiWin.exe");
        this->processMaxim->setProgram(".\\PipeDefMultiWin.exe");
    }
    // проверку на существование еще делаем в main  и оттуда закрываем приложение если файлов нет
    this->processFazus = new QProcess();
    if (QFile::exists(".\\VORON1.exe"))
    {
        //  this->processFazus->setProgram("E:\\FAZUS_DLL\\fazus-N\\VORON1.exe");
        this->processFazus->setProgram(".\\VORON1.exe");
    }

    ui->setupUi(this);
    QWidget::setFixedSize(938,520);

    QPixmap myPixmap(".\\traektoria.png");
    ui->labelFon->autoFillBackground();
    ui->labelFon->setPixmap(myPixmap);

    ui->stackedWidget->setCurrentIndex(0);

    ui->lineEditPass->setEchoMode(QLineEdit::Password);

    ui->labelFonFirstPoint->autoFillBackground();
    ui->labelFonFirstPoint->setPixmap(myPixmap);

    QPixmap myPixmapAxis(".\\axis.png");
    ui->labelAxis->autoFillBackground();
    ui->labelAxis->setPixmap(myPixmapAxis);

    QPixmap myPixmapC(".\\calib.png");
    ui->label_19->autoFillBackground();
    ui->label_19->setPixmap(myPixmapC);

    this->bRepeatControl = false;
    this->bReadyControl = false;

    ui->progressBar->setHidden(true);

    QFont qFont;

    this->bMassivButton[bnastrUZK] = true;
    this->bMassivButton[bviev] = true;
    this->bMassivButton[bgetTraect] = true;
    this->bMassivButton[bgetUzk] = false;
    this->bMassivButton[bcalibrovka] = false;
    this->bMassivButton[bresume] = true;
    this->bMassivButton[bhereshift] = false;
    this->bMassivButton[bstart] = false;
    this->bMassivButton[bstop] = false;
    this->bMassivButton[bsliv] = true;
    this->bMassivButton[bpump] = true;
    this->bMassivButton[borient] = true;
    this->bMassivButton[borient] = true;
    this->bMassivButton[bpark] = true;
    this->SetButtonControl();

    this->ui->plainTextEdit->setMaximumBlockCount(10);
    this->ui->plainTextEditServis->setMaximumBlockCount(100);
    this->HideTab();

    ui->pushButtonCalibration->setToolTip("Для запуска процедуры ориентации расположите датчики так, чтобы видеть эхо сигнал от детали ");
    ui->pushButtonHereShift->setToolTip(" Поместите датчики под деталью, не ударив об дно ванны.");
    ui->pushButtonStop->setToolTip(" Кнопка останавливает перемещение во время контроля. В ручном перемещении не действует");
    ui->pushButtonHomePos->setToolTip(" Перемещение манипулятора в начальное положение");
    ui->pushButtonOrientation->setToolTip(" В текущей точке фланец будет выставлен параллельно опорной плите ");

    ui->pushButtonStopEmergency->setStyleSheet(QString::fromUtf8("background-color: rgb(250, 0, 0);"));

    ui->listWidget->setToolTip(" Для проведения процедуры контроля двигайтесь по списку, выполненые действия будуь зачеркиваться ");

    ui->pushButtonStopEmergency->setToolTip(" Останавливает программу робота (лампа RUN/HOLD).");
    ui->pushButtonContinue->setToolTip(" Возобновление программы робота (лампа RUN/HOLD).");
    // ui->gridLayout->setEnabled(false);

    //  qDebug ("Start");

    this->threadF = new QThread;
    this->fazusD = new fazus;
    pF = &(*(this->fazusD));  // указатель для доступа к функция обьекта из обьекта управления роботом

    this->threadUDP = new QThread;
    // QThread threadUdp;
    this->udpClient = new UdpClient;
    // UdpClient*udpClient = new UdpClient;
    // udpClient->moveToThread(&threadUdp);

    //ui->pBoneShot->setEnabled(false);

    this->fazusD->moveToThread(threadF);
    this->udpClient->moveToThread(threadUDP);

    connect(this,SIGNAL(processStart()),fazusD,SLOT(Process()));   // для повторного запуска дефектоскопа

    connect(fazusD,SIGNAL(error(QString)),this,SLOT(ErrorHandlerF(QString)));
    connect(udpClient,SIGNAL(error(QString)),this,SLOT(ErrorHandlerUDP(QString)));
    connect(threadF,SIGNAL(started()),fazusD,SLOT(Process()));
    connect(fazusD,SIGNAL(finished()),fazusD,SLOT(deleteLater()));
    connect(threadF,SIGNAL(finished()),threadF,SLOT(deleteLater()));
    // connect(threadUDP,SIGNAL(destroyed(QObject*)),udpClient,SLOT(stop()));
    //connect(udpClient->timer,SIGNAL(timeout()),udpClient,SLOT(EventTimer()));


    connect(threadUDP,SIGNAL(started()),udpClient,SLOT(Process()));

    connect(ui->pushButtonNastr,SIGNAL(clicked()),this,SLOT(GetNastr()));
    // connect(fazusD,SIGNAL(scan()),this,SLOT(ShowScan()));
    connect(ui->pushButtonSend,SIGNAL(clicked()),this,SLOT(SendComand()));
    connect(udpClient,SIGNAL(answer()),this,SLOT(ReadAnswer()));
    connect(ui->pushButtonPump,SIGNAL(clicked()),this,SLOT(Pump()));
    connect(ui->pushButtonSliv,SIGNAL(clicked()),this,SLOT(Sliv()));
    connect(ui->pushButtonStop,SIGNAL(clicked()),this,SLOT(Stop()));
    connect(ui->pushButtonContinue,SIGNAL(clicked()),this,SLOT(Continue()));
    connect(ui->pushButtonStopEmergency,SIGNAL(clicked()),this,SLOT(StopE()));
    connect(ui->pushButtonStartControl,SIGNAL(clicked()),this,SLOT(StartControl()));
    connect(ui->pushButtonLoadT,SIGNAL(clicked()),this,SLOT(OpenFileT())); // пустое имя чтобы выбрать нужное из диалога
    connect(ui->pushButtonCreateHub,SIGNAL(clicked()),this,SLOT(CreateHub()));
    connect(ui->pushButtonPass,SIGNAL(clicked()),this,SLOT(ShowService()));
    connect(ui->pushButtonHide,SIGNAL(clicked()),this,SLOT(HideTab()));
    connect(ui->pushButtonStopFazus,SIGNAL(clicked()),this,SLOT(StopFazus()));
   // connect(ui->pushButtonStopFazus,SIGNAL(clicked()),qApp,SLOT(quit()));
    connect(ui->pushButtonPipe,SIGNAL(clicked()),this,SLOT(TestPipe()));

    //connect(ui->pBoneShot,SIGNAL(clicked(bool)),this,SLOT(OneShot1()));

    connect(ui->pushButtonXpMove,SIGNAL(clicked()),this,SLOT(xPmove()));
    connect(ui->pushButtonXmMove,SIGNAL(clicked()),this,SLOT(xMmove()));
    connect(ui->pushButtonYpMove,SIGNAL(clicked()),this,SLOT(yPmove()));
    connect(ui->pushButtonYmMove,SIGNAL(clicked()),this,SLOT(yMmove()));
    connect(ui->pushButtonZpMove,SIGNAL(clicked()),this,SLOT(zPmove()));
    connect(ui->pushButtonZmMove,SIGNAL(clicked()),this,SLOT(zMmove()));
    connect(ui->pushButtonXpMove_2,SIGNAL(clicked()),this,SLOT(xPmove()));
    connect(ui->pushButtonXmMove_2,SIGNAL(clicked()),this,SLOT(xMmove()));
    connect(ui->pushButtonYpMove_2,SIGNAL(clicked()),this,SLOT(yPmove()));
    connect(ui->pushButtonYmMove_2,SIGNAL(clicked()),this,SLOT(yMmove()));
    connect(ui->pushButtonZpMove_2,SIGNAL(clicked()),this,SLOT(zPmove()));
    connect(ui->pushButtonZmMove_2,SIGNAL(clicked()),this,SLOT(zMmove()));
    connect(ui->pushButtonZpMove_3,SIGNAL(clicked()),this,SLOT(zPmove()));
    connect(ui->pushButtonZmMove_3,SIGNAL(clicked()),this,SLOT(zMmove()));
    connect(ui->pushButtonZmMove_4,SIGNAL(clicked()),this,SLOT(zMmove()));
    connect(ui->pushButtonZpMove_4,SIGNAL(clicked()),this,SLOT(zPmove()));


    // connect(ui->pushButtonAscan,SIGNAL(clicked()),this,SLOT(Ascan()));
    //  connect(ui->pushButtonAscanStop,SIGNAL(clicked()),this,SLOT(AscanStop()));
    connect(ui->pushButtonHere,SIGNAL(clicked()),this,SLOT(Here()));
    connect(ui->pushButtonHereShift,SIGNAL(clicked()),this,SLOT(HereShift()));
    connect(ui->pushButtonClearStep,SIGNAL(clicked()),this,SLOT(ClearStep()));
    connect(ui->pushButtonDeletePoint,SIGNAL(clicked()),this,SLOT(DeletePoint()));
    connect(this->udpClient,SIGNAL(progres(int)),this,SLOT(ShowProgress(int)));
    connect(ui->pushButtonOrientation,SIGNAL(clicked()),this,SLOT(Orientation()));
    connect(ui->pushButtonCalibration,SIGNAL(clicked()),this,SLOT(Calibration()));
    connect(ui->pushButtonWriteComment,SIGNAL(clicked()),this,SLOT(WriteComment()));
    connect(ui->pushButtonFazus,SIGNAL(clicked()),this,SLOT(StartFazus()));
    connect(ui->pushButtonMaxim,SIGNAL(clicked()),this,SLOT(StartMaxim()));
    connect(ui->pushButtonSetSpeed,SIGNAL(clicked()),this,SLOT(SetSpeed()));
    connect(ui->pushButtonSetMech,SIGNAL(clicked()),this,SLOT(SetMech()));
    connect(ui->pushButtonWorkZone,SIGNAL(clicked()),this,SLOT(SetWorkSpace()));
    connect(ui->pushButtonHomePos,SIGNAL(clicked()),this,SLOT(GoHome()));
    connect(ui->pushButtonResetandResume,SIGNAL(clicked()),this,SLOT(ResetandResume()));

    connect(ui->pushButtonJ1M,SIGNAL(clicked()),this,SLOT(J1Mmove()));
    connect(ui->pushButtonJ1P,SIGNAL(clicked()),this,SLOT(J1Pmove()));
    connect(ui->pushButtonJ2M,SIGNAL(clicked()),this,SLOT(J2Mmove()));
    connect(ui->pushButtonJ2P,SIGNAL(clicked()),this,SLOT(J2Pmove()));
    connect(ui->pushButtonJ3M,SIGNAL(clicked()),this,SLOT(J3Mmove()));
    connect(ui->pushButtonJ3P,SIGNAL(clicked()),this,SLOT(J3Pmove()));
    connect(ui->pushButtonJ4M,SIGNAL(clicked()),this,SLOT(J4Mmove()));
    connect(ui->pushButtonJ4P,SIGNAL(clicked()),this,SLOT(J4Pmove()));
    connect(ui->pushButtonJ5M,SIGNAL(clicked()),this,SLOT(J5Mmove()));
    connect(ui->pushButtonJ5P,SIGNAL(clicked()),this,SLOT(J5Pmove()));
    connect(ui->pushButtonJ6M,SIGNAL(clicked()),this,SLOT(J6Mmove()));
    connect(ui->pushButtonJ6P,SIGNAL(clicked()),this,SLOT(J6Pmove()));

    connect(ui->pushButtonMoveDegree,SIGNAL(clicked()),this,SLOT(moveDegree()));
    connect(ui->pushButtonDegreePlus,SIGNAL(clicked()),this,SLOT(stepDegreeP()));
    connect(ui->pushButtonDegreeMinus,SIGNAL(clicked()),this,SLOT(stepDegreeM()));
    connect(ui->pushButtonMmPlus,SIGNAL(clicked()),this,SLOT(stepMmPlus()));
    connect(ui->pushButtonMmMinus,SIGNAL(clicked()),this,SLOT(stepMmMinus()));

    connect(ui->SpinBoxXmove_2,SIGNAL(valueChanged(double)),this,SLOT(SetSpinX(double)));
    connect(ui->SpinBoxXmove,SIGNAL(valueChanged(double)),this,SLOT(SetSpinX(double)));
    connect(ui->SpinBoxYmove_2,SIGNAL(valueChanged(double)),this,SLOT(SetSpinY(double)));
    connect(ui->SpinBoxYmove,SIGNAL(valueChanged(double)),this,SLOT(SetSpinY(double)));
    connect(ui->SpinBoxZmove_2,SIGNAL(valueChanged(double)),this,SLOT(SetSpinZ(double)));
    connect(ui->SpinBoxZmove,SIGNAL(valueChanged(double)),this,SLOT(SetSpinZ(double)));
    connect(ui->SpinBoxZmove_3,SIGNAL(valueChanged(double)),this,SLOT(SetSpinZ(double)));
    connect(ui->SpinBoxZmove_4,SIGNAL(valueChanged(double)),this,SLOT(SetSpinZ(double)));

    connect(ui->pushButtonUzkOk,SIGNAL(clicked()),this,SLOT(NextPageButton()));

    connect(this,SIGNAL(quit()),qApp,SLOT(quit()));
    this->threadUDP->start();

    this->nameFolder =  QCoreApplication::applicationDirPath();
    ui->plainTextEditServis->appendPlainText(this->nameFolder);
    QCoreApplication::processEvents();
    // попробуем запилить здесь проверку работоспосбночти
    this->InitRobot();

    // FormHelp  fH;// = new WindowHelp;;

    // fH.show();
    QCoreApplication::processEvents();
    if (QFile::exists(".\\conf2.conf"))
    {
        QString temp;
        QFile file(".\\conf2.conf");
        file.open(QIODevice::ReadOnly | QIODevice::Text); // открываем только для чтения
        temp = file.readAll(); // записываем весь массив обратно в файл
        file.close();
        QStringList tempL = temp.split("\n");
        int n = tempL.size();
        ui->plainTextEditComment->clear();
        for (int i = 0;i<n;i++ )
        {
            if (i>0)
            {
                ui->lineEditName->setText(tempL[0]);
            }
            if (i>1)
            {
                ui->lineEditNumberDet->setText(tempL[1]);
            }
            if (i>2)
            {
                ui->lineEditNumberPart->setText(tempL[2]);
            }
            if (i>3)
            {
                ui->lineEditNumberSbor->setText(tempL[3]);
            }
            if (i>=4)
            {
                ui->plainTextEditComment->appendPlainText(tempL[i]);
            }
        }
    }
    if (!(QDir(".\\_BD_DEF\\").exists()==true) )
        {
            QDir().mkdir(".\\_BD_DEF\\");

        }

    this->ItemTitle = new QListWidgetItem;
    ItemTitle->setText("-------Основные этапы контроля-------");
    ItemTitle->setFlags(Qt::ItemIsUserCheckable); // вообще запрещает тыкать в надпись мышой, а не только менять галочку
    ui->listWidget->insertItem(0, ItemTitle);

    // QListWidgetItem *ItemReg = new QListWidgetItem;
    this->ItemReg = new QListWidgetItem;
    ItemReg->setText("Регистрация");
    ui->listWidget->insertItem(1, ItemReg);

    this->ItemTraect = new QListWidgetItem;
    ItemTraect->setText("Выбор траектории");
    ui->listWidget->insertItem(2, ItemTraect);

    this->ItemOrient = new QListWidgetItem;
    ItemOrient->setText("Ориентация акустической оси");
    ui->listWidget->insertItem(3, ItemOrient);

    this->ItemNastrUzk = new QListWidgetItem;
    ItemNastrUzk->setText("Выбор настройки УЗК");
    ui->listWidget->insertItem(4, ItemNastrUzk);

    this->ItemStartPoint = new QListWidgetItem;
    ItemStartPoint->setText("Выход в начальную точку");
    ui->listWidget->insertItem(5, ItemStartPoint);

    this->ItemControl = new QListWidgetItem;
    ItemControl->setText("Контроль");
    ui->listWidget->insertItem(6, ItemControl);

    this->ItemVspomag = new QListWidgetItem;
    ItemVspomag->setText("-------Вспомогательные действия-------");
    ItemVspomag->setFlags(Qt::ItemIsUserCheckable);
    ui->listWidget->insertItem(7, ItemVspomag);

    this->ItemManualMove = new QListWidgetItem;
    ItemManualMove->setText("Ручное перемеещние");
    ui->listWidget->insertItem(8, ItemManualMove);

    this->ItemService = new QListWidgetItem;
    ItemService->setText("Сервис");
    ui->listWidget->insertItem(9, ItemService);

    this->ItemOrient->setToolTip(" Для запуска процедуры ориентации расположите датчики так, чтобы видеть эхо сигнал от детали ");
    this->ItemStartPoint->setToolTip(" Поместите датчики под деталью, не ударив об дно ванны.");
    this->ItemReg->setToolTip(" Введите информацию о процедуре контроля и нажмите кнопку сохранить.");
    this->ItemTraect->setToolTip(" Выберете существую траекторию нажав кнопку Загрузить  или создайте новую, введя геом. размеры и нажав кнопку Создать.");
    this->ItemNastrUzk->setToolTip(" Перемещая обьект по выбранной ранее траектории можно подобрать оптимальные настрйоки УЗК.");
    this->ItemControl->setToolTip(" После нажатия на кнопку Старт контроля робот начнет движение по заданной траектории.");
    this->ItemManualMove->setToolTip(" Перемещение манипулятора в декартовых или угловых( по суставно) координатах.");
    this->ItemService->setToolTip(" Служебные настройки.");
}
int Widget::GetNastr()
{
    QString fileName;
    if(this->processMaxim->isOpen()) this->processMaxim->close();
    QString nameFolderNastr = ".\\_NASTR"; // потом имя необъодимо брать из максимовского файла конфигурации
    if(this->processFazus->isOpen())
    {
        this->processFazus->close();
    }
    fileName = QFileDialog::getOpenFileName(this, tr("Open File"), nameFolderNastr,
                                            tr("TXT(*.nst);"));
    if (fileName != "")
    {
        int er = this->fazusD->one_shot(fileName);
        if (er>256)  // если все хорошо функция возвращает число от 0 до 255
        {
            data = QDateTime::currentDateTime();
            ui->plainTextEdit->appendPlainText("["+data.toString("HH:mm")+"] "  + "Нет связи с фазусом");
            return 1;
        }
        this->fileNameNastr = fileName;
        // this->fazusD->Nastr(fileNameNastr);
        // this->udpClient->GetPointer(*(this->fazusD));
        // нет смысла здесь пихать настрйоки в фазус, так они будут затерты последующими открваниями фазуса для калибровки и позиционирования
        // но имя мы сорхранили после загрузки точек вызовем процедуру настройки

        fileName = fileName.split("/").last();
        //  fileName = fileName.split(".").first();
        ui->lNastrName->setText(fileName);

        //ui->labelNastr->setEnabled(false);
        //нужно подложить фазусу имя настройки
        QString tempName;
        tempName = ".\\_NASTR\\nstr.txt";
        QString nameNastr = this->nameFolder +"/system-nastr/exo1-1-20db";
        nameNastr.replace("/","\\\\");
        QFile file(tempName);
        file.open(QIODevice::WriteOnly | QIODevice::Text); // открываем только для чтения
        QByteArray qbTemp;
        qbTemp.append(nameNastr);
        file.write(qbTemp); // записываем весь массив обратно в файл
        file.close();
        this->bMassivButton[bhereshift]= true;
        this->SetButtonControl();
        if (!bRepeatControl)
        {
            //     QFont qFont;
            //     qFont = ui->labelStart->font();
            //     qFont.setBold(true);
            //     ui->labelStart->setFont(qFont);
            //     ui->labelCent->setEnabled(false);
            this->bMassivButton[bcalibrovka]= false;
            this->bMassivButton[bhereshift]= true;
            this->SetButtonControl();
            this->CloseButtonMotion();
            ui->pushButtonZpMove_3->setEnabled(true);
            ui->pushButtonZmMove_3->setEnabled(true);
            //нужно подложить фазусу имя настройки
            QString tempName;
            tempName = ".\\_NASTR\\nstr.txt";
            QFile file(tempName);
            file.open(QIODevice::WriteOnly | QIODevice::Text); // открываем только для чтения
            QByteArray qbTemp;
            //tempName = this->fileNameNastr.split(".").last(); // если первая - если в имени содержится точка отрезает по ней
            // если последняя - берет только расширение
            // tempName = this->fileNameNastr.replace(".nst","");
            tempName = this->fileNameNastr;
            tempName = tempName.replace(".nst","");

            //   tempName.replace("/",""
            tempName.replace("/", "\\\\");
            qbTemp.append(tempName);
            file.write(qbTemp); // записываем весь массив обратно в файл
            file.close();
            this->StartFazus();
            //this->ItemNastrUzk->setText(this->ItemNastrUzk->text()+ "  (Выполненно)");
            QFont qFont;
            qFont = this->ItemNastrUzk->font();
            qFont.setStrikeOut(true);
            this->ItemNastrUzk->setFont(qFont);
            //ui->stackedWidget->setCurrentIndex(4);   // переключать на другую вкладку - по кнопке далее

            if(!(this->processFazus->isOpen())) this->StartFazus();
        }
    }
    return 0;
}
void Widget::ErrorHandlerF (QString serr)
{
    data = QDateTime::currentDateTime();
    if ((serr.contains("Нет связи с фазусом получение данных"))&&(this->udpClient->Get_bDef()))
    { // во время сьема данных ошибка - тормозим робота
        rs10nComand comand;
        comand.instruction=stop;
        this->udpClient->AddComand(comand);
        //this->InitSystem(); // если команда стоп будет принята и атк попадем в завршения контроля и пророведем инициализацию
    }
    ui->plainTextEdit->appendPlainText("["+data.toString("HH:mm")+"] "  +serr);

    QMessageBox msgBoxF;
    msgBoxF.setWindowTitle("Управление фазусом");
    msgBoxF.setText(serr);
    msgBoxF.exec();
}
void Widget::ErrorHandlerUDP(QString serr)
{
    // ui->plainTextEdit->appendPlainText(serr);
    //  ui->pushButtonPump->setEnabled(true);
    //  ui->pushButtonSliv->setEnabled(true);
    //  ui->pushButtonCalibration->setEnabled(true);
    QMessageBox msgBox1;
    msgBox1.setWindowTitle("Отправка данных");
    msgBox1.setText(serr);
    msgBox1.exec();
}
void Widget::ShowScan()
{
    this->fazusD->OneShot();
    for (int i=0;i<1000;i++)
    {
        char ch[256];
        sprintf(ch,"%d;",this->fazusD->get_amp_def(i));

        ui->plainTextEdit->appendPlainText(ch);
    }
}

void Widget::SendComand()
{
    // служебная функция на закрытой вкладке. данные в обьект управления роботом передаются не безопасно!!!
    QString comand = ui->lineEditSend->text();
    int numlast = comand.lastIndexOf(";");
    int tmp=comand.size();
    if ((tmp-1)!=numlast)
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle(" Управление роботом RS10");
        QString str;
        str="нет точки с запятой в конце ";
        msgBox.setText(str);
        msgBox.exec();
    }
    else
    {
        QByteArray Data;
        Data.append(ui->lineEditSend->text());// запихиваем в пакет набранный текст
        this->udpClient->qbDataSend=Data;
        this->udpClient->bSend=true;
    }
}
void Widget::ReadAnswer()
{
    int flag = 1; // если сообщения обработанно - ставим флаг ноль и не выводим его. Если флаг остался один - выводим принятое сообщение
    data = QDateTime::currentDateTime();
    // tempName = "F:\\prog qt\\NEWWW\\WIN\\VOR7file\\voronezh\\def" + data.toString("HH-mm-dd-MM-yy");
    QFont qFont;
    QString str(this->udpClient->ReadMessage());
    if(str != 0)
    {
        str.replace(" ", "");// стираем пробелы иначе плохо опознаются ответы
        /**/if (str.contains("signal4set",Qt::CaseInsensitive))
        {
            ui->plainTextEdit->appendPlainText("["+data.toString("HH:mm")+"]"  +" Насос включен ");
            this->bMassivButton[bpump]= true;
            this->SetButtonControl();
            flag = 0;
        }
        if (str.contains("signal5set",Qt::CaseInsensitive))
        {
            ui->plainTextEdit->appendPlainText("["+data.toString("HH:mm")+"]"  +" Сливаем воду");
            this->bMassivButton[bsliv]= true;
            this->SetButtonControl();
            flag = 0;
        }
        if (str.contains("проверьтеуровеньводывванне",Qt::CaseInsensitive))
        {
            ui->plainTextEdit->appendPlainText("["+data.toString("HH:mm")+"]"  +" Проверьте уровень воды в ванне " );
            this->bMassivButton[bpump]= true;
            this->SetButtonControl();
            flag = 0;
        }
        if (str.contains("Достаточныйуровеньводывванне",Qt::CaseInsensitive))
        {
            ui->plainTextEdit->appendPlainText("["+data.toString("HH:mm")+"]"  +" Достаточный уровень воды в ванне " );
            flag = 0;
        }
        if (str.contains("Достаточныйуровеньводывбаке",Qt::CaseInsensitive))
        {
            ui->plainTextEdit->appendPlainText("["+data.toString("HH:mm")+"]"  +" Достаточный уровень воды в баке " );
            flag = 0;
        }
        if (str.contains("проверьтеуровеньводывбаке",Qt::CaseInsensitive))
        {
            ui->plainTextEdit->appendPlainText("["+data.toString("HH:mm")+"]"  +"Проверьте уровень воды в баке " );
            flag = 0;
        }
        if (str.contains("робототвечает",Qt::CaseInsensitive))
        {
            ui->plainTextEdit->appendPlainText("["+data.toString("HH:mm")+"]"  +" Есть соединение с роботом " );
            flag = 0;
        }
        if (str.contains("моторневключен",Qt::CaseInsensitive))
        {
            ui->plainTextEdit->appendPlainText("["+data.toString("HH:mm")+"]"  +" Не включен двигатель манипулятора. Возможно нажата одна из стоп кнопок " );
            flag = 0;
        }
        if (str.contains("моторвключен",Qt::CaseInsensitive))
        {
            ui->plainTextEdit->appendPlainText("["+data.toString("HH:mm")+"]"  +" Двигатель включен " );
            flag = 0;
        }
        if (str.contains("Роботнеотвечает",Qt::CaseInsensitive))
        {
            ui->plainTextEdit->appendPlainText("["+data.toString("HH:mm")+"]"  +" Нет связи с контроллером робота " );
            flag = 0;
        }
        if (str.contains("неудалосьначатьконтроль",Qt::CaseInsensitive))
        {
            ui->plainTextEdit->appendPlainText("["+data.toString("HH:mm")+"]"  +" Не удалось начать контроль. Проверьте соединение с контроллером робота " );
            this->bMassivButton[bstart]= true;
            this->bMassivButton[bgetUzk]= true;
            this->bMassivButton[bpump]= true;
            this->bMassivButton[bsliv]= true;
            this->bMassivButton[bnastrUZK]= true;
            this->bMassivButton[bviev]= true;
            this->SetButtonControl();
            flag = 0;
        }
        this->bMassivButton[bnastrUZK]= true;
        this->bMassivButton[bviev]= true;
        if (str.contains("nosignal14",Qt::CaseInsensitive))
        {
            QMessageBox msgBox;
            msgBox.setWindowTitle(" Управление роботом RS10");
            msgBox.setText(" Нет связи с шкафом автоматики");
            msgBox.exec();
            this->bMassivButton[bstart]= false;
            this->bMassivButton[bstop]= false;
            this->bMassivButton[borient]= false;
            this->bMassivButton[bpark]= false;
            this->SetButtonControl();
            //ui->tab_2->setEnabled(false);
            flag = 0;
        }
        if (str.contains("Signal14set",Qt::CaseInsensitive))
        {
            QMessageBox msgBox;
            msgBox.setWindowTitle(" Управление роботом RS10");
            msgBox.setText(" Возобновлена связь с шкафом автоматики");
            msgBox.exec();
            this->bMassivButton[bstart]= true;
            this->bMassivButton[bstop]= true;
            this->bMassivButton[borient]= true;
            this->bMassivButton[bpark]= true;
            this->SetButtonControl();
            //ui->tab_2->setEnabled(true);
            flag = 0;
        }
        if (str.contains("trajectory",Qt::CaseInsensitive))
        {
            // qFont = ui->labelNastr->font();

            // ui->labelNastr->setFont(qFont);

            //         ui->labelTra->setEnabled(false);

            //       ui->tabWidget->setCurrentIndex(2);

            // ui->labelNastr->setEnabled(false);
            //нужно подложить фазусу имя настройки
            QString tempName;
            tempName = ".\\_NASTR\\nstr.txt";
            QString nameNastr = this->nameFolder +"/system-nastr/exo1-1-20db";
            nameNastr.replace("/","\\\\");
            QFile file(tempName);
            file.open(QIODevice::WriteOnly | QIODevice::Text); // открываем только для чтения
            QByteArray qbTemp;
            qbTemp.append(nameNastr);
            file.write(qbTemp); // записываем весь массив обратно в файл
            file.close();
            this->bMassivButton[bcalibrovka]= true;
            this->bMassivButton[bgetUzk]= true;
            this->SetButtonControl();
            if (!bRepeatControl)
            {
                //  ui->tab_5->setEnabled(true);
                //         ui->tabWidget->setCurrentIndex(2);
                //       QFont qFont;
                //        qFont = ui->labelCent->font();
                //       qFont.setBold(true);
                //        ui->labelCent->setFont(qFont);
                if(!(this->processFazus->isOpen())) this->StartFazus();
            }

            flag = 0;
        }
        if (str.contains("startposithion",Qt::CaseInsensitive))
        {
            this->bMassivButton[bstart]= true;
            this->bMassivButton[bstop]= true;
            this->SetButtonControl();

           // this->OpenButtonMotion();
            QFont qFont;
            qFont = this->ItemStartPoint->font();
            qFont.setStrikeOut(true);
            this->ItemStartPoint->setFont(qFont);
            // ui->stackedWidget->setCurrentIndex(1);
            //после выставления в начальную позицию поидее должныв начать грузить тчоки
            //а потом уже переключим вкладку
            flag = 0;
        }
        if (str.contains("finisheddownload",Qt::CaseInsensitive))
        {
            //      ui->labelStart->setEnabled(false);
            if(this->processFazus->isOpen())
            {
                this->processFazus->close();
            }
            ui->progressBar->setHidden(true);
            ui->plainTextEdit->appendPlainText("["+data.toString("HH:mm")+"]" + " Загрузка закончена " );
            // при повторном контроле не грузим точки - следовательно не попадаем сюда
            this->bMassivButton[bstart]= true;
            this->bMassivButton[bnastrUZK]= true;
            this->SetButtonControl();
            this->bReadyControl = true;
            // ui->page_4->setEnabled(false);
            // ui->page_5->setEnabled(false);
            ui->stackedWidget->setCurrentIndex(5);
            // ui->tab_5->setEnabled(false);
            //  ui->tab_6->setEnabled(false);
            flag = 0;
        }
        if (str.contains("filedel",Qt::CaseInsensitive))
        {
            ui->plainTextEdit->appendPlainText("["+data.toString("HH:mm")+"]"+"Данные удалены " );
            flag = 0;
        }
        if (str.contains("orientationfinish",Qt::CaseInsensitive))
        {
            ui->plainTextEdit->appendPlainText("["+data.toString("HH:mm")+"]"  +" Фланец расположен паралельно опорной плите " );
            flag = 0;
        }
        if (str.contains("calibrovka-error",Qt::CaseInsensitive))
        {
            ui->plainTextEdit->appendPlainText("["+data.toString("HH:mm")+"]"  +" Ориентация не завершена. Проверьте местоположение втулки и датчиков и повторите процедуру. " );
            this->bMassivButton[bcalibrovka]= true;
            this->bMassivButton[bnastrUZK]= true;
            this->bMassivButton[bgetTraect]= true;
            this->bMassivButton[bgetUzk]= true;
            this->bMassivButton[borient]= true;
            this->bMassivButton[bpark]= true;
            this->bMassivButton[bpump]= true;
            this->bMassivButton[bsliv]= true;
            this->SetButtonControl();

            this->OpenButtonMotion();
        }
        if (str.contains("calibrationfinish",Qt::CaseInsensitive))
        {
            ui->plainTextEdit->appendPlainText("["+data.toString("HH:mm")+"]"  +" Ориентация завершена, робот готов к выводу в начальную точку " );
            ui->stackedWidget->setCurrentIndex(3);
            //this->ItemOrient->setText(this->ItemOrient->text()+ "  (Выполненно)");
            QFont qFont;
            qFont = this->ItemOrient->font();
            qFont.setStrikeOut(true);
            this->ItemOrient->setFont(qFont);
            this->bMassivButton[bgetUzk]= true;

            /**/
            this->bMassivButton[bcalibrovka]= false;
            //  this->bMassivButton[bhereshift]= true; раньше открывали после калибровки. теперь после выбора настройки
            this->SetButtonControl();

            this->CloseButtonMotion();

            ui->pushButtonZpMove_4->setEnabled(true);
            ui->pushButtonZmMove_4->setEnabled(true);
            ui->pushButtonZpMove_3->setEnabled(true);
            ui->pushButtonZmMove_3->setEnabled(true);
            //нужно подложить фазусу имя настройки
            /*
            // до этого сначала выбирали имя настрорйки а потом после калибровкии ее открывали
            // теперь вся настройка происходит после калибровки сл имя не известно
            // можно сохранять придыдущее
            QString tempName;
            tempName = ".\\_NASTR\\nstr.txt";
            QFile file(tempName);
            file.open(QIODevice::WriteOnly | QIODevice::Text); // открываем только для чтения
            QByteArray qbTemp;
            //tempName = this->fileNameNastr.split(".").last(); // если первая - если в имени содержится точка отрезает по ней
            // если последняя - берет только расширение
            // tempName = this->fileNameNastr.replace(".nst","");
            tempName = this->fileNameNastr;
            tempName=tempName.replace(".nst","");

            //   tempName.replace("/",""
            tempName.replace("/", "\\\\");
            qbTemp.append(tempName);
            file.write(qbTemp); // записываем весь массив обратно в файл
            file.close();
            */
            //this->StartFazus();
            /* */
            if(!(this->processFazus->isOpen())) this->StartFazus();
            flag = 0;
        }
        if (str.contains("Роботвстартовойпозиции",Qt::CaseInsensitive))
        {
            ui->plainTextEdit->appendPlainText("["+data.toString("HH:mm")+"]"  +" Робот в стартовой позиции " );
            flag = 0;
        }
        if (str.contains("Ошибкаквитированна",Qt::CaseInsensitive))
        {
            QMessageBox msgBox1;
            msgBox1.setWindowTitle(" Управление роботом RS10");
            msgBox1.setInformativeText("Ошибки сброшены, вернуть робота на исходную позицию?");
            msgBox1.setIcon(QMessageBox::Question);
            QPushButton *yes = msgBox1.addButton(tr("Да"), QMessageBox::ActionRole);
            QPushButton *no = msgBox1.addButton(tr("Оставить для продолжения"), QMessageBox::ActionRole);
            msgBox1.exec();
            if(msgBox1.clickedButton()== yes)
            {
                this->GoHome();
            }
            this->InitSystem();
            flag =0;
        }
        if (str.contains("Номерошибки",Qt::CaseInsensitive))
        {
            QMessageBox msgBox1;
            msgBox1.setWindowTitle(" Управление роботом RS10");
            msgBox1.setText(" Ошибка движения манипулятора ! ");
            msgBox1.setInformativeText(" После сброса ошибки робот закончит текущий шаг и остановится. Устраните причину аварии и нажмите сброс ошибки.");
            msgBox1.setIcon(QMessageBox::Question);
            QPushButton *yes = msgBox1.addButton(tr("Сброс ошибки"), QMessageBox::ActionRole);
            msgBox1.exec();
            if(msgBox1.clickedButton()== yes)
            {
                rs10nComand comand;
                comand.instruction = resetError;
                this->udpClient->AddComand(comand);
            }
            flag = 0;
        }
        if ((str.contains("movestart",Qt::CaseInsensitive))&&(this->udpClient->Get_bDef()))
        {
            if (!(this->fazusD->Get_bStatDef()))  // дополнительная проверка флага снятия данных
            {                                 //чтобы не запустить поток повторно по запоздавшему пакету от робота
                this->fazusD->StartDef(); // раньше дергали переменную напрямую
                if (this->threadF->isRunning())
                {
                    emit processStart();
                }
                else
                {
                    this->threadF->start(); // запуск поток сьема данных
                }

                ui->plainTextEdit->appendPlainText("["+data.toString("HH:mm")+"]"  + " Старт контроля " );
                // int n = this->fazusD->get_nDef();
                // ui->plainTextEdit->appendPlainText(QString::number(n)+ "Номер прозвучивания начало контроля");
                this->bMassivButton[bstart]= false;
                this->bMassivButton[bstop]= true;
                this->SetButtonControl();
                flag = 0;
            }
        }
        if (str.contains("контрольнарушен",Qt::CaseInsensitive))
        {
            this->bMassivButton[bnastrUZK]= true;
            this->bMassivButton[bviev]= true;
            this->SetButtonControl();
            //ui->tab_8->setEnabled(true);
            //ui->tab_7->setEnabled(true);
            // ui->tab_6->setEnabled(true);
            //ui->tab_2->setEnabled(true);
            //ui->tab_3->setEnabled(true);
            ui->checkBoxAscanWrite->setEnabled(true);
            rs10nComand comand;
            comand.instruction=stop;
            this->udpClient->AddComand(comand);
            // ui->labelNastr->setText("Настройка не выбрана");
            QMessageBox msgBox;
            msgBox.setWindowTitle(" Управление роботом RS10");
            msgBox.setText("Авария !");
            msgBox.setInformativeText(" Процедура контроля была прервана. Сохранить записанные данные?");
            msgBox.setIcon(QMessageBox::Question);
            QPushButton *save = msgBox.addButton(tr("Сохранить результаты"), QMessageBox::ActionRole);
            QPushButton *del = msgBox.addButton(tr("Удалить"), QMessageBox::ActionRole);
            msgBox.exec();
            if(msgBox.clickedButton()== save)
            {
                //надо сохранить файл с координатами в обьекте udpclient
                //  rs10nComand comand;
                comand.instruction = saveC;
                // comand.name7 = comment;
                comand.array8 = this->encodedComment;
                this->udpClient->AddComand(comand);
                QString fileName = this->fazusD->tempName ;
                fileName = fileName.split("//").last();
                ui->plainTextEdit->appendPlainText("["+data.toString("HH:mm")+"]"  + " Данные сохранены. Имя " + fileName );
            }
            else if(msgBox.clickedButton()== del)
            {
                //необходимо удалить файл с данными с фазуса в обьекта фазус
                // QString tempName = this->fazusD->nameFileDef;
                //ui->plainTextEdit->appendPlainText(tempName);
                // rs10nComand comand;
                comand.instruction = deleteFileDef;
                comand.name7 = this->fazusD->nameFileDef;
                this->udpClient->AddComand(comand);
            }
            //rs10nComand comand;
            //  comand.instruction=stop;
            this->udpClient->AddComand(comand);
            ui->plainTextEdit->appendPlainText("["+data.toString("HH:mm")+"]"  + " Авария.После сброса ошибок повторите процедуру заново." );
            this->InitSystem();  // удаляет точки могут быть пробелмы с высотой изделия и выходом из него
            // comand.instruction=stop;
            /// this->udpClient->AddComand(comand);
            flag = 0;
        }
        if (str.contains("контрольокончен",Qt::CaseInsensitive))
        {
            this->bMassivButton[bnastrUZK]= true;
            this->bMassivButton[bviev]= true;

            this->SetButtonControl();
            ui->page_3->setEnabled(true);
            ui->page_4->setEnabled(true);
            ui->page_5->setEnabled(true);
            ui->page_6->setEnabled(true);
            ui->page_7->setEnabled(true);

            ui->checkBoxAscanWrite->setEnabled(true);
            //  int n = this->fazusD->get_nDef();
            //  ui->labelNastr->setText("Настройка не выбрана");
            // ui->plainTextEdit->appendPlainText(QString::number(n)+ "Номер прозвучивания конец  контроля");
            QMessageBox msgBox;
            msgBox.setWindowTitle("Управление роботом RS10 ");
            msgBox.setText("Контроль окончен");
            msgBox.setInformativeText("Сохранить полученные данные?");
            msgBox.setIcon(QMessageBox::Question);
            QPushButton *save = msgBox.addButton(tr("Сохранить результаты"), QMessageBox::ActionRole);
            QPushButton *del = msgBox.addButton(tr("Удалить"), QMessageBox::ActionRole);
            msgBox.exec();
            if(msgBox.clickedButton()== save)
            {
                //надо сохранить файл с координатами в обьекте udpclient
                rs10nComand comand;
                comand.instruction = saveC;
                comand.array8 = this->encodedComment;
                // comand.name7 = comment;
                this->udpClient->AddComand(comand);
                QString fileName = this->fazusD->tempName ;
                fileName = fileName.split("//").last();
                ui->plainTextEdit->appendPlainText("["+data.toString("HH:mm")+"]"+" Данные сохранены. Имя " + fileName);
            }
            else if(msgBox.clickedButton()== del)
            {
                //необходимо удалить файл с данными с фазуса в обьекта фазус
                // QString tempName = this->fazusD->nameFileDef;
                //ui->plainTextEdit->appendPlainText(tempName);
                rs10nComand comand;
                comand.instruction = deleteFileDef;
                comand.name7 = this->fazusD->nameFileDef;
                this->udpClient->AddComand(comand);
            }
            ui->plainTextEdit->appendPlainText("["+data.toString("HH:mm")+"]"+" Контроль завершен. ");

            //qFont = ui->labelCent->font();
            //qFont.setBold(false);
            //ui->labelControl->setFont(qFont);
            //ui->labelControl->setEnabled(false);
            QMessageBox msgBox1;
            msgBox1.setWindowTitle(" Управление роботом RS10");
            msgBox1.setText("Контроль окончен");
            msgBox1.setInformativeText("Необходим повторный контроль этой втулки? (Без повторной ориентации, и без изменения траектории сканирования");
            msgBox1.setIcon(QMessageBox::Question);
            QPushButton *no = msgBox1.addButton(tr("Да"), QMessageBox::ActionRole);
            QPushButton *yes = msgBox1.addButton(tr("Нет"), QMessageBox::ActionRole);

            msgBox1.exec();
            if(msgBox1.clickedButton()== yes)
            {
                this->bRepeatControl = false;
                this->GoHome();
                QCoreApplication::processEvents();
                this->InitSystem();
                ui->pushButtonStartControl->setEnabled(true);
            }
            if(msgBox1.clickedButton()== no)
            {
                //qFont = ui->labelControl->font();
                // qFont.setBold(false);
                // ui->labelControl->setFont(qFont);
                // ui->labelControl->setEnabled(true);

                //  qFont = ui->labelNastr->font();
                //  qFont.setBold(true);
                //ui->labelNastr->setFont(qFont);
                // ui->labelNastr->setEnabled(true);
                this->bRepeatControl = true;
                this->RepeatControl();
                this->GoHome();
            }
            flag =0;
        }
        if(flag)
        {
            ui->plainTextEditServis->appendPlainText("["+data.toString("HH:mm")+"] "  + str);
        }
    }
}
void Widget::Pump()
{
    // ui->pushButtonPump->setEnabled(false);
    rs10nComand comand;
    comand.instruction=pump;
    this->udpClient->AddComand(comand);
}
void Widget::Sliv()
{
    //  ui->pushButtonSliv->setEnabled(false);
    //  this->udpClient->bSliv = true;
    rs10nComand comand;
    comand.instruction=sliv;
    this->udpClient->AddComand(comand);
}
void Widget::xMmove()
{
    float value = ((ui->SpinBoxXmove->value())*(-1));
    rs10nComand comand;
    comand.instruction=manualMoveXYZ;
    comand.parametr1 = value;
    comand.parametr2 = 0;
    comand.parametr3 = 0;
    comand.parametr4 = 0;
    comand.parametr5 = 0;
    comand.parametr6 = 0;
    this->udpClient->AddComand(comand);
}
void Widget::xPmove()
{
    float value = (ui->SpinBoxXmove->value());
    rs10nComand comand;
    comand.instruction=manualMoveXYZ;
    comand.parametr1 = value;

    comand.parametr2 = 0;
    comand.parametr3 = 0;
    comand.parametr4 = 0;
    comand.parametr5 = 0;
    comand.parametr6 = 0;
    this->udpClient->AddComand(comand);
}
void Widget::yPmove()
{
    float value = (ui->SpinBoxYmove->value());
    rs10nComand comand;
    comand.instruction=manualMoveXYZ;
    comand.parametr1 = 0;
    comand.parametr2 = value;
    comand.parametr3 = 0;
    comand.parametr4 = 0;
    comand.parametr5 = 0;
    comand.parametr6 = 0;
    this->udpClient->AddComand(comand);
}
void Widget::yMmove()
{
    float value = (ui->SpinBoxYmove->value())*(-1);
    rs10nComand comand;
    comand.instruction=manualMoveXYZ;
    comand.parametr1 = 0;
    comand.parametr2 = value;
    comand.parametr3 = 0;
    comand.parametr4 = 0;
    comand.parametr5 = 0;
    comand.parametr6 = 0;
    this->udpClient->AddComand(comand);
}

void Widget::zPmove()
{
    float value = (ui->SpinBoxZmove->value());
    rs10nComand comand;
    comand.instruction=manualMoveXYZ;
    comand.parametr1 = 0;
    comand.parametr2 = 0;
    comand.parametr3 = value;
    comand.parametr4 = 0;
    comand.parametr5 = 0;
    comand.parametr6 = 0;
    this->udpClient->AddComand(comand);
}
void Widget::zMmove()
{
    float value = (ui->SpinBoxZmove->value()*(-1));
    rs10nComand comand;
    comand.instruction=manualMoveXYZ;
    comand.parametr1 = 0;
    comand.parametr2 = 0;
    comand.parametr3 = value;
    comand.parametr4 = 0;
    comand.parametr5 = 0;
    comand.parametr6 = 0;
    this->udpClient->AddComand(comand);
}
void Widget::J1Pmove()
{
    float value = (ui->SpinBoxJ1move->value());
    rs10nComand comand;
    comand.instruction=manualMoveJ1J6;
    comand.parametr1 = value;
    comand.parametr2 = 0;
    comand.parametr3 = 0;
    comand.parametr4 = 0;
    comand.parametr5 = 0;
    comand.parametr6 = 0;
    this->udpClient->AddComand(comand);
}
void Widget::J1Mmove()
{
    float value = (ui->SpinBoxJ1move->value())*(-1);
    rs10nComand comand;
    comand.instruction=manualMoveJ1J6;
    comand.parametr1 = value;
    comand.parametr2 = 0;
    comand.parametr3 = 0;
    comand.parametr4 = 0;
    comand.parametr5 = 0;
    comand.parametr6 = 0;
    this->udpClient->AddComand(comand);
}
void Widget::J2Pmove()
{
    float value = (ui->SpinBoxJ2move->value());
    rs10nComand comand;
    comand.instruction=manualMoveJ1J6;
    comand.parametr1 = 0;
    comand.parametr2 = value;
    comand.parametr3 = 0;
    comand.parametr4 = 0;
    comand.parametr5 = 0;
    comand.parametr6 = 0;
    this->udpClient->AddComand(comand);
}
void Widget::J2Mmove()
{
    float value = (ui->SpinBoxJ2move->value())*(-1);
    rs10nComand comand;
    comand.instruction=manualMoveJ1J6;
    comand.parametr1 = 0;
    comand.parametr2 = value;
    comand.parametr3 = 0;
    comand.parametr4 = 0;
    comand.parametr5 = 0;
    comand.parametr6 = 0;
    this->udpClient->AddComand(comand);
}
void Widget::J3Pmove()
{
    float value = (ui->SpinBoxJ3move->value());
    rs10nComand comand;
    comand.instruction=manualMoveJ1J6;
    comand.parametr1 = 0;
    comand.parametr2 = 0;
    comand.parametr3 = value;
    comand.parametr4 = 0;
    comand.parametr5 = 0;
    comand.parametr6 = 0;
    this->udpClient->AddComand(comand);
}
void Widget::J3Mmove()
{
    float value = (ui->SpinBoxJ3move->value())*(-1);
    rs10nComand comand;
    comand.instruction=manualMoveJ1J6;
    comand.parametr1 = 0;
    comand.parametr2 = 0;
    comand.parametr3 = value;
    comand.parametr4 = 0;
    comand.parametr5 = 0;
    comand.parametr6 = 0;
    this->udpClient->AddComand(comand);
}
void Widget::J4Pmove()
{
    float value = (ui->SpinBoxJ4move->value());
    rs10nComand comand;
    comand.instruction=manualMoveJ1J6;
    comand.parametr1 = 0;
    comand.parametr2 = 0;
    comand.parametr3 = 0;
    comand.parametr4 = value;
    comand.parametr5 = 0;
    comand.parametr6 = 0;
    this->udpClient->AddComand(comand);
}
void Widget::J4Mmove()
{
    float value = (ui->SpinBoxJ4move->value())*(-1);
    rs10nComand comand;
    comand.instruction=manualMoveJ1J6;
    comand.parametr1 = 0;
    comand.parametr2 = 0;
    comand.parametr3 = 0;
    comand.parametr4 = value;
    comand.parametr5 = 0;
    comand.parametr6 = 0;
    this->udpClient->AddComand(comand);
}
void Widget::J5Pmove()
{
    float value = (ui->SpinBoxJ5move->value());
    rs10nComand comand;
    comand.instruction=manualMoveJ1J6;
    comand.parametr1 = 0;
    comand.parametr2 = 0;
    comand.parametr3 = 0;
    comand.parametr4 = 0;
    comand.parametr5 = value;
    comand.parametr6 = 0;
    this->udpClient->AddComand(comand);
}
void Widget::J5Mmove()
{
    float value = (ui->SpinBoxJ5move->value())*(-1);
    rs10nComand comand;
    comand.instruction=manualMoveJ1J6;
    comand.parametr1 = 0;
    comand.parametr2 = 0;
    comand.parametr3 = 0;
    comand.parametr4 = 0;
    comand.parametr5 = value;
    comand.parametr6 = 0;
    this->udpClient->AddComand(comand);
}
void Widget::J6Pmove()
{
    float value = (ui->SpinBoxJ6move->value());
    rs10nComand comand;
    comand.instruction=manualMoveJ1J6;
    comand.parametr1 = 0;
    comand.parametr2 = 0;
    comand.parametr3 = 0;
    comand.parametr4 = 0;
    comand.parametr5 = 0;
    comand.parametr6 = value;
    this->udpClient->AddComand(comand);
}
void Widget::J6Mmove()
{
    float value = (ui->SpinBoxJ6move->value())*(-1);
    rs10nComand comand;
    comand.instruction=manualMoveJ1J6;
    comand.parametr1 = 0;
    comand.parametr2 = 0;
    comand.parametr3 = 0;
    comand.parametr4 = 0;
    comand.parametr5 = 0;
    comand.parametr6 = value;
    this->udpClient->AddComand(comand);
}
void Widget::Stop()
{
    this->fazusD->StopDef(); // завершает цикл снятия данных и сохраняет результаты в файл
    // this->fazusD->flagNastr = 0;
    // ui->labelNastr->setText("Настройка не выбрана");
    rs10nComand comand;
    comand.instruction=stop;
    this->udpClient->AddComand(comand);
}
void Widget::Continue()
{
    rs10nComand comand;
    comand.instruction= continueWork;
    this->udpClient->AddComand(comand);
}
void Widget::ResetandResume()
{
    rs10nComand comand;
    comand.instruction= resetError;
    this->udpClient->AddComand(comand);
}
void Widget::StopE()
{
    rs10nComand comand;
    comand.instruction=stopE;
    this->udpClient->AddComand(comand);

    this->fazusD->StopDef();
    QByteArray Data;
    Data.append("98;");// запихиваем в пакет сигнал остановки
    this->udpClient->qbDataSend = Data;
    this->udpClient->bSend = true; // кривая отправлялка данных старым методом
}
int Widget::StartControl()
{    
    if(this->processMaxim->isOpen())
    {
        this->processMaxim->close();
    }
    QFont qFont;
    if (this->comment.size() == 0)
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle(" Управление роботом RS10");
        msgBox.setText("Не заполненны данные региcтрации.");
        msgBox.exec();
        return 1;
    }
    int er = this->fazusD->one_shot(fileNameNastr);
    if (er>256)
    {
        data = QDateTime::currentDateTime();
        ui->plainTextEdit->appendPlainText("["+data.toString("HH:mm")+"] "  +"Нет связи с фазусом");
        return 1;
    }
    this->fazusD->SetFileNameNastr(this->fileNameNastr);
    this->fazusD->Nastr(this->fileNameNastr);
    this->udpClient->GetPointer(*(this->fazusD));
    if (this->fazusD->flagNastr==1)
    {
        rs10nComand comand;
        comand.instruction = openPipe;
        this->udpClient->AddComand(comand);
    }
    else
    {
        QMessageBox msgBox1;
        msgBox1.setWindowTitle(" Управление роботом RS10");
        msgBox1.setText(" Не загружен файл настройки ");
        msgBox1.exec();
        return 1;
    }

    if (ui->checkBoxAscanWrite->isChecked())
    {
        this->fazusD->ascan_write = true;
    }
    else
    {
        this->fazusD->ascan_write = false;
    }
    if(this->processFazus->isOpen())
    {
        this->processFazus->close();
    }
    if(!(this->processMaxim->isOpen()))
    {
        this->processMaxim->start();
    }

    // this->udpClient->bControl = true;
    rs10nComand comand;
    comand.instruction = startMove;
    this->udpClient->AddComand(comand);
    this->bMassivButton[bstart]= false;
    this->bMassivButton[bgetUzk]= false;
    this->bMassivButton[bpump]= false;
    this->bMassivButton[bsliv]= false;
    this->bMassivButton[bnastrUZK]= false;
    this->bMassivButton[bviev]= false;

    this->SetButtonControl();
    //ui->tab_8->setEnabled(false);
    //ui->tab_7->setEnabled(false);
    // ui->tab_6->setEnabled(false);
    // ui->tab_2->setEnabled(false);
    // ui->tab_3->setEnabled(false);
    // ui->checkBoxAscanWrite->setEnabled(false);

    // qFont = ui->labelNastr->font();
    // qFont.setBold(false);
    //ui->labelNastr->setFont(qFont);
    //ui->labelNastr->setEnabled(false);
    return 0;
}
void Widget::OneShot1()
{
    // int *p;
    //  int a;
    //  a = this->pF->one_shot("F:\\prog qt\\NEWWW\\WIN\\proba\\proba\\acan.nst");
    //  ui->plainTextEdit->appendPlainText(QString::number(a));
    //ui->plainTextEdit->appendPlainText(QString::number(p));
}

void Widget::Ascan()
{
    /*
    if (ui->checkBoxAscanWrite->isChecked())
    {
        this->fazusD->ascan_write = true;
    }
    else
    {
       this->fazusD->ascan_write = false;
    }
    this->fazusD->bstop = true;
   if (this->threadF->isRunning())
    {
     emit processStart();
     //this->fazusD->Process(); // так процесс запускается но занимает форму
    }
    else
    {
      this->threadF->start();
    }
    ui->plainTextEdit->appendPlainText("Начат контроль");
    */
}
void Widget::AscanStop()
{
    /*
    this->fazusD->bstop = false;
    ui->plainTextEdit->appendPlainText("Контроль закончен");
    */
}
void Widget::OpenFileT(QString fileName)
{
    data = QDateTime::currentDateTime();
    if (fileName == "0")
    {
        QString nameFoldertrace = ".\\traectori";
        fileName = QFileDialog::getOpenFileName(this, tr("Open File"), nameFoldertrace,
                                                tr("TXT(*.txt);; Text Files (*.dat);;C++ Files (*.cpp *.h)"));

    }
    if (fileName != "")
    {
        rs10nComand comand;
        comand.instruction=openFileT;
        comand.name7 = fileName;
        this->udpClient->AddComand(comand);
        this->bMassivButton[bgetUzk]= true;
        this->SetButtonControl();
        fileName = fileName.split("/").last();
        ui->plainTextEdit->appendPlainText("["+data.toString("HH:mm")+"]" + " Открыт файл с траекторией " + fileName);

        fileName = fileName.split("/").last();

        ui->lTrarName->setText(fileName);

        //  this->ItemTraect->setText(this->ItemTraect->text()+ "  (Выполненно)");
        QFont qFont;
        qFont = this->ItemTraect->font();
        qFont.setStrikeOut(true);
        this->ItemTraect->setFont(qFont);
        ui->stackedWidget->setCurrentIndex(2);
    }
}

void Widget::HereShift()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle(" Управление роботом RS10");
    msgBox.setText("Датчики расположены в начальной точке(под деталью, с учетом выбранной траектории)?");
    //  msgBox.setInformativeText(" Процедура контроля была прервана. Сохранить записанные данные?");
    msgBox.setIcon(QMessageBox::Question);
    QPushButton *yes = msgBox.addButton(tr("Да"), QMessageBox::ActionRole);
    QPushButton *no = msgBox.addButton(tr("Нет"), QMessageBox::ActionRole);
    msgBox.exec();
    if(msgBox.clickedButton()== yes)
    {
        this->bMassivButton[bhereshift]= false;
        this->CloseButtonMotion();
        this->SetButtonControl();
        data = QDateTime::currentDateTime();
        rs10nComand comand;
        comand.instruction=hereShift;
        this->udpClient->AddComand(comand);
        ui->plainTextEdit->appendPlainText(("["+data.toString("HH:mm")+"]" + " Начальная точка задана " ));
        this->DownloadPoint();
    }
    else if(msgBox.clickedButton()== no)
    {
        // окно закрылось оператор думает и двигает датчики дальше
    }
}
void Widget::Here()
{
    rs10nComand comand;
    comand.instruction=here;
    this->udpClient->AddComand(comand);
}
void Widget::ClearStep()
{
    rs10nComand comand;
    comand.instruction=clearStep;
    this->udpClient->AddComand(comand);
}
void Widget::DeletePoint()
{
    rs10nComand comand;
    QFont  qFont;
    comand.instruction = deletePoint;
    this->udpClient->AddComand(comand);
    /*
    qFont = ui->labelControl->font();
    qFont.setBold(false);
    ui->labelControl->setFont(qFont);
    ui->labelControl->setEnabled(true);

    qFont = ui->labelNastr->font();
    qFont.setBold(false);
    ui->labelNastr->setFont(qFont);
    ui->labelNastr->setEnabled(true);

    qFont = ui->labelStart->font();
    qFont.setBold(false);
    ui->labelStart->setFont(qFont);
    ui->labelStart->setEnabled(true);

    qFont = ui->labelCent->font();
    qFont.setBold(false);
    ui->labelCent->setFont(qFont);
    ui->labelCent->setEnabled(true);

  //  ui->pushButtonCalibration->setEnabled(true);

    // qFont = ui->labelTra->font();
    // qFont.setBold(false);
    // ui->labelTra->setFont(qFont);
    ui->labelTra->setEnabled(true);
    */
}
void Widget::DownloadPoint()
{
    data = QDateTime::currentDateTime();
    rs10nComand comand;
    comand.instruction = downloadPoint;
    this->udpClient->AddComand(comand);
    ui->progressBar->setValue(0);
    ui->progressBar->setHidden(false);
    ui->plainTextEdit->appendPlainText("["+data.toString("HH:mm")+"]" + " Загрузка траектории");
}

void Widget::ShowProgress( int step)
{
    ui->progressBar->setValue(ui->progressBar->value() + step);

}
void Widget::Orientation()
{
    rs10nComand comand;
    comand.instruction = orientation;
    this->udpClient->AddComand(comand);
}
void Widget::SetSpeed()
{
    rs10nComand comand;
    comand.instruction = setSpeed;
    comand.parametr1 = ui->spinBoxSpeed->value();
    this->udpClient->AddComand(comand);
}
void Widget::SetMech()
{
    rs10nComand comand;
    comand.instruction = setMechScan;
    comand.parametr1 = ui->spinBoxMech->value();
    this->udpClient->AddComand(comand);
}
void Widget::SetWorkSpace()
{
    this->udpClient->SetWorkSpaceINpc(ui->doubleSpinBoxWZx1->value(),ui->doubleSpinBoxWZy1->value(),ui->doubleSpinBoxWZz1->value(),ui->doubleSpinBoxWZx2->value(),ui->doubleSpinBoxWZy2->value(),ui->doubleSpinBoxWZz2->value());
    rs10nComand comand;
    comand.instruction = setWorkSpace;
    comand.parametr1 = ui->doubleSpinBoxWZx1->value();
    comand.parametr2 = ui->doubleSpinBoxWZy1->value();
    comand.parametr3 = ui->doubleSpinBoxWZz1->value();
    comand.parametr4 = ui->doubleSpinBoxWZx2->value();
    comand.parametr5 = ui->doubleSpinBoxWZy2->value();
    comand.parametr6 = ui->doubleSpinBoxWZz2->value();
    this->udpClient->AddComand(comand);
}

void Widget::GetWorkSpace(float *x1,float *y1,float *z1,float *x2,float *y2,float *z2)
{
    rs10nComand comand;
    comand.instruction = getWorkSpace;
    comand.parametr1 =   *x1;
    comand.parametr2 =   *y1;
    comand.parametr3 =   *z1;
    comand.parametr4 =   *x2;
    comand.parametr5 =   *y2;
    comand.parametr6 =   *z2;
    this->udpClient->AddComand(comand);
}

void Widget::CreateHub()
{
    if (!(QDir(".\\traectori\\").exists()==true) )
    {
        QDir().mkdir(".\\traectori\\");
    }
    data = QDateTime::currentDateTime();
    //QString name ="vtulka-" ;
    QString name ="vtulka-" ;
    name = name + QString::number(ui->spinBoxOutD->value()) + "-" + QString::number(ui->spinBoxInD->value()) + "-" + QString::number(ui->spinBoxHight->value()) + "-" +QString::number(ui->doubleSpinBoxStepT->value())+".txt";
    name = name.prepend(".\\traectori\\");
    this->writeTrace(name.toUtf8().data(),ui->spinBoxOutD->value(),ui->spinBoxInD->value(),ui->spinBoxHight->value(),ui->doubleSpinBoxStepT->value());
    ui->plainTextEdit->appendPlainText("["+data.toString("HH:mm")+"] Файл траектории " + name + " создан " );
    QMessageBox msgBox;
    msgBox.setWindowTitle(" Создание траектории движения");
    //msgBox.setText("Траектория " + name +"создана. Выбрать ее для дальнейшей работы?");
    msgBox.setInformativeText("Траектория " + name +"  создана. Выбрать ее для дальнейшей работы?");
    msgBox.setIcon(QMessageBox::Question);
    QPushButton *yes = msgBox.addButton(tr("Да, выбрать эту траекторию"), QMessageBox::ActionRole);
    QPushButton *no = msgBox.addButton(tr("Нет(необоходимо создать или загрузить новую траекторию"), QMessageBox::ActionRole);
    msgBox.exec();
    if(msgBox.clickedButton()== yes)
    {
        this->OpenFileT(name);
    }
    else if(msgBox.clickedButton()== no)
    {

    }
}

void Widget::writeTrace(const char *chFileName, 		// Название файла
                        float 	d_H,					// Внешений диаметр
                        float 	d_L,					// Внутренний диаметр
                        float	H,						// Высота
                        float 	stepscan				// Шаг сканирования по внешнему диаметру
                        )
{
    float 	r = (d_H + d_L)/4;					// Средний радиус
    char 	list[256];							// Массив для создания строки
    int 	istepangl = 0,						// Шаг для доворота
            fh;									// Хэндлер файла
    unsigned int i = 0;							// Счетчик
    _sopen_s(&fh, chFileName, _O_RDWR | _O_TRUNC | _O_CREAT, _SH_DENYNO, _S_IREAD | _S_IWRITE);
    //внесем изменение - сдвиг по углу O  чтобы компенсировать установку фланца
    //float oShift = this->udpClient->Get_OinstrShift();  // траектория и так сдвигаеться перед загрузкой
    for(i = 0; i < (d_H*M_PI/(stepscan))*1.03; i++)  // домножили для перекрытия
    {
        sprintf_s(list, sizeof(list), "1;%5.3f;%5.3f;%5.3f;%5.3f;%5.3f;%5.3f;\n",
                  r*cos(i*stepscan*360*(2*M_PI/360)/(d_H*M_PI)),
                  r*sin(i*stepscan*360*(2*M_PI/360)/(d_H*M_PI)),
                  0.0,
                  // oShift + istepangl*360/(d_H*M_PI/(stepscan)),
                  istepangl*360/(d_H*M_PI/(stepscan)),
                  180.0,
                  0.0);
        _write(fh, list, strlen(list));
        sprintf_s(list, sizeof(list), "1;%5.3f;%5.3f;%5.3f;%5.3f;%5.3f;%5.3f;\n",
                  r*cos(i*stepscan*360*(2*M_PI/360)/(d_H*M_PI)),
                  r*sin(i*stepscan*360*(2*M_PI/360)/(d_H*M_PI)),
                  H,
                  //oShift + istepangl*360/(d_H*M_PI/(stepscan)),
                  istepangl*360/(d_H*M_PI/(stepscan)),
                  180.0,
                  0.0);
        _write(fh, list, strlen(list));
        i++;
        istepangl++;
        sprintf_s(list, sizeof(list), "1;%5.3f;%5.3f;%5.3f;%5.3f;%5.3f;%5.3f;\n",
                  r*cos(i*stepscan*360*(2*M_PI/360)/(d_H*M_PI)),
                  r*sin(i*stepscan*360*(2*M_PI/360)/(d_H*M_PI)),
                  H,
                  //oShift + istepangl*360/(d_H*M_PI/(stepscan)),
                  istepangl*360/(d_H*M_PI/(stepscan)),
                  180.0,
                  0.0);
        _write(fh, list, strlen(list));
        sprintf_s(list, sizeof(list), "1;%5.3f;%5.3f;%5.3f;%5.3f;%5.3f;%5.3f;\n",
                  r*cos(i*stepscan*360*(2*M_PI/360)/(d_H*M_PI)),
                  r*sin(i*stepscan*360*(2*M_PI/360)/(d_H*M_PI)),
                  0.0,
                  //oShift +  istepangl*360/(d_H*M_PI/(stepscan)),
                  istepangl*360/(d_H*M_PI/(stepscan)),
                  180.0,
                  0.0);
        _write(fh, list, strlen(list));
        istepangl++;
    }
    sprintf_s(list, sizeof(list), "1;%5.3f;%5.3f;%5.3f;%5.3f;%5.3f;%5.3f;\n",
              r*cos(i*stepscan*360*(2*M_PI/360)/(d_H*M_PI)),
              r*sin(i*stepscan*360*(2*M_PI/360)/(d_H*M_PI)),
              0.0,
              //oShift + istepangl*360/(d_H*M_PI/(stepscan)),
              istepangl*360/(d_H*M_PI/(stepscan)),
              180.0,
              0.0);
    _write(fh, list, strlen(list));
    _close(fh);
}
void Widget::Calibration()
{
    data = QDateTime::currentDateTime();
    QMessageBox msgBox;
    msgBox.setWindowTitle(" Управление роботом RS10");
    msgBox.setText("Датчики расположены в районе метки 0 градусов по обе стороны от обьекта контроля, так что виден эхо сгинал от внешней грани ?");
    //  msgBox.setInformativeText(" Процедура контроля была прервана. Сохранить записанные данные?");
    msgBox.setIcon(QMessageBox::Question);
    QPushButton *yes = msgBox.addButton(tr("Да, можно начинать ориентацию"), QMessageBox::ActionRole);
    QPushButton *no = msgBox.addButton(tr("Нет"), QMessageBox::ActionRole);
    msgBox.exec();
    if(msgBox.clickedButton()== yes)
    {
        ui->plainTextEdit->appendPlainText("["+data.toString("HH:mm")+"]"  +" Началась процедура ориентации ");
        this->bMassivButton[bcalibrovka]= false;
        this->bMassivButton[bnastrUZK]= false;
        this->bMassivButton[bgetTraect]= false;
        this->bMassivButton[bgetUzk]= false;
        this->bMassivButton[borient]= false;
        this->bMassivButton[bpark]= false;
        this->bMassivButton[bpump]= false;
        this->bMassivButton[bsliv]= false;
        this->CloseButtonMotion();

        this->SetButtonControl();
        if(this->processFazus->isOpen())
        {
            this->processFazus->close();
        }
        this->udpClient->GetPointer(*(this->fazusD));
        rs10nComand comand;
        comand.instruction = calibration;
        this->udpClient->AddComand(comand);
    }
    else if(msgBox.clickedButton()== no)
    {
        // оператор решил что калибровку начинать рано, окно пропало он думает дальше
    }
}
void Widget::GoHome()
{
    rs10nComand comand;
    comand.instruction = goHome;
    this->udpClient->AddComand(comand);
    SleeperThread::msleep(100);
    this->ClearStep();
}
void Widget::GoFirstPoint()
{
    rs10nComand comand;
    comand.instruction = goFirstPoint;
    this->udpClient->AddComand(comand);
}
void Widget::WriteComment()
{

    // тест исключений
    /*

    try
    {
     int a[10];
     for (int x =10; x>-5;x--)
     {
         a[x] = x;
     }

     qDebug ("/null");
    }
    catch(const std::out_of_range& e)
    {


        qDebug()<<e.what();
        qDebug ("catch");
        QMessageBox msgBox;
        msgBox.setWindowTitle(" Исключение");
        msgBox.setText(" dgsdgs");
        msgBox.exec();
    }
    /**/

    this->comment.clear();
    QString stmp = ui->lineEditName->text();
    stmp.replace(QString("&"), QString(""));
    comment.append("{fieldname:" + this->chtosyQt(stmp)  +"}");
    comment.append("{ndet:" + this->chtosyQt(ui->lineEditNumberDet->text()) +"}");
    comment.append("{nassemplypart:" + this->chtosyQt(ui->lineEditNumberSbor->text()) +"}");
    comment.append("{nparty:" + this->chtosyQt(ui->lineEditNumberPart->text()) +"}");
    comment.append("{noperation:" + this->chtosyQt(ui->lineEditNumberOp->text()) +"}");
    comment.append("{fieldcomment:" + this->chtosyQt(ui->plainTextEditComment->toPlainText()) +"}");
    // записываем его потом в udpclient в функции SaveC

    QTextCodec *codec = QTextCodec::codecForName("Windows-1251");
    this->encodedComment = codec->fromUnicode(comment);

    QFont qFont;
    qFont = this->ItemReg->font();
    qFont.setStrikeOut(true);
    this->ItemReg->setFont(qFont);

    QByteArray temp;
    temp.append(comment);
    QFile file(".\\conf2.conf");
    file.open(QIODevice::WriteOnly | QIODevice::Text);
    file.write(temp); // записываем весь массив обратно в файл
    file.close();

    ui->stackedWidget->setCurrentIndex(1);
}
void Widget::StartFazus()
{
    if(this->processFazus->isOpen())
    {
        this->processFazus->close();
    }
    else
    {
        this->processFazus->start();
    }
}
void Widget::StartMaxim()
{
    if (this->processMaxim->isOpen())
    {
        this->processMaxim->close();
    }
    else
    {
        this->processMaxim->start();
    }
}
void Widget::OpenHelp()
{
    // FormHelp  fH;// = new WindowHelp;;
    // fH.show();
}
void Widget::InitSystem()
{
    // сброс всего для повторного контроля
    // надо - отдать все кнопки. вырубить все процессы
    // сбросить все настроки стереть точки из робота

    QFont qFont;
    for (int i = 1;i<   ui->listWidget->count(); i++)
    {
        qFont = ui->listWidget->item(i)->font();
        qFont.setStrikeOut(false);
        ui->listWidget->item(i)->setFont(qFont);
    }

    this->DeletePoint();

    //ui->tabWidget->setCurrentIndex(0);
    //   ui->tab_2->setEnabled(true);
    this->OpenButtonMotion();

    this->bMassivButton[bnastrUZK] = true;
    this->bMassivButton[bviev] = true;
    this->bMassivButton[bgetTraect] = true;
    this->bMassivButton[bgetUzk] = false;
    this->bMassivButton[bcalibrovka] = false;
    this->bMassivButton[bresume] = true;
    this->bMassivButton[bhereshift] = false;
    this->bMassivButton[bstart] = false;
    this->bMassivButton[bstop] = false;
    this->bMassivButton[bsliv] = true;
    this->bMassivButton[bpump] = true;
    this->bMassivButton[borient] = true;
    this->bMassivButton[borient] = true;
    this->bMassivButton[bpark] = true;
    this->SetButtonControl();

    this->comment.clear(); // стер данные регистрации
    ui->lTrarName->setText("Файл не выбран");
    ui->lNastrName->setText("Настройка не выбрана");
    /* if (sok)
    {
      sok =  this->fazusD->Stop_fazus();
      if(sok)
      {
          QMessageBox msgBox;
          msgBox.setWindowTitle(" Управление роботом RS10");
          QString str;
          str=" Не удалсоь закрыть фазус ";
          msgBox.setText(str);
          msgBox.exec();
      }
    }
    */
}
void Widget::RepeatControl()
{
    // повторный контроль не удаляя траекторию
    QFont qFont;
    this->ClearStep();
    //this->processMaxim->close();
    //   this->processFazus->close();
    //   this->processFazus->waitForFinished(500);
    // this->processMaxim->waitForFinished(500);
    rs10nComand comand;
    comand.instruction = init;
    this->udpClient->AddComand(comand);

    this->bMassivButton[bgetUzk]= true;
    this->bMassivButton[bstart]= true;
    this->bMassivButton[bstop]= false;

    this->SetButtonControl();
    qFont = this->ItemNastrUzk->font();
    qFont.setStrikeOut(false);
    this->ItemNastrUzk->setFont(qFont);
}
void Widget::HideTab()
{
    ui->doubleSpinBoxWZx1->setEnabled(false);
    ui->doubleSpinBoxWZx1->setHidden(true);
    ui->doubleSpinBoxWZx2->setEnabled(false);
    ui->doubleSpinBoxWZx2->setHidden(true);
    ui->doubleSpinBoxWZy1->setEnabled(false);
    ui->doubleSpinBoxWZy1->setHidden(true);
    ui->doubleSpinBoxWZy2->setEnabled(false);
    ui->doubleSpinBoxWZy2->setHidden(true);
    ui->doubleSpinBoxWZz1->setEnabled(false);
    ui->doubleSpinBoxWZz1->setHidden(true);
    ui->doubleSpinBoxWZz2->setEnabled(false);
    ui->doubleSpinBoxWZz2->setHidden(true);
    ui->pushButtonWorkZone->setEnabled(false);
    ui->pushButtonWorkZone->setHidden(true);
    ui->pushButtonSend->setEnabled(false);
    ui->pushButtonSend->setHidden(true);
    ui->pushButtonSetSpeed->setEnabled(false);
    ui->pushButtonSetSpeed->setHidden(true);
    ui->pushButtonSetMech->setEnabled(false);
    ui->pushButtonSetMech->setHidden(true);
    ui->pushButtonResetandResume->setEnabled(false);
    ui->pushButtonResetandResume->setHidden(true);
    ui->pushButtonHere->setEnabled(false);
    ui->pushButtonHere->setHidden(true);
    ui->pushButtonHide->setHidden(true);
    ui->pushButtonClearStep->setHidden(true);
    ui->pushButtonDeletePoint->setHidden(true);
    ui->pushButtonStopFazus->setHidden(true);
    ui->pushButtonPipe->setHidden(true);
    ui->label_16->setHidden(true);
    ui->plainTextEditServis->setHidden(true);
    ui->lineEditSend->setHidden(true);
    ui->spinBoxMech->setHidden(true);
    ui->spinBoxSpeed->setHidden(true);
    ui->lineEditPass->clear();
}
void Widget::ShowTab()
{
    ui->doubleSpinBoxWZx1->setEnabled(true);
    ui->doubleSpinBoxWZx1->setHidden(false);
    ui->doubleSpinBoxWZx2->setEnabled(true);
    ui->doubleSpinBoxWZx2->setHidden(false);
    ui->doubleSpinBoxWZy1->setEnabled(true);
    ui->doubleSpinBoxWZy1->setHidden(false);
    ui->doubleSpinBoxWZy2->setEnabled(true);
    ui->doubleSpinBoxWZy2->setHidden(false);
    ui->doubleSpinBoxWZz1->setEnabled(true);
    ui->doubleSpinBoxWZz1->setHidden(false);
    ui->doubleSpinBoxWZz2->setEnabled(true);
    ui->doubleSpinBoxWZz2->setHidden(false);
    ui->pushButtonWorkZone->setEnabled(true);
    ui->pushButtonWorkZone->setHidden(false);
    ui->pushButtonSend->setEnabled(true);
    ui->pushButtonSend->setHidden(false);
    ui->pushButtonSetSpeed->setEnabled(true);
    ui->pushButtonSetSpeed->setHidden(false);
    ui->pushButtonSetMech->setEnabled(true);
    ui->pushButtonSetMech->setHidden(false);
    ui->pushButtonResetandResume->setEnabled(true);
    ui->pushButtonResetandResume->setHidden(false);
    ui->pushButtonHere->setEnabled(true);
    ui->pushButtonHere->setHidden(false);
    ui->pushButtonHide->setHidden(false);
    ui->pushButtonClearStep->setHidden(false);
    ui->pushButtonDeletePoint->setHidden(false);
    ui->pushButtonStopFazus->setHidden(false);
    ui->pushButtonPipe->setHidden(false);
    ui->label_16->setHidden(false);
    ui->plainTextEditServis->setHidden(false);
    ui->lineEditSend->setHidden(false);
    ui->spinBoxMech->setHidden(false);
    ui->spinBoxSpeed->setHidden(false);
}
void Widget::ShowService()
{
    QString str;
    str = ui->lineEditPass->text();
    int pass;
    pass = str.toInt();
    if (pass == 1251)
    {
        this->ShowTab();
    }
    else
    {
        ui->lineEditPass->clear();
        QMessageBox msgBoxF;
        msgBoxF.setWindowTitle("Служебное");
        msgBoxF.setText(" Неверный пароль");
        msgBoxF.exec();
    }
}
void Widget::InitRobot()
{
    rs10nComand comand;
    comand.instruction=testHere;
    this->udpClient->AddComand(comand);
    data = QDateTime::currentDateTime();
    // QString nameFolder =  QCoreApplication::applicationDirPath();
    QString nameNastr;
    //int er = this->fazusD->one_shot(".\\_NASTR\\exo1-1-5db.nst");

    nameNastr =this->nameFolder +"/system-nastr/exo1-1-5db.nst";
    nameNastr.replace("/","\\\\");

    ui->plainTextEditServis->appendPlainText(nameNastr);
    // nameNastr ="F:\\prog qt\\NEWWW\\WIN\\DEF7\\build-defectoscop7-Desktop_Qt_5_5_0_MSVC2010_32bit-Release\\_NASTR\\exo1-1-5db.nst";
    int er = this->fazusD->one_shot(nameNastr);

    if ( er > 256)
    {
        ui->plainTextEdit->appendPlainText("["+data.toString("HH:mm")+"]"  +" Нет связи с фазусом " );
    }
    else
    {
        ui->plainTextEdit->appendPlainText("["+data.toString("HH:mm")+"]" +" Есть связь с фазусом " );
    }
    //  this->fazusD->Stop_fazus(); //попробуем закрывать внутри функции
    this->udpClient->SetWorkSpaceINpc(ui->doubleSpinBoxWZx1->value(),ui->doubleSpinBoxWZy1->value(),ui->doubleSpinBoxWZz1->value(),ui->doubleSpinBoxWZx2->value(),ui->doubleSpinBoxWZy2->value(),ui->doubleSpinBoxWZz2->value());
}
void Widget::StopFazus()
{
    if (this->fazusD->Stop_fazus())
    {
        ui->plainTextEditServis->appendPlainText("Не закрывается");
    }
}
void Widget::TestPipe()
{
    rs10nComand comand;
    comand.instruction = testPipe;
    this->udpClient->AddComand(comand);
}
void Widget::SetButtonControl()
{
    // была задумка использовать какие то макски, готовые сотояния для открытия закрытия кнопок
    for (int i = 0; i<13;i++) // размер массива не контролируется!!!
    {
        switch (i) {
        case bnastrUZK:
            ui->pushButtonFazus->setEnabled(this->bMassivButton[bnastrUZK]);
            break;
        case bviev:
            ui->pushButtonMaxim->setEnabled(this->bMassivButton[bviev]);
            break;
        case bgetTraect:
            ui->pushButtonLoadT->setEnabled(this->bMassivButton[bgetTraect]);
            break;
        case bgetUzk:
            ui->pushButtonNastr->setEnabled(this->bMassivButton[bgetUzk]);
            break;
        case bcalibrovka:
            ui->pushButtonCalibration->setEnabled(this->bMassivButton[bcalibrovka]);
            break;
        case bresume:
            ui->pushButtonContinue->setEnabled(this->bMassivButton[bresume]);
            break;
        case bhereshift:
            ui->pushButtonHereShift->setEnabled(this->bMassivButton[bhereshift]);
            break;
        case bstart:
            ui->pushButtonStartControl->setEnabled(this->bMassivButton[bstart]);
            break;
        case bstop:
            ui->pushButtonStop->setEnabled(this->bMassivButton[bstop]);
            break;
        case bsliv:
            ui->pushButtonSliv->setEnabled(this->bMassivButton[bsliv]);
            break;
        case bpump:
            ui->pushButtonPump->setEnabled(this->bMassivButton[bpump]);
            break;
        case borient:
            ui->pushButtonOrientation->setEnabled(this->bMassivButton[borient]);
            break;
        case bpark:
            ui->pushButtonHomePos->setEnabled(this->bMassivButton[bpark]);
            break;
        default:
            break;
        }
    }
}
void Widget::SetSpinX( double valueX)
{
    if ((valueX != (ui->SpinBoxXmove->value()))||(valueX != (ui->SpinBoxXmove_2->value())))
    {
        ui->SpinBoxXmove->setValue(valueX);
        ui->SpinBoxXmove_2->setValue(valueX);
    }
}
void Widget::SetSpinY( double valueY)
{
    if ((valueY != (ui->SpinBoxYmove->value()))||(valueY != (ui->SpinBoxYmove_2->value())))
    {
        ui->SpinBoxYmove->setValue(valueY);
        ui->SpinBoxYmove_2->setValue(valueY);
    }
}
void Widget::SetSpinZ( double valueZ)
{
    if ((valueZ != (ui->SpinBoxZmove->value()))||(valueZ != (ui->SpinBoxZmove_2->value()))||(valueZ != (ui->SpinBoxZmove_3->value())||(valueZ != (ui->SpinBoxZmove_4->value()))))
    {
        ui->SpinBoxZmove->setValue(valueZ);
        ui->SpinBoxZmove_2->setValue(valueZ);
        ui->SpinBoxZmove_3->setValue(valueZ);
        ui->SpinBoxZmove_4->setValue(valueZ);
    }
}
void Widget::on_pushButtonComment_clicked()
{
    ui->plainTextEditComment->clear();
}
void Widget::stepDegreeM()
{
    rs10nComand comand;
    comand.instruction = move2degree;
    comand.parametr1 = ui->doubleSpinBoxStepDegree->value();
    comand.parametr2 = -1;
    this->udpClient->AddComand(comand);
}
void Widget::stepDegreeP()
{
    rs10nComand comand;
    comand.instruction = move2degree;
    comand.parametr1 = ui->doubleSpinBoxStepDegree->value();
    comand.parametr2 = 1;
    this->udpClient->AddComand(comand);
}
void Widget::moveDegree()
{
    rs10nComand comand;
    comand.instruction = move2degree;
    comand.parametr1 = ui->doubleSpinBoxFinishDegree->value();
    comand.parametr2 = 0;
    this->udpClient->AddComand(comand);
}
void Widget::stepMmMinus()
{
    rs10nComand comand;
    comand.instruction = move2point;
    comand.parametr1 = ui->doubleSpinBoxStepMm->value();
    comand.parametr2 = -1;
    this->udpClient->AddComand(comand);
}
void Widget::stepMmPlus()
{
    rs10nComand comand;
    comand.instruction = move2point;
    comand.parametr1 = ui->doubleSpinBoxStepMm->value();
    comand.parametr2 = 1;
    this->udpClient->AddComand(comand);
}
QString Widget::chtosyQt(QString str)
{
    str.replace(QString("&"), QString("&amp;"));
    str.replace(QString(";"), QString("&sem;"));
    str.replace(QString("{"), QString("&bcb;"));
    str.replace(QString("}"), QString("&ecb;"));
    str.replace(QString("("), QString("&bb;"));
    str.replace(QString(")"), QString("&eb;"));
    str.replace(QString(":"), QString("&co;"));
    str.replace(QString("\n"), QString("&nl;"));
    return str;
}
int Widget::on_listWidget_currentRowChanged(int currentRow)
{
    if ((currentRow == 0)||(currentRow == 7)) return 0;
    if (currentRow == 8)
    {
        ui->stackedWidget->setCurrentIndex(6);
        return 0;
    }
    if (currentRow == 9)
    {
        ui->stackedWidget->setCurrentIndex(7);
        return 0;
    }
    ui->stackedWidget->setCurrentIndex(currentRow-1);
    return 0;
}

void Widget::NextPageButton()
{
    ui->stackedWidget->setCurrentIndex(1+ ui->stackedWidget->currentIndex());
}
void Widget::on_stackedWidget_currentChanged(int arg1)
{
    rs10nComand comand;

    switch (arg1) // следить за индексами вкладок, могут убежать
    {
    case 1:
        ui->plainTextEdit->appendPlainText(" Основной режим движения ");
        comand.instruction = changeMoveMode;
        comand.parametr1 = 0;
        this->udpClient->AddComand(comand);
        break;
    case 3:
        ui->plainTextEdit->appendPlainText(" Режим по дуге езденья ");
        comand.instruction = changeMoveMode;
        comand.parametr1 = 1;
        this->udpClient->AddComand(comand);
        comand.instruction = setCircle; // сразу создаем траекторию новую
        this->udpClient->AddComand(comand);
        break;
    case 4:
        ui->plainTextEdit->appendPlainText(" Основной режим движения ");
        comand.instruction = changeMoveMode;
        comand.parametr1 = 0;
        this->udpClient->AddComand(comand);
        break;
    case 5:
        ui->plainTextEdit->appendPlainText(" Основной режим движения ");
        comand.instruction = changeMoveMode;
        comand.parametr1 = 0;
        this->udpClient->AddComand(comand);
        //Widget::setTabOrder(ui->lineEditPass,ui->pushButtonPass);// не помагает
        break;
    default:
        break;
    }
}
void Widget::CloseButtonMotion()
{
    ui->pushButtonJ1P->setEnabled(false);
    ui->pushButtonJ1M->setEnabled(false);
    ui->pushButtonJ2P->setEnabled(false);
    ui->pushButtonJ2M->setEnabled(false);
    ui->pushButtonJ3P->setEnabled(false);
    ui->pushButtonJ3M->setEnabled(false);
    ui->pushButtonJ4P->setEnabled(false);
    ui->pushButtonJ4M->setEnabled(false);
    ui->pushButtonJ5P->setEnabled(false);
    ui->pushButtonJ5M->setEnabled(false);
    ui->pushButtonJ6P->setEnabled(false);
    ui->pushButtonJ6M->setEnabled(false);
    ui->pushButtonXmMove->setEnabled(false);
    ui->pushButtonXpMove->setEnabled(false);
    ui->pushButtonXmMove_2->setEnabled(false);
    ui->pushButtonXpMove_2->setEnabled(false);
    ui->pushButtonYmMove_2->setEnabled(false);
    ui->pushButtonYpMove_2->setEnabled(false);
    ui->pushButtonYmMove->setEnabled(false);
    ui->pushButtonYpMove->setEnabled(false);
    ui->pushButtonZmMove->setEnabled(false);
    ui->pushButtonZmMove_2->setEnabled(false);
    ui->pushButtonZmMove_3->setEnabled(false);
    ui->pushButtonZmMove_4->setEnabled(false);
    ui->pushButtonZpMove->setEnabled(false);
    ui->pushButtonZpMove_2->setEnabled(false);
    ui->pushButtonZpMove_3->setEnabled(false);
    ui->pushButtonZpMove_4->setEnabled(false);
}
void Widget::OpenButtonMotion()
{
    ui->pushButtonJ1P->setEnabled(true);
    ui->pushButtonJ1M->setEnabled(true);
    ui->pushButtonJ2P->setEnabled(true);
    ui->pushButtonJ2M->setEnabled(true);
    ui->pushButtonJ3P->setEnabled(true);
    ui->pushButtonJ3M->setEnabled(true);
    ui->pushButtonJ4P->setEnabled(true);
    ui->pushButtonJ4M->setEnabled(true);
    ui->pushButtonJ5P->setEnabled(true);
    ui->pushButtonJ5M->setEnabled(true);
    ui->pushButtonJ6P->setEnabled(true);
    ui->pushButtonJ6M->setEnabled(true);
    ui->pushButtonXmMove->setEnabled(true);
    ui->pushButtonXmMove_2->setEnabled(true);
    ui->pushButtonXpMove_2->setEnabled(true);
    ui->pushButtonXpMove_2->setEnabled(true);
    ui->pushButtonYmMove_2->setEnabled(true);
    ui->pushButtonXpMove->setEnabled(true);
    ui->pushButtonYmMove->setEnabled(true);
    ui->pushButtonYpMove->setEnabled(true);
    ui->pushButtonZmMove->setEnabled(true);
    ui->pushButtonZmMove_2->setEnabled(true);
    ui->pushButtonZmMove_3->setEnabled(true);
    ui->pushButtonZmMove_4->setEnabled(true);
    ui->pushButtonZpMove->setEnabled(true);
    ui->pushButtonZpMove_2->setEnabled(true);
    ui->pushButtonZpMove_3->setEnabled(true);
    ui->pushButtonZpMove_4->setEnabled(true);
}

Widget::~Widget()
{
    //  this->processMaxim->kill();
    //this->processFazus->kill();
    this->processMaxim->close();
    this->processFazus->close();
    this->processFazus->waitForFinished(500);
    this->processMaxim->waitForFinished(500);

    this->udpClient->Stop();
    this->threadUDP->quit();
    this->threadUDP->wait(500); // ;ждем заверщение потока
    this->threadUDP->terminate();
    this->threadF->quit();
    this->threadF->wait(500);
    this->threadF->terminate();
    delete ui;
}
