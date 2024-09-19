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
/// @file     ImDataAlarmRetestSettingsDlg.h
/// @brief    应用->项目设置->项目范围->数据报警复查
///
/// @author   8276/huchunli
/// @date     2023年8月30日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8276/huchunli，2023年8月30日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "ImDataAlarmRetestSettingsDlg.h"
#include "ui_ImDataAlarmRetestSettingsDlg.h"
#include <QStandardItemModel>
#include <QPixmap>
#include <QLabel>
#include <QComboBox>
#include "UtilityCommonFunctions.h"
#include "shared/QComDelegate.h"
#include "shared/uidef.h"
#include "shared/CommonInformationManager.h"
#include "shared/tipdlg.h"
#include "manager/ResultStatusCodeManager.h"
#include "manager/DictionaryQueryManager.h"
#include "thrift/im/ImLogicControlProxy.h"
#include "src/common/Mlog/mlog.h"
#include "shared/uicommon.h"


///
/// @bref
///		数据报警表格列序号
///
enum ColAlarm{
    ECA_Name,   // 数据报警
    ECA_Descrp, // 报警名称
    ECA_Dilu,   // 复查稀释倍数
    ECA_Use     // 启用
};

DataAlarmItem::DataAlarmItem(int alarmId, const std::string& name, const QString& decrip)
{
    m_alarmId = alarmId;
    m_alarmName = name;
    m_description = decrip;
    m_beUseCheck = nullptr;
    m_diluRateComb = nullptr;
}

DataAlarmItem::DataAlarmItem()
{
    m_alarmId = -1;
    m_rowIndex = -1;
    m_beUseCheck = nullptr;
    m_diluRateComb = nullptr;
}

ImDataAlarmRetestSettingsDlg::ImDataAlarmRetestSettingsDlg(QWidget *parent)
    : BaseDlg(parent)
{
    ui = new Ui::ImDataAlarmRetestSettingsDlg();
    ui->setupUi(this);

    SetTitleName(tr("数据报警复查"));
    m_selectedImg = new QPixmap(":/Leonis/resource/image/icon-select.png");

    m_model = new QStandardItemModel();
    ui->tableViewAlarmRetestSettings->setModel(m_model);
    ui->tableViewAlarmRetestSettings->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableViewAlarmRetestSettings->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableViewAlarmRetestSettings->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableViewAlarmRetestSettings->verticalHeader()->setVisible(false); 
    ui->tableViewAlarmRetestSettings->horizontalHeader()->setHighlightSections(false);

    // table head
    QStringList  listTitle = { tr("数据报警"), tr("描述"), tr("复查稀释倍数"), tr("启用") };
    m_model->setHorizontalHeaderLabels(listTitle);
    ui->tableViewAlarmRetestSettings->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableViewAlarmRetestSettings->horizontalHeader()->setSectionResizeMode(ECA_Descrp, QHeaderView::Fixed);
    ui->tableViewAlarmRetestSettings->setColumnWidth(ECA_Descrp, 670);
    ui->tableViewAlarmRetestSettings->verticalHeader()->setDefaultSectionSize(46);
    m_model->setHeaderData(ECA_Use, Qt::Horizontal, UI_TEXT_COLOR_HEADER_MARK, Qt::ForegroundRole);

    // 初始化报警项
    InitAlarmItems();
    InitAlarmTable();

    // 初始化事件
    connect(ui->tableViewAlarmRetestSettings, SIGNAL(clicked(const QModelIndex&)), this, SLOT(OnTableViewClicked(const QModelIndex&)));
    // 确定按钮
    connect(ui->pushButtonOk, SIGNAL(clicked()), this, SLOT(OnClickedOk()));
}

ImDataAlarmRetestSettingsDlg::~ImDataAlarmRetestSettingsDlg()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);
    delete ui;
}

bool ImDataAlarmRetestSettingsDlg::InitData(const std::string& ver, int assayCode)
{
    ULOG(LOG_INFO, "%s(%s, %d).", __FUNCTION__, ver, assayCode);

    m_assayCode = assayCode;
    m_paramVersion = ver;

    auto pImInfo = CommonInformationManager::GetInstance()->GetImmuneAssayInfo(assayCode, (::tf::DeviceType::DEVICE_TYPE_I6000), ver);
    if (pImInfo == nullptr)
    {
        ULOG(LOG_WARN, "Null im assayinfo.");
        return false;
    }

    // 复位界面
    for (auto& pr : m_alarmItem)
    {
        std::shared_ptr<DataAlarmItem>& curItem = pr.second;
        if (curItem == nullptr)
        {
            continue;
        }
        if (curItem->m_diluRateComb)
        {
            curItem->m_diluRateComb->setCurrentText(QString::number(pImInfo->reactInfo.dilutionRatio));
        }
        if (curItem->m_beUseCheck)
        {
            curItem->m_beUseCheck->setHidden(true);
        }
    }

    // 赋值当前数据
    for (const auto& aIter : pImInfo->rerunSet.alrams)
    {
        std::map<std::string, std::shared_ptr<DataAlarmItem>>::iterator it = m_alarmItem.find(aIter.alarmCode);
        if (it != m_alarmItem.end() && it->second != nullptr)
        {
            std::shared_ptr<DataAlarmItem>& curItem = it->second;
            curItem->m_diluRateComb->setCurrentText(QString::number(aIter.ratio));
            if (curItem->m_beUseCheck)
            {
                curItem->m_beUseCheck->setHidden(false);
            }
            else
            {
                CreateCheck(m_model->index(curItem->m_rowIndex, ECA_Use));
            }
        }
    }

    return true;
}

void ImDataAlarmRetestSettingsDlg::InitAlarmItems()
{
    std::vector<tf::DataAlarmItem> imDataAlarm;
    ResultStatusCodeManager::GetInstance()->GetImResultStatus(imDataAlarm);

    // 不是数据报警的码
    std::set<std::string> exceptCode = DictionaryQueryManager::GetInstance()->GetImAlaramDataCodeExcept();

    int ararmId = 0;
    for (const tf::DataAlarmItem& di : imDataAlarm)
    {
        if (exceptCode.find(di.code) != exceptCode.end())
        {
            continue;
        }
        m_alarmItem[di.code] = std::make_shared<DataAlarmItem>(ararmId++, di.code, QString::fromStdString(di.name));
    }
}

void ImDataAlarmRetestSettingsDlg::InitAlarmTable()
{
    // 稀释倍率下拉框
    QStringList duliRatios = {""}; 
    duliRatios.append(utilcomm::GetDuliRatioListIm());

    std::map<int, std::shared_ptr<DataAlarmItem>> sorteDt;
    std::map<std::string, std::shared_ptr<DataAlarmItem>>::iterator it = m_alarmItem.begin();
    for (; it != m_alarmItem.end(); ++it)
    {
        if (it->second == nullptr)
        {
             continue;
        }
        sorteDt.insert(std::pair<int, std::shared_ptr<DataAlarmItem>>(it->second->m_alarmId, it->second));
    }
    int iRow = 0;
    for (const auto& item : sorteDt)
    {
        const std::shared_ptr<DataAlarmItem>& curItem = item.second;
        QList<QStandardItem*> rowCells;

        QStandardItem* qAlarmName = new QStandardItem(QString::fromStdString(curItem->m_alarmName));
        qAlarmName->setTextAlignment(Qt::AlignCenter);
        rowCells.append(qAlarmName);

        rowCells.append(new QStandardItem(curItem->m_description));
        rowCells.append(new QStandardItem(QStringLiteral("")));
        rowCells.append(new QStandardItem(QStringLiteral("")));

        m_model->appendRow(rowCells);

        // 稀释倍率
        CenterComboBox *pCombDilu = new CenterComboBox(ui->tableViewAlarmRetestSettings);
        pCombDilu->addItems(duliRatios);
        ui->tableViewAlarmRetestSettings->setIndexWidget(m_model->index(iRow, ECA_Dilu), pCombDilu);
        pCombDilu->installEventFilter(this);
        curItem->m_diluRateComb = pCombDilu;
        curItem->m_rowIndex = iRow;

        iRow++;
    }
}

void ImDataAlarmRetestSettingsDlg::CreateCheck(const QModelIndex& qIndex)
{
    QLabel *imgLable = new QLabel(ui->tableViewAlarmRetestSettings);
    imgLable->setPixmap(*m_selectedImg);
    imgLable->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);
    ui->tableViewAlarmRetestSettings->setIndexWidget(qIndex, imgLable);

    QModelIndex nameIndex = m_model->index(qIndex.row(), ECA_Name);
    if (nameIndex.isValid())
    {
        auto it = m_alarmItem.find(nameIndex.data().toString().toStdString());
        if (it != m_alarmItem.end())
        {
            std::shared_ptr<DataAlarmItem>& curItem = it->second;
            if (curItem != nullptr)
            {
                curItem->m_beUseCheck = imgLable;
            }
        }
    }
}

///
/// @brief 添加报警复查的操作日志
///
/// @param[in]  alarmCfgNew  新设置的报警复查信息
/// @param[in]  spImAssayInfo  项目中保存的报警复查信息
/// @param[in]  strAlarmOptLog  操作日志
///
/// @return 
///
/// @par History:
/// @li 1556/Chenjianlin，2024年3月14日，新建函数
///
void ImDataAlarmRetestSettingsDlg::GetImAlarmAssayOptLog(std::vector<im::tf::autoRerunByAlarm> alarmCfgNew, const std::shared_ptr<im::tf::GeneralAssayInfo>& spImAssayInfo, QString& strAlarmOptLog)
{
	strAlarmOptLog.clear();
	if (nullptr == spImAssayInfo)
	{
		ULOG(LOG_ERROR, "The input param is nullptr.");
		return;
	}
	auto spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(spImAssayInfo->assayCode);
	if (nullptr == spAssay)
	{
		ULOG(LOG_ERROR, "The assay information is nullptr.");
		return;
	}
	QString strOptLog(""), strAlaram(tr("数据报警")), strRatio(tr("复查稀释倍数")), strCheck(tr("勾选")), strUncheck(tr("取消勾选"));
	for (auto& atVal : spImAssayInfo->rerunSet.alrams)
	{
		bool bFind = false;
		for (auto atValUi = alarmCfgNew.begin(); atValUi!=alarmCfgNew.end(); ++atValUi)
		{
			if (atVal.alarmCode != atValUi->alarmCode)
			{
				continue;
			}
			// 检查稀释倍数
			if (atVal.ratio != atValUi->ratio)
			{
				strOptLog += "{" + strAlaram + ":" + QString::fromStdString(atVal.alarmCode) + " " + strRatio + ":" + QString::number(atValUi->ratio) + " " + strCheck + "}";
			}
			bFind = true;
			atValUi = alarmCfgNew.erase(atValUi);
			break;
		}
		// 如果没有找到，说明关闭了
		if (!bFind && atVal.ratio > 0)
		{
			strOptLog += "{" + strAlaram + ":" + QString::fromStdString(atVal.alarmCode) + " " + strRatio + ":" + QString::number(atVal.ratio) + " " + strUncheck + "}";
		}
	}

	for (auto atValUi = alarmCfgNew.begin(); atValUi != alarmCfgNew.end(); ++atValUi)
	{
		// 如果添加了稀释倍数
		if (atValUi->ratio > 0)
		{
			strOptLog += "{" + strAlaram + ":" + QString::fromStdString(atValUi->alarmCode) + " " + strRatio + ":" + QString::number(atValUi->ratio) + " " + strCheck + "}";
		}
	}
	if (!strOptLog.isEmpty())
	{
		strAlarmOptLog = QString::fromStdString(spAssay->assayName) + GetTitleName() + ":" + strOptLog;
	}
}

///
/// @brief 项目报警信息勾选有是否有变更
///
/// @param[in]  alarmCfgNew  界面勾选信息
/// @param[in]  spImAssayInfo  项目信息
///
/// @return 
///
/// @par History:
/// @li 1556/Chenjianlin，2024年3月15日，新建函数
///
bool ImDataAlarmRetestSettingsDlg::IsChangeAlarmInfo(const std::vector<im::tf::autoRerunByAlarm>& alarmCfgNew, const std::shared_ptr<im::tf::GeneralAssayInfo>& spImAssayInfo)
{
	if (nullptr == spImAssayInfo)
	{
		ULOG(LOG_ERROR, "The input param is nullptr.");
		return false;
	}
	// 数量不一致，一定有变更
	if (spImAssayInfo->rerunSet.alrams.size() != alarmCfgNew.size())
	{
		return true;
	}
	bool bChange = false;
	for (auto& atVal : spImAssayInfo->rerunSet.alrams)
	{
		bool bFind = false;
		for (auto& atValUi : alarmCfgNew)
		{
			if (atVal.alarmCode != atValUi.alarmCode)
			{
				continue;
			}
			bFind = true;
			if (atVal.ratio != atValUi.ratio)
			{
				bChange = true;
				break;
			}
		}
		// 如果没有找到 或 已知有变更
		if (!bFind || bChange)
		{
			bChange = true;
			break;
		}
	}
	return bChange;
}

void ImDataAlarmRetestSettingsDlg::OnTableViewClicked(const QModelIndex& qIndex)
{
    if (qIndex.column() == ECA_Use)
    {
        QWidget *qWidget = ui->tableViewAlarmRetestSettings->indexWidget(qIndex);
        if (qWidget != Q_NULLPTR)
        {
            qWidget->setHidden(!qWidget->isHidden());
        }
        else
        {
            // 勾选框
            CreateCheck(qIndex);
        }
    }
}

void ImDataAlarmRetestSettingsDlg::OnClickedOk()
{
    ULOG(LOG_INFO, __FUNCTION__);

	std::shared_ptr<CommonInformationManager> &assayMgr = CommonInformationManager::GetInstance();
    auto pImInfo = assayMgr->GetImmuneAssayInfo(m_assayCode, (::tf::DeviceType::DEVICE_TYPE_I6000), m_paramVersion);
    if (pImInfo == nullptr)
    {
        ULOG(LOG_WARN, "Null im assayinfo, %d, ver:%s.", m_assayCode, m_paramVersion.c_str());
        return;
    }

    std::vector<im::tf::autoRerunByAlarm> alarmCfg;
    for (const auto& aItem : m_alarmItem)
    {
        const std::shared_ptr<DataAlarmItem>& curItem = aItem.second;
        if (curItem == nullptr)
        {
            continue;
        }
        if (curItem->m_beUseCheck && curItem->m_beUseCheck->isVisible())
        {
            im::tf::autoRerunByAlarm alarmItem;
            alarmItem.__set_alarmCode(aItem.first);
            alarmItem.__set_ratio(curItem->m_diluRateComb->currentText().toInt());

            if (alarmItem.ratio > pImInfo->reactInfo.maxDilutionRatio)
            {
                TipDlg(QString::fromStdString(aItem.first) + tr("值不合法，稀释倍数不能大于最大稀释倍数!")).exec();
                return;
            }
            if (alarmItem.ratio < pImInfo->reactInfo.dilutionRatio)
            {
                TipDlg(QString::fromStdString(aItem.first) + tr("值不合法，稀释倍数不能小于默认稀释倍数!")).exec();
                return;
            }
            alarmCfg.push_back(alarmItem);
        }
    }

	// 检查是否有变更
	if (IsChangeAlarmInfo(alarmCfg, pImInfo))
    {
		// 获取报警复查的操作日志
		QString strAlarmOptLog;
		GetImAlarmAssayOptLog(alarmCfg, pImInfo, strAlarmOptLog);

		// 更新设置到数据库和内存（解决多版本同步、全部取消未更新的问题）
		std::vector<std::shared_ptr<im::tf::GeneralAssayInfo>> vecImAssayInfo;		
		assayMgr->GetImmuneAssayInfos(vecImAssayInfo, m_assayCode, ::tf::DeviceType::DEVICE_TYPE_I6000);
		bool bSaveFlag = true;
		for (auto& atImAssay : vecImAssayInfo)
		{
			im::tf::GeneralAssayInfo tempInfo = *atImAssay;
			tempInfo.rerunSet.__set_alrams(alarmCfg);
			tempInfo.__set_rerunSet(tempInfo.rerunSet);
			// 更新内存中的项目设置信息
			atImAssay->__set_rerunSet(tempInfo.rerunSet);
			// 保存数据库
			if (!im::LogicControlProxy::ModifyGeneralAssayInfo(tempInfo))
			{
				ULOG(LOG_ERROR, "Failed to save autoRerun config to im generalAssayinfo.");
				TipDlg(tr("保存失败，请重试!")).exec();
				bSaveFlag = false;
			}
		}
		// 如果保存成功，添加操作日志
		if (bSaveFlag)
		{
			AddOptLog(::tf::OperationType::MOD, strAlarmOptLog);
		}
    }
    this->accept();
}

bool ImDataAlarmRetestSettingsDlg::eventFilter(QObject *obj, QEvent *event)
{
    if (obj->inherits("QComboBox") && event->type() == QEvent::Wheel)
    {
        return true;
    }

    return BaseDlg::eventFilter(obj, event);
}
