#ifndef REGISTERSCREEN_HPP
#define REGISTERSCREEN_HPP

#include <QClipboard>
#include "mainwindow.h"
#include "passwordgen.hpp"

class RegisterScreen : public QWidget {
    Q_OBJECT

public:
    RegisterScreen(QWidget *parent = nullptr) : QWidget(parent) {
        setupUI();
        connectSignals();
    }

    void getData(std::string& company_arg, std::string& username_arg, std::string& password_arg)
    {
        company_arg=company.toStdString();
        username_arg=username.toStdString();
        password_arg=password.toStdString();
    }

signals:
    void registrationSuccessful(const QString &company, const QString &username);
    void backToLoginRequested();

private slots:
    void attemptRegistration() {
        company = companyLineEdit->text();
        username = adminUsernameLineEdit->text();
        password = adminPasswordLineEdit->text();
        confirmPassword = confirmPasswordLineEdit->text();

        // Basic validation
        if (company.isEmpty() || username.isEmpty() || password.isEmpty()) {
            QMessageBox::warning(this, "Registration Error", "All fields are required");
            return;
        }

        if (password != confirmPassword) {
            QMessageBox::warning(this, "Registration Error", "Passwords do not match");
            return;
        }

        if (password.length() < 8) {
            QMessageBox::warning(this, "Registration Error", "Password must be at least 8 characters");
            return;
        }


        emit registrationSuccessful(company, username);


    }

    void returnToLogin() {
        emit backToLoginRequested();
    }

private:
    // UI Elements
    QLabel *titleLabel;
    QLabel *companyLabel;
    QLabel *adminUsernameLabel;
    QLabel *adminPasswordLabel;
    QLabel *confirmPasswordLabel;
    QLineEdit *companyLineEdit;
    QLineEdit *adminUsernameLineEdit;
    QLineEdit *adminPasswordLineEdit;
    QLineEdit *confirmPasswordLineEdit;
    QPushButton *registerButton;
    QPushButton *backButton;
    QPushButton *generatePasswordButton;
    QString company;
    QString username;
    QString password;
    QString confirmPassword;

    PasswordGenerator passwordGenerator;


    void setupUI() {
        // Create main layout
        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        mainLayout->setSpacing(15);
        mainLayout->setContentsMargins(30, 30, 30, 30);

        // Create title label
        titleLabel = new QLabel("Register your Company", this);
        QFont titleFont = titleLabel->font();
        titleFont.setPointSize(18);
        titleFont.setBold(true);
        titleLabel->setFont(titleFont);
        titleLabel->setAlignment(Qt::AlignCenter);

        // Create form layout
        QGridLayout *formLayout = new QGridLayout();
        formLayout->setHorizontalSpacing(15);
        formLayout->setVerticalSpacing(10);

        // Create labels
        companyLabel = new QLabel("Company Name:", this);
        adminUsernameLabel = new QLabel("Admin Username:", this);
        adminPasswordLabel = new QLabel("Admin Password:", this);
        confirmPasswordLabel = new QLabel("Confirm Password:", this);

        // Create input fields
        companyLineEdit = new QLineEdit(this);
        companyLineEdit->setPlaceholderText("Enter company name");

        adminUsernameLineEdit = new QLineEdit(this);
        adminUsernameLineEdit->setPlaceholderText("Enter admin username");

        adminPasswordLineEdit = new QLineEdit(this);
        adminPasswordLineEdit->setPlaceholderText("Enter password");
        adminPasswordLineEdit->setEchoMode(QLineEdit::Password);

        confirmPasswordLineEdit = new QLineEdit(this);
        confirmPasswordLineEdit->setPlaceholderText("Confirm password");
        confirmPasswordLineEdit->setEchoMode(QLineEdit::Password);

        generatePasswordButton = new QPushButton("Generate Password",this);
        generatePasswordButton->setFixedHeight(20);

        // Add form elements to grid layout
        formLayout->addWidget(companyLabel, 0, 0);
        formLayout->addWidget(companyLineEdit, 0, 1);
        formLayout->addWidget(adminUsernameLabel, 1, 0);
        formLayout->addWidget(adminUsernameLineEdit, 1, 1);
        formLayout->addWidget(adminPasswordLabel, 2, 0);
        formLayout->addWidget(adminPasswordLineEdit, 2, 1);
        formLayout->addWidget(confirmPasswordLabel, 3, 0);
        formLayout->addWidget(confirmPasswordLineEdit, 3, 1);
        formLayout->addWidget(generatePasswordButton, 4, 0, 1, 2);

        // Create buttons
        registerButton = new QPushButton("Register", this);
        registerButton->setFixedHeight(40);
        registerButton->setCursor(Qt::PointingHandCursor);

        backButton = new QPushButton("Back to Login", this);
        backButton->setFixedHeight(40);
        backButton->setCursor(Qt::PointingHandCursor);

        // Create button layout
        QHBoxLayout *buttonLayout = new QHBoxLayout();
        buttonLayout->addWidget(registerButton);
        buttonLayout->addWidget(backButton);

        // Add all elements to main layout
        mainLayout->addWidget(titleLabel);
        mainLayout->addSpacing(20);
        mainLayout->addLayout(formLayout);
        mainLayout->addSpacing(20);
        mainLayout->addLayout(buttonLayout);

        // Set window properties
        setWindowTitle("Register New Company");
        resize(450, 350);
    }


    void connectSignals() {
        connect(registerButton, &QPushButton::clicked, this, &RegisterScreen::attemptRegistration);
        connect(backButton, &QPushButton::clicked, this, &RegisterScreen::returnToLogin);
        connect(generatePasswordButton, &QPushButton::clicked, this, &RegisterScreen::generatePassword);
        connect(confirmPasswordLineEdit, &QLineEdit::returnPressed, this, &RegisterScreen::attemptRegistration);
    }

    void generatePassword() {
        QString password = QString::fromStdString(passwordGenerator.generatePassword()); // Generates a password and saves it as a QString as setText does not accept normal C++ strings
        adminPasswordLineEdit->setText(password);
        confirmPasswordLineEdit->setText(password);
        QApplication::clipboard()->setText(password);
    }


};

#endif // REGISTERSCREEN_HPP
