#ifndef EDITUSERSCREEN_HPP
#define EDITUSERSCREEN_HPP

#include "mainwindow.h"

#include "firebaselib.hpp"

class EditUserScreen : public QWidget {
    Q_OBJECT

public:
    explicit EditUserScreen(FirebaseDB* db, const json& companyData, std::string companyName, QWidget* parent = nullptr)
        : QWidget(parent), db(*db), companyData(companyData), companyName(companyName) {
        setupUI();
        loadUsers(); // Load users into the dropdown
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
        loadUsers();
    }

    void onUserSelected(int index) {
        if (index <= 0 || index >= userComboBox->count()) { // Index 0 is placeholder, also check upper bound
            clearForm();
            setFormEnabled(false);
            currentUsernameKey = ""; // Use a specific variable for the original key
            return;
        }

        QString selectedUsernameKey = userComboBox->itemData(index).toString(); // Get key from item data
        currentUsernameKey = selectedUsernameKey; // Store the key of the user being edited

        if (usersData.contains(selectedUsernameKey)) {
            try {
                // Fetch fresh data for the selected user upon selection
                std::string userPath = "companies/" + companyName + "/users/" + selectedUsernameKey.toStdString();
                userPath = db.urlEncode(userPath);
                json specificUserData = db.readData(userPath);

                // Update local cache just in case
                usersData[selectedUsernameKey] = specificUserData;

                populateForm(specificUserData);
                setFormEnabled(true); // Enable form fields for editing

            } catch (const std::exception& e) {
                QMessageBox::critical(this, tr("Error"), QString(tr("Failed to load data for user key %1: %2").arg(selectedUsernameKey).arg(e.what())));
                clearForm();
                setFormEnabled(false);
                currentUsernameKey = "";
            }
        } else {
            QMessageBox::warning(this, tr("Error"), tr("Selected user data not found in cache."));
            clearForm();
            setFormEnabled(false);
            currentUsernameKey = "";
        }
    }

    void onSaveUserClicked() {
        if (currentUsernameKey.isEmpty()) {
            QMessageBox::warning(this, tr("Error"), tr("No user selected to save."));
            return;
        }

        // --- Get Old and New Usernames ---
        QString oldUsernameKey = currentUsernameKey; // The key used to fetch the user
        QString newUsername = usernameEdit->text().trimmed();

        // --- Basic Validations ---
        if (newUsername.isEmpty()) {
            QMessageBox::warning(this, tr("Error"), tr("Username cannot be empty."));
            return;
        }
        if (std::regex_search(newUsername.toStdString(), std::regex("[\\[\\]\\$\\#\\/\\.]")))
        {
            QMessageBox::warning(this, tr("Error"), tr("Username must not contain these symbols: $ # [ ] / or ."));
            return;
        }

        QString password = passwordEdit->text().trimmed(); // Password is now editable and visible
        if (password.length() < 8) {
            QMessageBox::warning(this, tr("Error"), tr("Password must be at least 8 characters."));
            return;
        }

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

        // --- Check if new username already exists (if changed) ---
        bool usernameChanged = (newUsername != oldUsernameKey);
        if (usernameChanged) {
            // Check against other keys in our cached map
            for(const auto& key : usersData.keys()) {
                if (key == oldUsernameKey) continue; // Skip self
                if (key.compare(newUsername, Qt::CaseInsensitive) == 0) { // Case-insensitive check recommended
                    QMessageBox::warning(this, tr("Error"), QString(tr("Username '%1' already exists.").arg(newUsername)));
                    return;
                }
                // Also check the actual username field within the data if it differs from the key
                if (usersData[key].contains("username")) {
                    QString existingUsernameInData = QString::fromStdString(usersData[key].value("username", ""));
                    if (existingUsernameInData.compare(newUsername, Qt::CaseInsensitive) == 0) {
                        QMessageBox::warning(this, tr("Error"), QString(tr("Username '%1' already exists.").arg(newUsername)));
                        return;
                    }
                }
            }
        }


        // --- Collect Data ---
        json privileges;
        privileges["viewDatabase"] = true;
        privileges["addProductDeleteProduct"] = addProductDeleteProductCheckbox->isChecked();
        json columnPrivileges;
        columnPrivileges["editProductName"] = editProductNameCheckbox->isChecked();
        columnPrivileges["editBuyPrice"] = editBuyPriceCheckbox->isChecked();
        columnPrivileges["editSellPrice"] = editSellPriceCheckbox->isChecked();
        columnPrivileges["editQuantity"] = editQuantityCheckbox->isChecked();
        columnPrivileges["editUnitsSold"] = editUnitsSoldCheckbox->isChecked();
        columnPrivileges["editSupplier"] = editSupplierCheckbox->isChecked();
        columnPrivileges["editDescription"] = editDescriptionCheckbox->isChecked();
        privileges["columnPrivileges"] = columnPrivileges;

        json updatedUserData;
        updatedUserData["username"] = newUsername.toStdString(); // Use the potentially new username
        updatedUserData["password"] = password.toStdString();
        updatedUserData["privileges"] = privileges;
        updatedUserData["privileges"]["categories"] = selectedCategories;

        // --- Firebase Operation ---
        try {
            if (!usernameChanged) {
                // --- Case 1: Username NOT changed - Simple Update ---
                std::string path = "companies/" + companyName + "/users/" + oldUsernameKey.toStdString();
                path = db.urlEncode(path);
                db.writeData(path, updatedUserData);
                usersData[oldUsernameKey] = updatedUserData; // Update cache
                QMessageBox::information(this, "Success", QString(tr("User '%1' updated successfully!").arg(oldUsernameKey)));

            } else {
                // --- Case 2: Username CHANGED - Add New then Delete Old ---
                std::string newPath = "companies/" + companyName + "/users/" + newUsername.toStdString();
                newPath = db.urlEncode(newPath);
                std::string oldPath = "companies/" + companyName + "/users/" + oldUsernameKey.toStdString();
                oldPath = db.urlEncode(oldPath);

                // 1. Write data to the new path
                db.writeData(newPath, updatedUserData);

                // 2. If write successful, delete data from the old path
                bool deleteSuccess = false;
                std::string deleteErrorMsg;
                try {
                    // Assuming deleteData returns bool or throws on failure
                    // Adjust based on your firebaselib implementation
                    db.deleteData(oldPath); // Assuming this function exists and works
                    deleteSuccess = true;
                } catch (const std::exception& del_e) {
                    deleteErrorMsg = del_e.what();
                    deleteSuccess = false;
                }

                // 3. Update UI and Cache
                usersData.remove(oldUsernameKey);       // Remove old entry from cache
                usersData[newUsername] = updatedUserData; // Add new entry to cache

                // Update ComboBox
                int oldIndex = userComboBox->findData(oldUsernameKey);
                if (oldIndex != -1) {
                    userComboBox->removeItem(oldIndex);
                }
                userComboBox->addItem(newUsername, newUsername); // Add new display text and key data
                int newIndex = userComboBox->findData(newUsername);
                if (newIndex != -1) {
                    userComboBox->setCurrentIndex(newIndex); // Select the newly added item
                }

                currentUsernameKey = newUsername; // Update the tracked key

                // 4. Report outcome
                if (deleteSuccess) {
                    QMessageBox::information(this, "Success", QString(tr("User '%1' successfully renamed to '%2'!").arg(oldUsernameKey).arg(newUsername)));
                } else {
                    QMessageBox::warning(this, "Partial Success", QString(tr("User renamed to '%1', but failed to remove old entry '%2'. Manual cleanup may be required. Error: %3")
                                             .arg(newUsername).arg(oldUsernameKey).arg(QString::fromStdString(deleteErrorMsg))));
                }
            }

        } catch (const std::exception& e) {
            QMessageBox::critical(this, tr("Error"), QString(tr("Failed to save user data: %1").arg(e.what())));
        }
    }

    void onDeleteUserClicked() {
        if (currentUsernameKey.isEmpty()) {
            QMessageBox::warning(this, tr("Delete Error"), tr("No user selected to delete."));
            return;
        }

        // --- Confirmation Dialog ---
        QMessageBox::StandardButton reply;
        reply = QMessageBox::warning(this, tr("Confirm Delete"),
                                     QString(tr("Are you sure you want to permanently delete user '%1'?").arg(currentUsernameKey)),
                                     QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::No) {
            return; // User cancelled
        }

        // --- Proceed with Deletion ---
        try {
            std::string path = "companies/" + companyName + "/users/" + currentUsernameKey.toStdString();
            path = db.urlEncode(path);

            // Call Firebase delete function (ensure db.deleteData exists in your firebaselib)
            db.deleteData(path);

            // --- Update UI and Cache on Success ---
            usersData.remove(currentUsernameKey); // Remove from cache

            // Remove from ComboBox
            int indexToRemove = userComboBox->findData(currentUsernameKey);
            if (indexToRemove != -1) {
                userComboBox->removeItem(indexToRemove);
            }

            QMessageBox::information(this, tr("Success"), QString(tr("User deleted successfully.")));

            // Reset form
            clearForm();
            setFormEnabled(false);
            currentUsernameKey = ""; // Clear the selected user key
            userComboBox->setCurrentIndex(0); // Reset dropdown to placeholder


        } catch (const std::exception& e) {
            QMessageBox::critical(this, tr("Deletion Failed"), QString(tr("Failed to delete user '%1': %2")
                                      .arg(currentUsernameKey).arg(e.what())));
            // Keep the user selected in case deletion failed and user wants to retry or save changes
        }
    }


private:
    void setupUI() {
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        // Title
        QLabel* titleLabel = new QLabel(tr("Edit Existing User"));
        QFont titleFont = titleLabel->font();
        titleFont.setBold(true); titleFont.setPointSize(14);
        titleLabel->setFont(titleFont);
        mainLayout->addWidget(titleLabel);

        // User Selection
        QHBoxLayout* userSelectionLayout = new QHBoxLayout();
        QLabel* userSelectLabel = new QLabel(tr("Select User:"));
        userComboBox = new QComboBox();
        userSelectionLayout->addWidget(userSelectLabel);
        userSelectionLayout->addWidget(userComboBox);
        mainLayout->addLayout(userSelectionLayout);

        // Username
        QHBoxLayout* usernameLayout = new QHBoxLayout();
        QLabel* usernameLabel = new QLabel(tr("Username:"));
        usernameEdit = new QLineEdit();
        usernameLayout->addWidget(usernameLabel);
        usernameLayout->addWidget(usernameEdit);
        mainLayout->addLayout(usernameLayout);

        // Password
        QHBoxLayout* passwordLayout = new QHBoxLayout();
        QLabel* passwordLabel = new QLabel(tr("Password:"));
        passwordEdit = new QLineEdit();
        passwordEdit->setToolTip(tr("Password will be stored as entered. Minimum 8 characters.")); // Add tooltip
        passwordLayout->addWidget(passwordLabel);
        passwordLayout->addWidget(passwordEdit);
        mainLayout->addLayout(passwordLayout);

        // Privileges Group Box
        privilegesGroup = new QGroupBox(tr("User Privileges"));
        QVBoxLayout* privilegesLayout = new QVBoxLayout();
        viewDatabaseCheckbox = new QCheckBox(tr("View Database"));
        viewDatabaseCheckbox->setChecked(true); viewDatabaseCheckbox->setEnabled(false);
        privilegesLayout->addWidget(viewDatabaseCheckbox);
        addProductDeleteProductCheckbox = new QCheckBox(tr("Add/Delete Products"));
        privilegesLayout->addWidget(addProductDeleteProductCheckbox);
        QGroupBox* columnPrivilegesGroup = new QGroupBox(tr("Column Edit Privileges"));
        QVBoxLayout* columnPrivilegesLayout = new QVBoxLayout();
        editProductNameCheckbox = new QCheckBox(tr("Edit Product Name")); columnPrivilegesLayout->addWidget(editProductNameCheckbox);
        editBuyPriceCheckbox = new QCheckBox(tr("Edit Buy Price")); columnPrivilegesLayout->addWidget(editBuyPriceCheckbox);
        editSellPriceCheckbox = new QCheckBox(tr("Edit Sell Price")); columnPrivilegesLayout->addWidget(editSellPriceCheckbox);
        editQuantityCheckbox = new QCheckBox(tr("Edit Quantity")); columnPrivilegesLayout->addWidget(editQuantityCheckbox);
        editUnitsSoldCheckbox = new QCheckBox(tr("Edit Units Sold")); columnPrivilegesLayout->addWidget(editUnitsSoldCheckbox);
        editSupplierCheckbox = new QCheckBox(tr("Edit Supplier")); columnPrivilegesLayout->addWidget(editSupplierCheckbox);
        editDescriptionCheckbox = new QCheckBox(tr("Edit Description")); columnPrivilegesLayout->addWidget(editDescriptionCheckbox);
        columnPrivilegesGroup->setLayout(columnPrivilegesLayout);
        privilegesLayout->addWidget(columnPrivilegesGroup);
        privilegesGroup->setLayout(privilegesLayout);
        mainLayout->addWidget(privilegesGroup);

        // --- Buttons Layout ---
        // Create a horizontal layout for the buttons
        QHBoxLayout* buttonsLayout = new QHBoxLayout();

        // Save Changes Button
        saveChangesButton = new QPushButton(tr("Save Changes"));
        buttonsLayout->addWidget(saveChangesButton); // Add save button to horizontal layout

        // Delete User Button
        deleteUserButton = new QPushButton(tr("Delete User"));
        buttonsLayout->addWidget(deleteUserButton); // Add delete button to horizontal layout

        // Add the buttons layout to the main vertical layout
        mainLayout->addLayout(buttonsLayout);

        mainLayout->addStretch();

        // Categories Group Box
        categoriesGroup = new QGroupBox(tr("Accessible Categories"));
        categoriesLayout = new QVBoxLayout();
        scrollArea = new QScrollArea();
        scrollContent = new QWidget();
        scrollLayout = new QVBoxLayout(scrollContent);
        categoryCheckboxes.clear();
        populateCategories();
        scrollArea->setWidget(scrollContent);
        scrollArea->setWidgetResizable(true);
        categoriesLayout->addWidget(scrollArea);
        categoriesGroup->setLayout(categoriesLayout);
        mainLayout->addWidget(categoriesGroup);

        // Connections
        connect(userComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &EditUserScreen::onUserSelected);
        connect(deleteUserButton, &QPushButton::clicked, this, &EditUserScreen::onDeleteUserClicked);
        connect(saveChangesButton, &QPushButton::clicked, this, &EditUserScreen::onSaveUserClicked);

        setFormEnabled(false); // Initially disable form
    }

    void loadUsers() {
        userComboBox->clear();
        usersData.clear();
        userComboBox->addItem(tr("-- Select User --"), ""); // Placeholder with empty data key

        try {
            std::string path = "companies/" + companyName + "/users";
            path = db.urlEncode(path);
            json allUsersData = db.readData(path);

            if (!allUsersData.empty()) {
                for (auto& [usernameKey, userData] : allUsersData.items()) {
                    // Admin Exclusion
                    if (!userData.contains("privileges")) {
                        continue; // Skip admin user
                    }

                    QString qUsernameKey = QString::fromStdString(usernameKey);
                    // Use username from data for display, but store the key for selection
                    QString displayName = qUsernameKey;
                    if (userData.contains("username") && userData["username"].is_string()) {
                        displayName = QString::fromStdString(userData["username"].get<std::string>());
                    }

                    userComboBox->addItem(displayName, qUsernameKey); // Display name, Store key as data
                    usersData[qUsernameKey] = userData; // Store data by key
                }
            }
        } catch (const std::exception& e) {
            QMessageBox::critical(this, tr("Error"), QString(tr("Failed to load users: %1").arg(e.what())));
        }
        // Ensure form is disabled if loading fails or no users (except admin) exist
        if (userComboBox->count() <= 1) { // Only placeholder exists
            setFormEnabled(false);
        }
    }

    void populateForm(const json& userData) {
        usernameEdit->setText(QString::fromStdString(userData.value("username", "")));
        passwordEdit->setText(QString::fromStdString(userData.value("password", "")));

        if (userData.contains("privileges") && userData["privileges"].is_object()) {
            const json& privileges = userData["privileges"];
            addProductDeleteProductCheckbox->setChecked(privileges.value("addProductDeleteProduct", false));

            if (privileges.contains("columnPrivileges") && privileges["columnPrivileges"].is_object()) {
                const json& colPrivs = privileges["columnPrivileges"];
                editProductNameCheckbox->setChecked(colPrivs.value("editProductName", false));
                editBuyPriceCheckbox->setChecked(colPrivs.value("editBuyPrice", false));
                editSellPriceCheckbox->setChecked(colPrivs.value("editSellPrice", false));
                editQuantityCheckbox->setChecked(colPrivs.value("editQuantity", false));
                editUnitsSoldCheckbox->setChecked(colPrivs.value("editUnitsSold", false));
                editSupplierCheckbox->setChecked(colPrivs.value("editSupplier", false));
                editDescriptionCheckbox->setChecked(colPrivs.value("editDescription", false));
            } else {
                // Reset column privileges if missing
                editProductNameCheckbox->setChecked(false); editBuyPriceCheckbox->setChecked(false);
                editSellPriceCheckbox->setChecked(false); editQuantityCheckbox->setChecked(false);
                editUnitsSoldCheckbox->setChecked(false); editSupplierCheckbox->setChecked(false);
                editDescriptionCheckbox->setChecked(false);
            }

            std::set<std::string> userCategorySet;
            if (privileges.contains("categories") && privileges["categories"].is_array()) {
                for(const auto& cat : privileges["categories"]) {
                    if (cat.is_string()) {
                        userCategorySet.insert(cat.get<std::string>());
                    }
                }
            }
            for (auto* checkbox : categoryCheckboxes) {
                checkbox->setChecked(userCategorySet.count(checkbox->text().toStdString()) > 0);
            }
        } else {
            // Reset all privileges if structure missing
            addProductDeleteProductCheckbox->setChecked(false); editProductNameCheckbox->setChecked(false);
            editBuyPriceCheckbox->setChecked(false); editSellPriceCheckbox->setChecked(false);
            editQuantityCheckbox->setChecked(false); editUnitsSoldCheckbox->setChecked(false);
            editSupplierCheckbox->setChecked(false); editDescriptionCheckbox->setChecked(false);
            for (auto* checkbox : categoryCheckboxes) { checkbox->setChecked(false); }
        }
    }

    void clearForm() {
        usernameEdit->clear();
        passwordEdit->clear();
        addProductDeleteProductCheckbox->setChecked(false);
        editProductNameCheckbox->setChecked(false); editBuyPriceCheckbox->setChecked(false);
        editSellPriceCheckbox->setChecked(false); editQuantityCheckbox->setChecked(false);
        editUnitsSoldCheckbox->setChecked(false); editSupplierCheckbox->setChecked(false);
        editDescriptionCheckbox->setChecked(false);
        for (auto* checkbox : categoryCheckboxes) { checkbox->setChecked(false); }
    }

    void setFormEnabled(bool enabled) {
        usernameEdit->setEnabled(enabled); // Now enable/disable username field too
        passwordEdit->setEnabled(enabled);
        privilegesGroup->setEnabled(enabled);
        categoriesGroup->setEnabled(enabled);
        saveChangesButton->setEnabled(enabled);
        deleteUserButton->setEnabled(enabled);
        viewDatabaseCheckbox->setEnabled(false); // Keep this one always disabled
    }

    // Member variables
    FirebaseDB& db;
    json companyData;
    std::string companyName;
    QMap<QString, json> usersData; // Cache: Key (from Firebase) -> User Data JSON
    QString currentUsernameKey = ""; // Firebase Key of the user currently selected/being edited

    // UI Elements
    QComboBox* userComboBox;
    QLineEdit* usernameEdit;
    QLineEdit* passwordEdit;
    QGroupBox* privilegesGroup;
    QCheckBox* viewDatabaseCheckbox;
    QCheckBox* addProductDeleteProductCheckbox;
    QCheckBox* editProductNameCheckbox; QCheckBox* editBuyPriceCheckbox;
    QCheckBox* editSellPriceCheckbox; QCheckBox* editQuantityCheckbox;
    QCheckBox* editUnitsSoldCheckbox; QCheckBox* editSupplierCheckbox;
    QCheckBox* editDescriptionCheckbox;
    QGroupBox* categoriesGroup;
    QList<QCheckBox*> categoryCheckboxes;
    QPushButton* saveChangesButton;
    QPushButton* deleteUserButton;


    QVBoxLayout* categoriesLayout;
    QScrollArea* scrollArea;
    QWidget* scrollContent;
    QVBoxLayout* scrollLayout;
};

#endif // EDITUSERSCREEN_HPP
