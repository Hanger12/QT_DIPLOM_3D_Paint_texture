#include "aboutprogramwidjet.h"
#include "ui_aboutprogramwidjet.h"

AboutProgramWidjet::AboutProgramWidjet(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutProgramWidjet)
{
    ui->setupUi(this);
}

AboutProgramWidjet::~AboutProgramWidjet()
{
    delete ui;
}
