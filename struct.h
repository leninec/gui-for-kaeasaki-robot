#ifndef STRUCT
#define STRUCT
#include <QString>

struct stCoord
{
    int                 marker;						// Что содержит данная координата
    float               x,							// Координата х
    y,							// Координата y
    z;							// Координата z
    unsigned int        nPoint;						// Узел траектории (номер линии)
    unsigned  int	nDef;						// Номер прозвучивания

    // раньше был unsigned long int
};
enum ButtonName
{
    bnastrUZK,
    bviev,
    bgetTraect,
    bgetUzk,
    bcalibrovka,
    bresume,
    bhereshift,
    bstart,
    bstop,
    bsliv,
    bpump,
    borient,
    bpark

};


enum DirectionManualMove
{
    moveX,
    moveY,
    moveZ,
    moveJ1,
    moveJ2,
    moveJ3,
    moveJ4,
    moveJ5,
    moveJ6
};
enum comand
{
    sliv,
    pump,
    stop,
    stopE,
    resetError,
    continueWork,
    startMove,
    deletePoint,
    downloadPoint,
    clearStep,
    here,
    uploadPoint,
    shiftPoint,
    setSpeed,
    setMechScan,
    hereShift,
    uploadShift,
    getRS10parametr,
    manualMoveXYZ,
    manualMoveJ1J6,
    getWorkSpace,
    setWorkSpace,
    setFragment,
    openPipe,
    openFileT,
    deleteFileDef,
    orientation,
    calibration,
    saveC,
    testHere,
    goHome,
    goFirstPoint,
    init,    // очищаем вектора с данными скидываем соединение по пайпу и флаги
    getDigitalInput,
    testPipe,
    setCircle,
    move2point,
    move2degree,
    savePoint,
    changeMoveMode


};

struct rs10nComand
{
    int instruction; // а сюда пихать enum с метками
    float parametr1;
    float parametr2;
    float parametr3;
    float parametr4;
    float parametr5;
    float parametr6;
    QString name7;
    QByteArray array8;
};

#endif // STRUCT

