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
/// @file     alarmDialogui.cpp
/// @brief    主界面->告警对话框
///
/// @author   7951/LuoXin
/// @date     2022年7月19日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2022年7月19日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "alarmDialog.h"
#include "ui_alarmDialog.h"
#include <QToolTip>
#include <QHelpEvent>
#include <QMediaPlaylist>
#include <QHBoxLayout>
#include <QStandardItemModel>
#include <QMediaPlayer>

#include "mainResetDlg.h"
#include "alarmShieldDlg.h"
#include "SortHeaderView.h"

#include "manager/UserInfoManager.h"
#include "manager/DictionaryQueryManager.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/uidef.h"
#include "shared/tipdlg.h"
#include "shared/CommonInformationManager.h"
#include "shared/CReadOnlyDelegate.h"

#include "thrift/DcsControlProxy.h"
#include "src/common/defs.h"
#include "src/common/common.h"
#include "src/common/Mlog/mlog.h"

#define                     LEVEL                             "level"    // 音频播放器的当前报警等级动态属性名称
#define                     VOL                               "volume"   // 播放列表的音量动态属性名称
#define                     SN                                "sn"       // 筛选按钮设备序列号的动态属性
#define                     TableViewModel                    AlarmModel::Instance()    // 表格的数据模型

QAlarmDialog::QAlarmDialog(QWidget *parent)
	: BaseDlg(parent),
	  m_bDelFalg(false),
	  m_bInitMediaPlayerFlag(false),
	  m_mediaPlayer(new QMediaPlayer(this)),
      m_stopMediaPlaylist(new QMediaPlaylist()),
      m_sampleStopMediaPlaylist(new QMediaPlaylist()),
      m_attentionMediaPlaylist(new QMediaPlaylist()),
      m_pAlarmShieldDlg(nullptr),
      m_lastRaBtn(nullptr),
      m_loginUserType(::tf::UserType::USER_TYPE_GENERAL),
      ui(new Ui::QAlarmDialog)
{
	ui->setupUi(this);
	InitBeforeShow();
    InitDevRabtn();
    InitTableview();
    LoadShowLowerAlarmBtnStatus();
    OnPermisionChanged();
}

QAlarmDialog::~QAlarmDialog()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);
	delete ui;
}

void QAlarmDialog::LoadAlarmBeforeInit(std::vector<::tf::AlarmDesc>&& ads)
{
    // 按时间排序
    std::sort(ads.begin(), ads.end(), [](const tf::AlarmDesc& a, const tf::AlarmDesc& b)
    {
        return a.alarmTime < b.alarmTime;
    });

    for (auto& ad : ads)
    {
        OnDealFaultUpdate(ad);
    }
}

void QAlarmDialog::StopAlarmSound()
{
    m_mediaPlayer->stop(); 
}

bool QAlarmDialog::IsExistUnReadAlarm()
{
    bool showElc = ui->checkBox->isChecked();
    auto reagentIter = find_if(m_alarmData.begin(), m_alarmData.end(), [&](auto& iter)
    {
        return iter->unreadFlag && !(iter->isShield) && (showElc || iter->isVisible);
    });

    return reagentIter != m_alarmData.end();
}

void QAlarmDialog::OnDealFaultUpdate(const tf::AlarmDesc& alarmDesc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
  
    LoadDataToTabView({ alarmDesc });

    if (alarmDesc.mainCode == USER_STOP_ALARM_MAINCODE
        && alarmDesc.middleCode == USER_STOP_ALARM_MIDCODE
        && alarmDesc.subCode == USER_STOP_ALARM_SUBCODE)
    {
        TableViewModel.UpdateStopDeviceList();
    }

    if (!m_bInitMediaPlayerFlag)
    {
        // 初始化播放器
        m_bInitMediaPlayerFlag = true;
        OnAlarmComfigUpdate();
    }

    int level = TableViewModel.GetCurrentAlarmLevel();
    if (ui->checkBox->isChecked() || alarmDesc.isVisible)
    {
        PlayAlarmSound((::tf::AlarmLevel::type)level);
        emit AlarmLevelChanged(level);
    }
}

void QAlarmDialog::OnDealUpdateAlarmDetail(const tf::AlarmDesc& alarmDesc, const bool increaseParams)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    auto iter = find_if(m_alarmData.begin(), m_alarmData.end(), [&alarmDesc](auto& iter)
    {
        return iter->devSn == alarmDesc.deviceSN
            && iter->mainCode == alarmDesc.mainCode
            && iter->middleCode == alarmDesc.middleCode
            && iter->subCode == alarmDesc.subCode;
    });

    if (iter == m_alarmData.end())
    {
        ULOG(LOG_ERROR, "This alarmCode[%d-%d-%d] non-existent", alarmDesc.mainCode, alarmDesc.middleCode, alarmDesc.subCode);
        return;
    }

    auto data = std::make_shared<AlarmModel::AlarmRowItem>(**iter);
    m_alarmData.erase(iter);
    
    // 只更新报警码的报警详情和时间
    data->detail = alarmDesc.alarmDetail;
    data->time = alarmDesc.alarmTime;

    // 如果是增加详情中的参数，那么设置此条报警为未处理和未读
    if (increaseParams)
    {
        data->isDeal = false;
        data->unreadFlag = true;
    }

    auto stm = ui->tableView->selectionModel();
    auto rowLists = stm->selectedRows();

    m_alarmData.insert(m_alarmData.begin(), data);
    TableViewModel.SetData(m_alarmData);

    if (!rowLists.isEmpty())
    {
        stm->clear();
        for (const auto& rowIndex : rowLists)
        {
            QModelIndex topLeft = TableViewModel.index(rowIndex.row(), AlarmModel::COLUNM_MODEL);
            QModelIndex bottomRight = TableViewModel.index(rowIndex.row(), AlarmModel::COLUNM_TIME);
            stm->select(QItemSelection(topLeft, bottomRight), QItemSelectionModel::Select);
        }

        ui->tableView->scrollTo(TableViewModel.index(rowLists[rowLists.size() - 1].row(), AlarmModel::COLUNM_MODEL));
    }

    emit AlarmLevelChanged(TableViewModel.GetCurrentAlarmLevel());
}

void QAlarmDialog::OnDisCurrentAlarm(const tf::AlarmDesc& alarmDesc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 在缓存中删除指定的报警
    // 取消报警的功能只用于试剂、耗材的余量不足的告警
    // 此类告警应该是去重的，界面上始终只显示一条
    auto iter = find_if(m_alarmData.begin(), m_alarmData.end(), [&alarmDesc](auto& iter)
    {
        return iter->devSn == alarmDesc.deviceSN
            && iter->devType == alarmDesc.deviceType
            && iter->mainCode == alarmDesc.mainCode
            && iter->middleCode == alarmDesc.middleCode
            && iter->subCode == alarmDesc.subCode
            && iter->subName.toStdString() == alarmDesc.subModuleName;       
    });

    if (iter != m_alarmData.end())
    {
        m_alarmData.erase(iter);
    }

    // 检查是否存在试剂/耗材不足的报警
    bool showElc = ui->checkBox->isChecked();

    // 获取选中行
    std::vector<int> selectRows;
    auto selectionModel = ui->tableView->selectionModel();
    auto rowLists = selectionModel->selectedRows();
    for (const auto& rowIndex : rowLists)
    {
        selectRows.push_back(rowIndex.row());
    }

    // 删除行
    int deleteRow = TableViewModel.DeleteData(alarmDesc);

    // 选中原来的行
    int reduceRowNum = 0;
    if (!selectRows.empty())
    {
        if (deleteRow < selectRows[0])
        {
            reduceRowNum = -1;
        }

        for (int iRow : selectRows)
        {
            QModelIndex topLeft = TableViewModel.index(iRow + reduceRowNum, AlarmModel::COLUNM_MODEL);
            QModelIndex bottomRight = TableViewModel.index(iRow + reduceRowNum, AlarmModel::COLUNM_TIME);
            selectionModel->select(QItemSelection(topLeft, bottomRight), QItemSelectionModel::Select);
        }

        ui->tableView->scrollTo(TableViewModel.index(selectRows[selectRows.size()-1] + reduceRowNum, AlarmModel::COLUNM_MODEL));
    }

    emit AlarmLevelChanged(TableViewModel.GetCurrentAlarmLevel());
}

void QAlarmDialog::OnPermisionChanged()
{
    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;
    std::shared_ptr<UserInfoManager> userPms = UserInfoManager::GetInstance();

    m_loginUserType = userPms->GetLoginUserInfo()->type;
    // 电子报警
    ui->checkBox->setVisible(userPms->IsPermissionShow(PSM_IM_GENERAL_ALARMFUN));
    // 报警屏蔽
    ui->shield_btn->setVisible(userPms->IsPermissionShow(PSM_IM_GENERAL_ALARMSHEILD));
}

void QAlarmDialog::OnAlarmReaded(std::vector<tf::AlarmDesc, std::allocator<tf::AlarmDesc>> alarmDescs)
{
    for (auto& alarm : m_alarmData)
    {
        const auto& iter = std::find_if(alarmDescs.begin(), alarmDescs.end(), [&alarm](const auto& item)
        {
            return item.deviceSN == alarm->devSn && item.mainCode == alarm->mainCode
                && item.middleCode == alarm->middleCode && item.subCode == alarm->subCode;
        });

        if (iter != alarmDescs.end())
        {
            alarm->unreadFlag = false;
        }
    }

    TableViewModel.UpdateAllData();
    emit AlarmLevelChanged(TableViewModel.GetCurrentAlarmLevel());
}

void QAlarmDialog::InitBeforeShow()
{
	// 设置标题
	SetTitleName(tr("报警"));

    ui->all_rabtn->adjustSize();

    // 报警屏蔽
	connect(ui->shield_btn, SIGNAL(clicked()), this, SLOT(OnAlarmShield()));
    // 筛选
	connect(ui->all_rabtn, &QRadioButton::toggled, this, &QAlarmDialog::OnDevChanged);
    connect(ui->main_app_rabtn, &QRadioButton::toggled, this, &QAlarmDialog::OnDevChanged);
    //清空
    connect(ui->reset_btn, &QPushButton::clicked, this, &QAlarmDialog::OnClearAllAlarm);

    // 关闭按钮
    connect(ui->cancel_btn, &QPushButton::clicked, this, [&]
    {
		// 关闭后重新打开该界面导致ui->cancel_btn处于hover状态，bug 0013003
		QMouseEvent *leaveEvent = new QMouseEvent(QEvent::Leave, QPointF(0, 0), Qt::NoButton, Qt::NoButton, Qt::NoModifier);
		QApplication::postEvent(ui->cancel_btn, leaveEvent);

        reject();
        SetDataRead(m_lastRaBtn->text());
        emit AlarmLevelChanged(TableViewModel.GetCurrentAlarmLevel());
    });

    // 报警屏蔽列表按钮
    connect(ui->shield_list_btn, &QPushButton::clicked, this, [=] 
    {
        m_pAlarmShieldDlg->LoadCodeToview(m_asi);
        m_pAlarmShieldDlg->show();
    });

    // 电子报警勾选
    connect(ui->checkBox, &QCheckBox::toggled, this, [&](bool checked) 
    {
        ui->detail_label->clear();
        ui->solution_label->clear();
        UpdateLowerAlarmBtnStatus(checked);
        emit AlarmLevelChanged(TableViewModel.GetCurrentAlarmLevel());
    });

    connect(&TableViewModel, &AlarmModel::rowsInserted, [&] 
    { 
        ui->reset_btn->setEnabled(true); 
    });

	// 监听告警配置更新信息
	REGISTER_HANDLER(MSG_ID_ASSAY_SYSTEM_UPDATE, this, OnAlarmComfigUpdate);
    // 监听报警屏蔽清空
    REGISTER_HANDLER(MSG_ID_ALARM_SHIELD_CLEAR_ALL, this, OnClearAllShieldCode);
    // 监听告警信息
    REGISTER_HANDLER(MSG_ID_DEVS_FAULT_INFO, this, OnDealFaultUpdate);
    // 监听告警消失
    REGISTER_HANDLER(MSG_ID_DEVS_DIS_ALARM, this, OnDisCurrentAlarm);
    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);
    // 监听报警已读消息
    REGISTER_HANDLER(MSG_ID_ALARM_READED, this, OnAlarmReaded);
    // 监听报警信息的详情更新
    REGISTER_HANDLER(MSG_ID_ALARM_DETAIL_UPDATE, this, OnDealUpdateAlarmDetail);

    DictionaryQueryManager::GetAlarmShieldConfig(m_asi);
}

void QAlarmDialog::InitTableview()
{
    ui->tableView->setModel(&TableViewModel);

    // 设置表格列宽
    ui->tableView->setColumnWidth(AlarmModel::COLUNM_MODEL, 140);
    ui->tableView->setColumnWidth(AlarmModel::COLUNM_CODE, 180);
    ui->tableView->setColumnWidth(AlarmModel::COLUNM_LEVEL, 110);
    ui->tableView->setColumnWidth(AlarmModel::COLUNM_DESCRIBE, 400);
	ui->tableView->horizontalHeader()->setMinimumSectionSize(100);
	ui->tableView->horizontalHeader()->setStretchLastSection(true);

    // 字体颜色
    ui->tableView->setItemDelegateForColumn(AlarmModel::COLUNM_MODEL, new CReadOnlyDelegate(this));
    // 设置未读代理
    ui->tableView->setItemDelegateForColumn(AlarmModel::COLUNM_CODE, new QAlarmCodeDelegate(this));
    // 设置报警等级颜色代理
    ui->tableView->setItemDelegateForColumn(AlarmModel::COLUNM_LEVEL, new CReadOnlyDelegate(this));
    // 字体颜色
    ui->tableView->setItemDelegateForColumn(AlarmModel::COLUNM_DESCRIBE, new CReadOnlyDelegate(this));
    // 设置日期列代理（日期格式按设置格式显示）
    ui->tableView->setItemDelegateForColumn(AlarmModel::COLUNM_TIME, new CReadOnlyDelegate(this));

    // 表格选中行改变
    connect(ui->tableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QAlarmDialog::OnCurrentRowChanged);

    // 注册事件处理对象
    ui->tableView->viewport()->installEventFilter(this);
}

void QAlarmDialog::InitDevRabtn()
{
    auto CIM = CommonInformationManager::GetInstance();

    // 当前是否为单机版
    bool isSingle = !DictionaryQueryManager::GetInstance()->GetPipeLine();

    // 获取设备名称，用set去重
    std::set<std::string> devs;
    for (auto dev : CIM->GetDeviceMaps())
    {
        // 单机版排除轨道
        auto& spDev = dev.second;
        if (isSingle && spDev->deviceType == ::tf::DeviceType::DEVICE_TYPE_TRACK)
        {
            continue;
        }

        devs.insert(spDev->groupName.empty() ? spDev->name : spDev->groupName);
    }

    for (const auto& dev : devs)
    {
        QRadioButton* btn = new QRadioButton(ui->groupBox);

        QString name = QString::fromStdString(dev);
        btn->setText(name);
        btn->setProperty(SN, CIM->GetDeviceSnByName(name.toStdString()));
        btn->adjustSize();
        ui->horizontalLayout->addWidget(btn);
        connect(btn, &QRadioButton::toggled, this, &QAlarmDialog::OnDevChanged);
    }

    m_lastRaBtn = ui->all_rabtn;
    ui->all_rabtn->setProperty(SN, QStringList(tr("全部")));
    ui->main_app_rabtn->setProperty(SN, QStringList(tr("ControlUnit")));
}

void QAlarmDialog::LoadDataToTabView(const ::tf::AlarmDesc& alarmDesc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 设备名称
    QString devname;
    if (alarmDesc.deviceSN == CONTROL_UNIT_NAME)
    {
        devname = tr("控制单元");
    }
    else
    {
        auto name = CommonInformationManager::GetInstance()->GetDeviceName(alarmDesc.deviceSN);
        if (name .empty())
        {
            return;
        }

        devname = QString::fromStdString(name);
    }

    // 组装显示数据
    auto item = std::make_shared<AlarmModel::AlarmRowItem>(alarmDesc, devname);

    // 如果报警去重，删除旧的记录
     bool delDistinctAlarm = false;
     if (alarmDesc.isDistinct)
     {
         auto iter = std::find_if(m_alarmData.begin(), m_alarmData.end(), [&item](const auto& data)
         {
             return item->devSn == data->devSn
                 && item->subName == data->subName
                 && item->mainCode == data->mainCode
                 && item->middleCode == data->middleCode
                 && item->subCode == data->subCode
                 && item->detail == data->detail;           // 告警详情一样才去重,bug:13797
         });
         if (iter != m_alarmData.end())
         {
             delDistinctAlarm = true;
             m_alarmData.erase(iter);
         }
     }

    //设置报警屏蔽
    std::string code = std::to_string(alarmDesc.mainCode) + "-" + std::to_string(alarmDesc.middleCode)
        + "-" + std::to_string(alarmDesc.subCode);

    AlarmShieldInfoItem asiItem(alarmDesc.deviceSN, alarmDesc.deviceType, alarmDesc.subModuleName);
    item->isShield = m_asi.codeIsShield(code, asiItem);
    
    m_alarmData.insert(m_alarmData.begin(), item);

    // 电子报警，但是未打开电子报警的开关就不刷新到表格
    if (!item->isVisible && !ui->checkBox->isChecked())
    {
        return;
    }

    // 添加新产生的报警到表格
    if (m_lastRaBtn->text() == tr("全部") || m_lastRaBtn->text() == devname)
    {
        int addRow = 1;
        auto selectionModel = ui->tableView->selectionModel();
        auto rowLists = selectionModel->selectedRows();

        if (delDistinctAlarm)
        {
            addRow = 0;
            TableViewModel.SetData(m_alarmData);
        }
        else
        {
            TableViewModel.AppendData(item);
        }
       
        if (!rowLists.isEmpty())
        {
            selectionModel->clear();
            for (const auto& rowIndex : rowLists)
            {
                QModelIndex topLeft = TableViewModel.index(rowIndex.row() + addRow, AlarmModel::COLUNM_MODEL);
                QModelIndex bottomRight = TableViewModel.index(rowIndex.row() + addRow, AlarmModel::COLUNM_TIME);
                selectionModel->select(QItemSelection(topLeft, bottomRight), QItemSelectionModel::Select);
            }

            ui->tableView->scrollTo(TableViewModel.index(rowLists[rowLists.size()-1].row() + 1, AlarmModel::COLUNM_MODEL));
        }
    }

    ui->shield_btn->setEnabled(ui->tableView->selectionModel()->hasSelection() && m_loginUserType > ::tf::UserType::USER_TYPE_GENERAL);
}

void QAlarmDialog::PlayAlarmSound(::tf::AlarmLevel::type level)
{
    if (level <= m_mediaPlayer->property(LEVEL).toInt()
        && m_mediaPlayer->state() == QMediaPlayer::PlayingState)
    {
        return;
    }

	// 设置音频播放列表
	if (level == ::tf::AlarmLevel::ALARM_LEVEL_TYPE_STOP)
	{   
		if (m_mediaPlayer->playlist() != m_stopMediaPlaylist)
		{
            m_mediaPlayer->stop();
            m_mediaPlayer->setPlaylist(m_stopMediaPlaylist);
		}
	}
	else if(level == ::tf::AlarmLevel::ALARM_LEVEL_TYPE_STOPSAMPLING)
	{
        if (m_mediaPlayer->playlist() != m_sampleStopMediaPlaylist)
        {
            m_mediaPlayer->stop();
            m_mediaPlayer->setPlaylist(m_sampleStopMediaPlaylist);
        }
	}
    else
    {
        if (m_mediaPlayer->playlist() != m_attentionMediaPlaylist)
        {
            m_mediaPlayer->stop();
            m_mediaPlayer->setPlaylist(m_attentionMediaPlaylist);
        }
    }

    // 设置播放器当前的报警的等级
    m_mediaPlayer->setProperty(LEVEL,(int)level);

    // 播放音频
    if (m_mediaPlayer->state() != QMediaPlayer::PlayingState)
    {
        m_mediaPlayer->setVolume(m_mediaPlayer->playlist()->property(VOL).toInt());
        m_mediaPlayer->play();
    }
}

void QAlarmDialog::UpdateMediaPlayer(QMediaPlaylist* playList, const std::string& name, int vol)
{
    //设置停机音频文件
    QString path = QString::fromStdString(GetCurrentDir()) + ALARM_MUSIC_CONFIG_DIR;
    QFile alarmFile(path + QString::fromStdString(name));
    if (alarmFile.exists())
    {
        // 保存当前播放状态
        bool playing = (m_mediaPlayer->state() == QMediaPlayer::PlayingState);

        // 更新播放列表
        playList->clear();
        playList->addMedia(QUrl::fromLocalFile(path + QString::fromStdString(name)));
        playList->setPlaybackMode(QMediaPlaylist::CurrentItemInLoop);
        playList->setProperty(VOL, vol);
    }
}

void QAlarmDialog::showEvent(QShowEvent *event)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

    if (m_pAlarmShieldDlg == nullptr)
    {
        m_pAlarmShieldDlg = new alarmShieldDlg(this);
        // 解除报警屏蔽
        connect(m_pAlarmShieldDlg->GetDisAlarmShieldBtnPtr(), SIGNAL(clicked()), this, SLOT(OnDisAlarmShield()));
    }
    
	// 基类先处理
	BaseDlg::showEvent(event);

    // 默认
    if (ui->tableView->selectionModel()->hasSelection())
    {
        ui->shield_btn->setEnabled(m_loginUserType > ::tf::UserType::USER_TYPE_GENERAL);
    }
    else if(TableViewModel.rowCount() > 0)
    {
        ui->tableView->selectRow(0);
    }

    ui->all_rabtn->setChecked(true);
    m_lastRaBtn = ui->all_rabtn;
}

void QAlarmDialog::LoadShowLowerAlarmBtnStatus()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    bool isShowLower(false);
    if (!DictionaryQueryManager::GetShowLowerAlarmConfig(isShowLower))
    {
        return;
    }
    ui->checkBox->setChecked(isShowLower);
}

void QAlarmDialog::UpdateLowerAlarmBtnStatus(bool checked)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    if (!DictionaryQueryManager::UpdateLowerAlarmStatus(checked))
    {
        ULOG(LOG_ERROR, "ModifyDictionaryInfo Failed");
        return;
    }

    UpdateShowAlarmByDevName();
}

void QAlarmDialog::SetDataRead(const QString& name)
{
    for (auto& item : m_alarmData)
    {
        if (name == tr("全部") || item->devName == name)
        {
            item->unreadFlag = false;
        }
    }

    TableViewModel.UpdateAllData();

    bool isAll = name == tr("全部");
    std::vector<std::string> snVec;
    for (auto& btn : findChildren<QRadioButton*>())
    {
        if (btn->isChecked() && btn->text() == tr("控制单元"))
        {
            return;
        }

        if ((isAll || btn->isChecked()) && btn->text() != tr("控制单元") && btn->text() != tr("全部"))
        {
            for (auto& sn : btn->property(SN).toList())
            {
                snVec.emplace_back(sn.toString().toStdString());
            }
        }
    }

    if (!DcsControlProxy::GetInstance()->DeviceAlarmReaded(snVec, true))
    {
        ULOG(LOG_ERROR, "Send alarm readed to dcs failed !");
    }
}

void QAlarmDialog::UpdateShowAlarmByDevName()
{
    QString ss = m_lastRaBtn->text();
    std::vector<std::shared_ptr<AlarmModel::AlarmRowItem>>	data;
    for (auto& item : m_alarmData)
    {
        if ((m_lastRaBtn->text() == tr("全部") || item->devName.startsWith(m_lastRaBtn->text()))
            &&(item->isVisible || ui->checkBox->isChecked()))
        {
            data.push_back(item);
        }
    }

    TableViewModel.SetData(std::move(data));
    ui->reset_btn->setEnabled(!data.empty());
}

void QAlarmDialog::SetShieldRow(const std::string& devSn, const std::string& name, int mainCode, int midCode, int subCode, bool isShield)
{
    for (auto& item : m_alarmData)
    {
        if (item->devSn == devSn && item->subName == QString::fromStdString(name)
            && item->mainCode == mainCode && item->middleCode == midCode
            && item->subCode == subCode)
        {
            item->isShield = isShield;
        }
    }

    TableViewModel.UpdateAllData();
}

bool QAlarmDialog::eventFilter(QObject* target, QEvent* event)
{
    if (event->type() == QEvent::ToolTip)
    {
        QHelpEvent* helpEvent = static_cast<QHelpEvent*>(event);
        if (helpEvent != Q_NULLPTR)
        {
            auto index = ui->tableView->indexAt(helpEvent->pos());
            if (index.isValid())
            {
                auto rawData = ui->tableView->model()->data(index, Qt::DisplayRole);
                QString tipContent = rawData.toString();
                QToolTip::showText(helpEvent->globalPos(), tipContent);
            }
        }
    }

    return BaseDlg::eventFilter(target, event);    
}

void QAlarmDialog::OnAlarmComfigUpdate()
{
	AlarmMusic am;
	if (!DictionaryQueryManager::GetAlarmMusicConfig(am))
	{
		ULOG(LOG_ERROR, "Failed to get alarmmusic config.");
		return;
	}

    bool isPlay = m_mediaPlayer->state() == QMediaPlayer::PlayingState;
	//设置停机音频文件
    UpdateMediaPlayer(m_stopMediaPlaylist, am.strStopSoundName, am.iStopSoundVolume);
    //设置加样停音频文件
    UpdateMediaPlayer(m_sampleStopMediaPlaylist, am.strSampleStopSoundName, am.iSampleStopSoundVolume);
	// 设置注意音频文件
    UpdateMediaPlayer(m_attentionMediaPlaylist, am.strAttentionSoundName, am.iAttentionSoundVolume);

    if (isPlay)
    {
        m_mediaPlayer->setVolume(m_mediaPlayer->playlist()->property(VOL).toInt());
        m_mediaPlayer->play();
    }
}

void QAlarmDialog::OnCurrentRowChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    ui->detail_label->clear();
    ui->solution_label->clear();

    bool bEnabled = false;
    for (auto& rowIndex : ui->tableView->selectionModel()->selectedRows())
    {
        auto data = TableViewModel.GetDataByIndex(rowIndex);
        if (data == nullptr)
        {
            ULOG(LOG_ERROR, "GetDataByIndex Failed!");
            return;
        }

        // 报警屏蔽按钮的使能
        if (!bEnabled)
        {
            bEnabled = !data->isShield;
        }

        // 详情
        ui->detail_label->setText(QString::fromStdString(data->detail));

        // 解决方案
        auto userInfo = UserInfoManager::GetInstance()->GetLoginUserInfo();
        if (userInfo == nullptr || userInfo->type <= ::tf::UserType::USER_TYPE_ADMIN)
        {
            ui->solution_label->setText(QString::fromStdString(data->customSolution));
        }
        else
        {
            ui->solution_label->setText(QString::fromStdString(data->managerSolution));
        }    
    }

    ui->shield_btn->setEnabled(bEnabled && m_loginUserType > ::tf::UserType::USER_TYPE_GENERAL);
}

void QAlarmDialog::OnDevChanged(bool checked)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	if (!checked)
	{
		return;
	}

    ui->solution_label->clear();
    ui->detail_label->clear();

    // 清除最新报警标记
    SetDataRead(m_lastRaBtn->text());

    // 设置当前展示的设备
    m_lastRaBtn = qobject_cast<QRadioButton *>(sender());

    UpdateShowAlarmByDevName();
}

void QAlarmDialog::OnAlarmShield()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

    DictionaryQueryManager::GetAlarmShieldConfig(m_asi);
    for (auto& rowIndex : ui->tableView->selectionModel()->selectedRows())
    {
        // 在当前报警列表中设置被屏蔽的报警
        auto data = TableViewModel.GetDataByIndex(rowIndex);

        if (data == nullptr)
        {
            ULOG(LOG_ERROR, "GetDataByIndex Failed!");
            return;
        }

        if (data->level != ::tf::AlarmLevel::ALARM_LEVEL_TYPE_CAUTION)
        {
            TipDlg(tr("提示"), tr("只能屏蔽注意级别的报警")).exec();
            return;
        }

        std::string subName = data->subName.toStdString();     
        std::string code = std::to_string(data->mainCode) + "-" + std::to_string(data->middleCode) + "-" + std::to_string(data->subCode);
        if (m_asi.mapCodeAndDev.find(code) == m_asi.mapCodeAndDev.end())
        {
            m_asi.mapCodeAndDev[code] = { AlarmShieldInfoItem(data->devSn, data->devType, subName)};
        }
        else
        {
            m_asi.mapCodeAndDev[code].insert(AlarmShieldInfoItem(data->devSn, data->devType, subName));
        }
    }

    // 保存
    if (!DictionaryQueryManager::SaveAlarmShieldConfig(m_asi))
    {
        ULOG(LOG_ERROR, "ModifyDictionaryInfo Failed !");
        TipDlg(tr("保存失败"), tr("保存失败")).exec();
    }

    // 刷新表格中被屏蔽的报警码
    for (const auto& codeAndItems : m_asi.mapCodeAndDev)
    {
        const auto& codeList = QString::fromStdString(codeAndItems.first).split("-");

        for (const auto& item : codeAndItems.second)
        {
            SetShieldRow(item.devSn, item.subName, codeList[0].toInt(), codeList[1].toInt(), codeList[2].toInt(), true);
        }
    }

    ui->detail_label->clear();
    ui->solution_label->clear();
    ui->shield_btn->setEnabled(ui->tableView->selectionModel()->hasSelection() && m_loginUserType > ::tf::UserType::USER_TYPE_GENERAL);
}

void QAlarmDialog::OnDisAlarmShield()
{
    if (TipDlg(tr("即将解除选中的报警码的屏蔽状态，请确认！"), TipDlgType::TWO_BUTTON).exec() == QDialog::Rejected)
    {
        return;
    }

    DictionaryQueryManager::GetAlarmShieldConfig(m_asi);
    const auto& tempAsi = m_pAlarmShieldDlg->GetDisAlarmShieldCodeMap();
    for (const auto& iter : tempAsi.mapCodeAndDev)
    {
        // 解除屏蔽的设备列表
        auto& disAlarmShieldDev = iter.second;

        auto mapShieldCodeAndSnIter = m_asi.mapCodeAndDev.find(iter.first);
        if (mapShieldCodeAndSnIter != m_asi.mapCodeAndDev.end())
        {
            // 已屏蔽的设备列表
            auto& alarmShield = mapShieldCodeAndSnIter->second;
            
            // 将已屏蔽的设备列表中需要解除屏蔽的设备移除
            for (const auto& dev : disAlarmShieldDev)
            {
                const auto& codeList = QString::fromStdString(mapShieldCodeAndSnIter->first).split("-");
                // 报警码一定是三段
                if (codeList.size() == 3)
                {
                    SetShieldRow(dev.devSn, dev.subName, codeList[0].toInt(), codeList[1].toInt(), codeList[2].toInt(), false);
                }

                alarmShield.erase(dev);
            }

            if (alarmShield.empty())
            {
                m_asi.mapCodeAndDev.erase(mapShieldCodeAndSnIter);
            }
        }
    }

    if (!DictionaryQueryManager::SaveAlarmShieldConfig(m_asi))
    {
        ULOG(LOG_ERROR, "Failed to save alarmshield config!");
        TipDlg(tr("保存失败"), tr("保存失败")).exec();
        return;
    }

    m_pAlarmShieldDlg->LoadCodeToview(m_asi);
}

void QAlarmDialog::OnClearAllAlarm()
{
    if (TipDlg(tr("清空"), tr("即将清空当前勾选设备的所有的报警"), TipDlgType::TWO_BUTTON).exec() == QDialog::Rejected)
    {
        return;
    }

    QStringList snList;
    for (auto& btn : ui->widget->findChildren<QRadioButton*>())
    {
        if (btn->isChecked())
        {
            snList = btn->property(SN).toStringList();
            break;
        }
    }

    // 清空表格
    TableViewModel.RemoveRowByDevSn(snList);

    // 清空缓存
    if (snList.contains(tr("全部")))
    {
        m_alarmData.clear();
    }
    else
    {
        for (auto iter = m_alarmData.begin(); iter != m_alarmData.end();)
        {
            if (snList.contains(QString::fromStdString((*iter)->devSn)))
            {
                iter = m_alarmData.erase(iter);
            }
            else
            {
                iter++;
            }
        }
    }

    ui->detail_label->clear();
    ui->solution_label->clear();
    ui->shield_btn->setEnabled(ui->tableView->selectionModel()->hasSelection() && m_loginUserType > ::tf::UserType::USER_TYPE_GENERAL);

    std::vector<std::string> sns;
    for (auto& sn : snList)
    {
        sns.push_back(sn.toStdString());
    }

    bool deleteAll = ui->all_rabtn->isChecked();
    if (!DcsControlProxy::GetInstance()->DeleteCurAlarm(deleteAll, sns))
    {
        TipDlg(tr("清空失败"),tr("清空当前页面的报警失败")).exec();
        return;
    }

    ui->reset_btn->setEnabled(false);
    emit AlarmLevelChanged(TableViewModel.GetCurrentAlarmLevel());
}

void QAlarmDialog::OnClearAllShieldCode()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    for (auto& item : m_alarmData)
    {
        item->isShield = false;
    }

    m_asi.mapCodeAndDev.clear();

    if (m_pAlarmShieldDlg != nullptr)
    {
        m_pAlarmShieldDlg->LoadCodeToview(m_asi);
        update();
    }
}
