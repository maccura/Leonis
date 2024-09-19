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
/// @file     QAssayResultShowCurve.h
/// @brief    测试结果的曲线展示
///
/// @author   5774/WuHongTao
/// @date     2022年5月30日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月30日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QAssayResultShowCurve.h"
#include "calibration/ch/CaliBrateCommom.h"
#include "curveDraw/common.h"
#include "src/common/Mlog/mlog.h"


QAssayResultShowCurve::QAssayResultShowCurve(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);
	InitWidget();
}

QAssayResultShowCurve::~QAssayResultShowCurve()
{
}

///
/// @brief 显示项目反应过程
///
/// @param[in]  result  项目测试结果
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月30日，新建函数
///
void QAssayResultShowCurve::ShowAssayResultReactionProcess(ch::tf::AssayTestResult& result)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
	ui.reaction_curve->clearCurve();

	// 获取曲线
	std::vector<std::shared_ptr<curveSingleDataStore>> CurveList;
	if (!CaliBrateCommom::GetScatterFromResult(result, CurveList) || CurveList.empty())
	{
		return;
	}

	// 依次显示曲线
	for (auto curve : CurveList)
	{
		if (curve == nullptr)
		{
			continue;
		}

		// 添加曲线
		ui.reaction_curve->addCurveData(*curve);
	}

	// 显示波长的数值
	//ShowDetectPointView(result);
}

///
/// @brief 清除历史残留数据
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月30日，新建函数
///
void QAssayResultShowCurve::ClearContent()
{
	// 清除表格
	//ClearViewContent();
	// 清除曲线信息
	ui.reaction_curve->clearCurve();
}

///
/// @brief 初始化控件
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月30日，新建函数
///
void QAssayResultShowCurve::InitWidget()
{
	//初始化控件最开始的大小
	CoordinateDataType tmpSetData = ui.reaction_curve->GetCurveAttrbute();
	// 改变类型
	tmpSetData.scaleType = 1;
	// 改变单位
	tmpSetData.unit = QString("");
	// 坐标系小数点余数
	tmpSetData.retainedNumber = 0;
	// 设置坐标属性
	ui.reaction_curve->setAttribute(tmpSetData);
	// 设置数据反应模块为空
	//m_CurvePointMode = nullptr;
	// 限制新增数目
	QRegExp regx("[-]?[0-4][0-9]{0,4}");
}

///
/// @brief
///     显示曲线的检测点
///
/// @param[in]  result  曲线检测点
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月2日，新建函数
///
//void QAssayResultShowCurve::ShowDetectPointView(ch::tf::AssayTestResult& result)
//{
//	int primWave = -1;
//	int subWave = -1;
//
//	ClearViewContent();
//
//	// 仅有主波长
//	if (result.primarySubWaves.size() == 1)
//	{
//		primWave = result.primarySubWaves[0];
//	}
//	// 主次波长都有
//	else if (result.primarySubWaves.size() >= 2)
//	{
//		primWave = result.primarySubWaves[0];
//		subWave = result.primarySubWaves[1];
//	}
//
//	// 依次显示内容
//	int column = 0;
//	for (auto detectPoint : result.detectPoints)
//	{
//		int row = 0;
//		// 测光点编号
//		m_CurvePointMode->setItem(row++, column, new QStandardItem(QString::number(detectPoint.pointSN)));
//		// 主波长
//		if (primWave == -1)
//		{
//			m_CurvePointMode->setItem(row++, column, new QStandardItem(""));
//		}
//		else
//		{
//			m_CurvePointMode->setItem(row++, column, new QStandardItem(QString::number(detectPoint.ods[primWave])));
//		}
//
//		if (subWave == -1)
//		{
//			m_CurvePointMode->setItem(row++, column, new QStandardItem(""));
//			m_CurvePointMode->setItem(row++, column, new QStandardItem(""));
//		}
//		else
//		{
//			// 次波长
//			m_CurvePointMode->setItem(row++, column, new QStandardItem(QString::number(detectPoint.ods[subWave])));
//			// 主-次
//			m_CurvePointMode->setItem(row++, column, new QStandardItem(QString::number(detectPoint.ods[primWave] - detectPoint.ods[subWave])));
//		}
//
//		column++;
//	}
//}

///
/// @brief 清空测光点的表格内容
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月30日，新建函数
///
//void QAssayResultShowCurve::ClearViewContent()
//{
	//ULOG(LOG_INFO, "%s()", __FUNCTION__);
	//if (m_CurvePointMode == nullptr)
	//{
		//m_CurvePointMode = new QStandardItemModel(this);
		//ui.result_process_view->setModel(m_CurvePointMode);
		//ui.result_process_view->setSelectionBehavior(QAbstractItemView::SelectRows);
		//ui.result_process_view->setSelectionMode(QAbstractItemView::SingleSelection);
		//ui.result_process_view->setEditTriggers(QAbstractItemView::NoEditTriggers);
	//}

	//m_CurvePointMode->clear();
	//QStringList CurveTypeShowList;
	//CurveTypeShowList << tr("测光点") << tr("主波长") << tr("次波长") << tr("主-次");
	//m_CurvePointMode->setVerticalHeaderLabels(CurveTypeShowList);
	//m_CurvePointMode->setColumnCount(34);
//}

///
/// @brief 最大吸光度的修改
///
/// @param[in]  absorb 最大吸光度的值  
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月30日，新建函数
///
void QAssayResultShowCurve::OnModifyMaxAbsorb(QString absorb)
{
	CoordinateDataType tmpSetData = ui.reaction_curve->GetCurveAttrbute();
	tmpSetData.needRefreshYScale = true;
	//tmpSetData.maxYsacle = max;
	ui.reaction_curve->setAttribute(tmpSetData);
}

///
/// @brief 设置校准品反应次数信息
///
/// @param[in]  items  项目列表
///
/// @return 
///
/// @par History:
/// @li 8090/YeHuaNing，2022年9月13日，新建函数
///
void QAssayResultShowCurve::SetCalibrateAbstract(const QStringList & items)
{
	
}

///
/// @brief 设置默认视图比例
///
/// @par History:
/// @li 8090/YeHuaNing，2022年9月13日，新建函数
///
void QAssayResultShowCurve::setDefaultViewScale()
{
	ui.reaction_curve->setDefaultViewScale();
}

///
/// @brief 最小吸光度的修改
///
/// @param[in]  absorb  最小吸光度的值
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月30日，新建函数
///
void QAssayResultShowCurve::OnModifyMinAbsorb(QString absorb)
{
	CoordinateDataType tmpSetData = ui.reaction_curve->GetCurveAttrbute();


	tmpSetData.needRefreshYScale = true;
	//tmpSetData.minYscale = min;
	ui.reaction_curve->setAttribute(tmpSetData);
}


///
/// @brief 坐标系自动跟随
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月30日，新建函数
///
void QAssayResultShowCurve::OnAutoBtnClicked()
{
	// 设置坐标系为自动
	CoordinateDataType tmpSetData = ui.reaction_curve->GetCurveAttrbute();
	tmpSetData.needRefreshYScale = false;
	ui.reaction_curve->setAttribute(tmpSetData);
}

///
/// @brief
///     根据设置的手动数据更新坐标系
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月4日，新建函数
///
void QAssayResultShowCurve::OnManualBtnClicked()
{
	CoordinateDataType tmpSetData = ui.reaction_curve->GetCurveAttrbute();
	//tmpSetData.minYscale = min;
	//tmpSetData.maxYsacle = max;
	tmpSetData.needRefreshYScale = true;
	ui.reaction_curve->setAttribute(tmpSetData);
}
