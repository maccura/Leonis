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

#include "QStatusModule.h"
#include "ResetCountInfoDlg.h"
#include "ui_QStatusModule.h"
#include "CupHistoryWidget.h"
#include "src/common/Mlog/mlog.h"
#include "thrift/DcsControlProxy.h"
#include "shared/uidef.h"
#include "shared/uicommon.h"
#include "shared/msgiddef.h"
#include "shared/messagebus.h"
#include "shared/CReadOnlyDelegate.h"
#include "shared//CommonInformationManager.h"
#include "manager/UserInfoManager.h"
#include "manager/DictionaryQueryManager.h"
#include "shared/tipdlg.h"
#include "Serialize.h"
#include "printcom.hpp"
#include "shared/FileExporter.h"
#include <QDateTime>
#include <QFileDialog>

#define  TEMPERAUTURE_FACTOR	(10)	            // 温度倍率
#define  TEMP_GET_ERR			(9990)	            // 温度获取失败

// 温度表格列
enum TEMPERAUTURE_COL
{
	TEMPERAUTURE_MOUDLE,                 // 模块
	TEMPERAUTURE_SUB_SYSTEM_NAME,		 // 部件名称
	TEMPERAUTURE_CURRENT_TEMP,           // 当前温度
	TEMPERAUTURE_TEMP_RANGE,             // 温度范围
    TEMPERAUTURE_STATUS                  // 温控状态
};

// 液路表格列
enum LIUQUIPATH_COL
{
	LIUQUIPATH_MOUDLE,					// 模块
	LIUQUIPATH_SUB_SYSTEM_NAME,			// 部件名称
	LIUQUIPATH_CURRENT_STATUS			// 当前状态
};

// 单元表格列
enum UNITINFO_COL
{
	UNITINFO_MOUDLE,					// 模块
	UNITINFO_SUB_SYSTEM_NAME,			// 部件名称
	UNITINFO_CURRENT_STATUS				// 当前状态
};

// 计数左侧表格列
enum COUNTINFO_COL_LEFT
{
    COUNTINFO_MOUDLE,					// 模块
    COUNTINFO_UNIT_NAME			        // 单元名称
};

// 计数右侧表格列
enum COUNTINFO_COL_RIGHRT
{
	COUNTINFO_SUB_SYSTEM_NAME,			// 部件名称
    COUNTINFO_SUB_SN,			        // 部件序列号
    COUNTINFO_COUNT_LIMIT,              // 使用上限
	COUNTINFO_CURRENT_COUNT,			// 当前数目
    COUNTINFO_RESET_TIME,               // 重置时间
    COUNTINFO_COUNT			            // 累计数目
};

QStatusModule::QStatusModule(QWidget *parent)
    : QWidget(parent)
    , m_pResetCountInfoDlg(new ResetCountInfoDlg(this))
{
    ui = new Ui::QStatusModule();
    ui->setupUi(this);
    Init();
}

QStatusModule::~QStatusModule()
{
}

void QStatusModule::Init()
{
	ui->table_view_temperature->setModel(&m_temperautureItemModel);
	ui->table_view_liquid->setModel(&m_liquidPathItemModel);
	ui->table_view_unit->setModel(&m_unitItemModel);
	ui->table_view_count_right->setModel(&m_countItemModel_right);
    ui->table_view_count_left->setModel(&m_countItemModel_left);

	m_temperautureItemModel.setHorizontalHeaderLabels({tr("模块"), tr("部件名称"),tr("当前温度"), tr("温控范围"), tr("温控状态") });
	m_liquidPathItemModel.setHorizontalHeaderLabels({ tr("模块"), tr("名称"),tr("状态")});
	m_unitItemModel.setHorizontalHeaderLabels({ tr("模块"), tr("名称"),tr("状态")});
    m_countItemModel_left.setHorizontalHeaderLabels({ tr("模块"), tr("单元名称") });
	m_countItemModel_right.setHorizontalHeaderLabels({ tr("部件名称"), 
        tr("部件序列号"), tr("使用上限"), tr("当前使用数目"), tr("重置时间"), tr("累计使用数目") });

    ui->table_view_temperature->setItemDelegateForColumn(TEMPERAUTURE_COL::TEMPERAUTURE_CURRENT_TEMP, new CReadOnlyDelegate(this));
	ui->table_view_liquid->setItemDelegateForColumn(LIUQUIPATH_COL::LIUQUIPATH_CURRENT_STATUS, new CReadOnlyDelegate(this));
	ui->table_view_unit->setItemDelegateForColumn(UNITINFO_COL::UNITINFO_CURRENT_STATUS, new CReadOnlyDelegate(this));

	ui->table_view_temperature->horizontalHeader()->setMinimumSectionSize(100);
	ui->table_view_count_left->horizontalHeader()->setMinimumSectionSize(80);
	ui->table_view_count_right->setColumnWidth(0, 330);
	ui->table_view_count_right->horizontalHeader()->setMinimumSectionSize(120);

	ui->table_view_liquid->setColumnWidth(LIUQUIPATH_COL::LIUQUIPATH_MOUDLE, 465);
	ui->table_view_liquid->setColumnWidth(LIUQUIPATH_COL::LIUQUIPATH_SUB_SYSTEM_NAME, 890);
	ui->table_view_unit->setColumnWidth(UNITINFO_COL::UNITINFO_MOUDLE, 465);
	ui->table_view_unit->setColumnWidth(UNITINFO_COL::UNITINFO_SUB_SYSTEM_NAME, 890);

    // 获取设备
    auto CMI = CommonInformationManager::GetInstance();
    const auto& devs = CMI->GetDeviceFromType({ ::tf::DeviceType::DEVICE_TYPE_C1000,
                ::tf::DeviceType::DEVICE_TYPE_I6000, ::tf::DeviceType::DEVICE_TYPE_ISE1005 });

    // 处理2000速设备名字
    for (auto& dev : devs)
    {
        auto name = QString::fromStdString(dev->groupName.empty() ? dev->name : dev->groupName + dev->name);
        QRadioButton* btn = new QRadioButton(name, this);
        btn->setProperty("type",dev->deviceType);
        ui->rabtn_widget->layout()->addWidget(btn);
        connect(btn, &QRadioButton::toggled, this, &QStatusModule::OnSelcetDevChanged); 
    }

    ui->all_rabtn->setProperty("type", ::tf::DeviceType::DEVICE_TYPE_INVALID);

    // 免疫单机版不显示筛选按钮
    auto type = CMI->GetSoftWareType();
    if (!DictionaryQueryManager::GetInstance()->GetPipeLine() && type == SOFTWARE_TYPE::IMMUNE)
    {
        ui->all_rabtn->setChecked(true);
        ui->rabtn_widget->hide();
    }

    // 添加反应杯历史
    if (type == IMMUNE && ui->tabWidget->widget(0)->objectName() == "cupHistoryWidget")
    {
        ui->tabWidget->removeTab(0);
        ui->tabWidget->setCurrentIndex(0);
        ui->all_rabtn->show();
    }

    //  免疫不需要液路
    if (type == IMMUNE && ui->tabWidget->widget(2)->objectName() == "tabLiquid")
    {
        ui->tabWidget->removeTab(2);
    }

    connect(ui->all_rabtn, &QRadioButton::toggled, this, &QStatusModule::OnSelcetDevChanged);
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, &QStatusModule::OnTabWidgetChangePage);
    connect(ui->flat_print, &QPushButton::clicked, this, &QStatusModule::OnPrintBtnClicked);
    connect(ui->export_btn, &QPushButton::clicked, this, &QStatusModule::OnExportBtnClicked);
    connect(ui->reset_btn, &QPushButton::clicked, this, &QStatusModule::OnResetBtnClicked);
    connect(ui->cancel_reset_btn, &QPushButton::clicked, this, &QStatusModule::OnCancelResetBtnClicked);
    connect(m_pResetCountInfoDlg->GetSaveBtnPtr(), &QPushButton::clicked, this, &QStatusModule::OnResetDlgSaveClicked);

    connect(ui->table_view_count_right->selectionModel(), &QItemSelectionModel::selectionChanged, this, [&]
    {
        bool enable = ui->table_view_count_right->selectionModel()->hasSelection();
        ui->reset_btn->setEnabled(enable);
        ui->cancel_reset_btn->setEnabled(enable);
    });

    //设置取消重置按钮
    //QString tmpStr = ui->cancel_reset_btn->fontMetrics().elidedText(ui->cancel_reset_btn->text(), Qt::ElideRight, 60);
    //ui->cancel_reset_btn->setToolTip(ui->cancel_reset_btn->text());
    //ui->cancel_reset_btn->setText(tmpStr);

    // 监听状态信息
    REGISTER_HANDLER(MSG_ID_DEVICE_STATUS_INFO, this, OnStatusInfoUpdate);
    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);

    // 初始化界面的表格
    InitStatusInfoUpdate();
}

void QStatusModule::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    ui->tabWidget->setCurrentIndex(0);

    // 反应杯历史页面选中第一个比色设备
    if (ui->tabWidget->currentWidget()->objectName() == "cupHistoryWidget")
    {
        for (auto& btn : ui->rabtn_widget->findChildren<QRadioButton*>())
        {
            if (btn->property("type").toInt() == ::tf::DeviceType::DEVICE_TYPE_C1000)
            {
                btn->setChecked(true);
                break;
            }
        }
    }
    // 其他页面选中“全部”
    else
    {
        ui->all_rabtn->setChecked(true);
    }

    OnTabWidgetChangePage();

}

void QStatusModule::UpdateTemperautureInfo(const QString& devName, const std::vector<tf::TemperautureInfo>& temperautureInfo)
{
	ULOG(LOG_INFO, "%s(dev:%s)", __FUNCTION__, devName.toStdString());

	for (const tf::TemperautureInfo& item : temperautureInfo)
	{
        if (m_tempData.count(devName) == 0)
        {
            ULOG(LOG_ERROR,"%s is not found.", devName.toStdString());
            continue;
        }

        auto spDev = CommonInformationManager::GetInstance()->GetDeviceInfo(m_tempData[devName].toStdString());
        QString qstrTmpDevName = QString::fromStdString(item.subDevName);

		// 遍历所有行
		for (int i=0; i < m_temperautureItemModel.rowCount(); i++)
		{
			if (m_temperautureItemModel.index(i, TEMPERAUTURE_COL::TEMPERAUTURE_MOUDLE).data().toString() == devName
				&& m_temperautureItemModel.index(i, TEMPERAUTURE_COL::TEMPERAUTURE_SUB_SYSTEM_NAME).data().toString() == qstrTmpDevName)
			{
                // 设置温控状态的背景色和文字
                bool status = item.currentTemp < item.tempLower || item.currentTemp > item.tempUpper;
                QStandardItem* statusSi = m_temperautureItemModel.item(i, TEMPERAUTURE_COL::TEMPERAUTURE_STATUS);
                statusSi->setText(status ? tr("预热") : tr("正常"));// 2024年8月7日 需求变更公文要求显示预热
                statusSi->setData(Qt::AlignCenter, Qt::TextAlignmentRole);

                //免疫缺陷0029829: [应用] 温度异常，应用 - 状态 - 温度界面“温控状态”多余红色填充要求更改 by ldx
                //statusSi->setData(QColor(status ? UI_REAGENT_WARNCOLOR : "#ffffff"), Qt::UserRole + DELEGATE_COLOR_OFFSET_POS);

				// 设置温度
                QStandardItem* si = m_temperautureItemModel.item(i, TEMPERAUTURE_COL::TEMPERAUTURE_CURRENT_TEMP);
                // 若当前温度获取失败，则温度界面显示"--"
                if (item.currentTemp == TEMP_GET_ERR)
                {
                    si->setText(QString("--") + tr("℃"));
                }
                else
                {
                    si->setText(QString::number(double(item.currentTemp) / TEMPERAUTURE_FACTOR, 'f', 1) + tr("℃"));
                }

                si->setData(QColor(status ? UI_REAGENT_WARNFONT : "#565656"), Qt::UserRole + 1);
				continue;
			}
		}
	}
}

void QStatusModule::SetLiquidPathInfoItem(const QString& name, bool status, QStandardItem* item)
{
    QString text;
    if (name.contains(tr("水位传感器")))
    {
        text = status ? tr("高") : tr("低");
    }
    else
    {
        text = status ? tr("正常") : tr("异常");
    }
    item->setText(text);

    // 设置背景色
    QColor itemColor("#ffffff");
    if (text == tr("异常")
        || (text == tr("低") && name.contains(tr("低水位传感器"))))
    {
        itemColor = QColor(UI_REAGENT_WARNCOLOR);
    }
    item->setData(itemColor, Qt::UserRole + DELEGATE_COLOR_OFFSET_POS);
}

void QStatusModule::UpdateLiquidPathInfo(const QString& devName, const std::vector<tf::LiquidPathInfo>& liquidPathInfo)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	for (const tf::LiquidPathInfo& item : liquidPathInfo)
	{
        QString qstrPthName = QString::fromStdString(item.name);
		// 遍历所有行
		bool useFalg = false;
		for (int i = 0; i < m_liquidPathItemModel.rowCount(); i++)
		{
			if (m_liquidPathItemModel.index(i, LIUQUIPATH_COL::LIUQUIPATH_MOUDLE).data().toString() == devName
				&& m_liquidPathItemModel.index(i, LIUQUIPATH_COL::LIUQUIPATH_SUB_SYSTEM_NAME).data().toString() == qstrPthName)
			{
                auto si = m_liquidPathItemModel.item(i, LIUQUIPATH_COL::LIUQUIPATH_CURRENT_STATUS);
                SetLiquidPathInfoItem(QString::fromStdString(item.name), item.bStatus, si);

				useFalg = true;
				continue;
			}
		}

		if (useFalg)
		{
			// 当前数据已处理
			continue;
		}

		// 模块
		QStandardItem* siDev = new QStandardItem(devName);
        siDev->setData(Qt::AlignCenter, Qt::TextAlignmentRole);

		// 部件名称
		QStandardItem* siName = new QStandardItem(QString::fromStdString(item.name));
        siName->setData(Qt::AlignCenter, Qt::TextAlignmentRole);

		// 状态
        QStandardItem* siStatus = new QStandardItem;
        SetLiquidPathInfoItem(QString::fromStdString(item.name), item.bStatus, siStatus);
        siStatus->setData(Qt::AlignCenter, Qt::TextAlignmentRole);

		m_liquidPathItemModel.appendRow({ siDev,siName,siStatus });
	}
}

void QStatusModule::UpdateUnitInfo(const QString& devName, const std::vector<tf::UnitInfo>& unitInfo)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	for (const tf::UnitInfo& item : unitInfo)
	{
        QString qstrUnitName = QString::fromStdString(item.name);
		// 遍历所有行
		bool useFalg = false;
		for (int i = 0; i < m_unitItemModel.rowCount(); i++)
		{
			if (m_unitItemModel.index(i, UNITINFO_COL::UNITINFO_MOUDLE).data().toString() == devName
				&& m_unitItemModel.index(i, UNITINFO_COL::UNITINFO_SUB_SYSTEM_NAME).data().toString() == qstrUnitName)
			{
				// 设置背景色
                QStandardItem* si = m_unitItemModel.item(i, UNITINFO_COL::UNITINFO_CURRENT_STATUS);
                if (qstrUnitName.contains(tr("值")))
                {
                    si->setText(QString::number(item.status));
                }
                else
                {
                    si->setText(item.status ? tr("正常") : tr("异常"));
                    si->setData(QColor(item.status ? "#ffffff" : UI_REAGENT_WARNCOLOR), Qt::UserRole + DELEGATE_COLOR_OFFSET_POS);
                }

				useFalg = true;
				continue;
			}
		}

		if (useFalg)
		{
			// 当前数据已处理
			continue;
		}

		// 模块
		QStandardItem* siDev = new QStandardItem(devName);
        siDev->setData(Qt::AlignCenter, Qt::TextAlignmentRole);

		// 部件名称
		QStandardItem* siName = new QStandardItem(qstrUnitName);
        siName->setData(Qt::AlignCenter, Qt::TextAlignmentRole);

		// 状态
        QStandardItem* siStatus;
        if (qstrUnitName.contains(tr("值")))
        {
            siStatus = new QStandardItem(QString::number(item.status));
        }
        else
        {
            siStatus = new QStandardItem(item.status ? tr("正常") : tr("异常"));
            siStatus->setData(QColor(item.status ? "#ffffff" : UI_REAGENT_WARNCOLOR), Qt::UserRole + DELEGATE_COLOR_OFFSET_POS);
        }

        siStatus->setData(Qt::AlignCenter, Qt::TextAlignmentRole);
		m_unitItemModel.appendRow({ siDev,siName,siStatus });
	}
}

void QStatusModule::UpdateCountInfo(const QString& devName, const QString& unitName)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 缓存选中行
    auto index = ui->table_view_count_right->currentIndex();
    int currentRow = -1;
    if (index.isValid())
    {
        currentRow = index.row();
    }

    //移除所有行
    m_countItemModel_right.removeRows(0, m_countItemModel_right.rowCount());

    // 加载数据,m_data<<设备名称，单元名称>，<设备序列号,<计数信息>>>
	for (const auto& iter : m_data)
	{
        if (iter.first.first != devName || iter.first.second != unitName)
        {
            continue;
        }

        for (auto& ci : iter.second.second)
        {
            // name:部件名称-单位
            QStringList tempList = QString::fromStdString(ci.name).split("-");

            //错误返回
            if (tempList.size() < 2)
            {
                ULOG(LOG_ERROR, "subName format error");
                continue;
            }
            //拼接字符串（若单独分割会导致 two-substrate - times 类型字符串不显示）
            QStringList spliceTempList;
            spliceTempList.push_back("");
            spliceTempList.push_back("");
            for (auto i = 0; i < tempList.size(); i++)
            {
                if (i != tempList.size() - 1)
                {
                    spliceTempList[0] += tempList[i];
                }
                else
                {
                    spliceTempList[1] = tempList[i];
                }
            }
            tempList.clear();
            tempList.push_back(spliceTempList[0]);
            tempList.push_back(spliceTempList[1]);

            // 部件名称
            QStandardItem* siSubName = new QStandardItem(tempList[0]);
            siSubName->setData(Qt::AlignCenter, Qt::TextAlignmentRole);

            // 部件序列号
            QStandardItem* siSubSn = new QStandardItem(QString::fromStdString(ci.item.subSn));
            siSubSn->setData(Qt::AlignCenter, Qt::TextAlignmentRole);

            // 当前使用数目
            QStandardItem* siCurrentCount = new QStandardItem(QString::number(ci.item.currentCount) + tempList[1]);
            siCurrentCount->setData(Qt::AlignCenter, Qt::TextAlignmentRole);

            // 累计使用数目
            QStandardItem* siCount = new QStandardItem(QString::number(ci.item.count) + tempList[1]);
            siCount->setData(Qt::AlignCenter, Qt::TextAlignmentRole);

            // 使用上限
            QStandardItem* siLimitCount;
            if (ci.limitCount == 0)
            {
                //显示"/"的部件
                siLimitCount = new QStandardItem("/");
            }
            else
            {
                siLimitCount = new QStandardItem(QString("<") + QString::number(ci.limitCount) + tempList[1]);
            }
            siLimitCount->setData(Qt::AlignCenter, Qt::TextAlignmentRole);

            // 重置时间
            QStandardItem* siResetTime = new QStandardItem(QString::fromStdString(ci.item.resetTime));
            siResetTime->setData(Qt::AlignCenter, Qt::TextAlignmentRole);

            bool CurrentTimeLimitStu = ci.item.currentCount >= ci.limitCount ? true : false;
            //判断当前使用数目置红
            //限制为0的是显示"/"的部件
            if (ci.limitCount != 0)
            {
                //限制值不为0则判断颜色
                siCurrentCount->setData(QColor(CurrentTimeLimitStu ? UI_REAGENT_WARNFONT : "#565656"), Qt::TextColorRole);
            }
            

            m_countItemModel_right.appendRow({ siSubName,siSubSn,siLimitCount,siCurrentCount,siResetTime, siCount });
        }
	}

    // 还原选中行
    if (currentRow < m_countItemModel_right.rowCount() && currentRow >= 0)
    {
        ui->table_view_count_right->selectRow(currentRow);
    }
}

///
/// @brief  更新设备状态信息
///
/// @param[in]    iMoudle:模块号
///               lstTempInfo:设备状态信息
/// @return void
///
/// @par History:
/// @li 7915/LeiDingXiang，2022年11月16日，新建函数
///
void QStatusModule::OnStatusInfoUpdate(const tf::UnitStatusInfo& statusInfo)
{
	ULOG(LOG_INFO, "%s(dev:%s), sCnt:%d.", __FUNCTION__, statusInfo.devSn.c_str(), statusInfo.countInfos.size());

	//模块名字
	auto spDev = CommonInformationManager::GetInstance()->GetDeviceInfo(statusInfo.devSn);
	if (spDev == nullptr)
	{
		ULOG(LOG_ERROR, "Failed to get deviceinfo.");
		return;
	}

    QString devName = QString::fromStdString(spDev->groupName + spDev->name);

	// 温度
	if (!statusInfo.temperautureInfos.empty())
	{
		UpdateTemperautureInfo(devName, statusInfo.temperautureInfos);
	}

	// 液路
	if (!statusInfo.liquidPathInfos.empty())
	{
		UpdateLiquidPathInfo(devName, statusInfo.liquidPathInfos);
	}

	// 单元
	if (!statusInfo.unitInfos.empty())
	{
		UpdateUnitInfo(devName, statusInfo.unitInfos);
	}

	// 计数
    for (const auto& iter : statusInfo.countInfos)
    {
        if (spDev->deviceType == ::tf::DeviceType::DEVICE_TYPE_ISE1005 && spDev->moduleCount > 1)
        {
            if (iter.first < 1)
            {
                ULOG(LOG_ERROR, "ISE moduleIndex[%d] error", iter.first);
                return;
            }
            else
            {
                // 获取屏蔽的项目,ISE多模块显示ISE-A,ISE-B,模块索引从1开始
                devName += "-";
                devName += 'A' + (iter.first - 1);
            }
        }

        for (const auto& ci : iter.second)
        {
            auto& cis = m_data[{devName, QString::fromStdString(ci.unitName)}].second;
            auto findKey = std::find_if(cis.begin(), cis.end(), [&ci](const tf::CountInfo& other){
                return ci.name == other.name;
            });

            if (findKey != cis.end())
            {
                findKey->item = ci.item;
            }
        }

        // 更新当前展示的视图
        auto index = ui->table_view_count_left->currentIndex();
        if (index.isValid() && isVisible())
        {
            QString viewDevName = m_countItemModel_left.item(index.row(), COUNTINFO_COL_LEFT::COUNTINFO_MOUDLE)->text();
            if (devName == viewDevName)
            {
                UpdateCountInfo(devName, m_countItemModel_left.item(index.row(), COUNTINFO_COL_LEFT::COUNTINFO_UNIT_NAME)->text());
            }           
        }
	}
}

void QStatusModule::PermissionOfPrintExportBtn()
{
    SEG_NO_LOGIN_RETURN;
    std::shared_ptr<UserInfoManager> userPms = UserInfoManager::GetInstance();

    QWidget* curTab = ui->tabWidget->currentWidget();
    if (curTab != Q_NULLPTR)
    {
        bool isShowPrint = false;
        bool isShowExport = false;

        QString objName = curTab->objectName();
        if (objName == "cupHistoryWidget")
        {
            isShowPrint = userPms->IsPermissionShow(PSM_INSTRU_CUPHIS_PRINT); // ? ui->flat_print->show() : ui->flat_print->hide();
            isShowExport = userPms->IsPermissionShow(PSM_INSTRU_CUPHIS_EXPORT);
        }
        else if (objName == "tabTemperature")
        {
            isShowPrint = userPms->IsPermissionShow(PSM_INSTRU_TEMPRATURE_PRINT);
            isShowExport = userPms->IsPermissionShow(PSM_INSTRUMENT_TEMPARATURE_STATUS_EXPORT);
        }
        else if (objName == "tabCount")
        {
            isShowPrint = userPms->IsPermissionShow(PSM_INSTRU_COUNTSTATU_PRINT);
            isShowExport = userPms->IsPermissionShow(PSM_INSTRUMENT_COUNT_STATU_EXPORT);
        }
        else if (objName == "tabLiquid")
        {
            isShowPrint = userPms->IsPermissionShow(PSM_INSTRUMENT_LEQUIT_STATU_PRINT);
            isShowExport = userPms->IsPermissionShow(PSM_INSTRUMENT_LEQUIT_STATU_EXPORT);
        }
        else if (objName == "tabBaseUnit")
        {
            isShowPrint = userPms->IsPermissionShow(PSM_INSTRUMENT_LOWERUNIT_STA_PRINT);
            isShowExport = userPms->IsPermissionShow(PSM_INSTRUMENT_LOWERUNIT_STA_EXPORT);
        }

        isShowPrint ? ui->flat_print->show() : ui->flat_print->hide();
        isShowExport ? ui->export_btn->show() : ui->export_btn->hide();
    }
}

///
/// @bref
///		权限变化响应
///
/// @par History:
/// @li 8276/huchunli, 2023年7月26日，新建函数
///
void QStatusModule::OnPermisionChanged()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;
    std::shared_ptr<UserInfoManager> userPms = UserInfoManager::GetInstance();

    int tabCount = ui->tabWidget->count();

    // 隐藏页签
    auto funcHideTab = [&](QWidget* tabWidget) {
        for (int i = 0; i < tabCount; ++i)
        {
            QWidget* curGet = ui->tabWidget->widget(i);
            if (curGet == tabWidget)
            {
                ui->tabWidget->removeTab(i);
                break;
            }
        }
    };

    // 显示页签
    auto funcShowTab = [&](QWidget* tabWidget, const QString& strName, int iPos) {
        for (int i = 0; i < tabCount; ++i)
        {
            QWidget* curGet = ui->tabWidget->widget(i);
            if (curGet == tabWidget)
            {
                break;
            }
        }
        ui->tabWidget->insertTab(iPos, tabWidget, strName);
    };

    // 计数状态-累计使用数目
    if (userPms->IsPermissionShow(PSM_INSTRU_SUMUSECOUNT))
    {
        ui->table_view_count_right->showColumn(COUNTINFO_COUNT);
    }
    else
    {
        ui->table_view_count_right->hideColumn(COUNTINFO_COUNT);
    }

    // 计数状态-重置时间
    if (userPms->IsPermissionShow(PSM_INSTRU_RESETTIME))
    {
        ui->table_view_count_right->showColumn(COUNTINFO_RESET_TIME);
    }
    else
    {
        ui->table_view_count_right->hideColumn(COUNTINFO_RESET_TIME);
    }

    // 温度
    bool bShow = userPms->IsPermissionShow(PSM_TEMPERATURE_SHOW);
    for (int i = 0; i < m_temperautureItemModel.rowCount(); i++)
    {
        QString name = m_temperautureItemModel.index(i, TEMPERAUTURE_COL::TEMPERAUTURE_SUB_SYSTEM_NAME).data().toString();
        if (name == tr("加热棒温度") || name == tr("清洗站温度")
            || name == tr("压缩机冷水温度") || name == tr("试剂盘探头温度"))
        {
            ui->table_view_temperature->setRowHidden(i, !bShow);
        }
    }

    // 重置
    userPms->IsPermissionShow(PSM_INSTRU_RESET) ? ui->reset_btn->show() : ui->reset_btn->hide();
    // 取消重置
    userPms->IsPermissionShow(PSM_INSTRU_UNRESET) ? ui->cancel_reset_btn->show() : ui->cancel_reset_btn->hide();

    // 打印 导出
    PermissionOfPrintExportBtn();

    // 计数-重置
    userPms->IsPermissionShow(PSM_INSTRU_RESET) ? ui->reset_btn->show() : ui->reset_btn->hide();
    // 计数-取消重置
    userPms->IsPermissionShow(PSM_INSTRU_UNRESET) ? ui->cancel_reset_btn->show() : ui->cancel_reset_btn->hide();
}

void QStatusModule::OnSelcetDevChanged()
{
    // 获取当前选中的设备
    QString name;
    for (auto& btn : ui->rabtn_widget->findChildren<QRadioButton*>())
    {
        if (btn->isChecked())
        {
            name = btn->text();
            break;
        }
    }

    if (ui->tabWidget->currentWidget()->objectName() == "cupHistoryWidget")
    {
        auto wid = qobject_cast<CupHistoryWidget *>(ui->tabWidget->currentWidget());
        wid->SetDevName(name);
    }
    else
    {
        FilterViewRowByDevName(name);
    }  
}

void QStatusModule::FilterViewRowByDevName(const QString& devName)
{
    m_strCurDevName = devName;
    auto view = ui->tabWidget->currentWidget()->findChild<QTableView*>();
    if (view->objectName().endsWith("right"))
    {
        view = ui->tabWidget->currentWidget()->findChild<QTableView*>("table_view_count_left");
    }

    if (view == nullptr || view->objectName() == "cup_info_view")
    {
        return;
    }

    auto model = view->model();
    for (int i=0; i<model->rowCount(); i++)
    {
        bool hid = !model->index(i, 0).data().toString().contains(devName);
        view->setRowHidden(i, hid && devName != tr("全部"));
    }
    
    for (int i = 0; i < model->rowCount(); i++)
    {
        if (!view->isRowHidden(i))
        {
            view->selectRow(i);
            break;
        }
    }
}

void QStatusModule::OnTabWidgetChangePage()
{
    PermissionOfPrintExportBtn();

    QString name;
    int showBtnNum = 0;
    bool isShow = false;
    for (auto& btn : ui->rabtn_widget->findChildren<QRadioButton*>())
    {
        int type = btn->property("type").toInt();

        // 反应杯历史仅可选择比色设备
        if (ui->tabWidget->currentWidget()->objectName() == "cupHistoryWidget")
        {
            isShow = type == ::tf::DeviceType::DEVICE_TYPE_C1000;
            btn->setVisible(isShow);

            if (isShow)
            {
                showBtnNum++;
            }
        }
        // 计数页面显示所有的按钮，
        else if(ui->tabWidget->currentWidget()->objectName() == "tabCount")
        {
            btn->show();
            showBtnNum++;
            isShow = true;
        }
        // 其他页面隐藏ISE
        else
        {
            isShow = type != ::tf::DeviceType::DEVICE_TYPE_ISE1005;
            btn->setVisible(isShow);

            if (isShow)
            {
                showBtnNum++;
            }
        }

        if (isShow && name.isEmpty())
        {
            btn->setChecked(true);
            name = btn->text();
        }
    }

    // 反应杯历史页面需显示的按钮只有一个时隐藏选择按钮
    if (ui->tabWidget->currentWidget()->objectName() == "cupHistoryWidget")
    {
        ui->rabtn_widget->setVisible(showBtnNum > 1);
    }
    //其他页面需显示的按钮少于三个时隐藏选择按钮
    else
    {
        ui->rabtn_widget->setVisible(showBtnNum > 2);
    }

    FilterViewRowByDevName(name);
}

///
/// @brief 响应打印按钮
///
/// @par History:
/// @li 6889/ChenWei，2023年12月15日，新建函数
///
void QStatusModule::OnPrintBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    int irect = -1;
    bool bSendData = false;
    auto wid = ui->tabWidget->currentWidget();

    QString strPrintTime = QDateTime::currentDateTime().toString(DictionaryQueryManager::GetUIDateFormat() + " hh:mm:ss");
    if (wid->objectName().startsWith("cupHistory"))
    {
        CupHistoryInfo info;
        ui->cupHistoryWidget->GetCupHistoryInfo(info);
        if (!info.vecRecord.empty())
        {
            info.strPrintTime = strPrintTime.toStdString();
            std::string strInfo = GetJsonString(info);
            std::string unique_id;
            irect = printcom::printcom_async_print(strInfo, unique_id);
            bSendData = true;
            ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);
        }
    }
    else if (wid->objectName().endsWith("Temperature"))     // 温度界面
    {
        TemperatureLog Log;
        GetTemperatureLog(Log);
        if (!Log.vecRecord.empty())
        {
            Log.strInstrumentModel = "i 6000";
            Log.strPrintTime = strPrintTime.toStdString();
            std::string strInfo = GetJsonString(Log);
            std::string unique_id;
            irect = printcom::printcom_async_print(strInfo, unique_id);
            bSendData = true;
            ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);
        }
    }
    else if (wid->objectName().endsWith("Count"))    // 计数界面
    {
        StatisticalInfo Log;
        GetCountInfo(Log);
        if (!Log.vecRecord.empty())
        {
            Log.strInstrumentModel = "i 6000";
            Log.strPrintTime = strPrintTime.toStdString();
            std::string strInfo = GetJsonString(Log);
            std::string unique_id;
            irect = printcom::printcom_async_print(strInfo, unique_id);
            bSendData = true;
            ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);
        }
    }
    else if (wid->objectName().endsWith("Liquid"))      // 液路
    {
        LiquidInfo info;
        GetLiquidInfo(info);
        if (!info.vecRecord.empty())
        {
            info.strPrintTime = strPrintTime.toStdString();
            std::string strInfo = GetJsonString(info);
            std::string unique_id;
            irect = printcom::printcom_async_print(strInfo, unique_id);
            bSendData = true;
            ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);
        }
    }
    else if (wid->objectName().endsWith("BaseUnit"))    // 下位机单元界面
    {
        LowerCpLog Log;
        GetUnitLog(Log);
        if (!Log.vecRecord.empty())
        {
            Log.strInstrumentModel = "i 6000";
            Log.strPrintTime = strPrintTime.toStdString();
            std::string strInfo = GetJsonString(Log);
            std::string unique_id;
            irect = printcom::printcom_async_print(strInfo, unique_id);
            bSendData = true;
            ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);
        }
    }

    // 弹框提示打印结果
    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(bSendData ? tr("数据已发送到打印机!") : tr("没有可打印的数据。")));
    pTipDlg->exec();

}

///
/// @brief 响应导出按钮
///
/// @par History:
/// @li 6889/ChenWei，2023年3月31日，新建函数
///
void QStatusModule::OnExportBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 弹出保存文件对话框
    QString strFilepath = QFileDialog::getSaveFileName(this, tr("保存为..."), QString(), tr("EXCEL files (*.xlsx);;CSV files (*.csv);;PDF files (*.pdf)"));
    if (strFilepath.isEmpty())
    {
        //ALOGI << "Export canceled!";
        return;
    }

    bool bRect = false;
    QFileInfo FileInfo(strFilepath);
    QString strSuffix = FileInfo.suffix();
    auto wid = ui->tabWidget->currentWidget();

    std::string strPrintTime = QDateTime::currentDateTime().toString(
        DictionaryQueryManager::GetUIDateFormat() + " hh:mm:ss").toStdString();

    if (wid->objectName().startsWith("cupHistory"))
    {
        CupHistoryInfo info;
        ui->cupHistoryWidget->GetCupHistoryInfo(info);

        if (strSuffix == "pdf")
        {
            info.strPrintTime = strPrintTime;
            std::string strInfo = GetJsonString(info);
            std::string unique_id;
            QString strDirPath = QCoreApplication::applicationDirPath();
            QString strUserTmplatePath = strDirPath + "/resource/exportTemplate/ExportChCupHistory.lrxml";
            int irect = printcom::printcom_async_assign_export(strInfo, strFilepath.toStdString(), strUserTmplatePath.toStdString(), unique_id);
            ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);
            bRect = true;
        }
        else
        {
            std::shared_ptr<FileExporter> pFileEpt(new FileExporter());
            bRect = pFileEpt->ExportCupHistoryInfo(info, strFilepath);
        }
    }
    else if (wid->objectName().endsWith("Temperature"))
    {
        TemperatureLog Log;
        GetTemperatureLog(Log);
        if (strSuffix == "pdf")
        {
            Log.strPrintTime = strPrintTime;
            std::string strInfo = GetJsonString(Log);
            ULOG(LOG_INFO, "Print datas : %s", strInfo);
            std::string unique_id;
            QString strDirPath = QCoreApplication::applicationDirPath();
            QString strUserTmplatePath = strDirPath + "/resource/exportTemplate/ExportTemperatureLog.lrxml";
            int irect = printcom::printcom_async_assign_export(strInfo, strFilepath.toStdString(), strUserTmplatePath.toStdString(), unique_id);
            ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);
            bRect = true;
        }
        else
        {
            std::shared_ptr<FileExporter> pFileEpt(new FileExporter());
            bRect = pFileEpt->ExportTemperatureInfo(Log, strFilepath);
        }
    }
    else if (wid->objectName().endsWith("Count"))
    {
        StatisticalInfo Info;
        GetCountInfo(Info);
        if (strSuffix == "pdf")
        {
            Info.strPrintTime = strPrintTime;
            std::string strInfo = GetJsonString(Info);
            ULOG(LOG_INFO, "Print datas : %s", strInfo);
            std::string unique_id;
            QString strDirPath = QCoreApplication::applicationDirPath();
            QString strUserTmplatePath = strDirPath + "/resource/exportTemplate/ExportStatisticalInfo.lrxml";
            int irect = printcom::printcom_async_assign_export(strInfo, strFilepath.toStdString(), strUserTmplatePath.toStdString(), unique_id);
            ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);
            bRect = true;
        }
        else
        {
            std::shared_ptr<FileExporter> pFileEpt(new FileExporter());
            bRect = pFileEpt->ExportCountInfo(Info, strFilepath);
        }
    }
    else if (wid->objectName().endsWith("Liquid"))
    {
        LiquidInfo info;
        GetLiquidInfo(info);

        if (strSuffix == "pdf")
        {
            info.strPrintTime = strPrintTime;
            std::string strInfo = GetJsonString(info);
            std::string unique_id;
            QString strDirPath = QCoreApplication::applicationDirPath();
            QString strUserTmplatePath = strDirPath + "/resource/exportTemplate/ExportLiquidStatus.lrxml";
            int irect = printcom::printcom_async_assign_export(strInfo, strFilepath.toStdString(), strUserTmplatePath.toStdString(), unique_id);
            ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);
            bRect = true;
        }
        else
        {
            std::shared_ptr<FileExporter> pFileEpt(new FileExporter());
            bRect = pFileEpt->ExportLiquidInfo(info, strFilepath);
        }
    }
    else if (wid->objectName().endsWith("BaseUnit"))
    {
        LowerCpLog Log;
        GetUnitLog(Log);
        if (strSuffix == "pdf")
        {
            Log.strPrintTime = strPrintTime;
            std::string strInfo = GetJsonString(Log);
            ULOG(LOG_INFO, "Print datas : %s", strInfo);
            std::string unique_id;
            QString strDirPath = QCoreApplication::applicationDirPath();
            QString strUserTmplatePath = strDirPath + "/resource/exportTemplate/ExportLowerCpLog.lrxml";
            int irect = printcom::printcom_async_assign_export(strInfo, strFilepath.toStdString(), strUserTmplatePath.toStdString(), unique_id);
            ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);
            bRect = true;
        }
        else
        {
            std::shared_ptr<FileExporter> pFileEpt(new FileExporter());
            bRect = pFileEpt->ExportUnitLog(Log, strFilepath);
        }
    }

    // 弹框提示导出结果
    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(bRect? tr("导出完成!") : tr("导出失败！")));
    pTipDlg->exec();
}

void QStatusModule::OnResetBtnClicked()
{
    // 当前选中行
    auto index = ui->table_view_count_right->currentIndex();
    if (!index.isValid())
    {
        return;
    }

    QString name = m_countItemModel_right.item(index.row(), COUNTINFO_SUB_SYSTEM_NAME)->text();
    QString subSn = m_countItemModel_right.item(index.row(), COUNTINFO_SUB_SN)->text();

    m_pResetCountInfoDlg->showDlg(name, subSn);
}

void QStatusModule::OnResetDlgSaveClicked()
{
    QString name;
    QString subSn;
    m_pResetCountInfoDlg->GetData(name, subSn);

    // 当前选中行
    auto index_left = ui->table_view_count_left->currentIndex();
    if (!index_left.isValid())
    {
        TipDlg(tr("重置失败")).exec();
        return;
    }

    // 单元名称
    QString unitName = m_countItemModel_left.item(index_left.row(), COUNTINFO_UNIT_NAME)->text();
    // 设备名称和模块索引
    auto nameAndModuleIndex = m_countItemModel_left.item(index_left.row(), COUNTINFO_MOUDLE)->text().split("-");
    // 遍历选定设备下选定的单元
    for (auto& iter : m_data[{nameAndModuleIndex[0], unitName}].second)
    {
        string sn = m_data[{nameAndModuleIndex[0], unitName}].first;

        // 找到选定的部件
        auto oldName = QString::fromStdString(iter.name.substr(0, iter.name.find_last_of("-")));
        if (oldName == name)
        {
            int moduleIndex = 1;
            const auto& spDev = CommonInformationManager::GetInstance()->GetDeviceInfo(sn);
            if (spDev != nullptr && spDev->moduleCount > 1)
            {
                // 获取屏蔽的项目,ISE多模块显示ISE-A,ISE-B,数据库中存储模块索引，从1开始
                moduleIndex = nameAndModuleIndex[nameAndModuleIndex.size() - 1].toLatin1()[0];
                moduleIndex -= 64;
            }

            iter.item.subSn = subSn.toStdString();
            if (!DcsControlProxy::GetInstance()->SetLowerComponentCount(true, iter, sn, moduleIndex))
            {
                TipDlg(tr("重置失败")).exec();
            }
        }
    }
}

void QStatusModule::OnCancelResetBtnClicked()
{
    if (TipDlg(tr("取消重置"), tr("确定取消重置？"), TipDlgType::TWO_BUTTON).exec() == QDialog::Rejected)
    {
        return;
    }

    // 当前选中行
    auto& index_left = ui->table_view_count_left->currentIndex();
    auto& index_right = ui->table_view_count_right->currentIndex();
    if (!index_left.isValid() || !index_right.isValid())
    {
        TipDlg(tr("取消重置失败")).exec();
        return;
    }

    // 单元名称
    QString unitName = m_countItemModel_left.item(index_left.row(), COUNTINFO_UNIT_NAME)->text();
    // 部件名称
    string name = m_countItemModel_right.item(index_right.row(), COUNTINFO_SUB_SYSTEM_NAME)->text().toStdString();

    // 设备名称和模块索引
    auto nameAndModuleIndex = m_countItemModel_left.item(index_left.row(), COUNTINFO_MOUDLE)->text().split("-");

    // 遍历选定设备下选定的单元
    for (auto& item : m_data[{nameAndModuleIndex[0], unitName}].second)
    {
        string sn = m_data[{nameAndModuleIndex[0], unitName}].first;
        // 找到选定的部件
        if (GetUnitName(item.name) == name)
        {
            int moduleIndex = 1;
            const auto& spDev = CommonInformationManager::GetInstance()->GetDeviceInfo(sn);
            if (spDev != nullptr && spDev->moduleCount > 1)
            {
                // 获取屏蔽的项目,ISE多模块显示ISE-A,ISE-B,数据库中存储模块索引，从1开始
                moduleIndex = nameAndModuleIndex[nameAndModuleIndex.size() - 1].toLatin1()[0];
                moduleIndex -= 64;
            }

            if (!DcsControlProxy::GetInstance()->SetLowerComponentCount(false, item, sn, moduleIndex))
            {
                TipDlg(tr("取消重置失败")).exec();
            }
        }
    }
}

///
/// @brief  获取温度日志
/// 
/// @return void
///
/// @par History:
/// @li 6889/ChenWei，2023年12月15日，新建函数
///
void QStatusModule::GetTemperatureLog(TemperatureLog& Log)
{
    for (int i = 0; i < m_temperautureItemModel.rowCount(); i++)
    {
        // 跳过空行
        if (m_temperautureItemModel.item(i, 0) == Q_NULLPTR || m_temperautureItemModel.item(i, 0)->text().isEmpty())
        {
            continue;
        }

        // 过滤掉非选中的模块
        QString strDevName = m_temperautureItemModel.item(i, TEMPERAUTURE_MOUDLE)->text();
        if (m_strCurDevName != tr("全部") && m_strCurDevName != strDevName)
        {
            continue;
        }

        TemperatureInfo info;
        info.strModule = strDevName.toStdString();                // 模块
        if (m_temperautureItemModel.item(i, TEMPERAUTURE_SUB_SYSTEM_NAME) != Q_NULLPTR)
            info.strPartName = m_temperautureItemModel.item(i, TEMPERAUTURE_SUB_SYSTEM_NAME)->text().toStdString(); // 部件名称

        if (m_temperautureItemModel.item(i, TEMPERAUTURE_CURRENT_TEMP) != Q_NULLPTR)
            info.strCurTem = m_temperautureItemModel.item(i, TEMPERAUTURE_CURRENT_TEMP)->text().toStdString();      // 当前温度

        if (m_temperautureItemModel.item(i, TEMPERAUTURE_TEMP_RANGE) != Q_NULLPTR)
            info.strTemControlRange = m_temperautureItemModel.item(i, TEMPERAUTURE_TEMP_RANGE)->text().toStdString();// 温控范围

        if (m_temperautureItemModel.item(i, TEMPERAUTURE_STATUS) != Q_NULLPTR)
            info.strTemControlStatu = m_temperautureItemModel.item(i, TEMPERAUTURE_STATUS)->text().toStdString();   // 温控状态
        
        Log.vecRecord.push_back(std::move(info));
    }
}

///
/// @brief  获取计数信息
/// 
/// @return void
///
/// @par History:
/// @li 6889/ChenWei，2023年12月15日，新建函数
///
void QStatusModule::GetCountInfo(StatisticalInfo& Info)
{
    for (auto& iter : m_data)
    {
        for (auto& ci : iter.second.second)
        {
            auto pos = ci.name.find_last_of("-");
            if (pos == std::string::npos)
            {
                continue;
            }

            // 过滤掉非选中的模块
            QString strDevName = iter.first.first;
            if (m_strCurDevName != tr("全部") && m_strCurDevName != strDevName)
            {
                continue;
            }

            StatisticalUnit Unit;
            Unit.strModule = iter.first.first.toStdString();                // 模块
            Unit.strUnitName = ci.unitName;                                 // 单元名称
            Unit.strPartName = ci.name.substr(0,pos);                       // 部件名称
            Unit.strSN = ci.item.subSn;                                     // 部件序列号
            string unit = ci.name.substr(pos + 1);
            Unit.strCurUseNum = to_string(ci.item.currentCount) + unit;     // 当前使用数目
            Unit.strAllUseNum = to_string(ci.item.count) + unit;            // 累计使用数目
            Unit.strUseUpperLimit = to_string(ci.limitCount) + unit;        // 使用上限
            Unit.strResetTime = ci.item.resetTime;                          // 重置时间
            Info.vecRecord.push_back(std::move(Unit));
        }
    }
}

void QStatusModule::GetLiquidInfo(LiquidInfo& info)
{
    for (int i=0; i<m_liquidPathItemModel.rowCount(); i++)
    {
        LiquidItem item;
        item.strModule = m_liquidPathItemModel.item(i, LIUQUIPATH_MOUDLE)->text().toStdString();
        item.strName = m_liquidPathItemModel.item(i, LIUQUIPATH_SUB_SYSTEM_NAME)->text().toStdString();
        item.strStatus = m_liquidPathItemModel.item(i, LIUQUIPATH_CURRENT_STATUS)->text().toStdString();
        
        info.vecRecord.push_back(std::move(item));
    }   
}

///
/// @brief  获取下位机单元日志
/// 
/// @return void
///
/// @par History:
/// @li 6889/ChenWei，2023年12月15日，新建函数
///
void QStatusModule::GetUnitLog(LowerCpLog& Log)
{
    for (int i = 0; i < m_unitItemModel.rowCount(); i++)
    {
        // 跳过空行
        if (m_unitItemModel.item(i, 0) == Q_NULLPTR || m_unitItemModel.item(i, 0)->text().isEmpty())
            continue;

        // 过滤掉非选中的模块
        QString strDevName = m_unitItemModel.item(i, 0)->text();
        if (m_strCurDevName != tr("全部") && m_strCurDevName != strDevName)
        {
            continue;
        }

        LowerCpUnit Unit;
        Unit.strIndex = QString::number(i + 1).toStdString();
        Unit.strModule = m_unitItemModel.item(i, 0)->text().toStdString();      // 模块

        if (m_unitItemModel.item(i, 1) != Q_NULLPTR)
            Unit.strName = m_unitItemModel.item(i, 1)->text().toStdString();        // 名称

        if (m_unitItemModel.item(i, 2) != Q_NULLPTR)
        Unit.strState = m_unitItemModel.item(i, 2)->text().toStdString();       // 状态

        Log.vecRecord.push_back(std::move(Unit));
    }
}

void QStatusModule::InitTemperautureTableView(const std::vector<::tf::DeviceType::type>& types,
    const std::map<QString, ::tf::DeviceType::type>& devNameAndTypes)
{
    // 获取配置信息
    std::map<::tf::DeviceType::type, std::vector< ::tf::TemperautureInfo>> result;
    DcsControlProxy::GetInstance()->GetTemperautureConfigInfo(result, types);

    // 遍历设备
    for (const auto& item : devNameAndTypes)
    {
        // 按设备类型遍历配置信息
        for (const auto& iter : result)
        {
            // 设备类型不一致，温度页面不显示ISE,跳过
            if (iter.first != item.second || iter.first == ::tf::DeviceType::DEVICE_TYPE_ISE1005)
            {
                continue;
            }

            for (const auto& ti : iter.second)
            {
                // 模块
                QStandardItem* siDev = new QStandardItem(item.first);
                siDev->setData(Qt::AlignCenter, Qt::TextAlignmentRole);

                // 部件名称
                QStandardItem* siName = new QStandardItem(QString::fromStdString(ti.subDevName));
                siName->setData(Qt::AlignCenter, Qt::TextAlignmentRole);

                // 当前温度
                QStandardItem* siTemp = new QStandardItem();
                siTemp->setData(Qt::AlignCenter, Qt::TextAlignmentRole);

                // 温控范围
                QString rang = QString::number(double(ti.tempLower) / TEMPERAUTURE_FACTOR, 'f', 1) + QString::fromStdString("℃≤ T ≤")
                    + QString::number(double(ti.tempUpper) / TEMPERAUTURE_FACTOR, 'f', 1) + QString::fromStdString("℃");
                QStandardItem* siRang = new QStandardItem(rang);
                siRang->setData(Qt::AlignCenter, Qt::TextAlignmentRole);

                // 设置温控状态
                QStandardItem*statusSi = new QStandardItem();
                statusSi->setData(Qt::AlignCenter, Qt::TextAlignmentRole);

                m_temperautureItemModel.appendRow({ siDev,siName,siTemp,siRang, statusSi });
            }
        }
    }
}

void QStatusModule::InitLiquidPathTableView(const std::vector< ::tf::DeviceType::type>& types,
    const std::map<QString, ::tf::DeviceType::type>& devNameAndTypes)
{
    // 获取配置信息
    std::map<::tf::DeviceType::type, std::vector<std::string>> result;
    DcsControlProxy::GetInstance()->GetLiquidPathConfigInfo(result, types);

    // 遍历设备
    for (const auto& item : devNameAndTypes)
    {
        // 按设备类型遍历配置信息
        for (const auto& iter : result)
        {
            // 设备类型不一致，液路页面不显示ISE,跳过
            if (iter.first != item.second || iter.first == ::tf::DeviceType::DEVICE_TYPE_ISE1005)
            {
                continue;
            }

            for (const auto& subName : iter.second)
            {
                // 模块
                QStandardItem* siDev = new QStandardItem(item.first);
                siDev->setData(Qt::AlignCenter, Qt::TextAlignmentRole);

                // 部件名称
                QStandardItem* siName = new QStandardItem(QString::fromStdString(subName));
                siName->setData(Qt::AlignCenter, Qt::TextAlignmentRole);

                // 状态
                QStandardItem* siStatus = new QStandardItem();
                siStatus->setData(Qt::AlignCenter, Qt::TextAlignmentRole);

                m_liquidPathItemModel.appendRow({ siDev,siName,siStatus });
            }
        }
    }
}

void QStatusModule::InitUnitTableView(const std::vector< ::tf::DeviceType::type>& types,
    const std::map<QString, ::tf::DeviceType::type>& devNameAndTypes)
{
    // 获取配置信息
    std::map<::tf::DeviceType::type, std::vector<std::string>> result;
    DcsControlProxy::GetInstance()->GetUnitConfigInfo(result, types);

    // 遍历设备
    for (const auto& item : devNameAndTypes)
    {
        // 按设备类型遍历配置信息
        for (const auto& iter : result)
        {
            // 设备类型不一致，单元页面不显示ISE设备，跳过
            if (iter.first != item.second || iter.first == ::tf::DeviceType::DEVICE_TYPE_ISE1005)
            {
                continue;
            }

            for (const auto& subName : iter.second)
            {
                // 模块
                QStandardItem* siDev = new QStandardItem(item.first);
                siDev->setData(Qt::AlignCenter, Qt::TextAlignmentRole);

                // 部件名称
                QStandardItem* siName = new QStandardItem(QString::fromStdString(subName));
                siName->setData(Qt::AlignCenter, Qt::TextAlignmentRole);

                // 状态
                QStandardItem* siStatus = new QStandardItem();
                siStatus->setData(Qt::AlignCenter, Qt::TextAlignmentRole);

                m_unitItemModel.appendRow({ siDev,siName,siStatus });
            }
        }
    }
}

void QStatusModule::InitCountTableView(const std::vector< ::tf::DeviceType::type>& types,
    const std::map<QString, ::tf::DeviceType::type>& devNameAndTypes)
{
    std::map<::tf::DeviceType::type, std::vector<::tf::CountInfo>> result;
    DcsControlProxy::GetInstance()->GetCountConfigInfo(result, types);

    // 初始化缓存
    auto& CIM = CIM_INSTANCE;
    for (auto& iter : devNameAndTypes)
    {
        for (auto& cis : result)
        {
            // 设备类型不一致，跳过
            if (iter.second != cis.first)
            {
                continue;
            }

            for (auto& ci : cis.second)
            {
                QString name = iter.first;
                auto devNameAndUnitName = make_pair(name, QString::fromStdString(ci.unitName));
                const auto& m_dataIter = m_data.find(devNameAndUnitName);
                if (m_dataIter == m_data.end())
                {
                    // ISE设备名称特殊处理
                    if (cis.first == ::tf::DeviceType::DEVICE_TYPE_ISE1005)
                    {
                        name = name.split("-").first();
                    }

                    const auto& spDev = CIM->GetDeviceInfoByDeviceName(name.toStdString());
                    m_data[devNameAndUnitName] = { spDev->deviceSN,{ ci } };
                }
                else
                {
                    m_data[devNameAndUnitName].second.push_back(ci);
                }
            }
        }
    }

    // 初始化左侧表格
    int row = m_countItemModel_left.rowCount();
    for (auto& item : m_data)
    {
        // 模块
        AddTextToTableView((&m_countItemModel_left), row, COUNTINFO_COL_LEFT::COUNTINFO_MOUDLE, item.first.first);
        // 单元名称
        AddTextToTableView((&m_countItemModel_left), row, COUNTINFO_COL_LEFT::COUNTINFO_UNIT_NAME, item.first.second);

        row++;
    }

    connect(ui->table_view_count_left->selectionModel(), &QItemSelectionModel::currentChanged, this, 
        [&](const QModelIndex &current, const QModelIndex &previous)
    {
        if (current.isValid())
        {
            QString devName = m_countItemModel_left.item(current.row(), COUNTINFO_COL_LEFT::COUNTINFO_MOUDLE)->text();
            QString unitName = m_countItemModel_left.item(current.row(), COUNTINFO_COL_LEFT::COUNTINFO_UNIT_NAME)->text();
            UpdateCountInfo(devName, unitName);
        }
    });
}


void QStatusModule::InitStatusInfoUpdate()
{
    std::vector< ::tf::DeviceType::type> types;
    const auto& CIM = CommonInformationManager::GetInstance();

    auto softType = CIM->GetSoftWareType();
    if (softType == CHEMISTRY || softType == CHEMISTRY_AND_IMMUNE)
    {
        types.emplace_back(::tf::DeviceType::DEVICE_TYPE_C1000);
        if (CIM->HasIseDevice())
        {
            types.emplace_back(::tf::DeviceType::DEVICE_TYPE_ISE1005);
        }
    } 

    if (softType == IMMUNE || softType == CHEMISTRY_AND_IMMUNE)
    {
        types.emplace_back(::tf::DeviceType::DEVICE_TYPE_I6000);
    }

    // 构造设备信息map<<设备名称，设备序列号>，设备类型>
    map<QString, ::tf::DeviceType::type> devNames;

    // 获取设备信息
    const auto& devices = CommonInformationManager::GetInstance()->GetDeviceFromType(types);

    for (auto& dev : devices)
    {
        QString devName = QString::fromStdString(dev->groupName + dev->name);

        //组装模块-序列号信息
        if (m_tempData.count(devName) == 0)
        {
            m_tempData[devName] = QString::fromStdString(dev->deviceSN);
        }

        // ISE设备名字特殊处理
        if (dev->moduleCount > 1)
        {
            for (int i = 0; i < dev->moduleCount; i++)
            {
                // 获取屏蔽的项目,ISE多模块显示ISE-A,ISE-B,模块索引从1开始
                QString name = (devName + "-") + ('A' + i);
                devNames[name] = dev->deviceType;
            }
        }
        else
        {
            devNames[devName] = dev->deviceType;
        }
    }

    InitUnitTableView(types, devNames);
    InitCountTableView(types, devNames);
    InitLiquidPathTableView(types, devNames);
    InitTemperautureTableView(types, devNames);
}

///
/// @brief  获取取消分割后的部件名称
/// 
/// tfCountInfoName thrift结构体信息
/// 
/// @return std::string
///
/// @par History:
/// @li 7915/LeiDingXiang，2023年12月15日，新建函数
///
std::string QStatusModule::GetUnitName(const std::string& tfCountInfoName)
{
    QStringList tempList = QString::fromStdString(tfCountInfoName).split("-");

    //错误返回
    if (tempList.size() < 2)
    {
        ULOG(LOG_ERROR, "subName format error");
    }
    //拼接字符串（若单独分割会导致 two-substrate - times 类型字符串不显示）
    QStringList spliceTempList;
    spliceTempList.push_back("");
    spliceTempList.push_back("");
    for (auto i = 0; i < tempList.size(); i++)
    {
        if (i != tempList.size() - 1)
        {
            spliceTempList[0] += tempList[i];
        }
        else
        {
            spliceTempList[1] = tempList[i];
        }
    }
    tempList.clear();
    tempList.push_back(spliceTempList[0]);
    tempList.push_back(spliceTempList[1]);

    return tempList[0].toStdString();
}
