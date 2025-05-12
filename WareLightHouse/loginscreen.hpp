#ifndef LOGINSCREEN_HPP
#define LOGINSCREEN_HPP


#include "mainwindow.h"


class LoginScreen : public QWidget {
    Q_OBJECT

public:

    LoginScreen(QWidget *parent = nullptr) : QWidget(parent) {
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

    void loginSuccessful(const QString &company, const QString &username);
    void registerRequested();

private slots:
    void attemptLogin() {
        company = companyLineEdit->text().trimmed();
        username = usernameLineEdit->text().trimmed();
        password = passwordLineEdit->text().trimmed();

        // Basic validation
        if (company.isEmpty() || username.isEmpty() || password.isEmpty()) {
            QMessageBox::warning(this, tr("Login Error"), tr("All fields are required"));
            return;
        }

        emit loginSuccessful(company, username);


    }

    void openRegisterScreen() {
        emit registerRequested();
    }

private:
    // UI Elements
    QLabel *titleLabel;
    QLabel *companyLabel;
    QLabel *usernameLabel;
    QLabel *passwordLabel;
    QLineEdit *companyLineEdit;
    QLineEdit *usernameLineEdit;
    QLineEdit *passwordLineEdit;
    QPushButton *loginButton;
    QPushButton *registerButton;
    QString company;
    QString username;
    QString password;

    void setupUI() {
        // Create main layout
        QVBoxLayout *mainLayout = new QVBoxLayout(this);
        mainLayout->setSpacing(15);
        mainLayout->setContentsMargins(30, 30, 30, 30);

        // Create title label
        titleLabel = new QLabel(tr("Login"), this);
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
        companyLabel = new QLabel(tr("Company Name:"), this);
        usernameLabel = new QLabel(tr("Username/Role:"), this);
        passwordLabel = new QLabel(tr("Password:"), this);

        // Create input fields
        companyLineEdit = new QLineEdit(this);
        companyLineEdit->setPlaceholderText(tr("Enter company name"));

        usernameLineEdit = new QLineEdit(this);
        usernameLineEdit->setPlaceholderText(tr("Enter username or role"));

        passwordLineEdit = new QLineEdit(this);
        passwordLineEdit->setPlaceholderText(tr("Enter password"));
        passwordLineEdit->setEchoMode(QLineEdit::Password);

        // Add form elements to grid layout
        formLayout->addWidget(companyLabel, 0, 0);
        formLayout->addWidget(companyLineEdit, 0, 1);
        formLayout->addWidget(usernameLabel, 1, 0);
        formLayout->addWidget(usernameLineEdit, 1, 1);
        formLayout->addWidget(passwordLabel, 2, 0);
        formLayout->addWidget(passwordLineEdit, 2, 1);

        // Create buttons
        loginButton = new QPushButton(tr("Login"), this);
        loginButton->setMinimumHeight(40);
        loginButton->setCursor(Qt::PointingHandCursor);

        registerButton = new QPushButton(tr("Back to Register"), this);
        registerButton->setMinimumHeight(40);
        registerButton->setCursor(Qt::PointingHandCursor);

        // Create button layout
        QHBoxLayout *buttonLayout = new QHBoxLayout();
        buttonLayout->addWidget(registerButton);
        buttonLayout->addWidget(loginButton);

        // Add all elements to main layout
        mainLayout->addWidget(titleLabel);
        mainLayout->addSpacing(20);
        mainLayout->addLayout(formLayout);
        mainLayout->addSpacing(20);
        mainLayout->addLayout(buttonLayout);

    }

    void connectSignals() {
        connect(loginButton, &QPushButton::clicked, this, &LoginScreen::attemptLogin);
        connect(registerButton, &QPushButton::clicked, this, &LoginScreen::openRegisterScreen);
        connect(passwordLineEdit, &QLineEdit::returnPressed, this, &LoginScreen::attemptLogin);
    }

};

#endif // LOGINSCREEN_HPP
