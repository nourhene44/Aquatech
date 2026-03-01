#include "actiondelegate.h"
#include <QPushButton>
#include <QHBoxLayout>
#include <QWidget>

ActionDelegate::ActionDelegate(QWidget *parent)
    : QStyledItemDelegate(parent)
{
}

QWidget *ActionDelegate::createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                                     const QModelIndex &index) const
{
    Q_UNUSED(option);
    m_lastClickedRow = index.row();

    QWidget *editorWidget = new QWidget(parent);
    QHBoxLayout *layout = new QHBoxLayout(editorWidget);
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
        "}"
        "QPushButton:hover { background-color: #45a049; }"
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
        "}"
        "QPushButton:hover { background-color: #da190b; }"
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
        "}"
        "QPushButton:hover { background-color: #0b7dda; }"
    );

    layout->addWidget(editBtn);
    layout->addWidget(deleteBtn);
    layout->addWidget(refreshBtn);
    layout->addStretch();

    // Connect buttons to signals
    connect(editBtn, &QPushButton::clicked, [this, index]() {
        const_cast<ActionDelegate*>(this)->editClicked(index.row());
    });
    connect(deleteBtn, &QPushButton::clicked, [this, index]() {
        const_cast<ActionDelegate*>(this)->deleteClicked(index.row());
    });
    connect(refreshBtn, &QPushButton::clicked, [this, index]() {
        const_cast<ActionDelegate*>(this)->refreshClicked(index.row());
    });

    return editorWidget;
}

void ActionDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    Q_UNUSED(editor);
    Q_UNUSED(index);
}

void ActionDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                 const QModelIndex &index) const
{
    Q_UNUSED(editor);
    Q_UNUSED(model);
    Q_UNUSED(index);
}

void ActionDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                                         const QModelIndex &index) const
{
    Q_UNUSED(index);
    Q_UNUSED(option);
    editor->setGeometry(option.rect);
}
