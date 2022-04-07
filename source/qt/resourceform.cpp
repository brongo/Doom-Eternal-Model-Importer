#include "resourceform.h"

// Public Functions
ResourceForm::ResourceForm(QWidget *parent)
    : QWidget(parent)
{
    ui.setupUi(this);
    ui.btnClear->setVisible(false);
    ui.btnSearch->setEnabled(false);
    ui.btnReplaceLWO->setEnabled(false);
}

void ResourceForm::ThrowError(std::string errorMessage, std::string errorDetail)
{
    const QString qErrorMessage = QString::fromStdString(errorMessage);
    const QString qErrorDetail = QString::fromStdString(errorDetail);

    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setText(qErrorMessage);
    msgBox.setInformativeText(qErrorDetail);
    msgBox.exec();
    return;
}

void ResourceForm::ThrowFatalError(std::string errorMessage, std::string errorDetail)
{
    const QString qErrorMessage = QString::fromStdString(errorMessage);
    const QString qErrorDetail = QString::fromStdString(errorDetail);

    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Critical);
    msgBox.setText(qErrorMessage);
    msgBox.setInformativeText(qErrorDetail);
    msgBox.exec();
    return;
}

void ResourceForm::EmitResourceInfo(QString lwoFile, QString resourceFile)
{
    emit SendResourceInfo(lwoFile, resourceFile);
    return;
}

// Private Functions
int ResourceForm::ShowLoadStatus()
{
    _LoadStatusBox.setStandardButtons(QMessageBox::Cancel);
    _LoadStatusBox.setText("Loading resource, please wait...");
    int result = 0;
    result = _LoadStatusBox.exec();
    return result;
}

void ResourceForm::DisableGUI()
{
    ui.inputSearch->clear();
    ui.inputSearch->setEnabled(false);
    ui.btnSearch->setEnabled(false);
    ui.btnReplaceLWO->setEnabled(false);
    ui.btnLoadResource->setEnabled(false);
    ui.tableWidget->setEnabled(false);
    return;
}

void ResourceForm::EnableGUI()
{
    ui.inputSearch->setEnabled(true);
    ui.btnSearch->setEnabled(true);
    ui.btnReplaceLWO->setEnabled(true);
    ui.btnLoadResource->setEnabled(true);
    ui.tableWidget->setEnabled(true);
    return;
}

void ResourceForm::ResetGUITable()
{
    _ViewIsFiltered = 0;
    ui.labelStatus->clear();
    ui.tableWidget->clearContents();
    ui.tableWidget->setRowCount(0);
    ui.tableWidget->setEnabled(true);
    ui.tableWidget->setSortingEnabled(true);
    return;
}

void ResourceForm::PopulateGUIResourceTable(std::vector<std::string> searchWords)
{
    // Must disable sorting or rows won't populate correctly
    ui.tableWidget->setSortingEnabled(false);

    // Clear any existing contents
    ui.tableWidget->clearContents();
    ui.tableWidget->setRowCount(0);
    _ViewIsFiltered = 0;

    // Load resource file data
    std::vector<HAYDEN::ResourceEntry> resourceData = ModelConverter.GetResourceData();
    for (int i = 0; i < resourceData.size(); i++)
    {

        // Search for .LWO files ONLY - SERAPHIM
        if (resourceData[i].Version != 67)
            continue;

        // Filter out anything we didn't search for
        if (searchWords.size() > 0)
        {
            _ViewIsFiltered = 1;
            bool matched = 1;

            for (int j = 0; j < searchWords.size(); j++)
                if (resourceData[i].Name.find(searchWords[j]) == -1)
                    matched = 0;

            if (matched == 0)
                continue;
        }

        // Filter out unsupported .lwo
        if (resourceData[i].Version == 67)
        {
            if (resourceData[i].Name.rfind("world_") != -1 && (resourceData[i].Name.find("maps/game") != -1))
                continue;

            if (resourceData[i].Name.rfind(".bmodel") != -1)
                continue;

            if (resourceData[i].Name.rfind(".vpaint") != -1)
                continue;
        }

        int row_count = ui.tableWidget->rowCount();
        ui.tableWidget->insertRow(row_count);

        // Set Resource Name
        std::string resourceName = resourceData[i].Name;
        QString qResourceName = QString::fromStdString(resourceName);
        QTableWidgetItem *tableResourceName = new QTableWidgetItem(qResourceName);

        // Populate Table Row
        ui.tableWidget->setItem(row_count, 0, tableResourceName);
    }

    QString labelCount = QString::number(ui.tableWidget->rowCount());
    QString labelText = "Found " + labelCount + " files.";
    ui.labelStatus->setText(labelText);

    // Enable sorting again
    ui.tableWidget->setSortingEnabled(true);

    QHeaderView* tableHeader = ui.tableWidget->horizontalHeader();
    tableHeader->setSectionResizeMode(0, QHeaderView::Stretch);
    return;
}

std::vector<std::string> ResourceForm::SplitSearchTerms(std::string inputString)
{
    std::string singleWord;
    std::vector<std::string> searchWords;

    // Convert search string to lowercase
    std::for_each(inputString.begin(), inputString.end(), [](char& c) {
        c = ::tolower(c);
        });

    // Split into words, separated by spaces
    const char* delimiter = " ";
    std::stringstream checkline(inputString);

    while (std::getline(checkline, singleWord, *delimiter))
    {
        if (singleWord != delimiter)
            searchWords.push_back(singleWord);
    }
    return searchWords;
}

// Private Slots
void ResourceForm::on_btnClear_clicked()
{
    ui.btnClear->setVisible(false);
    ui.inputSearch->setText("");
    PopulateGUIResourceTable();
    return;
}

void ResourceForm::on_btnLoadResource_clicked()
{
    const QString fileName = QFileDialog::getOpenFileName(this, "", "", tr("*.resources *.resources.backup"));

    if (!fileName.isEmpty())
    {
        _ResourcePath = fileName.toStdString();

        DisableGUI();
        _LoadResourceThread = QThread::create(&HAYDEN::ModelConverter::LoadResource, &ModelConverter, _ResourcePath);

        connect(_LoadResourceThread, &QThread::finished, this, [this]()
        {
            if (_LoadStatusBox.isVisible())
                _LoadStatusBox.close();

            if (ModelConverter.HasResourceLoadError() == 1)
            {
                ThrowError(ModelConverter.GetLastErrorMessage(), ModelConverter.GetLastErrorDetail());
                ResetGUITable();
                ui.labelStatus->setText("Failed to load resource.");
            }

            if (ModelConverter.HasResourceLoadError() == 0)
            {
                PopulateGUIResourceTable();
                EnableGUI();
                _ResourceFileIsLoaded = 1;
            }

            // Must enable this even if resource loading failed
            ui.btnLoadResource->setEnabled(true);
        });

        ui.labelStatus->setText("Loading resource...");
        _LoadResourceThread->start();

        if (ShowLoadStatus() == 0x00400000 && _LoadResourceThread->isRunning()) // CANCEL
            _LoadResourceThread->terminate();
    }
}

void ResourceForm::on_btnReplaceLWO_clicked()
{
    // Show error if no items are selected for export
    QList<QTableWidgetItem*> itemExportQList = ui.tableWidget->selectedItems();
    if (itemExportQList.size() == 0)
    {
        ThrowError("Please select a .LWO file to replace.");
        return;
    }

    // Send our LWO name and resource path to mainwindow object
    LWOFileName = itemExportQList[0]->text();
    QString ResourceFilePath = QString(_ResourcePath.c_str());
    EmitResourceInfo(LWOFileName, ResourceFilePath);

    return;
}

void ResourceForm::on_btnSearch_clicked()
{
    if (ui.inputSearch->text().isEmpty())
        return;

    std::string searchText = ui.inputSearch->text().toStdString();
    std::vector<std::string> searchWords = SplitSearchTerms(searchText);

    PopulateGUIResourceTable(searchWords);

    ui.btnClear->setVisible(true);
    return;
}

void ResourceForm::on_inputSearch_returnPressed()
{
    on_btnSearch_clicked();
    return;
}

void ResourceForm::on_tableWidget_itemDoubleClicked(QTableWidgetItem* item)
{
    on_btnReplaceLWO_clicked();
    return;
}
