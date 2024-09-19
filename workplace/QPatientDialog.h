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
/// @file     QPatientDialog.h
/// @brief    病人信息
///
/// @author   5774/WuHongTao
/// @date     2022年5月24日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月24日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include <QMap>
#include <functional>
#include "shared/basedlg.h"
#include "src/public/ConfigDefine.h"
#include "src/thrift/gen-cpp/defs_types.h"

namespace Ui {
    class QPatientDialog;
};
class QLayoutItem;

template<typename classType, typename Fun>
class ExitDoit
{
public:
	ExitDoit(classType* obj, Fun& functionData)
	{
		m_exitDo = functionData;
		m_obj = obj;
	}

	~ExitDoit()
	{
		if (m_obj != nullptr && m_exitDo != nullptr)
		{
			(m_obj->*m_exitDo)();
		}
	}

private:
	Fun			m_exitDo;
	classType*  m_obj;
};

class QPatientDialog : public BaseDlg
{
	Q_OBJECT

public:
	QPatientDialog(QWidget *parent = Q_NULLPTR);
	~QPatientDialog();

	///
	/// @brief 刷新样本的病人对象信息
	///
	/// @param[in]  sampleDb  样本数据库主键
	/// @param[in]  Isfirst   是否是第一条
	/// @param[in]  Islast    是否最后一条
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月19日，新建函数
	///
	void UpdatePatientData(int sampleDb, bool Isfirst, bool Islast);

signals:
	///
	/// @brief 上一条或者下一条按钮被点击
	///
	/// @param[in]  orient  1：代表下一条，-1：代表下一条
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月19日，新建函数
	///
	void MoveButtonPressed(int orient);

protected:
	///
	/// @brief 初始化患者的表格信息
	///
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月19日，新建函数
	///
	void InitDialog();

	///
	/// @brief 设置表单初始状态
	///
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月19日，新建函数
	///
	void SetDeFaultStatus();

	///
	/// @brief 设置病人默认信息
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月20日，新建函数
	///
	void SetDefaultMessage();

	///
	/// @brief		获取自定义配置信息
	///     
	///
	/// @param[in]  type		患者信息定义值
	/// @param[in]  customList	自定义配置信息
	///
	/// @return 成功返回true
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年11月24日，新建函数
	///
	bool GetCustomData(int type, QStringList& customList, QMap<QString, QString>& noteMap);

	///
	/// @brief 更新下拉框
	///
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年11月24日，新建函数
	///
	void UpdateComBoBoxItem();

	///
	/// @brief 检查用户输入数据是否合法
	///
	///
	/// @return 合法返回true
	/// @par History:
	/// @li 7951/LuoXin，2022年11月24日，新建函数
	///
	bool CheckUserData();

    ///
    /// @brief 更新当前页面的显示项目
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年8月7日，新建函数
    ///
    void UpdatePageItems();

    ///
    /// @brief 设置布局中控件的显示/隐藏
    ///
    /// @param[in]  layout  布局指针
    /// @param[in]  visible  是否显示 (true:显示）
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年8月10日，新建函数
    ///
    void SetLayoutVisible(QLayout* layout, bool visible);

    ///
    /// @brief 设置病人信息
    ///
    /// @param[in]  sampleId  病人ID
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年8月15日，新建函数
    ///
    void ShowPatientInfo(int sampleId);

	///
	/// @brief 设置保存按钮的状态
	///
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年5月20日，新建函数
	///
	void SaveButtonStatus();

    ///
    /// @brief 设置当前样本号和样本条码编辑框规则
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年8月16日，新建函数
    ///
    void SetSeqNoAndBarcodeEditRule();

    ///
    /// @brief 重写按键事件
    ///
    /// @param[in]  e  事件
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年8月31日，新建函数
    ///
    virtual void keyPressEvent(QKeyEvent *e) override;

protected slots:
	///
	/// @brief 保存病人信息
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月19日，新建函数
	///
	void OnSavePatientInfo();

	///
	/// @brief 向上移动
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月24日，新建函数
	///
	void OnMovePreBtn();

	///
	/// @brief 向下移动
	///
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月24日，新建函数
	///
	void OnMoveNextBtn();

    ///
    /// @brief 根据配置显示页面信息
    ///
    /// @param[in]  set  病人信息配置数据
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年8月7日，新建函数
    ///
    void OnShowDisplaySet(DisplaySet set);

    ///
    /// @brief 应用页面患者信息相关页面更新
    ///
    /// @param[in]  set  配置信息
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年8月15日，新建函数
    ///
    void OnPatientSettingUpdated(DisplaySet set);

    ///
    /// @brief 响应数据字典页面 右侧表格数据信息发生变化
    ///
    /// @param[in]  dictKey  变化的数据字典主键
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年8月15日，新建函数
    ///
    void OnDataDictInfoUpdate(/*std::string dictKey*/);

    ///
    /// @brief 设置控件的几何位置和控件下拉框的基础数据
    ///
    /// @param[in]  type  类型
    /// @param[in]  enable  是否显示
    /// @param[in]  pos  排序位置
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年8月8日，新建函数
    ///
    void SetItemGeometryAndData(const DisplaySetItem& item, bool enable, int pos);

    ///
    /// @brief 样本信息更新
    ///
    /// @param[in]  enUpdateType  更新类型
    /// @param[in]  vSIs		  更新的样本信息数组
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年7月25日，新建函数
    ///
    void OnSampleUpdate(tf::UpdateType::type enUpdateType, std::vector<tf::SampleInfo, std::allocator<tf::SampleInfo>> vSIs);

private:
	Ui::QPatientDialog*				ui;
	int								m_sampleDb;
	int								m_patientId;
	bool							m_readOnly;
    QMap<QString, QLayoutItem*>     m_patientCtrls;             /// 页面的病人信息控件
    QLayoutItem*                    m_bottomSpacer;             /// 顶部的弹簧
    ::tf::SampleInfo                m_sampleInfo;               /// 样本信息
};
