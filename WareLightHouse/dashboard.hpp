#ifndef DASHBOARD_HPP
#define DASHBOARD_HPP

#include "mainwindow.h"
#include "firebaselib.hpp"
#include "viewscreen.hpp"
#include "adduserscreen.hpp"
#include "edituserscreen.hpp"
#include "messageboard.hpp"

// Dashboard with navigation bar
class Dashboard : public QWidget {
    Q_OBJECT

public:
    Dashboard(QWidget *parent = nullptr, FirebaseDB *db=nullptr, json stockData={}, std::string companyName="", std::string username="") :
        QWidget(parent), db(db), companyName(companyName), username(username), stockData(stockData) {    // If privileges are not specified, the user is admin
        privileges=getPrivileges();
        bool isAdmin = privileges.empty();
        // Create main layout
        QVBoxLayout *mainLayout = new QVBoxLayout(this);

        // Create and set up the navigation bar
        QWidget *navbar = new QWidget(this);
        QHBoxLayout *navLayout = new QHBoxLayout(navbar);

        // Create navigation buttons
        stockButton = new QPushButton("View Stock", navbar);
        if (privileges.empty())
        {
            addUserButton = new QPushButton("Add Users", navbar);   // admin only
            editUserButton = new QPushButton("Edit Users", navbar);
        }
        messageBoardButton = new QPushButton("Messages", navbar);

        // Add buttons to navigation layout
        navLayout->addWidget(stockButton);
        if (isAdmin)
        {
            navLayout->addWidget(addUserButton);
            navLayout->addWidget(editUserButton);
        }
        navLayout->addWidget(messageBoardButton);

        // Create stacked widget for different pages
        contentStack = new QStackedWidget(this);

        // Create page content
        dbPage = new ViewScreen(db, stockData, companyName, privileges);
        if (isAdmin)
        {
            addUserPage = new AddUserScreen(db, stockData, companyName);
            editUserPage = new EditUserScreen(db, stockData, companyName);
        }
        messageBoardPage = new MessageBoard(db, companyName, username, isAdmin);
        //QWidget *messageBoardPage = createPage("About Page", "Information about the application.");

        // Add pages to stacked widget
        contentStack->addWidget(dbPage);
        if (isAdmin)
        {
            contentStack->addWidget(addUserPage);
            contentStack->addWidget(editUserPage);
        }
        contentStack->addWidget(messageBoardPage);

        // Add widgets to main layout
        mainLayout->addWidget(navbar);
        mainLayout->addWidget(contentStack, 1);

        // Connect navigation buttons
        connect(stockButton, &QPushButton::clicked, this, [this](){ setCurrentPage(0); });
        if (privileges.empty())
        {
            connect(addUserButton, &QPushButton::clicked, this, [this](){ setCurrentPage(1); addUserPage->populateCategories();});
            connect(editUserButton, &QPushButton::clicked, this, [this](){ setCurrentPage(2); editUserPage->populateCategories();});
            connect(messageBoardButton, &QPushButton::clicked, this, [this](){ setCurrentPage(3); });
        }
        else
        {
            connect(messageBoardButton, &QPushButton::clicked, this, [this](){ setCurrentPage(1); });
        }

    }

    // Set the currently visible page
    void setCurrentPage(int index) {
        contentStack->setCurrentIndex(index);
    }

private:
    // Helper method to create content pages
    QWidget* createPage(const QString &title, const QString &content) {
        QWidget *page = new QWidget(this);
        QVBoxLayout *layout = new QVBoxLayout(page);

        QLabel *titleLabel = new QLabel(title, page);
        QFont titleFont = titleLabel->font();
        titleFont.setPointSize(18);
        titleFont.setBold(true);
        titleLabel->setFont(titleFont);

        QLabel *contentLabel = new QLabel(content, page);
        contentLabel->setWordWrap(true);

        layout->addWidget(titleLabel, 0, Qt::AlignCenter);
        layout->addWidget(contentLabel, 0, Qt::AlignCenter);
        layout->addStretch(1);

        return page;
    }

    json getPrivileges()
    {
        std::string path = "companies/"+companyName+"/users/"+username+"/privileges/";
        path = std::regex_replace(path, std::regex(" "), "%20");
        return db->readData(path);
    }

    QPushButton *stockButton;
    QPushButton *addUserButton;
    QPushButton *editUserButton;
    QPushButton *messageBoardButton;
    QStackedWidget *contentStack;
    ViewScreen *dbPage;
    AddUserScreen *addUserPage;
    EditUserScreen *editUserPage;
    MessageBoard *messageBoardPage;
    json privileges;
    std::string companyName;
    std::string username;
    FirebaseDB *db;
    json stockData;
};



#endif // DASHBOARD_HPP
