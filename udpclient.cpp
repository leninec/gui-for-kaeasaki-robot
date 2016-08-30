#include "udpclient.h"
#include <Windows.h>

UdpClient::UdpClient(QObject *parent) : QObject(parent)
{
    this->qha = QHostAddress(SERVERADDR);
    this->socket = new QUdpSocket(this);

    this->socket->bind(this->qha.AnyIPv4, PORTADDR);

    //this->socket->bind(this->qha.AnyIPv4, PORTADDR);
    this->timer = new QTimer(this);
    this->timer->setInterval(8000); // таймер для проверки состояния робота во время контроля если нет пакетов 2 секунды - посмотреть не выключился ли робот

    this->vPpriemMessage.clear();
    this->vectorComand.clear();
    this->vectorC.clear();

    //this->vPriemCoordinat.clear();
    this->brun = true;
    this->bcheck = false;
    this->bSend = false;
    // this->bControl = false;
    this->bDef = false;
    this->bLevelVanna = false;
    this->bLevelBak = false;
    this->bSdvigOK = false;
    this->bPipeOpen = false;
    this->fStepAngle = 0;
    this->rHub = 0;
    this->pFazus = NULL;
   // this->fOinstrShift = -42;   // временная компенсация положения датчиков 42 на вилке которая стоит в воронеже
    this->fOinstrShift = -37;   // временная компенсация положения датчиков
    this->flagCircle = 0;
    //    this->bSdvig = false;
    connect(this->timer,SIGNAL(timeout()),this,SLOT(EventTimer()), Qt::DirectConnection );
    //QEventLoop loop;
    // loop.exec();
}

int UdpClient::Process()
{
    unsigned int last_number_def = 0;
    unsigned int number_def = 0;
    float last_z = -1;
    QByteArray buffer;
    QByteArray bPipe;  // для отправки в максиму
    QStringList coordLIst ;
    QEventLoop loop;

    // long int n;
    //  int coord;
    while(brun)
    {
        loop.processEvents();
        //  if (socket->waitForReadyRead(500))
        if ((this->socket->pendingDatagramSize())!=-1)
        {
            buffer.resize(socket->pendingDatagramSize());
            socket->readDatagram(buffer.data(), buffer.size(),
                                 &(this->qha));//, &senderPort);

            if ((buffer.indexOf("move point") != -1)&&(this->bDef)) // поидее возможна потеря данных когда не идет контроль
            {
                // контроль идет, получаем пакеты с координатами и пихаем их в вектор
                // запускаю таймер и если следующий пакет не приходит нужное время - опрашиваем статус робота
                this->timer->stop();
                this->timer->start();
                QString  str(buffer);
                coordLIst = str.split(";");
                this->stC.x=coordLIst[0].toFloat()-fXShift;
                this->stC.y=coordLIst[1].toFloat()-fYShift;
                this->stC.z=coordLIst[2].toFloat()-fZShift;

                // emit error(QString::number(this->stC.z)+" Z " + coordLIst[2]);
                // coord=coordLIst[7].toInt();

                this->stC.marker = 4;
                // this->stC.nPoint=coord;
                this->stC.nPoint = coordLIst[7].toInt();

                if (this->pFazus)
                {
                    this->stC.nDef = pFazus->get_nDef();
                    //  n = this->pFazus->get_nDef();  // обращение в обьект дефеткоскоп через указатель
                }
                else
                {
                    this->bDef = false;
                    emit error("Ошибка работы с фазусом");
                }
                // this->stC.nDef = n;
                this->vectorC.push_back(stC);

                if (this->bPipeOpen)
                {
                    /*
                    if (last_z !=-1) // можем добавлять точки только после первой
                    {
                        float z = this->stC.z - last_z;
                        int coef= 1;
                        if (z<0)
                        {
                            z=abs(z);
                            coef =-1;
                        }
                        number_def = this->pFazus->get_nDef()-1;
                        if ((number_def-last_number_def>0)&&(z>1)) // если точки расположены шире чем через 1 мм и между ними есть прозвучивания - создаем промежуточные
                        {
                            //z=1/z; // нормируем чтобы иметь шаг в 1
                            // z = z/2;
                            // z = z+last_z ; // промежуточное значение по Z
                            //    z = 0.5+last_z ; // промежуточное значение по Z
                            int d_num_def = number_def - last_number_def;
                            for (int i = 1;i<z;i++)
                            {
                                int n_def_temp;
                                n_def_temp = d_num_def/z;
                                float tempZ = last_z+i*coef;
                                bPipe.clear();
                                char float_bytes[sizeof(float)];

                                bPipe.append(4); // маркер посылки
                                memcpy(float_bytes, &(this->stC.x), sizeof(float));
                                for (int i = 0; i < sizeof(this->stC.x); ++i)
                                {
                                    bPipe.append(float_bytes[i]);
                                }
                                memcpy(float_bytes, &(this->stC.y), sizeof(float));
                                for (int i = 0; i < sizeof(float); ++i)
                                {
                                    bPipe.append(float_bytes[i]);
                                }
                                memcpy(float_bytes, &(tempZ), sizeof(float));

                                //   memcpy(float_bytes, &(this->stC.z), sizeof(float));

                                for (int i = 0; i < sizeof(float); ++i)
                                {
                                    bPipe.append(float_bytes[i]);
                                }
                                memcpy(float_bytes, &(this->stC.nPoint), sizeof(int));
                                for (int i = 0; i < sizeof(int); ++i)
                                {
                                    bPipe.append(float_bytes[i]);
                                }

                               // bPipe.append(this->pFazus->get_amp_def((number_def+last_number_def)/2));
                                   bPipe.append(this->pFazus->get_amp_def(last_number_def+n_def_temp));
                                //bPipe.append(this->pFazus->get_amp_def());
                                this->sendInPipe(bPipe);
                                SleeperThread::msleep(5);
                            }
                        }
                    }
                    */
                    /**/
                    if(this->pFazus->strob_can[0].fl_op )
                    {
                        bPipe.clear();
                        char float_bytes[sizeof(float)];

                        bPipe.append(7); // маркер посылки
                        memcpy(float_bytes, &(this->stC.x), sizeof(float));
                        for (int i = 0; i < sizeof(this->stC.x); ++i)
                        {
                            bPipe.append(float_bytes[i]);
                        }
                        memcpy(float_bytes, &(this->stC.y), sizeof(float));
                        for (int i = 0; i < sizeof(float); ++i)
                        {
                            bPipe.append(float_bytes[i]);
                        }
                        memcpy(float_bytes, &(this->stC.z), sizeof(float));
                        for (int i = 0; i < sizeof(float); ++i)
                        {
                            bPipe.append(float_bytes[i]);
                        }
                        memcpy(float_bytes, &(this->stC.nPoint), sizeof(int));
                        for (int i = 0; i < sizeof(int); ++i)
                        {
                            bPipe.append(float_bytes[i]);
                        }
                        //                        bPipe.append(this->pFazus->get_amp_def());
                        bPipe.append(this->pFazus->get_amp_def(this->pFazus->get_nDef() - 1));
                        bPipe.append(this->pFazus->get_amp_opor(this->pFazus->get_nDef() - 1));

                        this->sendInPipe(bPipe);
                    }
                    else
                    {
                        bPipe.clear();
                        char float_bytes[sizeof(float)];

                        bPipe.append(4); // маркер посылки
                        memcpy(float_bytes, &(this->stC.x), sizeof(float));
                        for (int i = 0; i < sizeof(this->stC.x); ++i)
                        {
                            bPipe.append(float_bytes[i]);
                        }
                        memcpy(float_bytes, &(this->stC.y), sizeof(float));
                        for (int i = 0; i < sizeof(float); ++i)
                        {
                            bPipe.append(float_bytes[i]);
                        }
                        memcpy(float_bytes, &(this->stC.z), sizeof(float));
                        for (int i = 0; i < sizeof(float); ++i)
                        {
                            bPipe.append(float_bytes[i]);
                        }
                        memcpy(float_bytes, &(this->stC.nPoint), sizeof(int));
                        for (int i = 0; i < sizeof(int); ++i)
                        {
                            bPipe.append(float_bytes[i]);
                        }
                        bPipe.append(this->pFazus->get_amp_def(number_def));
                        //bPipe.append(this->pFazus->get_amp_def());
                        this->sendInPipe(bPipe);
                    }
                    /* */
                }
                else
                {
                    emit error(" Нет соеденения с программой отображения");
                }
                last_number_def = number_def;//this->pFazus->get_amp_def(this->pFazus->get_nDef()-1);
                last_z= this->stC.z;
                buffer.clear();
            }
            else
            {
                if (buffer.indexOf("move point") == -1) // чтобы не читать координаты когда не пишем дефектограмму
                {
                    if (buffer.indexOf("move finish") != -1)
                    {
                        if (this->bDef)
                        {
                            this->pFazus->StopDef();
                            this->timer->stop(); // выключаем сторожевой таймер
                            QByteArray qbM;
                            bDef = false;
                            if (buffer.indexOf("hold") != -1)
                            {
                                qbM.append("контроль нарушен");
                            }
                            else
                            {
                                qbM.append("контроль окончен");
                            }
                            {
                                QMutexLocker locker(&vPpriemMessage_mutex);
                                this->vPpriemMessage.append(qbM);
                            }
                            emit answer();   // создали сообщения для индикации о завершении контроля
                        }
                        buffer.clear();
                    }
                    else
                    {
                        {
                            QMutexLocker locker(&vPpriemMessage_mutex);
                            this->vPpriemMessage.append(buffer);
                        }
                        emit answer();
                        buffer.clear();
                    }
                }
            }
        }
        if (this->bSend)
        {
            this->bSend = false;
            this->SendData(this->qbDataSend);
            this->qbDataSend.clear();
        }

        /*  if (this->bSdvig)
        {
           this->bSdvig = false;
           this->Sdvig();
        }*/
        int i;
        {
            QMutexLocker locker(&vectorComand_mutex);
            i = this->vectorComand.size();
        }
        if (i !=0)
        {
            rs10nComand comand;
            {
                QMutexLocker locker(&vectorComand_mutex);
                comand = this->vectorComand[0];
                this->vectorComand.removeFirst();
            }
            QByteArray data;
            // if (comand.instruction)
            switch (comand.instruction) {

            case stopE:
                data.clear();
                data.append("98;");  // стоп отправляем напрямки без подверждений чтобы отправка проходила как можно быстрее
                socket->writeDatagram(data,this->qha, PORTADDR);
                break;
            case stop:
                data.clear();
                data.append("99;");  // стоп отправляем напрямки без подверждений чтобы отправка проходила как можно быстрее
                socket->writeDatagram(data,this->qha, PORTADDR);
                // this->WaitingMoveFinish();
                break;
            case continueWork:
                this->ContinueWork();
                //data.clear();
                //data.append("97;");  // стоп отправляем напрямки без подверждений чтобы отправка проходила как можно быстрее
                //socket->writeDatagram(data,this->qha, PORTADDR);
                break;
            case sliv:
                this->SendCommand("34;6;0.5","signal5se"," Слив не включен",1 );
                //this->SendCommand("34;6;0.5",2000,2 );
                break;
            case pump:
                this->GetDigitalInput();
                if (this->bLevelBak )
                {
                    this->SendCommand("34;5;0.5","signal5se"," Насос не включен",1 );
                }
                else
                {
                    emit error(" Проверьте уровень воды в баке" );
                }
                break;
            case testPipe:
                this->TestPipe();
                break;
            case uploadShift:
                this->UploadShift();
                break;
            case saveC:
                // this->SaveC(comand.name7);
                this->SaveC(comand.array8);
                break;
            case setSpeed:
                this->SetSpeed(comand.parametr1);
                break;
            case startMove:
                if (this->StartControl()!=0)
                {
                    data.clear();
                    data.append(" Не удалось начать контроль");
                    {
                        QMutexLocker locker(&vPpriemMessage_mutex);
                        this->vPpriemMessage.append(data);
                    }
                    emit answer();
                }
                break;
            case goHome:
                this->GoHome();
                break;
            case setWorkSpace:
                this->SetWorkSpace(comand.parametr1,comand.parametr2,comand.parametr3,comand.parametr4,comand.parametr5,comand.parametr6);
                break;
            case getWorkSpace:
                //   this->GetWorkSpace(comand.parametr1,comand.parametr2,comand.parametr3,comand.parametr4,comand.parametr5,comand.parametr6);
                break;
            case setMechScan:
                this->SetMech(comand.parametr1);
                break;
            case openFileT:
                this->OpenFileT(comand.name7);
                break;
            case resetError:
                this->ResumeAfterError();
                break;
            case here:
                this->Here();
                break;
            case init:
                this->Init();
                break;
            case testHere:
                if (this->TestHere())
                {
                    QByteArray qbM;
                    qbM.append(" Робот не отвечает");
                    {
                        QMutexLocker locker(&vPpriemMessage_mutex);
                        this->vPpriemMessage.append(qbM);
                    }
                    emit answer();
                }
                else
                {
                    QByteArray qbM;
                    qbM.append(" Робот отвечает ");
                    {
                        QMutexLocker locker(&vPpriemMessage_mutex);
                        this->vPpriemMessage.append(qbM);
                    }
                    emit answer();
                }
                break;
            case hereShift:
                this->HereShift();
                break;
            case clearStep:
                this->ClearStep();
                break;
            case deletePoint:
                this->DeletePoint();
                break;
            case downloadPoint:
                this->DownloadPoint();
                break;
            case deleteFileDef:
                this->DeleteFileDef(comand.name7);
                break;
            case getDigitalInput:
                this->GetDigitalInput();
                break;
            case orientation:
                this->Orientation180();
                break;
            case calibration:
                if (this->Calibration(1)) // желательно передовать параметр который будет определять датчик для калибровки
                {
                    //ошибка калибровки. Ошибка будет выведена через мс бокс, нужно отправить сообщение чтобы разблокировать интерфейс
                    QByteArray qbM;
                    qbM.append("calibrovka-error");
                    {
                        QMutexLocker locker(&vPpriemMessage_mutex);
                        this->vPpriemMessage.append(qbM);
                    }
                    emit answer();
                }
                break;
            case setCircle:  // нужно создать круговую категорию на текущей высоте
                if ((this->fStepAngle)&&(this->rHub))
                {
                    this->SetCircle(this->rHub,this->fStepAngle);
                }
                else
                {
                    emit error(" Не выбран файл траектории");
                }
                break;
            case changeMoveMode:
                this->ChangeMoveMode((int)comand.parametr1);
                break;
            case move2degree:
                this->Move2degree(comand.parametr1,comand.parametr2);
                break;
            case move2point:
                this->Move2step(comand.parametr1,comand.parametr2);
                break;
            case manualMoveXYZ:
                if(comand.parametr1)
                {
                    this->MoveX(comand.parametr1);
                }
                if(comand.parametr2)
                {
                    this->MoveY(comand.parametr2);
                }
                if(comand.parametr3)
                {
                    this->MoveZ(comand.parametr3);
                }
                break;
            case manualMoveJ1J6:
                if(comand.parametr1)
                {
                    QByteArray Data;
                    QString stroka="22;1;";
                    stroka=stroka+QString::number(comand.parametr1)+";";
                    Data.clear();
                    Data.append(stroka);
                    this->SendCommand(Data,5000*abs(comand.parametr1),1); //добавим чтобы время ожидания зависело от величины на которую нужно сместиться
                }
                if(comand.parametr2)
                {
                    QByteArray Data;
                    QString stroka="22;2;";
                    stroka=stroka+QString::number(comand.parametr2)+";";
                    Data.clear();
                    Data.append(stroka);
                    this->SendCommand(Data,"movefinish","Ошибка перемещения J2",1,5000*abs(comand.parametr2),1);
                }
                if(comand.parametr3)
                {
                    QByteArray Data;
                    QString stroka="22;3;";
                    stroka=stroka+QString::number(comand.parametr3)+";";
                    Data.clear();
                    Data.append(stroka);
                    this->SendCommand(Data,"movefinish","Ошибка перемещения J3",1,5000*abs(comand.parametr3),1);
                }
                if(comand.parametr4)
                {
                    QByteArray Data;
                    QString stroka="22;4;";
                    stroka=stroka+QString::number(comand.parametr4)+";";
                    Data.clear();
                    Data.append(stroka);
                    this->SendCommand(Data,"movefinish","Ошибка перемещения J4",1,5000*abs(comand.parametr4),1);
                }
                if(comand.parametr5)
                {
                    QByteArray Data;
                    QString stroka="22;5;";
                    stroka=stroka+QString::number(comand.parametr5)+";";
                    Data.clear();
                    Data.append(stroka);
                    this->SendCommand(Data,"movefinish","Ошибка перемещения J5",1,5000*abs(comand.parametr5),1);
                }
                if(comand.parametr6)
                {
                    QByteArray Data;
                    QString stroka="22;6;";
                    stroka=stroka+QString::number(comand.parametr6)+";";
                    Data.clear();
                    Data.append(stroka);
                    this->SendCommand(Data,abs(5000*abs(comand.parametr6)),1);
                }
                break;
            case openPipe:
                this->createPipe("имя пока не забирается" ); // каждый раз создаем заново
                //if (!(this->bPipeOpen)) this->createPipe("имя пока не забирается" );
                break;
                //   break;
            default:
                break;
            }
        }

    }
    emit finished();
    return 0;
}
void UdpClient::ClearData()
{
    {
        QMutexLocker locker(&vPpriemMessage_mutex);
        this->vPpriemMessage.clear();
    }
    this->vectorC.clear();
}
int UdpClient::SendData(QByteArray data, int timeout,int nSend)
{
    // SleeperThread::msleep(3);
    int i=0;
    socket->writeDatagram(data,this->qha, PORTADDR);
    while (i<nSend)
    {
        if (!(socket->waitForReadyRead(timeout)))
        {
            socket->writeDatagram(data,this->qha, PORTADDR);
            i++;
            if (socket->waitForReadyRead(1000)) return 0;
        }
        else
        {
            return 0;
        }
    }
    emit error(" Робот не отвечает " + QString(data));
    return 1;
    /*
    //if (!(socket->waitForReadyRead(2000)))
    {
       socket->writeDatagram(data,this->qha, PORTADDR);
       if (!(socket->waitForReadyRead(2000)))
       {
          emit error("Робот не отвечает" +QString(data));
          return 1;
       }
    }
    return 0;
    */
}
QByteArray  UdpClient::ReadMessage()
{

    QByteArray mess;
    {
        QMutexLocker locker(&vPpriemMessage_mutex);
        // vPpriemMessage_mutex.lock();

        if (this->vPpriemMessage.isEmpty())
        {
            mess = 0;
            // emit error(" Буфер сообщений пуст");
            return 0; // будем вызывать это функцию пока она не вернет 0. Внимание
            // тогда важно не положить в вектор 0
        }
        mess = this->vPpriemMessage[0];
        this->vPpriemMessage.removeFirst();
    }
    return mess;

    /*  SleeperThread::msleep(100);
    QByteArray mess;
    if (this->vPpriemMessage.isEmpty())
    {
        mess = 0;
        // emit error(" Буфер сообщений пуст");
        return 0;
    }

    mess = this->vPpriemMessage[0];
    this->vPpriemMessage.removeFirst();
    SleeperThread::msleep(100);
    if (this->vPpriemMessage.size() != 0)  // если что-то забыли в буфере сообщений - прочитать
    {
        emit answer();
    }
    return mess;

    */
}
int UdpClient::ReadData()
{
    QByteArray buffer;
    // if ((this->socketrecv->pendingDatagramSize())==-1)
    if (!(this->socket->hasPendingDatagrams())) // 24.08.16 если нет данных тогда подождать еще
    {
        if (!(this->socket->waitForReadyRead(3000)))
        {
            emit error("Нет принятых данных ");
            return 1;
        }
    }
    buffer.resize(socket->pendingDatagramSize());
    socket->readDatagram(buffer.data(), buffer.size(),
                         &(this->qha));//, &senderPort);

    //qDebug("2");
    if (buffer.indexOf("move point") != -1)
    {
        //  this->vPriemCoordinat.append(buffer);
        // можно потерять данные о перемещении!!!
        //  emit error("Приняли информацию о движении к точке");
        return 2;
    }
    if (buffer.indexOf("No signal 14") != -1)
    {
        {
            QMutexLocker locker(&vPpriemMessage_mutex);
            this->vPpriemMessage.append(buffer);
        }
        buffer.clear();
        emit answer();
        return 1;
    }
    if (buffer.indexOf("move finish") != -1) // добавлена проверка так как команда stop уходит без подтверждения
    {
        if (this->bDef)
        {
            this->pFazus->StopDef();
            this->timer->stop(); // выключаем сторожевой таймер
            QByteArray qbM;
            this->bDef = false;
            if (buffer.indexOf("hold") != -1)
            {
                qbM.append("контроль нарушен");
            }
            else
            {
                qbM.append("контроль окончен");
                //  emit error("приняли move finish");
            }
            {
                QMutexLocker locker(&vPpriemMessage_mutex);
                this->vPpriemMessage.append(qbM);
            }

            emit answer();   // создали сообщения для индикации о завершении контроля
        }
        {
            QMutexLocker locker(&vPpriemMessage_mutex);
            this->vPpriemMessage.append(buffer);
        }
        buffer.clear();
        return 0;
    }
    {
        QMutexLocker locker(&vPpriemMessage_mutex);
        this->vPpriemMessage.append(buffer);
    }
    buffer.clear();
    return 0;
}
int UdpClient::SendCommand(QByteArray baCommand,
                           QString sAnswer,
                           QString sError,
                           int par,                //0 - delete message, 1 - only read message, 2 - no action
                           int timeout,
                           int nSend)
{
    if (this->SendData(baCommand,timeout,nSend)) // не было ответа в течении таймаута - прекращем функцию
    {
        // если ответа нет выскочит окошко робот не отвечает
        return 1;
    }
    if (this->ReadData())
    {
        //emit error("Ошибка чтения ответа, ждем " + sAnswer);
        return -1;  /// бляяя если все не работает смотреть тут!
    }
    {
        QString sWrongAnswer;
        QMutexLocker locker(&vPpriemMessage_mutex);
        int i = this->vPpriemMessage.size();
        while (i>0)
        {
            QString str(this->vPpriemMessage[i-1]);
            str.replace(" ", "");
            if(str.contains(sAnswer,Qt::CaseInsensitive))
            {
                switch (par)
                {
                case 0:
                    this->vPpriemMessage.remove(i-1);
                    break;
                case 1:
                    emit answer();
                    break;
                default:
                    break;
                }
                return 0; // нашли нужный нам ответ, выходим

            }
            sWrongAnswer = str;
            i--;
        }
        emit error(sError + " "+ sWrongAnswer);
        /*
        if(int i = this->vPpriemMessage.size())
        {
            QString str(this->vPpriemMessage[i-1]);
            str.replace(" ", "");
            if(str.contains(sAnswer,Qt::CaseInsensitive))
            {
                if (par)
                {
                    emit answer();
                }
                else
                {
                    this->vPpriemMessage.remove(i-1);
                }
            }
            else
            {
                emit error(sError +" "+ str);
                return 1;
            }
        }
        else
        {
            emit error(sError + "2");
        }
        */
    }
    return 1; // в случае ошибки возвразаем минус 1!!!!!!!!!!!!!!!!
}
int UdpClient::SendCommand(QByteArray baCommand, int timeout,int nSend)
{
    if (this->SendData(baCommand,timeout, nSend)) // не было ответа в течении таймаута - прекращем функцию
    {
        return 1;
    }
    return 0;
}

void UdpClient::GetPointer(fazus &pointer)
{
    this->pFazus = &pointer;
}
//void UdpClient::SaveC(QString comment)
void UdpClient::SaveC(QByteArray array)
{
    QString str;
    QString tempName = this->pFazus->tempName;
    tempName =tempName + ".c";

    /*    if (int  er = _sopen_s( &fh, tempName.toUtf8().data(), _O_BINARY | _O_RANDOM | _O_WRONLY | _O_TRUNC| _O_CREAT, _SH_DENYNO,_S_IWRITE))
        // if (int  er = _sopen_s( &fh, "F:\\prog qt\\NEWWW\\WIN\\VOR7file\\voronezh\\def.c", _O_BINARY | _O_RANDOM | _O_WRONLY | _O_TRUNC| _O_CREAT, _SH_DENYNO,_S_IWRITE))
    {
        str="не удалось открыть файл ";
        str = str+QString::number(er);
        emit error(str);
    }
    else
    {
        if (this->fStepAngle)
        {
            this->infoScan[0]=24;// версия программы
            this->infoScan[50]=(this->iSpeed&0x000000FF);  // скорость перемещения машипулятора в процентах
            this->infoScan[51]=(this->iMesh&0x000000FF);   // шаг координатной сетки робота
            char float_bytes[sizeof(float)];
            memcpy(float_bytes, &fStepAngle, sizeof(float));

            this->infoScan[60]=float_bytes[0];   // шаг по углу
            this->infoScan[61]=float_bytes[1];
            this->infoScan[62]=float_bytes[2];
            this->infoScan[63]=float_bytes[3];
        }

        // перед записью координат помещаем в файл 128 байт служебной информации
        _write(fh, &this->infoScan,sizeof(this->infoScan));



        for (int q=0;q<this->vectorC.size();q++)
        {
            _write(fh, &this->vectorC[q],sizeof(stCoord));
        }
        _close(fh);
        //   this->flagNastr=0; // для повторного контроля снова загрузить настройку
        //this->mt.stop_fazus();
    }
    */
    //
    QFile bfile(tempName);
    if (!bfile.open(QIODevice::WriteOnly))
    {
        emit error("Не удалось открыть файл координат");

    }
    if (this->fStepAngle)
    {
        this->infoScan[0] = 24;// версия программы
        this->infoScan[50] = (this->iSpeed&0x000000FF);  // скорость перемещения машипулятора в процентах
        this->infoScan[51] = (this->iMesh&0x000000FF);   // шаг координатной сетки робота
        char float_bytes[sizeof(float)];
        memcpy(float_bytes, &fStepAngle, sizeof(float));

        this->infoScan[60] = float_bytes[0];   // шаг по углу
        this->infoScan[61] = float_bytes[1];
        this->infoScan[62] = float_bytes[2];
        this->infoScan[63] = float_bytes[3];
    }
    bfile.write(this->infoScan,sizeof(this->infoScan));

    QDataStream out(&bfile);
    for (int q=0;q<this->vectorC.size();q++)
    {
        out.setByteOrder(QDataStream::LittleEndian);
        out<<this->vectorC[q].marker;
        out.setFloatingPointPrecision(QDataStream::SinglePrecision);
        out<<this->vectorC[q].x;
        out<<this->vectorC[q].y;
        out<<this->vectorC[q].z;
        out<<this->vectorC[q].nPoint;
        out<<this->vectorC[q].nDef;
    }
    bfile.close();
    //
    tempName = this->pFazus->tempName;
    tempName =tempName + ".cc";
    QFile file(tempName);
    file.open(QIODevice::WriteOnly | QIODevice::Text); // открываем только для чтения
    // QByteArray qbTemp;
    // qbTemp.append(comment);
    //  file.write(qbTemp); // записываем весь массив обратно в файл
    file.write(array);
    file.close();
    SleeperThread::msleep(500);
    QByteArray bPipe;
    bPipe.clear();
    bPipe.append(8);

    tempName = this->pFazus->tempName + ".c";
    tempName = tempName.split("\\").last();
    bPipe.append(tempName);
    this->sendInPipe(bPipe);
    QString str1(bPipe);
    // emit error(str1 + " отправил в пайп");
}
void UdpClient::Sdvig()
{
    QByteArray Data;
    Data.append("8;");
    if (this->SendCommand(Data,"shift","Ошибка получения смещения.",1) )
    {
    }
    else
    {
        // поидее правильный ответ помещен в vPpriemMessage  по факту туда могли придти еще данные!!!
        // сделать возврат нужного ответа
        QString str;

        {
            QMutexLocker locker(&vPpriemMessage_mutex);
            int i = this->vPpriemMessage.size();
            str.append(this->vPpriemMessage[i-1]);
        }
        QStringList strList = str.split(";");
        this->fXShift=strList[0].toDouble();
        this->fYShift=strList[1].toDouble();
        this->fZShift=strList[2].toDouble();
        this->fOShift=strList[3].toDouble();
        this->fAShift=strList[4].toDouble();
        this->fTShift=strList[5].toDouble();
        this->bSdvigOK= true;
    }
}
void UdpClient::AddComand(rs10nComand command )
{
    if (vectorComand.size()<2)
    {
        QMutexLocker locker(&vectorComand_mutex);
        this->vectorComand.append(command);
    }
    else
    {
        emit  error("Подождите завершения предыдущих действий.");
    }
}
void UdpClient::sendInPipe(QByteArray qb)
{
    if(!WriteFile(this->hNamedPipe, qb, qb.size(), &cbWritten, NULL))
    {
        if(ReadFile(hNamedPipe, szBuf, 512, &cbRead, NULL))
        {
            //this->readPipe = szBuf;
            this->bPipeOpen = true;
        }
        else
        {
            // раньше строчка со сбросом флага была активна
            // в режиме эмулятор попадаем сюда сразу после задания сетки
            // this->bPipeOpen = false;
            // emit error(" Не удалось отправить данные в пайп");
            // ui->filePlainTextEdit->appendPlainText("no read");
            //  int er = (int)GetLastError();
            // ui->lcdNumber->display(er);
        }
    }
}
void UdpClient::TestPipe()
{
    QByteArray qbM;
    if(!(bPipeOpen))
    {
        this->createPipe("Здесь должно быть имя");
    }
}
void UdpClient::createPipe(QString pipeName)
{
    // emit error("Создаем пайп");
    QByteArray qbM;
    lpszPipeName = L"\\\\.\\pipe\\$MyPipe$"; // Имя создаваемого канала Pipe

    //lpszPipeName = pipeName; // Имя создаваемого канала Pipe

    // Создаем канал Pipe, имеющий имя lpszPipeName
    hNamedPipe = CreateNamedPipe(
                lpszPipeName,
                PIPE_ACCESS_DUPLEX,
                PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                PIPE_UNLIMITED_INSTANCES,
                512, 512, 5000, NULL);

    // Если возникла ошибка, выводим ее код и завершаем
    // работу приложения
    if(hNamedPipe == INVALID_HANDLE_VALUE)
    {
        int er = (int)GetLastError();
        //ui->lcdNumber->display(er);
        emit error("CreateNamedPipe: Error" + QString::number(er));
    }
    else
    {
        // QByteArray qbM;
        // qbM.append("Waiting for connect");
        // this->vPpriemMessage.append(qbM);
        // emit answer();
        this->bPipeOpen = true;
        //  emit error("Создали пайп");
    }

    // При возникновении ошибки выводим ее код
    if(hNamedPipe == INVALID_HANDLE_VALUE)
    {
        qbM.append("CreateFile: Error ");
        {
            QMutexLocker locker(&vPpriemMessage_mutex);
            this->vPpriemMessage.append(qbM);
        }
        emit answer();
        // ui->filePlainTextEdit->appendPlainText("CreateFile: Error ");
    }
    /* fConnected = ConnectNamedPipe(hNamedPipe, NULL);  // бесконечно ждем подключения
    if (this->fConnected)
    {
        qbM.append("Есть соединение Pipe " );
        this->vPpriemMessage.append(qbM);
        emit answer();
    }
    else
    {
        qbM.append("Нет  соединения по PIpe ");
        this->vPpriemMessage.append(qbM);
        emit answer();
    }
*/
    //ui->filePlainTextEdit->appendPlainText(QString::number(fConnected));

    // Выводим сообщение о создании канала

    //  ui->filePlainTextEdit->appendPlainText("\nConnected. Type 'exit' to terminate\n");

    /* fConnected = ConnectNamedPipe(hNamedPipe, NULL);
  QByteArray qbM;
  if (fConnected)
  {
      qbM.append("Есть соединение Pipe ");
      this->vPpriemMessage.append(qbM);
      emit answer();
  }
  else
  {
      qbM.append("Нет  соединения по PIpe ");
      this->vPpriemMessage.append(qbM);
      emit answer();
  }
  */
}
int UdpClient::getRS10parametr()
{
    float tmp;
    float tmpO1;
    float tmpO2;

    QByteArray Data;
    Data.append("10;");
    if (SendCommand(Data,"iload,istep,imaxz,mesh_scan,ispeed","Ошибка определния параметров",1)!=0)
    {
        return 1;
    }
    {
        QMutexLocker locker(&vPpriemMessage_mutex);
        int i = this->vPpriemMessage.size();
        if (i ==0 )
        {
            emit error("Потерял данные о настройках робота в векторее сообщений ");
            return 1;
        }
        Data = vPpriemMessage[i-1];
    }
    QString str(Data);
    QStringList message = str.split(";");
    int q = message.size();
    if (q != 7)
    {
        emit error("Ошибка определения скорости.");
        return 1;
    }
    else
    {
        this->iNumPoint       = message[0].toInt();
        this->iCurrentPoint   = message[1].toInt();
        this->iMaxZ           = message[2].toInt();
        this->iMesh           = message[3].toInt();
        this->iSpeed          = message[4].toInt();
    }
    if ((this->iMaxZ<10)||(this->iMaxZ>100))
    {
        QString str;
        str="Высота изделия определна как ";
        str=str+QString::number(this->iMaxZ);
        emit error(str);

        if (this->iMaxZ == 0) return 3;
    }
    if (this->iNumPoint<4)
    {
        emit error(" Траектория не загружена");
        return 2;
    }
    int er;
    er = this->getPointCoordint(1,"определение шага",&tmp,&tmp,&tmp,&tmpO1,&tmp,&tmp);
    er = er + this->getPointCoordint(3,"определение шага2",&tmp,&tmp,&tmp,&tmpO2,&tmp,&tmp);
    if (er != 0)
    {
        return 1;
    }
    this->fStepAngle = tmpO2-tmpO1;
    return 0;
}
int UdpClient::getPointCoordint(int nP, QString timeoutText, float *x,float *y,float *z,float *o,float *a,float *t)
{
    QByteArray Data;
    QString stroka;
    Data.append("45;"+QString::number(nP)+";");
    if (this->SendCommand(Data,"coordpoint","Ошибка получения координаты точки",1)  )
    {}
    else
    {
        {
            QMutexLocker locker(&vPpriemMessage_mutex);


            int i = this->vPpriemMessage.size();
            if (i ==0 )
            {
                emit error("Потерял данные в векторее сообщений получение точек");
                return 1;
            }
            Data = this->vPpriemMessage[i-1];
        }
        QString str(Data);
        QStringList message = str.split(";");
        *x = message[0].toFloat();
        *y = message[1].toFloat();
        *z = message[2].toFloat();
        *o = message[3].toFloat();
        *a = message[4].toFloat();
        *t = message[5].toFloat();
    }
    return 0;
}
int UdpClient::UploadShift()
{
    QByteArray Data;
    Data.append("8;");
    if (this->SendCommand(Data,"shift","Ошибка получения смещения",1)  )
    {
        return 1;
    }
    {
        QMutexLocker locker(&vPpriemMessage_mutex);

        int i = this->vPpriemMessage.size();
        if (i ==0 )
        {
            emit error("Потерял данные в векторее сообщений");
            return 1;
        }
        Data = this->vPpriemMessage[i-1];
        // размер 1 а индекс первого ноль
    }
    QString str(Data);
    if (!(str.contains("shift",Qt::CaseInsensitive)))
    {
        emit error("Не то прочитал из вектора сообщений");
        return 1; //не то
    }
    QStringList strList=str.split(';');
    this->fXShift=strList[0].toFloat();
    this->fYShift=strList[1].toFloat();
    this->fZShift=strList[2].toFloat();
    this->fOShift=strList[3].toFloat();
    this->fAShift=strList[4].toFloat();
    this->fTShift=strList[5].toFloat();
    this->bSdvigOK = true;

    return 0;
}
int UdpClient::OpenFileT(QString nameFile)
{
    QFile bfile(nameFile);
    if (!bfile.open(QIODevice::ReadOnly))
    {
        emit error("Ошибка открытия файла");
        return 1;
    }
    QDataStream in(&bfile);
    //QByteArray ba = file.readLine(); // читаем первую строку
    this->qbaTrajectory = bfile.readAll();
    bfile.close();
    // вынем из файла радиус и шаг по углу

    QString tmp(this->qbaTrajectory);
    QStringList point = tmp.split("\n");
    QStringList coord = point[0].split(";");
    QString tmp2 = coord[1];
    this->rHub = tmp2.toFloat();

    float x1tmp;
    float x2tmp;
    float y1tmp;
    float y2tmp;
    coord = point[1].split(";"); //берем вторую и третью строчку для рассычета шага
    tmp2 = coord[1];
    x1tmp = tmp2.toFloat();
    tmp2 = coord[2];
    y1tmp = tmp2.toFloat();
    coord = point[2].split(";");
    tmp2 = coord[1];
    x2tmp = tmp2.toFloat();
    tmp2 = coord[2];
    y2tmp = tmp2.toFloat();
    this->fStepAngle = sqrt((x2tmp-x1tmp)*(x2tmp-x1tmp)+((y2tmp-y1tmp)*(y2tmp-y1tmp)));
    // emit error(" радиус " + QString::number(this->rHub) +" шаг = " + QString::number(this->fStepAngle));
    QByteArray mes;
    mes.append("trajectory");
    {
        QMutexLocker locker(&vPpriemMessage_mutex);
        this->vPpriemMessage.append(mes);
    }

    emit answer();
    return 0;
}
int UdpClient::HereShift() // функция, берущая текущую координату и сдвигающая на нее массив точек
{
    QByteArray Data;
    float x,y,z;

    this->DeletePoint();
    if (this->qbaTrajectory.count() == 0)
    {                      // если траектория не загружена сдвигать нечего
        emit error("Не открыт файл с траекторий.");
        return 1;
    }
    // предпологается что это функция вызывается только после калибровки
    // а во время калибровки мы сохранили координаты начальной точки
    // нам нужно только дописать туда новый z

    if (this->Here(&x,&y,&z) ) return 1;
    this->fZShift = z;


    /*
    this->Here();
    int i = this->vPpriemMessage.size();
    if (i ==0 )
    {
        emit error("Потерял данные в векторее сообщений");
        return 1;
    }

    Data = this->vPpriemMessage[i-1];
    // размер 1 а индекс первого ноль

    QString str(Data);

    QStringList strList=str.split(';');
    if (!(str.contains("here",Qt::CaseInsensitive)))
    {
        emit error("Не то прочитал из вектора сообщений сдвиг координат");
        return 1; //не то
    }
    this->fXShift = strList[0].toDouble();
    this->fYShift = strList[1].toDouble();
    this->fZShift = strList[2].toDouble();
    this->fOShift = strList[3].toDouble();
    this->fAShift = strList[4].toDouble();
    this->fTShift = strList[5].toDouble();
*/
    QString st(this->qbaTrajectory);
    QStringList coordList = st.split('\n').first().split(";");

    this->fXShift = fXShift - coordList[1].toDouble();
    this->fYShift = fYShift - coordList[2].toDouble();
    this->fZShift = fZShift - coordList[3].toDouble();
    this->fOShift = fOShift - coordList[4].toDouble();
    this->fAShift = fAShift - coordList[5].toDouble();
    this->fTShift = fTShift - coordList[6].toDouble();

    this->fXShift = this->Round(fXShift);
    this->fYShift = this->Round(fYShift);
    this->fZShift = this->Round(fZShift);
    this->fOShift = this->Round(fOShift);
    this->fAShift = this->Round(fAShift);
    this->fTShift = this->Round(fTShift);

    QString stroka;
    stroka = "4;";
    stroka = stroka+QString::number(fXShift)+";"+QString::number(fYShift)+";"+QString::number(fZShift)+";";
    stroka = stroka+QString::number(fOShift)+";"+QString::number(fAShift)+";"+QString::number(fTShift)+";";

    Data.clear();
    Data.append(stroka);// запихиваем в пакет матрицу смещения

    if( this->SendCommand(Data,"shift","Ошибка задания смещения",0)) return 1;

    QByteArray qbM;
    qbM.append("start posithion");
    {
        QMutexLocker locker(&vPpriemMessage_mutex);
        this->vPpriemMessage.append(qbM);
    }

    emit answer();

    qbM.clear();
    qbM.append(stroka + "задаваемое смещение для начальной точки через сохраненное после калибровки смещение");
    {
        QMutexLocker locker(&vPpriemMessage_mutex);
        this->vPpriemMessage.append(qbM);
    }

    emit answer();

    return 0;
}
float UdpClient::Round(float x)
{
    x = floor(x*100)/100;
    return x;
}
int UdpClient::Here()
{
    QByteArray Data;
    Data.append("2;");// узнать текущую координату
    int er;
    er = this->SendCommand(Data,"here","Ошибка определения местоположения",1);
    return er;
}
int UdpClient::Here(float *x,float *y,float *z)
{
    QByteArray Data;
    Data.append("2;");// узнать текущую координату
   // int er = this->SendCommand(Data,"here","Ошибка определения местоположения",1);
    // функция SendComand в случае успеха выхвала фукцию ReadMessge которая забирала сообшение в интерфейс и стирало его
    int er = this->SendCommand(Data,"here","Ошибка определения местоположения",2);
    if ( er == 0 )
    {
        QMutexLocker locker(&vPpriemMessage_mutex);
        {
            int i = this->vPpriemMessage.size();
            if (i == 0 )
            {
                emit error("Потерял данные в векторе сообщений определение координаты");
                return 1;
            }
            while (i>0)   //24.08/16 в SendComand теперь перебираем весь вектор принятых сообщений поэтому првоеряем и здесь где именно лежит ответ
            {
              Data = this->vPpriemMessage[i-1];
              QString str(Data);
              if (str.contains("here",Qt::CaseInsensitive)) break;
              i--;
            }

        }
        QString str(Data);
        QStringList message = str.split(";");
        *x = message[0].toFloat();
        *y = message[1].toFloat();
        *z = message[2].toFloat();
    }
    return er;
}
int UdpClient::Here(float *x,float *y,float *z,float *o,float *a,float *t)
{
    QByteArray Data;
    Data.append("2;");// узнать текущую координату
    int er = this->SendCommand(Data,"here","Ошибка определения местоположения",2);
    if ( er ==0 )
    {
        {
            QMutexLocker locker(&vPpriemMessage_mutex);

            int i = this->vPpriemMessage.size();
            if (i == 0 )
            {
                emit error("Потерял данные в векторее сообщений определение координаты");
                return 1;
            }
            while (i > 0)   //24.08/16 в SendComand теперь перебираем весь вектор принятых сообщений поэтому првоеряем и здесь где именно лежит ответ
            {
              Data = this->vPpriemMessage[i-1];
              QString str(Data);
              if (str.contains("here",Qt::CaseInsensitive)) break;
              i--;
            }

        }
        QString str(Data);
        QStringList message = str.split(";");
        *x = message[0].toFloat();
        *y = message[1].toFloat();
        *z = message[2].toFloat();
        *o = message[3].toFloat();
        *a = message[3].toFloat();
        *t = message[3].toFloat();
    }
    return er;
}
int UdpClient::TestHere()
{ // смысл функции - в случае ошибки не выводить всплывающих окон

    //   QByteArray qbM;
    //   qbM.append("Робота пока нет, пропускаем начальную проверку");
    //   this->vPpriemMessage.append(qbM);
    //   emit answer();
    /* */   QByteArray Data;
    Data.append("2;");// узнать текущую координату
    this->socket->writeDatagram(Data,this->qha, PORTADDR);
    if (this->socket->waitForReadyRead(3000))
    {
        this->ReadData();
        {
            QMutexLocker locker(&vPpriemMessage_mutex);

            int i = this->vPpriemMessage.size();
            if (i ==0 )
            {
                return 1;
            }
            Data = this->vPpriemMessage[i-1];
            QString str(Data);
            if(str.contains("here",Qt::CaseInsensitive))
            {
                this->vPpriemMessage.removeFirst();
                //  QStringList message = str.split(";");
                // *x = message[0].toFloat();
                // *y = message[1].toFloat();
                // *z = message[2].toFloat();
            }
            else
            {
                return 1;
            }
        }
        this->TestMotor(); // Заодно проверяем включен ли двигатель
        this->GetErrorRobot() ; // и ошибки
        this->GetDigitalInput();
    }
    else
    {
        return 1;
    }
    /**/
    return 0;
}
int UdpClient::ParametrsRs10N(float *angle, int *speed, int *mesh)
{
    *angle = this->fStepAngle;
    *speed = this->iSpeed;
    *mesh = this->iSpeed;
    return 0;
}
int UdpClient::ClearStep()
{
    QByteArray Data;
    Data.append("18;");
    int er;
    er = this->SendCommand(Data,"iload,istep","Ошибка очистки счетчика шагов ",1);
    return er;
}
int UdpClient::DeletePoint()
{
    QByteArray Data;
    Data.append("17;");
    int er;
    er = this->SendCommand(Data,"iload,istep","Ошибка удаления траектории ",1);
    return er;
}
int UdpClient::DownloadPoint()
{
    this->ChangeMoveMode(0);  // принудительно переходим в основной режим движения перед загрузкой тчоек
    SleeperThread::msleep(5);   // чтобы не загрузить точки для контроля в режим по дугу
    this->DeletePoint(); //при стирании точек  теперь не удаляется матрица смещения
    int j = 0;
    int count = 0;            // Количество строчек в файле
    int count2 = 0;           // Индекс символа начала строки
    QByteArray Data;

    float size = this->qbaTrajectory.count("\n");
    int stepPb = 1;
    if (size<100)
    {
        stepPb = 100/size;
    }
    size = size/100;
    float step = size;

    while ((j = this->qbaTrajectory.indexOf("\n", j)) != -1)  //посчитать кол-во строк
    {
        if (count > size)
        {
            size = size + step;
            emit progres(stepPb);
        }
        for (int i=0; i<=(j-count2-2); i++)
        {
            Data[i] = this->qbaTrajectory[i+count2];   //каждую строку положить в отдельный массив
        }
        //socket->writeDatagram(Data, this->qha, PORTADDR);  // и выслать
        if( this->SendCommand(Data,"point","Ошибка загрузки точки",0))
        {
            this->DeletePoint();
            return 1;
        }
        ++j;
        count++;
        count2=j;
    }

    QByteArray qbM;
    qbM.append("finisheddownload");
    {
        QMutexLocker locker(&vPpriemMessage_mutex);
        this->vPpriemMessage.append(qbM);
    }
    emit answer();
    return 0;
}
int UdpClient::Calibration(int napr)
{
    // по напру калибруемся или изнутри втулки или снаружи - пока не работает
    float xTmp = 0;
    float yTmp = 0;
    float zTmp = 0;
    float oTmp = 0;
    float aTmp = 0;
    float tTmp = 0;
    float fycoord1 = 0;
    float fycoord2 = 0;
    float fxcoord1 = 0;
    float fxcoord2 = 0;
    int   increase = 0;
    float stepNastr = (float)0.7;  // величина шага для поиска максимума.
    int   ampLow = 55;  // с ацп получаем амплитуда в 255 отчетов. для поиска максимума стараемся держать сигнал где то посередине
    int   ampMax = 207;
    int   znak = 1;
    int   itimeX1 = 0;
    int   itimeX2 = 0;
    int   itimeY1 = 0;
    int   itimeY2 = 0;
    int   countZnak = 0; // накопитель для определения направления движения
    float fZmax = 100;
    float sdvigX = 0;
    float sdvigY = 0;

    QString nameFolder =  ".\\system-nastr\\exo1-1-";
    QString nameNastr;

    QVector <int> viAmpMax;
    QByteArray Data;
    if(!(this->pFazus))
    {
        emit error("Ошибка работы с фазусом");
        return 1;
    }
    if (this->GetDigitalInput()) return 0;
    if (!(this->bLevelVanna))
    {
        emit error("Проверьте уровень воды в ванне");
        return 0;
    }

    int er = this->pFazus->one_shot(".\\system-nastr\\exo1-1-5db.nst");
    //  emit error(QString::number(er)+ " Ампл на 5 Дб");
    if ( er > 256)
    {
        emit error("Нет связи с фазусом");
        return 1;
    }

    if (this->qbaTrajectory.size()==0)
    {
        emit error(" Не выбран файл траектори");
        return 1;
    }
    this->ChangeMoveMode(0);

    //if (this->DeletePoint()) return 1; // в ориентации фланца и так стираем точки
    int begin = 0;
    int end   = 0;
    begin = this->qbaTrajectory.indexOf(";");
    end = this->qbaTrajectory.indexOf(";",2);
    QByteArray qbtemp;
    QString qstmp;
    for (int i=0;i<(end-2);i++)
    {
        qbtemp[i]=this->qbaTrajectory[2+i];
    }
    qstmp = QString::QString(qbtemp);
    float radius_hub = qstmp.toFloat();

    Data.clear();
    int nStr = this->qbaTrajectory.indexOf("\n");
    for (int i = 0; i<=(nStr); i++)
    {
        Data.append(this->qbaTrajectory[i]);   //каждую строку положить в отдельный массив
    }
    QString str(Data);
    QStringList list = str.split(";");

    float ftemp = list[3].toFloat();
    Data.clear();
    int nStr1 = this->qbaTrajectory.indexOf("\n",nStr+1);
    for (int i = nStr+3; i<=(nStr1); i++)
    {
        Data.append(this->qbaTrajectory[i+nStr]);   //каждую строку положить в отдельный массив
    }
    qstmp.clear();
    QString str1(Data);

    list = str1.split(";");
    float ftemp2 = list[3].toFloat();

    if (ftemp>ftemp2)
    {
        if (ftemp >fZmax)fZmax = ftemp;
    }
    else
    {
        if (ftemp2 >fZmax)fZmax = ftemp2;
    }
    //  emit error("максимальная высота = " + QString::number(fZmax));
    // необоходимо развернуть фланец перепендикулярно ванне

    if( this->Orientation180(0)) return 1;  // параметр 0 чтобы сохранить первую точку
    // парметр пока не учитывается!!

    if (this->DeleteShift()) return 1;   // в ориентации теперь не удаляем смещение, а все точки дял калибровки создаються без смещения
    SleeperThread::msleep(50);
    // фланец установлен начинаем искать сигнал
    nameNastr = this->SearhIncreace(nameFolder,ampLow,ampMax);
    // emit error("настройка" + nameNastr);
    if (nameNastr == "NULL")
        // if ((nameNastr.contains("Не смогли найти сигнал"))&&((nameNastr.contains("Нет связи с фазусом"))))
    {
        emit error(" Неверное имя настройки");
        // emit error(nameNastr);
        this->pFazus->Stop_fazus();
        return 1;
    }
    for (int i=0;i<5;i++) // проходим 5 точек а одном направлении и записываем величину сигнала
    {
        this->MoveY(1*znak);
        viAmpMax.append(this->pFazus->one_shot(nameNastr.toUtf8().data()));
    }
    // определяем в каком направлении необходимо двигаться
    for (int i=1;i<viAmpMax.size();i++)
    {
        if (viAmpMax[i]-viAmpMax[i-1]>0)
        {
            countZnak++;
        }
        else
        {
            if (countZnak>0)
            {
                if(i==(viAmpMax.size()-1))
                {
                    // последняя точка сменила знак снять еще
                    //        emit error("последняя точка сменила знак");
                    this->MoveY(1*znak);
                    viAmpMax.append(this->pFazus->one_shot(nameNastr.toUtf8().data()));
                    if ((viAmpMax[viAmpMax.size()-1]-viAmpMax[i])<=0)
                    {
                        countZnak-=2;
                        //подтверждена смена знака");
                    }
                    else
                    {
                        // " не подтверждена смена знака");
                    }
                    i++;
                }
                else
                {
                    if(viAmpMax[i+1]-viAmpMax[i]<0)
                    {
                        countZnak-=2;
                        countZnak-=viAmpMax.size(); // прошли максимум нужно двигаться в обратную сторону
                        i++;
                        // ui->filePlainTextEdit->appendPlainText(" смена знака");
                    }
                    else
                    {
                        //выброс, не учитываем значение
                    }
                }
            }
            else
            {
                countZnak--;
            }
        }
    }
    //number(countZnak) + " приращение");
    if (countZnak>0)
    {
        znak=znak; //продолжаем движение");
    }
    else
    {
        znak=-znak;//" движемся обратно");
    }
    // движение маленькими шажками в выбранную сторону для выхода в максимум
    nameNastr = ".\\system-nastr\\exo1-1-5db.nst" ;
    viAmpMax.clear();
    viAmpMax.append(this->pFazus->one_shot(nameNastr.toUtf8().data()));
    while(1)
    {
        if (this->MoveY(stepNastr*znak)) return 1;
        viAmpMax.append(this->pFazus->one_shot(nameNastr.toUtf8().data()));
        if ((viAmpMax[viAmpMax.size()-1]-viAmpMax[viAmpMax.size()-2])<0)
        {   // текущее минус предыдущее меньше - проехали максимум
            // на всякий случай делаем еще один шаг
            if (this->MoveY(stepNastr*znak)) return 1;
            SleeperThread::msleep(200);
            viAmpMax.append(this->pFazus->one_shot(nameNastr.toUtf8().data()));
            if ((viAmpMax[viAmpMax.size()-1]-viAmpMax[viAmpMax.size()-2])<0)
            {   // следующая точка тоже меньше - нашли максимум возвращаемся назад на две точки
                if (this->MoveY(2*stepNastr*(-znak))) return 1;
                break;
            }
        }
        if (!(viAmpMax.size()%((int)(35/stepNastr))))
        {
            emit error("Проверьте расположение втулки");
            this->pFazus->Stop_fazus();
            return 1;
        }
    }
    //  emit error("Вышли в максимум");
    //  Находимся в максимуме, мерим время
    this->pFazus->one_shot_pin(nameNastr.toUtf8().data(),&itimeY1); // померили время прихода эхо сигнала в 1 точке
    if (this->Here(&xTmp,&yTmp,&zTmp)) return 1; // точка максимума по Y найдена сохраняем координату 1

    fycoord1 = yTmp; // запомнили координату  1 точки
    fxcoord1 = xTmp;

    // Поиск максимума  по оси X 1

    // emit error(QString::number(radius_hub)+ " радиус втулки");
    // нашли Х координату относительно 0
    // рассчиттываем координаты точки максимально удаленной от робта плюс подворот фланца
    // из текущего Х необходимо вычесть coordinatX а к Y прибавить
    xTmp = fxcoord1 - radius_hub;       // Считаем вторую точку, относительно первой
    yTmp = fycoord1 + radius_hub;
    zTmp = zTmp + fZmax;
    oTmp = 90 + this->fOinstrShift;  // Изменение от 06.06.16 так как инструмент стоит криво(((
    aTmp = 180;
    tTmp = 0;

    qstmp = "1;" + QString::number(xTmp) + ";" + QString::number(yTmp)+";" + QString::number(zTmp) + ";"+QString::number(oTmp)+";" + QString::number(aTmp)+";" + QString::number(tTmp)+";";

    //emit error("вторая точка " + qstmp);
    Data.clear();
    Data.append(qstmp);
    if (this->SendCommand(Data,"point"," Ошибка создания точки  ",0 ))
    {
        this->pFazus->Stop_fazus();
        return 1; // не создали
    }
    if (this->MoveZ(fZmax)) return 1;      // подняться перед началом движения

    //  SleeperThread::sleep(1); //погодить пока доедет.

    Data.clear();
    Data.append("16;");
    if (this->SendCommand(Data, "movestart"," Ошибка начала перемещения во вторую точку",1)) return 1;
    if (this->WaitingMoveFinish()) return 1;
    SleeperThread::sleep(1); //погодить пока доедет.
    if (this->MoveZ(-fZmax)) return 1; // опускаемся обратно на деталь
    viAmpMax.clear();
    znak = 1;
    countZnak = 0;
    increase = 0;

    nameNastr = this->SearhIncreace(nameFolder,ampLow,ampMax); // ищем сигнал во второй точке
    if (nameNastr == "NULL")
    {
        this->pFazus->Stop_fazus();
        emit error(" Неверное имя натсройки");
        return 1;
    }
    for (int i = 0;i<5;i++) // проходим 5 точек а одном направлении и записываем величину сигнала
    {
        this->MoveX(1*znak);
        SleeperThread::msleep(5);
        viAmpMax.append(this->pFazus->one_shot(nameNastr.toUtf8().data()));
    }
    // определяем в каком направлении необходимо двигаться
    for (int i = 1;i<viAmpMax.size();i++)
    {
        if (viAmpMax[i]-viAmpMax[i-1]>0)
        {
            countZnak++;
        }
        else
        {
            if (countZnak>0)
            {
                if(i==(viAmpMax.size()-1))
                {
                    // последняя точка сменила знак снять еще
                    this->MoveX(1*znak);
                    viAmpMax.append(this->pFazus->one_shot(nameNastr.toUtf8().data()));
                    if ((viAmpMax[viAmpMax.size()-1]-viAmpMax[i])<=0)
                    {
                        countZnak-=2;
                        //подтверждена смена знака");
                    }
                    else
                    {
                        // " не подтверждена смена знака");
                    }
                    i++;
                }
                else
                {
                    if(viAmpMax[i+1]-viAmpMax[i]<0)
                    {
                        countZnak-= 2;
                        countZnak-= viAmpMax.size(); // прошли максимум нужно двигаться в обратную сторону
                        i++;
                        // ui->filePlainTextEdit->appendPlainText(" смена знака");
                    }
                    else
                    {
                        //выброс, не учитываем значение
                    }
                }
            }
            else
            {
                countZnak--;
            }
        }
    }
    //number(countZnak) + " приращение");
    if (countZnak > 0)
    {
        znak = znak; //продолжаем движение");
    }
    else
    {
        znak = -znak;//" движемся обратно");
    }
    // движение маленькими шажками в выбранную сторону для выхода в максимум
    nameNastr = ".\\system-nastr\\exo1-1-5db.nst" ;
    viAmpMax.clear();
    viAmpMax.append(this->pFazus->one_shot(nameNastr.toUtf8().data()));
    while(1)
    {
        if (this->MoveX(stepNastr*znak)) return 1;
        viAmpMax.append(this->pFazus->one_shot(nameNastr.toUtf8().data()));
        if ((viAmpMax[viAmpMax.size()-1]-viAmpMax[viAmpMax.size()-2])<0)
        {   // текущее минус предыдущее меньше - проехали максимум
            // на всякий случай делаем еще один шаг
            if (this->MoveX(stepNastr*znak)) return 1;
            viAmpMax.append(this->pFazus->one_shot(nameNastr.toUtf8().data()));
            if ((viAmpMax[viAmpMax.size()-1]-viAmpMax[viAmpMax.size()-2])<0)
            {   // следующая точка тоже меньше - нашли максимум возвращаемся назад на две точки
                this->MoveX(2*stepNastr*(-znak));
                break;
            }
        }
        if (!(viAmpMax.size()%((int)(35/stepNastr))))
        {
            this->pFazus->Stop_fazus();
            emit error("Проверьте расположение втулки");
            return 1;
        }
    }
    //  Находимся в максимуме, мерим время
    this->pFazus->one_shot_pin(nameNastr.toUtf8().data(),&itimeX1); // померили время прихода эхо сигнала в 1 точке
    if (this->Here(&xTmp,&yTmp,&zTmp)) return 1; // точка максимума по Y найдена сохраняем координату 1

    fycoord2 = yTmp; // запомнили координату  2 точки
    fxcoord2 = xTmp;
    //едем в третью точку
    oTmp = 180 + this->fOinstrShift;
    QString threePoint = "1;" + QString::number(fxcoord1 - 2*radius_hub) + ";" + QString::number(fycoord1) + ";" + QString::number(zTmp+fZmax)+";" + QString::number(oTmp)+";180;0;";// 06.06.16
    Data.clear();
    Data.append(threePoint);
    //emit error("точка 3 " + threePoint);
    if(this->SendCommand(Data,"point"," Ошибка создания третьей точки",0))
    {
        this->pFazus->Stop_fazus();
        return 1;
    }
    if (this->MoveZ(fZmax)) return 1;
    Data.clear();
    Data.append("16;");
    if (this->SendCommand(Data,"movestart", "Ошибка начала движения к третьей точке",1)) return 1;
    if (this->WaitingMoveFinish()) return 1;
    SleeperThread::sleep(1);
    if (this->MoveZ(-fZmax))return 1; // опускаемся вниз на деталь
    SleeperThread::sleep(1);

    viAmpMax.clear();
    znak = 1;
    countZnak = 0;
    increase = 0; // начинаем искать сигнал в 3 точке

    nameNastr = this->SearhIncreace(nameFolder,ampLow,ampMax);
    if (nameNastr == "NULL")
    {
        this->pFazus->Stop_fazus();
        emit error(" Неверное имя настройки");
        return 1;
    }
    for (int i=0;i<5;i++) // проходим 5 точек а одном направлении и записываем величину сигнала
    {
        this->MoveY(1*znak);
        viAmpMax.append(this->pFazus->one_shot(nameNastr.toUtf8().data()));
    }
    // определяем в каком направлении необходимо двигаться
    for (int i=1;i<viAmpMax.size();i++)
    {
        if (viAmpMax[i]-viAmpMax[i-1]>0)
        {
            countZnak++;
        }
        else
        {
            if (countZnak>0)
            {
                if(i==(viAmpMax.size()-1))
                {
                    // последняя точка сменила знак снять еще
                    this->MoveY(1*znak);
                    viAmpMax.append(this->pFazus->one_shot(nameNastr.toUtf8().data()));
                    if ((viAmpMax[viAmpMax.size()-1]-viAmpMax[i])<=0)
                    {
                        countZnak-=2;
                        //подтверждена смена знака");
                    }
                    else
                    {
                        // " не подтверждена смена знака");
                    }
                    i++;
                }
                else
                {
                    if(viAmpMax[i+1]-viAmpMax[i]<0)
                    {
                        countZnak-=2;
                        countZnak-=viAmpMax.size(); // прошли максимум нужно двигаться в обратную сторону
                        i++;
                        // ui->filePlainTextEdit->appendPlainText(" смена знака");
                    }
                    else
                    {
                        //выброс, не учитываем значение
                    }
                }
            }
            else
            {
                countZnak--;
            }
        }
    }
    //number(countZnak) + " приращение");
    if (countZnak>0)
    {
        znak=znak; //продолжаем движение");
    }
    else
    {
        znak=-znak;//" движемся обратно");
    }
    // движение маленькими шажками в выбранную сторону для выхода в максимум
    nameNastr = ".\\system-nastr\\exo1-1-5db.nst" ;
    viAmpMax.clear();
    viAmpMax.append(this->pFazus->one_shot(nameNastr.toUtf8().data()));
    while(1)
    {
        if (this->MoveY(stepNastr*znak)) return 1;
        viAmpMax.append(this->pFazus->one_shot(nameNastr.toUtf8().data()));
        if ((viAmpMax[viAmpMax.size()-1]-viAmpMax[viAmpMax.size()-2])<0)
        {   // текущее минус предыдущее меньше - проехали максимум
            // на всякий случай делаем еще один шаг
            if (this->MoveY(stepNastr*znak)) return 1;
            viAmpMax.append(this->pFazus->one_shot(nameNastr.toUtf8().data()));
            if ((viAmpMax[viAmpMax.size()-1]-viAmpMax[viAmpMax.size()-2])<0)
            {   // следующая точка тоже меньше - нашли максимум возвращаемся назад на две точки
                this->MoveY(2*stepNastr*(-znak));
                break;
            }
        }
        if (!(viAmpMax.size()%((int)(35/stepNastr))))
        {
            this->pFazus->Stop_fazus();
            emit error("Проверьте расположение втулки");
            return 1;
        }
    }
    //  Находимся в максимуме, мерим время
    this->pFazus->one_shot_pin(nameNastr.toUtf8().data(),&itimeY2); // померили время прихода эхо сигнала в 3 точке

    if(this->MoveZ(fZmax)) return 1;

    QString point4 = "1;"+QString::number(fxcoord1-radius_hub) + ";" + QString::number(fycoord2-2*radius_hub) + ";" + QString::number(zTmp+fZmax)+";" + QString::number(-90 + this->fOinstrShift)+ ";180;0;";// 06.06.16
    Data.clear();
    Data.append(point4);
    //emit error("точка 4 " + point4 );
    if(this->SendCommand(Data,"point"," Ошибка создания точки",0)) return 1;
    SleeperThread::sleep(2);

    Data.clear();
    Data.append("16;");
    if(this->SendCommand(Data,"movestart"," Ошибка начала движения",1)) return 1;
    if (this->WaitingMoveFinish()) return 1;
    SleeperThread::sleep(1);
    if(this->MoveZ(-fZmax)) return 1;
    viAmpMax.clear();
    znak = 1;
    countZnak = 0;
    increase = 0; //поиск максимума 4 точка

    nameNastr = this->SearhIncreace(nameFolder,ampLow,ampMax);
    if (nameNastr == "NULL")
    {
        this->pFazus->Stop_fazus();
        emit error(" Неверное имя натсройки");
        return 1;
    }
    for (int i = 0;i < 5;i++) // проходим 5 точек а одном направлении и записываем величину сигнала
    {
        this->MoveX(1*znak);
        viAmpMax.append(this->pFazus->one_shot(nameNastr.toUtf8().data()));
    }
    // определяем в каком направлении необходимо двигаться
    for (int i = 1;i < viAmpMax.size();i++)
    {
        if (viAmpMax[i]-viAmpMax[i-1]>0)
        {
            countZnak++;
        }
        else
        {
            if (countZnak>0)
            {
                if(i==(viAmpMax.size()-1))
                {
                    // последняя точка сменила знак снять еще
                    this->MoveX(1*znak);
                    viAmpMax.append(this->pFazus->one_shot(nameNastr.toUtf8().data()));
                    if ((viAmpMax[viAmpMax.size()-1]-viAmpMax[i])<=0)
                    {
                        countZnak-=2;
                        //подтверждена смена знака");
                    }
                    else
                    {
                        // " не подтверждена смена знака");
                    }
                    i++;
                }
                else
                {
                    if(viAmpMax[i+1]-viAmpMax[i]<0)
                    {
                        countZnak-=2;
                        countZnak-=viAmpMax.size(); // прошли максимум нужно двигаться в обратную сторону
                        i++;
                        // ui->filePlainTextEdit->appendPlainText(" смена знака");
                    }
                    else
                    {
                        //выброс, не учитываем значение
                    }
                }
            }
            else
            {
                countZnak--;
            }
        }
    }
    //number(countZnak) + " приращение");
    if (countZnak>0)
    {
        znak=znak; //продолжаем движение");
    }
    else
    {
        znak=-znak;//" движемся обратно");
    }
    // движение маленькими шажками в выбранную сторону для выхода в максимум
    nameNastr = ".\\system-nastr\\exo1-1-5db.nst" ;
    viAmpMax.clear();
    viAmpMax.append(this->pFazus->one_shot(nameNastr.toUtf8().data()));
    while(1)
    {
        if (this->MoveX(stepNastr*znak)) return 1;
        viAmpMax.append(this->pFazus->one_shot(nameNastr.toUtf8().data()));

        if ((viAmpMax[viAmpMax.size()-1]-viAmpMax[viAmpMax.size()-2])<0)
        {   // текущее минус предыдущее меньше - проехали максимум
            // на всякий случай делаем еще один шаг
            if (this->MoveX(stepNastr*znak)) return 1;
            viAmpMax.append(this->pFazus->one_shot(nameNastr.toUtf8().data()));
            if ((viAmpMax[viAmpMax.size()-1]-viAmpMax[viAmpMax.size()-2])<0)
            {   // следующая точка тоже меньше - нашли максимум возвращаемся назад на две точки
                this->MoveX(2*stepNastr*(-znak));
                break;
            }
        }
        if (!(viAmpMax.size()%((int)(35/stepNastr))))
        {
            this->pFazus->Stop_fazus();
            emit error("Проверьте расположение втулки");
            return 1;
        }
    }
    //  Находимся в максимуме, мерим время
    this->pFazus->one_shot_pin(nameNastr.toUtf8().data(),&itimeX2); // померили время прихода эхо сигнала в 4 точке
    // создаем правильную точоку начала
    sdvigX = ((itimeY1-itimeY2)/2)/2;
    sdvigX=sdvigX*1500/100/1000;    // пересчитываем отсчеты в мм смещения

    sdvigY = ((itimeX1-itimeX2)/2)/2;
    sdvigY=sdvigY*1500/100/1000;

    Data.clear();
    Data.append("time x1 "+QString::number(itimeX1));
    Data.append("time x2 "+QString::number(itimeX2));
    Data.append("time y1 "+QString::number(itimeY1));
    Data.append("time Y2 "+QString::number(itimeY2));
    Data.append("сдвиг x "+QString::number(sdvigX));
    Data.append("сдвиг y "+QString::number(sdvigY));
    {
        QMutexLocker locker(&vPpriemMessage_mutex);
        this->vPpriemMessage.append(Data);
    }
    emit answer();

    QString point1 = "1;"+QString::number(fxcoord1-sdvigX) + ";" + QString::number(fycoord1-sdvigY) + ";" + QString::number(zTmp+100)+";"+QString::number(0 + this->fOinstrShift) + ";180;0;";  // 07.06.16
    //emit error("точка 5 " + point1 + " значение сдвига по углу O " + QString::number(0 + this->fOinstrShift));
    Data.clear();
    Data.append(point1);

    if(this->SendCommand(Data,"point"," Ошибка создания точки",0)) return 1;
    if(this->MoveZ(fZmax)) return 1;
    SleeperThread::sleep(2);
    Data.clear();
    Data.append("16;");
    if(this->SendCommand(Data,"movestart"," Ошибка начала движения",1)) return 1;
    if (this->WaitingMoveFinish()) return 1;
    this->DeletePoint();

    if ( this->pFazus->Stop_fazus())
    {
        SleeperThread::msleep(200);
        this->pFazus->Stop_fazus();
    }

    // закончили калибровку и теперь на роботе сохраняем точку чтобы не потерять результат калибровки
    Data.clear();
    Data.append("64;");
    this->SendCommand(Data,"shiftsave","Ошибка сохранения смещения после калибровки",1);
    // координаты смещения сохранены на роботе но при загрузке точек используем обычное смещение надо сохранить текущие координаты кроме z

    // сохранили место после калибровки в смещениии.  Z координату надо будет поменять после нажатия кнопки начальная тчока
    this->fXShift = fxcoord1 - sdvigX;
    this->fYShift = fycoord1 - sdvigY;
    this->fZShift = zTmp + 100;
    this->fOShift = 0 + this->fOinstrShift;
    this->fAShift = 180;
    this->fTShift  = 0;
    // присохранение координаты через here углы сильно пляшут

    QByteArray message;
    message.clear();
    message.append("calibration finish");
    {
        QMutexLocker locker(&vPpriemMessage_mutex);
        this->vPpriemMessage.append(message);
    }
    emit answer();
    return 0;
}
int UdpClient::Orientation180(int del)
{
    QByteArray Data;
    if (this->DeletePoint()) return 1; //можно угробить уже залитую траекторию
    //  if (this->DeleteShift()) return 1;     //в результате при калибровке словили ошибку так не был стерто смещение сотрем смещние в калибровке
    /* координаты в блокнотовских файлах мы формируем относительно 0. Если такую координату
     залить в робота без смещения то робот попытается уехать себе в основание и сотановиться с ошибкой
     чтобы избежать этого изначально в роботе заданно смещение -1000 по z
     следовательно перед ориентацией фланца возьмем текущее смещение
     зададим 0 а после ориентации вернем обратно
     */
    this->Sdvig(); // метод должен записать координаты смещения в глобальные переменные
    QString stroka;
    stroka = "4;0;0;0;0;0;0;";
    Data.clear();
    Data.append(stroka);// запихиваем в пакет матрицу смещения
    if( this->SendCommand(Data,"shift","Ошибка задания смещения для ориентации фланца 1",0)) return 1;

    float xTmp = 0;
    float yTmp = 0;
    float zTmp = 0;
    float oTmp = 0;
    float aTmp = 0;
    float tTmp = 0;

   // int i;
    if (this->Here(&xTmp,&yTmp,&zTmp))return 1;  // 30.08.16 были потери ответа при получении координаты. заменил код на функцию

    /*{
        QMutexLocker locker(&vPpriemMessage_mutex);
        i = this->vPpriemMessage.size();
    }
    if (i == 0 )
    {
        this->pFazus->Stop_fazus();
        emit error("Потерял данные в векторее сообщений ориентация фланца 1");
        return 1;
    }
    {
        QMutexLocker locker(&vPpriemMessage_mutex);
        Data = this->vPpriemMessage[i-1];
    }
    // размер 1 а индекс первого ноль

    QString str(Data);
    if (!(str.contains("here",Qt::CaseInsensitive)))
    {
        this->pFazus->Stop_fazus();
        emit error("Не то прочитал из вектора сообщений фланец 1" + str);
        return 1;
        //не то
    }
    {
        QMutexLocker locker(&vPpriemMessage_mutex);
        this->vPpriemMessage.clear();
    }
    //  i = this->vPpriemMessage.size();
    //    emit error(QString::number(i) + " кол-во сообщений в векторе, посл собщение = " +vPpriemMessage[i-1]);
    QStringList strList = str.split(';');

    xTmp = strList[0].toFloat();
    yTmp = strList[1].toFloat();
    zTmp = strList[2].toFloat();
    */
    oTmp = 0 + this->fOinstrShift;   // добавили сдвиг так как другой фланец
    aTmp = 180;
    tTmp = 0;
    //emit error(QString::number(oTmp)+ " Угол О новой точки");
    Data.clear();
    Data.append("1;" + QString::number(xTmp) + ";" + QString::number(yTmp)+";" + QString::number(zTmp) + ";"+QString::number(oTmp)+";" + QString::number(aTmp)+";" + QString::number(tTmp)+";");
    // this->SendCommand(Data);
    if (this->SendCommand(Data,"point","Ошибка создания новой точки",0)) return 1;
    /*
     при создании точки робот вышлет лишнее собщение об отсувии калибровки
     информация об отсувии смещения добавлено внутрь собщение, в ответ одно сообщение теперь
     */

    Data.clear();
    Data.append("16;"); // команда поехать
    if (this->SendCommand(Data,"movestart","Ошибка ориентации фланца, движение",1)) return 1;
    // this->SendCommand(Data);

    if (this->WaitingMoveFinish()) return 1;
    Data.clear();
    Data.append("orientationfinish");
    {
        QMutexLocker locker(&vPpriemMessage_mutex);
        this->vPpriemMessage.append(Data);
    }

    //теперь вернем смещение обратно
    stroka.clear();
    stroka= "4;" + QString::number(this->fXShift)+";"+QString::number(this->fYShift)+";"+QString::number(this->fZShift)+";";
    stroka = stroka +QString::number(this->fOShift)+";"+QString::number(this->fAShift)+";"+QString::number(this->fTShift)+";";
    Data.clear();
    Data.append(stroka);// запихиваем в пакет матрицу смещения

    if( this->SendCommand(Data,"shift","Ошибка задания смещения после ориентации фланца",0)) return 1;

    emit answer();
    return 0;
}
int UdpClient::DeleteFileDef(QString name)
{
    int er;
    if (!(QFile::remove(name.toUtf8().data())))
    {
        SleeperThread::sleep(2);
        er = QFile::remove(name.toUtf8().data());
        if (er) emit error("Ошибка удалени файла");
        //emit error(" Ошибка удаления");
    }
    else
    {
        er =0;
        QByteArray Data;
        Data.append("filedel");
        {
            QMutexLocker locker(&vPpriemMessage_mutex);
            this->vPpriemMessage.append(Data);
        }
        emit answer();
        Data.clear();
        Data.append(9);  // отправляем максиму информацию о том контроль завершен результаты не сохранены
        this->sendInPipe(Data);
    }
    return er;
}
QString UdpClient::SearhIncreace(QString Nastr, int ampLow, int ampMax)
{ // функции надо передовать неполное имя без усиления и разрешения
    QString nameNastr;
    int increase=0;
    int ampC;
    QByteArray qbM;

    do   /// подбор величины усиления
    {
        increase += 2;
        if (increase >= 50)
        {
            emit error("Не смогли найти сигнал, подбор усилиения " + QString::number(increase));
            return ("NULL");
        }
        nameNastr = Nastr + QString::number(increase)+"db.nst";
        ampC = this->pFazus->one_shot(nameNastr.toUtf8().data());
        qbM.append(QString::number(ampC) +" ампл ");
        {
            QMutexLocker locker(&vPpriemMessage_mutex);
            this->vPpriemMessage.append(qbM);
        }
        emit answer();
        if (ampC>256)
        {
            emit error("Нет связи с фазусом "+QString::number(ampC-255));
            return ("NULL");
        }
    }while(ampC < ampLow || ampC > ampMax);

    return  nameNastr;
}
int UdpClient::MoveX(float value)
{
    int er;
    float x,y,z;
    if (this->Here( &x, &y, &z)) return 1;
    if (((x+value) < (this->fWZx1)) || (x+value > this->fWZx2))
    {
        emit error(" Точка назначения за границами рабочей зоны X.");
        return 1;
    } 
    QByteArray Data;
    QString stroka="21;";
    stroka = stroka + QString::number(value)+";";
    Data.clear();
    Data.append(stroka);
    er = this->SendCommand(Data,"movefinish","Ошибка перемещение X",1,15000,1);
    return er;
}
int UdpClient::MoveY(float value)
{
    int er;
    float x,y,z;
    if (this->Here( &x, &y, &z)) return 1;
    if (((y+value) < (this->fWZy1)) || (y+value > this->fWZy2))
    {
        emit error(" Точка назначения за границами рабочей зоны Y.");
        return 1;
    }
    QByteArray Data;
    QString stroka="21;0;";
    stroka = stroka + QString::number(value)+";";
    Data.clear();
    Data.append(stroka);
    er = this->SendCommand(Data,"movefinish","Ошибка перемещение Y",1,15000,1);
    return er;
}
int UdpClient::MoveZ(float value)
{
    int er;
    float x,y,z;
    if (this->Here( &x, &y, &z)) return 1;
    if (((z+value) < (this->fWZz1)) || (z+value > this->fWZz2))
    {
        emit error(" Точка назначения за границами рабочей зоны Z");
        return 1;
    }
    QByteArray Data;
    QString stroka="21;0;0;";
    stroka = stroka + QString::number(value)+";";
    Data.clear();
    Data.append(stroka);
    er = this->SendCommand(Data,"movefinish","Ошибка перемещение Z",1,15000,1);
    this->flagCircle = 0;
    return er;
}
int UdpClient::SetSpeed(float value)
{
    int er;
    QByteArray Data;
    QString stroka="5;";
    stroka=stroka+QString::number(value)+";";
    Data.clear();
    Data.append(stroka);
    er = this->SendCommand(Data,"speed","Ошибка установки скорости",1);
    return er;
}
int UdpClient::SetMech(float value)
{
    int er;
    QByteArray Data;
    QString stroka="6;";
    stroka=stroka+QString::number(value)+";";
    Data.clear();
    Data.append(stroka);
    er = this->SendCommand(Data,"mesh scan","Ошибка установки координатной сетки",1);
    return er;
}
int UdpClient::ContinueWork()
{
    int er;
    QByteArray Data;
    Data.clear();
    Data.append("97;");
    er = this->SendCommand(Data,"continue","Ошибка перезапуска программы",1);
    return er;
}
int UdpClient::ResetError()
{
    int i;
    QByteArray Data;
    Data.clear();
    Data.append("56;");
    if (this->SendCommand(Data,"error","Ошибка квитирования робота",1)) return 1;
    {
        QMutexLocker locker(&vPpriemMessage_mutex);
        i = this->vPpriemMessage.size();
    }
    if (i == 0 )
    {
        emit error("Потерял данные в векторее сообщений квитирование");
        return 1;
    }
    {
        QMutexLocker locker(&vPpriemMessage_mutex);
        Data = this->vPpriemMessage[i-1];
    }

    // размер 1 а индекс первого ноль

    QString str(Data);
    if (!(str.contains("error",Qt::CaseInsensitive)))
    {
        emit error("Не то прочитал из вектора сообщений квитирование" + str);
        return 1;//не то
    }
    QStringList strList = str.split(';');
    int er = strList[0].toInt();

    return er;
}
int UdpClient::MotorOn()
{
    QByteArray Data;
    Data.clear();
    Data.append("57;");
    if (this->SendCommand(Data,"power","Ошибка включения моторов",0)) return 1;

    return 0;
}
int UdpClient::GoHome()
{
    int er;
    er = this->getRS10parametr();
    if ( er == 1)
    {
        return 1;
    }
    if ( er == 3)
    {
        if(this->MoveZ(200)) return 1;
    }
    else
    {
        if(this->MoveZ(this->iMaxZ)) return 1;
    }

    QByteArray Data;
    QString stroka="51;";
    Data.clear();
    Data.append(stroka);
    // emit error("dshrgjkl");
    er = this->SendCommand(Data,"gohome","Ошибка возврата на стартовую позицию",1);
    if (er ==0)
    {
        Data.clear();
        Data.append("Робот в стартовой позиции");
        {
            QMutexLocker locker(&vPpriemMessage_mutex);
            this->vPpriemMessage.append(Data);
        }
        emit answer();
    }
    return er;
}
int UdpClient::SetWorkSpace(float x1,float y1,float z1,float x2,float y2,float z2)
{
    int er;
    QByteArray Data;
    QString stroka="25;";
    stroka=stroka+QString::number(x1)+";"+QString::number(y1)+";"+QString::number(z1)+";"+QString::number(x2)+";"+QString::number(y2)+";"+QString::number(z2)+";";
    Data.clear();
    Data.append(stroka);
    er = this->SendCommand(Data,"workpoint","Ошибка установки рабочей зоны",1); // вообще робот отвечает двумя командами
    return er;
}
int UdpClient::GetWorkSpace(float *x1,float *y1,float *z1,float *x2,float *y2,float *z2)
{
    int er;
    QByteArray Data;

    QString stroka="50;";
    Data.clear();
    Data.append(stroka);
    er = this->SendCommand(Data);
    if (this->socket->waitForReadyRead(2000))
    {
        if (this->ReadData()) return 1;
        {
            QMutexLocker locker(&vPpriemMessage_mutex);
            int i = this->vPpriemMessage.size();
            Data = this->vPpriemMessage[i-1];
        }

        // размер 1 а индекс первого ноль

        QString str(Data);

        QStringList message = str.split(";");

        int q = message.size();

        if (q != 7)
        {
            emit error("Ошибка определения рабочей зоны");
            return 1;
            //не то
        }
        else
        {
            *x1 = message[0].toFloat();
            *y1 = message[1].toFloat();
            *z1 = message[2].toFloat();
            *x2 = message[3].toFloat();
            *y2 = message[4].toFloat();
            *z2 = message[5].toFloat();
        }
    }
    return er;
}
int UdpClient::WaitingMoveFinish()
{
    int i = 0;
    if ((this->socket->pendingDatagramSize())==-1)
    {
        if (!(this->socket->waitForReadyRead(40000)))  // ждем move finish
        {
            this->pFazus->Stop_fazus();
            emit error("Не дождались сигнала завершения движения");
            return 1;
        }
    }
    if (int er = this->ReadData())
    {
        if (er == 2)
        {
            while (er)// в режиме движения после прохождения первой точки робот постоянно шлет свои координаты с меткой move point
            {
                if (this->socket->waitForReadyRead(2000))
                {
                    er = this->ReadData();
                    if (er==1)
                    {
                        emit error("Ошибка получения данных ожидание заверщения движения 1");
                        this->pFazus->Stop_fazus();
                        return 1;
                    }
                }
            }
        }
        else
        {
            emit error("Ошибка получения данных ожидание завершения движения 2");
            this->pFazus->Stop_fazus();
            return 1;
        }
    }
    {
        QMutexLocker locker(&vPpriemMessage_mutex);
        i = this->vPpriemMessage.size();
    }

    if (i ==0 )
    {
        this->pFazus->Stop_fazus();
        emit error("Потерял данные в векторее сообщений ожидание завершения движения 3 точка");
        return 1;
    }
    QByteArray Data;
    {
        QMutexLocker locker(&vPpriemMessage_mutex);
        Data = this->vPpriemMessage[i-1];
    }

    QString str(Data);
    if (!(str.contains("move finish",Qt::CaseInsensitive)))
    {
        this->pFazus->Stop_fazus();
        emit error("Не то прочитал из вектора сообщений, ждем move finish " + str);
        return 1;//не то
    }

    return 0;
}
int UdpClient::StartControl()
{
    QByteArray bPipe;
    int er=0;
    if (this->UploadShift())
    {
        return 1;//не получили смещения
    }
    if (this->GetDigitalInput()) return 0;
    if (!(this->bLevelVanna))
    {
        emit error("Проверьте уровень воды в ванне");
        return 0;
    }
    if ((this->getRS10parametr())&&er)
    {
        return 1;// ошибка
    }
    else
    {
        this->vectorC.clear();
        QByteArray data;
        if (this->pFazus)
        {
            for (int i=0; i<128;i++) // переписываю данные настроек
            {
                this->infoScan[i]=this->pFazus->infoScan[i];
            }  // обращение в обьект дефеткоскоп через указатель
        }
        else
        {
            emit error("Ошибка работы с фазусом копирование настроек");
            return 1;
        }
        if (!(this->bPipeOpen))
        {
            // emit error("Создаем пайп");
            this->createPipe("имя пока не обрабатывается");
        }
        if (this->bPipeOpen)
        {
            SleeperThread::sleep(1);
            bPipe.clear(); // перед стартом отправить в пайп настройки
            bPipe[0] = 2;
            bPipe[1] = this->iNumPoint;
            bPipe[2] = this->iNumPoint>>8;
            bPipe[3] = this->iNumPoint>>16;
            bPipe[4] = this->iNumPoint>>24;
            bPipe[5] = this->iMaxZ;
            bPipe[6] = this->iMaxZ>>8;
            bPipe[7] = this->iMaxZ>>16;
            bPipe[8] = this->iMaxZ>>24;
            this->sendInPipe(bPipe);
            bPipe.clear();

            SleeperThread::sleep(3);

            bPipe.append(5); // маркер посылки

            //  bPipe.append(this->infoScan[25]);// кол-во каналов
            bPipe.append(1);// кол-во каналов
            bPipe.append(this->infoScan[9]);// амплитуда строба первого канала
            bPipe.append(this->infoScan[34]); // строб канал 2
            bPipe.append(this->infoScan[11]); // флаг логики стробов
            bPipe.append(this->infoScan[36]); // флаг логики стробов творого канала
            bPipe.append(this->infoScan[21]); // амплитуда опорного строба 1 канал
            bPipe.append(this->infoScan[46]); // амплитуда опорного строба 2 канал
            bPipe.append(this->infoScan[10]); // смещение
            char float_bytes[sizeof(float)];
            memcpy(float_bytes, &fStepAngle, sizeof(float)); // угловая развертка
            for (int i = 0; i < sizeof(float); ++i)
            {
                bPipe.append(float_bytes[i]);
            }
            this->sendInPipe(bPipe);
            bPipe.clear();
        }

        data.append("16;");
        //data.append("2;");
        if (this->SendCommand(data,"movestart","Ошибка начала движения", 1))
        {
            return 1;
            //ошибка и так должна будет выведена из функции
        }
        else
        {
            this->bDef = true;
            // this->timer->start();
        }
        //робот начал двжиение
        //фазус еще не запущен
        //если получен ответ движение начато запустить сьем данных из интрфейсного потока
    }
    return 0;
}
void UdpClient::SetWorkSpaceINpc(float x1, float y1, float z1, float x2, float y2, float z2)
{
    this->fWZx1 = x1;
    this->fWZy1 = y1;
    this->fWZz1 = z1;
    this->fWZx2 = x2;
    this->fWZy2 = y2;
    this->fWZz2 = z2;
}
void UdpClient::EventTimer()
{
    // emit error("Попали в обработчик таймера");
    int iError = this->GetErrorRobot();

    if ((iError)||(this->TestMotor() == 2))
    {
        QByteArray data;  // робот скорее  всего уже остановился но в ответ на команду стоп он вышлет сигнал завешения по которому мы сохраним данные
        this->timer->stop();
        data.clear();
        data.append("99;");  // стоп отправляем напрямки без подверждений чтобы отправка проходила как можно быстрее
        this->SendCommand(data,500, 1);  // не дождались ответа - сами заканчиваем процесс снятия данных

        this->pFazus->StopDef();
        data.clear();
        data.append("контроль нарушен");
        {
            QMutexLocker locker(&vPpriemMessage_mutex);
            this->vPpriemMessage.append(data);
        }

        emit answer();   // создали сообщения для индикации о завершении контроля

        bDef = false;
    }
    else
    {
        //  2секунды робот молчал но статус ошибки 0 - скорее всего мы разворачиваем фланец
        //пукскаем таймер заново. по факту надо запросить параметры робота и при повторном попадании сранить отличаеться ли счетчик шагов
        this->timer->start();
    }
}
int UdpClient::GetErrorRobot()
{
    int i=0;
    QByteArray data;
    data.append("55;");
    if (this->SendCommand(data,"error","Ошибка получения статуса робота",1))return 1;

    {
        QMutexLocker locker(&vPpriemMessage_mutex);
        i = this->vPpriemMessage.size();
    }

    if (i == 0 )
    {
        emit error("Потерял данные в векторее сообщений получения статуса робота");
        return 1;
    }
    {
        QMutexLocker locker(&vPpriemMessage_mutex);
        data = this->vPpriemMessage[i-1];
    }

    // размер 1 а индекс первого ноль

    QString str(data);
    if (!(str.contains("error",Qt::CaseInsensitive)))
    {
        emit error("Не то прочитал из вектора сообщений статус робота" + str);
        return 1;//не то
    }
    QStringList strList = str.split(';');
    str = strList[0];
    int er;
    er = str.toInt();
    if (er != 0)
    {
        data.clear();
        data.append( "Номер ошибки " + str);
        {
            QMutexLocker locker(&vPpriemMessage_mutex);
            this->vPpriemMessage.append(data);
        }

        emit answer();
    }
    return er;
}
int UdpClient::ResumeAfterError()
{
    QByteArray data;
    if (this->ResetError()) return 1;
    SleeperThread::msleep(10);
    if (this->MotorOn()) return 1;
    SleeperThread::msleep(10);
    if (this->ContinueWork()) return 1;

    data.append( "Ошибка квитированна " );
    {
        QMutexLocker locker(&vPpriemMessage_mutex);
        this->vPpriemMessage.append(data);
    }

    emit answer();

    return 0;
}
int UdpClient::Init()
{
    this->vectorC.clear();
    {
        QMutexLocker locker(&vPpriemMessage_mutex);
        this->vPpriemMessage.clear();
    }
    this->bDef = false;
    this->bPipeOpen = false;
    return 0;
}
int UdpClient::TestMotor()
{
    QByteArray Data;
    Data.append("58;");// запрос включен ли двигатель
    int er = this->SendCommand(Data,"motor","Ошибка получения статуса мотора",2);
    if ( er == 0 )
    {
        {
            QMutexLocker locker(&vPpriemMessage_mutex);

            int i = this->vPpriemMessage.size();
            if (i == 0 )
            {
                emit error("Потерял данные в векторее сообщений получение статуса мотора");
                return 1;
            }
            while (i > 0)   //24.08/16 в SendComand теперь перебираем весь вектор принятых сообщений поэтому првоеряем и здесь где именно лежит ответ
            {
              Data = this->vPpriemMessage[i-1];
              QString str(Data);
              if (str.contains("motor",Qt::CaseInsensitive)) break;
              i--;
            }

            QString str(Data);
            if (str.contains("motor")) //два раза проверяю одно и то же!
            {
                if(str.contains("on"))
                {
                    Data.clear();
                    Data.append( " Мотор включен " );
                    this->vPpriemMessage.append(Data);
                    emit answer();
                }
                if(str.contains("off"))
                {
                    Data.clear();
                    Data.append( " Мотор  не включен " );
                    this->vPpriemMessage.append(Data);
                    emit answer();
                    return 2;
                }
            }
            else
            {
                emit error("Ошибка определения статуса мотора");
                return 1;
            }
        }
    }
    return 0;
}
int UdpClient::GetDigitalInput()
{
    QByteArray Data;
    Data.append("33;0;");// запрос включен ли двигатель
    int er = this->SendCommand(Data,"in10-13","Ошибка получения статуса цифровых входов",1);
    if ( er ==0 )
    {
        {
            QMutexLocker locker(&vPpriemMessage_mutex);


            int i = this->vPpriemMessage.size();
            if (i ==0 )
            {
                emit error("Потерял данные в векторее сообщений получение статуса цифровых входов");
                return 1;
            }
            Data = this->vPpriemMessage[i-1];
            QString str(Data);
            if (str.contains("in10-13"))
            {
                QStringList strList = str.split(';');
                if (strList[0].toInt() == 0)
                {
                    this->bLevelVanna = true;
                    Data.clear();
                    Data.append( " Достаточный уровень воды в ванне" );
                    this->vPpriemMessage.append(Data);
                    emit answer();
                }
                else
                {
                    this->bLevelVanna = false;
                    Data.clear();
                    Data.append( "Проверьте уровень воды в ванне" );
                    this->vPpriemMessage.append(Data);
                    emit answer();
                }
                //if (strList[1].toInt() == 0) \\15 02 16 сигнал приходит инверсный
                if (strList[1].toInt())
                {
                    this->bLevelBak = true;
                    Data.clear();
                    Data.append( " Достаточный уровень воды в баке" );
                    this->vPpriemMessage.append(Data);
                    emit answer();
                }
                else
                {
                    this->bLevelBak = false;
                    Data.clear();
                    Data.append( "Проверьте уровень воды в баке" );
                    this->vPpriemMessage.append(Data);
                    emit answer();
                }
                if (strList[2].toInt() == 0)
                {
                    //     this->bLevelBak = false;
                    // здесь можно узнать о состоянии насоса и далее
                }
                else
                {
                    //        this->bLevelBak = true;
                }
                if (strList[3].toInt() == 0)
                {
                    //      this->bLevelBak = false;
                }
                else
                {
                    //      this->bLevelBak = true;
                }
            }
            else
            {
                emit error("Ошибка определения статуса цифровых входов");
                return 1;
            }
        }
    }
    return 0;
}
int UdpClient::DeleteShift()
{
    int er;
    QByteArray Data;
    Data.clear();
    Data.append("19");
    er =this->SendCommand(Data,"shiftdel", " Ошибка удаления смещения",2);
    return er;
}
int UdpClient::SetCircle(float r,float step)
{
    QByteArray Data;
    Data.clear();
    QString stroka= "60;";
    stroka = stroka + QString::number(r)+";" +QString::number(step)+";";
    Data.append(stroka);
    if (this->SendCommand(Data,"circlecreate", " Ошибка создания круговой траектории",1)) return 1;
    this->flagCircle = 1; // траектория для движения по дуге созданна
    return 0;
}
int UdpClient::ChangeMoveMode(int mode)
{
    QByteArray Data;
    Data.clear();
    QString stroka="61;";
    stroka = stroka+QString::number(mode)+";" ;
    Data.append(stroka);
    this->SendCommand(Data,"setmode", " Ошибка переключения режима движения",1);
    return 0;
}
int UdpClient::Move2step(float step,int napr)
{
    if (!(this->flagCircle))
    {
        this->SetCircle(this->rHub,this->fStepAngle);
    }
    QByteArray Data;
    Data.clear();
    QString stroka = "62;";
    stroka = stroka+QString::number(step)+";" + QString::number(napr)+";";
    Data.append(stroka);
    this->SendCommand(Data);
    if (this->ReadData())
        return 1;
    {
        QMutexLocker locker(&vPpriemMessage_mutex);
        int i = this->vPpriemMessage.size();
        Data  = this->vPpriemMessage[i-1];// размер 1 а индекс первого ноль
    }
    emit answer(); // проверочная строка чтобы посмотреть ответ

    QString str(Data);
    if ((str.contains("wait",Qt::CaseInsensitive)))
    {
        emit error("Дождитесь завершения движения");
        return 0;
    }

    if ((str.contains("step",Qt::CaseInsensitive)))
    {
        // все хорошо
        return 0;
    }

    if (((str.contains("move",Qt::CaseInsensitive)))) return 0;

    emit error("Ошибка перемещения по окружности " + str);
    return 1;
}
int UdpClient::Move2degree(float degree,int napr)
{
    if (!(this->flagCircle))
    {
        this->SetCircle(this->rHub,this->fStepAngle);
    }
    QByteArray Data;
    Data.clear();
    QString stroka = "63;";
    stroka=stroka+QString::number(degree)+";" + QString::number(napr)+";";
    Data.append(stroka);
    this->SendCommand(Data);
    if (this->ReadData()) return 1;
    {
        QMutexLocker locker(&vPpriemMessage_mutex);
        int i = this->vPpriemMessage.size();
        Data = this->vPpriemMessage[i-1];
    }
    emit answer(); // проверочная строка чтобы посмотреть ответ
    // размер 1 а индекс первого ноль
    QString str(Data);
    if ((str.contains("wait",Qt::CaseInsensitive)))
    {
        emit error("Дождитесь завершения движения");
        return 0;
    }

    if ((str.contains("step",Qt::CaseInsensitive)))
    {
        // все хорошо
        return 0;
    }

    if ((str.contains("move",Qt::CaseInsensitive)))return 0;    // словили move finish от предудущего шага
    emit error("Ошибка перемещения по окружности " + str);
    return 1;
}
UdpClient::~UdpClient()
{
    // this->brun = false;   // всеравно завершаеться неверно
}
