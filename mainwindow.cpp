#include "mainwindow.h"
#include "ui_mainwindow.h"
#include<QMessageBox>
#include<QFileDialog>
#include "sdlrenderwnd.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_action_triggered()
{
    SDLRenderWnd* sdl_wnd = new SDLRenderWnd(NULL);
    QFileDialog* dialog = new QFileDialog(this, QStringLiteral("选择文件"), QStringLiteral("D:\\movie"), tr("File(*.mp4)"));
    dialog->setFileMode(QFileDialog::ExistingFile);
    dialog->setViewMode(QFileDialog::Detail);
    QStringList filenames;
    if(dialog->exec())
    {
        filenames = dialog->selectedFiles();
        if(filenames.size()>0)
        {
            //sdl_wnd->show();
            sdl_wnd->PlayVideo(filenames.front());
        }
        else {
            QMessageBox::information(this, "not select file", filenames.front(),  QMessageBox::Ok);
        }

    }
    else {
        QMessageBox::information(this, "select file error", filenames.front(),  QMessageBox::Ok);
    }

}
