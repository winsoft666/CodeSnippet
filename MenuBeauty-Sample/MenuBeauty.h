#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_MenuBeauty.h"
#include <QtWidgets>

class MenuBeauty : public QMainWindow
{
    Q_OBJECT

public:
    MenuBeauty(QWidget *parent = nullptr);
    ~MenuBeauty();

private:
    Ui::MenuBeautyClass ui;
    QMenu* menu_ = Q_NULLPTR;
};
