#include "windowhelp.h"

WindowHelp::WindowHelp(QWidget *parent) : QWidget(parent),
   ui(new Ui::Form2)
{

  //  ui->setupUi(this);
    //setAttribute(Qt::WA_DeleteOnClose, true);
}

WindowHelp::~WindowHelp()
{
    delete ui;
}


