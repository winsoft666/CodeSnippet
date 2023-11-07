#include "stdafx.h"
#include "MenuBeauty.h"

MenuBeauty::MenuBeauty(QWidget* parent) :
    QMainWindow(parent) {
    ui.setupUi(this);

    QIcon uncheckIcon(":/MenuBeauty/check.png");
    QIcon checkedIcon(":/MenuBeauty/unchecked.png");
    QIcon settingIcon(":/MenuBeauty/setting.png");

    // 定义Action
    QAction* action1 = new QAction("动作1");
    QAction* action2 = new QAction(settingIcon, "动作2 有图标");

    QAction* action3 = new QAction("动作3 [未选中]");
    action3->setCheckable(true);
    action3->setChecked(false);
    connect(action3, &QAction::triggered, this, [action3](bool checked) {
        action3->setText(checked ? "动作3 [选中]" : "动作3 [未选中]");
    });

    QAction* action4 = new QAction("Action4 动作四测试");

    QAction* action5 = new QAction(settingIcon, "动作5 禁用");
    action5->setEnabled(false);

    QAction* action6 = new QAction(settingIcon, "动作6 子菜单");
    {
        // 动作6的子动作
        QAction* action6_1 = new QAction("动作6.1");
        action6_1->setCheckable(true);

        QAction* action6_2 = new QAction("动作6.1");
        action6_2->setCheckable(true);

        QAction* action6_3 = new QAction("动作6.1");
        action6_3->setCheckable(true);

        // 动作6的子菜单
        QMenu* action6SubMenu = new QMenu(this);
        action6SubMenu->setWindowFlags(action6SubMenu->windowFlags() |
                                       Qt::FramelessWindowHint |   // 无边框
                                       Qt::NoDropShadowWindowHint  // 移除系统阴影
        );
        action6SubMenu->setAttribute(Qt::WA_TranslucentBackground);

        action6SubMenu->addAction(action6_1);
        action6SubMenu->addAction(action6_2);
        action6SubMenu->addAction(action6_3);

        // 互斥子菜单项，同时只能选择一个
        QActionGroup* action6Group = new QActionGroup(this);
        action6Group->setExclusive(true);
        action6Group->addAction(action6_1);
        action6Group->addAction(action6_2);
        action6Group->addAction(action6_3);

        action6->setMenu(action6SubMenu);
    }

    QWidgetAction* action7 = new QWidgetAction(this);
    {
        QWidget* widget = new QWidget();
        QHBoxLayout* hl = new QHBoxLayout(widget);

        auto createPushButtonFn = [this](QString title) {
            QPushButton* btn = new QPushButton(title);
            connect(btn, &QPushButton::clicked, this, [this]() {
                QMessageBox::information(this, "Clicked", ((QPushButton*)sender())->text());
            });
            return btn;
        };

        hl->addWidget(createPushButtonFn("Button1"));
        hl->addWidget(createPushButtonFn("Button2"));
        hl->addWidget(createPushButtonFn("Button3"));

        action7->setDefaultWidget(widget);
    }

    // 定义菜单
    QMenu* menu = new QMenu(this);
    menu->setWindowFlags(menu->windowFlags() | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    menu->setAttribute(Qt::WA_TranslucentBackground);

    // 添加Action
    menu->addActions({action1, action2, action3});
    menu->addSeparator();
    menu->addActions({action4, action5, action6});
    menu->addAction(action7);

    // 鼠标右键弹出菜单
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, [this, menu](const QPoint& pos) {
        menu->exec(this->mapToGlobal(pos));
    });

    // 设置菜单QSS样式
#if 1
    setStyleSheet(
        u8R"(
            /* 菜单的属性 */
            QMenu {
	            border: 1px solod red;
                /* 圆角生效的前提是菜单窗口背景透明 */
	            border-radius: 10px;
	            background-color: blue;
	            font-size: 14px;
	            font-family: "Microsoft YaHei";
                /* 设置菜单项最小宽度，此处有bug：虽可自动增长但实际宽度比所需宽度小 */
                min-width: 160px;
                /* 菜单内容上下边距为10px，预留圆角区域 */
                padding: 10px 0px 10px 0px;
            }

            /* 菜单项的属性 */
            QMenu::item {
	            border: none;
	            background-color: transparent;
	            color: white;
                /* 设置菜单项最小高度，可以让菜单项的高适应内容自动增长 */
	            min-height: 20px;
                /* 设置菜单项最小宽度，此处有bug：虽可自动增长但实际宽度比所需宽度小 */
                min-width: 160px;
                /* 使菜单项之间及菜单项和图标之间保持一定间隔，设置上下左右边距为8px，此处不能用margin */
                padding: 8px 8px;
            }

            /* 菜单项-鼠标移入时的属性 */
            QMenu::item:selected {
	            background-color: green;
                color: black;
            }

            /* 菜单项-禁用时的属性 */
            QMenu::item:disabled {
	            background-color: gray;
                color: white;
            }

            /* 分割线的属性 */
            QMenu::separator {
                height: 1px;
                background-color: red;
                /* 左右边距：6px */
                margin: 0x 6px 0px 6px;
            }
            
            /* 图标的属性 */
            QMenu::icon {
                width: 12px;
                height: 12px;
                /* 左边距：12px */
                margin: 0 0 0 12px;
            }

            /* 指示器的属性 */
            /* 这里将互斥和非互斥Action的指示器样式设置为一样的，也可以设置为不一样 */
            QMenu::indicator:non-exclusive:checked, QMenu::indicator:exclusive:checked {
                width: 12px;
                height: 12px;
                /* 左边距：8px */
                margin: 0 0 0 8px;
                image: url(:/MenuBeauty/check.png);
            }

            QMenu::indicator:non-exclusive:unchecked, QMenu::indicator:exclusive:unchecked {
                width: 12px;
                height: 12px;
                /* 左边距：8px */
                margin: 0 0 0 8px;
                image: url(:/MenuBeauty/unchecked.png);
            }

            QMenu QPushButton {
                border: none;
                border-radius: 10px;
                background-color: black;
                color: white;
            }
        )");
#endif
}

MenuBeauty::~MenuBeauty() {}
