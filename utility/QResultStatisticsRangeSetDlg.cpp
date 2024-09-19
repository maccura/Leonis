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
/// @file     QResultStatisticsRangeSetDlg.h
/// @brief 	  应用->统计分析->结果统计->结果分段设置
///
/// @author   7656/zhang.changjiang
/// @date      2023年7月19日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年7月19日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QResultStatisticsRangeSetDlg.h"
#include "ui_QResultStatisticsRangeSetDlg.h"
#include "shared/tipdlg.h"
#include "src/common/Mlog/mlog.h"

QResultStatisticsRangeSetDlg::QResultStatisticsRangeSetDlg(QWidget *parent)
	: BaseDlg(parent)
{
	ui = new Ui::QResultStatisticsRangeSetDlg();
	ui->setupUi(this);
	InitBeforeShow();
}

QResultStatisticsRangeSetDlg::~QResultStatisticsRangeSetDlg()
{
	delete ui;
}

void QResultStatisticsRangeSetDlg::showEvent(QShowEvent * event)
{
	ShowLastSettingRange();
}

///
/// @brief  设置参数范围选中
///
/// @param[in]    void
///
/// @return void
///
/// @par History:
/// @li 7915/LeiDingXiang，2023年9月14日，新建函数
///
void QResultStatisticsRangeSetDlg::SetSpinBoxFocus()
{
    ui->doubleSpinBox_lv_1->setFocus();
}

///
/// @brief  获取分段条件列表
///
///
/// @return 分段条件列表
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月15日，新建函数
///
std::vector<::tf::RangQueryCond> QResultStatisticsRangeSetDlg::GetRangQueryConds()
{
	return m_vecRangQueryCond;
}

///
///  @brief 显示前初始化
///
///
///
///  @return	
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年7月17日，新建函数
///
void QResultStatisticsRangeSetDlg::InitBeforeShow()
{
	SetTitleName(tr("结果分段设置"));
	m_vecRangQueryCond = CreateRangQueryCondVec();
	connect(ui->cancerBtn, &QPushButton::clicked, this, [&]() {close(); });
	connect(ui->okBtn, &QPushButton::clicked, this, &QResultStatisticsRangeSetDlg::OnOkBtnClicked);
	connect(ui->comboBox_lc_1, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int index) 
	{
		ui->comboBox_rc_1->setEnabled(index != ::tf::LowerSymbol::type::EQUAL_TO);
		ui->doubleSpinBox_rv_1->setEnabled(index != ::tf::LowerSymbol::type::EQUAL_TO);
	});

	connect(ui->comboBox_lc_2, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int index)
	{
		ui->comboBox_rc_2->setEnabled(index != ::tf::LowerSymbol::type::EQUAL_TO);
		ui->doubleSpinBox_rv_2->setEnabled(index != ::tf::LowerSymbol::type::EQUAL_TO);
	});

	connect(ui->comboBox_lc_3, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int index)
	{
		ui->comboBox_rc_3->setEnabled(index != ::tf::LowerSymbol::type::EQUAL_TO);
		ui->doubleSpinBox_rv_3->setEnabled(index != ::tf::LowerSymbol::type::EQUAL_TO);
	});

	connect(ui->comboBox_lc_4, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int index)
	{
		ui->comboBox_rc_4->setEnabled(index != ::tf::LowerSymbol::type::EQUAL_TO);
		ui->doubleSpinBox_rv_4->setEnabled(index != ::tf::LowerSymbol::type::EQUAL_TO);
	});

	connect(ui->comboBox_lc_5, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int index)
	{
		ui->comboBox_rc_5->setEnabled(index != ::tf::LowerSymbol::type::EQUAL_TO);
		ui->doubleSpinBox_rv_5->setEnabled(index != ::tf::LowerSymbol::type::EQUAL_TO);
	});

	connect(ui->comboBox_lc_6, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int index)
	{
		ui->comboBox_rc_6->setEnabled(index != ::tf::LowerSymbol::type::EQUAL_TO);
		ui->doubleSpinBox_rv_6->setEnabled(index != ::tf::LowerSymbol::type::EQUAL_TO);
	});

	connect(ui->comboBox_lc_7, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int index)
	{
		ui->comboBox_rc_7->setEnabled(index != ::tf::LowerSymbol::type::EQUAL_TO);
		ui->doubleSpinBox_rv_7->setEnabled(index != ::tf::LowerSymbol::type::EQUAL_TO);
	});

	connect(ui->comboBox_lc_8, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int index)
	{
		ui->comboBox_rc_8->setEnabled(index != ::tf::LowerSymbol::type::EQUAL_TO);
		ui->doubleSpinBox_rv_8->setEnabled(index != ::tf::LowerSymbol::type::EQUAL_TO);
	});

	connect(ui->comboBox_lc_9, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int index)
	{
		ui->comboBox_rc_9->setEnabled(index != ::tf::LowerSymbol::type::EQUAL_TO);
		ui->doubleSpinBox_rv_9->setEnabled(index != ::tf::LowerSymbol::type::EQUAL_TO);
	});

	connect(ui->comboBox_lc_10, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int index)
	{
		ui->comboBox_rc_10->setEnabled(index != ::tf::LowerSymbol::type::EQUAL_TO);
		ui->doubleSpinBox_rv_10->setEnabled(index != ::tf::LowerSymbol::type::EQUAL_TO);
	});

}

///
/// @brief  获取区间结构体
///
/// @param[in]  lc  左区间符号控件
/// @param[in]  lv  右区间符号控件
/// @param[in]  rc  区间左值
/// @param[in]  rv  区间右值
///
/// @return 区间结构
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月14日，新建函数
///
::tf::RangQueryCond QResultStatisticsRangeSetDlg::GetRangQueryCond(QComboBox *lc, QDoubleSpinBox *lv, QComboBox *rc, QDoubleSpinBox *rv)
{
	if (lc == nullptr || lv == nullptr || rc == nullptr || rv == nullptr)
	{
		return ::tf::RangQueryCond();
	}
	::tf::RangQueryCond rangeCond;
	rangeCond.__set_bUse(!(lc->currentIndex() != ::tf::LowerSymbol::type::EQUAL_TO && FloatIsEqual(lv->value(), 0) && FloatIsEqual(rv->value(), 0)));
	rangeCond.__set_lCond(static_cast<::tf::LowerSymbol::type>(lc->currentIndex()));
	rangeCond.__set_lData(lv->value());
	rangeCond.__set_rCond(static_cast<::tf::UpperSymbol::type>(rc->currentIndex()));
	rangeCond.__set_rData(rv->value());
	return std::move(rangeCond);
}

///
/// @brief  创建区间列表
///
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月14日，新建函数
///
std::vector<::tf::RangQueryCond> QResultStatisticsRangeSetDlg::CreateRangQueryCondVec()
{
	std::vector<::tf::RangQueryCond> vecRangQueryCond;
	vecRangQueryCond.push_back(GetRangQueryCond(ui->comboBox_lc_1, ui->doubleSpinBox_lv_1, ui->comboBox_rc_1, ui->doubleSpinBox_rv_1));
	vecRangQueryCond.push_back(GetRangQueryCond(ui->comboBox_lc_2, ui->doubleSpinBox_lv_2, ui->comboBox_rc_2, ui->doubleSpinBox_rv_2));
	vecRangQueryCond.push_back(GetRangQueryCond(ui->comboBox_lc_3, ui->doubleSpinBox_lv_3, ui->comboBox_rc_3, ui->doubleSpinBox_rv_3));
	vecRangQueryCond.push_back(GetRangQueryCond(ui->comboBox_lc_4, ui->doubleSpinBox_lv_4, ui->comboBox_rc_4, ui->doubleSpinBox_rv_4));
	vecRangQueryCond.push_back(GetRangQueryCond(ui->comboBox_lc_5, ui->doubleSpinBox_lv_5, ui->comboBox_rc_5, ui->doubleSpinBox_rv_5));
	vecRangQueryCond.push_back(GetRangQueryCond(ui->comboBox_lc_6, ui->doubleSpinBox_lv_6, ui->comboBox_rc_6, ui->doubleSpinBox_rv_6));
	vecRangQueryCond.push_back(GetRangQueryCond(ui->comboBox_lc_7, ui->doubleSpinBox_lv_7, ui->comboBox_rc_7, ui->doubleSpinBox_rv_7));
	vecRangQueryCond.push_back(GetRangQueryCond(ui->comboBox_lc_8, ui->doubleSpinBox_lv_8, ui->comboBox_rc_8, ui->doubleSpinBox_rv_8));
	vecRangQueryCond.push_back(GetRangQueryCond(ui->comboBox_lc_9, ui->doubleSpinBox_lv_9, ui->comboBox_rc_9, ui->doubleSpinBox_rv_9));
	vecRangQueryCond.push_back(GetRangQueryCond(ui->comboBox_lc_10, ui->doubleSpinBox_lv_10, ui->comboBox_rc_10, ui->doubleSpinBox_rv_10));

	return std::move(vecRangQueryCond);
}

///
/// @brief  找出非法区间
///
/// @param[in]  vecRangQueryCond  区间列表
///
/// @return 非法区间
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月16日，新建函数
///
std::map<int, ::tf::RangQueryCond> QResultStatisticsRangeSetDlg::FindInvalidRanges(std::vector<::tf::RangQueryCond>& vecRangQueryCond)
{
	std::map<int, ::tf::RangQueryCond>  result;
	for (size_t i = 0; i < vecRangQueryCond.size(); ++i)
	{
		if (vecRangQueryCond[i].lData > 0 && FloatIsEqual(vecRangQueryCond[i].rData, 0))
		{
			vecRangQueryCond[i].__set_rData(INFINITE);
		}

		if (FloatIsEqual(vecRangQueryCond[i].lData, 0) && FloatIsEqual(vecRangQueryCond[i].rData, 0))
		{
			continue;
		}

		if ((vecRangQueryCond[i].lData > vecRangQueryCond[i].rData) || FloatIsEqual(vecRangQueryCond[i].lData, vecRangQueryCond[i].rData))
		{
			result[i] = vecRangQueryCond[i];
		}
	}

	return result;
}

///
/// @brief  判断两个区间是否有交集
///
/// @param[in]  range1 区间1 
/// @param[in]  range2 区间2
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月14日，新建函数
///
bool QResultStatisticsRangeSetDlg::HasIntersection(const::tf::RangQueryCond & range1, const::tf::RangQueryCond & range2)
{
	// 如果有一个区间不可用，认为它们没有交集
	if (!range1.bUse || !range2.bUse)
	{
		return false;
	}

	// 如果两个区间都是取定值
	if (range1.lCond == ::tf::LowerSymbol::type::EQUAL_TO && range2.lCond == ::tf::LowerSymbol::type::EQUAL_TO)
	{
		return FloatIsEqual(range1.lData, range2.lData);
	}

	// 如果range1取定值
	if (range1.lCond == ::tf::LowerSymbol::type::EQUAL_TO)
	{
		// 没有交集的情况
		if (range1.lData < range2.lData || range1.lData > range2.rData)
		{
			return false;
		}

		// 不取等的情况
		if (range2.lCond != ::tf::LowerSymbol::type::GREATER_THAN_OR_EQUAL_TO && FloatIsEqual(range1.lData, range2.lData))
		{
			return false;
		}

		// 不取等的情况
		if (range2.rCond != ::tf::LowerSymbol::type::GREATER_THAN_OR_EQUAL_TO && FloatIsEqual(range1.lData, range2.rData))
		{
			return false;
		}

		// 有交集的情况
		return true;
	}
	
	if (range2.lCond == ::tf::LowerSymbol::type::EQUAL_TO)
	{
		// 没有交集的情况
		if (range2.lData < range1.lData || range2.lData > range1.rData)
		{
			return false;
		}

		// 不取等的情况
		if (range1.lCond != ::tf::LowerSymbol::type::GREATER_THAN_OR_EQUAL_TO && FloatIsEqual(range2.lData, range1.lData))
		{
			return false;
		}

		// 不取等的情况
		if (range1.rCond != ::tf::LowerSymbol::type::GREATER_THAN_OR_EQUAL_TO && FloatIsEqual(range2.lData, range1.rData))
		{
			return false;
		}

		// 有交集的情况
		return true;
	}

	// 两个区间都不是定值
	// 没有交集的情况
	if (range1.rData < range2.lData || range1.lData > range2.rData)
	{
		return false;
	}

	// 不取等的情况
	if (range1.rCond != ::tf::LowerSymbol::type::GREATER_THAN_OR_EQUAL_TO && FloatIsEqual(range1.rData, range2.lData))
	{
		return false;
	}

	// 不取等的情况
	if (range2.rCond != ::tf::UpperSymbol::type::LESS_THAN_OR_EQUAL_TO && FloatIsEqual(range1.lData, range2.rData))
	{
		return false;
	}

	// 有交集的情况
	return true;
}

///
/// @brief  找到所有有交集的区间并返回
///
/// @param[in]  vecRangQueryCond  区间列表
///
/// @return 有交集的区间
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月16日，新建函数
///
std::map<int, ::tf::RangQueryCond> QResultStatisticsRangeSetDlg::FindIntersectingRanges(const std::vector<::tf::RangQueryCond>& vecRangQueryCond)
{
	std::map<int, ::tf::RangQueryCond> result;
	for (size_t i = 0; i < vecRangQueryCond.size(); ++i)
	{
		for (size_t j = i + 1; j < vecRangQueryCond.size(); ++j)
		{
			if (HasIntersection(vecRangQueryCond[i], vecRangQueryCond[j]))
			{
				result[i] = vecRangQueryCond[i];
				result[j] = vecRangQueryCond[j];
			}
		}
	}

	return result;
}

///
/// @brief  显示上次设置的区间
///
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月15日，新建函数
///
void QResultStatisticsRangeSetDlg::ShowLastSettingRange()
{
	if (m_vecRangQueryCond.size() < 10)
	{
		return;
	}
	
	ui->comboBox_lc_1->setCurrentIndex(m_vecRangQueryCond[0].lCond);
	ui->comboBox_rc_1->setCurrentIndex(m_vecRangQueryCond[0].rCond);
	ui->doubleSpinBox_lv_1->setValue(m_vecRangQueryCond[0].lData);
	ui->doubleSpinBox_rv_1->setValue(m_vecRangQueryCond[0].rData == INFINITE ? 0 : m_vecRangQueryCond[0].rData);

	ui->comboBox_lc_2->setCurrentIndex(m_vecRangQueryCond[1].lCond);
	ui->comboBox_rc_2->setCurrentIndex(m_vecRangQueryCond[1].rCond);
	ui->doubleSpinBox_lv_2->setValue(m_vecRangQueryCond[1].lData);
	ui->doubleSpinBox_rv_2->setValue(m_vecRangQueryCond[1].rData == INFINITE ? 0 : m_vecRangQueryCond[1].rData);

	ui->comboBox_lc_3->setCurrentIndex(m_vecRangQueryCond[2].lCond);
	ui->comboBox_rc_3->setCurrentIndex(m_vecRangQueryCond[2].rCond);
	ui->doubleSpinBox_lv_3->setValue(m_vecRangQueryCond[2].lData);
	ui->doubleSpinBox_rv_3->setValue(m_vecRangQueryCond[2].rData == INFINITE ? 0 : m_vecRangQueryCond[2].rData);

	ui->comboBox_lc_4->setCurrentIndex(m_vecRangQueryCond[3].lCond);
	ui->comboBox_rc_4->setCurrentIndex(m_vecRangQueryCond[3].rCond);
	ui->doubleSpinBox_lv_4->setValue(m_vecRangQueryCond[3].lData);
	ui->doubleSpinBox_rv_4->setValue(m_vecRangQueryCond[3].rData == INFINITE ? 0 : m_vecRangQueryCond[3].rData);

	ui->comboBox_lc_5->setCurrentIndex(m_vecRangQueryCond[4].lCond);
	ui->comboBox_rc_5->setCurrentIndex(m_vecRangQueryCond[4].rCond);
	ui->doubleSpinBox_lv_5->setValue(m_vecRangQueryCond[4].lData);
	ui->doubleSpinBox_rv_5->setValue(m_vecRangQueryCond[4].rData == INFINITE ? 0 : m_vecRangQueryCond[4].rData);

	ui->comboBox_lc_6->setCurrentIndex(m_vecRangQueryCond[5].lCond);
	ui->comboBox_rc_6->setCurrentIndex(m_vecRangQueryCond[5].rCond);
	ui->doubleSpinBox_lv_6->setValue(m_vecRangQueryCond[5].lData);
	ui->doubleSpinBox_rv_6->setValue(m_vecRangQueryCond[5].rData == INFINITE ? 0 : m_vecRangQueryCond[5].rData);

	ui->comboBox_lc_7->setCurrentIndex(m_vecRangQueryCond[6].lCond);
	ui->comboBox_rc_7->setCurrentIndex(m_vecRangQueryCond[6].rCond);
	ui->doubleSpinBox_lv_7->setValue(m_vecRangQueryCond[6].lData);
	ui->doubleSpinBox_rv_7->setValue(m_vecRangQueryCond[6].rData == INFINITE ? 0 : m_vecRangQueryCond[6].rData);

	ui->comboBox_lc_8->setCurrentIndex(m_vecRangQueryCond[7].lCond);
	ui->comboBox_rc_8->setCurrentIndex(m_vecRangQueryCond[7].rCond);
	ui->doubleSpinBox_lv_8->setValue(m_vecRangQueryCond[7].lData);
	ui->doubleSpinBox_rv_8->setValue(m_vecRangQueryCond[7].rData == INFINITE ? 0 : m_vecRangQueryCond[7].rData);

	ui->comboBox_lc_9->setCurrentIndex(m_vecRangQueryCond[8].lCond);
	ui->comboBox_rc_9->setCurrentIndex(m_vecRangQueryCond[8].rCond);
	ui->doubleSpinBox_lv_9->setValue(m_vecRangQueryCond[8].lData);
	ui->doubleSpinBox_rv_9->setValue(m_vecRangQueryCond[8].rData == INFINITE ? 0 : m_vecRangQueryCond[8].rData);

	ui->comboBox_lc_10->setCurrentIndex(m_vecRangQueryCond[9].lCond);
	ui->comboBox_rc_10->setCurrentIndex(m_vecRangQueryCond[9].rCond);
	ui->doubleSpinBox_lv_10->setValue(m_vecRangQueryCond[9].lData);
	ui->doubleSpinBox_rv_10->setValue(m_vecRangQueryCond[9].rData == INFINITE ? 0 : m_vecRangQueryCond[9].rData);
}

///
/// @brief  判断两个浮点数是否相等
///
/// @param[in]  f1  浮点数1
/// @param[in]  f2  浮点数2
///
/// @return true 相等 false 不等
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月28日，新建函数
///
bool QResultStatisticsRangeSetDlg::FloatIsEqual(double f1, double f2)
{
	bool dd = std::abs(f1 - f2) < 1e-9;
	return dd;
}

///
/// @brief  确定按钮被点击
///
///
/// @return 
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年11月15日，新建函数
///
void QResultStatisticsRangeSetDlg::OnOkBtnClicked()
{
	// 获取用户设置的区间
	std::vector<::tf::RangQueryCond> vecRangQueryCond = CreateRangQueryCondVec();

	// 检查区间是否合法
	std::map<int, ::tf::RangQueryCond> mapInvalidRanges = FindInvalidRanges(vecRangQueryCond);
	if (!mapInvalidRanges.empty())
	{
		QString strDlg;
		for (auto it = mapInvalidRanges.begin(); it != mapInvalidRanges.end(); ++it)
		{
			strDlg += tr("范围") + QString::number(it->first + 1);
			strDlg += it->second.lCond == ::tf::LowerSymbol::type::EQUAL_TO ? QString("[%1]").arg(it->second.lData == INFINITE ? "∞" : QString::number(it->second.lData, 'f', 4)) : QString("%1%2,%3%4").
				arg(it->second.lCond ? "[" : "(").
				arg(it->second.lData == INFINITE ? "-∞" : QString::number(it->second.lData, 'f', 4)).
				arg(it->second.rData == INFINITE ? "+∞" : QString::number(it->second.rData, 'f', 4)).
				arg(it->second.rCond ? "]" : ")");
			if (it != --mapInvalidRanges.end())
			{
				strDlg += "、";
			}
		}
		strDlg += tr("不合法，请设置正确的结果范围！");
		TipDlg(tr("区间设置错误"), strDlg).exec();
		return;
	}

	// 检查区间是否有交集
	std::map<int, ::tf::RangQueryCond> mapIntersectingRanges = FindIntersectingRanges(vecRangQueryCond);
	if (!mapIntersectingRanges.empty())
	{
		ULOG(LOG_INFO, "Find intersecting ranges!");
		QString strDlg;
		for (auto it = mapIntersectingRanges.begin(); it != mapIntersectingRanges.end(); ++it)
		{
			strDlg += tr("范围") + QString::number(it->first + 1);
			strDlg += it->second.lCond == ::tf::LowerSymbol::type::EQUAL_TO ? QString("[%1]").arg(it->second.lData == INFINITE ? "∞" : QString::number(it->second.lData, 'f', 4)) : QString("%1%2,%3%4").
				arg(it->second.lCond ? "[" : "(").
				arg(it->second.lData == INFINITE ? "-∞" : QString::number(it->second.lData, 'f', 4)).
				arg(it->second.rData == INFINITE ? "+∞" : QString::number(it->second.rData, 'f', 4)).
				arg(it->second.rCond ? "]" : ")");
			if (it != --mapIntersectingRanges.end())
			{
				strDlg += "、";
			}
		}
		strDlg += tr("有交集！是否保存？");

		std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("区间设置有交集"), strDlg, TipDlgType::TWO_BUTTON));
		if (pTipDlg->exec() == QDialog::Rejected)
		{
			ULOG(LOG_INFO, "Cancel set range!");
			return;
		}
	}

	// 如果区间合法且不存在交集，保存当前区间列表
	m_vecRangQueryCond = std::move(vecRangQueryCond);

	// 退出结果分段设置
	close();
}

