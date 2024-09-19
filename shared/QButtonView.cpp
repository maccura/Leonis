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
/// @file     QButtonView.cpp
/// @brief    项目选择容器的基础控件（项目的view）
///
/// @author   5774/WuHongTao
/// @date     2023年12月11日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2023年12月11日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QButtonView.h"
#include "ui_QButtonViewPage.h"
#include "ui_QAssayCardPage.h"
#include <QRadioButton>
#include <QGroupBox>
#include <QGridLayout>
#include "QSampleAssayModel.h"
#include "shared/ReagentCommon.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/tipdlg.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/StringUtil.h"

#define COLUMNCOUNT 7
#define	MAXROWCOUNT 7
#define MAXROW		4
#define MINROW		1

#define ReportErr(bErr, msg) \
if ((bErr))\
{\
	TipDlg(msg).exec();\
}

QButtonModel::QButtonModel(int elementCount)
{
	m_data.assign(elementCount, AssayButtonData());
}

///
/// @brief 根据位置信息，设置数据
///
/// @param[in]  index  位置索引（row*cloumn + currentIndex）
/// @param[in]  data   设置信息
///
/// @return true设置成功
///
/// @par History:
/// @li 5774/WuHongTao，2023年12月11日，新建函数
///
bool QButtonModel::setData(int index, AssayButtonData& data)
{
	int total = m_data.size();
	if (index >= total)
	{
		ULOG(LOG_INFO, "%s( the postion is %d)", __FUNCTION__, index);
		return false;
	}

	// 更新数据
	m_data[index] = data;
	// 通知view更新状态
	emit dataChanged(index);
	return true;
}

///
/// @brief 清除界面所有的值
///
///
/// @par History:
/// @li 5774/WuHongTao，2023年12月11日，新建函数
///
void QButtonModel::clearData()
{
	int index = 0;
	AssayButtonData initData;
	for (auto& data : m_data)
	{
		setData(index, initData);
		index++;
	}
}

///
/// @brief 根据编号获取按钮的信息
///
/// @param[in]  index  编号
///
/// @return 按钮信息
///
/// @par History:
/// @li 5774/WuHongTao，2023年12月11日，新建函数
///
boost::optional<AssayButtonData> QButtonModel::GetDataByIndex(const int index) const
{
	int total = m_data.size();
	if (index >= total)
	{
		ULOG(LOG_INFO, "%s( the postion is %d)", __FUNCTION__, index);
		return boost::none;
	}

	return m_data[index];
}

QButtonBasicView::QButtonBasicView(QWidget* parent, const int row /*= 5*/, const int column /*= 7*/)
	: QWidget(parent)
{
	m_model = Q_NULLPTR;
	m_row = row;
	m_column = column;

	// 初始化有问题，行和列不可能是小于等于0
	if (m_row <= 0 || m_column <= 0)
	{
		ULOG(LOG_WARN, "The view can not be inital the row: %d - column: %d", m_row, m_column);
		return;
	}

	m_layout = new QGridLayout(this);
	if (m_layout == Q_NULLPTR)
	{
		ULOG(LOG_WARN, "the layout can not be Inital");
		return;
	}

	// 设置间隔属性
	m_layout->setSpacing(18);
	// 生成模板对象
	GenTemplateOfView(m_row, m_column);
}

///
/// @brief 设置数据模型
///
/// @param[in]  model  数据模型
///
/// @return true:设置成功，false:设置失败
///
/// @par History:
/// @li 5774/WuHongTao，2023年12月11日，新建函数
///
bool QButtonBasicView::setModel(QButtonModel* model)
{
	m_model = model;
	if (m_model == Q_NULLPTR)
	{
		return false;
	}

	// 刷新数据的消息
	connect(m_model, SIGNAL(dataChanged(int)), this, SLOT(OnUpdateButtonStatus(int)));
	int total = m_buttonElements.size();
	// 刷新所有数据
	const auto allData = m_model->GetAllData();
	int index = 0;
	for (auto data : allData)
	{
		// 数据比按钮数目多
		if (index >= total)
		{
			break;
		}

		// 更新按钮状态
		m_buttonElements[index]->SetButtonAttribute(data);
		index++;
	}

	return true;
}

///
/// @brief 调整view的结构（行数和列数）
///
/// @param[in]  row		行数
/// @param[in]  column  列数
///
/// @return true:调整成功
///
/// @par History:
/// @li 5774/WuHongTao，2023年12月11日，新建函数
///
bool QButtonBasicView::AdjustView(const int row, const int column)
{
	if (row <= 0 || column <= 0)
	{
		ULOG(LOG_WARN, "The view can not be adjust the row: %d - column: %d", row, column);
		return false;
	}

	if (Q_NULLPTR == m_layout)
	{
		return false;
	}

	m_row = row;
	m_column = column;
	// 依次清空每一个按钮信息
	for (auto& assayButton : m_buttonElements)
	{
		// 从布局之中清除
		disconnect(assayButton, SIGNAL(select(int, int)), this, SIGNAL(dataSelected(int, int)));
		m_layout->removeWidget(assayButton);
		// 清空属性
		assayButton->SetButtonAttribute();
	}

	int total = m_buttonElements.size();
	int need = row * column;

	if (total > need)
	{
		auto start = m_buttonElements.begin() + need;
		std::vector<QAssaySelectButton*> leftButton;
		// 获取需要删除的按钮列表
		leftButton.insert(leftButton.end(), start, m_buttonElements.end());
		// 删除vector中的指针索引
		m_buttonElements.erase(start, m_buttonElements.end());
		// 依次删除button
		for (auto& button : leftButton)
		{
			button->deleteLater();
		}
	}
	else if(total < need)
	{
		int index = total;
		while (index < need)
		{
			auto element = new QAssaySelectButton(this, -1);
			m_buttonElements.push_back(element);
			index++;
		}
	}

	// 依次删除按钮属性和值，重新布局
	int realNumber = m_buttonElements.size();
	int i = 0;
	while (i < row)
	{
		int j = 0;
		while (j < column)
		{
			// 根据需要找到对应的button
			auto index = i * column + j;
			if (index >= realNumber)
			{
				break;
			}

			auto element = m_buttonElements[index];
			connect(element, SIGNAL(select(int, int)), this, SIGNAL(dataSelected(int, int)));
			// 设置位置信息
			element->SetPrivatePosIndex(index);
			m_layout->addWidget(element, i, j);
			j++;
		}

		i++;
	}

	return true;
}

///
/// @brief 模型更新界面的状态
///
/// @param[in]  index  未知
///
/// @par History:
/// @li 5774/WuHongTao，2023年12月11日，新建函数
///
void QButtonBasicView::OnUpdateButtonStatus(int index)
{
	int total = m_buttonElements.size();
	if (index >= total)
	{
		return;
	}

	if (Q_NULLPTR == m_model)
	{
		return;
	}

	auto data = m_model->GetDataByIndex(index);
	if (!data)
	{
		return;
	}

	m_buttonElements[index]->SetButtonAttribute(data.value());
}

///
/// @brief 根据参数生成按钮的view
///
/// @param[in]  row		行
/// @param[in]  column  列
///
/// @return true:生成成功，反之：失败
///
/// @par History:
/// @li 5774/WuHongTao，2023年12月11日，新建函数
///
bool QButtonBasicView::GenTemplateOfView(const int row /*= 5*/, const int column /*= 7*/)
{
	if (row <= 0 || column <= 0)
	{
		ULOG(LOG_WARN, "%s The view can not be inital the row: %d - column: %d", __FUNCTION__, row, column);
		return false;
	}

	int count = 0;
	int i = 0;
	while (i < row)
	{
		int j = 0;
		while (j < column)
		{
			auto element = new QAssaySelectButton(this, count++);
			connect(element, SIGNAL(select(int, int)), this, SIGNAL(dataSelected(int, int)));
			m_buttonElements.push_back(element);
			m_layout->addWidget(element, i, j);
			j++;
		}

		i++;
	}

	return true;
}

QButtonViewPage::QButtonViewPage(QWidget* parent)
	: QWidget(parent)
{
	ui = new Ui::QButtonViewPage();
	ui->setupUi(this);

	// 项目(单)按钮被选中的信号
	connect(ui->singleAssay, &QButtonBasicView::dataSelected, this, [&](int postion, int assayCode)
	{
		emit dataSelected(false, postion, assayCode);
	});

	// 项目(组合)按钮被选中的信号
	connect(ui->combinAssay, &QButtonBasicView::dataSelected, this, [&](int postion, int assayCode)
	{
		emit dataSelected(true, postion, assayCode);
	});
}

///
/// @brief 设置页面的布局模板
///
/// @param[in]  totalRow		总共的行数
/// @param[in]  singelRow		单项目行数
/// @param[in]  columnCount		列数
///
/// @par History:
/// @li 5774/WuHongTao，2023年12月11日，新建函数
///
void QButtonViewPage::setViewMapTmp(int totalRow, int singelRow, int columnCount)
{
	// 不能为0
	if (totalRow <= 0 || singelRow <= 0 || columnCount <= 0)
	{
		return;
	}

	// 组合项目可以为0，不显示，但是不能是负值
	int combinRow = totalRow - singelRow;
	if (combinRow < 0)
	{
		return;
	}

	// 获取之前的view的size（row和column
	bool update = false;
	auto size = ui->singleAssay->GetSizeOfView();
	auto combinsize = ui->combinAssay->GetSizeOfView();
	if (size.first != singelRow || size.second != columnCount
		|| combinsize.first != combinRow || combinsize.second != columnCount)
	{
		update = true;
	}

	// 只有行列有所变化才调整大小
	if (update)
	{
		// 重新设定单项目的行列数
		ui->singleAssay->AdjustView(singelRow, columnCount);
		// 重新设定组合项目的行列数
		ui->combinAssay->AdjustView(combinRow, columnCount);
	}

	// 单项目的模式
	QButtonModel* singleModel = new QButtonModel(singelRow * columnCount);
	ui->singleAssay->setModel(singleModel);

	// 组合项目的模式
	QButtonModel* combinModel = new QButtonModel(combinRow * columnCount);
	ui->combinAssay->setModel(combinModel);
}

///
/// @brief 更新单项目和组合项目的数据状态
/// 
/// @param[in]  singleMap   单项目状态map
/// @param[in]  combinMap   组合项目状态map
///
/// @par History:
/// @li 5774/WuHongTao，2023年12月11日，新建函数
///
void QButtonViewPage::updateViewMap(std::map<int, AssayButtonData>& singleMap, std::map<int, AssayButtonData>& combinMap)
{
	auto singleMode = ui->singleAssay->GetModel();
	if (Q_NULLPTR == singleMode)
	{
		return;
	}
	upDateMap(singleMap, singleMode);

	auto combinMode = ui->combinAssay->GetModel();
	if (Q_NULLPTR == combinMode)
	{
		return;
	}
	upDateMap(combinMap, combinMode);
}

///
/// @brief 更新对应的模型的数据状态
///
/// @param[in]  singleMap  数据
/// @param[in]  model      模型
///
/// @return true:更新成功
///
/// @par History:
/// @li 5774/WuHongTao，2023年12月11日，新建函数
///
bool QButtonViewPage::upDateMap(std::map<int, AssayButtonData>& singleMap, QButtonModel* model)
{
	int total = model->GetTotal();
	// 首先清空所有项目
	model->clearData();
	// 更新单项目
	for (auto& iter : singleMap)
	{
		if (iter.first >= total)
		{
			continue;
		}

		// 更新项目状态
		model->setData(iter.first, iter.second);
	}

	return true;
}

QAssayCardPage::QAssayCardPage(QWidget* parent)
	: QWidget(parent)
{
	ui = new Ui::QAssayCardPage();
	ui->setupUi(this);
	Init();
}

void QAssayCardPage::Show()
{
	OnShowPage(m_currentPageIndex);
}

void QAssayCardPage::Redo()
{
	// 手动设置项目
	OnSelectAssay(true, std::get<0>(m_redokey), std::get<1>(m_redokey), std::get<2>(m_redokey));
	m_redokey = { false, -1, -1 };
}

///
/// @brief 获取所有样本信息，包括生化和免疫
///
/// @param[in]  testItems  项目信息
///
/// @return 请求测试项目的数目-手工稀释的倍数
///
/// @par History:
/// @li 5774/WuHongTao，2023年12月12日，新建函数
///
std::pair<int, int> QAssayCardPage::GetSampleTestItemInfo(std::vector<::tf::TestItem>& testItems)
{
	std::pair<int, int> requestData = {0, 0};
	for (const auto& process : m_assaySelectHander)
	{
		if (Q_NULLPTR == process)
		{
			continue;
		}

		auto requestTimes = process->GetSampleTestItemInfo(testItems);
		// 没有项目则不计入计算范围
		if (requestTimes.first <= 0)
		{
			continue;
		}

		if (requestTimes.second < 1)
		{
			requestData = std::make_pair(requestData.first + requestTimes.first, requestData.second);
		}
		else
		{
			requestData = std::make_pair(requestData.first + requestTimes.first, requestTimes.second);
		}
	}

	return requestData;
}

void QAssayCardPage::GetProfiles(std::vector<int64_t>& profiles)
{
	for (const auto& process : m_assaySelectHander)
	{
		if (Q_NULLPTR == process)
		{
			continue;
		}

		if (process->GetAssayType() == AssayDeviceType::COMBINTYPE)
		{
			process->GetProfiles(profiles);
		}
	}
}

std::map<int, int> QAssayCardPage::GetAssayPostion()
{
	std::map<int, int> assayPostion;
	for (const auto& process : m_assaySelectHander)
	{
		if (Q_NULLPTR == process)
		{
			continue;
		}

		auto currentPostion = process->GetAssayPostion();
		for (const auto& postion : currentPostion)
		{
			assayPostion[postion.first] = postion.second;
		}
	}

	return assayPostion;
}

void QAssayCardPage::RefreshCard()
{
	for (const auto& process : m_assaySelectHander)
	{
		if (Q_NULLPTR == process)
		{
			continue;
		}

		process->RefreshCard();
	}

	m_testItems.clear();
	m_postion = boost::none;
}

bool QAssayCardPage::DeleteAssayByCode(int assayCode)
{
	for (const auto& process : m_assaySelectHander)
	{
		if (Q_NULLPTR == process)
		{
			continue;
		}

		process->DeleteAssayByCode(assayCode);
	}

	return true;
}

bool QAssayCardPage::UpdateAssayProfileStatus(const std::vector<int64_t>& profiles)
{
	for (const auto& process : m_assaySelectHander)
	{
		if (process->GetAssayType() != AssayDeviceType::COMBINTYPE)
		{
			continue;
		}

		process->RefreshCard();
	}

	for (const auto& profile : profiles)
	{
		for (const auto& process : m_assaySelectHander)
		{
			if (Q_NULLPTR == process)
			{
				continue;
			}

			if (process->GetAssayType() != AssayDeviceType::COMBINTYPE)
			{
				continue;
			}

			bool hasDone = process->UpdateAssayProfileStatus(profile);
			if (hasDone)
			{
				break;
			}
		}
	}

	return true;
}

bool QAssayCardPage::UpdateTestItemStatus(const std::vector<std::shared_ptr<tf::TestItem>>& testItems)
{
	ULOG(LOG_INFO, "%s( the testItem size is %d)", __FUNCTION__, testItems.size());
	// 首先刷新为默认状态
	m_testItems = testItems;
	for (const auto& process : m_assaySelectHander)
	{
		if (process->GetAssayType() == AssayDeviceType::COMBINTYPE)
		{
			continue;
		}

		process->RefreshCard();
	}

	for (auto spTestItem : testItems)
	{
		for (const auto& process : m_assaySelectHander)
		{
			if (Q_NULLPTR == process)
			{
				continue;
			}

			bool hasDone = process->UpdateTestItemStatus(spTestItem);
			if (hasDone)
			{
				break;
			}
		}
	}

	return true;
}

bool QAssayCardPage::UpdateTestItemTimes(int assayCode, int testTimes)
{
	for (const auto& process : m_assaySelectHander)
	{
		if (Q_NULLPTR == process)
		{
			continue;
		}

		bool hasDone = process->UpdateTestItemTimes(assayCode, testTimes);
		if (hasDone)
		{
			return true;
		}
	}

	return false;
}

///
/// @brief 显示第几页
///
/// @param[in]  index  页数（1,2,3）
///
/// @par History:
/// @li 5774/WuHongTao，2023年12月11日，新建函数
///
void QAssayCardPage::OnShowPage(int index)
{
	std::map<int, AssayButtonData> singleMap;
	std::map<int, AssayButtonData> combinMap;

	int singleCount = PageViewCount(true, m_singleCount);
	int combinCount = PageViewCount(false, m_singleCount);

	for (const auto& process : m_assaySelectHander)
	{
		if (Q_NULLPTR == process)
		{
			continue;
		}

		bool hasDone = false;
		if (process->GetAssayType() == AssayDeviceType::COMBINTYPE)
		{
			hasDone = process->GenAssayButtonDataByIndex(combinCount, combinMap, index);
			continue;
		}

		if (m_chemistry)
		{
			if (process->GetAssayType() == AssayDeviceType::CHEMISTRYTYPE)
			{
				hasDone = process->GenAssayButtonDataByIndex(singleCount, singleMap, index);
			}			
		}
		else
		{
			if (process->GetAssayType() == AssayDeviceType::IMMUNETYPE)
			{
				hasDone = process->GenAssayButtonDataByIndex(singleCount, singleMap, index);
			}
		}
	}

	if (m_keyStatus.assayType == AssayTypeNew::ASSAYTYPE3_POSITION_SET)
	{
		GenButtonPages();
	}

	// 记录显示的页面
	if (index < m_pageButton.size())
	{
		SetProperty(m_pageButton, m_pageButton[index], "assays", "card_page");
	}
	m_currentPageIndex = index;

	ui->assayCards->updateViewMap(singleMap, combinMap);
}

KeyDataIn QAssayCardPage::GetKeyStatus()
{
	for (const auto& process : m_assaySelectHander)
	{
		if (Q_NULLPTR == process)
		{
			continue;
		}

		 return process->GetKeyStatus();
	}

	KeyDataIn defaultData;
	return defaultData;
}

///
/// @brief 设置关键属性
///
/// @param[in]  keyStatus  属性
///
/// @par History:
/// @li 5774/WuHongTao，2023年12月12日，新建函数
///
void QAssayCardPage::SetKeyStatus(KeyDataIn& keyStatus)
{
	for (const auto& process : m_assaySelectHander)
	{
		if (Q_NULLPTR == process)
		{
			continue;
		}

		process->SetKeyStatus(keyStatus);
	}

	m_keyStatus = keyStatus;
	// 只有位置设置模式才能显示
	if (m_keyStatus.assayType == AssayTypeNew::ASSAYTYPE1_WORKPAGE)
	{
		ui->widget_postion->hide();
		ui->lengend->show();
	}
	else if(m_keyStatus.assayType == AssayTypeNew::ASSAYTYPE3_POSITION_SET)
	{
		ui->widget_postion->show();
		ui->lengend->hide();
	}
	else
	{
		ui->widget_postion->hide();
		ui->lengend->hide();
	}

	OnShowPage(m_currentPageIndex);
}

void QAssayCardPage::SetCurrentSampleType(int currentType)
{
	for (const auto& process : m_assaySelectHander)
	{
		if (Q_NULLPTR == process)
		{
			continue;
		}

		process->SetCurrentSampleType(currentType);
	}

	OnShowPage(m_currentPageIndex);
	
}

void QAssayCardPage::SetAudit(bool audit)
{
	for (const auto& process : m_assaySelectHander)
	{
		if (Q_NULLPTR == process)
		{
			continue;
		}

		process->SetAudit(audit);
	}
}

void QAssayCardPage::SetDilution(SampleSize absortSize, int dulitions)
{
	for (const auto& process : m_assaySelectHander)
	{
		if (Q_NULLPTR == process)
		{
			continue;
		}

		 process->SetDilution(absortSize, dulitions);
	}
}

bool QAssayCardPage::SetAssayCardInfo(int assayCode)
{
	if (!m_postion.has_value())
	{
		return false;
	}

	// 若前一次的位置是组合项目则返回
	if (std::get<0>(m_redokey))
	{
		return false;
	}

	// 首先获取生化免疫的项目基础信息
	std::shared_ptr<CommonInformationManager> assayMgr = CommonInformationManager::GetInstance();
	// 每页项目数目
	int pageAssayCount = PageViewCount(true, m_singleCount);
	// 每页的按钮多少+每页位置
	int currentIndex = m_currentPageIndex * pageAssayCount + m_postion.value();
	for (const auto& process : m_assaySelectHander)
	{
		if (process->GetAssayType() == AssayDeviceType::COMBINTYPE)
		{
			continue;
		}

		bool isOk = process->SetAssayCardInfo(assayCode, currentIndex);
		if (isOk)
		{
			OnShowPage(m_currentPageIndex);
			return true;
		}
	}

	return false;
}

bool QAssayCardPage::SetProfileCardInfo(int profileId)
{
	if (!m_postion.has_value())
	{
		return false;
	}

	// 若前一次的位置是单项目则返回
	if (!std::get<0>(m_redokey))
	{
		return false;
	}

	// 首先获取生化免疫的项目基础信息
	std::shared_ptr<CommonInformationManager> assayMgr = CommonInformationManager::GetInstance();
	// 每页的按钮多少+每页位置(组合项目只有一页)
	int currentIndex = m_postion.value();
	for (const auto& process : m_assaySelectHander)
	{
		if (process->GetAssayType() != AssayDeviceType::COMBINTYPE)
		{
			continue;
		}

		bool isOk = process->SetProfileCardInfo(profileId, currentIndex);
		if (isOk)
		{
			OnShowPage(m_currentPageIndex);
			return true;
		}
	}

	return false;
}

void QAssayCardPage::OnUpdateView()
{	
	ULOG(LOG_INFO, "%s(update page index: %d)", __FUNCTION__, m_currentPageIndex);
	std::shared_ptr<CommonInformationManager> assayMgr = CommonInformationManager::GetInstance();
	m_singleCount = assayMgr->GetSingleAssayRowAmount();
	ui->assayCards->setViewMapTmp(MAXROWCOUNT, m_singleCount, COLUMNCOUNT);
	ui->spinBox->setValue(MAXROWCOUNT - m_singleCount);
	GenButtonPages();

	for (const auto& process : m_assaySelectHander)
	{
		process->Init();
	}

	OnShowPage(m_currentPageIndex);
}

void QAssayCardPage::showEvent(QShowEvent * event)
{
	for (int index = 0; index < m_pageButton.size(); index++)
	{
		if (m_currentPageIndex == index)
		{
			SetProperty(m_pageButton, m_pageButton[index], "assays", "card_page");
		}
	}

	// 首先获取生化免疫的项目基础信息
	OnUpdateView();
}

void QAssayCardPage::Init()
{
	// 首先获取生化免疫的项目基础信息
	std::shared_ptr<CommonInformationManager> assayMgr = CommonInformationManager::GetInstance();
	// 单项目的行数
	m_singleCount = assayMgr->GetSingleAssayRowAmount();
	// 默认7列总行为7行
	ui->spinBox->setMaximum(MAXROW);
	ui->spinBox->setMinimum(MINROW);
	ui->spinBox->setSuffix(tr("行"));
	ui->spinBox->setValue(MAXROWCOUNT - m_singleCount);
	ui->assayCards->setViewMapTmp(MAXROWCOUNT, m_singleCount, COLUMNCOUNT);

	connect(ui->assayCards, &QButtonViewPage::dataSelected, this, [&](bool isCombin, int postion, int assayCode){
        bool isPositionChanged = (postion != m_postion);
		OnSelectAssay(false, isCombin, postion, assayCode);

        if (isPositionChanged)
        {
            emit selectChanged();
        }
	});

	// 更新项目状态信息
	connect(&DataPrivate::Instance(), &DataPrivate::testItemChanged, this, [&](std::set<int64_t> updateDbs)
	{
		if (m_testItems.empty() || updateDbs.empty())
		{
			return;
		}

		// 更新项目信息
		auto updateItem = [&](std::shared_ptr<tf::TestItem> spTestItem)
		{
			if (Q_NULLPTR == spTestItem)
			{
				return;
			}

			for (const auto& process : m_assaySelectHander)
			{
				if (Q_NULLPTR == process)
				{
					continue;
				}

				bool hasDone = process->UpdateTestItemStatus(spTestItem);
				if (hasDone)
				{
					break;
				}
			}
		};

		std::vector<int64_t> needUpdateDbs;
		for (auto& testItem : m_testItems)
		{
			if (testItem == Q_NULLPTR)
			{
				continue;
			}

			if (updateDbs.count(testItem->id) > 0)
			{
				needUpdateDbs.push_back(testItem->id);
				updateItem(testItem);
			}
		}

		if (!needUpdateDbs.empty())
		{
			QStringList dbListString;
			for (auto db : needUpdateDbs)
			{
				dbListString << QString::number(db);
			}

			ULOG(LOG_INFO, "%s(update dbs: %s)", __FUNCTION__, dbListString.join(";").toStdString());
			OnShowPage(m_currentPageIndex);
		}
	});

	connect(ui->spinBox, SIGNAL(valueChanged(int)), this, SLOT(OnChangeRowNumber(int)));
	connect(ui->clear_btn, SIGNAL(clicked()), this, SLOT(OnClear()));
	connect(ui->del_btn, SIGNAL(clicked()), this, SLOT(OnDelete()));
	// 注册项目状态更新函数
	REGISTER_HANDLER(MSG_ID_WORK_ASSAY_STATUS_UPDATE, this, OnUpdateAssayStatus);
	REGISTER_HANDLER(MSG_ID_ASSAY_CODE_MANAGER_UPDATE, this, OnUpdateView);
	// 初始化
	m_redokey = { false, -1, -1 };
	auto softwareType = CommonInformationManager::GetInstance()->GetSoftWareType();
	if (softwareType == SOFTWARE_TYPE::CHEMISTRY 
		|| softwareType == SOFTWARE_TYPE::CHEMISTRY_AND_IMMUNE)
	{
		// 生化模块
		QAssaySelectProcess* chProcess = new QAssayChSelectProcess();
		chProcess->SetCardPage(this);
		m_assaySelectHander.push_back(chProcess);
		m_chemistry = true;
	}

	if (softwareType == SOFTWARE_TYPE::IMMUNE
		|| softwareType == SOFTWARE_TYPE::CHEMISTRY_AND_IMMUNE)
	{
		// 免疫模块
		QAssaySelectProcess* imProcess = new QAssayImSelectProcess();
		imProcess->SetCardPage(this);
		m_assaySelectHander.push_back(imProcess);
		m_chemistry = false;
	}

	if (softwareType == SOFTWARE_TYPE::IMMUNE)
	{
		ui->legend_inc->hide();
		ui->legend_inc_icon->hide();
		ui->legend_dec->hide();
		ui->legend_dec_icon->hide();
	}

	// 默认情况下是生化，当生化和免疫设备都有的情况下
	if (softwareType == SOFTWARE_TYPE::CHEMISTRY_AND_IMMUNE)
	{
		m_chemistry = true;
		QRadioButton* chemistryButton = new QRadioButton(ui->widget_device);
		chemistryButton->setObjectName(tr("item_chemistry"));
		chemistryButton->setText(tr("生化"));
		chemistryButton->setProperty("chemistry",true);
		connect(chemistryButton, SIGNAL(clicked()), this, SLOT(OnChangeDeviceType()));
		ui->widget_device->layout()->addWidget(chemistryButton);

		QRadioButton* immuneButton = new QRadioButton(ui->widget_device);
		immuneButton->setObjectName(tr("item_immune"));
		immuneButton->setText(tr("免疫"));
		immuneButton->setProperty("chemistry", false);
		connect(immuneButton, SIGNAL(clicked()), this, SLOT(OnChangeDeviceType()));
		ui->widget_device->layout()->addWidget(immuneButton);
		ui->widget_device->show();
		chemistryButton->setChecked(true);
	}
	else
	{
		ui->widget_device->hide();
	}

	// 组合项目
	QAssayComBinSelectProcess* combinProcess = new QAssayComBinSelectProcess();
	// 设置子模块
	combinProcess->SetProcess(m_assaySelectHander);
	m_assaySelectHander.push_back(combinProcess);

	m_pageButton.push_back(ui->page_1);
	m_pageButton.push_back(ui->page_2);
	m_pageButton.push_back(ui->page_3);
	m_pageButton.push_back(ui->page_4);
	m_pageButton.push_back(ui->page_5);
	m_pageButton.push_back(ui->page_6);
	for (int index = 0; index < m_pageButton.size(); index++)
	{
		m_pageButton[index]->setProperty("index", index);
		connect(m_pageButton[index], &QPushButton::clicked, this, [&]()
		{
			auto senderPointer = qobject_cast<QPushButton*>(sender());
			if (Q_NULLPTR == senderPointer)
			{
				return;
			}

			int index = senderPointer->property("index").toInt();
			OnShowPage(index);
		});

		// 初次选中按钮
		if (m_currentPageIndex == index)
		{
			SetProperty(m_pageButton, m_pageButton[index], "assays", "card_page");
		}
	}

	// 初始化页面
	GenButtonPages();
	OnShowPage(0);
}

int QAssayCardPage::PageViewCount(bool isSingle, int singleCount)
{
	ULOG(LOG_INFO, "%s(single column count: %d)", __FUNCTION__, singleCount);
	int elementCount = -1;
	if (isSingle)
	{
		elementCount = m_singleCount * COLUMNCOUNT;
	}
	else
	{
		elementCount = (MAXROWCOUNT - m_singleCount) * COLUMNCOUNT;
	}

	if (elementCount <= 0)
	{
		if (isSingle)
		{
			elementCount = UI_DEFAULT_ASSAYSELECTTB_SINGLEROW * COLUMNCOUNT;
		}
		else
		{
			elementCount = (MAXROWCOUNT - UI_DEFAULT_ASSAYSELECTTB_SINGLEROW) * COLUMNCOUNT;
		}
	}

	return elementCount;
}

void QAssayCardPage::SetProperty(std::vector<QPushButton*> groupButtons, QPushButton* pushButton, const char* name, const QString& propertyMessage)
{
	if (pushButton == Q_NULLPTR || name == Q_NULLPTR)
	{
		return;
	}

	// 清除其他按钮的状态
	for (auto button : groupButtons)
	{
		if (button == pushButton)
		{
			continue;
		}

		button->setProperty(name, QVariant());
		button->style()->unpolish(button);
		button->style()->polish(button);
	}

	pushButton->setProperty(name, propertyMessage);
	pushButton->style()->unpolish(pushButton);
	pushButton->style()->polish(pushButton);
}

void QAssayCardPage::GenButtonPages()
{
	int maxPostion = -1;
	for (const auto& process : m_assaySelectHander)
	{
		if (Q_NULLPTR == process)
		{
			continue;
		}

		if (m_chemistry)
		{
			if (process->GetAssayType() == AssayDeviceType::CHEMISTRYTYPE)
			{
				maxPostion = process->GetMaxPostion();
				break;
			}
		}
		else
		{
			if (process->GetAssayType() == AssayDeviceType::IMMUNETYPE)
			{
				maxPostion = process->GetMaxPostion();
				break;
			}
		}
	}

	int pageNeed = UI_ASSAYPAGEMAX;
	std::shared_ptr<CommonInformationManager> assayMgr = CommonInformationManager::GetInstance();
	int pageButtonNumber = PageViewCount(true, m_singleCount);
	if (pageButtonNumber == 0)
	{
		pageNeed = UI_ASSAYPAGEMAX;
	}
	else
	{
		maxPostion++;
		int tmp = maxPostion / pageButtonNumber;
		if (maxPostion%pageButtonNumber != 0)
		{
			tmp++;
		}

		if (tmp > pageNeed)
		{
			pageNeed = tmp;
		}
	}

	int index = 0;
	for (auto& button :m_pageButton)
	{
		if (index >= pageNeed)
		{
			button->hide();
			index++;
			continue;;
		}

		index++;
		button->show();
	}
}

///
/// @brief 根据项目基础信息生成按钮信息(组合项目)
///
/// @param[in]  rawAssayInfo   项目基础信息
/// @param[in]  keyMap         位置映射关系
/// @param[in]  buttonData     项目按钮信息
///
/// @return 
///
/// @par History:
/// @li 5774/WuHongTao，2023年12月11日，新建函数
///
void QAssayCardPage::GenButtonCombinInfo(const std::vector<tf::ProfileAssayPageItem>& rawAssayInfo, boost::bimap<int, int>& keyMap, std::map<int, AssayButtonData>& buttonData)
{
	auto assayMgr = CommonInformationManager::GetInstance();
	if (assayMgr == Q_NULLPTR)
	{
		return;
	}

	auto ProfileAssayMaps = assayMgr->GetCommonProfileMap();
	if (!ProfileAssayMaps.has_value())
	{
		return;
	}

	// 生化的序列关系
	for (const auto& item : rawAssayInfo)
	{
		AssayButtonData assayBtnData;
		auto iter = ProfileAssayMaps.value().find(item.id);
		if (iter == ProfileAssayMaps.value().end())
		{
			continue;
		}

		if (Q_NULLPTR == iter->second)
		{
			continue;
		}
		auto assayProfile = iter->second;

		assayBtnData.bIsUse = true;
		// 编号和号码的生成规则
		assayBtnData.assayCode = assayProfile->id + UI_PROFILESTART;
		// 填充组合项目的代码
		assayBtnData.strSubAssayList.insert(assayBtnData.strSubAssayList.end(), item.subAssays.begin(), item.subAssays.end());
		int position = item.positionIdx;
		keyMap.insert({ position, int(assayBtnData.assayCode) });

		assayBtnData.strAssayName = QString::fromStdString(assayProfile->profileName);
		buttonData[assayBtnData.assayCode] = assayBtnData;
	}
}

///
/// @brief 项目选中消息
///
/// @param[in]  assayCode  项目编号
///
/// @par History:
/// @li 5774/WuHongTao，2023年12月12日，新建函数
///
void QAssayCardPage::OnSelectAssay(bool isManual,  bool isCombin, int postion, int assayCode)
{
	ULOG(LOG_INFO, "%s(process type: %s postion: %d assayCode: %d)", __FUNCTION__, isCombin?"C":"S", postion, assayCode);

	m_redokey = { isCombin, postion, assayCode };
	if (postion == -1)
	{
		return;
	}

	// 首先获取生化免疫的项目基础信息
	std::shared_ptr<CommonInformationManager> assayMgr = CommonInformationManager::GetInstance();
	// 单项项目每页项目数目
	int pageAssayCount = PageViewCount(true, m_singleCount);
	// 组合项目每页的项目数目
	int pageCombinCount = UI_ASSAYPAGENUM - pageAssayCount;

	for (const auto& process : m_assaySelectHander)
	{
		if (Q_NULLPTR == process)
		{
			continue;
		}

		// modify bug0011835 by wuht
		if (m_keyStatus.assayType == AssayTypeNew::ASSAYTYPE3_POSITION_SET)
		{
			process->ClearSelectd();
		}

		int currentIndex = -1;
		// 组合项目
		if (process->GetAssayType() == AssayDeviceType::COMBINTYPE && isCombin)
		{
			currentIndex = postion;
			process->ProcessAssay(assayCode, currentIndex);
		}
		// 生化
		else if (process->GetAssayType() == AssayDeviceType::CHEMISTRYTYPE && !isCombin && m_chemistry)
		{
			// 每页的按钮多少+每页位置
			currentIndex = m_currentPageIndex * pageAssayCount + postion;
			process->ProcessAssay(assayCode, currentIndex);
		}
		// 免疫
		else if (process->GetAssayType() == AssayDeviceType::IMMUNETYPE && !isCombin && !m_chemistry)
		{
			// 每页的按钮多少+每页位置
			currentIndex = m_currentPageIndex * pageAssayCount + postion;
			process->ProcessAssay(assayCode, currentIndex);
		}
	}

	m_postion = postion;
	OnShowPage(m_currentPageIndex);
	// 通知应用层某项目被选中(非手动选中才发出信息，以免造成消息循环)
	if (!isManual)
	{
		emit assaySelected(postion, assayCode);
	}
}

void QAssayCardPage::OnChangeDeviceType()
{
	auto radioButton = static_cast<QRadioButton*>(sender());
	if (Q_NULLPTR == radioButton)
	{
		return;
	}

	m_chemistry = radioButton->property("chemistry").toBool();
	OnShowPage(m_currentPageIndex);

	// 若是切换到生化设备，0代表生化
	if (m_chemistry)
	{
		emit deviceTypeChanged(0);
	}
	// 免疫设备 1
	else
	{
		emit deviceTypeChanged(1);
	}
}

void QAssayCardPage::OnChangeRowNumber(int pageNumber)
{
	ULOG(LOG_INFO, "%s(pageNumber: %d)", __FUNCTION__, pageNumber);
	if (pageNumber < MINROW || pageNumber > MAXROW)
	{
		ULOG(LOG_WARN, "%s(over pageNumber: %d)", __FUNCTION__, pageNumber);
		return;
	}

	m_singleCount = MAXROWCOUNT - pageNumber;
	ui->assayCards->setViewMapTmp(MAXROWCOUNT, m_singleCount, COLUMNCOUNT);
	GenButtonPages();
	OnShowPage(m_currentPageIndex);
}

void QAssayCardPage::OnClear()
{
	for (const auto& process : m_assaySelectHander)
	{
		if (Q_NULLPTR == process)
		{
			continue;
		}

		process->ClearAllAssay();
	}

	OnShowPage(m_currentPageIndex);
}

void QAssayCardPage::OnDelete()
{
	for (const auto& process : m_assaySelectHander)
	{
		if (Q_NULLPTR == process)
		{
			continue;
		}

		process->DeleteCurrentAssay();
	}

	OnShowPage(m_currentPageIndex);
}

void QAssayCardPage::OnUpdateAssayStatus(QString assayCodes)
{
	ULOG(LOG_INFO, "%s, Param %s.", __FUNCTION__, assayCodes.toStdString().c_str());

	// 只有工作页面才做更新
	if (m_keyStatus.assayType != AssayTypeNew::ASSAYTYPE1_WORKPAGE)
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

		for (const auto& process : m_assaySelectHander)
		{
			if (Q_NULLPTR == process)
			{
				continue;
			}
			process->UpdateAssayStatus(assayCode.toInt());
		}
	}

	OnShowPage(m_currentPageIndex);
}

void QAssaySelectProcess::SetKeyStatus(KeyDataIn& keyStatus)
{
	// 当改变样本类型以后需要立即让有些项目不能选择，变灰
	m_keyStatus = keyStatus;
	// 生化项目单项目
	for (auto& data : m_currentButtons)
	{
		data.second.bIsForbidden = IsEnableAssayCode(data.second.assayCode, keyStatus.sourceType);
		if (m_keyStatus.assayType == AssayTypeNew::ASSAYTYPE3_POSITION_SET)
		{
			data.second.bIsCaliAbn = false;
			data.second.bIsMask = false;
			data.second.bIsRegentAbn = false;
		}
	}
}

void QAssaySelectProcess::SetCurrentSampleType(int currentType)
{
	ULOG(LOG_INFO, "%s(currentType: %d)", __FUNCTION__, currentType);
	// 生化项目单项目
	for (auto& data : m_currentButtons)
	{
		data.second.bIsForbidden = IsEnableAssayCode(data.second.assayCode, currentType);
	}

	m_keyStatus.sourceType = currentType;
}

void QAssaySelectProcess::SetAudit(bool audit)
{
	ULOG(LOG_INFO, "%s(audit: %s)", __FUNCTION__, audit ? "yes" : "No");
	m_keyStatus.isAudit = audit;
}

void QAssaySelectProcess::SetDilution(SampleSize absortSize, int dulitions)
{
	ULOG(LOG_INFO, "%s(absortSize: %d, dulitions: %d)", __FUNCTION__, absortSize, dulitions);
	m_keyStatus.dulitionType = absortSize;
	if (absortSize == USER_SIZE || absortSize == MANUAL_SIZE)
	{
		m_keyStatus.dulitions = dulitions;
	}
	else
	{
		m_keyStatus.dulitions = 1;
	}
}

void QAssaySelectProcess::ClearSelectd()
{
	ULOG(LOG_INFO, "%s)", __FUNCTION__);
	for (auto& data : m_currentButtons)
	{
		if (data.second.bIsRequest)
		{
			data.second.bIsRequest = false;
		}
	}
}

int QAssaySelectProcess::GetMaxPostion()
{
	int maxPostion = -1;
	for (const auto& data : m_singleMaps)
	{
		if (data.left > maxPostion)
		{
			maxPostion = data.left;
		}
	}

	return maxPostion;
}

bool QAssaySelectProcess::DeleteCurrentAssay()
{
	ULOG(LOG_INFO, "%s)", __FUNCTION__);
	if (!m_postion.has_value())
	{
		return  false;
	}

	int postion = m_postion.value();
	auto iter = m_singleMaps.left.find(postion);
	if (iter == m_singleMaps.left.end())
	{
		return false;
	}

	int code = iter->second;
	DeleteBimap(m_singleMaps, postion, boost::none);
    
    // 删除当前选择的按钮，使m_currentButtons中的记录与界面保持一致
    std::map<int, AssayButtonData>::iterator curBtnIter = m_currentButtons.find(code);
    if (curBtnIter != m_currentButtons.end())
    {
        m_currentButtons.erase(curBtnIter);
    }

	m_changeMap[code] = -1;
	return true;
}

bool QAssaySelectProcess::DeleteAssayByCode(int assayCode)
{
	ULOG(LOG_INFO, "%s(assayCode: %d)", __FUNCTION__, assayCode);
	auto iter = m_singleMaps.right.find(assayCode);
	if (iter == m_singleMaps.right.end())
	{
		ULOG(LOG_WARN, "%s(can not find assayCode: %d)", __FUNCTION__, assayCode);
		return false;
	}

	DeleteBimap(m_singleMaps, boost::none, assayCode);
	m_changeMap[assayCode] = -1;
	return true;
}

bool QAssaySelectProcess::ClearAllAssay()
{
	for (auto postion : m_singleMaps)
	{
		m_changeMap[postion.right] = -1;
	}

	m_singleMaps.clear();
	return true;
}

void QAssaySelectProcess::RefreshCard()
{
	if (m_keyStatus.assayType == AssayTypeNew::ASSAYTYPE3_POSITION_SET)
	{
		Init();
		m_postion = boost::none;
		m_changeMap.clear();
	}
	else
	{
		m_currentButtons = m_basicAssayButtons;
	}
}

void QAssaySelectProcess::UpdateAssayStatus(int assayCode)
{
	ULOG(LOG_INFO, "%s, Param %d.", __FUNCTION__, assayCode);
	// 只有工作页面才做更新
	if (m_keyStatus.assayType != AssayTypeNew::ASSAYTYPE1_WORKPAGE)
	{
		ULOG(LOG_INFO, "Short ret by type diff.");
		return;
	}

	// 主状态
	auto assayStatus = QSystemMonitor::GetInstance().GetAssayStatus(assayCode);
	// 支持ISE和SIND
	std::vector<int> subCodes;
	ConvertCodes(subCodes, assayCode);
	for (const auto& code : subCodes)
	{
		auto iterAssay = m_basicAssayButtons.find(code);
		if (iterAssay == m_basicAssayButtons.end())
		{
			ULOG(LOG_INFO, "Can not find the assaycode: %d.", code);
			continue;
		}

		auto subAssayStatus = QSystemMonitor::GetInstance().GetAssayStatus(code);
		if (subAssayStatus)
		{
			assayStatus = subAssayStatus;
		}

		// 获取当前的项目的属性
		auto attribute = iterAssay->second;
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
			m_basicAssayButtons[code] = attribute;
			// 同时设置当前状态
			if (m_currentButtons.count(code) > 0)
			{
				// 项目遮蔽
				m_currentButtons[code].bIsMask = status.maskAssay;
				// 试剂异常
				m_currentButtons[code].bIsRegentAbn = status.abnormalReagent;
				// 校准异常
				m_currentButtons[code].bIsCaliAbn = status.abnormalCalibrate;
			}
		}
	}
}

bool QAssaySelectProcess::SetAssayCardInfo(int assayCode, int postion)
{
	ULOG(LOG_INFO, "%s(postion : %d ,code : %d)", __FUNCTION__, postion, assayCode);

	std::shared_ptr<::tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(assayCode);
	if (Q_NULLPTR == spAssayInfo)
	{
		ULOG(LOG_WARN, "%s(Invalid assay name, code : %d)", __FUNCTION__, assayCode);
		return false;
	}

	// 若当前的选中页面是组合
	if (m_assayType == AssayDeviceType::COMBINTYPE)
	{
		ULOG(LOG_WARN, "%s(profile not allowed, code : %d)", __FUNCTION__, assayCode);
		return false;
	}

	// 生化项目必须设置在生化的Page上
	if (spAssayInfo->assayClassify == tf::AssayClassify::type::ASSAY_CLASSIFY_CHEMISTRY)
	{
		if (m_assayType != AssayDeviceType::CHEMISTRYTYPE)
		{
			ULOG(LOG_WARN, "%s(wrong device type, code : %d)", __FUNCTION__, assayCode);
			return false;
		}
	}

	// 免疫项目必须设置在免疫的Page上
	if (spAssayInfo->assayClassify == tf::AssayClassify::type::ASSAY_CLASSIFY_IMMUNE)
	{
		if (m_assayType != AssayDeviceType::IMMUNETYPE)
		{
			ULOG(LOG_WARN, "%s(wrong device type, code : %d)", __FUNCTION__, assayCode);
			return false;
		}
	}

	// 1：若项目之前不存在，无论当前位置是否存在项目，需要覆盖当前位置的项目
	// 2: 将项目加入到列表中
	auto iter = m_currentButtons.find(assayCode);
	if (iter == m_currentButtons.end())
	{
		AssayButtonData buttonData;
		buttonData.bIsUse = true;
		buttonData.bIsRequest = true;
		buttonData.assayCode = spAssayInfo->assayCode;
		buttonData.strAssayName = QString::fromStdString(spAssayInfo->assayName);
		m_currentButtons[assayCode] = buttonData;
	}
	// 若能找到则需要抹除之前的位置
	else
	{
		iter->second.bIsRequest = true;
		boost::bimap<int, int>::right_iterator iterPostion = m_singleMaps.right.find(assayCode);
		if (iterPostion != m_singleMaps.right.end())
		{
			// 清除位置
			m_changeMap[assayCode] = -1;
			DeleteBimap(m_singleMaps, boost::none, assayCode);
		}
	}

	// 查看需要设置的位置，当前是否有项目在位，若没有直接覆盖
	// 若当前位置有有项目在位，在覆盖之前需要首先从内容表中去掉
	boost::bimap<int, int>::left_iterator iterPostion = m_singleMaps.left.find(postion);
	if (iterPostion != m_singleMaps.left.end())
	{
		auto iterCode = m_currentButtons.find(iterPostion->second);
		if (iterCode != m_currentButtons.end())
		{
			m_currentButtons.erase(iterCode);
		}

		// 清除位置
		m_changeMap[iterPostion->second] = -1;
		DeleteBimap(m_singleMaps, postion, boost::none);
	}

	// 添加位置信息
	m_singleMaps.insert({ postion, assayCode });
	m_changeMap[assayCode] = postion;
	return true;
}

bool QAssaySelectProcess::SetProfileCardInfo(int profileId, int postion)
{
	ULOG(LOG_INFO, "%s(postion : %d ,code : %d)", __FUNCTION__, postion, profileId);

	// 若当前的选中页面是组合
	if (m_assayType != AssayDeviceType::COMBINTYPE)
	{
		ULOG(LOG_WARN, "%s(muse combin assay, profileId : %d)", __FUNCTION__, profileId);
		return false;
	}

	auto ProfileAssayMaps = CommonInformationManager::GetInstance()->GetCommonProfileMap();
	if (!ProfileAssayMaps.has_value())
	{
		return false;
	}

	auto iter = ProfileAssayMaps.value().find(profileId - UI_PROFILESTART);
	if (iter == ProfileAssayMaps.value().end())
	{
		return false;
	}

	if (Q_NULLPTR == iter->second)
	{
		return false;
	}
	auto assayProfile = iter->second;

	// 若项目之前没有被加入，则加入
	auto iterAssay = m_currentButtons.find(profileId);
	if (iterAssay == m_currentButtons.end())
	{
		AssayButtonData assayBtnData;
		assayBtnData.bIsUse = true;
		assayBtnData.bIsRequest = true;
		// 编号和号码的生成规则
		assayBtnData.assayCode = profileId;
		// 填充组合项目的代码
		assayBtnData.strSubAssayList.insert(assayBtnData.strSubAssayList.end(), assayProfile->subItems.begin(), assayProfile->subItems.end());
		assayBtnData.strAssayName = QString::fromStdString(assayProfile->profileName);
		m_currentButtons[assayBtnData.assayCode] = assayBtnData;
	}
	else
	{
		iterAssay->second.bIsRequest = true;
		boost::bimap<int, int>::right_iterator iterPostion = m_singleMaps.right.find(profileId);
		if (iterPostion != m_singleMaps.right.end())
		{
			m_changeMap[profileId] = -1;
			DeleteBimap(m_singleMaps, boost::none, profileId);
		}
	}

	// 位置已有项目在使用，则先删除
	auto iterPostion = m_singleMaps.left.find(postion);
	if (iterPostion != m_singleMaps.left.end())
	{
        int iPosition = iterPostion->second;
		auto iterCode = m_currentButtons.find(iPosition);
		if (iterCode != m_currentButtons.end())
		{
			m_currentButtons.erase(iterCode);
        }
        if (iPosition == -1)
        {
            std::map<int, int>::iterator dIter = m_changeMap.find(-1);
            if (dIter != m_changeMap.end())
            {
                m_changeMap.erase(dIter);
            }
        }
        else
        {
            m_changeMap[iPosition] = -1;
        }
		DeleteBimap(m_singleMaps, postion, boost::none);
	}

	// 添加位置信息
	m_singleMaps.insert({ postion, profileId });
	m_changeMap[profileId] = postion;
	return true;
}

std::pair<int, int> QAssaySelectProcess::GetSampleTestItemInfo(std::vector<::tf::TestItem>& testItems, const std::map<int, AssayButtonData>& AssayButtonDataMap)
{
	ULOG(LOG_INFO, "%s(assayNumber: %d)", __FUNCTION__, AssayButtonDataMap.size());

	int request = 0;
	int preDilutionFactor = 1;
	for (auto& rawData : AssayButtonDataMap)
	{
		auto stuAssayBtnData = rawData.second;
		// 仅仅是展示，未使用
		if (!stuAssayBtnData.bIsUse)
		{
			continue;
		}

		::tf::TestItem testItem;
		std::shared_ptr<::tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(stuAssayBtnData.assayCode);
		if (Q_NULLPTR == spAssayInfo)
		{
			ULOG(LOG_WARN, "Invalid assay name, code is %d,", stuAssayBtnData.assayCode);
			continue;
		}

		// 1：首先设置稀释类型和项目基础信息
		testItem.__set_assayCode(spAssayInfo->assayCode);
		testItem.__set_suckVolType((::tf::SuckVolType::type)stuAssayBtnData.enSampleSize);
		testItem.__set_dilutionFactor(-1);

		// 2： 再次根据情况设置稀释的具体信息
		// 测试项目请求状态
		if (stuAssayBtnData.bIsRequest && !stuAssayBtnData.bIsAnalyzed && !stuAssayBtnData.bIsRecheck)
		{
			testItem.__set_status(::tf::TestItemStatus::TEST_ITEM_STATUS_PENDING);
			// 手工稀释
			if (stuAssayBtnData.enSampleSize == MANUAL_SIZE)
			{
				testItem.__set_suckVolType(::tf::SuckVolType::type::SUCK_VOL_TYPE_STD);
				testItem.__set_preDilutionFactor(stuAssayBtnData.iDilution);
				preDilutionFactor = stuAssayBtnData.iDilution;
			}
			else
			{
				// 机内稀释首先将机外稀释置为1
				testItem.__set_preDilutionFactor(1);
				testItem.__set_dilutionFactor(stuAssayBtnData.iDilution);
				preDilutionFactor = 1;
			}

			// 统计请求项目数
			request++;
		}
		// 已完成
		else if (!stuAssayBtnData.bIsRequest && stuAssayBtnData.bIsAnalyzed && !stuAssayBtnData.bIsRecheck)
		{
			// 当增加项目的手工稀释倍数以后，已经完成的稀释倍数可能变化（复查->已完成）
			// 手工稀释
			if (stuAssayBtnData.enSampleSize == MANUAL_SIZE)
			{
				testItem.__set_suckVolType(::tf::SuckVolType::type::SUCK_VOL_TYPE_STD);
				testItem.__set_preDilutionFactor(stuAssayBtnData.iDilution);
			}
			else
			{
				// 机内稀释首先将机外稀释置为1
				testItem.__set_preDilutionFactor(1);
				testItem.__set_dilutionFactor(stuAssayBtnData.iDilution);
			}

			testItem.__set_rerun(false);
		}
		// 复查
		else if (!stuAssayBtnData.bIsRequest && !stuAssayBtnData.bIsAnalyzed && stuAssayBtnData.bIsRecheck)
		{
			testItem.__set_rerun(true);
			//	复查的时候设置状态为项目请求状态
			testItem.__set_status(::tf::TestItemStatus::TEST_ITEM_STATUS_PENDING);
			// 手工稀释
			if (stuAssayBtnData.enSampleSize == MANUAL_SIZE)
			{
				testItem.__set_suckVolType(::tf::SuckVolType::type::SUCK_VOL_TYPE_STD);
				testItem.__set_preDilutionFactor(stuAssayBtnData.iDilution);
				preDilutionFactor = stuAssayBtnData.iDilution;
			}
			else
			{
				// 机内稀释首先将机外稀释置为1
				testItem.__set_preDilutionFactor(1);
				testItem.__set_dilutionFactor(stuAssayBtnData.iDilution);
				preDilutionFactor = 1;
			}

			request++;
		}
		// 未在范围内
		else
		{
			testItem.__set_dilutionFactor(stuAssayBtnData.iDilution);
			continue;
		}

		// 当拥有id的时候表示非新增项目
		int i = 0;
		for (auto guid : stuAssayBtnData.guidList)
		{
			if (guid > 0)
			{
				testItem.__set_id(guid);
			}
			else
			{
				testItem.__isset.id = false;
				testItem.id = -1;
			}

			// 重复索引
			testItem.__set_repeatIdx(i);
			i++;

			testItems.push_back(testItem);
		}
	}

	return std::make_pair(request, preDilutionFactor);
}

bool QAssaySelectProcess::UpdateTestItemStatus(std::shared_ptr<tf::TestItem>& testItems)
{
	if (Q_NULLPTR == testItems)
	{
		ULOG(LOG_WARN, "%s(the testItem is null: %d)", __FUNCTION__);
		return false;
	}

	ULOG(LOG_INFO, "%s(assayCode: %d)", __FUNCTION__, testItems->assayCode);
	auto iter = m_currentButtons.find(testItems->assayCode);
	if (iter == m_currentButtons.end())
	{
		return false;
	}

	auto& attribute = iter->second;
	int testTimes = attribute.guidList.size();
	if (testTimes < 1)
	{
		MergeAssayStatus(attribute, *testItems);
	}
	else if ((testItems->__isset.rerun && testItems->rerun
		&& !(testItems->__isset.status && testItems->status == tf::TestItemStatus::TEST_ITEM_STATUS_TESTED)))
	{
		MergeAssayStatus(attribute, *testItems);
	}
	else
	{
		attribute.guidList.push_back(testItems->id);
	}

	return true;
}

bool QAssaySelectProcess::UpdateTestItemTimes(int assayCode, int testTimes)
{
	ULOG(LOG_INFO, "%s(assayCode: %d, testTimes: %d)", __FUNCTION__, assayCode, testTimes);
	auto iter = m_currentButtons.find(assayCode);
	if (iter == m_currentButtons.end())
	{
		ULOG(LOG_WARN, "%s(can not find assayCode: %d)", __FUNCTION__, assayCode);
		return false;
	}

	auto attribute = iter->second;
	// 获取测试项目的id列表
	int dbsize = attribute.guidList.size();
	// 减少
	if (dbsize > testTimes)
	{
		attribute.guidList.erase((attribute.guidList.begin() + testTimes), attribute.guidList.end());
	}
	// 增加
	else if (dbsize < testTimes)
	{
		// 增加-1表示新增
		attribute.guidList.insert(attribute.guidList.end(), (testTimes - dbsize), -1);
	}
	// 等于的话不做任何变化
	else
	{
		return true;
	}

	m_currentButtons[assayCode] = attribute;
	return true;
}

bool QAssaySelectProcess::GenAssayButtonDataByIndex(const int pageElementCount, std::map<int, AssayButtonData>& assayDatas, int index)
{
	ULOG(LOG_INFO, "%s(index: %d, size: %d)", __FUNCTION__, index, assayDatas.size());
	// 每页的开始位置
	int start = pageElementCount * index;
	// 每页结束位置
	int end = pageElementCount * (index + 1);

	// 单项目显示
	for (auto& data : m_singleMaps)
	{
		if (data.left < start || data.left > end)
		{
			continue;
		}

		int assayCode = data.right;
		auto iter = m_currentButtons.find(assayCode);
		if (iter == m_currentButtons.end())
		{
			continue;
		}

		// 位置设置模式（位置设置模式的时候，任何时候只能有一个项目被选中，其他的位置选中状态必须被清除）
		if (m_keyStatus.assayType == AssayTypeNew::ASSAYTYPE3_POSITION_SET)
		{
			if (m_postion.has_value() && m_postion.value() != data.left)
			{
				iter->second.bIsRequest = false;
			}

			iter->second.bIsCaliAbn = false;
			iter->second.bIsMask = false;
			iter->second.bIsRegentAbn = false;
		}

		// 备用订单的时候，只需要清除其中的试剂状态，屏蔽等状态信息
		if (m_keyStatus.assayType == AssayTypeNew::ASSAYTYPE2)
		{
			iter->second.bIsCaliAbn = false;
			iter->second.bIsMask = false;
			iter->second.bIsRegentAbn = false;
		}

		// 相对位置
		int indexStart = data.left - start;
		assayDatas[indexStart] = iter->second;
	}

	return true;
}

void QAssaySelectProcess::ConvertCodes(std::vector<int>& convertCodes, const int assayCode)
{
	// ise项目
	if (assayCode == ::ise::tf::g_ise_constants.ASSAY_CODE_ISE)
	{
		convertCodes = { ::ise::tf::g_ise_constants.ASSAY_CODE_NA,
			::ise::tf::g_ise_constants.ASSAY_CODE_K,
			::ise::tf::g_ise_constants.ASSAY_CODE_CL };
	}

	// LHI血清指数
	if (assayCode == ::ch::tf::g_ch_constants.ASSAY_CODE_SIND)
	{
		convertCodes = { ::ch::tf::g_ch_constants.ASSAY_CODE_L,
			::ch::tf::g_ch_constants.ASSAY_CODE_H,
			::ch::tf::g_ch_constants.ASSAY_CODE_I };
	}

	if (convertCodes.empty())
	{
		convertCodes = { assayCode };
	}
}

///
/// @brief 获取组合项目信息
///
/// @param[in]  profiles			组合项目Id合集
/// @param[in]  AssayButtonDataMap  项目信息
///
/// @par History:
/// @li 5774/WuHongTao，2023年12月14日，新建函数
///
void QAssaySelectProcess::GetSampleProfileInfo(std::vector<int64_t>& profiles, const std::map<int, AssayButtonData>& AssayButtonDataMap)
{
	ULOG(LOG_INFO, "%s(assayNumber: %d)", __FUNCTION__, AssayButtonDataMap.size());

	for (auto& rawData : AssayButtonDataMap)
	{
		auto stuAssayBtnData = rawData.second;
		// 仅仅是展示，未使用
		if (!stuAssayBtnData.bIsUse)
		{
			continue;
		}

		if (stuAssayBtnData.bIsRequest)
		{
			profiles.push_back(stuAssayBtnData.assayCode);
		}
	}
}

///
/// @brief 检测免疫项目稀释倍数设置是否超过要求的范围
///
/// @param[in]  data  项目选择按钮属性
///
/// @return true：通过， false：不通过
///
/// @par History:
/// @li 1556/Chenjianlin，2023年9月14日，新建函数
///
bool QAssaySelectProcess::IsImDiluTionPolicy(const AssayButtonData& data)
{
	// 如果方式手工稀释
	if (m_keyStatus.dulitionType == MANUAL_SIZE)
	{
		// 手工稀释稀释倍数超出范围 // 0018196: [工作] 能成功添加稀释因子为1的样本 modify by chenjianlin 20230913
		if (m_keyStatus.dulitions < 2 || m_keyStatus.dulitions > 640000000)
		{
			ReportErr(true, tr("手工稀释倍数超过允许范围（2-640000000），请重新设置！"));
			return false;
		}
	}
	else if (USER_SIZE == m_keyStatus.dulitionType) // 如果时自动稀释
	{
		// 获取免疫项目参数
		auto spAssay = CommonInformationManager::GetInstance()->GetImmuneAssayInfo(data.assayCode, tf::DeviceType::DEVICE_TYPE_I6000);
		if (nullptr == spAssay)
		{
			ULOG(LOG_ERROR, "%s(can not find assyaCode : %d)", __FUNCTION__, data.assayCode);
			return true;
		}
		// bug修改 0018596: [工作] 稀释倍数可设置超出项目稀释范围的值
		// 不能大于本项目的最大稀释倍数
		if (m_keyStatus.dulitions > spAssay->reactInfo.maxDilutionRatio)
		{
			QString strMsg = tr("稀释倍数超过允许范围（%1-%2），请重新设置！").arg(spAssay->reactInfo.dilutionRatio).arg(spAssay->reactInfo.maxDilutionRatio);
			ReportErr(true, strMsg);
			return false;
		}
		// 不能小于默认稀释倍数
		if (m_keyStatus.dulitions < spAssay->reactInfo.dilutionRatio)
		{
			QString strMsg = tr("稀释倍数超过允许范围（%1-%2），请重新设置！").arg(spAssay->reactInfo.dilutionRatio).arg(spAssay->reactInfo.maxDilutionRatio);
			ReportErr(true, strMsg);
			return false;
		}
	}

	return true;
}

///
/// @brief 稀释策略是否符合标准
///
/// @param[in]  data  项目属性
///
/// @return true:符合
///
/// @par History:
/// @li 5774/WuHongTao，2023年12月12日，新建函数
///
bool QAssaySelectProcess::IsDiluTionPolicy(const AssayButtonData& data)
{
	ULOG(LOG_INFO, "%s( dulition judge assayCode: %d)", __FUNCTION__, data.assayCode);
	// 为备用订单的类型的特殊处理
	// 项目位置设置也不需要关注稀释标准
	if (m_keyStatus.assayType == AssayTypeNew::ASSAYTYPE2
		|| m_keyStatus.assayType == AssayTypeNew::ASSAYTYPE3_POSITION_SET)
	{
		return true;
	}

	// 请求测试(取消请求测试)
	if (data.bIsRequest && !data.bIsAnalyzed && !data.bIsRecheck)
	{
		return true;
	}

	// 复查（取消复查）
	if (!data.bIsRequest && !data.bIsAnalyzed && data.bIsRecheck)
	{
		return true;
	}

	std::vector<::tf::TestItem> testItems;
	if (m_keyStatus.isAudit)
	{
		TipDlg(tr("提示"), tr("样本已审核，不可修改！")).exec();
		return false;
	}

	// 查找项目类型
	auto spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(data.assayCode);
	if (spAssay == Q_NULLPTR)
	{
		return false;
	}

	// 是否适配ISE和LHI(ISE和LHI只能使用手工稀释和常量稀释)
	auto isFitForSINandISE = [&]()->bool
	{
		if (m_keyStatus.dulitionType == STD_SIZE || m_keyStatus.dulitionType == MANUAL_SIZE)
		{
			return true;
		}

		return false;
	};

	// ISE,SIND,HBA1c(糖化血红蛋白),都属于特殊项目，只能做手工稀释和常量稀释
	auto isSpecialAssay = [&](int assayCode)->bool
	{
		if (IsSIND(data.assayCode)
			|| IsISE(data.assayCode)
			|| (data.assayCode == ::ch::tf::g_ch_constants.ASSAY_CODE_HBA1c))
		{
			return true;
		}

		return false;
	};

	if (isSpecialAssay(data.assayCode) && !isFitForSINandISE())
	{
		ReportErr(true, tr("当前项目不能使用此稀释方式！"));
		return false;
	}

	// 首先判断句柄是否为空
	if (nullptr == m_cardPage)
	{
		return false;
	}

	// 所有项目（生化，免疫）
	auto keyData = m_cardPage->GetSampleTestItemInfo(testItems);
	// 没有请求项目，返回true
	if (keyData.first == 0)
	{
		if (m_keyStatus.dulitionType == MANUAL_SIZE || m_keyStatus.dulitionType == USER_SIZE)
		{
			if (m_keyStatus.dulitions <= 0)
			{
				ReportErr(true, tr("稀释倍数不能为空！"));
				return false;
			}

			// 如果是免疫 	0018196: [工作] 能成功添加稀释因子为1的样本 modify by chenjianlin 20230913
			if (spAssay->assayClassify == tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE)
			{
				// 检测免疫项目稀释倍数设置是否超过要求的范围
				if (!IsImDiluTionPolicy(data))
				{
					return false;
				}
			}
			else
			{
				// 手工稀释不能是1
				if (m_keyStatus.dulitionType == MANUAL_SIZE && m_keyStatus.dulitions == 1)
				{
					ReportErr(true, tr("手工稀释不能是1！"));
					return false;
				}

				if (spAssay->assayClassify == tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY
					&& m_keyStatus.dulitionType == USER_SIZE
					&& m_keyStatus.dulitions < 3)
				{
					ReportErr(true, tr("自动稀释倍数超出范围！"));
					return false;
				}
			}
		}

		return true;
	}

	if (m_keyStatus.dulitionType == MANUAL_SIZE || m_keyStatus.dulitionType == USER_SIZE)
	{
		if (m_keyStatus.dulitions <= 0)
		{
			ReportErr(true, tr("稀释倍数不能为空！"));
			return false;
		}

		if (spAssay->assayClassify == tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY
			&& m_keyStatus.dulitionType == USER_SIZE
			&& m_keyStatus.dulitions < 3)
		{
			ReportErr(true, tr("自动稀释倍数超出范围！"));
			return false;
		}

		// 如果是免疫 	0018196: [工作] 能成功添加稀释因子为1的样本 modify by chenjianlin 20230913
		if (spAssay->assayClassify == tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE)
		{
			// 检测免疫项目稀释倍数设置是否超过要求的范围
			if (!IsImDiluTionPolicy(data))
			{
				return false;
			}
		}
	}

	// 手工稀释的情况下（手工稀释的参数不能改变）
	if (m_keyStatus.dulitionType == MANUAL_SIZE && keyData.second != 1)
	{
		if (m_keyStatus.dulitions != keyData.second)
		{
			ReportErr(true, tr("同一样本添加手工稀释项目，稀释倍数应保持一致！"));
			return false;
		}
		return true;
	}

	// 自动稀释
	if (m_keyStatus.dulitionType != MANUAL_SIZE && keyData.second == 1)
	{
		return true;
	}

	ReportErr(true, tr("同一样本不能同时添加手工稀释与自动稀释项目！"));
	return false;
}

///
/// @brief 根据项目基础信息生成按钮信息
///
/// @param[in]  rawAssayInfo  项目基础信息
/// @param[in]  keyMap         位置映射关系
/// @param[in]  buttonData    项目按钮信息
///
/// @par History:
/// @li 5774/WuHongTao，2023年12月11日，新建函数
///
void QAssaySelectProcess::GenButtonInfo(const std::vector<tf::GeneralAssayPageItem>& rawAssayInfo, boost::bimap<int, int>& keyMap, std::map<int, AssayButtonData>& buttonDataMap)
{
	auto assayMgr = CommonInformationManager::GetInstance();
	for (const auto& item : rawAssayInfo)
	{
		AssayButtonData buttonData;
		auto spAssayInfo = assayMgr->GetAssayInfo(item.assayCode);
		if (Q_NULLPTR == spAssayInfo)
		{
			ULOG(LOG_WARN, "Invalid assay code :%d,", item.assayCode);
			continue;
		}

		int postion = item.positionIdx;
		keyMap.insert({ postion, item.assayCode });
		buttonData.bIsUse = true;
		buttonData.assayCode = spAssayInfo->assayCode;
		buttonData.strAssayName = QString::fromStdString(spAssayInfo->assayName);

		// ise的项目
		int assayCode = item.assayCode;
		if (assayMgr->IsAssayISE(item.assayCode))
		{
			assayCode = ::ise::tf::g_ise_constants.ASSAY_CODE_ISE;
		}

		// 血清指数
		if (assayMgr->IsAssaySIND(item.assayCode))
		{
			assayCode = ::ch::tf::g_ch_constants.ASSAY_CODE_SIND;
		}

		// 公共属性（屏蔽，校准，试剂等异常）
		auto assayStatus = QSystemMonitor::GetInstance().GetAssayStatus(assayCode);
		if (assayStatus)
		{
			buttonData.bIsCaliAbn = assayStatus->abnormalCalibrate;
			buttonData.bIsMask = assayStatus->maskAssay;
			buttonData.bIsRegentAbn = assayStatus->abnormalReagent;
		}

		buttonDataMap[item.assayCode] = buttonData;
	}
}

///
/// @brief 根据项目的状态，刷新项目card状态
///
/// @param[in]  status		项目展示状态
/// @param[in]  testItem	项目
///
/// @return true:刷新成功
///
/// @par History:
/// @li 5774/WuHongTao，2023年12月12日，新建函数
///
bool QAssaySelectProcess::MergeAssayStatus(AssayButtonData& attribute, const tf::TestItem& testItem)
{
	ULOG(LOG_INFO, "%s(assayCode: %d)", __FUNCTION__, testItem.assayCode);

	// 是否禁用状态
	attribute.bIsForbidden = IsEnableAssayCode(testItem.assayCode, m_keyStatus.sourceType);
	// 存储数据库的id
	attribute.guidList.push_back(testItem.id);
	// 复查状态
	if (testItem.__isset.rerun && testItem.rerun
		&& !(testItem.__isset.status && testItem.status == tf::TestItemStatus::TEST_ITEM_STATUS_TESTED))
	{
		attribute.bIsRequest = false;
		attribute.bIsAnalyzed = false;
		attribute.bIsRecheck = true;
	}
	// 待测试状态
	else if (testItem.__isset.status && ((testItem.status == tf::TestItemStatus::TEST_ITEM_STATUS_PENDING)
		|| (testItem.status == tf::TestItemStatus::TEST_ITEM_STATUS_TESTING)))
	{
		attribute.bIsRequest = true;
		attribute.bIsAnalyzed = false;
		attribute.bIsRecheck = false;
	}
	// 测试完毕状态
	else if (testItem.__isset.status && testItem.status == tf::TestItemStatus::TEST_ITEM_STATUS_TESTED)
	{
		attribute.bIsRequest = false;
		attribute.bIsAnalyzed = true;
		attribute.bIsRecheck = false;
	}
	else
	{
		attribute.bIsRequest = false;
		attribute.bIsAnalyzed = false;
		attribute.bIsRecheck = false;
	}

	// 稀释参数的设置
	DulitionConvert(attribute, testItem);
	return true;
}

///
/// @brief 稀释倍数的转换
///
/// @param[in]  status		项目展示状态
/// @param[in]  testItem	项目
///
/// @return true:刷新成功
///
/// @par History:
/// @li 5774/WuHongTao，2023年12月12日，新建函数
///
bool QAssaySelectProcess::DulitionConvert(AssayButtonData& attribute, const tf::TestItem& testItem)
{
	if (!testItem.__isset.preDilutionFactor)
	{
		ULOG(LOG_WARN, "please check dulition set ,testItem id : %d", testItem.id);
		return false;
	}

	// 机内稀释的时候preDilutionFacto==1
	// modify bug0011950 wuht
	if (testItem.preDilutionFactor <= 1)
	{
		if (testItem.dilutionFactor <= 1)
		{
			switch (testItem.suckVolType)
			{
				// 增量
				case tf::SuckVolType::type::SUCK_VOL_TYPE_INC:
					attribute.enSampleSize = INC_SIZE;
					break;
					// 减量
				case tf::SuckVolType::type::SUCK_VOL_TYPE_DEC:
					attribute.enSampleSize = DEC_SIZE;
					break;
					// 标准--用户自定义
				case tf::SuckVolType::type::SUCK_VOL_TYPE_STD:
					attribute.enSampleSize = STD_SIZE;
					break;
				default:
					attribute.enSampleSize = USER_SIZE;
					break;
			}
		}
		else
		{
			attribute.enSampleSize = USER_SIZE;
		}

		attribute.iDilution = testItem.dilutionFactor;
	}
	// 机外稀释
	else
	{
		attribute.iDilution = testItem.preDilutionFactor;
		attribute.enSampleSize = MANUAL_SIZE;
	}

	return true;
}

bool QAssaySelectProcess::DeleteBimap(boost::bimap<int, int>& biMap, boost::optional<int> first, boost::optional<int> second)
{
	auto deleFuntion = [&biMap](int parameter, bool isleft)->boost::optional<int>
	{
		// 删除左边的
		if (isleft)
		{
			auto iter = biMap.left.find(parameter);
			if (iter != biMap.left.end())
			{
				int code = iter->second;
				biMap.left.erase(iter);
				return code;
			}
		}
		// 删除右边的
		else
		{
			auto iter = biMap.right.find(parameter);
			if (iter != biMap.right.end())
			{
				int postion = iter->second;
				biMap.right.erase(iter);
				return postion;
			}
		}

		return boost::none;
	};

	if (!first.has_value() && !second.has_value())
	{
		return false;
	}

	if (first.has_value())
	{
		auto data = deleFuntion(first.value(), true);
		if (data.has_value())
		{
			deleFuntion(data.value() , false);
			return true;
		}
	}

	if (second.has_value())
	{
		auto data = deleFuntion(second.value(), false);
		if (data.has_value())
		{
			deleFuntion(data.value(), true);
			return true;
		}
	}

	return false;
}

QAssayChSelectProcess::QAssayChSelectProcess()
	: QAssaySelectProcess()
{
	Init();
}

void QAssayChSelectProcess::Init()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	m_singleAssays.clear();
	m_singleMaps.clear();
	m_basicAssayButtons.clear();
	m_currentButtons.clear();
	// 首先获取生化免疫的项目基础信息
	std::shared_ptr<CommonInformationManager> assayMgr = CommonInformationManager::GetInstance();
	int pageIndex = 0;
	while (pageIndex < UI_ASSAYPAGEMAX)
	{
		assayMgr->GetAssayItems(tf::AssayClassify::type::ASSAY_CLASSIFY_CHEMISTRY, pageIndex, m_singleAssays);
		assayMgr->GetAssayItems(tf::AssayClassify::type::ASSAY_CLASSIFY_ISE, pageIndex, m_singleAssays);
		pageIndex++;
	}

	// 生化单项目
	GenButtonInfo(m_singleAssays, m_singleMaps, m_basicAssayButtons);
	// 初始化
	m_currentButtons = m_basicAssayButtons;
	m_assayType = AssayDeviceType::CHEMISTRYTYPE;
}

bool QAssayChSelectProcess::IsEnableAssayCode(int assayCode, int sampleSourceType)
{
	ULOG(LOG_INFO, "%s(assayCode: %d)", __FUNCTION__, assayCode);

	// 项目位置设置的时候，都可以显示
	if (m_keyStatus.assayType == AssayTypeNew::ASSAYTYPE3_POSITION_SET)
	{
		return false;
	}

	// 若是血清指数,只支持血清血浆
	if (IsSIND(assayCode))
	{
		if (tf::SampleSourceType::SAMPLE_SOURCE_TYPE_XQXJ == sampleSourceType)
		{
			return false;
		}

		return true;
	}

	// 若是NA,K,CL，支持血清血浆和尿液
	if (IsISE(assayCode))
	{
		if (tf::SampleSourceType::SAMPLE_SOURCE_TYPE_XQXJ == sampleSourceType
			|| tf::SampleSourceType::SAMPLE_SOURCE_TYPE_NY == sampleSourceType)
		{
			return false;
		}

		return true;
	}

	auto spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(assayCode);
	if (spAssay == Q_NULLPTR)
	{
		return true;
	}

	// 获取项目对应的样本类型
	std::set<int32_t> sampleSourceTypes;
	CommonInformationManager::GetInstance()->GetSampleTypeFromCode(assayCode, sampleSourceTypes);
	if (sampleSourceTypes.count(sampleSourceType) > 0)
	{
		return false;
	}

	return true;
}

bool QAssayChSelectProcess::ProcessAssay(int assayCode, int postion)
{
	ULOG(LOG_INFO, "%s(process assayCode: %d, postion: %d)", __FUNCTION__, assayCode, postion);

	m_postion = postion;
	// 位置设置模式
	if (m_keyStatus.assayType == AssayTypeNew::ASSAYTYPE3_POSITION_SET)
	{
		// 空位置，等待项目加载
		if (assayCode == -1)
		{
			// 1：增加项目信息
			AssayButtonData selectButtonData;
			selectButtonData.bIsRequest = true;
			m_currentButtons[assayCode] = selectButtonData;
			DeleteBimap(m_singleMaps, postion, assayCode);
			m_singleMaps.insert({ postion, assayCode });
			return true;
		}
	}

	auto iter = m_currentButtons.find(assayCode);
	if (iter == m_currentButtons.end())
	{
		return false;
	}

	AssayButtonData& attribute = iter->second;
	// 若禁能，直接返回
	if (attribute.bIsForbidden)
	{
		return true;
	}

	// 是否符合稀释逻辑
	if (!IsDiluTionPolicy(attribute))
	{
		return true;
	}

	// 请求测试状态->取消测试
	if (attribute.bIsRequest)
	{
		// 取消测试
		attribute.enSampleSize = SampleSize::STD_SIZE;
		attribute.bIsRequest = false;
	}
	// 已完成状态->复查
	else if (attribute.bIsAnalyzed)
	{
		// 复查--已经审核的情况下不允许复查
		if (m_keyStatus.isAudit)
		{
			return true;
		}

		// 将本次的稀释方式赋给pre,当取消复查的时候需要返回
		attribute.preiDilution = attribute.iDilution;
		attribute.preSampleSize = attribute.enSampleSize;
		// 使用当前的稀释方式进行稀释
		attribute.enSampleSize = m_keyStatus.dulitionType;
		attribute.iDilution = m_keyStatus.dulitions;
		// 设置状态
		attribute.bIsRecheck = true;
		attribute.bIsAnalyzed = false;
	}
	// 复查状态->已完成（复查转往已完成的稀释倍数必须从结果信息中去获取）
	else if (attribute.bIsRecheck)
	{
		if (attribute.guidList.empty())
		{
			ULOG(LOG_WARN, "The attribute's testInfo id is empty ,please check it; assayCode is :%d", attribute.assayCode);
		}
		else
		{
			auto testItem = DataPrivate::Instance().GetTestItemInfoByDb(attribute.guidList[0]);
			if (testItem == Q_NULLPTR)
			{
				ULOG(LOG_WARN, "There is no testitem :%d", attribute.guidList[0]);
			}
			else
			{
				tf::TestItem testItemInfo = *testItem;
				// 当从复查恢复到已完成的时候，需要恢复到上次的稀释方式
				DataPrivate::Instance().ResetTestItemDulition(testItemInfo);
				// 转化对应关系
				DulitionConvert(attribute, testItemInfo);
				attribute.preSampleSize = attribute.enSampleSize;
				attribute.preiDilution = attribute.iDilution;
			}
		}

		attribute.enSampleSize = attribute.preSampleSize;
		attribute.iDilution = attribute.preiDilution;
		// 切换状态
		attribute.bIsAnalyzed = true;
		attribute.bIsRecheck = false;
	}
	//->请求测试
	else
	{
		// 必须增加一个新增（-1代表的是新增）
		if (attribute.guidList.empty())
		{
			attribute.guidList.push_back(-1);
		}

		attribute.enSampleSize = m_keyStatus.dulitionType;
		attribute.iDilution = m_keyStatus.dulitions;
		attribute.bIsRequest = true;
		attribute.bIsAnalyzed = false;
		attribute.bIsRecheck = false;
	}

	return true;
}

bool QAssayChSelectProcess::ProcessFromComBin(int assayCode, bool isRequest)
{
	auto iter = m_currentButtons.find(assayCode);
	if (iter == m_currentButtons.end())
	{
		return false;
	}

	auto& attribute = iter->second;
	if (isRequest)
	{
		// 若禁能，下一个
		if (attribute.bIsForbidden)
		{
			return true;
		}

		// 是否符合稀释逻辑
		if (!IsDiluTionPolicy(attribute))
		{
			return true;
		}

		if (attribute.bIsAnalyzed)
		{
			// 复查
			attribute.bIsRecheck = true;
			attribute.bIsAnalyzed = false;
			attribute.enSampleSize = m_keyStatus.dulitionType;
			attribute.iDilution = m_keyStatus.dulitions;
		}
		else if (!attribute.bIsRecheck)
		{
			// 必须增加一个新增
			if (attribute.guidList.empty())
			{
				attribute.guidList.push_back(-1);
			}

			attribute.enSampleSize = m_keyStatus.dulitionType;
			attribute.iDilution = m_keyStatus.dulitions;
			attribute.bIsRequest = true;
			attribute.bIsAnalyzed = false;
			attribute.bIsRecheck = false;
		}
	}
	// 父项目未选中
	else
	{
		if (attribute.bIsRecheck)
		{
			attribute.bIsAnalyzed = true;
			attribute.bIsRecheck = false;
			auto testItem = DataPrivate::Instance().GetTestItemInfoByDb(attribute.guidList[0]);
			if (testItem == Q_NULLPTR)
			{
				ULOG(LOG_WARN, "There is no testitem :%d", attribute.guidList[0]);
			}
			else
			{
				tf::TestItem testItemInfo = *testItem;
				// 当从复查恢复到已完成的时候，需要恢复到上次的稀释方式
				DataPrivate::Instance().ResetTestItemDulition(testItemInfo);
				// 转化对应关系
				DulitionConvert(attribute, testItemInfo);
				attribute.preSampleSize = attribute.enSampleSize;
				attribute.preiDilution = attribute.iDilution;
			}

		}
		else if (!attribute.bIsAnalyzed)
		{
			attribute.enSampleSize = SampleSize::STD_SIZE;
			attribute.bIsRequest = false;
			attribute.bIsAnalyzed = false;
			attribute.bIsRecheck = false;
		}
	}

	m_currentButtons[assayCode] = attribute;
	return true;
}

std::pair<int, int> QAssayChSelectProcess::GetSampleTestItemInfo(std::vector<::tf::TestItem>& testItems)
{
	ULOG(LOG_INFO, "%s(size: %d)", __FUNCTION__, testItems.size());
	// 获取生化的所有项目的信息
	auto chemistry = QAssaySelectProcess::GetSampleTestItemInfo(testItems, m_currentButtons);
	return chemistry;
}

QAssayImSelectProcess::QAssayImSelectProcess()
{
	Init();
}

std::pair<int, int> QAssayImSelectProcess::GetSampleTestItemInfo(std::vector<::tf::TestItem>& testItems)
{
	ULOG(LOG_INFO, "%s(size: %d)", __FUNCTION__, testItems.size());
	// 获取免疫的所有项目的信息
	auto immune = QAssaySelectProcess::GetSampleTestItemInfo(testItems, m_currentButtons);
	return immune;
}

bool QAssayImSelectProcess::ProcessAssay(int assayCode, int postion)
{
	ULOG(LOG_INFO, "%s(process assayCode: %d, postion: %d)", __FUNCTION__, assayCode, postion);
	m_postion = postion;
	// 位置设置模式
	if (m_keyStatus.assayType == AssayTypeNew::ASSAYTYPE3_POSITION_SET)
	{
		// 空位置，等待项目加载
		if (assayCode == -1)
		{
			// 1：增加项目信息
			AssayButtonData selectButtonData;
			selectButtonData.bIsRequest = true;
			m_currentButtons[assayCode] = selectButtonData;

			// 2: 位置信息
			DeleteBimap(m_singleMaps, postion, assayCode);
			m_singleMaps.insert({ postion, assayCode });

			return true;
		}
	}

	auto iter = m_currentButtons.find(assayCode);
	if (iter == m_currentButtons.end())
	{
		return false;
	}

	AssayButtonData& attribute = iter->second;
	// 若禁能，直接返回
	if (attribute.bIsForbidden)
	{
		return true;
	}

	// 是否符合稀释逻辑
	if (!IsDiluTionPolicy(attribute))
	{
		return true;
	}

	// 请求测试状态->取消测试
	if (attribute.bIsRequest)
	{
		// 取消测试
		attribute.enSampleSize = SampleSize::STD_SIZE;
		attribute.bIsRequest = false;
	}
	// 已完成状态->复查
	else if (attribute.bIsAnalyzed)
	{
		// 复查--已经审核的情况下不允许复查
		if (m_keyStatus.isAudit)
		{
			return true;
		}

		// 将本次的稀释方式赋给pre,当取消复查的时候需要返回
		attribute.preiDilution = attribute.iDilution;
		attribute.preSampleSize = attribute.enSampleSize;
		if (m_keyStatus.dulitionType == STD_SIZE)
		{
			attribute.enSampleSize = USER_SIZE;
			attribute.iDilution = GetDefaultDuliTon(attribute.assayCode);
		}
		else
		{
			attribute.enSampleSize = m_keyStatus.dulitionType;
			attribute.iDilution = m_keyStatus.dulitions;
		}

		// 设置状态
		attribute.bIsRecheck = true;
		attribute.bIsAnalyzed = false;
	}
	// 复查状态->已完成（复查转往已完成的稀释倍数必须从结果信息中去获取）
	else if (attribute.bIsRecheck)
	{
		if (attribute.guidList.empty())
		{
			ULOG(LOG_WARN, "The attribute's testInfo id is empty ,please check it; assayCode is :%d", attribute.assayCode);
		}
		else
		{
			auto testItem = DataPrivate::Instance().GetTestItemInfoByDb(attribute.guidList[0]);
			if (testItem == Q_NULLPTR)
			{
				ULOG(LOG_WARN, "There is no testitem :%d", attribute.guidList[0]);
			}
			else
			{
				tf::TestItem testItemInfo = *testItem;
				// 当从复查恢复到已完成的时候，需要恢复到上次的稀释方式
				DataPrivate::Instance().ResetTestItemDulition(testItemInfo);
				// 转化对应关系
				DulitionConvert(attribute, testItemInfo);
				attribute.preSampleSize = attribute.enSampleSize;
				attribute.preiDilution = attribute.iDilution;
			}
		}

		attribute.enSampleSize = attribute.preSampleSize;
		attribute.iDilution = attribute.preiDilution;
		// 切换状态
		attribute.bIsAnalyzed = true;
		attribute.bIsRecheck = false;
	}
	//->请求测试
	else
	{
		// 必须增加一个新增（-1代表的是新增）
		if (attribute.guidList.empty())
		{
			attribute.guidList.push_back(-1);
		}

		if (m_keyStatus.dulitionType == STD_SIZE)
		{
			attribute.enSampleSize = USER_SIZE;
			attribute.iDilution = GetDefaultDuliTon(attribute.assayCode);
		}
		else
		{
			attribute.enSampleSize = m_keyStatus.dulitionType;
			attribute.iDilution = m_keyStatus.dulitions;
		}
		attribute.bIsRequest = true;
		attribute.bIsAnalyzed = false;
		attribute.bIsRecheck = false;
	}

	return true;
}

bool QAssayImSelectProcess::ProcessFromComBin(int assayCode, bool isRequest)
{
	auto iter = m_currentButtons.find(assayCode);
	if (iter == m_currentButtons.end())
	{
		return false;
	}

	auto& attribute = iter->second;
	if (isRequest)
	{
		// 若禁能，下一个
		if (attribute.bIsForbidden)
		{
			return true;
		}

		// 是否符合稀释逻辑
		if (!IsDiluTionPolicy(attribute))
		{
			return true;
		}

		if (attribute.bIsAnalyzed)
		{
			// 复查
			attribute.bIsRecheck = true;
			attribute.bIsAnalyzed = false;
			if (m_keyStatus.dulitionType == STD_SIZE)
			{
				attribute.enSampleSize = USER_SIZE;
				attribute.iDilution = GetDefaultDuliTon(attribute.assayCode);
			}
			else
			{
				attribute.enSampleSize = m_keyStatus.dulitionType;
				attribute.iDilution = m_keyStatus.dulitions;
			}
		}
		else if (!attribute.bIsRecheck)
		{
			// 必须增加一个新增
			if (attribute.guidList.empty())
			{
				attribute.guidList.push_back(-1);
			}

			if (m_keyStatus.dulitionType == STD_SIZE)
			{
				attribute.enSampleSize = USER_SIZE;
				attribute.iDilution = GetDefaultDuliTon(attribute.assayCode);
			}
			else
			{
				attribute.enSampleSize = m_keyStatus.dulitionType;
				attribute.iDilution = m_keyStatus.dulitions;
			}
			attribute.bIsRequest = true;
			attribute.bIsAnalyzed = false;
			attribute.bIsRecheck = false;
		}
	}
	// 父项目未选中
	else
	{
		if (attribute.bIsRecheck)
		{
			attribute.bIsAnalyzed = true;
			attribute.bIsRecheck = false;
		}
		else if (!attribute.bIsAnalyzed)
		{
			attribute.enSampleSize = SampleSize::STD_SIZE;
			attribute.bIsRequest = false;
			attribute.bIsAnalyzed = false;
			attribute.bIsRecheck = false;
		}
	}

	m_currentButtons[assayCode] = attribute;
	return true;
}

void QAssayImSelectProcess::Init()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
	m_singleAssays.clear();
	m_singleMaps.clear();
	m_basicAssayButtons.clear();
	m_currentButtons.clear();
	// 首先获取生化免疫的项目基础信息
	std::shared_ptr<CommonInformationManager> assayMgr = CommonInformationManager::GetInstance();
	int pageIndex = 0;
	while (pageIndex < UI_ASSAYPAGEMAX)
	{
		assayMgr->GetAssayItems(tf::AssayClassify::type::ASSAY_CLASSIFY_IMMUNE, pageIndex, m_singleAssays);
		pageIndex++;
	}

	// 免疫单项目
	GenButtonInfo(m_singleAssays, m_singleMaps, m_basicAssayButtons);
	// 初始化
	m_currentButtons = m_basicAssayButtons;
	m_assayType = AssayDeviceType::IMMUNETYPE;
}

int QAssayImSelectProcess::GetDefaultDuliTon(int assayCode)
{
	auto spAssayInfo = CommonInformationManager::GetInstance()->GetImmuneAssayInfo(assayCode, tf::DeviceType::DEVICE_TYPE_I6000);
	if (spAssayInfo == Q_NULLPTR || spAssayInfo->reactInfo.dilutionRatio <= 0)
	{
		return 1;
	}

	return spAssayInfo->reactInfo.dilutionRatio;
}

QAssayComBinSelectProcess::QAssayComBinSelectProcess()
{
	Init();
}

void QAssayComBinSelectProcess::GetProfiles(std::vector<int64_t>& profiles)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	for (auto& rawData : m_currentButtons)
	{
		auto stuAssayBtnData = rawData.second;
		// 仅仅是展示，未使用
		if (!stuAssayBtnData.bIsUse)
		{
			continue;
		}

		// 非组合项目，下一个
		if (!stuAssayBtnData.bIsProfile)
		{
			continue;
		}

		// 非请求状态，下一个
		if (!stuAssayBtnData.bIsRequest)
		{
			continue;
		}

		profiles.push_back(stuAssayBtnData.assayCode);
	}
}

bool QAssayComBinSelectProcess::UpdateAssayProfileStatus(int64_t profiles)
{
	ULOG(LOG_INFO, "%s(assayCode: %d)", __FUNCTION__, profiles);
	auto iter = m_currentButtons.find(profiles);
	if (iter == m_currentButtons.end())
	{
		return false;
	}

	iter->second.bIsRequest = true;
	return true;
}

bool QAssayComBinSelectProcess::ProcessAssay(int assayCode, int postion)
{
	ULOG(LOG_INFO, "%s(process assayCode: %d, postion: %d)", __FUNCTION__, assayCode, postion);
	m_postion = postion;
	// 位置设置模式
	if (m_keyStatus.assayType == AssayTypeNew::ASSAYTYPE3_POSITION_SET)
	{
		// 空位置，等待项目加载
		if (assayCode == -1)
		{
			// 1：增加项目信息
			AssayButtonData selectButtonData;
			selectButtonData.bIsRequest = true;
			m_currentButtons[assayCode] = selectButtonData;

			// 2: 位置信息
			DeleteBimap(m_singleMaps, postion, assayCode);
			m_singleMaps.insert({ postion, assayCode });
			return true;
		}
	}

	auto iter = m_currentButtons.find(assayCode);
	// 若非组合项目，直接返回，不处理
	if (iter == m_currentButtons.end())
	{
		return false;
	}

	auto& attribute = iter->second;
	// 组合项目，但是没有项目（？）
	if (attribute.strSubAssayList.empty())
	{
		return true;
	}

	// modify bug0011866 by wuht(组合项目只需要一次)
	if (m_keyStatus.assayType == AssayTypeNew::ASSAYTYPE1_WORKPAGE)
	{
		if (m_keyStatus.isAudit)
		{
			TipDlg(tr("提示"), tr("样本已审核，不可修改！")).exec();
			return true;
		}

		// 生免联机的情况下，组合项目只允许选择常量或者手工稀释
		if ((m_softwareType == SOFTWARE_TYPE::CHEMISTRY_AND_IMMUNE) 
			&& m_keyStatus.dulitionType != STD_SIZE && m_keyStatus.dulitionType != MANUAL_SIZE)
		{
			TipDlg(tr("提示"), tr("组合项目只允许使用常量或者手工稀释倍数！")).exec();
			return true;
		}
	}

	if (attribute.bIsRequest)
	{
		attribute.bIsRequest = false;
	}
	else
	{
		attribute.bIsRequest = true;
	}

	// 设置组合项目的时候，不需要传递到单项目
	if (m_keyStatus.assayType == AssayTypeNew::ASSAYTYPE3_POSITION_SET)
	{
		return true;
	}

	// 点击组合项目相当于点击所有的子项目
	for (auto subAssayCode : attribute.strSubAssayList)
	{
		for (auto& process : m_assaySelectHander)
		{
			if (Q_NULLPTR == process)
			{
				continue;
			}

			// 处理完毕退出内循环
			bool finished = process->ProcessFromComBin(subAssayCode, attribute.bIsRequest);
			if (finished)
			{
				break;
			}
		}
	}

	return true;
}

void QAssayComBinSelectProcess::GenButtonCombinInfo(const std::vector<tf::ProfileAssayPageItem>& rawAssayInfo, boost::bimap<int, int>& keyMap, std::map<int, AssayButtonData>& buttonData)
{
	auto assayMgr = CommonInformationManager::GetInstance();
	if (assayMgr == Q_NULLPTR)
	{
		return;
	}

	auto ProfileAssayMaps = assayMgr->GetCommonProfileMap();
	if (!ProfileAssayMaps.has_value())
	{
		return;
	}

	// 生化的序列关系
	for (const auto& item : rawAssayInfo)
	{
		AssayButtonData assayBtnData;
		auto iter = ProfileAssayMaps.value().find(item.id);
		if (iter == ProfileAssayMaps.value().end())
		{
			continue;
		}

		if (Q_NULLPTR == iter->second)
		{
			continue;
		}
		auto assayProfile = iter->second;

		assayBtnData.bIsUse = true;
		assayBtnData.bIsProfile = true;
		// 编号和号码的生成规则
		assayBtnData.assayCode = assayProfile->id + UI_PROFILESTART;
		// 填充组合项目的代码
		assayBtnData.strSubAssayList.insert(assayBtnData.strSubAssayList.end(), item.subAssays.begin(), item.subAssays.end());
		int position = item.positionIdx;
		keyMap.insert({ position, int(assayBtnData.assayCode) });

		assayBtnData.strAssayName = QString::fromStdString(assayProfile->profileName);
		buttonData[assayBtnData.assayCode] = assayBtnData;
	}
}

void QAssayComBinSelectProcess::Init()
{
	m_combinAssays.clear();
	m_singleMaps.clear();
	m_basicAssayButtons.clear();
	m_currentButtons.clear();
	std::shared_ptr<CommonInformationManager> assayMgr = CommonInformationManager::GetInstance();
	int pageIndex = 0;
	while (pageIndex < UI_ASSAYPAGEMAX)
	{
		assayMgr->GetAssayProfiles(tf::AssayClassify::type::ASSAY_CLASSIFY_CHEMISTRY, pageIndex, m_combinAssays);
		assayMgr->GetAssayProfiles(tf::AssayClassify::type::ASSAY_CLASSIFY_ISE, pageIndex, m_combinAssays);
		assayMgr->GetAssayProfiles(tf::AssayClassify::type::ASSAY_CLASSIFY_IMMUNE, pageIndex, m_combinAssays);
		pageIndex++;
	}

	// 组合项目
	GenButtonCombinInfo(m_combinAssays, m_singleMaps, m_basicAssayButtons);
	// 初始化
	m_currentButtons = m_basicAssayButtons;
	m_assayType = AssayDeviceType::COMBINTYPE;
	m_softwareType = CommonInformationManager::GetInstance()->GetSoftWareType();
}

bool QAssayComBinSelectProcess::GenAssayButtonDataByIndex(const int pageElementCount, std::map<int, AssayButtonData>& assayDatas, int index)
{
	ULOG(LOG_INFO, "%s(index: %d, size: %d)", __FUNCTION__, index, assayDatas.size());
	int startCombin = 0;
	int endCombin = pageElementCount;

	// 组合项目显示
	for (auto& data : m_singleMaps)
	{
		if (data.left < startCombin || data.left > endCombin)
		{
			continue;
		}

		int assayCode = data.right;
		auto iter = m_currentButtons.find(assayCode);
		if (iter == m_currentButtons.end())
		{
			continue;
		}

		// 位置设置模式
		if (m_keyStatus.assayType == AssayTypeNew::ASSAYTYPE3_POSITION_SET)
		{
			if (m_postion.has_value() && m_postion.value() != data.left)
			{
				iter->second.bIsRequest = false;
			}
		}

		int indexStart = data.left - startCombin;
		assayDatas[indexStart] = iter->second;
	}

	return true;
}

