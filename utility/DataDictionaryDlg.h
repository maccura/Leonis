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
/// @file     DataDictionaryDlg.h
/// @brief    应用--系统--数据字典
///
/// @author   7951/LuoXin
/// @date     2022年11月22日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2022年11月22日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include <QWidget>
#include <QMap>
#include "src/public/ConfigDefine.h"

class QStandardItemModel;
class QAutoSeqNoModel;
class QStandardItem;
class QItemSelection;
class LimitLenDelegate;

namespace Ui {
    class DataDictionaryDlg;
}

class DataDictionaryDlg : public QWidget
{
	Q_OBJECT

public:
	DataDictionaryDlg(QWidget *parent = Q_NULLPTR);
	~DataDictionaryDlg();

	///
	/// @brief	加载数据到控件
	///     
	/// @par History:
	/// @li 7951/LuoXin，2022年11月22日，新建函数
	///
    void LoadDataToCtrls();

    ///
    /// @brief 设置字典数据
    ///
    /// @param[in]  data  数据
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年8月2日，新建函数
    ///
    void SetDictData(DisplaySet data);

    ///
    /// @brief 是否存在未保存的数据
    ///
    ///
    /// @return true:表示有
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2024年4月3日，新建函数
    ///
    bool isExistUnsaveData();

protected:
    // 界面隐藏事件
    virtual void hideEvent(QHideEvent *event) override;

private:
	///
	/// @brief	初始化控件
	///     
	/// @par History:
	/// @li 7951/LuoXin，2022年11月22日，新建函数
	///
	void InitCtrls();

    ///
    /// @brief 更新按钮可用状态
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年8月3日，新建函数
    ///
    void UpdateBtnEnable();

    ///
    /// @brief 获取数据项列表的名称
    ///
    /// @return 空列表：重复， 非控列表：未重复
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年8月3日，新建函数
    ///
    QList<QString> IsNameTableDuplicate();

    ///
    /// @brief 显示失败对话框
    ///
    /// @param[in]  reason  失败原因
    ///
    /// @return 窗口返回值
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年8月3日，新建函数
    ///
    int ShowFaliedReasonDlg(const QString& reason);

    ///
    /// @brief 更新数据项表
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年8月3日，新建函数
    ///
    void UpdateNameTableDB();

    ///
    /// @brief 获取当前数据表对应的字典主键
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年8月3日，新建函数
    ///
    void GetCurrentDictKey();

    ///
    /// @brief 增加一行空行
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年8月15日，新建函数
    ///
    void AddInfoTableEmptyRow();

    ///
    /// @brief 获取当前选中行的名称（不能在选项发生变化时，获取之前选中的）
    ///
    ///
    /// @return 当前选中的名称
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年10月20日，新建函数
    ///
    QString GetCurrentName();

    ///
    /// @brief 获取当前选中项的患者信息字典定义值（不能在选项发生变化时，获取之前选中的）
    ///
    ///
    /// @return 定义值
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年10月20日，新建函数
    ///
    int GetCurrentType();

    ///
    /// @brief 获取左侧名称表格当前选中单元格
    ///
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2024年6月24日，新建函数
    ///
    QModelIndex GetNameTabCurrentIndex();
    int GetNameTabCurrentRow();

protected Q_SLOTS:
	///
	/// @brief	保存按钮被点击
	///     
	/// @par History:
	/// @li 7951/LuoXin，2022年11月22日，新建函数
	///
	void OnSaveBtnClicked();

	///
	/// @brief	删除按钮被点击
	///     
	/// @par History:
	/// @li 7951/LuoXin，2022年11月22日，新建函数
	///
    void OnDelBtnClicked();

    ///
    /// @brief	清空自定义被点击
    ///     
    /// @par History:
    /// @li 8090/YeHuaNing，2023年8月14日，新建函数
    ///
    void OnClearCustomBtnClicked();

	///
	/// @brief		名称列表选中表格改变
	///     
	/// @par History:
	/// @li 7951/LuoXin，2022年11月23日，新建函数
	///
	void OnNameTableViewCurrentChanged(const QModelIndex& index);

    ///
    /// @brief 选项发生变化
    ///
    /// @param[in]  selected  选择项
    /// @param[in]  deselected  取消项
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年8月2日，新建函数
    ///
    void OnSelectionChanged(const QItemSelection& selected, const QItemSelection &deselected);

    ///
    /// @brief 当项目发生了变化
    ///
    /// @param[in]  item  变化的项目
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年8月2日，新建函数
    ///
    //void OnNameTableItemChanged(QStandardItem* item);
    void OnInfoTableItemChanged(QStandardItem* item);

    ///
    /// @brief 数据项表双击信号处理
    ///
    /// @param[in]  index  触发索引
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年8月3日，新建函数
    ///
    //void OnNameTableDoubleClicked(const QModelIndex& index);

    ///
    /// @brief 数据字典 数据项更新
    ///
    /// @param[in]  dsSet  更新后的数据
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年8月15日，新建函数
    ///
    void OnDataDictUpdated(DisplaySet dsSet);

    ///
    /// @brief 当右侧输入框数据发生变化
    ///
    /// @param[in]  text  变化后的文字
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年10月20日，新建函数
    ///
    void OnNameTextChanged(const QString& text);

    ///
    /// @bref
    ///		权限变化响应
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年2月24日，新建函数
    ///
    void OnPermisionChanged();

private:
	Ui::DataDictionaryDlg*			ui;
	QStandardItemModel*				m_nameModel;
    QAutoSeqNoModel*				m_infoModel;
    bool                            m_bKeyExisted;              /// 是否存在key
    bool                            m_bDataInfoChanged;         /// 数据信息是否发生过改变
    bool                            m_bNameEditted;             /// 是否编辑过名称
    bool                            m_bPermit;                  /// 当前用户操作权限
    int                             m_customStartRow;           /// 自定义设置开始行
    QString                         m_edittingName;             /// 编辑中的原项目名称（鼠标点击左侧表时，提示保存后，才能赋值）
    int                             m_edittingType;             /// 编辑中的类型（用于检查是否已经保存
    QList<QString>                  m_nameCheckList;            /// 名称重复检查表
    DisplaySet                      m_storedSet;                ///
    QMap<int, DisplaySetItem>       m_dsItems;                  /// 显示项目索引表
    std::string                     m_changedDict;              /// 数据发生变化的数项
    QVector<QString>                m_cosSettingData;           /// 自定义项目的名称
    LimitLenDelegate*               m_limitLenDelegate;         /// 自定义内容输入代理（用于显示显示内容长度）
};
