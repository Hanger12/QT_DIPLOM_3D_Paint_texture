#ifndef ABOUTPROGRAMWIDJET_H
#define ABOUTPROGRAMWIDJET_H

#include <QWidget>
#include <QDialog>
namespace Ui {
class AboutProgramWidjet;
}

class AboutProgramWidjet : public QDialog
{
    Q_OBJECT

public:
    explicit AboutProgramWidjet(QWidget *parent = nullptr);
    ~AboutProgramWidjet();

private:
    Ui::AboutProgramWidjet *ui;
};

#endif // ABOUTPROGRAMWIDJET_H
