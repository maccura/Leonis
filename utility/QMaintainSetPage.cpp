/***************************************************************************
*   This file is part of the leonis project                               *
*   Copyright (C) 2024 by Mike Medical Electronics Co., Ltd               *
*   zhouguangming@maccura.com                                             *
*                                                                         *
**                   GNU General Public License Usage                    **
*                                                                         *
*   This library is free software: you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation, either version 3 of the License, or     *
*   (at your option) any later version.                                   *
*   You should have received a copy of the GNU General Public License     *
*   along with this program.  If not, see <http://www.gnu.org/licenses/>. *
*                                                                         *
**                  GNU Lesser General Public License                    **
*                                                                         *
*   This library is free software: you can redistribute it and/or modify  *
*   it under the terms of the GNU Lesser General Public License as        *
*   published by the Free Software Foundation, either version 3 of the    *
*   License, or (at your option) any later version.                       *
*   You should have received a copy of the GNU Lesser General Public      *
*   License along with this library.                                      *
*   If not, see <http://www.gnu.org/licenses/>.                           *
*                                                                         *
*   This library is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
****************************************************************************/

///////////////////////////////////////////////////////////////////////////
/// @file     QMaintainSetPage.h
/// @brief 	 应用->系统->维护保养组合设置界面
///
/// @author   7656/zhang.changjiang
/// @date      2022年8月30日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7656/zhang.changjiang，2022年8月30日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QMaintainSetPage.h"
#include "ui_QMaintainSetPage.h"
#include <QMouseEvent>
#include <QCursor>
#include <QCheckBox>
#include <QRegularExpression>
#include <QPainter>
#include <QToolButton>
#include <QHBoxLayout>
#include <QStandardItemModel>
#include <QKeyEvent>
#include <QToolTip>

#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/uicommon.h"
#include "shared/tipdlg.h"
#include "shared/CommonInformationManager.h"
#include "manager/DictionaryQueryManager.h"
#include "manager/UserInfoManager.h"
#include "uidcsadapter/uidcsadapter.h"
#include "utility/maintaindatamng.h"

#include "model/MaintainLogModel.h"
#include "model/MaintainGroupModel.h"
#include "MaintainDefaultSetDlg.h"

#include "../thrift/DcsControlProxy.h"
#include "src/common/Mlog/mlog.h"

//联想下拉框四边间距
#define ITEM_LEFT_MARGIN 10
#define ITEM_RIGHT_MARGIN 10
#define ITEM_TOP_MARGIN 20
#define ITEM_BOTTOM_MARGIN 20

// 维护组名称最大长度
#define GROUP_NAME_MAX_LEN 45
#define TABLE_CELL_MARGIN (20)


SearchLineEdit::SearchLineEdit(QWidget *parent) :
    QLineEdit(parent)
{
    Init();
}

SearchLineEdit::~SearchLineEdit()
{
}

///
///  @brief 初始化
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年3月3日，新建函数
///
void SearchLineEdit::Init()
{
    //使输入框具有初始化提示语句
    this->setPlaceholderText(tr("搜索"));
    this->setFixedSize(200, 24);
    this->setWindowFlag(Qt::FramelessWindowHint);
    this->move(300, 300);

    //布局设置搜索按钮
    m_searchBtn = new QToolButton(this);
    m_searchBtn->setFixedSize(24, 24);
    m_searchBtn->setIcon(QIcon(QString(":/Leonis/resource/image/icon-query-dis.png")));
    m_searchBtn->setIconSize(QSize(24, 24));
    m_searchBtn->setStyleSheet("border: 0; background: transparent; margin-left: 10px;");

    //布局设置清除按钮
    m_cleanBtb = new QToolButton(this);
    m_cleanBtb->setFixedSize(16, 16);
    connect(m_cleanBtb, &QToolButton::clicked, this, [&] {this->clear(); });
    m_cleanBtb->setIcon(QIcon(QString(":/Leonis/resource/image/icon-cancel.png")));
    QHBoxLayout* layout = new QHBoxLayout;
    layout->addWidget(m_searchBtn);
    layout->addSpacing(this->width() - m_searchBtn->width() + 5/*- m_cleanBtb->width()*/);
    layout->addWidget(m_cleanBtb);
    layout->setContentsMargins(0, 0, 0, 0);
    this->setLayout(layout);

    // 隐藏清除按钮（以后需求可能需要该按钮）
    m_cleanBtb->hide();

    //设置起始输入位置
    this->setTextMargins(m_searchBtn->width(), 0, m_cleanBtb->width(), 0);
    connect(this, SIGNAL(textChanged(QString)), this, SLOT(SlotEditChange(QString)));

    m_searchBtn->installEventFilter(this);
}

bool SearchLineEdit::eventFilter(QObject * obj, QEvent * event)
{
    if (obj == m_searchBtn && event->type() == QEvent::HoverEnter)
    {
        QApplication::setOverrideCursor(QCursor(Qt::ArrowCursor));
        m_searchBtn->setFocus();
        return true;
    }
    else if (obj == m_searchBtn && event->type() == QEvent::HoverLeave)
    {
        QApplication::restoreOverrideCursor();
        this->setFocus();
        return true;
    }

    return QLineEdit::eventFilter(obj, event);
}

ScrollBar::ScrollBar(QWidget * parent)
    : QScrollBar(parent)
{
}

ScrollBar::ScrollBar(Qt::Orientation orientation, QWidget * parent)
    : QScrollBar(orientation, parent)
{
}

///
///  @brief 显示事件
///
void ScrollBar::showEvent(QShowEvent * event)
{
    emit barShow();
    QScrollBar::showEvent(event);
}

///
///  @brief 隐藏事件
///
void ScrollBar::hideEvent(QHideEvent * event)
{
    emit barHide();
    QScrollBar::hideEvent(event);
}

QMaintainSetPage::QMaintainSetPage(QWidget *parent)
    : BaseDlg(parent)
    , m_pMaintainDefaultSetDlg(nullptr)
{
    ui = new Ui::QMaintainSetPage();
    ui->setupUi(this);
    Init();
    installEventFilter(this);
}

QMaintainSetPage::~QMaintainSetPage()
{
}

bool QMaintainSetPage::eventFilter(QObject * obj, QEvent * event)
{
    if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease)
    {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent == nullptr)
        {
            return QWidget::eventFilter(obj, event);
        }

        if (keyEvent->key() == Qt::Key_Control || keyEvent->key() == Qt::Key_Shift)
        {
            ui->tableView_mantain_group->setSelectionMode(event->type() == QEvent::KeyPress ? QAbstractItemView::MultiSelection : QAbstractItemView::SingleSelection);
        }

		if (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return)
		{
			OnSerachItemBtnClicked();
			return true;
		}
    }
    return QWidget::eventFilter(obj, event);
}

void QMaintainSetPage::showEvent(QShowEvent * event)
{
    // 更新全部清除按钮位置
    UpdateClearBtnPos();

    // 更新搜索框位置
    UpdateSearchBoxPos();

    // 调用基类显示事件处理
    BaseDlg::showEvent(event);
}

///
///  @brief:	初始化
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年8月29日，新建函数
///
void QMaintainSetPage::Init()
{
    // 设置标题
    SetTitleName(tr("组合设置"));

    // 初始化维护项
    InitItem();

    // 初始化维护组
    InitGroup();

    // 连接信号槽
    ConnectSigAndSlots();

    // 只有联机时才显示机型选中框
    ui->comboBox->hide();

    MaintainGroupModel::Instance().BackupCurrentData();

    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);
}

///
///  @brief 初始化维护组表
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年9月6日，新建函数
///
void QMaintainSetPage::InitGroup()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 设置置顶、向上、向下、置底按钮初始状态
    ui->upBtn->setEnabled(false);
    ui->downBtn->setEnabled(false);
    ui->topBtn->setEnabled(false);
    ui->bottomBtn->setEnabled(false);

    // 组合名称输入框置灰
    ui->lineEdit_group_name->setEnabled(false);

    // 初始化维护组配置表模型
    m_pCfgModel = new QMaintainGroupCfgFilterProxyModel(ui->tableView_group_cfg);
    ui->tableView_group_cfg->setModel(m_pCfgModel);
    m_pCfgModelDelegate = new QMaintainItemSelectDelegate(this);
    ui->tableView_group_cfg->setItemDelegateForColumn(0, m_pCfgModelDelegate);

    // 更新维护配置表已选项目数
    connect(m_pCfgModel, &QMaintainGroupCfgFilterProxyModel::updateSelected, this, [&](int size) {ui->label_selected->setText(QString(tr("已选（%1）")).arg(size)); }, Qt::UniqueConnection);

    ScrollBar* scrollBar = new ScrollBar(Qt::Orientation::Vertical, this);
    connect(scrollBar, &ScrollBar::barShow, this, &QMaintainSetPage::UpdateClearBtnPos, Qt::UniqueConnection);
    connect(scrollBar, &ScrollBar::barHide, this, &QMaintainSetPage::UpdateClearBtnPos, Qt::UniqueConnection);
    ui->tableView_group_cfg->setVerticalScrollBar(scrollBar);
    ui->tableView_group_cfg->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView_group_cfg->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView_group_cfg->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView_group_cfg->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_group_cfg->verticalHeader()->hide();
    ui->tableView_group_cfg->setDragEnabled(true);
    ui->tableView_group_cfg->setMouseTracking(true);
    ui->tableView_group_cfg->setDefaultDropAction(Qt::MoveAction);
    ui->tableView_group_cfg->setDragDropMode(QAbstractItemView::DragDrop);
    ui->tableView_group_cfg->setAutoScroll(true);

    // 初始化维护组表模型
    m_pMaintainGroupModel = new QMaintainGroupFilterProxyModel(ui->tableView_mantain_group);
    MaintainGroupModel::Instance().SetmHeaderNames({ tr("序号"), tr("组合名称") });
    ui->tableView_mantain_group->setModel(m_pMaintainGroupModel);
    ui->tableView_mantain_group->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView_mantain_group->horizontalHeader()->setTextElideMode(Qt::ElideRight);
    ui->tableView_mantain_group->verticalHeader()->hide();
    ui->tableView_mantain_group->setEditTriggers(QAbstractItemView::DoubleClicked);
    ui->tableView_mantain_group->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView_mantain_group->setMouseTracking(true);
    m_pDelegate = new QTableDoubleClickDelegate();

    ui->tableView_mantain_group->setItemDelegateForColumn(1, m_pDelegate);
    ui->tableView_mantain_group->setColumnWidth(0, 80);

    // 默认选中第一行
    if (!MaintainGroupModel::Instance().GetData().empty())
    {
        RefreshGroupCfg(0);
        ui->tableView_mantain_group->selectRow(0);
    }
    else
    {
        ui->delete_btn->setEnabled(false);
    }
}

///
///  @brief 初始化维护项表
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年9月6日，新建函数
///
void QMaintainSetPage::InitItem()
{
    // 初始化维护项表模型
    m_pMaintainItemModel = new QStandardItemModel(this);
    ui->tableView_maintain_item->setModel(m_pMaintainItemModel);

    // 查询所有的维护项
    std::vector<::tf::MaintainItem> items;
    QueryAllItemData(items);

    // 过滤不显示的维护项
    FilterItems(items);

    // 初始化维护项表数据
    InitItemTableData(items);

    QHBoxLayout* pHlayout = new QHBoxLayout(ui->frame_searchBox);
    pHlayout->setMargin(0);
    pHlayout->setSpacing(30);
    pHlayout->addStretch(1);
    m_pSearchLineEdit = new SearchLineEdit(this);
    m_pSearchLineEdit->setFixedWidth(ui->frame_searchBox->width());
    m_pSearchLineEdit->setFixedHeight(ui->frame_searchBox->height());
    pHlayout->addWidget(m_pSearchLineEdit);

    // 初始化滚动条
    ScrollBar* scrollBar = new ScrollBar(Qt::Orientation::Vertical, this);
    connect(scrollBar, &ScrollBar::barShow, this, &QMaintainSetPage::UpdateSearchBoxPos, Qt::UniqueConnection);
    connect(scrollBar, &ScrollBar::barHide, this, &QMaintainSetPage::UpdateSearchBoxPos, Qt::UniqueConnection);
    ui->tableView_maintain_item->setVerticalScrollBar(scrollBar);
    ui->tableView_maintain_item->horizontalHeader()->setStretchLastSection(true);
    ui->tableView_maintain_item->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView_maintain_item->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView_maintain_item->setSelectionMode(QAbstractItemView::NoSelection);
    ui->tableView_maintain_item->horizontalHeader()->setTextElideMode(Qt::ElideRight);
    ui->tableView_maintain_item->setMouseTracking(true);
    ui->tableView_maintain_item->verticalHeader()->hide();

    ui->tableView_maintain_item->horizontalHeader()->setMinimumSectionSize(44);
    ui->tableView_maintain_item->setColumnWidth(0, 44);
    ui->tableView_maintain_item->setItemDelegateForColumn(1, new QMaintItemTableDelegate());
}

///
///  @brief	连接信号槽
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年9月6日，新建函数
///
void QMaintainSetPage::ConnectSigAndSlots()
{
    // 新增按钮
    connect(ui->add_new_btn, &QPushButton::clicked, this, &QMaintainSetPage::OnAddNewBtnClicked);
    // 删除按钮
    connect(ui->delete_btn, &QPushButton::clicked, this, &QMaintainSetPage::OnDeleteBtnClicked);
    // 选项设置按钮
    //connect(ui->optionItem_set_btn, &QPushButton::clicked, this, &QMaintainSetPage::OnOptionItemSetClicked);
    // 保存按钮
    connect(ui->save_btn, &QPushButton::clicked, this, &QMaintainSetPage::OnSaveBtnClicked);
    // 搜索框按钮
    connect(m_pSearchLineEdit->GetSearchBtn(), &QToolButton::clicked, this, &QMaintainSetPage::OnSerachItemBtnClicked);
	// 搜索框文本变化
	//connect(m_pSearchLineEdit, &SearchLineEdit::textChanged, this, &QMaintainSetPage::OnSerachItemBtnClicked);
    // 全部清除按钮
    connect(ui->cleanBtn, &QPushButton::clicked, this, &QMaintainSetPage::OnCleanBtnClicked);
    // 关闭按钮
    connect(ui->close_btn, &QPushButton::clicked, this, &QMaintainSetPage::OnCloseBtnClicked);
    // 置顶按钮
    connect(ui->topBtn, &QPushButton::clicked, this, &QMaintainSetPage::OnTopBtnClicked);
    // 向上按钮
    connect(ui->upBtn, &QPushButton::clicked, this, &QMaintainSetPage::OnUpBtnClicked);
    // 向下按钮
    connect(ui->downBtn, &QPushButton::clicked, this, &QMaintainSetPage::OnDownBtnClicked);
    // 置底按钮
    connect(ui->bottomBtn, &QPushButton::clicked, this, &QMaintainSetPage::OnBottomBtnClicked);
    // 重置按钮
    //connect(ui->reset_btn, &QPushButton::clicked, this, &QMaintainSetPage::OnResetBtnClicked);
    // 点击选中维护组表的一行
    connect(ui->tableView_mantain_group, &QTableView::clicked, this, [=](const QModelIndex &index) {RefreshGroupCfg(index.row());m_pSearchLineEdit->clear(); OnSerachItemBtnClicked(); });
	connect(ui->tableView_mantain_group->selectionModel(), &QItemSelectionModel::currentRowChanged, this, 
		[=](const QModelIndex &current, const QModelIndex &previous) {RefreshGroupCfg(current.row()); m_pSearchLineEdit->clear(); OnSerachItemBtnClicked(); });
    // 更新按钮状态
    connect(ui->tableView_group_cfg, &QTableView::clicked, this, &QMaintainSetPage::UpdateBtnState);
	connect(ui->tableView_group_cfg->selectionModel(), &QItemSelectionModel::currentRowChanged, this,
		[=](const QModelIndex &current, const QModelIndex &previous) { UpdateBtnState(); });
    // 更新复选框
    connect(&MaintainGroupModel::Instance(), &MaintainGroupModel::CleanItemTableCheckBox, this, [&](int itemType) {SetCheckBoxState(itemType, false); }, Qt::UniqueConnection);
    // 代理维护组名称改变
    connect(m_pDelegate, &QTableDoubleClickDelegate::sigTextChanged, this, [&](QString text)
    {
        ui->lineEdit_group_name->blockSignals(true);
        ui->lineEdit_group_name->setText(text);
        ui->lineEdit_group_name->blockSignals(false);
    });
    // 代理关闭事件
    connect(m_pDelegate, &QTableDoubleClickDelegate::closeEditor, this, [&]()
    {
        ui->tableView_mantain_group->selectRow(m_pMaintainGroupModel->GetSelectedRow());
    });
    // 代理修改的维护组名称为空
    connect(m_pDelegate, &QTableDoubleClickDelegate::sigInputNullStr, this, [&] {TipDlg(tr("提示"), tr("修改维护组合名称时，维护组名称不能为空！")).exec(); });
    // 系统组合名称不可修改
    connect(m_pDelegate, &QTableDoubleClickDelegate::sigCannotChange, this, [&] {TipDlg(tr("提示"), tr("系统组合名称不可修改！")).exec(); });
    // 维护组名称改变
    connect(ui->lineEdit_group_name, &QLineEdit::textChanged, this, [&](const QString text)
    {
        m_pDelegate->blockSignals(true);
        auto selectedIndexs = ui->tableView_mantain_group->selectionModel()->selectedRows();
        ::tf::MaintainGroup group;
        if (!MaintainGroupModel::Instance().GetSelectData(group))
        {
            ULOG(LOG_WARN, "Failed to get selected maintaingroup.");
            return;
        }
        if (group.groupType != ::tf::MaintainGroupType::type::MAINTAIN_GROUP_CUSTOM)
        {
            return;
        }
        MaintainGroupModel::Instance().SetSelectedGroupName(text);
        for (const QModelIndex& it : selectedIndexs)
        {
            if (!it.isValid())
            {
                continue;
            }
            ui->tableView_mantain_group->selectRow(it.row());
        }
        m_pDelegate->blockSignals(false);
    });
}

///
///  @brief 更新维护组配置表
///
///
///  @param[in]   groupType    维护组类型
///  @param[in]   vecItemType  维护组包含的维护项类型列表
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年3月1日，新建函数
///
void QMaintainSetPage::UpdateGroupCfg(const ::tf::MaintainGroupType::type groupType, const std::vector<tf::MaintainItem>& vecItem)
{
    // 获取DCS客户端
    std::shared_ptr<DcsControlProxy> spCilent = DcsControlProxy::GetInstance();
    if (spCilent == nullptr)
    {
        return;
    }
    ::tf::MaintainItemQueryResp retItem;
    ::tf::MaintainItemQueryCond miqcItem;
    if (!spCilent->QueryMaintainItem(retItem, miqcItem))
    {
        ULOG(LOG_WARN, "Failed to query all maintainitem.");
        return;
    }

    // 通过维护项类型获取维护项
    auto funcTakeItemByType = [&](tf::MaintainItemType::type iType, std::vector<tf::MaintainItem>& targetItems) {
        for (const tf::MaintainItem& curItem : retItem.lstMaintainItems)
        {
            if (curItem.itemType == iType)
            {
                targetItems.push_back(curItem);
            }
        }
    };

    // 查询配置配置维护项信息
    std::vector<tf::MaintainItem> lstItem;
    for (const tf::MaintainItem& item : vecItem)
    {
        std::vector<tf::MaintainItem> lstMaintainItems;
        funcTakeItemByType(item.itemType, lstMaintainItems);

        if (lstMaintainItems.size() == 0)
        {
            continue;
        }
        for (tf::MaintainItem& it : lstMaintainItems)
        {
            it.__set_param(item.param);
            lstItem.push_back(it);
        }
    }
    m_pCfgModel->SetData(std::move(lstItem));
}

///
///  @brief 刷新维护组的配置表
///
///
///  @param[in]   iRow  维护组所在行
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年11月15日，新建函数
///
void QMaintainSetPage::RefreshGroupCfg(int iRow)
{
	// 过滤不显示的维护项    
	std::vector<::tf::MaintainItem> items;
	QueryAllItemData(items);
	FilterItems(items);
	InitItemTableData(items);
	RefreshGroupCfg(iRow, items);
}

///
/// @brief  刷新维护组的配置表（只刷筛选的数据）
///
/// @param[in]  iRow    维护组所在行
/// @param[in]  items   筛选的数据
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2024年6月12日，新建函数
///
void QMaintainSetPage::RefreshGroupCfg(const int iRow, std::vector<tf::MaintainItem>& items)
{
	ui->delete_btn->setEnabled(true);
	m_pMaintainGroupModel->SetSelectedRow(iRow);
    tf::MaintainGroup group;
    if (!MaintainGroupModel::Instance().GetSelectData(group))
    {
        ULOG(LOG_WARN, "Failed to get selected maintaingroup.");
        return;
    }
	FilterItems(items, std::make_shared<tf::MaintainGroup>(group));

	// 设置组合组合名称输入框是否可用
	ui->lineEdit_group_name->setEnabled(group.groupType == ::tf::MaintainGroupType::type::MAINTAIN_GROUP_CUSTOM);

	// 更新维护组配置表
	UpdateGroupCfg(group.groupType, group.items);

	// 刷新维护项表复选框
	std::shared_ptr<UserInfoManager> insUr = UserInfoManager::GetInstance();
	SetAllCheckBoxState(false);
	for (const tf::MaintainItem& item : group.items)
	{
		SetCheckBoxState(item.itemType, true, insUr->IsPermissonMaint(item.itemType));
	}

	// 单项维护不可修改
	if (group.groupType == ::tf::MaintainGroupType::MAINTAIN_GROUP_SINGLE)
	{
		SetAllCheckBoxEnable(false);
	}
	else
	{
		SetAllCheckBoxEnable(true);
		OnPermisionChanged();
	}
	ui->lineEdit_group_name->blockSignals(true);
	ui->lineEdit_group_name->setText(group.groupType == ::tf::MaintainGroupType::type::MAINTAIN_GROUP_CUSTOM ? QString::fromStdString(group.groupName) : ConvertTfEnumToQString(group.groupType));
	ui->lineEdit_group_name->blockSignals(false);

	UpdateSelect();
	UpdateBtnState();
}

///
///  @brief 更新备选数
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年3月27日，新建函数
///
void QMaintainSetPage::UpdateSelect()
{
    ui->label_select->setText(QString(tr("备选（%1）")).arg(m_pMaintainItemModel->rowCount()));
}

///
///  @brief 设置指定维护项复选框状态
///
///
///  @param[in]   itemType   维护项类型
///  @param[in]   isChecked  是否选中 
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年3月27日，新建函数
///
void QMaintainSetPage::SetCheckBoxState(int itemType, bool isChecked, int isEabled/* = -1*/)
{
    int iRow = m_itemTypeData.indexOf(itemType);
    if (iRow < 0)
    {
        return;
    }
    auto item = m_pMaintainItemModel->item(iRow, 0);
    if (item == nullptr)
    {
        return;
    }

    QWidget *wgt = qobject_cast<QWidget*>(ui->tableView_maintain_item->indexWidget(item->index()));
    if (wgt == nullptr)
    {
        return;
    }

    QCheckBox* itemCheckBox = wgt->findChild<QCheckBox*>(QString("itemCheckBox"), Qt::FindChildOption::FindDirectChildrenOnly);
    if (itemCheckBox == nullptr)
    {
        return;
    }
    itemCheckBox->setChecked(isChecked);

    // 是否允许修改
    if (isEabled != -1)
    {
        itemCheckBox->setEnabled(isEabled);
    }
}

///
///  @brief 设置全部复选框状态
///
///
///  @param[in]   isChecked   是否选中
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年3月27日，新建函数
///
void QMaintainSetPage::SetAllCheckBoxState(bool isChecked)
{
    for (int i = 0; i < m_pMaintainItemModel->rowCount(); ++i)
    {
        auto item = m_pMaintainItemModel->item(i, 0);
        if (item == nullptr)
        {
            return;
        }

        QWidget *wgt = qobject_cast<QWidget*>(ui->tableView_maintain_item->indexWidget(item->index()));
        if (wgt == nullptr)
        {
            return;
        }

        QCheckBox* itemCheckBox = wgt->findChild<QCheckBox*>(QString("itemCheckBox"), Qt::FindChildOption::FindDirectChildrenOnly);
        if (itemCheckBox == nullptr)
        {
            return;
        }
        itemCheckBox->setChecked(isChecked);
    }
}

///
///  @brief 设置所有复选框是否可用（置灰）
///
///
///  @param[in]   isEnable  是否可用
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年3月27日，新建函数
///
void QMaintainSetPage::SetAllCheckBoxEnable(bool isEnable)
{
    std::shared_ptr<UserInfoManager> insUser = UserInfoManager::GetInstance();
    for (int i = 0; i < m_pMaintainItemModel->rowCount(); ++i)
    {
        QStandardItem* item = m_pMaintainItemModel->item(i, 0);
        if (item == Q_NULLPTR)
        {
            continue;
        }

        QWidget *wgt = qobject_cast<QWidget*>(ui->tableView_maintain_item->indexWidget(item->index()));
        if (wgt == Q_NULLPTR)
        {
            continue;
        }

        QCheckBox* itemCheckBox = wgt->findChild<QCheckBox*>(QString("itemCheckBox"), Qt::FindChildOption::FindDirectChildrenOnly);
        if (itemCheckBox == Q_NULLPTR)
        {
            continue;
        }

        // 如果需要设置为可勾选，需要先满足权限
        if (isEnable)
        {
            QVariant valueProp = itemCheckBox->property("itemType");
            if (valueProp.isValid())
            {
                int maintainType = valueProp.toInt();
                itemCheckBox->setEnabled(insUser->IsPermissonMaint((::tf::MaintainItemType::type)maintainType));
            }
        }
        else
        {
            itemCheckBox->setEnabled(false);
        }
    }
}

///
///  @brief 关机维护检测
///
///  @param[in]   vecGroups  维护组
///
///  @return	false:不符合要求
///
///  @par History: 
///  @li 8580/GongZhiQiang，2023年6月26日，新建函数
///
bool QMaintainSetPage::ShutdownMaintainCheck(const std::vector<tf::MaintainGroup>& vecGroups)
{
    // 查找关机维护组
    auto aimIter = std::find_if(vecGroups.begin(), vecGroups.end(), [](const ::tf::MaintainGroup & stopGrop) {return stopGrop.groupType == ::tf::MaintainGroupType::MAINTAIN_GROUP_STOP; });
    if (aimIter == vecGroups.end())
    {
        return false;
    }

    // 检查第一项是否为仪器复位
    if (aimIter->items.front().itemType != tf::MaintainItemType::MAINTAIN_ITEM_RESET)
    {
        return false;
    }

    // 检查自动关机,最多只有一项，如果有，必须在最后一项
    int autoShutCount = 0;
    for (int index = 0; index < aimIter->items.size(); index++)
    {
        if (aimIter->items[index].itemType == ::tf::MaintainItemType::MAINTAIN_ITEM_AUTO_SHUTDOWN)
        {
            autoShutCount++;

            if (index != aimIter->items.size() - 1)
            {
                return false;
            }
        }
    }
    if (autoShutCount > 1)
    {
        return false;
    }

    return true;
}

///
/// @brief  维护项目列表中是否存在指定的维护项目
///
/// @param[in]  vecItems  维护项目列表
/// @param[in]  itemType  指定的维护项
///
/// @return true 有 false 没有
///
/// @par History:
/// @li 7656/zhang.changjiang，2024年4月20日，新建函数
///
bool QMaintainSetPage::HasItemTypeInItems(const std::vector<tf::MaintainItem>& vecItems, const ::tf::MaintainItemType::type itemType)
{
    if (vecItems.empty())
    {
        return false;
    }

    bool bHas = false;
    for (const auto& iter : vecItems)
    {
        if (iter.itemType == itemType)
        {
            bHas = true;
            break;
        }
    }

    return bHas;
}

///
/// @brief  维护项目列表中指定的维护项是否唯一
///
/// @param[in]  vecItems  维护项目列表
/// @param[in]  itemType  指定的维护项
///
/// @return  true 唯一 false 不唯一
///
/// @par History:
/// @li 7656/zhang.changjiang，2024年4月20日，新建函数
///
bool QMaintainSetPage::ItemAreUnique(const std::vector<tf::MaintainItem>& vecItems, const ::tf::MaintainItemType::type itemType)
{
    int iAutoShutdownCount = 0;
    for (const auto& iter : vecItems)
    {
        if (iter.itemType == itemType)
        {
            ++iAutoShutdownCount;
        }
    }

    return iAutoShutdownCount <= 1;
}

///
/// @brief  维护项目列表中指定的维护项是否是最后一项
///
/// @param[in]  vecItems  维护项目列表
/// @param[in]  itemType  指定的维护项
///
/// @return true 在最后一项 false 不在最后一项
///
/// @par History:
/// @li 7656/zhang.changjiang，2024年4月20日，新建函数
///
bool QMaintainSetPage::ItemAreLastOne(const std::vector<tf::MaintainItem>& vecItems, const ::tf::MaintainItemType::type itemType)
{
    if (vecItems.empty())
    {
        return false;
    }

    if (vecItems.back().itemType == itemType)
    {
        return true;
    }

    return false;
}

///
/// @brief  维护项目列表中自动关机是否是第一项
///
/// @param[in]  vecItems  维护项目列表
/// @param[in]  itemType  指定的维护项
///
/// @return true 在最后一项 false 不在最后一项
///
/// @par History:
/// @li 7656/zhang.changjiang，2024年4月20日，新建函数
///
bool QMaintainSetPage::ItemAreFirstOne(const std::vector<tf::MaintainItem>& vecItems, const::tf::MaintainItemType::type itemType)
{
    if (vecItems.empty())
    {
        return false;
    }

    if (vecItems.front().itemType == itemType)
    {
        return true;
    }

    return false;
}

///
/// @brief 开机维护检查
///
/// @param[in]  vecItems  维护项
///
/// @return true：检查通过
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年6月3日，新建函数
///
bool QMaintainSetPage::StartMaintainGroupCheck(const std::vector<tf::MaintainItem>& vecItems)
{
	// 检查是否有复位
	if (!HasItemTypeInItems(vecItems, ::tf::MaintainItemType::MAINTAIN_ITEM_RESET))
	{
		ULOG(LOG_ERROR, "Reset are not in items!");
		TipDlg(tr("保存失败"), tr("组合维护[开机维护]中必须有仪器复位，且必须放到第一项！")).exec();
		return false;
	}

	// 检查复位是否是第一项
	if (!ItemAreFirstOne(vecItems, ::tf::MaintainItemType::MAINTAIN_ITEM_RESET))
	{
		ULOG(LOG_ERROR, "Reset are not the first one!");
		TipDlg(tr("保存失败"), tr("组合维护[开机维护]仪器复位不是第一项，请将仪器复位放到第一项！")).exec();
		return false;
	}

    // 免疫检查
	if(CommonInformationManager::GetInstance()->IsContainsAimClassify(::tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE))
	{
		//TODO
	}

	// 生化检查
	if (CommonInformationManager::GetInstance()->IsContainsAimClassify(::tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY))
	{
		// 排气（不添加，2024.7.9）
		/*if (!HasItemTypeInItems(vecItems, ::tf::MaintainItemType::MAINTAIN_ITEM_CH_ISE_AIR_PURGE))
		{
			TipDlg(tr("保存失败"), tr("组合维护[开机维护]中必须有排气！")).exec();
			return false;
		}*/

		// 管路填充
		if (!HasItemTypeInItems(vecItems, ::tf::MaintainItemType::MAINTAIN_ITEM_CH_FLUID_PERFUSION))
		{
			TipDlg(tr("保存失败"), tr("组合维护[开机维护]中必须有比色管路填充！")).exec();
			return false;
		}

		// 生化试剂扫描
		if (!HasItemTypeInItems(vecItems, ::tf::MaintainItemType::MAINTAIN_ITEM_CH_REAGENT_SCAN))
		{
			TipDlg(tr("保存失败"), tr("组合维护[开机维护]中必须有生化试剂扫描！")).exec();
			return false;
		}

		// 反应槽水更换
		if (!HasItemTypeInItems(vecItems, ::tf::MaintainItemType::MAINTAIN_ITEM_CH_INCUBATION_WATER_EXCHANGE))
		{
			TipDlg(tr("保存失败"), tr("组合维护[开机维护]中必须有反应槽水更换！")).exec();
			return false;
		}

		// 光度计检查
		if (!HasItemTypeInItems(vecItems, ::tf::MaintainItemType::MAINTAIN_ITEM_CH_PHOTOMETER_CHECK))
		{
			TipDlg(tr("保存失败"), tr("组合维护[开机维护]中必须有光度计检查！")).exec();
			return false;
		}
	}

	// ISE检查
	if (CommonInformationManager::GetInstance()->IsContainsAimClassify(::tf::AssayClassify::ASSAY_CLASSIFY_ISE))
	{
		// ISE管路填充
		if (!HasItemTypeInItems(vecItems, ::tf::MaintainItemType::MAINTAIN_ITEM_ISE_FLUID_PERFUSION))
		{
			TipDlg(tr("保存失败"), tr("组合维护[开机维护]中必须有ISE管路填充！")).exec();
			return false;
		}
	}

    return true;
}

///
/// @brief 关机维护检查
///
/// @param[in]  vecItems  维护项
///
/// @return true：检查通过
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年6月3日，新建函数
///
bool QMaintainSetPage::StopMaintainGroupCheck(const std::vector<tf::MaintainItem>& vecItems)
{
	// 检查是否有复位
	if (!HasItemTypeInItems(vecItems, ::tf::MaintainItemType::MAINTAIN_ITEM_RESET))
	{
		ULOG(LOG_ERROR, "Reset are not in items!");
		TipDlg(tr("保存失败"), tr("组合维护[关机维护]中必须有仪器复位，且必须放到第一项！")).exec();
		return false;
	}

	// 检查复位是否是第一项
	if (!ItemAreFirstOne(vecItems, ::tf::MaintainItemType::MAINTAIN_ITEM_RESET))
	{
		ULOG(LOG_ERROR, "Reset are not the first one!");
		TipDlg(tr("保存失败"), tr("组合维护[关机维护]仪器复位不是第一项，请将仪器复位放到第一项！")).exec();
		return false;
	}

	// 免疫检查
	if (CommonInformationManager::GetInstance()->IsContainsAimClassify(::tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE))
	{
		//TODO
	}

	// 生化检查
	if (CommonInformationManager::GetInstance()->IsContainsAimClassify(::tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY))
	{
		//// 针清洗(不添加，周晓峰，2024.8.19)
		//if (!HasItemTypeInItems(vecItems, ::tf::MaintainItemType::MAINTAIN_ITEM_CH_ISE_PROBE_CLEAN))
		//{
		//	TipDlg(tr("保存失败"), tr("组合维护[关机维护]中必须有针清洗！")).exec();
		//	return false;
		//}
	}

	// ISE检查
	if (CommonInformationManager::GetInstance()->IsContainsAimClassify(::tf::AssayClassify::ASSAY_CLASSIFY_ISE))
	{
		//// 针清洗(不添加，周晓峰，2024.8.19)
		//if (!HasItemTypeInItems(vecItems, ::tf::MaintainItemType::MAINTAIN_ITEM_CH_ISE_PROBE_CLEAN))
		//{
		//	TipDlg(tr("保存失败"), tr("组合维护[关机维护]中必须有针清洗！")).exec();
		//	return false;
		//}
	}

	return true;
}

///
/// @brief 周维护检查
///
/// @param[in]  vecItems  维护项
///
/// @return true：检查通过
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年6月3日，新建函数
///
bool QMaintainSetPage::WeekMaintainGroupCheck(const std::vector<tf::MaintainItem>& vecItems)
{
	// 免疫检查
	if (CommonInformationManager::GetInstance()->IsContainsAimClassify(::tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE))
	{
		//TODO
	}

	// 生化检查
	if (CommonInformationManager::GetInstance()->IsContainsAimClassify(::tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY))
	{
		// 反应杯清洗
		if (!HasItemTypeInItems(vecItems, ::tf::MaintainItemType::MAINTAIN_ITEM_CH_WASH_REACTION_CUP))
		{
			TipDlg(tr("保存失败"), tr("组合维护[周维护]中必须有反应杯清洗！")).exec();
			return false;
		}

		// 杯空白测定
		if (!HasItemTypeInItems(vecItems, ::tf::MaintainItemType::MAINTAIN_ITEM_CH_WATER_BLANK))
		{
			TipDlg(tr("保存失败"), tr("组合维护[周维护]中必须有杯空白测定！")).exec();
			return false;
		}
	}

	// ISE检查
	if (CommonInformationManager::GetInstance()->IsContainsAimClassify(::tf::AssayClassify::ASSAY_CLASSIFY_ISE))
	{

	}

	return true;
}

///
/// @brief 维护组检查
///
/// @param[in]  maintainGroup  维护组
///
/// @return true：检查通过
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年6月3日，新建函数
///
bool QMaintainSetPage::MaintainGroupCheck(const tf::MaintainGroup& maintainGroup)
{
	// 维护组名称检查
	if (maintainGroup.groupName.empty())
	{
		ULOG(LOG_ERROR, "Groups name have a null!");
		TipDlg(tr("保存失败"), tr("存在空维护组名称，请输入一个有效的维护组名称！")).exec();
		return false;
	}

	// 维护项不能为空
	if (maintainGroup.items.empty())
	{
		ULOG(LOG_ERROR, "Group items have a null!");
		TipDlg(tr("保存失败"), tr("组合维护[%1]没有选择维护项，请先勾选维护项！").arg(MaintainGroupModel::GetGroupName(maintainGroup))).exec();
		return false;
	}

	// 检查清除样本架是否在复位前面
	if (!IsBefore(maintainGroup.items, ::tf::MaintainItemType::MAINTAIN_ITEM_RESET, ::tf::MaintainItemType::MAINTAIN_ITEM_CLEAN_TRACK))
	{
		ULOG(LOG_ERROR, "The MAINTAIN_ITEM_CLEAN_TRACK before MAINTAIN_ITEM_RESET!");
		TipDlg(tr("保存失败"), tr("组合维护[%1]中维护项目[%2]不能在[%3]的前面！")
			.arg(MaintainGroupModel::GetGroupName(maintainGroup))
			.arg(ConvertTfEnumToQString(::tf::MaintainItemType::MAINTAIN_ITEM_CLEAN_TRACK))
			.arg(ConvertTfEnumToQString(::tf::MaintainItemType::MAINTAIN_ITEM_RESET))).exec();
		return false;
	}

	// 仪器关机维护项特殊检查（唯一且必须在最后一项）
	if (HasItemTypeInItems(maintainGroup.items, ::tf::MaintainItemType::MAINTAIN_ITEM_AUTO_SHUTDOWN))
	{
		// 检查仪器关机的唯一性
		if (!ItemAreUnique(maintainGroup.items, ::tf::MaintainItemType::MAINTAIN_ITEM_AUTO_SHUTDOWN))
		{
			ULOG(LOG_ERROR, "Auto shutdown are not unique!");
			TipDlg(tr("保存失败"), tr("组合维护[%1]存在多个仪器关机维护！每个组合维护仪器关机维护项有且仅有一项，必须在最后一项！").arg(MaintainGroupModel::GetGroupName(maintainGroup))).exec();
			return false;
		}

		// 检查仪器关机是否是最后一项
		if (!ItemAreLastOne(maintainGroup.items, ::tf::MaintainItemType::MAINTAIN_ITEM_AUTO_SHUTDOWN))
		{
			ULOG(LOG_ERROR, "Auto shutdown are not unique!");
			TipDlg(tr("保存失败"), tr("组合维护[%1]仪器关机维护项不在最后一项！每个组合维护仪器关机维护项必须在最后一项！").arg(MaintainGroupModel::GetGroupName(maintainGroup))).exec();
			return false;
		}
	}

	// 根据维护项进行分发检查
	switch (maintainGroup.groupType)
	{
	// 开机维护检查
	case ::tf::MaintainGroupType::MAINTAIN_GROUP_START:
	{
		if (!StartMaintainGroupCheck(maintainGroup.items))
		{
			ULOG(LOG_ERROR, "StartMaintainGroupCheck error!");
			return false;
		}

	}break;

	// 关机维护检查
	case ::tf::MaintainGroupType::MAINTAIN_GROUP_STOP:
	{
		if (!StopMaintainGroupCheck(maintainGroup.items))
		{
			ULOG(LOG_ERROR, "StopMaintainGroupCheck error!");
			return false;
		}

	}break;

	// 周维护检查
	case ::tf::MaintainGroupType::MAINTAIN_GROUP_WEEK:
	{
		if (!WeekMaintainGroupCheck(maintainGroup.items))
		{
			ULOG(LOG_ERROR, "WeekMaintainGroupCheck error!");
			return false;
		}

	}break;

	default:
		break;
	}

	return true;
}

///
///  @brief 刷全部清除按钮的位置
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年3月7日，新建函数
///
void QMaintainSetPage::UpdateClearBtnPos()
{
    if (ui->tableView_group_cfg->verticalScrollBar()->isVisible())
    {
        ui->cleanBtn->move(1198, 190);
    }
    else
    {
        ui->cleanBtn->move(1198, 190);
    }
}

///
///  @brief 刷搜索框的位置
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年3月7日，新建函数
///
void QMaintainSetPage::UpdateSearchBoxPos()
{
    if (ui->tableView_maintain_item->verticalScrollBar()->isVisible())
    {
        ui->frame_searchBox->move(721, 187);
    }
    else
    {
        ui->frame_searchBox->move(721, 187);
    }
}

///
///  @brief	维护组新增按钮
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年9月6日，新建函数
///
void QMaintainSetPage::OnAddNewBtnClicked()
{
    ULOG(LOG_INFO, __FUNCTION__);

    if (MaintainGroupModel::Instance().IsExistEmptyNameGroup())
    {
        TipDlg(tr("提示"), tr("已存在新增的未命名的维护组，需保存后再进行新增。")).exec();
        return;
    }

    // 获取DCS客户端
    std::shared_ptr<DcsControlProxy> spCilent = DcsControlProxy::GetInstance();
    if (spCilent == nullptr)
    {
        ULOG(LOG_ERROR, "DcsControlProxy::GetInstance() Failed!");
        return;
    }

    // 新增维护组
    tf::MaintainGroup group;
    group.__set_groupType(tf::MaintainGroupType::type::MAINTAIN_GROUP_CUSTOM);
    group.__set_groupName(std::string());
    group.__set_items(std::vector<tf::MaintainItem>());

    std::shared_ptr<tf::UserInfo> userInfo = UserInfoManager::GetInstance()->GetLoginUserInfo();
    group.__set_creatUserType(userInfo == nullptr ? -1 : userInfo->type);
    MaintainGroupModel::Instance().AddGroupData(group);

    // 选中新增后的一行
    int iCurRow = m_pMaintainGroupModel->rowCount() - 1;
    ui->tableView_mantain_group->selectRow(iCurRow);
    m_pMaintainGroupModel->SetSelectedRow(iCurRow);

    // 启用组合名称输入框
    // ui->lineEdit_group_name->setEnabled(true);

    // 将光标放到组合名称输入框
    ui->lineEdit_group_name->setFocus();

    // 新增按钮置灰
    ui->add_new_btn->setEnabled(false);

    // 维护组表滚动条拉新增的行
    ui->tableView_mantain_group->scrollTo(m_pMaintainGroupModel->index(iCurRow, 0));

    // 将维护组配置表切换到当前新增的维护组
    RefreshGroupCfg(iCurRow);
    UpdateBtnState();
}

///
///  @brief	维护组删除按钮
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年9月6日，新建函数
///
void QMaintainSetPage::OnDeleteBtnClicked()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 获取DCS客户端
    std::shared_ptr<DcsControlProxy> spCilent = DcsControlProxy::GetInstance();
    if (spCilent == nullptr)
    {
        ULOG(LOG_ERROR, "DcsControlProxy::GetInstance() Failed!");
        return;
    }

    // 检查是否有数据可以删除
    auto data = MaintainGroupModel::Instance().GetData();
    if (data.size() == 0)
    {
        ULOG(LOG_ERROR, "Data is null!");
        TipDlg(tr("删除失败"), tr("数据为空！")).exec();
        return;
    }

    // 检查是否有选中的数据
    auto selectedIndex = ui->tableView_mantain_group->selectionModel()->selectedRows();
    if (selectedIndex.empty())
    {
        ULOG(LOG_ERROR, "Data is null!");
        TipDlg(tr("删除失败"), tr("请先选择你要删除的维护组合！")).exec();
        return;
    }

    // 正在执行的维护组
    std::vector<::tf::MaintainGroup> lstRunnigMg;
    // 系统维护组
    std::vector<::tf::MaintainGroup> lstSystemMg;
    // 可删除的维护组
    std::vector<::tf::MaintainGroup> lstMg;
    // 可删除维护组ID
    std::vector<std::int64_t> vecIds;
    for (auto index : selectedIndex)
    {
        ::tf::MaintainGroup group;
        if (!m_pMaintainGroupModel->GetGroupByIndex(group, index))
        {
            ULOG(LOG_ERROR, "GetGroupByIndex() Failed!");
            return;
        }

        // 检查维护组是否是系统组合
        if (group.groupType != ::tf::MaintainGroupType::MAINTAIN_GROUP_CUSTOM)
        {
            ULOG(LOG_ERROR, "Delete system group type Failed!");
            lstSystemMg.push_back(group);
            continue;
        }

        // 检查是否有仪器在执行该维护组合
        if (HasDeviceInExecting(group))
        {
            ULOG(LOG_ERROR, "Has device in executing!");
            lstRunnigMg.push_back(group);
            continue;
        }

        if (!group.__isset.id)
        {

            ui->add_new_btn->setEnabled(true);
        }
        else
        {
            vecIds.push_back(group.id);
        }

        lstMg.push_back(std::move(group));
    }

    QString strTip;
    if (!lstSystemMg.empty())
    {
        for (auto iter = lstSystemMg.begin(); iter != lstSystemMg.end(); ++iter)
        {
            strTip += QString(iter != --lstSystemMg.end() ? tr("[%1]、").arg(MaintainGroupModel::GetGroupName(*iter)) : QString(tr("[%1]").arg(MaintainGroupModel::GetGroupName(*iter))));
        }
        strTip += QString(tr("是系统维护组合，不可删除！"));

    }

    if (!lstRunnigMg.empty())
    {
        for (auto iter = lstRunnigMg.begin(); iter != lstRunnigMg.end(); ++iter)
        {
            strTip += QString(iter != --lstRunnigMg.end() ? tr("[%1]、").arg(MaintainGroupModel::GetGroupName(*iter)) : QString(tr("[%1]").arg(MaintainGroupModel::GetGroupName(*iter))));
        }
        strTip += QString(tr("正在执行，不可删除！"));

    }

    if (!strTip.isEmpty())
    {
        if (lstMg.empty())
        {
            TipDlg(tr("提示"), strTip).exec();
            return;
        }
        strTip += tr("是否删除选中的");
        for (auto iter = lstMg.begin(); iter != lstMg.end(); ++iter)
        {
            strTip += QString(iter != --lstMg.end() ? tr("[%1]、").arg(MaintainGroupModel::GetGroupName(*iter)) : QString(tr("[%1]").arg(MaintainGroupModel::GetGroupName(*iter))));
        }
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("提示"), strTip + tr("维护组合?"), TipDlgType::TWO_BUTTON));
        if (pTipDlg->exec() == QDialog::Rejected)
        {
            ULOG(LOG_INFO, "Cancel remove group!");
            return;
        }
    }
    else
    {
        // 提示用户是否删除
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("删除"), tr("是否删除选中的维护组合?"), TipDlgType::TWO_BUTTON));
        if (pTipDlg->exec() == QDialog::Rejected)
        {
            ULOG(LOG_INFO, "Cancel remove group!");
            return;
        }
    }

    if (!vecIds.empty())
    {
        // 删除数据库中的数据
        ::tf::MaintainGroupQueryCond qryCond;
        qryCond.__set_ids(vecIds);
        if (!spCilent->DeleteMaintainGroup(qryCond))
        {
            ULOG(LOG_ERROR, "DeleteMaintainGroups Failed!");
            TipDlg(tr("删除失败"), tr("删除维护组失败！")).exec();
            return;
        }
    }

    // 获取删除之前选中的行索引
    auto selectedIndexs = ui->tableView_mantain_group->selectionModel()->selectedRows();

    // 删除选中的维护组
    MaintainGroupModel::Instance().DeleteGroups(lstMg);

    // 计算删除后选中的行
    int iSelectedRow = 0;
    if (selectedIndexs.empty())
    {
        iSelectedRow = m_pMaintainGroupModel->rowCount() - 1;
    }
    else
    {
        // 找出最上面的选中行（即行号最小的行，因为多选的时候获取的selectedIndexs列表是按照用户点击选中的顺序排列的）
        int iFirstDelRow = selectedIndexs.first().row();
        for (int i = 1; i < selectedIndexs.size(); ++i)
        {
            int iRow = selectedIndexs[i].row();
            if (iFirstDelRow > iRow)
            {
                iFirstDelRow = iRow;
            }
        }

        iSelectedRow = (iFirstDelRow == 0 ? iFirstDelRow : --iFirstDelRow) + lstSystemMg.size() + lstRunnigMg.size();
    }

    // 刷新选中的行
    RefreshGroupCfg(iSelectedRow);
    ui->tableView_mantain_group->selectRow(iSelectedRow);

    MaintainGroupModel::Instance().BackupCurrentData();
    // 通知维护组组合设置更新
    POST_MESSAGE(MSG_ID_MAINTAIN_GROUP_CFG_UPDATE);
}

///
///  @brief	保存按钮
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年9月6日，新建函数
///
void QMaintainSetPage::OnSaveBtnClicked()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 获取DCS客户端
	std::shared_ptr<DcsControlProxy> spCilent = DcsControlProxy::GetInstance();
	if (spCilent == nullptr)
	{
		ULOG(LOG_ERROR, "DcsControlProxy::GetInstance() Failed!");
		return;
	}

	// 获取维护组列表
	std::vector<tf::MaintainGroup>& vecGroups = MaintainGroupModel::Instance().GetData();
	if (vecGroups.empty())
	{
		ULOG(LOG_ERROR, "vecGroups is empty!");
		return;
	}

	// 检查维护组列表中是否有空名称和空配置
	std::vector<int> newGroupIndex;
	for (int groupIndex = 0; groupIndex < vecGroups.size(); groupIndex++)
	{
		// 过滤掉单机维护
		if (vecGroups[groupIndex].groupType == ::tf::MaintainGroupType::type::MAINTAIN_GROUP_SINGLE)
		{
			continue;
		}

		// 检查维护组列表中是否有空名称和空配置,并且检查维护组内容
		if (!MaintainGroupCheck(vecGroups[groupIndex]))
		{
			ULOG(LOG_ERROR, "MaintainGroupCheck error!");
			return;
		}

		// 检查新增维护组名称是否重复
		if (!vecGroups[groupIndex].__isset.id)
		{
			// 往后遍历检查是否冲突
			for (int checkIndex = 0; checkIndex < vecGroups.size(); checkIndex++)
			{
				// 相同索引跳过
				if (checkIndex == groupIndex)
				{
					continue;
				}

				// 如果名称相同
				if (vecGroups[checkIndex].groupName == vecGroups[groupIndex].groupName)
				{
					ULOG(LOG_WARN, "The same group name conflict!");
					if (vecGroups[checkIndex].groupType == ::tf::MaintainGroupType::type::MAINTAIN_GROUP_SINGLE)
					{
						TipDlg(tr("新增失败"), tr("组合维护[%1]与单项维护命名重复！").arg(QString::fromStdString(vecGroups[checkIndex].groupName))).exec();
					}
					else
					{
						TipDlg(tr("新增失败"), tr("组合维护[%1]重复命名！").arg(QString::fromStdString(vecGroups[checkIndex].groupName))).exec();
					}
					return;
				}
			}

			// 添加到新增索引
			newGroupIndex.push_back(groupIndex);
		}
	}

	// 添加新的维护组
	if (!newGroupIndex.empty())
	{
		for (auto& newIndex : newGroupIndex)
		{
			tf::ResultLong ret;
			// 新增维护组
			if (!spCilent->AddMaintainGroup(ret, vecGroups[newIndex]))
			{
				ULOG(LOG_ERROR, "AddMaintainGroup Failed!");
				TipDlg(tr("新增失败"), tr("新增维护组失败！")).exec();
				return;
			}
			else
			{
				vecGroups[newIndex].__set_id(ret.value);
				// 启用新增按钮
				ui->add_new_btn->setEnabled(true);
				// 刷新维护组表格
				ui->tableView_mantain_group->update();
			}
		}
	}

	// 更新数据库的维护组数据
	if (!spCilent->ModifyMaintainGroups(vecGroups))
	{
		ULOG(LOG_ERROR, "ModifyMaintainGroups  Failed!");
		TipDlg(tr("保存失败"), tr("保存维护组失败！")).exec();
		return;
	}

	// 同步维护日志显示数据
	MaintainLogModel::Instance().SetVorList(vecGroups);
    MaintainGroupModel::Instance().BackupCurrentData();

	// 通知维护组组合设置更新
	POST_MESSAGE(MSG_ID_MAINTAIN_GROUP_CFG_UPDATE);
}

///
///  @brief 维护项搜索按钮
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年9月6日，新建函数
///
void QMaintainSetPage::OnSerachItemBtnClicked()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 查询所有的维护项
    std::vector<::tf::MaintainItem> items;
    QueryAllItemData(items);
    if (items.empty())
    {
        return;
    }

    // 过滤不显示的维护项
    FilterItems(items);

    // 找出和用户输入匹配的维护项
    SearchItems(items);

    // 初始化维护项表数据
    InitItemTableData(items);

    // 刷新当前选中的数据
    if (!MaintainGroupModel::Instance().GetData().empty())
    {
        RefreshGroupCfg(m_pMaintainGroupModel->GetSelectedRow(), items);
        ui->tableView_mantain_group->selectRow(m_pMaintainGroupModel->GetSelectedRow());
    }
}

///
///  @brief 全部清除按钮
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年2月28日，新建函数
///
void QMaintainSetPage::OnCleanBtnClicked()
{
    ULOG(LOG_INFO, __FUNCTION__);

    tf::MaintainGroup sGroup;
    if (!MaintainGroupModel::Instance().GetSelectData(sGroup))
    {
        ULOG(LOG_WARN, "Failed to get selected maintaingroup.");
        return;
    }
    // 如果当前选中的维护组是单项维护组,则不处理
    if (sGroup.groupType == ::tf::MaintainGroupType::type::MAINTAIN_GROUP_SINGLE)
    {
        ULOG(LOG_INFO, "Not need to process single group.");
        return;
    }
    m_pCfgModel->ClearData();
    MaintainGroupModel::Instance().CleanSelectedGroupItems();
    SetAllCheckBoxState(false);
    UpdateBtnState();
}

///
///  @brief 置顶按钮
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年2月14日，新建函数
///
void QMaintainSetPage::OnTopBtnClicked()
{
    ULOG(LOG_INFO, __FUNCTION__);

    if (MaintainGroupCfgModel::Instance().GetRows() == 0)
    {
        ULOG(LOG_WARN, "Empty rows.");
        return;
    }
    auto index = ui->tableView_group_cfg->currentIndex();
    if (!index.isValid())
    {
        ULOG(LOG_INFO, "Invalid index.");
        return;
    }
    int row = 0;
    m_pCfgModel->MoveItem(index.row(), row);
    MaintainGroupModel::Instance().SetSelectedGroup(MaintainGroupCfgModel::Instance().GetData());
    ui->tableView_group_cfg->selectRow(row);
    UpdateBtnState();
}

///
///  @brief 向上按钮
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年2月14日，新建函数
///
void QMaintainSetPage::OnUpBtnClicked()
{
    ULOG(LOG_INFO, __FUNCTION__);

    auto index = ui->tableView_group_cfg->currentIndex();
    if (!index.isValid())
    {
        return;
    }
    int row = index.row() - 1;
    if (row < 0)
    {
        row = 0;
    }
    m_pCfgModel->MoveItem(index.row(), row);
    MaintainGroupModel::Instance().SetSelectedGroup(MaintainGroupCfgModel::Instance().GetData());
    ui->tableView_group_cfg->selectRow(row);
    UpdateBtnState();
}

///
///  @brief 向下
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年2月14日，新建函数
///
void QMaintainSetPage::OnDownBtnClicked()
{
    ULOG(LOG_INFO, __FUNCTION__);

    auto index = ui->tableView_group_cfg->currentIndex();
    if (!index.isValid())
    {
        return;
    }
    int row = index.row() + 1;
    int size = MaintainGroupCfgModel::Instance().GetRows();
    if (row >= size)
    {
        row = size - 1;
    }

    m_pCfgModel->MoveItem(index.row(), row);
    MaintainGroupModel::Instance().SetSelectedGroup(MaintainGroupCfgModel::Instance().GetData());
    ui->tableView_group_cfg->selectRow(row);
    UpdateBtnState();
}

///
///  @brief 置底按钮
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年2月14日，新建函数
///
void QMaintainSetPage::OnBottomBtnClicked()
{
    ULOG(LOG_INFO, __FUNCTION__);

    auto index = ui->tableView_group_cfg->currentIndex();
    if (!index.isValid())
    {
        return;
    }
    int row = m_pCfgModel->rowCount() - 1;
    m_pCfgModel->MoveItem(index.row(), row);
    MaintainGroupModel::Instance().SetSelectedGroup(MaintainGroupCfgModel::Instance().GetData());
    ui->tableView_group_cfg->selectRow(row);
    UpdateBtnState();
}

///
///  @brief 重置按钮
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年8月15日，新建函数
///
void QMaintainSetPage::OnResetBtnClicked()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 获取选中的索引值列表
    auto selectedIndexs = ui->tableView_mantain_group->selectionModel()->selectedRows();
    if (selectedIndexs.empty())
    {
        TipDlg(tr("提示"), tr("请先选择一个维护组！")).exec();
        return;
    }

    // 获取选中的维护组的id
    std::vector<int64_t> vecGroupId;
    for (const QModelIndex& it : selectedIndexs)
    {
        if (!it.isValid())
        {
            continue;
        }
        ::tf::MaintainGroup group;
        if (!m_pMaintainGroupModel->GetGroupByIndex(group, it))
        {
            continue;
        }
        if (group.__isset.id)
        {
            vecGroupId.push_back(group.id);
        }
    }

    // 检查数据获取是否成功
    if (vecGroupId.empty())
    {
        return;
    }

    // 获取DCS客户端
    std::shared_ptr<DcsControlProxy> spCilent = DcsControlProxy::GetInstance();
    if (spCilent == nullptr)
    {
        return;
    }

    // 查询维护组
    ::tf::MaintainGroupQueryResp retGroup;
    ::tf::MaintainGroupQueryCond miqcGroup;
    miqcGroup.__set_ids(vecGroupId);
    if (!spCilent->QueryMaintainGroup(retGroup, miqcGroup))
    {
        ULOG(LOG_WARN, "Failed to query maintain group.");
        return;
    }

    MaintainGroupModel& gGroupModel = MaintainGroupModel::Instance();
    for (const tf::MaintainGroup& it : retGroup.lstMaintainGroups)
    {
        gGroupModel.UpdateGroup(it);
    }

    RefreshGroupCfg(m_pMaintainGroupModel->GetSelectedRow());
    for (const QModelIndex& it : selectedIndexs)
    {
        if (!it.isValid())
        {
            continue;
        }
        ui->tableView_mantain_group->selectRow(it.row());
    }
}

///
///  @brief 更新按钮状态
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年3月13日，新建函数
///
void QMaintainSetPage::UpdateBtnState()
{
    ui->upBtn->setEnabled(false);
    ui->downBtn->setEnabled(false);
    ui->topBtn->setEnabled(false);
    ui->bottomBtn->setEnabled(false);

    int rowCount = m_pCfgModel->rowCount();
    auto currentIndex = ui->tableView_group_cfg->currentIndex();
    if (!currentIndex.isValid() || rowCount <= 1)
    {
        return;
    }
    int currentRow = currentIndex.row();

    // 第一行
    if (currentRow == 0)
    {
        ui->bottomBtn->setEnabled(true);
        ui->downBtn->setEnabled(true);
    }
    // 最后一行
    else if (currentRow == (rowCount - 1))
    {
        ui->topBtn->setEnabled(true);
        ui->upBtn->setEnabled(true);
    }
    else
    {
        ui->downBtn->setEnabled(true);
        ui->upBtn->setEnabled(true);
        ui->bottomBtn->setEnabled(true);
        ui->topBtn->setEnabled(true);
    }
}

///
///  @brief:	关闭按钮被点击
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年8月29日，新建函数
///
void QMaintainSetPage::OnCloseBtnClicked()
{
    ULOG(LOG_INFO, __FUNCTION__);

    MaintainGroupModel::Instance().RecoverBackupData();
    this->hide();
}

///
///  @brief 复选框的槽函数（点击复选框）
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年3月27日，新建函数
///
void QMaintainSetPage::OnCheckBoxClicked()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 如不能获取对应的控件，则返回
    auto sendSrc = qobject_cast<QCheckBox*>(sender());
    if (sendSrc == Q_NULLPTR)
    {
        ULOG(LOG_WARN, "Null object ptr.");
        return;
    }
    int itemType = sendSrc->property("itemType").toInt();

    m_pMaintainGroupModel->ConfigData(sendSrc->isChecked(), m_itemDataMap[itemType]);
    m_pCfgModel->ConfigData(sendSrc->isChecked(), m_itemDataMap[itemType]);
}

///
///  @brief 选项设置按钮点击槽函数
///
///  @return	
///
///  @par History: 
///  @li 8580/GongZhiQiang，2023年6月26日，新建函数
///
void QMaintainSetPage::OnOptionItemSetClicked()
{
    ULOG(LOG_INFO, __FUNCTION__);

    if (m_pMaintainDefaultSetDlg == nullptr)
    {
        m_pMaintainDefaultSetDlg = new MaintainDefaultSetDlg(this);
        connect(m_pMaintainDefaultSetDlg, &MaintainDefaultSetDlg::ModifiedMaintainItem, this, [=]() {
            // 更新配置项
            if (!m_pCfgModel->ModifyItemByIndex(ui->tableView_group_cfg->currentIndex(), m_pMaintainDefaultSetDlg->GetModifiedMaintainItem()))
            {
                ULOG(LOG_INFO, "%s(),ModifyItemByIndex Fail!", __FUNCTION__);
                return;
            }

            // 更新组信息
            MaintainGroupModel::Instance().SetSelectedGroup(MaintainGroupCfgModel::Instance().GetData());
            UpdateBtnState();
        });
    }

    auto selectedIndexs = ui->tableView_group_cfg->selectionModel()->selectedRows();
    if (selectedIndexs.isEmpty())
    {
        TipDlg(tr("提示"), tr("请在已选列表中选择一项！")).exec();
        return;
    }

    auto currentIndex = selectedIndexs.last();
    MaintainGroupCfgModel::ItemData itemData;
    if (!m_pCfgModel->GetItemByIndex(currentIndex, itemData))
    {
        ULOG(LOG_INFO, "%s(),GetItemByIndex Fail!", __FUNCTION__);
        return;
    }

    // 加载数据,设置维护项和单项维护参数
    m_pMaintainDefaultSetDlg->LoadDataToCtrls(itemData);

    m_pMaintainDefaultSetDlg->show();
}

void QMaintainSetPage::OnPermisionChanged()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    std::shared_ptr<UserInfoManager> userPms = UserInfoManager::GetInstance();
    bool maintGroup = userPms->IsPermisson(PSM_IM_MANT_GROUP); // 是否允许设置组合维护

    ui->add_new_btn->setEnabled(maintGroup);
    ui->delete_btn->setEnabled(maintGroup);

    // 维护项勾选权限
    SetAllCheckBoxEnable(maintGroup);

    ui->tableView_group_cfg->setEnabled(maintGroup);
    ui->cleanBtn->setEnabled(maintGroup);
    ui->topBtn->setEnabled(maintGroup);
    ui->upBtn->setEnabled(maintGroup);
    ui->downBtn->setEnabled(maintGroup);
    ui->bottomBtn->setEnabled(maintGroup);
    ui->save_btn->setEnabled(maintGroup);
}

///
///  @brief 过滤ui不显示的维护项
///
///
///  @param[out]   items  维护项列表
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年8月17日，新建函数
///
void QMaintainSetPage::FilterItems(std::vector<::tf::MaintainItem>& outItems, const std::shared_ptr<tf::MaintainGroup>& groupLimit/* = nullptr*/)
{
    std::shared_ptr<UserInfoManager> insUser = UserInfoManager::GetInstance();

    outItems.erase(std::remove_if(outItems.begin(), outItems.end(), [&](::tf::MaintainItem &item) {
        // 删除指定位置试剂扫描单项维护组
        if (item.itemType == ::tf::MaintainItemType::type::MAINTAIN_ITEM_REAGENT_SCAN_POS ||
            item.itemType == ::tf::MaintainItemType::type::MAINTAIN_ITEM_REACTTRANSTRAY_RESET /*||
            // 要求显示整机针特殊清洗 by ldx 20230913
            item.itemType == ::tf::MaintainItemType::type::MAINTAIN_ITEM_SOAK_SAMPLE_WASH_PIPE*/)
        {
            return true;
        }

        // 生化组合维护不支持半自动维护项
        if (item.itemType == ::tf::MaintainItemType::type::MAINTAIN_ITEM_CH_CHANGE_REACTION_CUP ||
            item.itemType == ::tf::MaintainItemType::type::MAINTAIN_ITEM_CH_WASH_REACTION_PARTS ||
            item.itemType == ::tf::MaintainItemType::type::MAINTAIN_ITEM_CH_DRAIN_BUCKET ||
            item.itemType == ::tf::MaintainItemType::type::MAINTAIN_ITEM_CH_CLEAN_LIQUID_PATH ||
            item.itemType == ::tf::MaintainItemType::type::MAINTAIN_ITEM_ISE_CLEAN_FLUID_PATH ||
            item.itemType == ::tf::MaintainItemType::type::MAINTAIN_ITEM_ISE_CHANGE_ELECTRODE ||
			item.itemType == ::tf::MaintainItemType::type::MAINTAIN_ITEM_ISE_CLEAN_ELECTRODE  // 电极清洗不支持放到维护组中2024.7.24
			)
        {
            return true;
        }

        // 删除不支持该维护单项
        if (!gUiAdapterPtr()->WhetherSupportTargetMaintainItem(item))
        {
            return true;
        }

        // 联机版没有进样器，不显示进样器复位
        if (item.itemType == ::tf::MaintainItemType::MAINTAIN_ITEM_SAMPLE_HANDLE_SYS_RESET && DictionaryQueryManager::GetInstance()->GetPipeLine())
        {
            return true;
        }

        // 当前用户是否允许显示该维护项
        if (!insUser->IsPermissonMaint(item.itemType))
        {
            if (groupLimit != nullptr)
            {
                // 如果在维护组中存在，就不删除（但是要置灰）
                bool isExistInGroup = false;
                for (const ::tf::MaintainItem& gIt : groupLimit->items)
                {
                    if (gIt.itemType == item.itemType)
                    {
                        isExistInGroup = true;
                        break;
                    }
                }
                return !isExistInGroup;
            }
            else
            {
                // 如果没有指定维护组限制，则直接使用权限限制
                return true;
            }
        }

        return false;
    }), outItems.end());
}

///
///  @brief 找出和用户输入内容匹配的所有维护项
///
///
///  @param[out]   outItems  维护项列表
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年10月13日，新建函数
///
void QMaintainSetPage::SearchItems(std::vector<::tf::MaintainItem>& outItems)
{
    QString userInput = m_pSearchLineEdit->text();
    if (userInput.isEmpty())
    {
        return;
    }

    QRegularExpression regex(userInput);

    for (auto iter = outItems.begin(); iter != outItems.end();)
    {
        auto strItemName = ConvertTfEnumToQString(iter->itemType);
        if (!regex.match(strItemName).hasMatch())
        {
            iter = outItems.erase(iter);
        }
        else
        {
            ++iter;
        }
    }
}

bool QMaintainSetPage::IsMaintainListChanged(const std::vector<::tf::MaintainItem>& vecItems)
{
    std::set<std::string> tempCurrentNames;
    for (const ::tf::MaintainItem& itm : vecItems)
    {
        tempCurrentNames.insert(itm.itemName);
    }

    // 正向比较
    std::set<std::string> oldNames;
    for (int i = 0; i < m_pMaintainItemModel->rowCount(); ++i)
    {
        QStandardItem* pItem = m_pMaintainItemModel->item(i, 1);
        if (pItem != Q_NULLPTR)
        {
            std::string strName = pItem->text().toStdString();
            if (tempCurrentNames.find(strName) == tempCurrentNames.end())
            {
                return true;
            }
            oldNames.insert(strName);
        }
    }

    if (tempCurrentNames.size() != oldNames.size())
    {
        return true;
    }
    // 反向比较
    for (const std::string& strItem : tempCurrentNames)
    {
        if (oldNames.find(strItem) == oldNames.end())
        {
            return true;
        }
    }

    return false;
}

///
/// @brief  在维护组合中itemType1是否在itemType2前面
///
/// @param[in]  vecItems   维护组合
/// @param[in]  itemType1  维护项类型1
/// @param[in]  itemType2  维护项类型1
///
/// @return true itemType1在itemType2前面 false itemType1不在itemType2前面
///
/// @par History:
/// @li 7656/zhang.changjiang，2024年6月27日，新建函数
///
bool QMaintainSetPage::IsBefore(const std::vector<::tf::MaintainItem>& vecItems, const ::tf::MaintainItemType::type itemType1, const ::tf::MaintainItemType::type itemType2)
{
	int iIdx1 = -1;
	int iIdx2 = -1;
	bool found1 = false;
	bool found2 = false;

	for (int i = 0; i < vecItems.size(); ++i)
	{
		if (vecItems[i].itemType == itemType1)
		{
			iIdx1 = i;
			found1 = true;
		}

		if (vecItems[i].itemType == itemType2)
		{
			iIdx2 = i;
			found2 = true;
		}

		if (found1 && found2)
		{
			break;
		}
	}

    if (!found1 || !found2)
    {
        return true;
    }

	return (iIdx1 <= iIdx2);
}

///
///  @brief 初始化维护项表格数据
///
///
///  @param[in]   items  维护项列表
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年10月13日，新建函数
///
void QMaintainSetPage::InitItemTableData(const std::vector<::tf::MaintainItem>& items)
{
    // 判断是否存在变更
    if (!IsMaintainListChanged(items))
    {
        return;
    }

    // 清空数据
    m_pMaintainItemModel->removeRows(0, m_pMaintainItemModel->rowCount());
    m_itemTypeData.clear();
    m_itemDataMap.clear();
    m_pMaintainItemModel->setHorizontalHeaderLabels({ "","" });

    QString strItemName;
    for (int i = 0; i < items.size(); ++i)
    {
        // 设置行位置
        m_pMaintainItemModel->setItem(i, 0, new QStandardItem(""));
        strItemName = ConvertTfEnumToQString(items[i].itemType);
        // 构建维护项类型与维护项名称映射表
        m_itemDataMap[items[i].itemType] = items[i];
        m_itemTypeData.push_back(items[i].itemType);
        QWidget *widget = new QWidget(ui->tableView_maintain_item);
        QCheckBox *checkbox = new QCheckBox(widget);
        checkbox->setObjectName(QStringLiteral("itemCheckBox"));
        checkbox->setProperty("itemType", items[i].itemType);
        QHBoxLayout *hLayout = new QHBoxLayout();

        hLayout->addWidget(checkbox);
        hLayout->setAlignment(checkbox, Qt::AlignCenter);
        hLayout->setMargin(0);
        widget->setLayout(hLayout);
        ui->tableView_maintain_item->setIndexWidget(ui->tableView_maintain_item->model()->index(i, 0), widget);
        connect(checkbox, SIGNAL(clicked()), this, SLOT(OnCheckBoxClicked()));
        // 设置名称
        m_pMaintainItemModel->setItem(i, 1, new QStandardItem(strItemName));
    }
}

///
///  @brief 查询所有维护项数
///
///
///  @param[out]   outItems  维护项列表
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年10月13日，新建函数
///
void QMaintainSetPage::QueryAllItemData(std::vector<::tf::MaintainItem>& outItems)
{
    ULOG(LOG_INFO, __FUNCTION__);

    if (m_vecMaintainItemsCache.size() > 0)
    {
        outItems = m_vecMaintainItemsCache;
        return;
    }

    // 获取DCS客户端
    std::shared_ptr<DcsControlProxy> spCilent = DcsControlProxy::GetInstance();
    if (spCilent == nullptr)
    {
        ULOG(LOG_ERROR, "Get DcsControlProxy obj Fail!");
        return;
    }
    // 查询维护项
    ::tf::MaintainItemQueryResp retItem;
    ::tf::MaintainItemQueryCond miqcItem;
    if (!spCilent->QueryMaintainItem(retItem, miqcItem))
    {
        ULOG(LOG_ERROR, "QueryMaintainItem Fail!");
        return;
    }

    m_vecMaintainItemsCache = retItem.lstMaintainItems;
    outItems = std::move(retItem.lstMaintainItems);
}

///
///  @brief 检查是否有仪器正在执行该维护组
///
///
///  @param[in]   groupId  维护组
///
///  @return	true 有仪器执行 false 无仪器执行
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年9月7日，新建函数
///
bool QMaintainSetPage::HasDeviceInExecting(const ::tf::MaintainGroup& group)
{
    auto devMap = CommonInformationManager::GetInstance()->GetDeviceMaps();

    if (devMap.empty())
    {
        return false;
    }
    // 	auto currentDate = QDate::currentDate();
    // 	::tf::TimeRangeCond exeTime;
    // 	exeTime.__set_startTime(std::string(QString("%1-%2-%3 00:00:00").arg(currentDate.year()).arg(currentDate.month(), 2, 10, QChar('0')).arg(currentDate.day(), 2, 10, QChar('0')).toLocal8Bit()));
    // 	exeTime.__set_endTime(std::string(QString("%1-%2-%3 23:59:59").arg(currentDate.year()).arg(currentDate.month(), 2, 10, QChar('0')).arg(currentDate.day(), 2, 10, QChar('0')).toLocal8Bit()));

    std::vector<std::string> executingDevs;
    for (auto it = devMap.begin(); it != devMap.end(); ++it)
    {
        ::tf::LatestMaintainInfoQueryResp ret;
        ::tf::LatestMaintainInfoQueryCond lmiqc;
        lmiqc.__set_groupId(group.id);
        lmiqc.__set_deviceSN(it->second->deviceSN);
        //lmiqc.__set_exeTime(exeTime);
        if (!DcsControlProxy::GetInstance()->QueryLatestMaintainInfo(ret, lmiqc))
        {
            continue;
        }
        if (ret.lstLatestMaintainInfo.empty())
        {
            continue;
        }

        if (ret.lstLatestMaintainInfo.front().exeResult == ::tf::MaintainResult::type::MAINTAIN_RESULT_EXECUTING)
        {
            executingDevs.push_back(CommonInformationManager::GetDeviceName(it->second->deviceSN));
        }
    }

    if (executingDevs.empty())
    {
        return false;
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////
/// @file     QMaintainSetPage.h
/// @brief 	 维护组筛选排序模型代理
///
/// @author   7656/zhang.changjiang
/// @date      2023年2月22日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年2月22日，新建文件
///
///////////////////////////////////////////////////////////////////////////
QMaintainGroupFilterProxyModel::QMaintainGroupFilterProxyModel(QObject* parent /*= nullptr*/)
{
    setSourceModel(&MaintainGroupModel::Instance());
}

QMaintainGroupFilterProxyModel::~QMaintainGroupFilterProxyModel()
{
}

Qt::ItemFlags QMaintainGroupFilterProxyModel::flags(const QModelIndex & index) const
{
    if (!index.isValid())
    {
        return Qt::NoItemFlags;
    }
    return QSortFilterProxyModel::flags(index) | Qt::ItemIsEditable;
}

bool QMaintainGroupFilterProxyModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if (role != Qt::EditRole)
    {
        return QSortFilterProxyModel::setData(index, value, role);
    }

    QModelIndex sourceIndex = mapToSource(index);
    bool success = sourceModel()->setData(sourceIndex, value, role);
    if (success)
    {
        emit dataChanged(index, index, { Qt::DisplayRole, Qt::EditRole });
    }
    return success;
}

bool QMaintainGroupFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (MaintainGroupModel::Instance().GetData().empty())
    {
        return false;
    }
    if (sourceRow < 0 || sourceRow >= MaintainGroupModel::Instance().GetData().size())
    {
        return false;
    }
    auto groupType = MaintainGroupModel::Instance().GetData()[sourceRow].groupType;
    if (groupType != ::tf::MaintainGroupType::type::MAINTAIN_GROUP_SINGLE && groupType != ::tf::MaintainGroupType::type::MAINTAIN_GROUP_OTHER)
    {
        return true;
    }
    return false;
}

QVariant QMaintainGroupFilterProxyModel::data(const QModelIndex & index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    if (role == Qt::DisplayRole)
    {
        if (index.column() == 0)
        {
            return index.row() + 1;
        }
    }

    return QSortFilterProxyModel::data(index, role);
}

///
///  @brief 获取指定索引值的维护组
///
///
///  @param[out]   outGroup  输出维护组
///  @param[in]    index     索引值
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年8月8日，新建函数
///
bool QMaintainGroupFilterProxyModel::GetGroupByIndex(tf::MaintainGroup & outGroup, const QModelIndex index)
{
    auto sourceIndex = mapToSource(index);
    return MaintainGroupModel::Instance().GetGroupByIndex(outGroup, sourceIndex);
}

///
///  @brief 设置选中的行
///
///  @param[in]   iSelectedRow  
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年9月7日，新建函数
///
void QMaintainGroupFilterProxyModel::SetSelectedRow(int iSelectedRow)
{
    auto sourceIndex = mapToSource(index(iSelectedRow, 0));
    if (sourceIndex.isValid())
    {
        MaintainGroupModel::Instance().SetSelectedRow(sourceIndex.row());
    }
}

///
///  @brief 获取选中的行
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年11月15日，新建函数
///
int QMaintainGroupFilterProxyModel::GetSelectedRow()
{
    auto sourceIndex = MaintainGroupModel::Instance().index(MaintainGroupModel::Instance().GetSelectedRow(), 0);
    auto proxyIndex = mapFromSource(sourceIndex);
    return proxyIndex.row();
}

///
///  @brief 是否是最后一行
///
///
///  @param[in]   iRow  行数
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年8月8日，新建函数
///
bool QMaintainGroupFilterProxyModel::IsLastRow(int iRow)
{
    auto sourceIndex = mapToSource(index(iRow, 0));
    if (sourceIndex.isValid())
    {
        return MaintainGroupModel::Instance().IsLastRow(sourceIndex.row());
    }

    return false;
}

///
///  @brief	配置数据
///
///  @param[in]   isChecked    选中状态
///  @param[in]   item		   配置的维护项
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年9月7日，新建函数
///
void QMaintainGroupFilterProxyModel::ConfigData(bool isChecked, const tf::MaintainItem & item)
{
    MaintainGroupModel::Instance().ConfigData(isChecked, item);
}

QMaintainItemSelectDelegate::QMaintainItemSelectDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{

}
bool QMaintainItemSelectDelegate::editorEvent(QEvent * event, QAbstractItemModel * model, const QStyleOptionViewItem & option, const QModelIndex & index)
{
    if (!index.isValid())
    {
        return false;
    }

    auto mouseEvent = static_cast<QMouseEvent*>(event);
    if (mouseEvent == Q_NULLPTR)
    {
        return false;
    }
    if (event->type() == QEvent::MouseMove)
    {
        QString text = index.data(Qt::DisplayRole).toString();
        int iTextWidth = option.fontMetrics.width(text) + TABLE_CELL_MARGIN;
        int icellWidth = option.rect.width();
        if (iTextWidth > icellWidth)
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (option.rect.contains(mouseEvent->pos()))
            {
                QToolTip::showText(mouseEvent->globalPos(), text);
            }
        }
    }

    QMaintainGroupCfgFilterProxyModel* pModel = dynamic_cast<QMaintainGroupCfgFilterProxyModel*>(model);
    if (pModel == nullptr)
    {
        return false;
    }

    QRect delBtnRect = QRect(option.rect.right() - 38, option.rect.top() - 10, 10, option.rect.height() - 2);
    QRect addBtnRect = QRect(option.rect.left() + 25, option.rect.top() + 24, 20, option.rect.height() - 2);
    if (mouseEvent->type() == QMouseEvent::MouseButtonPress)
    {
        auto text = index.data(Qt::DisplayRole).toString();

        // 删除
        if (delBtnRect.contains(mouseEvent->pos()))
        {
            MaintainGroupModel::Instance().DelSelGroupItemByIndex(pModel->mapToSource(index));
            pModel->DelItemByIndex(index);
            return true;
        }

        // 新增
        if (addBtnRect.contains(mouseEvent->pos()))
        {
            MaintainGroupModel::Instance().CopySelGroupItemByIndex(index);
            pModel->CopyItemByIndex(index);
            return true;
        }
    }

    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

void QMaintainItemSelectDelegate::paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    QRect delBtnRect = QRect(option.rect.right() - 38, option.rect.top() + 24, option.rect.width() / 5, option.rect.height() - 2);
    QRect addBtnRect = QRect(option.rect.left() + 25, option.rect.top() + 24, option.rect.width() / 5, option.rect.height() - 2);

    QPixmap pixmap;
    pixmap.load(QString(":/Leonis/resource/image/icon-cancel.png"));
    painter->drawPixmap(delBtnRect.left(), delBtnRect.top(), pixmap);
    pixmap.load(QString(":/Leonis/resource/image/icon-add-item.png"));
    painter->drawPixmap(addBtnRect.left(), addBtnRect.top(), pixmap);
    QStyledItemDelegate::paint(painter, option, index);
}

///////////////////////////////////////////////////////////////////////////
/// @class     QTableDoubleClickDelegate
/// @brief 	   表格双击响应代理
///////////////////////////////////////////////////////////////////////////
QTableDoubleClickDelegate::QTableDoubleClickDelegate(QObject * parent)
    : QStyledItemDelegate(parent)
{
}

QWidget * QTableDoubleClickDelegate::createEditor(QWidget * parent, const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    ::tf::MaintainGroup group;
    auto pModel = dynamic_cast<QMaintainGroupFilterProxyModel*>(const_cast<QAbstractItemModel*> (index.model()));
    if (pModel == nullptr)
    {
        return nullptr;
    }

    if (!pModel->GetGroupByIndex(group, index))
    {
        return nullptr;
    }

    if (group.groupType != ::tf::MaintainGroupType::type::MAINTAIN_GROUP_CUSTOM)
    {
        emit const_cast<QTableDoubleClickDelegate*>(this)->sigCannotChange();
        return nullptr;
    }

    QLineEdit* lineEditor = new QLineEdit(parent);
    lineEditor->setMaxLength(GROUP_NAME_MAX_LEN);
    connect(lineEditor, &QLineEdit::textChanged, this, &QTableDoubleClickDelegate::sigTextChanged, Qt::UniqueConnection);
    return lineEditor;
}

void QTableDoubleClickDelegate::setEditorData(QWidget * editor, const QModelIndex & index) const
{
    QString value = index.data(Qt::EditRole).toString();
    QLineEdit* lineEditor = dynamic_cast<QLineEdit*>(editor);
    if (lineEditor != nullptr)
    {
        lineEditor->setText(value);
    }
}

void QTableDoubleClickDelegate::setModelData(QWidget * editor, QAbstractItemModel * model, const QModelIndex & index) const
{
    QLineEdit* lineEditor = dynamic_cast<QLineEdit*>(editor);
    if (lineEditor == nullptr)
    {
        return;
    }

    QString text = lineEditor->text();
    if (text.isEmpty())
    {
        emit const_cast<QTableDoubleClickDelegate*>(this)->sigInputNullStr();
        return;
    }

    model->setData(index, text, Qt::EditRole);
}

bool QTableDoubleClickDelegate::editorEvent(QEvent * event, QAbstractItemModel * model, const QStyleOptionViewItem & option, const QModelIndex & index)
{
    if (event->type() == QEvent::MouseMove)
    {
        QString text = index.data().toString();
        int iTextWidth = option.fontMetrics.width(text) + TABLE_CELL_MARGIN;
        int icellWidth = option.rect.width();
        if (iTextWidth > icellWidth)
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (option.rect.contains(mouseEvent->pos()))
            {
                QToolTip::showText(mouseEvent->globalPos(), text);
            }
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

///////////////////////////////////////////////////////////////////////////
/// @class     QMaintItemTableDelegate
/// @brief 	   维护项表格提示框代理
///////////////////////////////////////////////////////////////////////////
QMaintItemTableDelegate::QMaintItemTableDelegate(QObject * parent)
    : QStyledItemDelegate(parent)
{
}

bool QMaintItemTableDelegate::editorEvent(QEvent * event, QAbstractItemModel * model, const QStyleOptionViewItem & option, const QModelIndex & index)
{
    if (event->type() == QEvent::MouseMove)
    {
        QString text = index.data().toString();
        int iTextWidth = option.fontMetrics.width(text) + TABLE_CELL_MARGIN;
        int icellWidth = option.rect.width();
        if (iTextWidth > icellWidth)
        {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent *>(event);
            if (option.rect.contains(mouseEvent->pos()))
            {
                QToolTip::showText(mouseEvent->globalPos(), text);
            }
        }
    }
    return QStyledItemDelegate::editorEvent(event, model, option, index);
}

QMaintainGroupCfgFilterProxyModel::QMaintainGroupCfgFilterProxyModel(QObject * parent)
{
    setSourceModel(&MaintainGroupCfgModel::Instance());
}

QMaintainGroupCfgFilterProxyModel::~QMaintainGroupCfgFilterProxyModel()
{
}

bool QMaintainGroupCfgFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex & sourceParent) const
{
    auto item = MaintainGroupCfgModel::Instance().GetSelectedItemByRow(sourceRow);
    if (!item.__isset.itemType)
    {
        return false;
    }

    // 不支持则不显示
    return gUiAdapterPtr()->WhetherSupportTargetMaintainItem(item);
}

///
///  @brief	设置维护项目数据
///
///
///  @param[in]   data  
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年9月1日，新建函数
///
void QMaintainGroupCfgFilterProxyModel::SetData(const std::vector<tf::MaintainItem>& data)
{
    MaintainGroupCfgModel::Instance().SetData(data);
    emit updateSelected(rowCount());
}

///
///  @brief 将from位置的元素移动到to位置
///
///
///  @param[in]   from  移动的初始位置
///  @param[in]   to	移动的目标位置
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年2月14日，新建函数
///
void QMaintainGroupCfgFilterProxyModel::MoveItem(int from, int to)
{
    MaintainGroupCfgModel::Instance().MoveItem(mapToSource(index(from, 0)).row(), mapToSource(index(to, 0)).row());
}

///
///  @brief 复制一个维护项到指定inde
///
///
///  @param[in]   index  
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年3月7日，新建函数
///
void QMaintainGroupCfgFilterProxyModel::CopyItemByIndex(const QModelIndex & index)
{
    MaintainGroupCfgModel::Instance().CopyItemByIndex(mapToSource(index));
    emit updateSelected(rowCount());
}

///
///  @brief 删除指定index的维护项
///
///
///  @param[in]   index  
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年3月7日，新建函数
///
void QMaintainGroupCfgFilterProxyModel::DelItemByIndex(const QModelIndex & index)
{
    MaintainGroupCfgModel::Instance().DelItemByIndex(mapToSource(index));
    emit updateSelected(rowCount());
}

///
///  @brief 获取指定index的维护项
///
///  @param[in]   index  需要获取该维护项的索引
///  @param[in]   itemData  用于返回的维护项信息
///
///  @return	true:获取成功
///
///  @par History: 
///  @li 8580/GongZhiQiang，2023年6月27日，新建函数
///
bool QMaintainGroupCfgFilterProxyModel::GetItemByIndex(const QModelIndex & index, MaintainGroupCfgModel::ItemData & itemData)
{
    return MaintainGroupCfgModel::Instance().GetItemByIndex(mapToSource(index), itemData);
}

///
///  @brief 修改指定index的维护项
///
///  @param[in]   index  需要获取该维护项的索引
///  @param[in]   itemData  修改的维护项信息
///
///  @return	true:修改成功
///
///  @par History: 
///  @li 8580/GongZhiQiang，2023年6月27日，新建函数
///
bool QMaintainGroupCfgFilterProxyModel::ModifyItemByIndex(const QModelIndex & index, const MaintainGroupCfgModel::ItemData & itemData)
{
    return MaintainGroupCfgModel::Instance().ModifyItemByIndex(mapToSource(index), itemData);
}

///
///  @brief 修改制定index的维护项重复次数
///
///
///  @param[in]   index  需要获取该维护项的索引
///  @param[in]   repetition  重复次数
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年8月15日，新建函数
///
bool QMaintainGroupCfgFilterProxyModel::ModifyItemByIndex(const QModelIndex & index, int repetition)
{
    return MaintainGroupCfgModel::Instance().ModifyItemByIndex(mapToSource(index), repetition);
}

///
///  @brief 全部清除
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年3月7日，新建函数
///
void QMaintainGroupCfgFilterProxyModel::ClearData()
{
    MaintainGroupCfgModel::Instance().ClearData();
    emit updateSelected(rowCount());
}

///
///  @brief 配置数据
///
///
///  @param[in]   isChecked    选中状态
///  @param[in]   item		   配置的维护项
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年3月27日，新建函数
///
void QMaintainGroupCfgFilterProxyModel::ConfigData(bool isChecked, const tf::MaintainItem & item)
{
    MaintainGroupCfgModel::Instance().ConfigData(isChecked, item);
    emit updateSelected(rowCount());
}
