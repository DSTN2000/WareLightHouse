#ifndef STARTSCREEN_HPP
#define STARTSCREEN_HPP

#include "mainwindow.h"

// Start screen with a single button
class StartScreen : public QWidget {
    Q_OBJECT

public:
    StartScreen(QWidget *parent = nullptr, QString language = "English") : QWidget(parent) {
        // Create layout
        QVBoxLayout *layout = new QVBoxLayout(this);

        // Add Logo
        QLabel *logoLabel = new QLabel(this);
        QPixmap logoPixmap(":/images/logo.png");
        if (!logoPixmap.isNull()) {
            // Scale the logo to a reasonable size
            QPixmap scaledLogo = logoPixmap.scaled(500, 500, Qt::KeepAspectRatio);
            logoLabel->setPixmap(scaledLogo);
            logoLabel->setAlignment(Qt::AlignCenter);
        } else {
            qDebug() << "Error: Could not load logo image.";
        }

        // Create a welcome label
        QLabel *welcomeLabel = new QLabel(tr("Welcome to WareLightHouse"), this);
        welcomeLabel->setAlignment(Qt::AlignCenter);
        QFont font = welcomeLabel->font();
        font.setPointSize(20);
        welcomeLabel->setFont(font);

        // Add language selector
        languageComboBox = new QComboBox();
        if (language == "English")
        {
            languageComboBox->addItem("English");
            languageComboBox->addItem("Русский");
        }
        else
        {
            languageComboBox->addItem("Русский");
            languageComboBox->addItem("English");
        }

        // Create the register button
        registerButton = new QPushButton(tr("Register Your Company"), this);
        registerButton->setMinimumSize(120, 50);

        // Create the login button
        loginButton = new QPushButton(tr("Login"), this);
        loginButton->setMinimumSize(120, 50);

        // Add widgets to layout
        layout->addWidget(languageComboBox);

        layout->addWidget(logoLabel);
        //layout->addSpacing(20);
        layout->addWidget(welcomeLabel);
        //layout->addSpacing(30);
        layout->addWidget(registerButton);
        //layout->addSpacing(15);
        layout->addWidget(loginButton);

        setLayout(layout);

        // Connect the button clicks
        connect(registerButton, &QPushButton::clicked, this, &StartScreen::registerButtonClicked);
        connect(loginButton, &QPushButton::clicked, this, &StartScreen::loginButtonClicked);
    }
    QComboBox *languageComboBox;
signals:
    // Signals emitted when buttons are clicked
    void registerButtonClicked();
    void loginButtonClicked();

private:
    QPushButton *registerButton;
    QPushButton *loginButton;
};



#endif // STARTSCREEN_HPP
