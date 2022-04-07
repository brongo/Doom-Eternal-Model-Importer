#ifndef RESOURCEFORM_H
#define RESOURCEFORM_H

#include <QMainWindow>
#include <QFileDialog>
#include <QMessageBox>
#include <QThread>
#include <QTableWidgetItem>

#include "../core/ModelConverter.h"

#include "ui_resourceform.h"

class ResourceForm : public QWidget
{
    Q_OBJECT

    public:
        QString LWOFileName;
        explicit ResourceForm(QWidget* parent = nullptr);
        void ThrowError(std::string errorMessage, std::string errorDetail = "");
        void ThrowFatalError(std::string errorMessage, std::string errorDetail = "");
        void EmitResourceInfo(QString lwoFile, QString resourceFile);

    private:
        Ui::ResourceForm ui; 
        HAYDEN::ModelConverter ModelConverter;
        QMessageBox _LoadStatusBox;
        QThread* _LoadResourceThread = NULL;
        std::string _ResourcePath;
        bool _ResourceFileIsLoaded = 0;
        bool _ViewIsFiltered = 0;
        
        int  ShowLoadStatus();
        void DisableGUI();
        void EnableGUI();
        void ResetGUITable();
        void PopulateGUIResourceTable(std::vector<std::string> searchWords = std::vector<std::string>());
        std::vector<std::string> SplitSearchTerms(std::string inputString);

    private slots:
        void on_btnClear_clicked();
        void on_btnLoadResource_clicked();
        void on_btnReplaceLWO_clicked();
        void on_btnSearch_clicked();
        void on_inputSearch_returnPressed();
        void on_tableWidget_itemDoubleClicked(QTableWidgetItem* item);

    signals:
        void SendResourceInfo(const QString& lwoFile, const QString& resourceFile);
};

#endif
