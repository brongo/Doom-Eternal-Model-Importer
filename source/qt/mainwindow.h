#pragma once
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QThread>
#include <QTableWidgetItem>

#include "../core/ModelConverter.h"

#include "resourceform.h"

namespace fs = std::filesystem;

QT_BEGIN_NAMESPACE
namespace Ui
{
    class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

    public:
        void ThrowError(std::string errorMessage, std::string errorDetail = "");
        void ThrowFatalError(std::string errorMessage, std::string errorDetail = "");
        void ShowInfoBox(std::string infoMessage, std::string infoDetail = "");
        MainWindow(QWidget *parent = nullptr);
        ~MainWindow();

    private:
        Ui::MainWindow* ui;
        ResourceForm _ResourceForm;
        fs::path _GamePath;
        fs::path _OBJFilePath;
        fs::path _ResourceFilePath;
        QString _LWOFileName;
        QString _Material2Decl;
        bool _UseYOrientation = 1;

        void DisableGUI();
        void EnableGUI();

    private slots:
        void on_btnBack_clicked();
        void on_btnConvert_clicked(); 
        void on_btnLoadOBJ_clicked(); 
        void on_btnNext_clicked(); 
        void on_btnSelectLWO_clicked();
        void on_materialList_itemSelectionChanged(); 
        void on_radioOrientY_toggled(bool checked);
        void on_radioOrientZ_toggled(bool checked);
        void receiveResourceInfo(const QString& lwoFile, const QString& resourceFile);
};

#endif // MAINWINDOW_H
