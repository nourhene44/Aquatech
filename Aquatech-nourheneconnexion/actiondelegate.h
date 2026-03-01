#ifndef ACTIONDELEGATE_H
#define ACTIONDELEGATE_H

#include <QStyledItemDelegate>
#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>

class ActionDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit ActionDelegate(QWidget *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
                         const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model,
                     const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option,
                             const QModelIndex &index) const override;

signals:
    void editClicked(int row);
    void deleteClicked(int row);
    void refreshClicked(int row);

private:
    mutable int m_lastClickedRow = -1;
};

#endif // ACTIONDELEGATE_H
