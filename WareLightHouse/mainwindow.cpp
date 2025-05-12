#include "dashboard.hpp"
#include "startscreen.hpp"
#include "loginscreen.hpp"
#include "registerscreen.hpp"
#include "firebaselib.hpp"

std::string database_url = "https://test-b0c55-default-rtdb.europe-west1.firebasedatabase.app";
FirebaseDB db(database_url);

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    // #----------------- SET WINDOW PROPERTIES -----------------#
    srand(time(0));
    setWindowTitle("WareLightHouse");
    setWindowIcon(QIcon(":/icon.ico"));
    resize(800, 600);

    // Create the stacked widget to manage different pages
    stackedWidget = new QStackedWidget(this);

    // Create the screens
    startScreen = new StartScreen(this);
    loginScreen = new LoginScreen(this);
    registerScreen = new RegisterScreen(this);

    // Add screens to stacked widget
    stackedWidget->addWidget(startScreen);
    stackedWidget->addWidget(loginScreen);
    stackedWidget->addWidget(registerScreen);

    // Set the stacked widget as the central widget
    setCentralWidget(stackedWidget);

    translatorEn = new QTranslator(this);
    translatorRu = new QTranslator(this);
    translatorEn->load(":/translations/WareLightHouse_en.qm");
    translatorRu = new QTranslator(this);
    if (translatorRu->load(":/translations/WareLightHouse_ru.qm")) { // AND HERE
        qDebug() << "Russian translations loaded successfully.";
    } else {
        qDebug() << "Failed to load Russian translations. Ensure ru.qm is in the resources.";
    }
    //translatorRu->load(":/translations/WareLightHouse_ru.qm");


    // ----------------- CONNECT SIGNALS AND SLOTS -----------------#
    connect(startScreen, &StartScreen::registerButtonClicked, this, &MainWindow::showRegisterScreen);
    connect(startScreen, &StartScreen::loginButtonClicked, this, &MainWindow::showLoginScreen);

    connect(loginScreen, &LoginScreen::loginSuccessful, this, &MainWindow::showDashboard);
    connect(loginScreen, &LoginScreen::registerRequested, this, &MainWindow::showRegisterScreen);

    connect(registerScreen, &RegisterScreen::backToLoginRequested, this, &MainWindow::showLoginScreen);
    connect(registerScreen, &RegisterScreen::registrationSuccessful, this, &MainWindow::showDashboard);

    // Connect the language combobox
    connect(startScreen->languageComboBox, QOverload<const QString &>::of(&QComboBox::currentTextChanged), this, &MainWindow::changeLanguage);

}

MainWindow::~MainWindow()
{

}

void MainWindow::showDashboard()
{
    std::string company, username, password;
    if (stackedWidget->currentWidget()==registerScreen)
    {
        registerScreen->getData(company, username, password);
        if (db.addUser(company, username, password))
        {
            QMessageBox::information(this, tr("Success"), tr("Registration successful"));
        }
        else
        {
            QMessageBox::critical(this, tr("Error"), tr("Registration failed. Company already exists."));
            return;
        }
        dashboard = new Dashboard(this, &db, db.readData("companies")[company], company, username);
        stackedWidget->addWidget(dashboard);
    }
    else
    {
        loginScreen->getData(company, username, password);
        if (db.authenticateUser(company, username, password))
        {
            QMessageBox::information(this, tr("Success"), tr("Login successful"));
        }
        else
        {
            QMessageBox::critical(this, tr("Error"), tr("Invalid credentials"));
            return;
        }
        dashboard = new Dashboard(this, &db, db.readData("companies")[company], company, username);
        stackedWidget->addWidget(dashboard);
    }
    stackedWidget->setCurrentWidget(dashboard);
    delete loginScreen;
    delete registerScreen;
    delete startScreen;
}

void MainWindow::showLoginScreen()
{
    stackedWidget->setCurrentWidget(loginScreen);
}

void MainWindow::showRegisterScreen()
{
    stackedWidget->setCurrentWidget(registerScreen);
}


void MainWindow::showPage(int pageIndex)
{
    dashboard->setCurrentPage(pageIndex);
}

void MainWindow::changeLanguage(const QString &language)
{
    // Remove any previous translator
    qApp->removeTranslator(translatorEn);
    qApp->removeTranslator(translatorRu);

    if (language != "English") {

        qApp->installTranslator(translatorRu);
    } else {
        // Default to English
        qApp->installTranslator(translatorEn);
    }

    MainWindow::refreshScreens(language);

}


void MainWindow::refreshScreens(QString language)
{

    if (startScreen) {
        stackedWidget->removeWidget(startScreen);
        startScreen->deleteLater();
        startScreen = nullptr;
    }

    if (loginScreen) {
        stackedWidget->removeWidget(loginScreen);
        loginScreen->deleteLater();
        loginScreen = nullptr;
    }

    if (registerScreen) {
        stackedWidget->removeWidget(registerScreen);
        registerScreen->deleteLater();
        registerScreen = nullptr;
    }

    // Recreate screens
    startScreen = new StartScreen(this, language);
    loginScreen = new LoginScreen(this);
    registerScreen = new RegisterScreen(this);

    // Re-add screens to stacked widget
    stackedWidget->addWidget(startScreen);
    stackedWidget->addWidget(loginScreen);
    stackedWidget->addWidget(registerScreen);
    stackedWidget->setCurrentWidget(startScreen);

    // Reconnect signals and slots
    connect(startScreen, &StartScreen::registerButtonClicked, this, &MainWindow::showRegisterScreen);
    connect(startScreen, &StartScreen::loginButtonClicked, this, &MainWindow::showLoginScreen);

    connect(loginScreen, &LoginScreen::loginSuccessful, this, &MainWindow::showDashboard);
    connect(loginScreen, &LoginScreen::registerRequested, this, &MainWindow::showRegisterScreen);

    connect(registerScreen, &RegisterScreen::backToLoginRequested, this, &MainWindow::showLoginScreen);
    connect(registerScreen, &RegisterScreen::registrationSuccessful, this, &MainWindow::showDashboard);

    connect(startScreen->languageComboBox, QOverload<const QString &>::of(&QComboBox::currentTextChanged), this, &MainWindow::changeLanguage);
}

