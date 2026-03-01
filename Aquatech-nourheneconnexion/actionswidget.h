#ifndef ACTIONSWIDGET_H
#define ACTIONSWIDGET_H

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>

class ActionsButtonWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ActionsButtonWidget(int rowIndex, QWidget *parent = nullptr);

signals:
    void editClicked(int row);
    void deleteClicked(int row);
    void refreshClicked(int row);

private:
    int m_row;
};

#endif // ACTIONSWIDGET_H
