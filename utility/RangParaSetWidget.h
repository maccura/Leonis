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
/// @file     RangeParaWidget.h
/// @brief    项目参考范围参数配置
///
/// @author   7951/LuoXin
/// @date     2022年6月13日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2022年6月13日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once
#include "shared/basedlg.h"
#include "src/thrift/gen-cpp/defs_types.h"

class QPushButton;
class QComboBox;

#define STR_AUTO_DEFAULT_Y			("Y")			// 在没有条件判断时，默认使用该条参考范围
#define STR_AUTO_DEFAULT_N			("N")			// 在没有条件判断时，默认不使用使用该条参考范围

enum RangeSetParamDlgType{RPD_Im, RPD_Ch, CALC}; // 项目参数范围设置框类型（Im类型有额外的参考值范围）
enum RangeSetParamDlgEditType {Et_Add, Et_Modify}; // 新增、修改类型

namespace Ui { class CRangParaSetWidget; };


class CRangParaSetWidget : public BaseDlg
{
	Q_OBJECT

public:
	///
	/// @brief
	///     构造函数
	///
	/// @param[in]  parent  
	///
	/// @return 
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年6月13日，新建函数
	///
	CRangParaSetWidget(RangeSetParamDlgType rangeDlgType, QWidget *parent = Q_NULLPTR);
	///
	/// @brief
	///     析构函数
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年6月13日，新建函数
	///
	~CRangParaSetWidget();

	///
	/// @brief	检查用户输入数据是否合法
	///     
	/// @return 合法返回true
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年12月15日，新建函数
	///
	bool CheckUserInputData();

	///
	/// @brief
	///     获取参考范围配置
	///
	/// @param[in]  item  
	///
	/// @return 成功返回true
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年6月13日，新建函数
	///
	bool getSetParaData(tf::AssayReferenceItem &item);

    ///
    /// @brief	设置样本源下拉框
    /// @param[in]  sampleSource  样本源
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023.年8月30日，新建函数
    ///
    void SetSampleTypeCommbox(int sampleSource);

	///
	/// @brief
	///     外部获取保存按钮的指针
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年6月13日，新建函数
	///
    QPushButton *getSaveBtn();

	///
	/// @brief
	///     加载范围配置页面参数
	///
	/// @param[in]  item  选中参考范围
	///
	/// @return 
	///
	/// @par History:
	/// @li 1226/zhangjing，2023年4月26日，新建函数
	///
	void loadRangParaSetWidget(const tf::AssayReferenceItem &item);

	///
	/// @brief
	///     事件过滤器
	///
	/// @param[in]  obj    ui对象
	/// @param[in]  event  事件
	///
	/// @return true表示已处理
	///
	/// @par History:
	/// @li 1226/zhangjing，2023年5月9日，新建函数
	///
	bool eventFilter(QObject *obj, QEvent *event);

    ///
    /// @bref
    ///		设定对数据的处理类型，新增或者修改，并显示界面
    ///
    /// @param[in] eType 处理类型
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年9月15日，新建函数
    ///
    void ShowDlg(RangeSetParamDlgEditType eType);

    inline RangeSetParamDlgEditType GetEditType() { return m_editType; }

private:

    ///
    /// @brief
    ///     初始化ComBox
    ///
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 7951/LuoXin，2022年6月13日，新建函数
    ///
    void initCombox();

    ///
    /// @bref
    ///		样本类型转字符串(生免差异)
    ///
    /// @param[in] iSampleType 样本类型
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年4月18日，新建函数
    ///
    QString TransSampleName(int iSampleType);

protected Q_SLOTS:
	///
	/// @brief
	///     参考范围编辑框获得焦点
	///
	/// @par History:
	/// @li 1226/zhangjing，2023年5月9日，新建函数
	///
	void OnRefRangeEditFocusIn();

    ///
    /// @bref
    ///		是否为缺省值的下拉框发生变化
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年9月13日，新建函数
    ///
    void OnDefaultValueFlagChanged(const QString& text);

private:
	Ui::CRangParaSetWidget		*ui;                         // UI指针
    int                         m_defaultSampleSource;       // 默认样本源
    RangeSetParamDlgType        m_rangeDlgType;              // 弹窗类型
    RangeSetParamDlgEditType    m_editType;                  // 数据处理类型
};
