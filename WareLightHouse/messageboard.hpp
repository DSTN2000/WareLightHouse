#ifndef MESSAGEBOARD_HPP
#define MESSAGEBOARD_HPP


#include "mainwindow.h"

#include "firebaselib.hpp"

// Define constants for section keys
const std::string GENERAL_MESSAGES_KEY = "general";
const std::string ADMIN_ANNOUNCEMENTS_KEY = "announcements";

class MessageItemWidget : public QWidget {
    Q_OBJECT

public:
    explicit MessageItemWidget(const QString& author, const QString& timestamp, const QString& content,
                               const QString& messageKey, bool showDeleteButton, QWidget* parent = nullptr)
        : QWidget(parent), m_messageKey(messageKey)
    {
        // Labels for message info
        authorLabel = new QLabel(QString("<b>%1</b>").arg(author.toHtmlEscaped()));
        timestampLabel = new QLabel(QString("<small>%1</small>").arg(timestamp.toHtmlEscaped()));
        contentLabel = new QLabel(content.toHtmlEscaped().replace("\n", "<br/>"));
        contentLabel->setTextFormat(Qt::RichText);
        contentLabel->setWordWrap(true); // Allow content to wrap

        // Delete button (conditionally created and shown)
        deleteButton = new QPushButton(tr("Delete"));
        deleteButton->setFixedSize(60, 25); // Make button small
        deleteButton->setVisible(showDeleteButton);
        if (showDeleteButton) {
            connect(deleteButton, &QPushButton::clicked, this, &MessageItemWidget::onDeleteClicked);
        }

        // Layouts
        QHBoxLayout* topLayout = new QHBoxLayout();
        topLayout->addWidget(authorLabel);
        topLayout->addStretch();
        topLayout->addWidget(timestampLabel);

        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        mainLayout->addLayout(topLayout);
        mainLayout->addWidget(contentLabel);

        // Add delete button aligned to the right if visible
        if (showDeleteButton) {
            QHBoxLayout* bottomLayout = new QHBoxLayout();
            bottomLayout->addStretch();
            bottomLayout->addWidget(deleteButton);
            mainLayout->addLayout(bottomLayout);
        }
        // Add a line at the bottom for visual separation
        QFrame* line = new QFrame();
        line->setFrameShape(QFrame::HLine);
        line->setFrameShadow(QFrame::Sunken);
        mainLayout->addWidget(line);


        setLayout(mainLayout);
    }

    QString messageKey() const { return m_messageKey; }

signals:
    void deleteRequested(const QString& messageKey); // Signal to emit when delete is clicked

private slots:
    void onDeleteClicked() {
        emit deleteRequested(m_messageKey);
    }


private:
    QLabel* authorLabel;
    QLabel* timestampLabel;
    QLabel* contentLabel;
    QPushButton* deleteButton;
    QString m_messageKey; // Firebase key like "msg5"
};


class MessageBoard : public QWidget {
    Q_OBJECT

public:
    explicit MessageBoard(FirebaseDB* db, const std::string& companyName,
                          const std::string& currentUsername, bool isAdmin,
                          QWidget* parent = nullptr)
        : QWidget(parent), db(*db), companyName(companyName),
        currentUsername(currentUsername), isAdmin(isAdmin),
        currentSection(GENERAL_MESSAGES_KEY) // Default to general
    {
        setupUI();
        loadAndDisplayMessages(); // Initial load
        determineWriteAccess();
    }

private slots:
    void onSectionChanged(int index) {
        QString sectionKey = sectionComboBox->itemData(index).toString();
        if (sectionKey.toStdString() == currentSection) {
            return; // No change
        }

        currentSection = sectionKey.toStdString();
        messageListWidget->clear(); // Clear display
        loadAndDisplayMessages(); // Load messages for the new section
        determineWriteAccess();   // Update input access rights
    }

    void onSendMessageClicked() {
        QString messageText = messageInput->text().trimmed();
        if (messageText.isEmpty()) {
            return;
        }

        // Get current timestamp (client-side)
        long long currentTimestampMs = QDateTime::currentMSecsSinceEpoch();

        // --- Determine where to write ---
        int writeIndex = -1;
        std::vector<json> existingMessages;
        try {
            existingMessages = fetchMessagesForSection(currentSection);
        } catch (const std::exception& e) {
            QMessageBox::critical(this, tr("Error"), QString(tr("Could not read existing messages to determine write slot: %1").arg(e.what())));
            return;
        }

        int numExisting = static_cast<int>(existingMessages.size());

        if (numExisting < MAX_MESSAGES) {
            // Find the first available index slot (0 to MAX_MESSAGES - 1)
            std::set<int> existingIndices;
            for(const auto& msg : existingMessages) {
                if(msg.contains("_index") && msg["_index"].is_number()) {
                    existingIndices.insert(msg["_index"].get<int>());
                }
            }
            for (int i = 0; i < MAX_MESSAGES; ++i) {
                if (existingIndices.find(i) == existingIndices.end()) {
                    writeIndex = i;
                    break;
                }
            }
            // Should always find an index if numExisting < MAX_MESSAGES
            if (writeIndex == -1) {
                QMessageBox::critical(this, tr("Error"), tr("Logic error: Could not find an empty slot."));
                return;
            }

        } else {
            // Buffer is full, find the oldest message to overwrite
            if (existingMessages.empty()) {
                QMessageBox::critical(this, tr("Error"), tr("Logic error: Message count is max, but no messages found."));
                return; // Should not happen if numExisting == MAX_MESSAGES
            }

            auto oldestMsgIt = std::min_element(existingMessages.begin(), existingMessages.end(),
                                                [](const json& a, const json& b) {
                                                    long long ts_a = a.value("timestamp", 0LL); // Use value for safety
                                                    long long ts_b = b.value("timestamp", 0LL);
                                                    return ts_a < ts_b;
                                                });

            if (oldestMsgIt != existingMessages.end() && oldestMsgIt->contains("_index") && (*oldestMsgIt)["_index"].is_number()) {
                writeIndex = (*oldestMsgIt)["_index"].get<int>();
            } else {
                QMessageBox::critical(this, tr("Error"), tr("Could not determine the oldest message slot to overwrite."));
                return;
            }
        }

        // --- Write the new message ---
        if (writeIndex != -1) {
            json messageData;
            messageData["content"] = messageText.toStdString(); // Changed field name
            messageData["author"] = currentUsername;
            messageData["timestamp"] = currentTimestampMs;

            try {
                std::string writePath = "companies/" + companyName + "/messages/" + currentSection + "/msg" + std::to_string(writeIndex);
                writePath = db.urlEncode(writePath);
                db.writeData(writePath, messageData); // Overwrite or create data at the specific index path

                messageInput->clear(); // Clear input field
                loadAndDisplayMessages(); // Reload messages to show the update

            } catch (const std::exception& e) {
                QMessageBox::critical(this, tr("Send Error"), QString(tr("Failed to write message to slot msg%1: %2").arg(writeIndex).arg(e.what())));
            }
        } else {
            // This case should ideally be handled by the checks above
            QMessageBox::critical(this, tr("Error"), tr("Failed to determine a valid message slot."));
        }
    }

    void onDeleteMessageRequested(const QString& messageKey) {
        if (!isAdmin) {
            QMessageBox::warning(this, tr("Permission Denied"), tr("You do not have permission to delete messages."));
            return;
        }

        // --- Confirmation ---
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("Confirm Delete"),
                                      QString(tr("Are you sure you want to delete message %1?").arg(messageKey)),
                                      QMessageBox::Yes | QMessageBox::No);

        if (reply == QMessageBox::No) {
            return; // User cancelled
        }

        // --- Proceed with Deletion in Firebase ---
        try {
            std::string path = "companies/" + companyName + "/messages/" + currentSection + "/" + messageKey.toStdString();
            path = db.urlEncode(path);


            db.writeData(path, json(nullptr));

            QMessageBox::information(this, tr("Success"), QString(tr("Message deleted.")));

            // --- Refresh the display ---
            // Easiest way is to reload everything for this section
            loadAndDisplayMessages();

        } catch (const std::exception& e) {
            QMessageBox::critical(this, tr("Deletion Failed"), QString(tr("Failed to delete message %1: %2"))
                                      .arg(messageKey).arg(e.what()));
        }
    }

private:
    void setupUI() {
        QVBoxLayout* mainLayout = new QVBoxLayout(this);

        // --- Section Selection ---
        QHBoxLayout* selectionLayout = new QHBoxLayout();
        QLabel* sectionLabel = new QLabel(tr("View Section:"));
        sectionComboBox = new QComboBox();
        sectionComboBox->addItem(tr("General Messages"), QString::fromStdString(GENERAL_MESSAGES_KEY));
        sectionComboBox->addItem(tr("Admin Announcements"), QString::fromStdString(ADMIN_ANNOUNCEMENTS_KEY));
        selectionLayout->addWidget(sectionLabel);
        selectionLayout->addWidget(sectionComboBox);
        selectionLayout->addStretch();
        mainLayout->addLayout(selectionLayout);

        // --- Message Display Area ---
        messageListWidget = new QListWidget(); // ADD this
        messageListWidget->setSelectionMode(QAbstractItemView::NoSelection); // Disable selection appearance
        messageListWidget->setFocusPolicy(Qt::NoFocus); // Prevent list from taking focus easily
        mainLayout->addWidget(messageListWidget, 1); // Add the list widget

        // --- Input Area ---
        QHBoxLayout* inputLayout = new QHBoxLayout();
        messageInput = new QLineEdit();
        messageInput->setPlaceholderText(tr("Type your message here..."));
        sendButton = new QPushButton(tr("Send"));
        inputLayout->addWidget(messageInput);
        inputLayout->addWidget(sendButton);
        mainLayout->addLayout(inputLayout);

        // --- Connections ---
        connect(sectionComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MessageBoard::onSectionChanged);
        connect(sendButton, &QPushButton::clicked, this, &MessageBoard::onSendMessageClicked);
        connect(messageInput, &QLineEdit::returnPressed, sendButton, &QPushButton::click);

        setLayout(mainLayout);
    }

    // Fetches messages using a single read for the entire section
    std::vector<json> fetchMessagesForSection(const std::string& section) {
        std::vector<json> messages;
        std::string path = "companies/" + companyName + "/messages/" + section;
        path = db.urlEncode(path);

        try {
            json sectionData = db.readData(path);

            if (sectionData.is_object()) {
                for (auto& [key, value] : sectionData.items()) {
                    // Check if the key matches the expected format "msg{index}"
                    if (key.rfind("msg", 0) == 0 && value.is_object()) {
                        // Try to extract index from key
                        try {
                            std::string indexStr = key.substr(3); // Get string after "msg"
                            int index = std::stoi(indexStr);
                            if (index >= 0 && index < MAX_MESSAGES) {
                                // Add index to the message object for later use
                                value["_index"] = index;
                                // Validate essential fields (optional but good practice)
                                if (value.contains("author") && value.contains("content") && value.contains("timestamp")) {
                                    messages.push_back(value);
                                }
                            }
                        } catch (const std::invalid_argument& ia) {
                            // Key format invalid (e.g., "msgABC"), ignore
                            qWarning("Invalid message key format: %s", key.c_str());
                        } catch (const std::out_of_range& oor) {
                            // Index number too large, ignore
                            qWarning("Message index out of range in key: %s", key.c_str());
                        }
                    }
                }
            }
            // If sectionData is null or not an object, messages vector remains empty.
        } catch (const std::exception& e) {
            // Rethrow or handle error appropriately - rethrowing for now
            throw std::runtime_error("Failed to read message section from Firebase: " + std::string(e.what()));
        }
        return messages;
    }


    void loadAndDisplayMessages() {
        messageListWidget->clear(); // Clear the list widget

        // Add temporary item while loading
        QListWidgetItem* loadingItem = new QListWidgetItem(tr("Loading messages..."));
        messageListWidget->addItem(loadingItem);

        std::vector<json> messages;
        try {
            messages = fetchMessagesForSection(currentSection);
        } catch (const std::exception& e) {
            messageListWidget->clear(); // Clear loading message
            QListWidgetItem* errorItem = new QListWidgetItem(QString(tr("<p style='color:red;'><i>Error loading messages: %1</i></p>").arg(QString::fromStdString(e.what()).toHtmlEscaped())));
            messageListWidget->addItem(errorItem);
            return;
        }

        // Sort messages by timestamp (descending)
        std::sort(messages.begin(), messages.end(), [](const json& a, const json& b) {
            long long ts_a = a.value("timestamp", 0LL);
            long long ts_b = b.value("timestamp", 0LL);
            return ts_a > ts_b;
        });

        // --- Populate the QListWidget ---
        messageListWidget->clear(); // Clear loading/error message

        if (messages.empty()) {
            QListWidgetItem* emptyItem = new QListWidgetItem(tr("No messages yet."));
            messageListWidget->addItem(emptyItem);
        } else {
            for (const auto& msg : messages) {
                std::string author = msg.value("author", "Unknown");
                std::string content = msg.value("content", "");

                long long timestamp_ms = msg.value("timestamp", 0LL);
                int index = msg.value("_index", -1); // Get the index stored during fetch

                if (index == -1) continue; // Skip if index wasn't stored correctly

                QString messageKey = QString("msg%1").arg(index); // Reconstruct the key

                QDateTime timestamp = QDateTime::fromMSecsSinceEpoch(timestamp_ms);
                QString formattedTime = timestamp.toString(Qt::TextDate);

                // Create the custom widget for this message item
                MessageItemWidget* itemWidget = new MessageItemWidget(
                    QString::fromStdString(author),
                    formattedTime,
                    QString::fromStdString(content),
                    messageKey,
                    isAdmin, // Pass admin status to show/hide delete button
                    messageListWidget // Parent for memory management
                    );

                // Connect the delete signal from the item widget to our slot
                connect(itemWidget, &MessageItemWidget::deleteRequested, this, &MessageBoard::onDeleteMessageRequested);

                // Create a list widget item to hold the custom widget
                QListWidgetItem* listItem = new QListWidgetItem(messageListWidget);
                // Set the size hint for the list item based on the widget's needs
                listItem->setSizeHint(itemWidget->sizeHint());

                // Add the item to the list and set its widget
                messageListWidget->addItem(listItem);
                messageListWidget->setItemWidget(listItem, itemWidget);
            }

        }

    }


    void determineWriteAccess() {
        bool canWrite = false;
        if (currentSection == GENERAL_MESSAGES_KEY) {
            canWrite = true;
        } else if (currentSection == ADMIN_ANNOUNCEMENTS_KEY) {
            canWrite = isAdmin;
        }

        messageInput->setEnabled(canWrite);
        sendButton->setEnabled(canWrite);
        messageInput->setPlaceholderText(canWrite ? tr("Type your message here...") : tr("Read-only section"));
    }

    // --- Member Variables ---
    FirebaseDB& db;
    std::string companyName;
    std::string currentUsername;
    bool isAdmin;

    std::string currentSection;
    const int MAX_MESSAGES = 50; // Fixed message limit

    // --- UI Elements ---
    QComboBox* sectionComboBox;
    QListWidget* messageListWidget;
    QLineEdit* messageInput;
    QPushButton* sendButton;
};

#endif // MESSAGEBOARD_HPP


