#ifndef DASHBOARD_HPP
#define DASHBOARD_HPP

#include "mainwindow.h"
#include "firebaselib.hpp"
#include "viewscreen.hpp"

// Dashboard with navigation bar and content pages
class Dashboard : public QWidget {
    Q_OBJECT

public:
    // TODO: add priviledges to the constructor
    Dashboard(QWidget *parent = nullptr, FirebaseDB *db=nullptr, json stockData={}, std::string companyName="") : QWidget(parent) {
        // Create main layout
        QVBoxLayout *mainLayout = new QVBoxLayout(this);

        // Create and set up the navigation bar
        QWidget *navbar = new QWidget(this);
        QHBoxLayout *navLayout = new QHBoxLayout(navbar);

        // Create navigation buttons
        homeButton = new QPushButton("Home", navbar);
        profileButton = new QPushButton("Profile", navbar);
        settingsButton = new QPushButton("Settings", navbar);
        aboutButton = new QPushButton("About", navbar);

        // Add buttons to navigation layout
        navLayout->addWidget(homeButton);
        navLayout->addWidget(profileButton);
        navLayout->addWidget(settingsButton);
        navLayout->addWidget(aboutButton);

        // Create stacked widget for different pages
        contentStack = new QStackedWidget(this);

        // Create page content
        //QWidget *homePage = createPage("Home Page", "This is the home page content.");
        homePage = new ViewScreen(db, stockData,companyName);
        QWidget *profilePage = createPage("Profile Page", "User profile information would be displayed here.");
        QWidget *settingsPage = createPage("Settings Page", "Application settings would go here.");
        QWidget *aboutPage = createPage("About Page", "Information about the application.");

        // Add pages to stacked widget
        contentStack->addWidget(homePage);
        contentStack->addWidget(profilePage);
        contentStack->addWidget(settingsPage);
        contentStack->addWidget(aboutPage);

        // Add widgets to main layout
        mainLayout->addWidget(navbar);
        mainLayout->addWidget(contentStack, 1);

        // Connect navigation buttons
        connect(homeButton, &QPushButton::clicked, this, &Dashboard::showDB);
        connect(profileButton, &QPushButton::clicked, this, [this](){ setCurrentPage(1); });
        connect(settingsButton, &QPushButton::clicked, this, [this](){ setCurrentPage(2); });
        connect(aboutButton, &QPushButton::clicked, this, [this](){ setCurrentPage(3); });
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

    void showDB()
    {
        contentStack->setCurrentWidget(homePage);
    }

    QPushButton *homeButton;
    QPushButton *profileButton;
    QPushButton *settingsButton;
    QPushButton *aboutButton;
    QStackedWidget *contentStack;
    ViewScreen *homePage;
};



#endif // DASHBOARD_HPP
