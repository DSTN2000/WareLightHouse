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

    // ----------------- CONNECT SIGNALS AND SLOTS -----------------#
    connect(startScreen, &StartScreen::registerButtonClicked, this, &MainWindow::showRegisterScreen);
    connect(startScreen, &StartScreen::loginButtonClicked, this, &MainWindow::showLoginScreen);

    connect(loginScreen, &LoginScreen::loginSuccessful, this, &MainWindow::showDashboard);
    connect(loginScreen, &LoginScreen::registerRequested, this, &MainWindow::showRegisterScreen);

    connect(registerScreen, &RegisterScreen::backToLoginRequested, this, &MainWindow::showLoginScreen);
    connect(registerScreen, &RegisterScreen::registrationSuccessful, this, &MainWindow::showDashboard);
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
            QMessageBox::information(this, "Success", "Registration successful");
        }
        else
        {
            QMessageBox::critical(this, "Error", "Registration failed. Company already exists.");
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
            QMessageBox::information(this, "Success", "Login successful");
        }
        else
        {
            QMessageBox::critical(this, "Error", "Invalid credentials");
            return;
        }
        dashboard = new Dashboard(this, &db, db.readData("companies")[company], company, username);
        stackedWidget->addWidget(dashboard);
    }
    stackedWidget->setCurrentWidget(dashboard);
    delete loginScreen;
    delete registerScreen;
}

void MainWindow::showLoginScreen()
{
    stackedWidget->setCurrentWidget(loginScreen);
    delete startScreen;
}

void MainWindow::showRegisterScreen()
{
    stackedWidget->setCurrentWidget(registerScreen);
    delete startScreen;
}


void MainWindow::showPage(int pageIndex)
{
    dashboard->setCurrentPage(pageIndex);
}


