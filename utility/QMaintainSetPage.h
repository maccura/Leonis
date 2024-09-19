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
#pragma once

#include <QLineEdit>
#include <QScrollBar>
#include <QSortFilterProxyModel>
#include <QStyledItemDelegate>
#include "shared/basedlg.h"
#include "src/thrift/gen-cpp/defs_types.h"
#include "model/MaintainGroupCfgModel.h"

class MaintainGroupModel;
class QMaintainGroupFilterProxyModel;
class QMaintainItemSelectDelegate;
class MaintainDefaultSetDlg;
class QTableDoubleClickDelegate;
class QStandardItemModel;
class QMaintainGroupCfgFilterProxyModel;

namespace Ui {
    class QMaintainSetPage;
};

///////////////////////////////////////////////////////////////////////////
/// @class     SearchLineEdit
/// @brief 	   自定义搜素框
///////////////////////////////////////////////////////////////////////////
class SearchLineEdit : public QLineEdit
{
	Q_OBJECT

public:
	explicit SearchLineEdit(QWidget *parent = 0);
	~SearchLineEdit();
	
	///
	///  @brief 初始化
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年3月3日，新建函数
	///
	void Init();
	
	///
	///  @brief 设置联想功能model
	///
	///
	///  @param[in]   model  联想功能model
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年3月3日，新建函数
	///
	void SetWordListModel(QAbstractItemModel *model);

	///
	///  @brief 获取搜索按钮
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年3月3日，新建函数
	///
    QToolButton *GetSearchBtn() { return m_searchBtn; };
	
	///
	///  @brief 获取清空按钮
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年3月3日，新建函数
	///
    QToolButton *GetCleanBtn() { return m_cleanBtb; };

protected:
	bool eventFilter(QObject *obj, QEvent *event) override;

private slots:

private:
	QToolButton *                        m_searchBtn;			// 搜索按钮
	QToolButton *                        m_cleanBtb;			// 清除按钮
};

///////////////////////////////////////////////////////////////////////////
/// @class     ScrollBar
/// @brief 	   自定义滚动条（主要是为了实现滚动条显示和影藏的时候调整可能重合控件的位置）
///////////////////////////////////////////////////////////////////////////
class ScrollBar : public QScrollBar
{
	Q_OBJECT

public:
	explicit ScrollBar(QWidget *parent = Q_NULLPTR);
	explicit ScrollBar(Qt::Orientation orientation, QWidget *parent = Q_NULLPTR);
	
	///
	///  @brief 显示事件
	///
	void showEvent(QShowEvent *event);

	///
	///  @brief 隐藏事件
	///
	void hideEvent(QHideEvent *event);
signals:
	
	///
	///  @brief 滚动条显示信号
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年3月10日，新建函数
	///
	void barShow();
	
	///
	///  @brief 滚动条隐藏信号
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年3月10日，新建函数
	///
	void barHide();
};

///////////////////////////////////////////////////////////////////////////
/// @class     QMaintainSetPage
/// @brief 	   组合设置主窗体
///////////////////////////////////////////////////////////////////////////
class QMaintainSetPage : public BaseDlg
{
	Q_OBJECT

public:
	QMaintainSetPage(QWidget *parent = Q_NULLPTR);
	~QMaintainSetPage();
protected:
	bool eventFilter(QObject *obj, QEvent *event) override;
	void showEvent(QShowEvent *event);
private:

	///
	///  @brief:	初始化
	///
	///
	///  @return	:
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2022年8月29日，新建函数
	///
	void Init();

	///
	///  @brief 初始化维护组表
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2022年9月6日，新建函数
	///
	void InitGroup();

	///
	///  @brief 初始化维护项表
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2022年9月6日，新建函数
	///
	void InitItem();

	///
	///  @brief	连接信号槽
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2022年9月6日，新建函数
	///
	void ConnectSigAndSlots();

	///
	///  @brief 更新维护组配置表
	///
	///
	///  @param[in]   groupType    维护组类型
	///  @param[in]   vecItem      维护组包含的维护项列表
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年3月1日，新建函数
	///
	void UpdateGroupCfg(const ::tf::MaintainGroupType::type groupType, const std::vector<tf::MaintainItem>& vecItem);
	
	///
	///  @brief 刷新维护组的配置表(全刷)
	///
	///
	///  @param[in]   iRow  维护组所在行
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2022年11月15日，新建函数
	///
	void RefreshGroupCfg(int iRow);

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
	void RefreshGroupCfg(const int iRow, std::vector<tf::MaintainItem>& items);

	///
	///  @brief 更新备选数
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年3月27日，新建函数
	///
	void UpdateSelect();
	
	///
	///  @brief 设置指定维护项复选框状态
	///
	///
	///  @param[in]   itemType   维护项类型
    ///  @param[in]   isChecked  是否选中 
    ///  @param[in]   isEabled  是否允许修改（-1：不涉及， 0：不允许， 1：允许
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年3月27日，新建函数
	///
	void SetCheckBoxState(int itemType, bool isChecked, int isEabled = -1);

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
	void SetAllCheckBoxState(bool isChecked);
	
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
	void SetAllCheckBoxEnable(bool isEnable);

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
	bool ShutdownMaintainCheck(const std::vector<tf::MaintainGroup>& vecGroups);
	
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
	bool HasItemTypeInItems(const std::vector<tf::MaintainItem>& vecItems, const ::tf::MaintainItemType::type itemType);

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
	bool ItemAreUnique(const std::vector<tf::MaintainItem>& vecItems, const ::tf::MaintainItemType::type itemType);

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
	bool ItemAreLastOne(const std::vector<tf::MaintainItem>& vecItems, const ::tf::MaintainItemType::type itemType);

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
	bool ItemAreFirstOne(const std::vector<tf::MaintainItem>& vecItems, const ::tf::MaintainItemType::type itemType);

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
	bool MaintainGroupCheck(const tf::MaintainGroup& maintainGroup);

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
	bool StartMaintainGroupCheck(const std::vector<tf::MaintainItem>& vecItems);

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
	bool StopMaintainGroupCheck(const std::vector<tf::MaintainItem>& vecItems);

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
	bool WeekMaintainGroupCheck(const std::vector<tf::MaintainItem>& vecItems);

public slots:
	///
	///  @brief 刷全部清除按钮的位置
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年3月7日，新建函数
	///
	void UpdateClearBtnPos();

	///
	///  @brief 刷搜索框的位置
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年3月7日，新建函数
	///
	void UpdateSearchBoxPos();
private slots:
	///
	///  @brief	维护组新增按钮
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2022年9月6日，新建函数
	///
	void OnAddNewBtnClicked();

	///
	///  @brief	维护组删除按钮
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2022年9月6日，新建函数
	///
	void OnDeleteBtnClicked();

	///
	///  @brief	保存按钮
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2022年9月6日，新建函数
	///
	void OnSaveBtnClicked();

	///
	///  @brief 维护项搜索按钮
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2022年9月6日，新建函数
	///
	void OnSerachItemBtnClicked();
	
	///
	///  @brief 全部清除按钮
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年2月28日，新建函数
	///
	void OnCleanBtnClicked();

	///
	///  @brief 置顶按钮
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年2月14日，新建函数
	///
	void OnTopBtnClicked();
	
	///
	///  @brief 向上按钮
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年2月14日，新建函数
	///
	void OnUpBtnClicked();
	
	///
	///  @brief 向下
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年2月14日，新建函数
	///
	void OnDownBtnClicked();
	
	///
	///  @brief 置底按钮
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年2月14日，新建函数
	///
	void OnBottomBtnClicked();
	
	///
	///  @brief 重置按钮
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年8月15日，新建函数
	///
	void OnResetBtnClicked();

	///
	///  @brief 更新按钮状态
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年3月13日，新建函数
	///
	void UpdateBtnState();

	///
	///  @brief:	关闭按钮被点击
	///
	///
	///  @return	:
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2022年8月29日，新建函数
	///
	void OnCloseBtnClicked();
	
	///
	///  @brief 复选框的槽函数（点击复选框）
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年3月27日，新建函数
	///
	void OnCheckBoxClicked();

	///
	///  @brief 选项设置按钮点击槽函数
	///
	///  @return	
	///
	///  @par History: 
	///  @li 8580/GongZhiQiang，2023年6月26日，新建函数
	///
    void OnOptionItemSetClicked();

    ///
    /// @bref
    ///		权限变化响应
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年4月9日，新建函数
    ///
    void OnPermisionChanged();

private:
	
	///
	///  @brief 过滤ui不显示的维护项
	///
	///
    ///  @param[out]   outItems  维护项列表
    ///  @param[in]   groupLimit  当前维护组，当前维护组存在的维护项不进行过滤
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年8月17日，新建函数
	///
	void FilterItems(std::vector<::tf::MaintainItem> &outItems, const std::shared_ptr<tf::MaintainGroup>& groupLimit = nullptr);
	
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
	void SearchItems(std::vector<::tf::MaintainItem> &outItems);
	
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
	void InitItemTableData(const std::vector<::tf::MaintainItem> &items);
	
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
	void QueryAllItemData(std::vector<::tf::MaintainItem> &outItems);
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
	bool HasDeviceInExecting(const ::tf::MaintainGroup& group);

    ///
    /// @bref
    ///		与备选列表中的维护项是否存在差异, 如果返回true则表示存在差异
    ///
    /// @param[in] vecItems 新维护项
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年5月21日，新建函数
    ///
    bool IsMaintainListChanged(const std::vector<::tf::MaintainItem>& vecItems);
	
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
	bool IsBefore(const std::vector<::tf::MaintainItem>& vecItems, const ::tf::MaintainItemType::type itemType1, const ::tf::MaintainItemType::type itemType2);
private:
	Ui::QMaintainSetPage* ui;
	MaintainDefaultSetDlg*							m_pMaintainDefaultSetDlg;			// 维护项默认参数设置
	QMaintainGroupFilterProxyModel*					m_pMaintainGroupModel;				// 维护组表模型
	QStandardItemModel*								m_pMaintainItemModel;				// 维护项表模型
	QMaintainGroupCfgFilterProxyModel*				m_pCfgModel;						// 维护组配置表模型
	QMaintainItemSelectDelegate*					m_pCfgModelDelegate;				// 维护组配置表常显代理
	SearchLineEdit*									m_pSearchLineEdit;					// 维护项表表头搜索框
	std::map<int, ::tf::MaintainItem>				m_itemDataMap;						// 维护项类型和维护项映射表
	QVector<int>								    m_itemTypeData;						// 维护项类型列表
	QTableDoubleClickDelegate *						m_pDelegate;						// 双击响应代理

    std::vector<::tf::MaintainItem>                 m_vecMaintainItemsCache;            // 全维护项列表缓存，缓解每次查询全维护项
};

///////////////////////////////////////////////////////////////////////////
/// @class     QMaintainGroupFilterProxyModel
/// @brief 	   维护组筛选排序模型代理
///////////////////////////////////////////////////////////////////////////
class QMaintainGroupFilterProxyModel : public QSortFilterProxyModel
{
	Q_OBJECT

public:
	QMaintainGroupFilterProxyModel(QObject* parent = nullptr);
	~QMaintainGroupFilterProxyModel();

	Qt::ItemFlags flags(const QModelIndex& index)const override;
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
	bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole)const override;

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
	bool GetGroupByIndex(tf::MaintainGroup & outGroup, const QModelIndex index);

	///
	///  @brief 设置选中的行
	///
	///  @param[in]   iSelectedRow  
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2022年9月7日，新建函数
	///
	void SetSelectedRow(int iSelectedRow);

	///
	///  @brief 获取选中的行
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2022年11月15日，新建函数
	///
	int GetSelectedRow();

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
	bool IsLastRow(int iRow);
public slots:

	///
	///  @brief	配置数据
	///
	///  @param[in]   isChecked    选中状态
	///  @param[in]   item		   配置的维护项
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2022年9月7日，新建函数
	///
	void ConfigData(bool isChecked, const tf::MaintainItem &item);
};

///////////////////////////////////////////////////////////////////////////
/// @class     QMaintainItemSelectDelegate
/// @brief 	   维护组配置表维护项按钮控件
///////////////////////////////////////////////////////////////////////////
class QMaintainItemSelectDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:
	QMaintainItemSelectDelegate(QObject *parent = nullptr);
	bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);
	void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex & index) const;

signals:
	
	///
	///  @brief 删除指定维护项
	///
	///
	///  @param[in]   itemType  维护项类型
	///  @param[in]   index     删除维护项的索引
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年3月10日，新建函数
	///
	void delDelegate(::tf::MaintainItemType::type itemType, const QModelIndex &index);
	
	///
	///  @brief 复制指定维护项
	///
	///
	///  @param[in]   itemType  维护项类型
	///  @param[in]   index     复制的维护项的索引
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年3月10日，新建函数
	///
	void addDelegate(::tf::MaintainItemType::type itemType, const QModelIndex &index);
};

///////////////////////////////////////////////////////////////////////////
/// @class     QTableDoubleClickDelegate
/// @brief 	   表格双击响应代理
///////////////////////////////////////////////////////////////////////////
class QTableDoubleClickDelegate : public QStyledItemDelegate
{
	Q_OBJECT

public:
	QTableDoubleClickDelegate(QObject *parent = nullptr);

	QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

	void setEditorData(QWidget *editor, const QModelIndex &index) const override;

	void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
	
	bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
	
signals:
	// 代理文本改变信号
	void sigTextChanged(QString text);

	// 代理文本为空信号
	void sigInputNullStr();

	// 禁止修改系统组合信号
	void sigCannotChange();
};

///////////////////////////////////////////////////////////////////////////
/// @class     QMaintItemTableDelegate
/// @brief 	   维护项表格提示框代理
///////////////////////////////////////////////////////////////////////////
class QMaintItemTableDelegate : public QStyledItemDelegate
{
public:
	QMaintItemTableDelegate(QObject *parent = nullptr);
	bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
};

///////////////////////////////////////////////////////////////////////////
/// @class     QMaintItemTableDelegate
/// @brief 	   维护项配置表格代理模型
///////////////////////////////////////////////////////////////////////////
class QMaintainGroupCfgFilterProxyModel : public QSortFilterProxyModel
{
 	Q_OBJECT

public:
	QMaintainGroupCfgFilterProxyModel(QObject* parent = nullptr);
	~QMaintainGroupCfgFilterProxyModel();

	bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

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
	void SetData(const std::vector<tf::MaintainItem> & data);

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
	void MoveItem(int from, int to);

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
	void CopyItemByIndex(const QModelIndex &index);

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
	void DelItemByIndex(const QModelIndex &index);

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
	bool GetItemByIndex(const QModelIndex &index, MaintainGroupCfgModel::ItemData& itemData);

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
	bool ModifyItemByIndex(const QModelIndex &index, const MaintainGroupCfgModel::ItemData& itemData);

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
	bool ModifyItemByIndex(const QModelIndex &index, int repetition);

	///
	///  @brief 全部清除
	///
	///
	///
	///  @return	
	///
	///  @par History: 
	///  @li 7656/zhang.changjiang，2023年3月7日，新建函数
	///
	void ClearData();
signals:
	void updateSelected(int size);
public slots:

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
	void ConfigData(bool isChecked, const tf::MaintainItem &item);
};
