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
/// @file     QAutoMaintainCfgDlg.h
/// @brief 	  自动维护设置
///
/// @author   7656/zhang.changjiang
/// @date      2023年2月27日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年2月27日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QAutoMaintainCfgDlg.h"
#include "ui_QAutoMaintainCfgDlg.h"
#include <set>
#include <QHeaderView>
#include <QComboBox>
#include <QPushButton>
#include <QLineEdit>
#include <QHBoxLayout>
#include <QStandardItemModel>

#include "model/MaintainGroupModel.h"
#include "shared/CommonInformationManager.h"
#include "shared/uidef.h"
#include "shared/tipdlg.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "thrift/DcsControlProxy.h"
#include "utility/QCustomCard.h"
#include "manager/UserInfoManager.h"
#include "manager/DictionaryQueryManager.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/TimeUtil.h"
#include "QMaintainSetPage.h"


#define WEEK_ROW_MAX (7) // 7周时间
#define WEEK_DAY_CNT (7) // 一周7天
#define MAX_NUM_ROW  (12) // 最大行数
#define MAC_COL 4 // 右侧时间设置列数

QAutoMaintainCfgDlg::QAutoMaintainCfgDlg(QWidget *parent)
    : BaseDlg(parent)
{
    ui = new Ui::QAutoMaintainCfgDlg();
    ui->setupUi(this);
    Init();
}

QAutoMaintainCfgDlg::~QAutoMaintainCfgDlg()
{
}

///
///  @brief 初始化
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年2月27日，新建函数
///
void QAutoMaintainCfgDlg::Init()
{
    m_pMaintainGroupModel = new QAutoMaintainFilterProxyModel();
    ui->tableView_mantain_group->setModel(m_pMaintainGroupModel);
    ui->tableView_mantain_group->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView_mantain_group->verticalHeader()->hide();
    ui->tableView_mantain_group->setMouseTracking(true);
    ui->tableView_mantain_group->horizontalHeader()->setTextElideMode(Qt::ElideRight);
    ui->tableView_mantain_group->setItemDelegateForColumn(1, new QMaintItemTableDelegate());

    SetTitleName(tr("自动维护设置"));
    ui->tableView_mantain_group->hideColumn(0);
    MaintainGroupModel::Instance().SetmHeaderNames({ tr("序号"), tr("维护项目") });

    m_autoMaintModel = new QStandardItemModel();
    ui->tableViewAutoMaint->setModel(m_autoMaintModel);
    QStringList headerLable;
    headerLable << tr("仪器") << tr("间隔时间（小时）") << tr("自动维护时间") << "";
    m_autoMaintModel->setHorizontalHeaderLabels(headerLable);
    ui->tableViewAutoMaint->verticalHeader()->setVisible(false);
    for (int i = 0; i < headerLable.size(); ++i)
    {
        m_autoMaintModel->setHeaderData(i, Qt::Horizontal, UI_TEXT_COLOR_HEADER_MARK, Qt::ForegroundRole);
    }
    ui->tableViewAutoMaint->setColumnWidth(2, 460);
    ui->tableViewAutoMaint->setSelectionBehavior(QAbstractItemView::SelectRows);

    // 设置各个列宽度，最后一列自动延伸
    ui->tableViewAutoMaint->setColumnWidth(DEVICE_COL, 130);
    ui->tableViewAutoMaint->setColumnWidth(TIME_INTERVAL_COL, 150);
	ui->tableViewAutoMaint->horizontalHeader()->setSectionResizeMode(TIME_INTERVAL_COL, QHeaderView::Fixed);
    ui->tableViewAutoMaint->setColumnWidth(AUTO_TIME_COL, 555);
	ui->tableViewAutoMaint->horizontalHeader()->setSectionResizeMode(AUTO_TIME_COL, QHeaderView::Fixed);
	ui->tableViewAutoMaint->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
	ui->tableViewAutoMaint->horizontalHeader()->setMinimumSectionSize(100);

    // 初始化时间设置界面
    m_rowsCtrl.resize(MAX_NUM_ROW);
    for (int i = 0; i < MAX_NUM_ROW; ++i)
    {
        UiRowCtrl* currentRowCtrls = new UiRowCtrl(i, ui->tableViewAutoMaint);
        InitRowContentUi(i, currentRowCtrls);
        m_rowsCtrl[i] = currentRowCtrls;
    }

    ConnectSlots();
    ui->tableView_mantain_group->selectRow(0);

    // 缓存设置前的数据
    m_origData = MaintainGroupModel::Instance().GetData();

    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);
}

///
///  @brief 连接信号槽
///
///  @par History: 
///  @li 7656/zhang.changjiang，2023年2月27日，新建函数
///
void QAutoMaintainCfgDlg::ConnectSlots()
{
    // 确认按钮
    connect(ui->okBtn, &QPushButton::clicked, this, &QAutoMaintainCfgDlg::OnOkBtnClicked);

    // 取消按钮
    connect(ui->cancerBtn, &QPushButton::clicked, this, &QAutoMaintainCfgDlg::OnCancerBtnClicked);

    // 组合维护名的选择
    connect(ui->tableView_mantain_group->selectionModel(), &QItemSelectionModel::currentChanged, this, &QAutoMaintainCfgDlg::OnCurrentChanged);
}

///
///  @brief 确认按钮被点击
///
void QAutoMaintainCfgDlg::OnOkBtnClicked()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 缓存当前行的变化
    QModelIndex idxMode = ui->tableView_mantain_group->currentIndex();
    CacheCfgFromUI(idxMode);

    // 检查合法性
    if (!CheckWeekValid())
    {
        ULOG(LOG_WARN, "Invalid auto maintance settings.");
        return;
    }

    // 对所有变化的配置进行存储
    std::map<QString, std::vector<tf::AutoMaintainCfg>>::iterator it = m_changedCfgs.begin();
    for (; it != m_changedCfgs.end(); ++it)
    {
        for (const tf::MaintainGroup& origItem : m_origData)
        {
            // 如果存在变化的配置项，则进行保存
            if (MaintainGroupModel::GetGroupName(origItem) == it->first && !CompareAutoMaintenCfg(origItem.autoMaintainCfgs, it->second))
            {
                tf::MaintainGroup temp = origItem;
                // 周维护不更新设置时间
                if (origItem.groupType == tf::MaintainGroupType::type::MAINTAIN_GROUP_WEEK)
                {
                    MaintainGroupModel::Instance().UpdateGroupCfgWithNoSetTimeChanged(temp, it->second);
                }
                else
                {
                    temp.__set_autoMaintainCfgs(it->second);
                }

                if (!DcsControlProxy::GetInstance()->ModifyMaintainGroups({ temp }))
                {
                    ULOG(LOG_ERROR, "Faild to modify auto maintain config.");

                    // 如果保存失败，则还原单例中的修改
                    MaintainGroupModel::Instance().UpdateAutoMantains(origItem.groupName, origItem.autoMaintainCfgs);
                }
            }
        }
    }

    this->accept();
}

///
///  @brief 取消按钮被点击
///
void QAutoMaintainCfgDlg::OnCancerBtnClicked()
{
    ULOG(LOG_INFO, __FUNCTION__);

    MaintainGroupModel::Instance().SetData(m_origData);
    this->close();
}

void QAutoMaintainCfgDlg::OnCurrentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 缓存上一条修改 
    if (previous.isValid())
    {
        CacheCfgFromUI(previous);
    }

    QModelIndex dataModel = m_pMaintainGroupModel->index(current.row(), 1);
    if (dataModel.isValid())
    {
        QString groupName = dataModel.data().toString();
        tf::MaintainGroup groupItem;
        if (!MaintainGroupModel::Instance().TakeMaintainGroup(groupName, groupItem))
        {
            ULOG(LOG_ERROR, "Not matched any groupItem, %s.", groupName.toStdString().c_str());
            //return; // not return due to next clear.
        }
        UpdateView(groupItem, groupItem.autoMaintainCfgs);
    }
    // 滚动页面，使选中行可见
    ui->tableView_mantain_group->scrollTo(dataModel);
}

void QAutoMaintainCfgDlg::OnPermisionChanged()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    std::shared_ptr<UserInfoManager> userPms = UserInfoManager::GetInstance();
    bool isPrmAuto = userPms->IsPermisson(PSM_IM_MANT_AUTOMAINTSET);

    ui->tableViewAutoMaint->setEnabled(isPrmAuto);
    ui->okBtn->setEnabled(isPrmAuto);
}

void QAutoMaintainCfgDlg::showEvent(QShowEvent *event)
{
    m_origData = MaintainGroupModel::Instance().GetData();
    ui->tableView_mantain_group->clearSelection();
    ui->tableView_mantain_group->selectRow(0);
}

void QAutoMaintainCfgDlg::InitRowContentUi(int currRowIdx, UiRowCtrl* rowCtrl)
{
	// 参数判空
	if (rowCtrl == nullptr )
	{
		ULOG(LOG_ERROR, "%s()::Null UiRowCtrl", __FUNCTION__);
		return;
	}

    // 填充一行的空白
    QList<QStandardItem*> rowItems;
    for (unsigned int ridx = 0; ridx < MAC_COL; ++ridx)
    {
        rowItems.append(new QStandardItem(""));
    }
    m_autoMaintModel->insertRow(currRowIdx, rowItems);

    // 初始化"仪器"列
    QWidget* pCombDevLytWgt = new QWidget(ui->tableViewAutoMaint);
    QHBoxLayout* pCombDevLyt = new QHBoxLayout(pCombDevLytWgt);
    pCombDevLyt->setMargin(0);
    pCombDevLyt->setSpacing(0);
    rowCtrl->m_devComb = new QComboBox(ui->tableViewAutoMaint);
	rowCtrl->m_devComb->setMinimumSize(70, 34);

    pCombDevLyt->addWidget(rowCtrl->m_devComb);
    ui->tableViewAutoMaint->setIndexWidget(m_autoMaintModel->index(currRowIdx, DEVICE_COL), pCombDevLytWgt);

    // 初始化"间隔时间（小时）"列
    QWidget* pEditIntervLytWgt = new QWidget(ui->tableViewAutoMaint);
    QHBoxLayout* pEditIntervLyt = new QHBoxLayout(pEditIntervLytWgt);
    pEditIntervLyt->setMargin(0);
    pEditIntervLyt->setSpacing(0);
    rowCtrl->m_intervEdit = new QLineEdit(ui->tableViewAutoMaint);
    rowCtrl->m_intervEdit->setFixedSize(120, 34);
#ifndef _DEBUG
    rowCtrl->m_intervEdit->setValidator(new QRegExpValidator(QRegExp(UI_REG_SAMPLE_NUM)));
#endif
    pEditIntervLyt->addWidget(rowCtrl->m_intervEdit);
    ui->tableViewAutoMaint->setIndexWidget(m_autoMaintModel->index(currRowIdx, TIME_INTERVAL_COL), pEditIntervLytWgt);

    // 初始化"自动维护时间"列
    rowCtrl->m_cardTime = new QCustomCard(ui->tableViewAutoMaint);
    ui->tableViewAutoMaint->setIndexWidget(m_autoMaintModel->index(currRowIdx, AUTO_TIME_COL), rowCtrl->m_cardTime);

    // 初始化清空按钮列
    QWidget* pClearBtnLytWgt = new QWidget(ui->tableViewAutoMaint);
    QHBoxLayout* pClearBtnLyt = new QHBoxLayout(pClearBtnLytWgt);
    pClearBtnLyt->setMargin(0);
    pClearBtnLyt->setSpacing(0);
    QString strBtText = tr("清空");
	rowCtrl->m_clearBtn = new QPushButton(strBtText, ui->tableViewAutoMaint);
    rowCtrl->m_clearBtn->setProperty("buttons", "btn-common-dlg");
    pClearBtnLyt->addWidget(rowCtrl->m_clearBtn, Qt::AlignJustify);
    ui->tableViewAutoMaint->setIndexWidget(m_autoMaintModel->index(currRowIdx, CLEAR_COL), pClearBtnLytWgt);

    rowCtrl->m_clearBtn->setToolTip(strBtText);
    QFontMetrics elidFont(rowCtrl->m_clearBtn->font());
    rowCtrl->m_clearBtn->setText(elidFont.elidedText(strBtText, Qt::ElideRight, rowCtrl->m_clearBtn->width() - 5));

	// 初始化连接
	rowCtrl->InitConnect();
}

void QAutoMaintainCfgDlg::CacheChangedCfg(const QString& groupName, const std::vector<tf::AutoMaintainCfg>& cfg)
{
    std::map<QString, std::vector<tf::AutoMaintainCfg>>::iterator it = m_changedCfgs.find(groupName);
    if (it != m_changedCfgs.end())
    {
        // 如果存在变化，则缓存起来
        if (!CompareAutoMaintenCfg(cfg, it->second))
        {
            it->second = cfg;
        }
    }
    else
    {
        m_changedCfgs[groupName] = cfg;
    }
}

void QAutoMaintainCfgDlg::CacheCfgFromUI(const QModelIndex& rowIdx)
{
    if (!rowIdx.isValid())
    {
        ULOG(LOG_WARN, "Invalid modelindex in cachecfgfromui.");
        return;
    }

    QModelIndex dataModel = m_pMaintainGroupModel->index(rowIdx.row(), 1);
    if (dataModel.isValid())
    {
        QString preName = dataModel.data().toString();

        std::vector<tf::AutoMaintainCfg> currentCfg;
        GetMaintainUiCfg(currentCfg);
        CacheChangedCfg(preName, currentCfg);
        MaintainGroupModel::Instance().UpdateAutoMantains(preName.toStdString(), currentCfg);
    }
}

bool QAutoMaintainCfgDlg::CompareAutoMaintenCfg(const std::vector<tf::AutoMaintainCfg>& newCfg, const std::vector<tf::AutoMaintainCfg>& oldCfg)
{
    if (newCfg.size() != oldCfg.size())
    {
        return false;
    }
    for (int i = 0; i < newCfg.size(); ++i)
    {
        if (newCfg[i].deviceSN != oldCfg[i].deviceSN || \
            newCfg[i].timeInterval != oldCfg[i].timeInterval || \
            newCfg[i].maintainTime != oldCfg[i].maintainTime || \
            newCfg[i].lstWeekdays.size() != oldCfg[i].lstWeekdays.size())
        {
            return false;
        }
        const std::vector<int> &newWeek = newCfg[i].lstWeekdays;
        const std::vector<int> &oldWeek = oldCfg[i].lstWeekdays;
        for (int iw = 0; iw < newWeek.size(); ++iw)
        {
            if (newWeek[iw] != oldWeek[iw])
            {
                return false;
            }
        }
    }

    return true;
}

bool QAutoMaintainCfgDlg::CheckWeekValid()
{
    std::map<QString, std::vector<tf::AutoMaintainCfg>>::iterator it = m_changedCfgs.begin();
    for (; it != m_changedCfgs.end(); ++it)
    {
        for (const tf::AutoMaintainCfg& item : it->second)
        {
            // 选择时间同时必须选择日期
            if (item.lstWeekdays.size() == 0 && item.maintainTime != "00:00:00")
            {
                TipDlg(tr("【%1】设置的值不合法，选择了时间时需要选择周！").arg(it->first)).exec();
                return false;
            }
            if (item.deviceSN.empty() && (item.lstWeekdays.size() != 0 || !item.timeInterval.empty()))
            {
                TipDlg(tr("【%1】设置的值不合法，选择了时间时需要选择设备！").arg(it->first)).exec();
                return false;
            }
			if (!item.deviceSN.empty() && (item.lstWeekdays.size() == 0 && item.timeInterval.empty()))
			{
				TipDlg(tr("【%1】设置的值不合法，选择了设备时需要设置间隔时间或自动维护时间！").arg(it->first)).exec();
				return false;
			}
        }
    }

    // 检查周是否存在交集
    auto funcIntersection = [](std::vector<std::shared_ptr<tf::AutoMaintainCfg>>& vecCfg) ->bool {
        std::set<int> uniquWeek;
        for (const std::shared_ptr<tf::AutoMaintainCfg>& item : vecCfg)
        {
            if (item == nullptr)
            {
                continue;
            }
            for (int vi : item->lstWeekdays)
            {
                if (uniquWeek.find(vi) != uniquWeek.end())
                {
                    return true;
                }
                uniquWeek.insert(vi);
            }
        }
        return false;
    };

    // 检查不同维护组，相同设备号 是否存在相同的自动维护时间
    std::map<std::string, std::vector<std::shared_ptr<tf::AutoMaintainCfg>>> uniqDevTime; // Dev+Time
    std::vector<tf::MaintainGroup> mgrop = MaintainGroupModel::Instance().GetData();
    for (const tf::MaintainGroup& gItem : mgrop)
    {
        for (const tf::AutoMaintainCfg& cfg : gItem.autoMaintainCfgs)
        {
            if (cfg.lstWeekdays.empty())
            {
                continue;
            }
            std::string strKey = cfg.deviceSN + cfg.maintainTime;
            std::map<std::string, std::vector<std::shared_ptr<tf::AutoMaintainCfg>>>::iterator fIter = uniqDevTime.find(strKey);
            if (fIter != uniqDevTime.end())
            {
                fIter->second.push_back(std::make_shared<tf::AutoMaintainCfg>(cfg));

                if (funcIntersection(fIter->second))
                {
                    TipDlg(tr("多个自动维护的设置不能设置为在同一仪器同时执行，若要执行多个维护请自定义设置组合维护！")).exec();
                    return false;
                }
                continue;
            }
            uniqDevTime[strKey] = { std::make_shared<tf::AutoMaintainCfg>(cfg) };
        }
    }

    return true;
}

// 清除重复的配置项
void QAutoMaintainCfgDlg::ClearRepeatMaintainCfg(const std::vector<tf::AutoMaintainCfg>& maintanCfg, std::vector<tf::AutoMaintainCfg>& noRepeatCfg)
{
    auto funcEqualVec = [](const std::vector<int>& src, const std::vector<int>&dst) -> bool{
        if (src.size() != dst.size())
        {
            return false;
        }
        for (int i=0; i<src.size(); ++i)
        {
            if (src[i] != dst[i])
            {
                return false;
            }
        }
        return true;
    };

    for (const tf::AutoMaintainCfg& origItem : maintanCfg)
    {
        bool existFlag = false;
        for (const tf::AutoMaintainCfg& newItem : noRepeatCfg)
        {
            if (newItem.deviceSN == origItem.deviceSN && 
                newItem.maintainTime == origItem.maintainTime &&
                newItem.timeInterval == origItem.timeInterval &&
                funcEqualVec(newItem.lstWeekdays, origItem.lstWeekdays))
            {
                existFlag = true;
                break;
            }
        }
        if (!existFlag)
        {
            noRepeatCfg.push_back(origItem);
        }
    }
}

void QAutoMaintainCfgDlg::UpdateView(const tf::MaintainGroup& currentGroup, const std::vector<tf::AutoMaintainCfg>& autoMaintItems)
{
    ULOG(LOG_INFO, "%s, groupId:%lld.", __FUNCTION__, currentGroup.id);

    auto devMap = CommonInformationManager::GetInstance()->GetDeviceMaps();
    if (devMap.size() == 0)
    {
        ULOG(LOG_WARN, "Empty device map.");
        return;
    }
    bool isPip = DictionaryQueryManager::GetInstance()->GetPipeLine();

    // 判断设备是否运行显示出来
    auto funcInvalidDev = [&](const std::string& devSn) ->bool {
        if (devSn.empty())
        {
            return true; // 当设备SN为空，则这个值来自于界面只设置了时间，还未设置设备的行
        }
        for (const auto& dev : devMap)
        {
            if (dev.second->deviceSN == devSn)
            {
                if (dev.second->deviceType == tf::DeviceType::DEVICE_TYPE_INVALID ||
                    (!isPip && dev.second->deviceType == tf::DeviceType::DEVICE_TYPE_TRACK))
                {
                    return false;
                }
                return true;
            }
        }
        return false;
    };

	// 如果是周维护，则仪器列和间隔时间列不可编辑
	bool eAble = currentGroup.groupType == tf::MaintainGroupType::MAINTAIN_GROUP_WEEK;

    std::vector<tf::AutoMaintainCfg> noRepeatMaintItems;
    ClearRepeatMaintainCfg(autoMaintItems, noRepeatMaintItems);

    // 向表格填充内容
    int rowIdx = 0;
    for (const tf::AutoMaintainCfg& cfg : noRepeatMaintItems)
    {
        if (!funcInvalidDev(cfg.deviceSN))
        {
            continue;
        }

        if (rowIdx < m_rowsCtrl.size() && m_rowsCtrl[rowIdx]->IsValid())
        {
			m_rowsCtrl[rowIdx]->ClearAll();

            InitDeviceComb(m_rowsCtrl[rowIdx]->m_devComb, currentGroup, cfg.deviceSN);
			m_rowsCtrl[rowIdx]->m_intervEdit->setText(QString::fromStdString(cfg.timeInterval));
			m_rowsCtrl[rowIdx]->m_cardTime->UpdateData(cfg.maintainTime, cfg.lstWeekdays);

			// 周维护特殊处理
			m_rowsCtrl[rowIdx]->m_devComb->setDisabled(eAble);
			m_rowsCtrl[rowIdx]->m_intervEdit->setDisabled(eAble);
            m_rowsCtrl[rowIdx]->m_cardTime->setDisabled(eAble);
            m_rowsCtrl[rowIdx]->m_clearBtn->setDisabled(eAble);
        }

        rowIdx++;
        if (rowIdx >= MAX_NUM_ROW)
        {
            break;
        }
    }

    // 清空覆盖
    for (int clearIndex = rowIdx; clearIndex <= MAX_NUM_ROW; ++clearIndex)
    {
        if (clearIndex < m_rowsCtrl.size() && m_rowsCtrl[clearIndex]->IsValid())
        {
            InitDeviceComb(m_rowsCtrl[clearIndex]->m_devComb, currentGroup, "");
            m_rowsCtrl[clearIndex]->ClearAll();

			// 周维护特殊处理
			m_rowsCtrl[clearIndex]->m_devComb->setDisabled(eAble);
			m_rowsCtrl[clearIndex]->m_intervEdit->setDisabled(eAble);
            m_rowsCtrl[clearIndex]->m_cardTime->setDisabled(eAble);
            m_rowsCtrl[clearIndex]->m_clearBtn->setDisabled(eAble);
        }
    }
	// 默认选中第一行
    ui->tableViewAutoMaint->selectRow(0);
    ui->tableViewAutoMaint->scrollTo(ui->tableViewAutoMaint->model()->index(0, 0));
}

void QAutoMaintainCfgDlg::GetMaintainUiCfg(std::vector<tf::AutoMaintainCfg>& maintCfgs)
{
	// 当前时间
	std::string curTime = GetCurrentLocalTimeString();

    int rowCnt = m_autoMaintModel->rowCount();
    for (int i = 0; i < rowCnt; ++i)
    {
        // 获取间隔时间
        QString strInvterv = GetIntervlTimeFromUI(i);

        // 获取定时时间
        QString strFixTime;
        std::vector<int> effctDays;
        GetFixtimeFromUI(i, strFixTime, effctDays);

        // 获取设备SN
        QString strDevSn = GetDevSnFromUI(i);

        if (strDevSn.isEmpty() && strInvterv.isEmpty() && strFixTime == "00:00:00" && effctDays.empty())
        {
            continue;
        }

        tf::AutoMaintainCfg curCfg;
        curCfg.__set_deviceSN(strDevSn.toStdString());
        curCfg.__set_timeInterval(strInvterv.toStdString());
		curCfg.__set_intervalSetTime(curTime);
        curCfg.__set_maintainTime(strFixTime.toStdString());
        curCfg.__set_lstWeekdays(effctDays);
        maintCfgs.push_back(curCfg);
    }
}

void QAutoMaintainCfgDlg::InitDeviceComb(QComboBox* comb, const tf::MaintainGroup& currentGroup, const std::string& crrentDevSn)
{
    auto devMap = CommonInformationManager::GetInstance()->GetDeviceMaps();
    if (devMap.size() == 0)
    {
        ULOG(LOG_WARN, "Empty device map.");
        return;
    }
    bool isPip = DictionaryQueryManager::GetInstance()->GetPipeLine();
    int targetIdx = -1;
    int i = 1;
    comb->clear();
    comb->addItem("", "");
    for (const auto& dev : devMap)
    {
        if (dev.second->deviceType == tf::DeviceType::DEVICE_TYPE_INVALID)
        {
            continue;
        }
        // 单机时忽略轨道
        if (!isPip && dev.second->deviceType == tf::DeviceType::DEVICE_TYPE_TRACK)
        {
            continue;
        }

        if (currentGroup.groupType == ::tf::MaintainGroupType::type::MAINTAIN_GROUP_SINGLE)
        {
            if (currentGroup.items.size() > 0 &&
                currentGroup.items[0].itemType == ::tf::MaintainItemType::type::MAINTAIN_ITEM_CLEAN_TRACK)
            {
                // "清除样本架" 只支持轨道设备
                if (dev.second->deviceType != tf::DeviceType::DEVICE_TYPE_TRACK)
                {
                    continue;
                }
            }
            else
            {
                // 除“清除样本架”外，都不支持规定设备
                if (dev.second->deviceType == tf::DeviceType::DEVICE_TYPE_TRACK)
                {
                    continue;
                }
            }
        }

		std::string devName = dev.second->groupName.empty() ? dev.second->name : dev.second->groupName + dev.second->name;
        comb->addItem(QString::fromStdString(devName), QVariant(QString::fromStdString(dev.second->deviceSN)));

        if (crrentDevSn == dev.second->deviceSN)
        {
            targetIdx = i;
        }
        i++;
    }
    comb->setCurrentIndex(targetIdx);
}

///////////////////////////////////////////////////////////////////////////
/// @class     QAutoMaintainFilterProxyModel
/// @brief 	   维护组筛选排序模型代理
///////////////////////////////////////////////////////////////////////////
QAutoMaintainFilterProxyModel::QAutoMaintainFilterProxyModel(QObject * parent)
    :QSortFilterProxyModel(parent)
{
    setSourceModel(&MaintainGroupModel::Instance());
}

QAutoMaintainFilterProxyModel::~QAutoMaintainFilterProxyModel()
{
}

bool QAutoMaintainFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex & sourceParent) const
{
    if (MaintainGroupModel::Instance().GetData().empty())
    {
        return false;
    }

    if (sourceRow < 0 || sourceRow >= MaintainGroupModel::Instance().GetData().size())
    {
        return false;
    }

    auto& group = MaintainGroupModel::Instance().GetData()[sourceRow];
    if (group.groupType == ::tf::MaintainGroupType::type::MAINTAIN_GROUP_OTHER)
    {
        return false;
    }

    if (group.groupType != ::tf::MaintainGroupType::type::MAINTAIN_GROUP_SINGLE)
    {
        return true;
    }

    if (group.items.empty())
    {
        return false;
    }

    if (group.items.front().itemType == ::tf::MaintainItemType::type::MAINTAIN_ITEM_REAGENT_SCAN_POS)
    {
        return false;
    }

	// 生化过滤半自动维护
	if (group.groupType == ::tf::MaintainGroupType::type::MAINTAIN_GROUP_SINGLE &&
		(group.items.front().itemType == ::tf::MaintainItemType::type::MAINTAIN_ITEM_CH_CHANGE_REACTION_CUP ||
		group.items.front().itemType == ::tf::MaintainItemType::type::MAINTAIN_ITEM_CH_WASH_REACTION_PARTS ||
		group.items.front().itemType == ::tf::MaintainItemType::type::MAINTAIN_ITEM_CH_DRAIN_BUCKET ||
		group.items.front().itemType == ::tf::MaintainItemType::type::MAINTAIN_ITEM_CH_CLEAN_LIQUID_PATH ||
		group.items.front().itemType == ::tf::MaintainItemType::type::MAINTAIN_ITEM_ISE_CLEAN_FLUID_PATH ||
		group.items.front().itemType == ::tf::MaintainItemType::type::MAINTAIN_ITEM_ISE_CHANGE_ELECTRODE))
	{
		return false;
	}

    return true;
}

QString QAutoMaintainCfgDlg::GetDevSnFromUI(int rowIdx)
{
    QWidget* pDevRoot = ui->tableViewAutoMaint->indexWidget(m_autoMaintModel->index(rowIdx, 0));
    if (pDevRoot != nullptr)
    {
        QComboBox* pDev = pDevRoot->findChild<QComboBox*>();
        if (pDev != nullptr)
        {
            QVariant strDev = pDev->currentData();
            if (strDev.isValid() && !strDev.toString().isEmpty())
            {
                return strDev.toString();
            }
        }
    }

    return QString();
}

QString QAutoMaintainCfgDlg::GetIntervlTimeFromUI(int rowIdx)
{
    QWidget* pIntervRoot = ui->tableViewAutoMaint->indexWidget(m_autoMaintModel->index(rowIdx, 1));
	if (pIntervRoot != nullptr)
	{
		QLineEdit* pInterv = pIntervRoot->findChild<QLineEdit*>();
		if (pInterv != nullptr)
		{
			return pInterv->text();
		}
	}

    return QString();
}

void QAutoMaintainCfgDlg::GetFixtimeFromUI(int rowIdx, QString& strTime, std::vector<int>& vecDays)
{
    QWidget* pTime = ui->tableViewAutoMaint->indexWidget(m_autoMaintModel->index(rowIdx, 2));
    if (pTime != nullptr)
    {
        strTime = ((QCustomCard*)pTime)->GetTimeString();
    }

    // 获取设定的天
    vecDays.clear();
    ((QCustomCard*)pTime)->GetSelectedDays(vecDays);
}

UiRowCtrl::UiRowCtrl(int rowIdx, QTableView *parent /*= nullptr*/)
    :QObject(parent)
    , m_devComb(nullptr)
    , m_intervEdit(nullptr)
    , m_cardTime(nullptr)
    , m_clearBtn(nullptr)
    , m_rowIdx(rowIdx)
    , m_parentTb(parent)
{
}

UiRowCtrl::~UiRowCtrl()
{

}

void UiRowCtrl::InitConnect()
{
    if (!IsValid())
    {
        ULOG(LOG_WARN, "Not valid.");
        return;
    }
    // 清空按钮点击
    connect(m_clearBtn, &QPushButton::clicked, this, &UiRowCtrl::BtnClear);

    // 内容改变
    connect(m_cardTime, &QCustomCard::InputChanged, this, &UiRowCtrl::ChengeClearBtnState);
    connect(m_intervEdit, &QLineEdit::textChanged, this, [=](QString text) {
        ChengeClearBtnState();
    });
    connect(m_devComb, static_cast<void(QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
        [=](const QString &text) { ChengeClearBtnState(); });

    m_clearBtn->installEventFilter(this);
    m_cardTime->installEventFilter(this);
    m_devComb->installEventFilter(this);
    m_intervEdit->installEventFilter(this);

    // 清空按钮状态设置
    ChengeClearBtnState();
}

bool UiRowCtrl::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::FocusIn)
    {
        m_parentTb->selectRow(m_rowIdx);
    }
    return QObject::eventFilter(obj, event);
}

void UiRowCtrl::ClearAll()
{
    if (!IsValid())
    {
        ULOG(LOG_WARN, "Not valid.");
        return;
    }

    m_devComb->setCurrentIndex(-1);
    m_intervEdit->clear();
    m_cardTime->OnClear();
}

void UiRowCtrl::BtnClear()
{
    if (!IsValid())
    {
        ULOG(LOG_WARN, "Not valid.");
        return;
    }
    // 周维护会失能，不清空
    if (m_devComb->isEnabled())
    {
        m_devComb->setCurrentIndex(-1);
    }

    if (m_intervEdit->isEnabled())
    {
        m_intervEdit->clear();
    }

    m_cardTime->OnClear();
}

void UiRowCtrl::ChengeClearBtnState()
{
    if (!IsValid())
    {
        ULOG(LOG_WARN, "Not valid.");
        return;
    }

    // 判空
    if (m_devComb->currentText().isEmpty() &&
        m_intervEdit->text().isEmpty() &&
        m_cardTime->IsEmpty())
    {
        m_clearBtn->setDisabled(true);
    }
    else
    {
        m_clearBtn->setDisabled(false);
    }

}
