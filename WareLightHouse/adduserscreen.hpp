#ifndef ADDUSERSCREEN_HPP
#define ADDUSERSCREEN_HPP

#include "mainwindow.h"
#include "firebaselib.hpp"

class AddUserScreen : public QWidget {
    Q_OBJECT

public:
    explicit AddUserScreen(FirebaseDB* db, const json& companyData, std::string companyName, QWidget* parent = nullptr)
        : QWidget(parent), db(*db), companyData(companyData), companyName(companyName) {
        setupUI();
    }
    void populateCategories()
    {
        categoryCheckboxes.clear(); // Clear the list of checkboxes

        // Clear the existing layout in scrollContent
        QLayoutItem *item;
        while ((item = scrollLayout->takeAt(0)) != nullptr) {
            delete item->widget(); // Delete the widget
            delete item;         // Delete the layout item
        }

        companyData = db.readData("companies")[companyName];
        if (companyData.contains("stock") && companyData["stock"].is_object()) {
            for (auto& [category, _] : companyData["stock"].items()) {
                QCheckBox* categoryCheckbox = new QCheckBox(QString::fromStdString(category));
                scrollLayout->addWidget(categoryCheckbox);
                categoryCheckboxes.append(categoryCheckbox);
            }
        }
    }

private slots:
    void onCreateUserClicked() {
        // Validate username
        companyData = db.readData("companies")[companyName];
        QString username = usernameEdit->text().trimmed();
        if (username.isEmpty()) {
            QMessageBox::warning(this, tr("Error"), tr("Username cannot be empty."));
            return;
        }
        else if (companyData["users"].contains(username))
        {
            QMessageBox::warning(this, tr("Error"), tr("User already exists."));
            return;
        }

        // Validate password
        QString password = passwordEdit->text().trimmed();
        if (password.length()<8) {
            QMessageBox::warning(this, tr("Error"), tr("Password must be at least 8 characters."));
            return;
        }

        // Check if at least one category is selected
        bool categorySelected = false;
        std::vector<std::string> selectedCategories;
        for (auto* checkbox : categoryCheckboxes) {
            if (checkbox->isChecked()) {
                categorySelected = true;
                selectedCategories.push_back(checkbox->text().toStdString());
            }
        }

        if (!categorySelected) {
            QMessageBox::warning(this, tr("Error"), tr("Please select at least one category."));
            return;
        }

        // Collect privileges
        json privileges;
        privileges["viewDatabase"] = true; // Always set to true, overriding any potential unchecking
        privileges["addProductDeleteProduct"] = addProductDeleteProductCheckbox->isChecked();

        // Column edit privileges
        json columnPrivileges;
        columnPrivileges["editProductName"] = editProductNameCheckbox->isChecked();
        columnPrivileges["editBuyPrice"] = editBuyPriceCheckbox->isChecked();
        columnPrivileges["editSellPrice"] = editSellPriceCheckbox->isChecked();
        columnPrivileges["editQuantity"] = editQuantityCheckbox->isChecked();
        columnPrivileges["editUnitsSold"] = editUnitsSoldCheckbox->isChecked();
        columnPrivileges["editSupplier"] = editSupplierCheckbox->isChecked();
        columnPrivileges["editDescription"] = editDescriptionCheckbox->isChecked();

        privileges["columnPrivileges"] = columnPrivileges;

        // Prepare user data
        json userData;
        userData["username"] = username.toStdString();
        userData["password"] = password.toStdString();
        userData["privileges"] = privileges;
        userData["privileges"]["categories"] = selectedCategories;

        // Save user to Firebase
        try {
            std::string path = "companies/" + companyName + "/users/" + username.toStdString();
            path = db.urlEncode(path);

            db.writeData(path, userData);

            QMessageBox::information(this, tr("Success"), tr("User created successfully!"));

            // Clear form after successful creation
            usernameEdit->clear();
            passwordEdit->clear();
            viewDatabaseCheckbox->setChecked(true);
            addProductDeleteProductCheckbox->setChecked(false);
            editProductNameCheckbox->setChecked(false);
            editBuyPriceCheckbox->setChecked(false);
            editSellPriceCheckbox->setChecked(false);
            editQuantityCheckbox->setChecked(false);
            editUnitsSoldCheckbox->setChecked(false);
            editSupplierCheckbox->setChecked(false);
            editDescriptionCheckbox->setChecked(false);

            // Uncheck all category checkboxes
            for (auto* checkbox : categoryCheckboxes) {
                checkbox->setChecked(false);
            }
        }
        catch (const std::exception& e) {
            QMessageBox::critical(this, tr("Error"), QString(tr("Failed to create user: %1").arg(e.what())));
        }
    }

    void onSelectAllCategoriesClicked() {
        bool shouldBeChecked = selectAllCategoriesButton->isChecked();
        for (auto* checkbox : categoryCheckboxes) {
            checkbox->setChecked(shouldBeChecked);
        }
    }


private:
    void setupUI() {
        // Main layout
        QVBoxLayout* mainLayout = new QVBoxLayout(this);

        // Title
        QLabel* titleLabel = new QLabel(tr("Create New User"));
        QFont titleFont = titleLabel->font();
        titleFont.setBold(true);
        titleFont.setPointSize(14);
        titleLabel->setFont(titleFont);
        mainLayout->addWidget(titleLabel);

        // Username input
        QHBoxLayout* usernameLayout = new QHBoxLayout();
        QLabel* usernameLabel = new QLabel(tr("Username:"));
        usernameEdit = new QLineEdit();
        usernameLayout->addWidget(usernameLabel);
        usernameLayout->addWidget(usernameEdit);
        mainLayout->addLayout(usernameLayout);

        // Password input
        QHBoxLayout* passwordLayout = new QHBoxLayout();
        QLabel* passwordLabel = new QLabel(tr("Password:"));
        passwordEdit = new QLineEdit();
        passwordEdit->setEchoMode(QLineEdit::Password);
        passwordLayout->addWidget(passwordLabel);
        passwordLayout->addWidget(passwordEdit);
        mainLayout->addLayout(passwordLayout);

        // Privileges Group Box
        QGroupBox* privilegesGroup = new QGroupBox(tr("User Privileges"));
        QVBoxLayout* privilegesLayout = new QVBoxLayout();

        // Database View Privilege
        viewDatabaseCheckbox = new QCheckBox(tr("View Database"));
        viewDatabaseCheckbox->setChecked(true);  // Preselect the checkbox
        viewDatabaseCheckbox->setEnabled(false); // Grey out the checkbox
        privilegesLayout->addWidget(viewDatabaseCheckbox);

        // Add/Delete Products Privilege
        addProductDeleteProductCheckbox = new QCheckBox(tr("Add/Delete Products"));
        privilegesLayout->addWidget(addProductDeleteProductCheckbox);

        // Column Edit Privileges Group
        QGroupBox* columnPrivilegesGroup = new QGroupBox(tr("Column Edit Privileges"));
        QVBoxLayout* columnPrivilegesLayout = new QVBoxLayout();

        editProductNameCheckbox = new QCheckBox(tr("Edit Product Name"));
        editBuyPriceCheckbox = new QCheckBox(tr("Edit Buy Price"));
        editSellPriceCheckbox = new QCheckBox(tr("Edit Sell Price"));
        editQuantityCheckbox = new QCheckBox(tr("Edit Quantity"));
        editUnitsSoldCheckbox = new QCheckBox(tr("Edit Units Sold"));
        editSupplierCheckbox = new QCheckBox(tr("Edit Supplier"));
        editDescriptionCheckbox = new QCheckBox(tr("Edit Description"));

        columnPrivilegesLayout->addWidget(editProductNameCheckbox);
        columnPrivilegesLayout->addWidget(editBuyPriceCheckbox);
        columnPrivilegesLayout->addWidget(editSellPriceCheckbox);
        columnPrivilegesLayout->addWidget(editQuantityCheckbox);
        columnPrivilegesLayout->addWidget(editUnitsSoldCheckbox);
        columnPrivilegesLayout->addWidget(editSupplierCheckbox);
        columnPrivilegesLayout->addWidget(editDescriptionCheckbox);

        columnPrivilegesGroup->setLayout(columnPrivilegesLayout);
        privilegesLayout->addWidget(columnPrivilegesGroup);

        privilegesGroup->setLayout(privilegesLayout);
        mainLayout->addWidget(privilegesGroup);

        // Categories Group Box with Scroll Area
        categoriesGroup = new QGroupBox(tr("Accessible Categories"));
        categoriesLayout = new QVBoxLayout();
        scrollArea = new QScrollArea();
        scrollContent = new QWidget();
        scrollLayout = new QVBoxLayout(scrollContent);

        // Add Select All/Deselect All button
        selectAllCategoriesButton = new QCheckBox(tr("Select All/Deselect All"));
        connect(selectAllCategoriesButton, &QCheckBox::clicked, this, &AddUserScreen::onSelectAllCategoriesClicked);
        categoriesLayout->addWidget(selectAllCategoriesButton);
        populateCategories();
        scrollArea->setWidget(scrollContent);
        scrollArea->setWidgetResizable(true);
        categoriesLayout->addWidget(scrollArea);
        categoriesGroup->setLayout(categoriesLayout);
        mainLayout->addWidget(categoriesGroup);

        // Create User Button
        QPushButton* createUserButton = new QPushButton(tr("Create User"));
        connect(createUserButton, &QPushButton::clicked, this, &AddUserScreen::onCreateUserClicked);
        mainLayout->addWidget(createUserButton);

        // Add stretch to push everything to the top
        mainLayout->addStretch();
    }

    // Member variables
    FirebaseDB& db;
    json companyData;
    std::string companyName;

    // Input fields
    QLineEdit* usernameEdit;
    QLineEdit* passwordEdit;

    // Privilege Checkboxes
    QCheckBox* viewDatabaseCheckbox;
    QCheckBox* addProductDeleteProductCheckbox;
    QCheckBox* editProductNameCheckbox;
    QCheckBox* editBuyPriceCheckbox;
    QCheckBox* editSellPriceCheckbox;
    QCheckBox* editQuantityCheckbox;
    QCheckBox* editUnitsSoldCheckbox;
    QCheckBox* editSupplierCheckbox;
    QCheckBox* editDescriptionCheckbox;

    // Category Checkboxes
    QList<QCheckBox*> categoryCheckboxes;
    QCheckBox* selectAllCategoriesButton;

    QGroupBox* categoriesGroup;
    QVBoxLayout* categoriesLayout;
    QScrollArea* scrollArea;
    QWidget* scrollContent;
    QVBoxLayout* scrollLayout;
};

#endif // ADDUSERSCREEN_HPP
