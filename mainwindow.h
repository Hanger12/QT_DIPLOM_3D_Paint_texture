#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <mutex>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_actionImport_models_triggered();

    void on_actionCenter_Camera_triggered();

    void on_actionClear_Active_Texture_triggered();

    void on_actionSet_Paint_triggered();

    void on_actionSet_Stroke_Width_triggered();

private:
    Ui::MainWindow *ui;
    std::mutex loadingMeshMutex;
    bool loadingMesh;
};
#endif // MAINWINDOW_H
