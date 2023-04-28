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
//    textureModeGroup = new QActionGroup(this);
//    textureModeGroup->addAction(actionActivateAlbedoTexture);
//    textureModeGroup->addAction(actionActivateMetallicTexture);
//    textureModeGroup->addAction(actionActivateRoughnessTexture);
//    textureModeGroup->addAction(actionActivateAmbientOcclusionTexture);
//    textureModeGroup->addAction(actionActivateEmissiveTexture);
//    textureModeGroup->addAction(actionActivateDisplacementTexture);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_actionImport_models_triggered()
{
    // get file path
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Mesh File"), "", tr("Wavefront OBJ File (*.obj)"));

    // get lock on bool to check if there is already a loading operation going on
    std::lock_guard<std::mutex> lockCheck(loadingMeshMutex);

    // only continue if a file was selected and we are not already loading a mesh
    if (!fileName.isEmpty() && !loadingMesh)
    {
        // set flag that a mesh loading operation is going on
        loadingMesh = true;
        MainWindow *meshTool = this;

        QMessageBox::information(this, "Loading Model", "The model is being loaded in the background. This may take a while.");

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
                    QMessageBox::critical(meshTool, "Model Loading Failed!", "Loading the Model has failed.");

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
    msgBox.setWindowTitle("Clear Active Texture");
    msgBox.setText("Are you sure you want to clear the active texture?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Yes);

    if (msgBox.exec() == QMessageBox::Yes)
    {
        QColorDialog colorDialog(this);
        colorDialog.setWindowTitle("Set Clear Color");
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
    QVector3D prevColor = ui->openGLWidget->getPaintColor() * 255.0f; // QColor uses 0 - 255
    QColorDialog colorDialog({ int(prevColor.x()), int(prevColor.y()), int(prevColor.z() )}, this);
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
    int result = QInputDialog::getInt(this, tr("Set Stroke Width"), tr("Set Stroke Width"), ui->openGLWidget->getStrokeWidth(), 1, 256);
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

