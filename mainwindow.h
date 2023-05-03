#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <mutex>
#include <QActionGroup>
#include <QVector3D>
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

    void on_actionToggle_Wireframe_toggled(bool arg1);

    void on_actionDefault_View_toggled(bool arg1);

    void on_actionTexture_View_toggled(bool arg1);


    void on_actionAbou_Program_triggered();

    void on_actionUV_View_toggled(bool arg1);

    void on_actionRender_View_toggled(bool arg1);

    void on_actionAlbedo_Texture_toggled(bool arg1);

    void on_actionMetallic_Texture_toggled(bool arg1);

    void on_actionRoughness_Texture_toggled(bool arg1);

    void on_actionClear_All_Textures_triggered();

    void on_actionSet_Emissive_Color_triggered();

    void on_actionAmbient_Occlusion_Texture_toggled(bool arg1);

    void on_actionEmissive_Texture_toggled(bool arg1);

    void on_actionDisplacement_Texture_toggled(bool arg1);

    void on_actionAlbedo_triggered();

    void on_actionMetallic_triggered();

    void on_actionRoughness_triggered();

    void on_actionAmbient_Occlusion_triggered();

    void on_actionEmissive_triggered();

    void on_actionDisplacement_triggered();

    void on_actionSave_all_Texture_triggered();

    void on_actionAlbedo_Save_triggered();

    void on_actionMetallic_Save_triggered();

    void on_actionRoughness_Save_triggered();

    void on_actionAmbient_Occlusion_Save_triggered();

    void on_actionEmissive_Save_triggered();

    void on_actionDisplacement_Save_triggered();

private:
    Ui::MainWindow *ui;
    std::mutex loadingMeshMutex;
    bool loadingMesh;
    QActionGroup *viewModeGroup;
    QActionGroup *textureModeGroup;
    QVector3D prevcolor;
};
#endif // MAINWINDOW_H
