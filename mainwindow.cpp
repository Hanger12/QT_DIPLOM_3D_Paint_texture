#include "mainwindow.h"
#include "aboutprogramwidjet.h"
#include "ui_mainwindow.h"
#include <QVector3D>
#include<QFileDialog>
#include <iostream>
#include <QMessageBox>
#include <thread>
#include "objloader.h"
#include <qtimer.h>
#include <QColorDialog>
#include "sessionsettings.h"
#include <QInputDialog>
#include <QString>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),loadingMesh(false)
{
    ui->setupUi(this);
    viewModeGroup = new QActionGroup(this);
    viewModeGroup->addAction(ui->actionDefault_View);
    viewModeGroup->addAction(ui->actionTexture_View);
    viewModeGroup->addAction(ui->actionUV_View);
    viewModeGroup->addAction(ui->actionRender_View);
    textureModeGroup = new QActionGroup(this);
    textureModeGroup->addAction(ui->actionAlbedo_Texture);
    textureModeGroup->addAction(ui->actionMetallic_Texture);
    textureModeGroup->addAction(ui->actionRoughness_Texture);
    textureModeGroup->addAction(ui->actionAmbient_Occlusion_Texture);
    textureModeGroup->addAction(ui->actionEmissive_Texture);
    textureModeGroup->addAction(ui->actionDisplacement_Texture);

}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_actionImport_models_triggered()
{
    // get file path
    QString fileName = QFileDialog::getOpenFileName(this, tr("Открыть файл модели"), "", tr("Wavefront OBJ File (*.obj)"));

    // get lock on bool to check if there is already a loading operation going on
    std::lock_guard<std::mutex> lockCheck(loadingMeshMutex);

    // only continue if a file was selected and we are not already loading a mesh
    if (!fileName.isEmpty() && !loadingMesh)
    {
        // set flag that a mesh loading operation is going on
        loadingMesh = true;
        MainWindow *meshTool = this;

        QMessageBox::information(this, "Загрузка модели", "Модель загружается в фоновом режиме. Это может занять некоторое время.");

        // start thread to load mesh
        std::thread meshLoadThread([fileName, meshTool]()
        {
            // in order to use QTimer::singleShot, we need a QEventLoop
            QEventLoop loop;
            Q_UNUSED(loop)

            OBJLoader::Error loadError;
            // allocate on heap to avoid unnecessary copy when posting to gui thread
            IndexedMesh *indexedMesh =  new IndexedMesh(OBJLoader::loadOBJ(fileName.toLatin1().data(), loadError));

            // execture on gui thread
            QTimer::singleShot(0, qApp, [indexedMesh, meshTool, loadError]()
            {
                if (loadError != OBJLoader::Error::SUCCESS)
                {
                    QMessageBox::critical(meshTool, "Ошибка загрузки модели", "Не удалось загрузить модель.");

                    // acquire lock to signal that mesh loading is done
                    std::lock_guard<std::mutex> lockReset(meshTool->loadingMeshMutex);
                    meshTool->loadingMesh = false;
                    return;
                }

                // create GLMesh
                meshTool->ui->openGLWidget->setMesh(*indexedMesh);
                // the GLMesh created in GLWidget holds a copy of indexedMesh, so delete this instance
                delete indexedMesh;

                // reset material textures
                meshTool->ui->openGLWidget->update();

                // acquire lock to signal that mesh loading is done
                std::lock_guard<std::mutex> lockReset(meshTool->loadingMeshMutex);
                meshTool->loadingMesh = false;
            });
        });
        meshLoadThread.detach();
    }
}


void MainWindow::on_actionCenter_Camera_triggered()
{
    ui->openGLWidget->centercamera(QVector3D(0.0,0.0,-6.0));
}


void MainWindow::on_actionClear_Active_Texture_triggered()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("Очистить Активную Текстуру");
    msgBox.setText("Вы уверены, что хотите очистить активную текстуру?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Yes);

    if (msgBox.exec() == QMessageBox::Yes)
    {
        QColorDialog colorDialog(this);
        colorDialog.setWindowTitle("Задать новый цвет");
        if (colorDialog.exec())
        {
            float r, g, b;
            colorDialog.currentColor().getRgbF(&r, &g, &b);
            ui->openGLWidget->clearActiveTexture({ r, g, b });
            ui->openGLWidget->update();
        }
    }
}


void MainWindow::on_actionSet_Paint_triggered()
{
    QVector3D prevColor1 = ui->openGLWidget->getPaintColor() * 255.0f; // QColor uses 0 - 255
    QColorDialog colorDialog({ int(prevColor1.x()), int(prevColor1.y()), int(prevColor1.z() )}, this);
    if (colorDialog.exec())
    {
        QColor qcolor = colorDialog.currentColor();
        float r, g, b;
        qcolor.getRgbF(&r, &g, &b);
        settings()->setBrushColor(qcolor);
        ui->openGLWidget->update();
    }
}


void MainWindow::on_actionSet_Stroke_Width_triggered()
{
    int result = QInputDialog::getInt(this, tr("Задать размер кисти"), tr("Задать размер кисти"), ui->openGLWidget->getStrokeWidth(), 1, 256);
    settings()->setBrushSize(result);
    ui->openGLWidget->update();
}


void MainWindow::on_actionToggle_Wireframe_toggled(bool arg1)
{
    ui->openGLWidget->toggleWireframe(arg1);
    ui->openGLWidget->update();
}


void MainWindow::on_actionDefault_View_toggled(bool arg1)
{
    if (arg1)
    {
        ui->openGLWidget->setViewMode(ViewMode::DEFAULT);
        ui->openGLWidget->update();
    }
}


void MainWindow::on_actionTexture_View_toggled(bool arg1)
{
    if (arg1)
    {
        ui->openGLWidget->setViewMode(ViewMode::TEXTURE);
        ui->openGLWidget->update();
    }
}



void MainWindow::on_actionAbou_Program_triggered()
{
    AboutProgramWidjet* widjet=new AboutProgramWidjet(this);
    widjet->show();
}


void MainWindow::on_actionUV_View_toggled(bool arg1)
{
    if (arg1)
    {
        ui->openGLWidget->setViewMode(ViewMode::UV);
        ui->openGLWidget->update();
    }
}


void MainWindow::on_actionRender_View_toggled(bool arg1)
{
    if (arg1)
    {
        ui->openGLWidget->setViewMode(ViewMode::RENDER);
        ui->openGLWidget->update();
    }
}


void MainWindow::on_actionAlbedo_Texture_toggled(bool arg1)
{
    if (arg1)
    {
        ui->openGLWidget->setTextureMode(TextureMode::ALBEDO);
        ui->toolButton->setEnabled(true);
        ui->actionSet_Paint->setEnabled(true);
        ui->actionSet_Emissive_Color->setEnabled(false);
        ui->openGLWidget->update();
    }
}


void MainWindow::on_actionMetallic_Texture_toggled(bool arg1)
{
    if (arg1)
    {
        ui->openGLWidget->setTextureMode(TextureMode::METALLIC);
        ui->toolButton->setEnabled(false);
        ui->actionSet_Paint->setEnabled(false);
        ui->actionSet_Emissive_Color->setEnabled(false);
        ui->openGLWidget->update();
    }
}


void MainWindow::on_actionRoughness_Texture_toggled(bool arg1)
{
    if (arg1)
    {
        ui->openGLWidget->setTextureMode(TextureMode::ROUGHNESS);
        ui->toolButton->setEnabled(false);
        ui->actionSet_Paint->setEnabled(false);
        ui->actionSet_Emissive_Color->setEnabled(false);
        ui->openGLWidget->update();
    }
}


void MainWindow::on_actionClear_All_Textures_triggered()
{
    QMessageBox msgBox;
    msgBox.setWindowTitle("Очистить все Текстуры");
    msgBox.setText("Вы уверены, что хотите очистить все текстуры и сбросить их до значений по умолчанию?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Yes);

    if (msgBox.exec() == QMessageBox::Yes)
    {
        ui->openGLWidget->clearAllTextures();
        ui->openGLWidget->update();
    }
}


void MainWindow::on_actionSet_Emissive_Color_triggered()
{
    QVector3D prevColor = ui->openGLWidget->getMaterial()->getEmissive() * 255.0f;
    QColorDialog colorDialog({ int(prevColor.x()), int(prevColor.y()), int(prevColor.z()) }, this);
    if (colorDialog.exec())
    {
        QColor qcolor = colorDialog.currentColor();
        float r, g, b;
        qcolor.getRgbF(&r, &g, &b);
        ui->openGLWidget->getMaterial()->setEmissive({ r, g, b });
        ui->openGLWidget->update();
    }
}


void MainWindow::on_actionAmbient_Occlusion_Texture_toggled(bool arg1)
{
    if (arg1)
    {
        ui->openGLWidget->setTextureMode(TextureMode::AMBIENT_OCCLUSION);
        ui->toolButton->setEnabled(false);
        ui->actionSet_Paint->setEnabled(false);
        ui->actionSet_Emissive_Color->setEnabled(false);
        ui->openGLWidget->update();
    }
}


void MainWindow::on_actionEmissive_Texture_toggled(bool arg1)
{
    if (arg1)
    {
        ui->openGLWidget->setTextureMode(TextureMode::EMISSIVE);
        ui->toolButton->setEnabled(true);
        ui->actionSet_Paint->setEnabled(true);
        ui->actionSet_Emissive_Color->setEnabled(true);
        ui->openGLWidget->update();
    }

}


void MainWindow::on_actionDisplacement_Texture_toggled(bool arg1)
{
    if (arg1)
    {
        ui->openGLWidget->setTextureMode(TextureMode::DISPLACEMENT);
        ui->toolButton->setEnabled(false);
        ui->actionSet_Paint->setEnabled(false);
        ui->actionSet_Emissive_Color->setEnabled(false);
        ui->openGLWidget->update();
    }
}


void MainWindow::on_actionAlbedo_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Открыть Albedo Текстуру"), "", tr("PNG (*.png);;DirectDrawSurface (*.dds)"));
    if (!fileName.isEmpty())
    {
        ui->openGLWidget->setTexture(fileName.toLatin1().data(), TextureMode::ALBEDO);
        ui->openGLWidget->update();
    }
}



void MainWindow::on_actionMetallic_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Открыть Metallic Текстуру"), "", tr("PNG (*.png);;DirectDrawSurface (*.dds)"));
    if (!fileName.isEmpty())
    {
        ui->openGLWidget->setTexture(fileName.toLatin1().data(), TextureMode::METALLIC);
        ui->openGLWidget->update();
    }
}


void MainWindow::on_actionRoughness_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Открыть Roughness Текстуру"), "", tr("PNG (*.png);;DirectDrawSurface (*.dds)"));
    if (!fileName.isEmpty())
    {
        ui->openGLWidget->setTexture(fileName.toLatin1().data(), TextureMode::ROUGHNESS);
        ui->openGLWidget->update();
    }
}


void MainWindow::on_actionAmbient_Occlusion_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Открыть Ambient Occlusion Текстуру"), "", tr("PNG (*.png);;DirectDrawSurface (*.dds)"));
    if (!fileName.isEmpty())
    {
        ui->openGLWidget->setTexture(fileName.toLatin1().data(), TextureMode::AMBIENT_OCCLUSION);
        ui->openGLWidget->update();
    }
}


void MainWindow::on_actionEmissive_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Открыть Emissive Текстуру"), "", tr("PNG (*.png);;DirectDrawSurface (*.dds)"));
    if (!fileName.isEmpty())
    {
        ui->openGLWidget->setTexture(fileName.toLatin1().data(), TextureMode::EMISSIVE);
        ui->openGLWidget->update();
    }
}


void MainWindow::on_actionDisplacement_triggered()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Открыть Displacement Текстуру"), "", tr("PNG (*.png);;DirectDrawSurface (*.dds)"));
    if (!fileName.isEmpty())
    {
        ui->openGLWidget->setTexture(fileName.toLatin1().data(), TextureMode::DISPLACEMENT);
        ui->openGLWidget->update();
    }
}


void MainWindow::on_actionSave_all_Texture_triggered()
{
    QString directoryName = QFileDialog::getExistingDirectory(this, tr("Сохранить все Текстуры"), "", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!directoryName.isEmpty())
    {
        ui->openGLWidget->saveAllTextures(directoryName.toLatin1().data());
    }
}


void MainWindow::on_actionAlbedo_Save_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Сохранить Albedo Текстуру"), "", tr("PNG (*.png)"));
    if (!fileName.isEmpty())
    {
        ui->openGLWidget->saveTexture(fileName.toLatin1().data(), TextureMode::ALBEDO);
    }
}


void MainWindow::on_actionMetallic_Save_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Сохранить Metallic Текстуру"), "", tr("PNG (*.png)"));
    if (!fileName.isEmpty())
    {
        ui->openGLWidget->saveTexture(fileName.toLatin1().data(), TextureMode::METALLIC);
    }
}


void MainWindow::on_actionRoughness_Save_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Сохранить Roughness Текстуру"), "", tr("PNG (*.png)"));
    if (!fileName.isEmpty())
    {
        ui->openGLWidget->saveTexture(fileName.toLatin1().data(), TextureMode::ROUGHNESS);
    }
}


void MainWindow::on_actionAmbient_Occlusion_Save_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Сохранить Ambient Occlusion Текстуру"), "", tr("PNG (*.png)"));
    if (!fileName.isEmpty())
    {
        ui->openGLWidget->saveTexture(fileName.toLatin1().data(), TextureMode::AMBIENT_OCCLUSION);
    }
}


void MainWindow::on_actionEmissive_Save_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Сохранить Emissive Текстуру"), "", tr("PNG (*.png)"));
    if (!fileName.isEmpty())
    {
        ui->openGLWidget->saveTexture(fileName.toLatin1().data(), TextureMode::EMISSIVE);
    }
}


void MainWindow::on_actionDisplacement_Save_triggered()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Сохранить Displacement Текстуру"), "", tr("PNG (*.png)"));
    if (!fileName.isEmpty())
    {
        ui->openGLWidget->saveTexture(fileName.toLatin1().data(), TextureMode::DISPLACEMENT);
    }
}

