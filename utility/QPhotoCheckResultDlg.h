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
/// @file    QPhotoCheckResultDlg.h
/// @brief 	 光度计检查结果
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
#include <QMap>
#include "shared/basedlg.h"

namespace Ui {
    class QPhotoCheckResultDlg;
};
class QStandardItemModel;
class ChPhotoCheckResultInfo;

#define  DEFAULT_ROW_CNT_OF_PHOTO_CHECK_TABLE                   (13)         // 光度计检查结果默认行数
#define  DEFAULT_COL_CNT_OF_PHOTO_CHECK_TABLE                   (3)          // 光度计检查结果默认列数


class QPhotoCheckResultDlg : public BaseDlg
{
	Q_OBJECT

public:
	QPhotoCheckResultDlg(QWidget *parent = Q_NULLPTR);
	~QPhotoCheckResultDlg();

	///
	/// @brief
	///     信息在表格中的对应列
	///
	enum DATA_COLUNM
	{
		COLUNM_WAVE,					// 波长
		COLUNM_ADC,						// ADC值
		COLUNM_RANGE					// 极差值
	};

	///
	/// @brief	光度计检查波长索引
	///			共16路，预留3路
	///     
	///
	enum  PHOTOCHECK_WAVE_INDEX
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

	///
	/// @brief	光度计检查结果
	///			
	///     
	struct  PhotoCheckData
	{
		qreal	adcValue;				// ADC值
		bool	adcIsNormal;			// ADC值是否正常
		qreal	rangeValue;				// 极差值
		bool	rangeIsNormal;			// 极差值是否正常
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

protected Q_SLOTS:
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

private:

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
	///     初始化信号槽连接
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年8月2日，新建函数
	///
	void InitConnect();

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
	///     初始化模块下拉选择框
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年8月2日，新建函数
	///
	void InitComboBoxModule();

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
	bool UpdatePhotoCheckData(bool isTimeChanged = false);

	///
	/// @brief
	///     更新表格
	///
	/// @param[in]  spStdModel 表格模型（当前数据||历史数据）
	/// @param[in]  result	 吸光度数据
	///
	/// @return		true:更新成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年8月2日，新建函数
	///
	bool UpdateResultTable(QStandardItemModel* const &spStdModel, const QVector<PhotoCheckData> &result);

	///
	/// @brief
	///     将JSON格式的数据结果转换为vector形式的数据
	///
	/// @param[in] jsonResult 字符串格式的测定结果
	/// @param[in] selectedResult 吸光度数据
	///
	/// @return		true:转换成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年8月2日，新建函数
	///
	bool GetCheckDataFromJson(const std::string &jsonResult, QVector<PhotoCheckData> &selectedResult);

	///
	/// @brief 获取打印数据
	///
	/// @param[in]  info  光度计检查测试记录
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年3月20日，新建函数
	///
	void GetPrintExportInfo(ChPhotoCheckResultInfo& info);

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

private:
	Ui::QPhotoCheckResultDlg* ui;

	bool                                   m_bInit;                           ///< 是否已经初始化
	QStandardItemModel*					   m_pCurStdModel;                    ///< 光度计检查最新结果标准模型
	QStandardItemModel*					   m_pHisStdModel;                    ///< 光度计检查历史结果标准模型
	QMap<QString, QString>			       m_devNameOfSN;					  ///< 模块名称与sn的映射

	QVector<PhotoCheckData>				   m_currentResult;					  ///< 当前数据
	QVector<PhotoCheckData>				   m_historyResult;					  ///< 历史数据
};
