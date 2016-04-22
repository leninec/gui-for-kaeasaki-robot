#ifndef FAZUS_H
#define FAZUS_H

#include <QThread>
#include <QObject>
#include <io.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <share.h>
#include <QDate>
#include "bin.h"
#include "FAZUS_DLL.h"
#include "sleep.h"
#include <QFile>



typedef int                 BOOL;

class fazus : public QObject
{
    Q_OBJECT
public:
    char infoScan[128];
    QString  nameFileDef;
    QString tempName;
    int flagNastr;

    explicit fazus(QObject *parent = 0);
    void SetFileNameNastr(QString);
    void StopDef();
    bool Get_bStatDef(){return this->bstop;}
    void StartDef();
    void Nastr(QString fileName);
    QByteArray amp_def_array;
    volatile BOOL ascan_write;
    volatile BOOL bin_def;
    unsigned  int get_nDef();   // исправлено с long 20.02.16
    unsigned char get_def();
    unsigned char get_def(long int li);
    unsigned char get_amp_def();
    unsigned char get_amp_opor();
    unsigned char get_amp_opor(long int li);
    unsigned char get_amp_def(long int li);
    int one_shot_pin(QString nameNastr, int *pin);
    int one_shot(QString nameNastr);
    int Stop_fazus ();
    void Rasc_Tabl();
    int tabl_x[256];
    struct CANAL_DT
    { int can_izl;   //номер излучателя в текущем канале. на 1 меньше так начинаеться с 0
      int can_prm;    //номер приемника в текущем канале. на 1 меньше так начинаеться с 0
      int i_coeff_us;
      int i_dl_vyb, n_pack, pt_kl;  //
                                    //
                                    // пт кл развертка в 7 мкс на деление = 7000 0,2 = 200
      int i_n_per;  // число периодов в изл импульсе
      int metod;
      double d_chast_gen;  // частота в МГц
      double d_usil_can;   // усиление в дб
      double d_zad_can;   // задержка в мкс
      double d_st_rob;    // стробирование (общее)в мкс
    } cnl_dt[8];

 #pragma pack(push, 1)
    struct STROB_DT
    { BOOL fl[5], fl_op, // флаг - задействован ли строб
        fl_op_d;
      int am[5], am_op, fl_metod[5], fl_metod_op;   // флаг метода - тип используемой логики 0 превышение 1 спад
      double t1[5], // начало строба в мкс
      cc[5],  // задержка относительно опорного строба
      t2[5], //  длительность строба мкс
      t1_op, // время начала опорного строба в мкс
      cc_op,
      t2_op,    // длительность опрного строба
      t1_op_d;
    } strob_can[8];
    //функция получить а скан возвращает pt_kl отсчетов
    // автоматически начиная с задержки канала. Для поиска максимума в стробе
    // необоходимо вычесть время начала строба из общей задержки

    struct POVERHNOST
    { int x_dlt, y_dlt, x_ob, y_ob, x_ob_mm, y_ob_mm;
      int scale, n_sn_liniy, n_zv_all, n_abs;
      int skor_zv, tolsh, fl_nepr_scan, y_mm_zad;
      char nm[20], brak;
      time_t tm_s;
      int smech;
      char fl_vrch[8];
      int n_canal;    // число каналов ( если 1 то один!)
      char nstr[40];
      int x_mm, y_mm;
    } pov_par;
 #pragma pack(pop)

signals:
    void finished();
    void scan();
    void error(QString err);

public slots:
    int Process();
    void OneShot();

private:
     QString nameFileNastr;
     bool bstop;
     int tmp;
     int tmp_time;
     int t1c1s1;
     int t1c2s1;
     QDateTime data;
     unsigned char  b1[15000];
     unsigned char  b2[15000];
     unsigned char max_ampCh1S1;
     unsigned char max_ampCh2S1;
     int fh1;
     unsigned int number_def;
     QByteArray bin_def_arrayCh1;
     QByteArray amp_def_arrayStrobCh1;
     QByteArray amp_def_arrayCh1;
     QByteArray amp_oporn_arrayStrobCh1;
     QByteArray qbWrite;

     QByteArray bin_def_arrayCh2;
     QByteArray amp_def_arrayStrobCh2;
     QByteArray amp_def_arrayCh2;
     QByteArray amp_oporn_arrayStrobCh2;
     QFile *bfile;

};

#endif // FAZUS_H
