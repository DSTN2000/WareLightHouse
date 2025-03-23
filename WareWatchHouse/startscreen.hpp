#ifndef STARTSCREEN_HPP
#define STARTSCREEN_HPP

#include "mainwindow.h"

// Start screen with a single button
class StartScreen : public QWidget {
    Q_OBJECT

public:
    StartScreen(QWidget *parent = nullptr) : QWidget(parent) {
        // Create layout
        QVBoxLayout *layout = new QVBoxLayout(this);

        // Create a welcome label
        QLabel *welcomeLabel = new QLabel("Welcome to Multi-Page Qt Application", this);
        welcomeLabel->setAlignment(Qt::AlignCenter);
        QFont font = welcomeLabel->font();
        font.setPointSize(20);
        welcomeLabel->setFont(font);

        // Create the register button
        registerButton = new QPushButton("Register Your Company", this);
        registerButton->setMinimumSize(120, 50);

        // Create the login button
        loginButton = new QPushButton("Login", this);
        loginButton->setMinimumSize(120, 50);

        // Add widgets to layout with stretches for centering
        layout->addStretch(1);
        layout->addWidget(welcomeLabel);
        layout->addSpacing(30);
        layout->addWidget(registerButton, 0, Qt::AlignCenter);
        layout->addSpacing(15);
        layout->addWidget(loginButton, 0, Qt::AlignCenter);
        layout->addStretch(1);

        // Connect the button clicks
        connect(registerButton, &QPushButton::clicked, this, &StartScreen::registerButtonClicked);
        connect(loginButton, &QPushButton::clicked, this, &StartScreen::loginButtonClicked);
    }

signals:
    // Signals emitted when buttons are clicked
    void registerButtonClicked();
    void loginButtonClicked();

private:
    QPushButton *registerButton;
    QPushButton *loginButton;
};



#endif // STARTSCREEN_HPP
