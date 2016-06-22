#include "fazus.h"
#include "FAZUS_DLL.h"
#include <QMessageBox>
#include <QDate>



//#pragma comment(lib, "C:\\prog\\DEF9\\defectoscop\\FAZUS_DLL.lib")
#pragma comment(lib, "F:\\prog qt\\NEWWW\\udp-client-2\\udp-client\\dll\\FAZUS_DLL.lib")
fazus::fazus(QObject *parent) : QObject(parent)
{
    this->flagNastr = 0;
    this->number_def = 0;
    this->ascan_write = false;
    this->bin_def = true; // раньше можно было переключить в бинарнывй режим когда
    //дефектоскоп возвращал только да нет
    this->bstop = false;
}

int fazus::Process()    // дефектоскопим.
// два режима работы с фазусом- получение только бинарных данных или получения аплитуд
// по флагам сохранение всех амплитудных значений или только максимума в стробе
{
    if(this->nameFileNastr != "")
    {
        int errr;
        errr = Contr_Start(this->nameFileNastr.toUtf8().data());
        if (errr)
        {
            QString sEr;
            sEr="Нет связи с фазусом. Настройка ";
            sEr = sEr+QString::number(errr);
            qDebug ("er22");
            emit error(sEr);
            return 1;
        }
    }
    else
    {
        emit error(" Неверное имя настройки");
        return 1;
    }
    data = QDateTime::currentDateTime();
    tempName = ".\\_BD_DEF\\" + data.toString("yy-MM-dd-HH-mm");
    if (this->ascan_write)
    {
        nameFileDef = tempName + ".cda";
        // QFile bfile(nameFileDef);
        this->bfile =new QFile(nameFileDef);
        if (!this->bfile->open(QIODevice::WriteOnly))
            // if (int  er = _sopen_s( &fh1, nameFileDef.toUtf8().data(), _O_BINARY | _O_RANDOM | _O_WRONLY | _O_TRUNC| _O_CREAT, _SH_DENYNO,_S_IWRITE))
        {
            this->bstop=false;
            emit error("Не удалось открыть файл");
        }
    }
    else
    {
        nameFileDef = tempName + ".cd";
    }

    this->number_def = 0;
    this->Rasc_Tabl();
    this->bin_def_arrayCh1.clear();  // фазус возвращает 1 или 0
    this->amp_def_arrayCh1.clear();  // все значения выборки
    this->amp_def_arrayStrobCh1.clear(); // сюда сохраняем только максимум в стробе
    this->amp_oporn_arrayStrobCh1.clear();
    this->amp_oporn_arrayStrobCh2.clear();
    this->max_ampCh1S1 = 0;
    this->max_ampCh2S1 = 0;
    this->qbWrite.clear();

    while(bstop)
    {
        if (bin_def)
        {
            this->tmp = 0;
            this->tmp_time = 0;
            this->max_ampCh1S1 = 0;
            this->max_ampCh2S1 = 0;
            this->t1c1s1 = (int)(this->strob_can[0].t1[0]*100 - this->cnl_dt[0].d_zad_can*100);
            this->t1c2s1 = (int)(this->strob_can[1].t1[0]*100 - this->cnl_dt[1].d_zad_can*100); // чтобы при выпадении из опрного строба взять старое значение

            int j = AScan_Rec((unsigned char*)b1, (unsigned char*)b2, NULL, NULL, NULL, NULL, NULL, NULL,0);

            if (j !=0)
            {
                this->bstop=false;  // нет связи с фазусом закрываю поток и вывожу сообщение
                QString str;
                str="Нет связи с фазусом получение данных  ";
                str = str+QString::number(j);
                // this->Stop_fazus();
                emit error(str);
            }
            if (this->ascan_write)
            {
                //this->max_ampCh1S1=0;

                //-
                if (this->strob_can[0].fl_op)
                {
                    for ( int t = (int)(this->strob_can[0].t1_op*100 - this->cnl_dt[0].d_zad_can*100);t < (int)(this->strob_can[0].t1_op*100 - this->cnl_dt[0].d_zad_can*100 + this->strob_can[0].t2_op*100);t++)
                    {
                        this->tmp = b1[t];
                        if (this->tabl_x[(int)max_ampCh1S1] < this->tabl_x[(int)tmp])
                        {
                            this->max_ampCh1S1 = tmp;                   // максимум в опорном стробе
                            tmp_time = t+(int)(this->strob_can[0].cc[0]*100);    // индекс максимума в опорном стробе
                        }
                    }
                    if ((this->max_ampCh1S1) > (this->strob_can[0].am_op))
                    {
                        this->t1c1s1=tmp_time; // подменяем время начала развертки  если есть сигнал в опорном стробе
                    }
                    this->amp_oporn_arrayStrobCh1.append(this->max_ampCh1S1);
                    this->max_ampCh1S1 = 0;
                }
                //-

                for (int il = 0;il<this->cnl_dt[0].pt_kl;il++)
                {
                    amp_def_arrayCh1.append(b1[il]); // пишем все значения
                    if ((il>=t1c1s1)&&(il<=(t1c1s1+strob_can[0].t2[0]*100)))
                        if (this->tabl_x[(int)max_ampCh1S1]<tabl_x[(int)b1[il]]) this->max_ampCh1S1=(int)b1[il];
                }
                this->bfile->write(this->amp_def_arrayCh1, this->amp_def_arrayCh1.size());
                //_write(fh1, this->amp_def_arrayCh1, this->amp_def_arrayCh1.size());
                this->amp_def_arrayCh1.clear();
                // this->usleep(1);
                if (this->pov_par.n_canal==2)  // второй канал
                {
                    this->max_ampCh2S1=0;
                    for (int il=0;il<this->cnl_dt[1].pt_kl;il++)
                    {
                        amp_def_arrayCh2.append(b2[il]); // пишем все значения
                        //  if ((il>=this->strob_can[1].t1[0]*100)&&(il<=(this->strob_can[1].t1[0]*100+this->strob_can[1].t2[0]*100)))
                        //     if (this->max_ampCh2S1<tabl_x[(int)b2[il]]) this->max_ampCh2S1=(int)b2[il];
                    }
                    this->bfile->write(this->amp_def_arrayCh2, this->amp_def_arrayCh2.size());
                    // _write(fh1, this->amp_def_arrayCh2, this->amp_def_arrayCh2.size());
                    this->amp_def_arrayCh2.clear();
                    // this->usleep(1);
                }
            }
            else  // запись только амплитуд в стробах
            {
                tmp_time = 0;
                if (this->strob_can[0].fl_op)
                {
                    for ( int t = (int)(this->strob_can[0].t1_op*100 - this->cnl_dt[0].d_zad_can*100);t < (int)(this->strob_can[0].t1_op*100 - this->cnl_dt[0].d_zad_can*100 + this->strob_can[0].t2_op*100);t++)
                    {
                        this->tmp = b1[t];
                        if (this->tabl_x[(int)max_ampCh1S1] < this->tabl_x[(int)tmp])
                        {
                            this->max_ampCh1S1 = tmp;                   // максимум в опорном стробе
                            tmp_time = t+(int)(this->strob_can[0].cc[0]*100);    // индекс максимума в опорном стробе
                        }
                    }
                    if ((this->max_ampCh1S1) > (this->strob_can[0].am_op)) // амплитуда максимума в опорном стробе  должна быть через table_x
                        //if ((this->tabl_x[this->max_ampCh1S1]) > (this->strob_can[0].am_op))
                    {
                        this->t1c1s1 = tmp_time; // подменяем время начала развертки  если есть сигнал в опорном стробе
                    }
                    this->amp_oporn_arrayStrobCh1.append(this->max_ampCh1S1);
                    this->max_ampCh1S1 = 0;
                }

                for (int i_c = this->t1c1s1; i_c<t1c1s1+strob_can[0].t2[0]*100; i_c++)
                {
                    this->tmp = b1[i_c];
                    if (this->tabl_x[(int)max_ampCh1S1] < this->tabl_x[(int)tmp])
                    {
                        this->max_ampCh1S1 = tmp;
                    }
                }
                // здесь логично сохранять амплитуду первого канала
                if(this->pov_par.n_canal == 2)
                {
                    tmp_time = 0;
                    if(this->strob_can[1].fl_op)
                    {
                        for ( int t = (int)(this->strob_can[1].t1_op*100 - this->cnl_dt[1].d_zad_can*100) ;t<(int)(this->strob_can[1].t1_op*100 - this->cnl_dt[1].d_zad_can*100 + this->strob_can[1].t2_op*100);t++)
                        {
                            this->tmp = b2[t];
                            if (this->tabl_x[(int)max_ampCh2S1] < this->tabl_x[(int)tmp])
                            {
                                this->max_ampCh2S1 = tmp;
                                tmp_time = t+(int)(this->strob_can[1].cc[0]*100); // индекс максимума в опорном стробе
                            }
                        }
                        if (this->max_ampCh2S1 > this->strob_can[1].am_op) // амплитуда максимума в опорном стробе  должна быть через table_x
                        {
                            this->t1c2s1 = this->tmp_time;
                        }
                        this->amp_oporn_arrayStrobCh2.append(this->max_ampCh2S1);
                        this->max_ampCh2S1 = 0;
                    }

                    for (int i_c = this->t1c2s1; i_c<(t1c2s1+strob_can[1].t2[0]*100);i_c++)
                    {
                        this->tmp = b2[i_c];
                        if (this->tabl_x[(int)max_ampCh2S1]<this->tabl_x[(int)tmp])
                        {
                            this->max_ampCh2S1 = this->tmp;
                        }
                    }
                    // - перенести запись второго канала сюда и не делать лишнею проверку
                }
            }
            //             this->amp_def_arrayStrobCh1.append((sin(this->number_def/1000*180/3.14)+1)*127);// отдельно сохраняем максимум в стробе
            this->amp_def_arrayStrobCh1.append(this->max_ampCh1S1);// отдельно сохраняем максимум в стробе
            if (this->pov_par.n_canal == 2) this->amp_def_arrayStrobCh2.append(this->max_ampCh2S1);// отдельно сохраняем максимум в стробе
            this->max_ampCh1S1=0;
            this->max_ampCh2S1=0;
        }
        else
        {
            /*
               int  k = Def_ac((unsigned char*)b1);
               if (k)
                   {
                     this->bstop=false;  // нет связи с фазусом закрываю поток и вывожу сообщение
                 //    client.stopRobot();
                     QMessageBox msgBox;
                     QString str;
                     str="Нет связи с фазусом  ";
                     str = str+QString::number(k);
                     msgBox.setText(str);
                //     msgBox.exec();
                   }
                          if (b1[0]==1)
                             {
                               bin_def_arrayCh1.append((char)1);
                             }
                             else
                             {
                             bin_def_arrayCh1.append((char)0);
                             }
               */
        }
        this->number_def++;

    }
    if (this->Stop_fazus())
    {
        SleeperThread::msleep(100);
        if (this->Stop_fazus())
        {
            emit error("Не удалось закрыть фазус, для повторного контроля перезапустите программу.");
        }
    }
    //вышли из бесконечного цикла - закончили контроль сохраняем данные
    if (bin_def)
    {
        if(this->ascan_write)
        {
            this->bfile->close();
            // _close(fh1);// пишем по охду в конце только закрываем файл
        }
        else
        {
            // здесь же и открываем , походу записи нет
            // this->bfile =new QFile(nameFileDef);
            QFile file(nameFileDef);
            if (!file.open(QIODevice::WriteOnly))
                // if (int  er = _sopen_s( &fh1, nameFileDef.toUtf8().data() , _O_BINARY | _O_RANDOM | _O_WRONLY | _O_TRUNC| _O_CREAT, _SH_DENYNO,_S_IWRITE))
            {
                //this->bstop=false;
                // QString str;
                // str="не удалось открыть файл ";
                // str = str+QString::number(er);
                emit error(" Не удалось открыть файл");
            }
            unsigned int s = this->amp_def_arrayStrobCh1.size(); // Kokos 25.02.2016 Add "unsigned"
            //            char ch;

            /* */  QByteArray qbTemp;
            qbTemp.clear();
            for (unsigned int j = 0; j<s; j++)  // Kokos 25.02.2016 Add "unsigned"
            {
                qbTemp.append(this->amp_def_arrayStrobCh1[j]);
                if(this->strob_can[0].fl_op)
                {
                    qbTemp.append(this->amp_oporn_arrayStrobCh1[j]);
                }
                if(this->pov_par.n_canal == 2)
                {
                    qbTemp.append(this->amp_def_arrayStrobCh2[j]);
                    if(this->strob_can[1].fl_op)
                    {
                        qbTemp.append(this->amp_oporn_arrayStrobCh2[j]);
                    }
                }
            }
            // emit error(QString::number(qbTemp.size()));
            file.write(qbTemp,qbTemp.size());
            file.flush();
            SleeperThread::msleep(50);
            file.close();
            // int e = qbTemp.size();
            // _write(fh1,&qbTemp,e );
            /* */
            // for (int j=0;j<e;j++)
            // {
            //   ch = qbTemp[j];
            // _write(fh1,&ch,1 );
            //}

            /*
            for (int j=0;j<s;j++)
            {
                ch = this->amp_def_arrayStrobCh1[j];
                _write(fh1,&ch,1 );
                if(this->strob_can[0].fl_op)
                {
                    ch = this->amp_oporn_arrayStrobCh1[j];
                    _write(fh1,&ch,1 );
                }
              if(this->pov_par.n_canal == 2)
              {
                ch = this->amp_def_arrayStrobCh2[j];
                _write(fh1,&ch,1 );

                if(this->strob_can[1].fl_op)
                {
                    ch = this->amp_oporn_arrayStrobCh2[j];
                    _write(fh1,&ch,1 );
                }
              }

            }
            */
            // _close(fh1);
        }
    }
    else
    {
        // здесь должна быть запись в файл в режиме бинарной дефектограммы
    }
    //   this->Stop_fazus();
    //emit finished(); не запустить контроль повторно
    return 0;

    /**/
}
void fazus::OneShot()
{
    int errr = AScan_Rec((unsigned char*)b1, NULL, NULL, NULL, NULL, NULL, NULL, NULL,0);
    if (errr !=0)
    {
        QString sEr;
        sEr="Нет связи с фазусом.Однократное прозвучивание ";
        sEr = sEr+QString::number(errr);
        emit error(sEr);
    }
    else
    {
        this->amp_def_array.clear();
        for (int il=0;il<15000;il++)
        {
            amp_def_array.append(b1[il]); // пишем все значения
        }
        emit scan();
    }
}
void fazus::Nastr(QString fileName)
{
    //this->Stop_fazus();
    // SleeperThread::msleep(300);
    char chStrobCanal1;
    char chStrobCanal2;
    //bit 0 - флаг наличия строба 1
    //bit 1 - флаг наличия строба 2
    // bit 2 - флаг наличия опрного строба
    // bit3 - логика строба 1
    // bit 4 - логика строба 2
    // bit 5 - флаг метода опорного строба канала 1

    //    int errr=1;
    //    errr = Contr_Start(fileName.toUtf8().data());
    //    if (errr)
    //    {
    //        QString sEr;
    //        sEr="Нет связи с фазусом. Настройка ";
    //        sEr = sEr+QString::number(errr);
    //        qDebug ("er22");
    //        emit error(sEr);
    //    }
    //    else
    {
        int fh = _sopen(fileName.toUtf8().constData(), _O_BINARY | _O_RANDOM | _O_RDONLY, _SH_DENYNO);
        _read(fh, &cnl_dt, 8 * sizeof(CANAL_DT));
        _read(fh, &strob_can, 8 * sizeof(STROB_DT));
        _read(fh, &pov_par, sizeof(POVERHNOST));
        _close(fh);
        this->flagNastr = 1;

        memset(this->infoScan,0,sizeof(infoScan));     // предворительно обнуляем массив

        this->infoScan[0]=23;// версия программы
        // 23 нет информации по шагу в градусах 24 - инф в 60 байте

        this->infoScan[1]=(this->cnl_dt[0].pt_kl&0x000000FF);    // длинна выборки (кол-во отсчетов)
        this->infoScan[2]=(this->cnl_dt[0].pt_kl&0x0000FF00)>>8; // берем только два байта так не может быть число больше 15000

        this->infoScan[3]=(int(this->cnl_dt[0].d_zad_can*100)&0x000000FF);  // общая задержка канала 1 в отсчетах
        this->infoScan[4]=(int(this->cnl_dt[0].d_zad_can*100)&0x0000FF00)>>8;

        this->infoScan[5]=(int((this->strob_can[0].t2[0])*100)&0x000000FF);  // длина строба *100 - в отчетах
        this->infoScan[6]=(int((this->strob_can[0].t2[0])*100)&0x0000FF00)>>8;

        this->infoScan[7]=(int(((this->strob_can[0].t1[0])*100)-(this->cnl_dt[0].d_zad_can*100))&0x000000FF);  // начало строба в отсчетах сдвинутое на задержку
        this->infoScan[8]=(int(((this->strob_can[0].t1[0])*100)-(this->cnl_dt[0].d_zad_can*100))&0x0000FF00)>>8;

        this->infoScan[9]=(this->strob_can[0].am[0]&0x000000FF);  // амплитуда строба

        this->infoScan[10]=((this->pov_par.smech+21)&0x000000FF);  // смещение развертки относительно 0 (амплитудное)
        // сдвигаем на 21 в + чтобы одним байтом передовать отрицательные и положительные значения
        if (this->strob_can[0].fl[0])
        {
            chStrobCanal1=b00000001;   // если используется строб 1 канала 1
        }
        else
        {
            chStrobCanal1=0;
        }
        if (this->strob_can[0].fl[1])  SWITCH_ON(chStrobCanal1,b00000010);        // если испольузеться строб 2 канала 1
        if (this->strob_can[0].fl_op)SWITCH_ON(chStrobCanal1,b00000100);       // флаг наличия опорного строба
        if (this->strob_can[0].fl_metod[0])SWITCH_ON(chStrobCanal1,b00001000);   // логика строба 1   если установлена 1 - по спаду
        if (this->strob_can[0].fl_metod[1])SWITCH_ON(chStrobCanal1,b00010000);;  // логика строба 2
        if (this->strob_can[0].fl_metod_op)SWITCH_ON(chStrobCanal1,b00100000);   //логика опорного строба
        this->infoScan[20]=(strob_can[0].fl_op_d&0x000000FF);

        this->infoScan[11]=chStrobCanal1;

        this->infoScan[12]=(int(this->strob_can[0].t2[1]*100)&0x000000FF);
        this->infoScan[13]=(int(this->strob_can[0].t2[1]*100)&0x0000FF00)>>8; // длина  строба 2 в отсчетах сдвинутое на задержку

        this->infoScan[14]=(int(this->strob_can[0].t1[1]*100)&0x000000FF);
        this->infoScan[15]=(int(this->strob_can[0].t1[1]*100)&0x0000FF00)>>8; // начало строба 2 в отсчетах сдвинутое на задержку

        this->infoScan[16]=(strob_can[0].am[1]&0x000000FF);  // амплитуда 2 строба  канал1

        this->infoScan[17]=(int(this->strob_can[0].t1_op*100)&0x000000FF);   // время начала опрного строба
        this->infoScan[18]=(int(this->strob_can[0].t1_op*100)&0x0000FF00)>>8;

        this->infoScan[19]=(int(this->strob_can[0].t2_op*100)&0x000000FF);   // длительность  опорного строба
        this->infoScan[20]=(int(this->strob_can[0].t2_op*100)&0x0000FF00)>>8;

        this->infoScan[21]=(int(this->strob_can[0].am_op)&0x000000FF);  // аплитуда опорного строба канал 1

        this->infoScan[22]=(int(this->strob_can[0].cc[0]*100)&0x000000FF); // задержка 1 строба первого канала отоносительно опорного
        this->infoScan[23]=(int(this->strob_can[0].cc[0]*100)&0x0000FF00)>>8;

        this->infoScan[25]=(this->pov_par.n_canal&0x000000FF); // кол-во каналов

        if (this->infoScan[25]==2)  // если выбран один канал не писать инф по второму
        {
            this->infoScan[26]=(this->cnl_dt[1].pt_kl&0x000000FF);
            this->infoScan[27]=(this->cnl_dt[1].pt_kl&0x0000FF00)>>8; // длина выборки второго канала

            this->infoScan[28]=(int(this->cnl_dt[1].d_zad_can*100)&0x000000FF);   //задержка канала 2
            this->infoScan[29]=(int(this->cnl_dt[1].d_zad_can*100)&0x0000FF00)>>8;

            this->infoScan[30]=(int((this->strob_can[1].t2[0])*100)&0x000000FF);  // длина строба 1 канала 2
            this->infoScan[31]=(int((this->strob_can[1].t2[0])*100)&0x0000FF00)>>8;

            this->infoScan[32]=(int((this->strob_can[1].t1[0])*100)&0x000000FF);  // начало строба в отсчетах сдвинутое на задержку
            this->infoScan[33]=(int((this->strob_can[1].t1[0])*100)&0x0000FF00)>>8;

            this->infoScan[34]=(this->strob_can[1].am[0]&0x000000FF);  // амплитуда строба 1 канал 2

            if (this->strob_can[1].fl[0])
            {
                chStrobCanal2=b00000001;   // если используется строб 1 канала 2
            }
            else
            {
                chStrobCanal2=0;
            }
            if (this->strob_can[1].fl[1])  SWITCH_ON(chStrobCanal2,b00000010);        // если испольузеться строб 2 канала 2
            if (this->strob_can[1].fl_op)SWITCH_ON(chStrobCanal2,b00000100);       // флаг наличия опрного строба
            if (this->strob_can[1].fl_metod[0])SWITCH_ON(chStrobCanal2,b00001000);   // логика строба 1   если установлена 1 - по спаду
            if (this->strob_can[1].fl_metod[1])SWITCH_ON(chStrobCanal2,b00010000);;  // логика строба 2
            if (this->strob_can[1].fl_metod_op)SWITCH_ON(chStrobCanal2,b00100000);   //логика опрного строба

            this->infoScan[36]=chStrobCanal2;

            this->infoScan[37]=(int(this->strob_can[1].t2[1]*100)&0x000000FF);
            this->infoScan[38]=(int(this->strob_can[1].t2[1]*100)&0x0000FF00)>>8; // длина  строба 2 в отсчетах сдвинутое на задержку

            this->infoScan[39]=(int(this->strob_can[1].t1[1]*100)&0x000000FF);
            this->infoScan[40]=(int(this->strob_can[1].t1[1]*100)&0x0000FF00)>>8; // начало строба 2 в отсчетах сдвинутое на задержку

            this->infoScan[41]=(strob_can[1].am[1]&0x000000FF);  // амплитуда 2 строба  канал 2

            this->infoScan[42]=(int(this->strob_can[1].t1_op*100)&0x000000FF);   // время начала опрного строба
            this->infoScan[43]=(int(this->strob_can[1].t1_op*100)&0x0000FF00)>>8;

            this->infoScan[44]=(int(this->strob_can[1].t2_op*100)&0x000000FF);   // длительность  опорного строба
            this->infoScan[45]=(int(this->strob_can[1].t2_op*100)&0x0000FF00)>>8;

            this->infoScan[46]=(int(this->strob_can[1].am_op)&0x000000FF); // амплитуда опрного строба канал 2
            this->infoScan[47]=(int(this->strob_can[1].cc[0]*100)&0x000000FF); // задержка 1 строба второго канала относителньо опорного
            this->infoScan[48]=(int(this->strob_can[1].cc[0]*100)&0x0000FF00)>>8;



            this->infoScan[82]=(this->cnl_dt[1].can_izl&0x000000FF);  //номер излучателя 2 канал
            this->infoScan[83]=(this->cnl_dt[1].can_prm&0x000000FF);

            this->infoScan[87]=(this->cnl_dt[1].i_n_per&0x000000FF); // число периодов 2 канал

            this->infoScan[88]=(int((this->cnl_dt[1].d_chast_gen)*100)&0x000000FF);  // частота генератора умножаем на 100 чтобы передать в двух байтах с дробной частью
            this->infoScan[89]=(int((this->cnl_dt[1].d_chast_gen)*100)&0x0000FF00)>>8;

            this->infoScan[90]=(int((this->cnl_dt[1].d_usil_can)*100)&0x000000FF);  // усиление канала умножаем на 100 чтобы передать в двух байтах с дробной частью
            this->infoScan[91]=(int((this->cnl_dt[1].d_usil_can)*100)&0x0000FF00)>>8;

            this->infoScan[92]=(int((this->cnl_dt[1].d_st_rob)*100)&0x000000FF);  // величина стробирования умножаем на 100 чтобы передать в двух байтах с дробной частью
            this->infoScan[93]=(int((this->cnl_dt[1].d_st_rob)*100)&0x0000FF00)>>8;
        }
        this->infoScan[70]=(this->cnl_dt[0].can_izl&0x000000FF);  //номер излучателя 1 канал
        this->infoScan[71]=(this->cnl_dt[0].can_prm&0x000000FF);

        this->infoScan[75]=(this->cnl_dt[0].i_n_per&0x000000FF); // число периодов 1 канал

        this->infoScan[76]=(int((this->cnl_dt[0].d_chast_gen)*100)&0x000000FF);  // частота генератора умножаем на 100 чтобы передать в двух байтах с дробной частью
        this->infoScan[77]=(int((this->cnl_dt[0].d_chast_gen)*100)&0x0000FF00)>>8;
        this->infoScan[78]=(int((this->cnl_dt[0].d_usil_can)*100)&0x000000FF);  // усиление канала умножаем на 100 чтобы передать в двух байтах с дробной частью
        this->infoScan[79]=(int((this->cnl_dt[0].d_usil_can)*100)&0x0000FF00)>>8;
        this->infoScan[80]=(int((this->cnl_dt[0].d_st_rob)*100)&0x000000FF);  // величина стробирования умножаем на 100 чтобы передать в двух байтах с дробной частью
        this->infoScan[81]=(int((this->cnl_dt[0].d_st_rob)*100)&0x0000FF00)>>8;
    }
}

unsigned  int fazus::get_nDef(){return this->number_def;} // возвращает long !!!! всего 2 миллиарда исправленно
unsigned char fazus::get_def(){return this->bin_def_arrayCh1[this->number_def];}
unsigned char fazus::get_def(long int li){return this->bin_def_arrayCh1[li];}
unsigned char fazus::get_amp_def(){return this->amp_def_arrayStrobCh1[this->number_def];}
unsigned char fazus::get_amp_opor(){return this->amp_oporn_arrayStrobCh1[this->number_def];}
unsigned char fazus::get_amp_opor(long int li){return this->amp_oporn_arrayStrobCh1[li];}
unsigned char fazus::get_amp_def(long int li){return this->amp_def_arrayStrobCh1[li];}

int fazus::one_shot_pin(QString nameNastr, int *pin)
{
    // функция возвращает найденный максимум в развертке и
    // через указатель его метсоположение
    int errr;
    //int n;
    unsigned char  tmpB[15000];
    int maxAmp;
    this->Rasc_Tabl();
    memset(&tmpB, 0, sizeof(tmpB));
    maxAmp=0;
    errr = Contr_Start(nameNastr.toUtf8().data());
    if (errr)
    {
        // return (255+errr);
        emit error("Ошибка настройки фазуса "+QString::number(errr) + + " " + nameNastr);
        return 255+errr;
    }
    int fh = _sopen(nameNastr.toUtf8().constData(), _O_BINARY | _O_RANDOM | _O_RDONLY, _SH_DENYNO);
    _read(fh, &cnl_dt, 8 * sizeof(CANAL_DT));
    _close(fh);

    int j = AScan_Rec((unsigned char*)tmpB, NULL, NULL, NULL, NULL, NULL, NULL, NULL,0);
    if (j)
    {
        emit error ("Ошибка получения данных " +QString::number(j));
        return (255+j);
    }

    for ( int n=0;n<(this->cnl_dt[0].pt_kl);n++)
    {
        if (maxAmp<tabl_x[(int)tmpB[n]])
        {
            maxAmp=tabl_x[(int)tmpB[n]];
            *pin = n;
        }
    }
    this->Stop_fazus(); //18.12.15
    return maxAmp;

}
int fazus::one_shot(QString nameNastr)
{
    int errr;
    unsigned char  tmpB[15000];
    int maxAmp = 0;
    this->Rasc_Tabl();
    memset(&tmpB, 0, sizeof(tmpB));
    errr = Contr_Start(nameNastr.toUtf8().data());
    if (errr)
    {
        emit error("Ошибка настройки фазуса "+QString::number(errr) + " " + nameNastr);
        return (255+errr);
    }
    int fh = _sopen(nameNastr.toUtf8().constData(), _O_BINARY | _O_RANDOM | _O_RDONLY, _SH_DENYNO);
    _read(fh, &cnl_dt, 8 * sizeof(CANAL_DT));
    //  _read(fh, &strob_can, 8 * sizeof(STROB_DT));
    _read(fh, &pov_par, sizeof(POVERHNOST));
    // _close(fh);

    int j = AScan_Rec((unsigned char*)tmpB, NULL, NULL, NULL, NULL, NULL, NULL, NULL,0);
    if (j)
    {
        emit error ("Ошибка получения данных " +QString::number(j));
        return (255 + j);
    }
    for (int n=0;n<(this->cnl_dt[0].pt_kl);n++)
        //for (int n=500;n<(15000);n++)
    {
        if (maxAmp<tabl_x[(int)tmpB[n]])
        {
            maxAmp=tabl_x[(int)tmpB[n]];
        }
    }
    this->Stop_fazus();
    return maxAmp;
}

void fazus::Rasc_Tabl()
{
    char c1;
    BYTE b1;
    int i, j, s;
    //s = this->pov_par.smech; // смещение читать из файла настроек!
    s=0;
    // 1 в if  параметр детектирвоания при контроле всегда включен
    if ( 1 ) for(i = 0; i < 256; i++) { c1 = (char)i; j = (int)c1 + s; tabl_x[i] = j < 0 ? -2 * j : 2 * j; }
    else for(i = 0; i < 256; i++) { c1 = (char)i + 128; b1 = (BYTE)c1; j = b1; tabl_x[i] = j + s; }

    //  for(i = 0; i < 256; i++) { j = tabl_x[i]; tabl_x[i] = j < 0 ? 0 : j > 255 ? 255 : j; }
    for(i = 0; i < 256; i++) { j = tabl_x[i]; tabl_x[i] = j < 1 ? 1 : j > 255 ? 255 : j; }
}
int fazus::Stop_fazus()
{
    int error = Contr_Stop();
    return error;
}
void fazus::StartDef()
{
    this->bstop = true;
}
void fazus::StopDef()
{
    this->bstop = false;
}
void fazus::SetFileNameNastr(QString nameNastr)
{
    if(nameNastr.contains(".nst"))
    {
        this->nameFileNastr = nameNastr;
        this->flagNastr = 1;
    }
    else
    {
        this->flagNastr = 0;
    }
}
