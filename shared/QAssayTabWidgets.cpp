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
/// @file     QAssayTabWidgets.h
/// @brief    项目选择控件
///
/// @author   5774/WuHongTao
/// @date     2022年5月7日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月7日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QAssayTabWidgets.h"
#include "ui_QAssayTabWidgets.h"
#include "ui_statusShow.h"
#include "shared/QPageShellClass.h"
#include <QGridLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include "QAssaySelectButton.h"
#include "shared/CommonInformationManager.h"
#include "shared/uidef.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/StringUtil.h"


QAssayTabWidgets::QAssayTabWidgets(QWidget *parent)
	: QTabWidget(parent)
{
    ui = new Ui::QAssayTabWidgets();
	ui->setupUi(this);
    m_assayBtnContainer = nullptr;
	REGISTER_HANDLER(MSG_ID_WORK_ASSAY_STATUS_UPDATE, this, OnUpdateAssayStatus);
}

QAssayTabWidgets::~QAssayTabWidgets()
{
}

///
/// @brief 设置控件属性
///
/// @param[in]  type	 控件类型
/// @param[in]  firstRow 单项目的行数 
/// @param[in]  row      总行数
/// @param[in]  column   列数
/// @param[in]  pages    页数
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月7日，新建函数
///
void QAssayTabWidgets::SetAssayTabAttribute(AssayType type, int firstRow,  int row, int column, int pages)
{
	// 参数判断
	if (firstRow > row || column <= 0 || pages <= 0)
	{
		return;
	}

	m_type = type;
	m_pages = pages;

    LoadAssayButtons(firstRow, row, column, pages);

	// 如果小于1页则隐藏标签
	if (pages <= 1)
	{
		this->tabBar()->hide();
	}

}

///
/// @brief 更新项目的控件列表
///
/// @param[in]  assayDatas  控件列表数据
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月7日，新建函数
///
void QAssayTabWidgets::UpdateTabAssayStatus(std::vector<std::map<int, AssayButtonData>>& assayDatas)
{
	// 首先清除状态信息
	ClearStatus();
	// 依次遍历每页数据数据赋值
	int pageCount = 0;
	for (auto& assayPage : assayDatas)
	{
		// 判断数据是否超界
		if (pageCount >= m_pages)
		{
			break;
		}

		// 依次更新每页数据
		for (auto& assays : assayPage)
		{
			// 获取显示的控件位置
			int index = assays.first;
			if (index >= m_vAssayBtn.size())
			{
				continue;
			}

			// 设置控件属性
			m_vAssayBtn[index]->SetButtonAttribute(assays.second);
			auto assayCode = assays.second.assayCode;
			if (assayCode >= 0)
			{
				m_assayIndexMap[assays.second.assayCode] = index;
			}
		}

		pageCount++;
	}
}

// 更新项目名
void QAssayTabWidgets::UpdateTabAssayStatusWithNoClear(std::vector<std::map<int, AssayButtonData>>& assayDatas)
{
    // 获取所有项目名
    std::map<int, QString> assayInfo;
    for (const auto& ad : assayDatas)
    {
        for (const auto& md : ad)
        {
            assayInfo.insert(std::pair<int, QString>(md.second.assayCode, md.second.strAssayName));
        }
    }

    // 执行按钮内容更新
    for (QAssaySelectButton* curBtn : m_vAssayBtn)
    {
        const AssayButtonData& curDt = curBtn->GetButtonAttribute();
        std::map<int, QString>::iterator it = assayInfo.find(curDt.assayCode);
        if (it == assayInfo.end())
        {
            // 项目不存在了，进行清除
            std::map<int, int>::iterator idxIter = m_assayIndexMap.find(curDt.assayCode);
            if (idxIter != m_assayIndexMap.end())
            {
                m_assayIndexMap.erase(idxIter);
            }
            curBtn->ClearContent();
        }
        else if (curDt.strAssayName != it->second)
        {
            curBtn->ResetText(it->second);
        }
    }
}

///
/// @brief 设置是否显示状态栏
///
/// @param[in]  flag  true显示
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月9日，新建函数
///
void QAssayTabWidgets::SetShowStatuFlag(bool flag)
{
	// 开启
	if (flag)
	{
		QWidget* qwid = new QWidget(this);
        m_statusLabel = new Ui::statusLabel();
		m_statusLabel->setupUi(qwid);
		setCornerWidget(qwid, Qt::BottomLeftCorner);
	}
}

///
/// @brief 设置
///
/// @param[in]  pageInfos  按钮信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月9日，新建函数
///
void QAssayTabWidgets::SetDeviceSelect(const std::vector<CornerButtonInfo>& pageInfos)
{
	// 设置水平布局器
	QWidget* cornerItem = new QWidget(this);
	QHBoxLayout* hlayout = new QHBoxLayout(cornerItem);
	cornerItem->setLayout(hlayout);

	// 按照规则依次生成按钮
	for (const auto& pageinfo : pageInfos)
	{
		QPushButton* functionButton = new QPushButton(pageinfo.m_buttonText, cornerItem);
		functionButton->setObjectName(pageinfo.m_objectName);
		functionButton->setProperty("number", pageinfo.m_propertyNumber);

		// 根据按键类型--生化类型
		if (pageinfo.m_propertyNumber == 0)
		{
			connect(functionButton, SIGNAL(clicked()), this, SLOT(OnSelectChemistryButton()));
			emit functionButton->clicked();
		}
		// 其他类型
		else
		{
			connect(functionButton, SIGNAL(clicked()), this, SLOT(OnSelectImmunityButton()));
		}

		// 添加到布局
		hlayout->addWidget(functionButton);
	}

	// 设置控件
	setCornerWidget(cornerItem);
}

///
/// @brief 设置项目选择属性
///
/// @param[in]  indexPos   项目位置
/// @param[in]  attribute  属性  
///
/// @return true：表示设置成功，反之失败
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月11日，新建函数
///
bool QAssayTabWidgets::SetAssayButtonAttribute(int indexPos, AssayButtonData& attribute)
{
	if (indexPos < 0 || indexPos >= m_vAssayBtn.size())
	{
		return false;
	}

	m_vAssayBtn[indexPos]->SetButtonAttribute(attribute);
	m_assayIndexMap[attribute.assayCode] = indexPos;
	return true;
}

///
/// @brief 通过位置获取对应的项目属性信息
///
/// @param[in]  indexPos  位置信息
/// @param[out] attribute 属性信息 
///
/// @return true：表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月11日，新建函数
///
bool QAssayTabWidgets::GetAttributeByIndex(int indexPos, AssayButtonData& attribute)
{
	if (indexPos < 0 || indexPos >= m_vAssayBtn.size())
	{
		return false;
	}

	// 获取对应的属性
	attribute = m_vAssayBtn[indexPos]->GetButtonAttribute();
	return true;
}

///
/// @brief 根据属性code获取项目信息
///
/// @param[in]  assayCode  属性code
/// @param[in]  attribute  属性
///
/// @return true：表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月11日，新建函数
///
bool QAssayTabWidgets::GetAttributeByAssayCode(int assayCode, AssayButtonData& attribute, int& indexPostion)
{
	// 遍历所有属性按钮
	for (auto assayBtn : m_vAssayBtn)
	{
		attribute = assayBtn->GetButtonAttribute();
		if (attribute.assayCode == assayCode)
		{
			// 获取位置信息
			indexPostion = assayBtn->GetPrivatePostion();
			return true;
		}
	}

	return false;
}

bool QAssayTabWidgets::GetAttributeByName(const QString& assayName, AssayButtonData& attribute, int& indexPostion)
{
    // 遍历所有属性按钮
    for (auto assayBtn : m_vAssayBtn)
    {
        attribute = assayBtn->GetButtonAttribute();
        if (attribute.strAssayName == assayName)
        {
            // 获取位置信息
            indexPostion = assayBtn->GetPrivatePostion();
            return true;
        }
    }

    return false;
}

///
/// @brief 获取项目按钮最大个数
///
///
/// @return 项目最大数目
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月12日，新建函数
///
int QAssayTabWidgets::GetAssayButtonCount()
{
	return m_vAssayBtn.size();
}

///
/// @brief 完全清除状态（因为位置改变）
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月7日，新建函数
///
void QAssayTabWidgets::ClearComplete()
{
	AssayButtonData tmpAttribute;
	for (auto btn : m_vAssayBtn)
	{
		btn->SetButtonAttribute(tmpAttribute);
	}

	m_assayIndexMap.clear();
}

void QAssayTabWidgets::ClearAllButtons(std::vector<int>& effectedAssayCodes)
{
    effectedAssayCodes.clear();

    for (auto btn : m_vAssayBtn)
    {
        int assayCode = btn->ClearContent();

        // 保存受影响的项目编号
        if (assayCode != -1)
        {
            effectedAssayCodes.push_back(assayCode);
        }
    }

	m_assayIndexMap.clear();
}

///
/// @bref
///		更新按钮位置
///
/// @param[in] singleRowAmount 当项目的行数
///
/// @par History:
/// @li 8276/huchunli, 2023年6月13日，新建函数
///
void QAssayTabWidgets::RefreshAllButtons(int singleRowAmount)
{
    ULOG(LOG_INFO, "%s(%d)", __FUNCTION__, singleRowAmount);

    if (m_assayBtnContainer == nullptr)
    {
        ULOG(LOG_ERROR, "Null assayBtnContainer.");
        return;
    }
    m_assayBtnContainer->ResetSingleRowAmount(singleRowAmount);
    m_assayBtnContainer->GetAllButtonsVec(m_vAssayBtn);
    connect(m_assayBtnContainer.get(), SIGNAL(selectAssay(int)), this, SLOT(OnSelected(int)));
}

std::vector<std::pair<int, int>> QAssayTabWidgets::GetChangedAssaysList()
{
    if (m_assayBtnContainer == nullptr)
    {
        return{};
    }

    return m_assayBtnContainer->GetChangedAssays();
}

///
/// @brief 清除当前项目状态
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月11日，新建函数
///
void QAssayTabWidgets::ClearStatus()
{
	for (auto btn : m_vAssayBtn)
	{
		btn->ClearStatus();
	}

	m_assayIndexMap.clear();
}

///
/// @bref
///		清除选中状态
///
/// @par History:
/// @li 8276/huchunli, 2023年10月13日，新建函数
///
void QAssayTabWidgets::ClearAllSeletedStatu()
{
    for (QAssaySelectButton* btn : m_vAssayBtn)
    {
        btn->ClearSelected();
    }
}

///
/// @brief 初始化项目选择控件
///
/// @param[in]  assayRow  单独项目的行数
/// @param[in]  allRows   总项目行数（单独项目行数+组合项目行数）
/// @param[in]  columns   控件列数
/// @param[in]  pages     项目页数
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月7日，新建函数
///
void QAssayTabWidgets::LoadAssayButtons(int assayRow, int allRows, int columns, int pages)
{
    // 初始化按钮容器
    m_assayBtnContainer = std::shared_ptr<PageBtnContainer>(new PageBtnContainer(pages, allRows, assayRow, columns));
    m_assayBtnContainer->SetDlgType(m_type);

    // 根据每页的项目内容来初始化页面 
	int page = 0;
    QString objNamePrefix("pWgt");
	while (page < pages)
	{
		QWidget* pWgt = new QWidget(this);
		pWgt->setObjectName(objNamePrefix + QString::number(page));
        m_assayBtnContainer->SetParentWidget(pWgt);

		QVBoxLayout* pOuterLayout = new QVBoxLayout(pWgt);
		pOuterLayout->setObjectName(QStringLiteral("pOuterLayout"));

		// 单项目的框架
		QWidget* pWidgetSingle = new QWidget(pWgt);
		pWidgetSingle->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		pWidgetSingle->setObjectName(QStringLiteral("pWidgetSingle"));
		QGridLayout* pLayoutSingle = new QGridLayout(pWidgetSingle);
        pLayoutSingle->setSpacing(21);
		pOuterLayout->addWidget(pWidgetSingle);

		// 组合项目的标题
        QLabel* pUnonLable = new QLabel(tr("组合项目"), pWgt);
		pUnonLable->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        pUnonLable->setObjectName("pCombineAssayLable");
        pOuterLayout->addWidget(pUnonLable);
		pUnonLable->setStyleSheet("padding-top: 13px; padding-left: 8px; font-size: 16px; font-weight: bold;");

        // 组合项目的框架
		QWidget* pWidgetCom = new QWidget(pWgt);
		pWidgetCom->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		pWidgetCom->setObjectName(QStringLiteral("pWidgetCom"));
		QGridLayout* pLayoutCom = new QGridLayout(pWidgetCom);
        pLayoutCom->setSpacing(21);
		pOuterLayout->addWidget(pWidgetCom);

		// 设置伸缩因子
		pOuterLayout->setStretchFactor(pWidgetSingle, 1);
		pOuterLayout->setStretchFactor(pUnonLable, 0); // 不进行拉伸
		pOuterLayout->setStretchFactor(pWidgetCom, 1);

		for (int iRow = 0; iRow < allRows; iRow++)
		{
            // 单项目/组合项目
            QGridLayout* pLayout = iRow < assayRow ? pLayoutSingle : pLayoutCom;
            bool isProle = (pLayout == pLayoutCom);
			for (int iCol = 0; iCol < columns; iCol++)
			{
				QAssaySelectButton* pBtn = new QAssaySelectButton(pWgt, m_vAssayBtn.size());
				auto attribute = pBtn->GetButtonAttribute();
				attribute.assayType = m_type;
				pBtn->SetButtonAttribute(attribute);

				// 类型3主要用于项目位置的设置
				if (m_type == AssayType::ASSAYTYPE3_POSITION_SET)
				{
                    // 需要设置组合项目的范围
                    pBtn->SetButtonAttribute(true, isProle);
				}

				pLayout->addWidget(pBtn, iRow, iCol);
				m_vAssayBtn.push_back(pBtn);
                m_assayBtnContainer->AddButton(page, isProle, pLayout, pBtn, pUnonLable);

				// 连接信号槽
				connect(pBtn, SIGNAL(select(int)), this, SIGNAL(selectAssay(int)));
			}
		}

        // 对齐（避免全是单项目或全是组合项目时的临一中无页情况）
        m_assayBtnContainer->ProEmptyPageSingle(page, pLayoutSingle, nullptr);
        m_assayBtnContainer->ProEmptyPageUnion(page, pLayoutCom, pUnonLable);

		insertTab(page, pWgt, QString::number(page + 1));
		page++;
	}
}

void QAssayTabWidgets::OnSelected(int indexPostion)
{
	emit selectAssay(indexPostion);
}

///
/// @brief 更新项目状态的槽函数
///
/// @param[in]  assayCodes  项目编号之间用“,”来分割
///
/// @par History:
/// @li 5774/WuHongTao，2023年8月19日，新建函数
///
void QAssayTabWidgets::OnUpdateAssayStatus(QString assayCodes)
{
    ULOG(LOG_INFO, "%s, Param %s.", __FUNCTION__, assayCodes.toStdString().c_str());

	// 只有工作页面才做更新
	if (m_type != ASSAYTYPE1_WORKPAGE)
	{
        ULOG(LOG_INFO, "Short ret by type diff.");
		return;
	}
	if (assayCodes.isEmpty())
    {
        ULOG(LOG_WARN, "Short ret by empty param.");
		return;
	}

	auto assayCodeVec = assayCodes.split(",");
	for (const QString& assayCode : assayCodeVec)
	{
		// 是否纯数字，项目编号必须是纯数字
		if (!stringutil::IsPureDigit(assayCode.toStdString()))
		{
			ULOG(LOG_ERROR, "AssayCode string is not digit: %s)", assayCode.toStdString().c_str());
			continue;
		}

		// 找到项目对应的按钮
		auto iterAssay = m_assayIndexMap.find(assayCode.toInt());
		if (iterAssay == m_assayIndexMap.end())
		{
			ULOG(LOG_WARN, "Can not find the assaycode: %s.", assayCode.toStdString().c_str());
			continue;
		}

		// 获取按钮
		auto index = iterAssay->second;
		if (index < 0 || index >= m_vAssayBtn.size())
		{
			ULOG(LOG_WARN, "Out of range: %s.", assayCode.toStdString().c_str());
			continue;
		}
		
		// 获取当前的项目的属性
		auto attribute = m_vAssayBtn[index]->GetButtonAttribute();
		// 获取实时属性
		auto assayStatus = QSystemMonitor::GetInstance().GetAssayStatus(assayCode.toInt());
		if (assayStatus)
		{
			auto status = assayStatus.value();
			// 项目遮蔽
			attribute.bIsMask = status.maskAssay;
			// 试剂异常
			attribute.bIsRegentAbn = status.abnormalReagent;
			// 校准异常
			attribute.bIsCaliAbn = status.abnormalCalibrate;
			// 设置属性
			m_vAssayBtn[index]->SetButtonAttribute(attribute);
		}
	}
}

///
/// @bref 构造BtnContainer
///
PageBtnContainer::BtnContainer::BtnContainer(int idxPage, QGridLayout* pLay, QLabel* lable)
{
    m_pageIndex = idxPage;
    m_layout = pLay;
    m_titleLable = lable;
}

///
/// @bref
///		构造PageBtnContainer
///
/// @par History:
/// @li 8276/huchunli, 2023年6月13日，新建函数
///
PageBtnContainer::PageBtnContainer(int totalPage, int totalRowCnt, int pageSingleRowCnt, int col)
{
    m_totalPage = totalPage;
    m_pgTotalRowAmount = totalRowCnt;
    m_pgSingleRowAmount = pageSingleRowCnt;
    m_columAmount = col;
}

///
/// @bref
///		添加一个按钮到缓存容器
///
/// @param[in] pageIdx 页面索引
/// @param[in] isUnion 是否是组合项目
/// @param[in] pLayout 按钮的Layout
/// @param[in] pBtn 按钮
///
/// @par History:
/// @li 8276/huchunli, 2023年6月13日，新建函数
///
void PageBtnContainer::AddButton(int pageIdx, bool isUnion, QGridLayout* pLayout, QAssaySelectButton* pBtn, QLabel* titleLable)
{
    std::unordered_map<int, std::shared_ptr<BtnContainer>>& pBtns = isUnion ? m_pageBtnsUnion : m_pageBtnsSingle;
    std::unordered_map<int, std::shared_ptr<BtnContainer>>::iterator it = pBtns.find(pageIdx);
    if (it != pBtns.end())
    {
        it->second->m_pageIndex = pageIdx;
        it->second->m_buttons.push_back(pBtn);
    }
    else
    {
        std::shared_ptr<BtnContainer> newBtn(new BtnContainer(pageIdx, pLayout, titleLable));
        newBtn->m_buttons.push_back(pBtn);
        pBtns.insert(std::pair<int, std::shared_ptr<BtnContainer>>(pageIdx, newBtn));
    }

    // 缓存按钮
    if (isUnion)
    {
        m_btnsUnion.push_back(pBtn);
    }
    else
    {
        m_btnsSingle.push_back(pBtn);
    }
    m_allBtns.push_back(pBtn);
}

///
/// @bref
///		设置页数对齐，避免空联合项或单项找不到页的情况(在每页按钮加完后调用)
///
/// @param[in] pageIndex 页码
///
/// @par History:
/// @li 8276/huchunli, 2023年6月15日，新建函数
///
void PageBtnContainer::ProEmptyPageSingle(int pageIndex, QGridLayout* pLayout, QLabel* tLable)
{
    if (m_pageBtnsSingle.find(pageIndex) == m_pageBtnsSingle.end())
    {
        std::shared_ptr<BtnContainer> newBtn(new BtnContainer(pageIndex, pLayout, tLable));
        m_pageBtnsSingle.insert(std::pair<int, std::shared_ptr<BtnContainer>>(pageIndex, newBtn));
    }
}

void PageBtnContainer::ProEmptyPageUnion(int pageIndex, QGridLayout* pLayout, QLabel* tLable)
{
    if (m_pageBtnsUnion.find(pageIndex) == m_pageBtnsUnion.end())
    {
        std::shared_ptr<BtnContainer> newBtn(new BtnContainer(pageIndex, pLayout, tLable));
        m_pageBtnsUnion.insert(std::pair<int, std::shared_ptr<BtnContainer>>(pageIndex, newBtn));
    }
}

///
/// @bref
///		调整单项目与组合项目的行数占比
///
/// @param[in] singleRowAmount 单项目在一页中的行数
///
/// @par History:
/// @li 8276/huchunli, 2023年6月13日，新建函数
///
void PageBtnContainer::ResetSingleRowAmount(int singleRowAmount)
{
    if (m_pgSingleRowAmount == singleRowAmount)
    {
        return;
    }

    // 把按钮全hide
    for (int i=0; i<m_allBtns.size(); ++i)
    {
        m_allBtns[i]->hide();
    }

    m_pgSingleRowAmount = singleRowAmount;
    int unionRowAmount = m_pgTotalRowAmount - m_pgSingleRowAmount;
    int curTotalIndex = 0;
    int curIdxUnion = 0;
    int curIdxSingle = 0;

    std::vector<QAssaySelectButton*> curAllBtns;
    m_changedAssayCodes.clear();

    // 重新调整按钮位置
    std::unordered_map<int, std::shared_ptr<BtnContainer>>::iterator sit = m_pageBtnsSingle.begin();
    std::unordered_map<int, std::shared_ptr<BtnContainer>>::iterator uit = m_pageBtnsUnion.begin();
    for (; sit != m_pageBtnsSingle.end() && uit != m_pageBtnsUnion.end(); ++sit, ++uit)
    {
        // 单项目的调整
        RePositionButtons(sit, curTotalIndex, curIdxSingle, false, curAllBtns);
        // 组合项目的调整
        RePositionButtons(uit, curTotalIndex, curIdxUnion, true, curAllBtns);

        // 如果没有组合按钮框，则隐藏组合按钮标题
        if (unionRowAmount == 0 && uit->second->m_titleLable)
        {
            uit->second->m_titleLable->hide();
        }
        else if (uit->second->m_titleLable)
        {
            uit->second->m_titleLable->show();
        }
    }

    // 生成存在变化的按钮
    std::set<int> uniqAssayCode;
    for (int i=0; i<curAllBtns.size(); ++i)
    {
        int currentAssayCode = curAllBtns[i]->GetButtonAssayCode();
        if (currentAssayCode != -1)
        {
            m_changedAssayCodes.push_back(std::pair<int, int>(currentAssayCode, i));
            uniqAssayCode.insert(currentAssayCode);
        }
    }
    for (int i=0; i<m_allBtns.size(); ++i)
    {
        int currentAssayCode = m_allBtns[i]->GetButtonAssayCode();
        if (currentAssayCode != -1 && uniqAssayCode.find(currentAssayCode) == uniqAssayCode.end())
        {
            m_changedAssayCodes.push_back(std::pair<int, int>(currentAssayCode, -1));
        }
    }

    m_allBtns = curAllBtns;
}

///
/// @bref
///		获取选中按钮的索引(如果没有找到目标按钮，则返回-1)
///
/// @par History:
/// @li 8276/huchunli, 2023年11月21日，新建函数
///
int PageBtnContainer::GetSelecteButtonIndex()
{
    for (QAssaySelectButton* btn : m_allBtns)
    {
        if (btn != nullptr && btn->IsSelected())
        {
            return btn->GetPrivatePostion();
        }
    }

    return -1;
}

///
/// @bref
///		重新排列按钮的位置
///
/// @param[in] it 当页按钮的容器迭代器
/// @param[out] totalIdx 按钮总体的序号
/// @param[out] tpIdx 按钮单项目或组合项目的序号
/// @param[in] isUnion 是否是组合项目
/// @param[in] curAllBtns 重新排列的按钮容器
///
/// @par History:
/// @li 8276/huchunli, 2023年6月16日，新建函数
///
void PageBtnContainer::RePositionButtons(std::unordered_map<int, std::shared_ptr<BtnContainer>>::iterator& it, \
    int& totalIdx, int& tpIdx, bool isUnion, std::vector<QAssaySelectButton*>& curAllBtns)
{
    // 移除布局中的控件
    for (int i = 0; i < it->second->m_buttons.size(); ++i)
    {
        it->second->m_layout->removeWidget(it->second->m_buttons[i]);
    }
    it->second->m_buttons.clear();
    int rowAmount = isUnion ? (m_pgTotalRowAmount - m_pgSingleRowAmount) : m_pgSingleRowAmount;
    std::vector<QAssaySelectButton*>& vecBtn = isUnion ? m_btnsUnion : m_btnsSingle;
    std::shared_ptr<CommonInformationManager> assayMgr = CommonInformationManager::GetInstance();

    // 重新往布局中填入控件
    for (int r = 0; r < rowAmount; ++r)
    {
        for (int c = 0; c < m_columAmount; ++c)
        {
            // 重置当前类型的对应位置的按钮
            QAssaySelectButton* pNewBtn = nullptr;
            if (tpIdx < vecBtn.size())
            {
                pNewBtn = vecBtn[tpIdx];
                pNewBtn->SetPrivatePosIndex(totalIdx);
            }
            else
            {
                pNewBtn = new QAssaySelectButton(m_parent, totalIdx);
                if (m_dlgType == AssayType::ASSAYTYPE3_POSITION_SET)
                {
                    pNewBtn->SetButtonAttribute(true, isUnion);
                }
                connect(pNewBtn, SIGNAL(select(int)), this, SIGNAL(selectAssay(int)));
                vecBtn.push_back(pNewBtn);
            }
            pNewBtn->show();
            it->second->m_layout->addWidget(pNewBtn, r, c);

            curAllBtns.push_back(pNewBtn);
            it->second->m_buttons.push_back(pNewBtn);
            tpIdx++;
            totalIdx++;
        }
    }
}
