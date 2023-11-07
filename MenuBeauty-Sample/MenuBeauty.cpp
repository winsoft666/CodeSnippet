#include "stdafx.h"
#include "MenuBeauty.h"

MenuBeauty::MenuBeauty(QWidget* parent) :
    QMainWindow(parent) {
    ui.setupUi(this);

    QIcon uncheckIcon(":/MenuBeauty/check.png");
    QIcon checkedIcon(":/MenuBeauty/unchecked.png");
    QIcon settingIcon(":/MenuBeauty/setting.png");

    // ����Action
    QAction* action1 = new QAction("����1");
    QAction* action2 = new QAction(settingIcon, "����2 ��ͼ��");

    QAction* action3 = new QAction("����3 [δѡ��]");
    action3->setCheckable(true);
    action3->setChecked(false);
    connect(action3, &QAction::triggered, this, [action3](bool checked) {
        action3->setText(checked ? "����3 [ѡ��]" : "����3 [δѡ��]");
    });

    QAction* action4 = new QAction("Action4 �����Ĳ���");

    QAction* action5 = new QAction(settingIcon, "����5 ����");
    action5->setEnabled(false);

    QAction* action6 = new QAction(settingIcon, "����6 �Ӳ˵�");
    {
        // ����6���Ӷ���
        QAction* action6_1 = new QAction("����6.1");
        action6_1->setCheckable(true);

        QAction* action6_2 = new QAction("����6.1");
        action6_2->setCheckable(true);

        QAction* action6_3 = new QAction("����6.1");
        action6_3->setCheckable(true);

        // ����6���Ӳ˵�
        QMenu* action6SubMenu = new QMenu(this);
        action6SubMenu->setWindowFlags(action6SubMenu->windowFlags() |
                                       Qt::FramelessWindowHint |   // �ޱ߿�
                                       Qt::NoDropShadowWindowHint  // �Ƴ�ϵͳ��Ӱ
        );
        action6SubMenu->setAttribute(Qt::WA_TranslucentBackground);

        action6SubMenu->addAction(action6_1);
        action6SubMenu->addAction(action6_2);
        action6SubMenu->addAction(action6_3);

        // �����Ӳ˵��ͬʱֻ��ѡ��һ��
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

    // ����˵�
    QMenu* menu = new QMenu(this);
    menu->setWindowFlags(menu->windowFlags() | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
    menu->setAttribute(Qt::WA_TranslucentBackground);

    // ���Action
    menu->addActions({action1, action2, action3});
    menu->addSeparator();
    menu->addActions({action4, action5, action6});
    menu->addAction(action7);

    // ����Ҽ������˵�
    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &QWidget::customContextMenuRequested, this, [this, menu](const QPoint& pos) {
        menu->exec(this->mapToGlobal(pos));
    });

    // ���ò˵�QSS��ʽ
#if 1
    setStyleSheet(
        u8R"(
            /* �˵������� */
            QMenu {
	            border: 1px solod red;
                /* Բ����Ч��ǰ���ǲ˵����ڱ���͸�� */
	            border-radius: 10px;
	            background-color: blue;
	            font-size: 14px;
	            font-family: "Microsoft YaHei";
                /* ���ò˵�����С��ȣ��˴���bug������Զ�������ʵ�ʿ�ȱ�������С */
                min-width: 160px;
                /* �˵��������±߾�Ϊ10px��Ԥ��Բ������ */
                padding: 10px 0px 10px 0px;
            }

            /* �˵�������� */
            QMenu::item {
	            border: none;
	            background-color: transparent;
	            color: white;
                /* ���ò˵�����С�߶ȣ������ò˵���ĸ���Ӧ�����Զ����� */
	            min-height: 20px;
                /* ���ò˵�����С��ȣ��˴���bug������Զ�������ʵ�ʿ�ȱ�������С */
                min-width: 160px;
                /* ʹ�˵���֮�估�˵����ͼ��֮�䱣��һ������������������ұ߾�Ϊ8px���˴�������margin */
                padding: 8px 8px;
            }

            /* �˵���-�������ʱ������ */
            QMenu::item:selected {
	            background-color: green;
                color: black;
            }

            /* �˵���-����ʱ������ */
            QMenu::item:disabled {
	            background-color: gray;
                color: white;
            }

            /* �ָ��ߵ����� */
            QMenu::separator {
                height: 1px;
                background-color: red;
                /* ���ұ߾ࣺ6px */
                margin: 0x 6px 0px 6px;
            }
            
            /* ͼ������� */
            QMenu::icon {
                width: 12px;
                height: 12px;
                /* ��߾ࣺ12px */
                margin: 0 0 0 12px;
            }

            /* ָʾ�������� */
            /* ���ｫ����ͷǻ���Action��ָʾ����ʽ����Ϊһ���ģ�Ҳ��������Ϊ��һ�� */
            QMenu::indicator:non-exclusive:checked, QMenu::indicator:exclusive:checked {
                width: 12px;
                height: 12px;
                /* ��߾ࣺ8px */
                margin: 0 0 0 8px;
                image: url(:/MenuBeauty/check.png);
            }

            QMenu::indicator:non-exclusive:unchecked, QMenu::indicator:exclusive:unchecked {
                width: 12px;
                height: 12px;
                /* ��߾ࣺ8px */
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
