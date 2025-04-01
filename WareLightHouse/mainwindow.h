#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <set>
#include <algorithm>
#include <vector>
#include <QMainWindow>
#include <QApplication>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QWidget>
#include <QLineEdit>
#include <QMessageBox>
#include <QTableView>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QTextEdit>
#include <QDialogButtonBox>
#include <QStyledItemDelegate>
#include <QDoubleValidator>
#include <QIntValidator>
#include <QRegularExpressionValidator>
#include <QInputDialog>
#include <QListWidget>
#include <QFormLayout>
#include <QGroupBox>
#include <QScrollArea>
#include <QCheckBox>
#include <QToolTip>
#include <QPainter>
#include <QMap>
#include <QHelpEvent>
#include <QDateTime>
#include <QScrollBar>
#include <QStandardPaths>

// Forward declarations
class QStackedWidget;
class StartScreen;
class Dashboard;
class LoginScreen;
class RegisterScreen;
class FirebaseDB;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // Show the dashboard page
    void showDashboard();

    // Show the login screen
    void showLoginScreen();

    // Show the registartion screen
    void showRegisterScreen();

    // Show a specific page in the dashboard
    void showPage(int pageIndex);

private:
    QStackedWidget *stackedWidget;
    StartScreen *startScreen;
    Dashboard *dashboard;
    LoginScreen *loginScreen;
    RegisterScreen *registerScreen;
};

#endif // MAINWINDOW_H
