#include "actionswidget.h"

ActionsButtonWidget::ActionsButtonWidget(int rowIndex, QWidget *parent)
    : QWidget(parent), m_row(rowIndex)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(2, 2, 2, 2);
    layout->setSpacing(2);

    // Edit button
    QPushButton *editBtn = new QPushButton("✏️ Edit");
    editBtn->setMaximumWidth(70);
    editBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #4CAF50;"
        "  color: white;"
        "  border: none;"
        "  padding: 5px;"
        "  border-radius: 3px;"
        "  font-weight: bold;"
        "  font-size: 11px;"
        "}"
        "QPushButton:hover { background-color: #45a049; }"
        "QPushButton:pressed { background-color: #3d8b40; }"
    );

    // Delete button
    QPushButton *deleteBtn = new QPushButton("🗑️ Delete");
    deleteBtn->setMaximumWidth(80);
    deleteBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #f44336;"
        "  color: white;"
        "  border: none;"
        "  padding: 5px;"
        "  border-radius: 3px;"
        "  font-weight: bold;"
        "  font-size: 11px;"
        "}"
        "QPushButton:hover { background-color: #da190b; }"
        "QPushButton:pressed { background-color: #ba0000; }"
    );

    // Refresh button
    QPushButton *refreshBtn = new QPushButton("🔄 Refresh");
    refreshBtn->setMaximumWidth(85);
    refreshBtn->setStyleSheet(
        "QPushButton {"
        "  background-color: #2196F3;"
        "  color: white;"
        "  border: none;"
        "  padding: 5px;"
        "  border-radius: 3px;"
        "  font-weight: bold;"
        "  font-size: 11px;"
        "}"
        "QPushButton:hover { background-color: #0b7dda; }"
        "QPushButton:pressed { background-color: #0056b3; }"
    );

    layout->addWidget(editBtn);
    layout->addWidget(deleteBtn);
    layout->addWidget(refreshBtn);
    layout->addStretch();

    // Connect buttons to signals giving row information
    connect(editBtn, &QPushButton::clicked, [this]() {
        emit editClicked(m_row);
    });
    connect(deleteBtn, &QPushButton::clicked, [this]() {
        emit deleteClicked(m_row);
    });
    connect(refreshBtn, &QPushButton::clicked, [this]() {
        emit refreshClicked(m_row);
    });
}
