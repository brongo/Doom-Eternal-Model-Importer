#include "mainwindow.h"
#include "./ui_mainwindow.h"

// Public Functions
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    // setup & connect
    ui->setupUi(this);
    connect(&_ResourceForm,SIGNAL(SendResourceInfo(QString, QString)),this,SLOT(receiveResourceInfo(QString, QString)));

    // page 1 ui
    ui->lineOBJFile->setReadOnly(true);
    ui->lineLWOFile->setReadOnly(true);
    ui->btnNext->setEnabled(false);

    // page 2 ui
    ui->materialList->setSortingEnabled(false);
    ui->btnConvert->setEnabled(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::ThrowError(std::string errorMessage, std::string errorDetail)
{
    const QString qErrorMessage = QString::fromStdString(errorMessage);
    const QString qErrorDetail = QString::fromStdString(errorDetail);

    QMessageBox msgBox;
    msgBox.setWindowTitle("Doom Eternal Model Importer");
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText(qErrorMessage);
    msgBox.setInformativeText(qErrorDetail);
    msgBox.exec();
    return;
}

void MainWindow::ThrowFatalError(std::string errorMessage, std::string errorDetail)
{
    const QString qErrorMessage = QString::fromStdString(errorMessage);
    const QString qErrorDetail = QString::fromStdString(errorDetail);

    QMessageBox msgBox;
    msgBox.setWindowTitle("Doom Eternal Model Importer");
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(qErrorMessage);
    msgBox.setInformativeText(qErrorDetail);
    msgBox.exec();
    delete ui;
    return;
}

void MainWindow::ShowInfoBox(std::string infoMessage, std::string infoDetail)
{
    const QString qInfoMessage = QString::fromStdString(infoMessage);
    const QString qInfoDetail = QString::fromStdString(infoDetail);

    QMessageBox msgBox;
    msgBox.setWindowTitle("Doom Eternal Model Importer");
    msgBox.setText(qInfoMessage);
    msgBox.setInformativeText(qInfoDetail);
    msgBox.exec();
    return;
}

// Private Functions
void MainWindow::DisableGUI()
{
    ui->btnSelectLWO->setEnabled(false);
    return;
}

void MainWindow::EnableGUI()
{
    ui->btnSelectLWO->setEnabled(true);
    return;
}

// Private Slots
void MainWindow::on_btnBack_clicked()
{
    ui->stackedWidget->setCurrentIndex(0);
    return;
}

void MainWindow::on_btnConvert_clicked()
{
    HAYDEN::ModelConverter Converter;
    bool success = Converter.ConvertOBJtoLWO(_GamePath, _OBJFilePath, _LWOFileName.toStdString(), _ResourceFilePath, _Material2Decl.toStdString(), _UseYOrientation);

    if (!success)
    {
        if (Converter.VertexCount > 65535)
        {
            std::string vertCountStr = std::to_string(Converter.VertexCount);
            ThrowError("ERROR: Import failed.", "The imported model requires " + vertCountStr + " vertices. The maximum allowed is 65535.");
        }
        else
        {
            ThrowError("An unknown error has occured.");
        }
    }
    else
    {
        ShowInfoBox("Model converted successfully.");
    }
    return;
}

void MainWindow::on_btnLoadOBJ_clicked()
{
    const QString filePath = QFileDialog::getOpenFileName(this, "", "", tr("OBJ Files (*.obj)"));

    if (filePath.isEmpty())
        return;

    // Make sure this is an OBJ file
    fs::path fileName = fs::path(filePath.toStdString()).filename();
    std::string fileExtension = fileName.extension().string();

    if (fileExtension != ".obj")
    {
        ThrowError("Please select a valid .obj file.");
        return;
    }

    // Get mesh info
    HAYDEN::ModelConverter Converter;
    std::vector<std::string> meshInfo = Converter.GetOBJMeshInfo(filePath.toStdString());
    QString meshCount = QString::number(meshInfo.size());

    // Show error for bad format detected
    if (meshInfo.size() == 0)
    {
        ThrowError("This .obj file uses an unrecognized format.", "Try importing this file into Blender, then exporting it again as .obj format.");
        _OBJFilePath = "";
        QString labelText = "No file loaded";
        ui->lineOBJFile->setText("");
        ui->labelStatusOBJ->setText(labelText);
        ui->btnNext->setEnabled(false);
        return;
    }

    // We can only support importing a single mesh for now
    if (meshInfo.size() > 1)
    {
        ThrowError(".obj files containing multiple objects/meshes are not yet supported. This .obj file contains " + meshCount.toStdString() + " meshes.");
        _OBJFilePath = "";
        QString labelText = "No file loaded";
        ui->lineOBJFile->setText("");
        ui->labelStatusOBJ->setText(labelText);
        ui->btnNext->setEnabled(false);
        return;
    }

    // Store OBJ to our mainwindow object
    _OBJFilePath = filePath.toStdString();

    // Update form info
    QString labelText = "Loaded.";
    ui->lineOBJFile->setText(fileName.string().c_str());
    ui->labelStatusOBJ->setText(labelText);

    // Enable the "next" button if a LWO file has also been selected.
    if (!ui->lineLWOFile->text().isEmpty())
    {
        ui->btnNext->setEnabled(true);
    }

    return;
}

void MainWindow::on_btnNext_clicked()
{
    ui->stackedWidget->setCurrentIndex(1);
    return;
}

void MainWindow::on_btnSelectLWO_clicked()
{
    _ResourceForm.show();
    return;
}

void MainWindow::on_materialList_itemSelectionChanged()
{
    QString materialName;
    QTableWidgetItem* selectedMaterial;
    QList<QTableWidgetItem*> selectedMaterialList;

    selectedMaterialList = ui->materialList->selectedItems();

    if (selectedMaterialList.size() > 0)
    {
        selectedMaterial = selectedMaterialList[0];
        _Material2Decl = selectedMaterial->text();

        materialName = "Selected material: \"" + _Material2Decl + "\"";
        ui->btnConvert->setEnabled(true);
    }
    else
    {
        _Material2Decl = "";
        materialName = "No material selected.";
        ui->btnConvert->setEnabled(false);
    }

    ui->labelStatusMaterial->setText(materialName);
    return;
}

void MainWindow::on_radioOrientY_toggled(bool checked)
{
    if (checked)
        _UseYOrientation = 1;
    return;
}

void MainWindow::on_radioOrientZ_toggled(bool checked)
{
    if (checked)
        _UseYOrientation = 0;
    return;
}

void MainWindow::receiveResourceInfo(const QString& lwoFile, const QString& resourceFile) {

    // Get resource info from other window
    _ResourceForm.hide();
    _ResourceFilePath = resourceFile.toStdString();
    _LWOFileName = lwoFile;

    // Get game path
    auto baseIndex = _ResourceFilePath.string().find("base");
    _GamePath = _ResourceFilePath.string().substr(0, baseIndex - 1);

    // Get mesh info
    HAYDEN::ModelConverter Converter;
    std::vector<std::string> meshInfo = Converter.GetLWOMeshInfo(lwoFile.toStdString(), _ResourceFilePath);

    // Check for unsupported types
    if ((meshInfo.size() == 0 || meshInfo.size() > 256))
    {
        ThrowError("ERROR: this .LWO file is an unknown type. Not yet supported.");
        return;
    }

    // Update form info
    QString meshCount = QString::number(meshInfo.size());
    QString labelText = "Loaded. Meshes: " + meshCount;
    ui->lineLWOFile->setText(lwoFile.toStdString().c_str());
    ui->labelStatusLWO->setText(labelText);

    // Clear materials list (shown on next page)
    ui->materialList->clearContents();
    ui->materialList->setRowCount(0);

    // Populate materials list (shown on next page)
    for (int i = 0; i < meshInfo.size(); i++)
    {
        // Add row
        int row_count = ui->materialList->rowCount();
        ui->materialList->insertRow(row_count);

        // Set material name
        QString materialName = QString::fromStdString(meshInfo[i]);
        QTableWidgetItem* tableMaterialName = new QTableWidgetItem(materialName);

        // Populate Table Row
        ui->materialList->setItem(row_count, 0, tableMaterialName);
    }

    // Change material2 selection message depending on number of meshes
    if (meshInfo.size() == 1)
    {
        ui->labelSelectMaterial->setText("Confirm the material2 .decl that will be used:");
    }
    else if (meshInfo.size() == 2)
    {
        ui->labelSelectMaterial->setText("Select a material2 .decl for this model (the other will be discarded):");
    }
    else
    {
        ui->labelSelectMaterial->setText("Select a material2 .decl for this model (others will be discarded):");
    }

    // Enable the "next" button if an obj file has also been selected.
    if (!ui->lineOBJFile->text().isEmpty())
    {
        ui->btnNext->setEnabled(true);
    }

    return;
}
