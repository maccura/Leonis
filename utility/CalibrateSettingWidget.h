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

#ifndef CALIBRATESETTINGWIDGET_H
#define CALIBRATESETTINGWIDGET_H

#include "model/AssayListModel.h"
#include <QWidget>

namespace Ui
{
    class CalibrateSettingWidget;
};

namespace ch
{
    namespace tf
    {
        class SampleAspirateVol;
        class GeneralAssayInfo;
    }
};

class CalibrateSettingWidget : public QWidget
{
    Q_OBJECT

public:
    explicit CalibrateSettingWidget(QWidget *parent = nullptr);
    ~CalibrateSettingWidget();

	///
	/// @brief
	///     加载指定项目的分析参数
	///
	/// @param[in]  item  数据
	///
	/// @par History:
	/// @li 4170/TangChuXian，2020年8月24日，新建函数
	///
	bool LoadAnalysisParam(const AssayListModel::StAssayListRowItem& item);

    ///
    /// @brief  获取编辑的项目分析参数
    ///     
    /// @param[in]  chGAI       生化项目参数
    /// @param[in]  assayCode   项目编号
    ///
    /// @return 成功返回true
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年11月30日，新建函数
    ///
    bool GetAnalysisParam(ch::tf::GeneralAssayInfo& chGAI, int assayCode);

	///
	/// @brief
	///     复位界面
	///
	/// @par History:
	/// @li 6950/ChenFei，2022年05月23日，新建函数
	///
	void Reset();

	///
	/// @brief
	///     初始化控件
	///
	/// @par History:
	/// @li 6950/ChenFei，2022年05月23日，新建函数
	///
	void InitCtrls();

	///
	/// @brief 为输入框设置正则表达式
	///
	/// @par History:
	/// @li 6950/ChenFei，2022年10月24日，新建函数
	///
	void SetCtrlsRegExp();

    ///
    /// @brief	更新控件的显示和编辑权限
    ///    
    /// @param[in]  isOpenAssay 是否为开放项目  
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年2月20日，新建函数
    ///
    void UpdateCtrlsEnabled(bool isOpenAssay);

    ///
    /// @brief
    ///     当前页面是否有未保存的数据
    ///
    ///@param[in]    item 项目信息
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年4月11日，新建函数
    ///
    bool HasUnSaveChangedData(const AssayListModel::StAssayListRowItem& item);

public Q_SLOTS :

    ///
    /// @bref
    ///		权限变化响应
    ///
    /// @par History:
    /// @li 7951/LuoXin, 2024年3月13日，新建函数
    ///
    void OnPermisionChanged();

	///
	/// @brief
	///     槽函数，设置表格行数
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年6月23日，新建函数
	///
	void OnsetTableViewRows(const QString& text);

    ///
    /// @brief
    ///     槽函数，默认样本量改变
    ///
    /// @par History:
    /// @li 7951/LuoXin，2022年6月23日，新建函数
    ///
    void OnDefaultSampleVolChanged(double vol);

private:
    Ui::CalibrateSettingWidget*                     ui;
    int                                             m_currentCode;
    double                                          m_defaultSampleVol;         // 默认的稀释浓度样本量
};

#endif // CALIBRATESETTINGWIDGET_H
