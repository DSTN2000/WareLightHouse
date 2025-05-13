#ifndef VIEWSCREEN_HPP
#define VIEWSCREEN_HPP

#include "mainwindow.h"
#include "firebaselib.hpp"

// Base custom delegate with privilege awareness
class BaseDelegate : public QStyledItemDelegate {
public:
    BaseDelegate(int columnIndex, bool canEdit, QObject* parent = nullptr)
        : QStyledItemDelegate(parent), columnIndex(columnIndex), canEdit(canEdit) {}

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        if (!canEdit) {
            return nullptr;
        }
        return QStyledItemDelegate::createEditor(parent, option, index);
    }

    void paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        QStyleOptionViewItem modifiedOption = option;
        if (!canEdit) {
            modifiedOption.palette.setBrush(QPalette::Text, Qt::darkGray);
            painter->fillRect(modifiedOption.rect, QColor(240, 240, 240)); // Light grey background
        }
        QStyledItemDelegate::paint(painter, modifiedOption, index);

    }

    bool helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index) override {
        if (event->type() == QEvent::ToolTip) {
            if (!canEdit) {
                QToolTip::showText(event->globalPos(), tr("You do not have permission to edit this column."), view);
                //view->setToolTip("You do not have permission to edit this column.");
                return true;
            }
        }
        return QStyledItemDelegate::helpEvent(event, view, option, index);
    }

    bool canEdit;
    int columnIndex;
};

// Custom delegate for numeric double values (prices)
class PriceDelegate : public BaseDelegate {
public:
    PriceDelegate(int columnIndex, bool canEdit, QObject* parent = nullptr)
        : BaseDelegate(columnIndex, canEdit, parent) {}

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        if (!canEdit) {
            //QToolTip::showText(QCursor::pos(), "You do not have permission to edit this column.");
            return nullptr;
        }
        QLineEdit* editor = new QLineEdit(parent);
        QDoubleValidator* validator = new QDoubleValidator(0, 999999999.99, 2, editor);
        validator->setNotation(QDoubleValidator::StandardNotation);
        validator->setLocale(QLocale::system());
        editor->setValidator(validator);
        return editor;
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override {
        BaseDelegate::setModelData(editor, model, index);
        QLineEdit* lineEdit = static_cast<QLineEdit*>(editor);
        QString value = lineEdit->text();
        bool ok;
        double number = value.toDouble(&ok);
        if (ok) {
            model->setData(index, number);
        }
    }
};

// Custom delegate for integer values (quantity, units sold)
class QuantityDelegate : public BaseDelegate {
public:
    QuantityDelegate(int columnIndex, bool canEdit, QObject* parent = nullptr)
        : BaseDelegate(columnIndex, canEdit, parent) {}

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        if (!canEdit) {
            //QToolTip::showText(QCursor::pos(), "You do not have permission to edit this column.");
            return nullptr;
        }
        QLineEdit* editor = new QLineEdit(parent);
        QIntValidator* validator = new QIntValidator(0, 999999999, editor);
        editor->setValidator(validator);
        return editor;
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override {
        BaseDelegate::setModelData(editor,model,index);
        QLineEdit* lineEdit = static_cast<QLineEdit*>(editor);
        QString value = lineEdit->text();
        bool ok;
        int number = value.toInt(&ok);
        if (ok) {
            model->setData(index, number);
        }
    }
};

// Custom delegate for single-line text (supplier)
class TextDelegate : public BaseDelegate {
public:
    TextDelegate(int columnIndex, bool canEdit, QObject* parent = nullptr, bool forbiddenChars = false)
        : BaseDelegate(columnIndex, canEdit, parent), forbiddenChars(forbiddenChars) {}
    bool forbiddenChars;
    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        if (!canEdit) {
            //QToolTip::showText(QCursor::pos(), "You do not have permission to edit this column.");
            return nullptr;
        }
        return new QLineEdit(parent);
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override {
        BaseDelegate::setModelData(editor,model,index);
        QLineEdit* lineEdit = static_cast<QLineEdit*>(editor);
        QString value = lineEdit->text();
        if (forbiddenChars)
        {
            if (std::regex_search(value.toStdString(), std::regex("[\\[\\]\\$\\#\\/\\.]")))
            {
                // Replace all forbidden charcters with nothing
                QString correctedValue = QString::fromStdString(std::regex_replace(value.toStdString(), std::regex("[\\[\\]\\$\\#\\/\\.]"), ""));
                lineEdit->setText(correctedValue);
                model->setData(index, lineEdit->text());
            }
            else
            {
                model->setData(index, lineEdit->text());
            }
        }
        else
        {
            model->setData(index, lineEdit->text());
        }
    }
};

// Custom delegate for multi-line text with word wrap (description)
class MultiLineTextDelegate : public BaseDelegate {
public:
    MultiLineTextDelegate(int columnIndex, bool canEdit, QObject* parent = nullptr)
        : BaseDelegate(columnIndex, canEdit, parent) {}

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        if (!canEdit) {
            //QToolTip::showText(QCursor::pos(), "You do not have permission to edit this column.");
            return nullptr;
        }
        QTextEdit* editor = new QTextEdit(parent);
        editor->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
        editor->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        editor->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
        editor->setTabChangesFocus(true);
        return editor;
    }

    void setEditorData(QWidget* editor, const QModelIndex& index) const override {
        QString value = index.model()->data(index, Qt::EditRole).toString();
        QTextEdit* textEdit = static_cast<QTextEdit*>(editor);
        textEdit->setText(value);
    }

    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override {
        QTextEdit* textEdit = static_cast<QTextEdit*>(editor);
        model->setData(index, textEdit->toPlainText(), Qt::EditRole);
    }

    void updateEditorGeometry(QWidget* editor, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        QRect rect = option.rect;
        rect.setHeight(std::max(rect.height(), 100));
        editor->setGeometry(rect);
    }
};

class ViewScreen : public QWidget {
    Q_OBJECT

public:
    // Constructor that takes the json object of a specific company
    explicit ViewScreen(FirebaseDB *db, const json& companyData, const std::string& companyName, const json& privileges, QWidget* parent = nullptr)
        : QWidget(parent), db(*db), companyData(companyData), companyName(companyName) {
        userPrivileges = privileges;

        // Set up UI components
        setupUI();

        // Initialize with the first category and product
        populateCategoryComboBox();
        updateProductTable();
    }

private slots:
    // Slot that gets called when the category selection changes
    void onCategoryChanged(const QString& category) {
        currentCategory = category.toStdString();
        updateProductTable();
    }


    // Add product button handler
    void onAddProductClicked() {
        // Check if a category is selected
        if (currentCategory == "") {
            QMessageBox::warning(this, tr("Error"), tr("Create a category first!"));
            return;
        }
        // Create a dialog to get new product details
        QDialog dialog(this);
        dialog.setWindowTitle(tr("Add New Product"));

        // Create form layout for the dialog
        QFormLayout* formLayout = new QFormLayout(&dialog);

        // Create input fields
        QLineEdit* nameEdit = new QLineEdit();
        QDoubleSpinBox* buyPriceEdit = new QDoubleSpinBox();
        QDoubleSpinBox* sellPriceEdit = new QDoubleSpinBox();
        QSpinBox* quantityEdit = new QSpinBox();
        QLineEdit* supplierEdit = new QLineEdit();
        QTextEdit* descriptionEdit = new QTextEdit();

        // Set ranges for numeric fields
        buyPriceEdit->setMaximum(999999999.99);
        sellPriceEdit->setMaximum(999999999.99);
        quantityEdit->setMaximum(999999999);

        // Add fields to form
        formLayout->addRow(tr("Product Name:"), nameEdit);
        formLayout->addRow(tr("Buy Price:"), buyPriceEdit);
        formLayout->addRow(tr("Sell Price:"), sellPriceEdit);
        formLayout->addRow(tr("Quantity:"), quantityEdit);
        formLayout->addRow(tr("Supplier:"), supplierEdit);
        formLayout->addRow(tr("Description:"), descriptionEdit);

        // Add buttons to dialog
        QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
        formLayout->addRow(buttonBox);

        // Execute dialog
        if (dialog.exec() == QDialog::Accepted) {
            if (std::regex_search(nameEdit->text().toStdString(), std::regex("[\\[\\]\\$\\#\\/\\.]")))
            {
                QMessageBox::warning(this, tr("Error"), tr("Product name must not contain these symbols: $ # [ ] / or ."));
                return;
            }
            if (companyData["stock"][currentCategory].contains(nameEdit->text())) {
                QMessageBox::warning(this, tr("Error"), tr("A product with this name already exists!"));
                return;
            }
            else if (nameEdit->text().isEmpty()) {
                QMessageBox::warning(this, tr("Error"), tr("Please enter a product name!"));
                return;
            }

            // Create new product in the model
            int newRow = tableModel->rowCount();
            tableModel->insertRow(newRow);

            // Set data for the new row
            tableModel->setItem(newRow, 0, new QStandardItem(nameEdit->text()));
            tableModel->setItem(newRow, 1, new QStandardItem(QString::number(buyPriceEdit->value())));
            tableModel->setItem(newRow, 2, new QStandardItem(QString::number(sellPriceEdit->value())));
            tableModel->setItem(newRow, 3, new QStandardItem(QString::number(quantityEdit->value())));
            tableModel->setItem(newRow, 4, new QStandardItem("0")); // Units sold starts at 0
            tableModel->setItem(newRow, 5, new QStandardItem(supplierEdit->text()));
            tableModel->setItem(newRow, 6, new QStandardItem(descriptionEdit->toPlainText()));

            // Also update the JSON data
            json newProduct;
            newProduct["buyPrice"] = buyPriceEdit->value();
            newProduct["sellPrice"] = sellPriceEdit->value();
            newProduct["quantity"] = quantityEdit->value();
            newProduct["unitsSold"] = 0;
            newProduct["supplier"] = supplierEdit->text().toStdString();
            newProduct["description"] = descriptionEdit->toPlainText().toStdString();

            // Add to company data
            companyData["stock"][currentCategory][nameEdit->text().toStdString()] = newProduct;

            // Refresh the data for proper sorting
            updateProductTable();
        }
    }

    // Delete product button handler
    void onDeleteProductClicked() {
        // Get selected row
        QModelIndexList selection = tableView->selectionModel()->selectedRows();

        // Check if a row is selected
        if (selection.isEmpty()) {
            QMessageBox::warning(this, tr("Warning"), tr("Please select a product to delete."));
            return;
        }

        // Get the product name from the first column of the selected row
        int row = selection.first().row();
        QString productName = tableModel->item(row, 0)->text();

        // Confirm deletion
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("Confirm Deletion"),
                                      tr("Are you sure you want to delete ") + productName + "?",
                                      QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::Yes) {
            // Remove from the model
            tableModel->removeRow(row);

            // Remove from the JSON data
            companyData["stock"][currentCategory].erase(productName.toStdString());
        }
    }

    // Save changes button handler
    void onSaveChangesClicked() {
        companyData["stock"][currentCategory].clear();
        // Iterate through all rows in the table
        for (int row = 0; row < tableModel->rowCount(); row++) {
            QString productName = tableModel->item(row, 0)->text();

            // Update JSON data from table data
            json& product = companyData["stock"][currentCategory][productName.toStdString()];

            // Update each field if the item exists
            if (tableModel->item(row, 1))
                product["buyPrice"] = tableModel->item(row, 1)->text().toDouble();

            if (tableModel->item(row, 2))
                product["sellPrice"] = tableModel->item(row, 2)->text().toDouble();

            if (tableModel->item(row, 3))
                product["quantity"] = tableModel->item(row, 3)->text().toInt();

            if (tableModel->item(row, 4))
                product["unitsSold"] = tableModel->item(row, 4)->text().toInt();

            if (tableModel->item(row, 5))
                product["supplier"] = tableModel->item(row, 5)->text().toStdString();

            if (tableModel->item(row, 6))
                product["description"] = tableModel->item(row, 6)->text().toStdString();
        }


        // Clear the data
        std::string path = "companies/" + companyName + "/stock";
        qDebug() << path;
        path = db.urlEncode(path);
        db.deleteData(path);
        // Save to Firebase
        if (companyData["stock"].contains(""))
        {
            companyData["stock"].clear();
        }
        if (!db.writeData(path, companyData["stock"]))
        {
            QMessageBox::critical(this, tr("Error"), tr("Unknown error occured when writing data"));
        }
        else
        QMessageBox::information(this, tr("Success"), tr("Changes saved successfully."));
    }

    // Manage categories button handler
    void onManageCategoriesClicked() {
        // Create dialog
        QDialog dialog(this);
        dialog.setWindowTitle(tr("Manage Categories"));
        dialog.setMinimumWidth(400);

        // Create layout
        QVBoxLayout* layout = new QVBoxLayout(&dialog);

        // Create list widget to display categories
        QListWidget* categoryList = new QListWidget();
        layout->addWidget(categoryList);

        // Populate the list with categories
        if (companyData.contains("stock") && companyData["stock"].is_object()) {
            for (auto& [category, _] : companyData["stock"].items()) {
                categoryList->addItem(QString::fromStdString(category));
            }
        }

        // Create button panel
        QHBoxLayout* buttonLayout = new QHBoxLayout();
        QPushButton* addButton = new QPushButton(tr("Add Category"));
        QPushButton* renameButton = new QPushButton(tr("Rename Category"));
        QPushButton* deleteButton = new QPushButton(tr("Delete Category"));

        buttonLayout->addWidget(addButton);
        buttonLayout->addWidget(renameButton);
        buttonLayout->addWidget(deleteButton);

        layout->addLayout(buttonLayout);

        // Dialog buttons
        QDialogButtonBox* dialogButtons = new QDialogButtonBox(QDialogButtonBox::Ok);
        connect(dialogButtons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        layout->addWidget(dialogButtons);

        // Connect category management buttons
        connect(addButton, &QPushButton::clicked, [&]() {
            bool ok;
            QString newCategory = QInputDialog::getText(&dialog, tr("Add Category"),
                                                        tr("Category name:"), QLineEdit::Normal,
                                                        "", &ok).trimmed();
            if (std::regex_search(newCategory.toStdString(), std::regex("[\\[\\]\\$\\#\\/\\.]")))
            {
                QMessageBox::warning(&dialog, tr("Error"), tr("Category name must not contain these symbols: $ # [ ] / or ."));
                return;
            }
            if (ok && !newCategory.isEmpty()) {
                // Check if category already exists
                if (companyData["stock"].contains(newCategory.toStdString())) {
                    QMessageBox::warning(&dialog, tr("Warning"), tr("Category already exists."));
                    return;
                }

                // Add new category to JSON and list
                companyData["stock"][newCategory.toStdString()] = json::object();
                categoryList->addItem(newCategory);
            }
        });

        connect(renameButton, &QPushButton::clicked, [&]() {
            // Get selected category
            QListWidgetItem* selectedItem = categoryList->currentItem();
            if (!selectedItem) {
                QMessageBox::warning(&dialog, tr("Warning"), tr("Please select a category to rename."));
                return;
            }

            QString oldName = selectedItem->text();

            // Get new name
            bool ok;
            QString newName = QInputDialog::getText(&dialog, tr("Rename Category"),
                                                    tr("New name:"), QLineEdit::Normal,
                                                    oldName, &ok);
            if (std::regex_search(newName.toStdString(), std::regex("[\\[\\]\\$\\#\\/\\.]")))
            {
                QMessageBox::warning(&dialog, tr("Error"), tr("Category name must not contain these symbols: $ # [ ] / or ."));
                return;
            }
            if (ok && !newName.isEmpty() && newName != oldName) {
                // Check if the new name already exists
                if (companyData["stock"].contains(newName.toStdString())) {
                    QMessageBox::warning(&dialog, tr("Warning"), tr("Category name already exists."));
                    return;
                }

                // Copy data from old category to new
                companyData["stock"][newName.toStdString()] = companyData["stock"][oldName.toStdString()];
                companyData["stock"].erase(oldName.toStdString());

                // Update list item
                selectedItem->setText(newName);
            }
        });

        connect(deleteButton, &QPushButton::clicked, [&]() {
            // Get selected category
            QListWidgetItem* selectedItem = categoryList->currentItem();
            if (!selectedItem) {
                QMessageBox::warning(&dialog, tr("Warning"), tr("Please select a category to delete."));
                return;
            }

            QString categoryToDelete = selectedItem->text();

            // Confirm deletion
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(&dialog, tr("Confirm Deletion"),
                                          tr("Are you sure you want to delete category '") +
                                              categoryToDelete + tr("' and all its products?"),
                                          QMessageBox::Yes | QMessageBox::No);

            if (reply == QMessageBox::Yes) {
                // Remove from JSON
                companyData["stock"].erase(categoryToDelete.toStdString());

                // Remove from list
                delete categoryList->takeItem(categoryList->row(selectedItem));
            }
        });

        // Execute dialog
        if (dialog.exec() == QDialog::Accepted) {
            // Save changes to Firebase
            std::string path = "companies/" + companyName;
            path = db.urlEncode(path);
            if (!db.writeData(path, companyData))
            {
                QMessageBox::critical(this, tr("Error"), tr("Unknown error occured when writing data"));
            }

            // Refresh category combo box
            QString currentCategoryText = QString::fromStdString(currentCategory);
            populateCategoryComboBox();

            // Try to set the previously selected category
            int index = categoryComboBox->findText(currentCategoryText);
            if (index >= 0) {
                categoryComboBox->setCurrentIndex(index);
            } else if (categoryComboBox->count() > 0) {
                // If not found, set to first category
                categoryComboBox->setCurrentIndex(0);
            } else {
                // If no categories, clear the table
                tableModel->removeRows(0, tableModel->rowCount());
            }
        }
    }

    void onExportDataClicked() {
        json exportData;

        // Directly copy the stock data
        if (companyData.contains("stock") && companyData["stock"].is_object()) {
            exportData["stock"] = companyData["stock"];
        }
        // Remove inaccessible categories
        for (auto& [category, _] : exportData["stock"].items())
        {
            if (std::find(userCategories.begin(), userCategories.end(), category)==userCategories.end())
            {
                exportData["stock"].erase(category);
            }
        }

        // Serialize the json object to a string
        std::string jsonData = exportData.dump(4);

        // Get the default downloads path
        QString downloadsPath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
        auto timestamp = QDateTime::fromMSecsSinceEpoch(QDateTime::currentMSecsSinceEpoch()).toString("hhmmssddMMyyyy").toStdString();
        QString filePath = downloadsPath + "/stock_data_" + QString::fromStdString(companyName) + QString::fromStdString(timestamp) + ".json";
        filePath = QString::fromStdString(std::regex_replace(filePath.toStdString(), std::regex(" "), ""));

        std::ofstream outputFile(filePath.toStdString());
        if (outputFile.is_open()) {
            outputFile << jsonData << std::endl;
            outputFile.close();
            QMessageBox::information(this, tr("Export Successful"), tr("Data exported to:\n") + filePath);
        } else {
            QMessageBox::critical(this, tr("Export Failed"), tr("Could not open file for writing."));
        }
    }

private:
    // Set up the UI components
    void setupUI() {
        // Create layout
        QVBoxLayout* mainLayout = new QVBoxLayout(this);

        // Create company name label with bold font
        QLabel* companyLabel = new QLabel(QString::fromStdString(companyName + tr(" stock").toStdString()));
        QFont font = companyLabel->font();
        font.setBold(true);
        font.setPointSize(12);
        companyLabel->setFont(font);
        mainLayout->addWidget(companyLabel);

        // Create horizontal layout for category selection
        QHBoxLayout* categoryLayout = new QHBoxLayout();
        QLabel* categoryLabel = new QLabel(tr("Category:"));
        QPushButton* exportDataButton = new QPushButton(tr("Export Data (JSON)"));
        categoryComboBox = new QComboBox();


        categoryLayout->addWidget(categoryLabel);
        categoryLayout->addWidget(categoryComboBox, Qt::AlignLeft);
        //categoryLayout->addStretch();
        categoryLayout->addWidget(exportDataButton);

        mainLayout->addLayout(categoryLayout);

        // Create table view
        tableView = new QTableView();
        tableModel = new QStandardItemModel(this);

        // Set headers for the table
        QStringList headers = {tr("Product"), tr("Buy Price"), tr("Sell Price"), tr("Quantity"), tr("Units Sold"), tr("Supplier"), tr("Description")};
        tableModel->setHorizontalHeaderLabels(headers);

        tableView->setModel(tableModel);

        tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
        tableView->horizontalHeader()->setStretchLastSection(true);
        tableView->verticalHeader()->setSectionResizeMode(QHeaderView::Interactive);

        tableView->setWordWrap(true);
        tableView->setAlternatingRowColors(true);
        tableView->setSelectionBehavior(QTableView::SelectRows);

        tableView->setSortingEnabled(true);
        tableView->horizontalHeader()->setSortIndicatorShown(true);
        tableView->horizontalHeader()->setSortIndicator(-1, Qt::AscendingOrder); // No initial sorting

        tableModel->setSortRole(Qt::UserRole);

        // Allow editing of cells
        tableView->setEditTriggers(QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed);

        mainLayout->addWidget(tableView);

        // Create button panel
        QHBoxLayout* buttonLayout = new QHBoxLayout();

        // Create buttons
        QPushButton* addProductButton = new QPushButton(tr("Add Product"));
        QPushButton* deleteProductButton = new QPushButton(tr("Delete Product"));
        QPushButton* saveChangesButton = new QPushButton(tr("Save Changes"));
        QPushButton* manageCategoriesButton = new QPushButton(tr("Manage Categories")); // Manage Categories is an admin-only button


        // Restrict button usage to privileged users only
        if (!userPrivileges.empty()) {
            bool canAddDeleteProduct = userPrivileges["addProductDeleteProduct"].get<bool>();

            addProductButton->setEnabled(canAddDeleteProduct);
            deleteProductButton->setEnabled(canAddDeleteProduct);
        }

        // Add buttons to layout
        buttonLayout->addWidget(addProductButton);
        buttonLayout->addWidget(deleteProductButton);
        buttonLayout->addWidget(saveChangesButton);
        if (userPrivileges.empty())
        {
            buttonLayout->addWidget(manageCategoriesButton);
        }
        buttonLayout->addStretch();

        // Add button layout to main layout
        mainLayout->addLayout(buttonLayout);

        //Connect signals and slots
        connect(categoryComboBox, SIGNAL(currentTextChanged(const QString&)),
                this, SLOT(onCategoryChanged(const QString&)));
        connect(addProductButton, SIGNAL(clicked()),
                this, SLOT(onAddProductClicked()));
        connect(deleteProductButton, SIGNAL(clicked()),
                this, SLOT(onDeleteProductClicked()));
        connect(saveChangesButton, SIGNAL(clicked()),
                this, SLOT(onSaveChangesClicked()));
        connect(manageCategoriesButton, SIGNAL(clicked()),
                this, SLOT(onManageCategoriesClicked()));
        connect(exportDataButton, SIGNAL(clicked()),
                this, SLOT(onExportDataClicked()));

        if (!userPrivileges.empty()) {
            // Configure delegates based on user privileges
            bool canEditProductName = userPrivileges["columnPrivileges"]["editProductName"].get<bool>();
            bool canEditBuyPrice = userPrivileges["columnPrivileges"]["editBuyPrice"].get<bool>();
            bool canEditSellPrice = userPrivileges["columnPrivileges"]["editSellPrice"].get<bool>();
            bool canEditQuantity = userPrivileges["columnPrivileges"]["editQuantity"].get<bool>();
            bool canEditUnitsSold = userPrivileges["columnPrivileges"]["editUnitsSold"].get<bool>();
            bool canEditSupplier = userPrivileges["columnPrivileges"]["editSupplier"].get<bool>();
            bool canEditDescription = userPrivileges["columnPrivileges"]["editDescription"].get<bool>();

            // Set custom delegates with privilege checks
            tableView->setItemDelegateForColumn(0, new TextDelegate(0, canEditProductName, this, true));
            tableView->setItemDelegateForColumn(1, new PriceDelegate(1, canEditBuyPrice, this));
            tableView->setItemDelegateForColumn(2, new PriceDelegate(2, canEditSellPrice, this));
            tableView->setItemDelegateForColumn(3, new QuantityDelegate(3, canEditQuantity, this));
            tableView->setItemDelegateForColumn(4, new QuantityDelegate(4, canEditUnitsSold, this));
            tableView->setItemDelegateForColumn(5, new TextDelegate(5, canEditSupplier, this));
            tableView->setItemDelegateForColumn(6, new MultiLineTextDelegate(6, canEditDescription, this));

            // Disable editing of entire columns based on privileges
            tableView->setEditTriggers(
                (canEditProductName || canEditBuyPrice || canEditSellPrice ||
                 canEditQuantity || canEditUnitsSold ||
                 canEditSupplier || canEditDescription)
                    ? QAbstractItemView::DoubleClicked | QAbstractItemView::EditKeyPressed
                    : QAbstractItemView::NoEditTriggers);
        } else {
            // If no privileges are set, use the old delegates for editing
            tableView->setItemDelegateForColumn(0, new TextDelegate(0, true, this, true));
            tableView->setItemDelegateForColumn(1, new PriceDelegate(1, true, this));
            tableView->setItemDelegateForColumn(2, new PriceDelegate(2, true, this));
            tableView->setItemDelegateForColumn(3, new QuantityDelegate(3, true, this));
            tableView->setItemDelegateForColumn(4, new QuantityDelegate(4, true, this));
            tableView->setItemDelegateForColumn(5, new TextDelegate(5, true, this));
            tableView->setItemDelegateForColumn(6, new MultiLineTextDelegate(6, true, this));
        }
    }

    // Populate the category combo box with available categories from the JSON
    void populateCategoryComboBox() {
        categoryComboBox->clear();

        // Get user's accessible categories
        //userCategories;
        if (!userPrivileges.empty()) {
            userCategories = userPrivileges["categories"].get<std::vector<std::string>>();
        } else {
            for (auto& [category, _] : companyData["stock"].items()) userCategories.push_back(category);
        }

        // Check if stock exists in the company data
        if (companyData.contains("stock") && companyData["stock"].is_object()) {
            for (auto& [category, _] : companyData["stock"].items()) {
                // Only add categories the user has access to
                if (std::find(userCategories.begin(), userCategories.end(), category)
                    != userCategories.end()) {
                    categoryComboBox->addItem(QString::fromStdString(category));
                }
            }
            if (categoryComboBox->count() > 0) {
                currentCategory = categoryComboBox->itemText(0).toStdString();
            }

        }
    }

    // Update the table with products from the current category
    void updateProductTable() {
        // Clear previous data
        tableModel->removeRows(0, tableModel->rowCount());

        // Check if the current category exists
        if (companyData.contains("stock") &&
            companyData["stock"].contains(currentCategory) &&
            companyData["stock"][currentCategory].is_object()) {

            const auto& categoryData = companyData["stock"][currentCategory];

            // Add each product to the table
            int row = 0;
            for (auto& [productName, productDetails] : categoryData.items()) {
                if (productDetails.is_object()) {
                    // First column is product name
                    tableModel->setItem(row, 0, new QStandardItem(QString::fromStdString(productName)));
                    tableModel->item(row, 0)->setData(QString::fromStdString(productName), Qt::UserRole);

                    // Add product details
                    if (productDetails.contains("buyPrice") && productDetails["buyPrice"].is_number()) {
                        tableModel->setItem(row, 1, new QStandardItem(QString::number(productDetails["buyPrice"].get<double>())));
                        tableModel->item(row, 1)->setData(productDetails["buyPrice"].get<double>(), Qt::UserRole);
                    }

                    if (productDetails.contains("sellPrice") && productDetails["sellPrice"].is_number()) {
                        tableModel->setItem(row, 2, new QStandardItem(QString::number(productDetails["sellPrice"].get<double>())));
                        tableModel->item(row, 2)->setData(productDetails["sellPrice"].get<double>(), Qt::UserRole);
                    }

                    if (productDetails.contains("quantity") && productDetails["quantity"].is_number()) {
                        tableModel->setItem(row, 3, new QStandardItem(QString::number(productDetails["quantity"].get<int>())));
                        tableModel->item(row, 3)->setData(productDetails["quantity"].get<int>(), Qt::UserRole);
                    }

                    if (productDetails.contains("unitsSold") && productDetails["unitsSold"].is_number()) {
                        tableModel->setItem(row, 4, new QStandardItem(QString::number(productDetails["unitsSold"].get<int>())));
                        tableModel->item(row, 4)->setData(productDetails["unitsSold"].get<int>(), Qt::UserRole);
                    }

                    if (productDetails.contains("supplier") && productDetails["supplier"].is_string()) {
                        tableModel->setItem(row, 5, new QStandardItem(QString::fromStdString(productDetails["supplier"].get<std::string>())));
                        tableModel->item(row, 5)->setData(QString::fromStdString(productDetails["supplier"]), Qt::UserRole);
                    }

                    if (productDetails.contains("description") && productDetails["description"].is_string()) {
                        tableModel->setItem(row, 6, new QStandardItem(QString::fromStdString(productDetails["description"].get<std::string>())));
                        tableModel->item(row, 6)->setData(QString::fromStdString(productDetails["description"]), Qt::UserRole);
                    }

                    row++;
                }
            }
        }
    }




    // Member variables
    json companyData;         // Stores the JSON data for the company
    std::string companyName;       // Stores the name of the company
    std::string currentCategory;     // Currently selected category
    FirebaseDB db;
    json userPrivileges;
    std::vector<std::string> userCategories;

    QComboBox* categoryComboBox;     // Dropdown for selecting categories
    QTableView* tableView;          // Table view to display products
    QStandardItemModel* tableModel;    // Model for the table data
};


#endif // VIEWSCREEN_HPP
