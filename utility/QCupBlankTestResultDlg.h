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
/// @file    QCupBlankTestResultDlg.h
/// @brief 	 杯空白测定结果
///
/// @author  8580/GongZhiQiang
/// @date    2023年8月2日
/// @version 0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li GongZhiQiang，2023年8月2日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include <QVector>
#include <QMap>
#include "shared/basedlg.h"

namespace Ui {
    class QCupBlankTestResultDlg;
};
class QStandardItemModel;
class ChCupBlankTestInfo;

#define  REACTION_DISK_NUMBER_OF_CUPS							 (221)         // 反应盘杯子数
#define  REACTION_DISK_NUMBER_OF_UNITS							 (17)          // 反应盘联排数
#define  REACTION_DISK_NUMBER_OF_UNIT_CUPS						 (13)          // 反应盘一个联排的杯子数

#define  DEFAULT_ROW_CNT_OF_BLANK_RESULT_TABLE                   (13)          // 杯空白检测结果默认行数
#define  DEFAULT_COL_CNT_OF_BLANK_RESULT_TABLE                   (14)          // 杯空白检测结果默认列数

#define  CUP_BLANK_OPTICAL_PATH_LENGTH							 (4)		   // 吸光度计算光径(单位：mm)

///
/// @brief
///     信息在表格中的对应列
///
enum CUPBLANK_DATA_COLUNM
{
	COLUNM_CUP_NUM,					// 反应杯号
	COLUNM_ABS_340NM,				// 340nm
	COLUNM_ABS_380NM,				// 380nm
	COLUNM_ABS_405NM,				// 405nm
	COLUNM_ABS_450NM,				// 450nm
	COLUNM_ABS_480NM,				// 480nm
	COLUNM_ABS_505NM,				// 505nm
	COLUNM_ABS_546NM,				// 546nm
	COLUNM_ABS_570NM,				// 570nm
	COLUNM_ABS_600NM,				// 600nm
	COLUNM_ABS_660NM,				// 660nm
	COLUNM_ABS_700NM,				// 700nm
	COLUNM_ABS_750NM,				// 750nm
	COLUNM_ABS_800NM				// 800nm

};

///
/// @brief	杯空白测定波长索引
///			共16路，预留3路
///     
///
enum  CUPBLANK_WAVE_INDEX
{
	WAVE_340NM_INDEX,				// 340nm(0)
	WAVE_380NM_INDEX,				// 380nm(1)
	WAVE_405NM_INDEX,				// 405nm(2)
	WAVE_450NM_INDEX,				// 450nm(3)
	WAVE_480NM_INDEX,				// 480nm(4)
	WAVE_505NM_INDEX,				// 505nm(5)
	WAVE_546NM_INDEX,				// 546nm(6)
	WAVE_570NM_INDEX,				// 570nm(7)
	WAVE_600NM_INDEX,				// 600nm(8)
	WAVE_RESERVE1_INDEX,			// 预留1(9)
	WAVE_660NM_INDEX,				// 660nm(10)
	WAVE_700NM_INDEX,				// 700nm(11)
	WAVE_RESERVE2_INDEX,			// 预留2(12)
	WAVE_750NM_INDEX,				// 750nm(13)
	WAVE_800NM_INDEX,				// 800nm(14)
	WAVE_RESERVE3_INDEX				// 预留3(15)
};

class QCupBlankTestResultDlg : public BaseDlg
{
	Q_OBJECT

public:
	QCupBlankTestResultDlg(QWidget *parent = Q_NULLPTR);
	~QCupBlankTestResultDlg();

	///
	/// @brief	显示结果
	///			
	///     
	struct ShowData {
		int				cupNum;			/// 杯号
		QVector<int>	adcValue;		/// ADC值
		QVector<int>	absValue;		/// ABS值
		bool			isNormal;		/// 是否正常
		ShowData() = default;
		ShowData(int num, const QVector<int>& adcVal, const QVector<int>& absVal, bool isNor)
			:cupNum(num), adcValue(adcVal), absValue(absVal), isNormal(isNor)
		{
		}
	};

protected:
	///
	/// @brief
	///     界面显示前初始化
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年8月2日，新建函数
	///
	void InitBeforeShow();

	///
	/// @brief
	///     界面显示后初始化
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年8月2日，新建函数
	///
	void InitAfterShow();

	///
	/// @brief
	///     更新界面
	///
	/// @param[in]  isTimeChanged  是否是测定时间下拉框改变
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年8月2日，新建函数
	///
	void UpdatePage(bool isTimeChanged = false);
	

	///
	/// @brief
	///     窗口显示事件
	///
	/// @param[in]  event  事件对象
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年8月2日，新建函数
	///
	void showEvent(QShowEvent *event);

private:
	///
	/// @brief
	///     初始化数据表格控件
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年8月2日，新建函数
	///
	void InitTableCtrl();

	///
	/// @brief
	///     初始化反应盘控件
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年8月2日，新建函数
	///
	void InitReactionDisk();

	///
	/// @brief
	///     初始化模块下拉选择框
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年8月2日，新建函数
	///
	void InitComboBoxModule();

	///
	/// @brief
	///     初始化信号槽连接
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年8月2日，新建函数
	///
	void InitConnect();

	///
	/// @brief
	///     刷新前初始化
	///
	/// @param[in]  isTimeChanged  是否是测定时间下拉框改变
	///
	/// @return		true:初始化成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年8月2日，新建函数
	///
	bool InitBeforeUpdate(bool isTimeChanged = false);

	///
	/// @brief
	///     更新数据
	///
	/// @param[in]  isTimeChanged  是否是测定时间下拉框改变
	///
	/// @return		true:更新成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年8月2日，新建函数
	///
	bool UpdateCupBlankTestData(bool isTimeChanged = false);

	///
	/// @brief
	///     更新表格
	///
	/// @param[in]  unitIndex 反应盘联排编号
	///
	/// @return		true:更新成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年8月2日，新建函数
	///
	bool UpdateResultTable(int unitIndex);

	///
	/// @brief
	///     更新反应盘
	///
	///
	/// @return		true:更新成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年8月2日，新建函数
	///
	bool UpdateReactionDisk();

	///
	/// @brief
	///     将JSON格式的数据结果转换为map形式的数据
	///
	/// @param[in] jsonResult 字符串格式的测定结果
	/// @param[in] selectedResult 按照杯号映射的数据结果
	///
	/// @return		true:转换成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年8月2日，新建函数
	///
	bool GetTestDataFromJson(const std::string &jsonResult, QMap<int, ShowData> &selectedResult);

	///
	/// @brief
	///     更新异常反应杯号显示
	///
	/// @param[in]  unitAbnormalCups 当前联吸光度异常的杯号
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年8月2日，新建函数
	///
	void UpdateAbNormalCupsNotice(const QVector<int> &unitAbnormalCups);

protected Q_SLOTS:

	///
    /// @brief
    ///     表格选中行改变
    ///
    /// @par History:
    /// @li 8580/GongZhiQiang，2023年08月30日，新建函数
    ///
    void OnCurrentRowChanged(const QModelIndex &current, const QModelIndex &previous);

	///
	/// @brief
	///     导出功能
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年01月16日，新建函数
	///
	void OnExportBtnClicked();

	///
	/// @brief
	///     打印功能
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年03月20日，新建函数
	///
	void OnPrintBtnClicked();

	///
	/// @brief
	///     计算功能
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年03月20日，新建函数
	///
	void OnCalculateBtnClicked();


private:
	///
	/// @brief 获取打印数据
	///
	/// @param[in]  info  杯空白测试记录
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年3月20日，新建函数
	///
	void GetPrintExportInfo(ChCupBlankTestInfo& info);

	///
	/// @brief 获取导出表格数据
	///
	/// @param[in]  strList  表格数据
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年3月21日，新建函数
	///
	void GetExportExcelData(QStringList& strList);

	///
	/// @brief 是否显示ADC值
	///
	///
	/// @return true:ADC值，false:ABS值
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年7月8日，新建函数
	///
	bool IsShowADCValue();

private:
	Ui::QCupBlankTestResultDlg* ui;

	bool                                   m_bInit;                           ///< 是否已经初始化
	QStandardItemModel*					   m_pStdModel;                       ///< 杯空白测定结果标准模型
	QMap<QString, QString>			       m_devNameOfSN;					  ///< 模块名称与sn的映射
	QMap<int, ShowData>					   m_selectedResult;				  ///< 当前杯空白测定结果 (杯号，显示数据)

};
