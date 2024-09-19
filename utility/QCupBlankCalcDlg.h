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
/// @file     QCupBlankCalcDlg.h
/// @brief    杯空白计算对话框
///
/// @author   8580/GongZhiQiang
/// @date     2024年7月8日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年7月8日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once

#include "shared/basedlg.h"
#include <QVector>
#include <QMap>

namespace Ui { class QCupBlankCalcDlg; };

class QStandardItemModel;
class ChCupBlankCalculateInfo;

class QCupBlankCalcDlg : public BaseDlg
{
    Q_OBJECT

public:
	QCupBlankCalcDlg(QWidget *parent = Q_NULLPTR);
    ~QCupBlankCalcDlg();

	///
	/// @brief	显示结果
	///			
	///     
	struct CalculateData {
		
		std::string	time1;				/// 测定时间1
		std::string	time2;				/// 测定时间2
		QMap<int, QVector<int>>  absValue; /// 吸光度值
		CalculateData() = default;

		///
		/// @brief 清空数据
		///
		void Clear()
		{
			time1 = "";
			time1 = "";
			absValue.clear();
		}
	};

	///
	/// @brief 设置当前设备信息
	///
	/// @param[in]  devInfo 设备信息（模块名称和SN）
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年7月10日，新建函数
	///
	void SetDevSnInfo(const QPair<QString, QString>& devInfo);

protected:
    
    ///
    /// @brief 显示前初始化
    ///
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8580/GongZhiQiang，2024年7月8日，新建函数
    ///
    void InitBeforeShow();

    ///
    /// @brief 显示后初始化
    ///
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8580/GongZhiQiang，2024年7月8日，新建函数
    ///
    void InitAfterShow();

    ///
    /// @brief 初始化信号槽
    ///
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8580/GongZhiQiang，2024年7月8日，新建函数
    ///
    void InitConnect();

    ///
    /// @brief 初始化子控件
    ///
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8580/GongZhiQiang，2024年7月8日，新建函数
    ///
    void InitChildCtrl();
   
    ///
    /// @brief 显示事件处理
    ///
    /// @param[in]  event  事件
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8580/GongZhiQiang，2024年7月8日，新建函数
    ///
    void showEvent(QShowEvent *event);

protected Q_SLOTS:
	
	///
	/// @brief 计算按钮点击
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年7月8日，新建函数
	///
	void OnBtnCalculateClicked();

	///
	/// @brief 导出按钮点击
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年7月8日，新建函数
	///
	void OnBtnExportClicked();

	///
	/// @brief 打印按钮点击
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年7月8日，新建函数
	///
	void OnBtnPrintClicked();

private:
	
	///
	/// @brief 初始化表格控件
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年7月10日，新建函数
	///
	void InitTableCtrl();

	///
	/// @brief 更新表格信息
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年7月10日，新建函数
	///
	void UpdateTableInfo();

	///
	/// @brief 初始化时间下拉框
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年7月10日，新建函数
	///
	void InitTimeComboBox();

	///
	/// @brief 更新时间下拉框
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年7月10日，新建函数
	///
	void UpdateTimeComboBox();

	///
	/// @brief 获取杯空白测定结果
	///
	/// @param[in]  time  测定时间  
	/// @param[in]  adcData  结果数据（杯号，Vec(各个波长ADC值）)
	///
	/// @return true：获取成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年7月10日，新建函数
	///
	bool GetWaterBlankResult(const std::string& time, QMap<int, QVector<int>>& adcData);

	///
	/// @brief 计算吸光度
	///
	/// @param[in]  time1Data 测定时间1结果数据
	/// @param[in]  time2Data 测定时间2结果数据
	/// @param[in]  rData	  返回的计算结果
	///
	/// @return true：计算成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年7月10日，新建函数
	///
	bool CalculateAbsData(const QMap<int, QVector<int>>& time1Data, const QMap<int, QVector<int>>& time2Data, CalculateData& rData);

	///
	/// @brief 获取打印导出数据
	///
	/// @param[in]  info  杯空白计算打印导出数据
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年7月10日，新建函数
	///
	void GetPrintExportInfo(ChCupBlankCalculateInfo& info);

	///
	/// @brief 获取导出表格数据
	///
	/// @param[in]  strList  表格数据
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年7月10日，新建函数
	///
	void GetExportExcelData(QStringList& strList);

	///
	/// @brief 清空数据
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年7月10日，新建函数
	///
	void ClearData();

private:
    Ui::QCupBlankCalcDlg*				ui;                                // ui对象指针
    bool                                m_bInit;                           // 是否已经初始化

	QStandardItemModel*					m_pStdModel;                       // 计算结果标准模型
	QPair<QString, QString>			    m_devNameOfSN;					   // 模块名称与sn

	CalculateData						m_calculateData;			       // 计算结果
};
