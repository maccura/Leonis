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
/// @file     DetectionSetDlg.cpp
/// @brief    检测设置弹窗
///
/// @author   7951/LuoXin
/// @date     2022年12月08日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2022年12月08日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "DetectionSetDlg.h"
#include "ui_DetectionSetDlg.h"

#include <QGroupBox>
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/CommonInformationManager.h"
#include "shared/tipdlg.h"
#include "shared/uicommon.h"
#include "shared/mcpixmapitemdelegate.h"
#include "manager/DictionaryQueryManager.h"
#include "thrift/im/ImLogicControlProxy.h"
#include "src/public/SerializeUtil.hpp"
#include "src/public/im/ImConfigDefine.h"
#include "src/public/im/ImConfigSerialize.h"
#include "src/common/common.h"
#include "src/common/Mlog/mlog.h"
#include "manager/UserInfoManager.h"

DetectionSetDlg::DetectionSetDlg(QWidget *parent /*= Q_NULLPTR*/)
    : BaseDlg(parent)
    , ui(new Ui::DetectionSetDlg)
    , m_ciTblDefaultRowCnt(4)
    , m_ciTblDefaultColCnt(4)
{
    ui->setupUi(this);

	// 初始字符串信息
	m_mapImDetectModeName[im::tf::DetectMode::DETECT_MODE_ROUTINE] = QString(tr("常规模式"));
	m_mapImDetectModeName[im::tf::DetectMode::DETECT_MODE_STAT] = QString(tr("STAT模式"));
	m_mapImDetectModeName[im::tf::DetectMode::DETECT_MODE_QUICK] = QString(tr("快速模式"));

	// 初始化控件
    InitCtrls();
}

DetectionSetDlg::~DetectionSetDlg()
{

}

void DetectionSetDlg::LoadDataToCtrls()
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);
    // 清空检测设置缓存
    m_mapDevDetectMode.clear();

    // 获取所有设备列表
    auto mapAllDev = CommonInformationManager::GetInstance()->GetDeviceMaps();
    for (auto it = mapAllDev.begin(); it != mapAllDev.end(); it++)
    {
        // 轨道则跳过
        if (it->second->deviceType == tf::DeviceType::DEVICE_TYPE_TRACK)
        {
            continue;
        }

        // 不为免疫则跳过
        if (it->second->deviceClassify != tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE)
        {
            continue;
        }

        // 获取扩展信息
        ImDeviceOtherInfo devOtherInfo;
        if (!DecodeJson(devOtherInfo, it->second->otherInfo))
        {
            ULOG(LOG_ERROR, "%s DecodeJson ImDeviceOtherInfo Failed", it->second->name);
            continue;
        }

        // 记录到缓存中
        m_mapDevDetectMode[it->second->deviceSN] = devOtherInfo.iDetectMode;
    }

    // 获取检测设置
    DetectionSetting detectionSetting;
    if (!DictionaryQueryManager::GetDetectionConfig(detectionSetting))
    {
        ULOG(LOG_ERROR, "Failed to get detection config.");
        return;
    }

    // 模式
    for (auto box : QWidget::findChildren<QRadioButton*>())
    {
        box->setChecked(box->objectName().contains(QString::number(detectionSetting.testMode)));
    }

    ui->scan_tube_barcode_ckbox->setChecked(detectionSetting.scanTubeBarcode);
    ui->rotate_tube_ckbox->setChecked(detectionSetting.rotateTube);

    // 加载检测模式
    int iRow = 0;
    for (auto it = m_mapDevDetectMode.begin(); it != m_mapDevDetectMode.end(); it++)
    {
        // 如果行数不足，则行数增加
        if (ui->DetModeTbl->rowCount() <= iRow)
        {
            ui->DetModeTbl->setRowCount(iRow + 1);
        }

        // 如果在加样停或运行状态，则不允许修改
        auto spStuTfDevInfo = CommonInformationManager::GetInstance()->GetDeviceInfo(it->first);
        if (spStuTfDevInfo == Q_NULLPTR)
        {
            continue;
        }

        // 设置设备名
        ui->DetModeTbl->setItem(iRow, DetectModeHeader::Dmh_Device, new QTableWidgetItem(QString::fromStdString(spStuTfDevInfo->name)));
        ui->DetModeTbl->item(iRow, DetectModeHeader::Dmh_Device)->setData(Qt::UserRole, QString::fromStdString(it->first));

        // 取消勾选检测模式
        ui->DetModeTbl->setItem(iRow, DetectModeHeader::Dmh_RoutineMode, new QTableWidgetItem());
        ui->DetModeTbl->setItem(iRow, DetectModeHeader::Dmh_QuickMode, new QTableWidgetItem());
        ui->DetModeTbl->setItem(iRow, DetectModeHeader::Dmh_StatMode, new QTableWidgetItem());

        // 勾选对应模式列
        int iSelCol = -1;
        if (it->second == im::tf::DetectMode::DETECT_MODE_ROUTINE)
        {
            iSelCol = DetectModeHeader::Dmh_RoutineMode;
        }
        else if (it->second == im::tf::DetectMode::DETECT_MODE_QUICK)
        {
            iSelCol = DetectModeHeader::Dmh_QuickMode;
        }
        else if (it->second == im::tf::DetectMode::DETECT_MODE_STAT)
        {
            iSelCol = DetectModeHeader::Dmh_StatMode;
        }
        else
        {
            iSelCol = -1;
        }

        // 如果有勾选列，则勾选
        if (iSelCol > 0)
        {
            ui->DetModeTbl->item(iRow, iSelCol)->setIcon(QIcon(":/Leonis/resource/image/icon-select.png"));
        }

        // 行号自增
        ++iRow;
    }

    // 文本居中
    SetTblTextAlign(ui->DetModeTbl, Qt::AlignCenter);

	// 暂存设置
	m_detectionSetting = detectionSetting;
}

void DetectionSetDlg::InitCtrls()
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);

    // 设置标题
    SetTitleName(tr("检测设置"));

    // 初始化表格
    // 初始化行数列数
    ui->DetModeTbl->setRowCount(m_ciTblDefaultRowCnt);
    ui->DetModeTbl->setColumnCount(m_ciTblDefaultColCnt);

    // 设置表格选中模式为行选中，不可多选
    ui->DetModeTbl->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->DetModeTbl->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->DetModeTbl->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 表头不高亮
    ui->DetModeTbl->horizontalHeader()->setHighlightSections(false);

    // 设置表头
    // 设置表头名称, 和枚举值顺序对应
    std::vector<std::pair<DetectModeHeader, QString>> detectModeHeader = {
        { Dmh_Device, tr("仪器") },
        { Dmh_RoutineMode, m_mapImDetectModeName[im::tf::DetectMode::DETECT_MODE_ROUTINE] },
        { Dmh_QuickMode, m_mapImDetectModeName[im::tf::DetectMode::DETECT_MODE_QUICK] },
        { Dmh_StatMode, m_mapImDetectModeName[im::tf::DetectMode::DETECT_MODE_STAT] }
    };

    QStringList strHeaderList;
    for (auto it = detectModeHeader.begin(); it != detectModeHeader.end(); ++it)
    {
        strHeaderList.append(it->second);
    }
    ui->DetModeTbl->setHorizontalHeaderLabels(strHeaderList);

    // 常规模式、快速模式、STAT模式显示蓝色
    ui->DetModeTbl->horizontalHeaderItem(Dmh_RoutineMode)->setData(Qt::TextColorRole, UI_TEXT_COLOR_HEADER_MARK);
    ui->DetModeTbl->horizontalHeaderItem(Dmh_QuickMode)->setData(Qt::TextColorRole, UI_TEXT_COLOR_HEADER_MARK);
    ui->DetModeTbl->horizontalHeaderItem(Dmh_StatMode)->setData(Qt::TextColorRole, UI_TEXT_COLOR_HEADER_MARK);

    // 设置代理
    ui->DetModeTbl->setItemDelegateForColumn(Dmh_RoutineMode, new McPixmapItemDelegate(this));
    ui->DetModeTbl->setItemDelegateForColumn(Dmh_QuickMode, new McPixmapItemDelegate(this));
    ui->DetModeTbl->setItemDelegateForColumn(Dmh_StatMode, new McPixmapItemDelegate(this));

    // 水平表头不显示
    ui->DetModeTbl->verticalHeader()->setVisible(false);
    ResizeTblColToContent(ui->DetModeTbl);

    // 没有免疫设备隐藏“检测模式”
    if (CommonInformationManager::GetInstance()->GetSoftWareType() == CHEMISTRY)
    {
        ui->label_3->hide();
        ui->DetModeTbl->hide();
        ui->line_h_1->hide();
        ui->line_h_2->hide();
        ui->line_v_1->hide();
        ui->line_v_2->hide();
        int moveHight = ui->label_3->y() - ui->DetModeTbl->y() + ui->DetModeTbl->height();
        ui->save_btn->move(ui->save_btn->x(), ui->save_btn->y()- moveHight);
        ui->cancel_btn->move(ui->cancel_btn->x(), ui->cancel_btn->y() - moveHight);
        resize(width(), height() - moveHight);
    }

    // 保存按钮
    connect(ui->save_btn, &QPushButton::clicked, this, &DetectionSetDlg::OnSaveBtnClicked);

    // 条码模式必须扫描样本管
    for (auto& raBtn : findChildren<QRadioButton*>())
    {
        connect(raBtn, &QRadioButton::toggled, this, [&](bool status)
        {
            if (status)
            {
                QRadioButton* btn = qobject_cast<QRadioButton *>(sender());                
                ui->scan_tube_barcode_ckbox->setEnabled(!btn->objectName().contains("barcode")
                    && UserInfoManager::GetInstance()->IsPermisson(PSM_TESTSET_FUNCSWITCH));

                if (btn->objectName().split("_").last().toInt() == m_detectionSetting.testMode)
                {
                    ui->scan_tube_barcode_ckbox->setChecked(m_detectionSetting.scanTubeBarcode);
                    ui->rotate_tube_ckbox->setChecked(m_detectionSetting.rotateTube);
                }
                else
                {
                    ui->scan_tube_barcode_ckbox->setChecked(btn->objectName().contains("barcode"));
                    ui->rotate_tube_ckbox->setChecked(btn->objectName().contains("barcode"));
                }
            }
        });
    }

    // 检测模式表单元格被点击
    connect(ui->DetModeTbl, SIGNAL(clicked(const QModelIndex&)), this, SLOT(OnDetectTableItemClicked(const QModelIndex&)));

    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, UpdateCtrlsEnabled);
}

void DetectionSetDlg::OnSaveBtnClicked()
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);

    // 检查中禁止检查模式，待dcs实现检测中修改检测模式后放开此限制
    for (const auto& spDev : CommonInformationManager::GetInstance()->GetDeviceMaps())
    {
        if ((spDev.second->deviceClassify == ::tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY
            || spDev.second->deviceClassify == ::tf::AssayClassify::ASSAY_CLASSIFY_ISE)
            && devIsRun(*(spDev.second)))
        {
            TipDlg(tr("保存失败"), tr("请在待机或停机状态下修改检测模式！")).exec();
            return;
        }
    }

	// 先检查是否可以运行修改参数
	if (!EnableModifyConfig())
	{
		return;
	}

    // 进样模式
	DetectionSetting oldDetectSet = m_detectionSetting;
	QString strTestModName("");
    for (auto btn : QWidget::findChildren<QRadioButton*>())
    {
		// 如果勾选了对应的模式
        if (btn->isChecked())
        {
            m_detectionSetting.testMode = btn->objectName().split("_").constLast().toInt();
			strTestModName = btn->text();
        }
    }
    m_detectionSetting.scanTubeBarcode = ui->scan_tube_barcode_ckbox->isChecked();
    m_detectionSetting.rotateTube = ui->rotate_tube_ckbox->isChecked();

    // 记录检测模式更新的设备
    std::vector<std::string> vecUpdateDevSn;
    std::vector<std::string> vecAllUpdateDevSn;
    QMap<QString, im::tf::DetectMode::type> mapUpdateInfo;

    // 遍历表格，加载检测模式
    for (int iRow = 0; iRow < ui->DetModeTbl->rowCount(); iRow++)
    {
        QTableWidgetItem* pItem = Q_NULLPTR;
        pItem = ui->DetModeTbl->item(iRow, Dmh_Device);
        if (pItem == Q_NULLPTR)
        {
            continue;
        }

        // 获取设备序列号
        std::string strDevSn = pItem->data(Qt::UserRole).toString().toStdString();
        im::tf::DetectMode::type enOldMode = im::tf::DetectMode::DETECT_MODE_ROUTINE;
        im::tf::DetectMode::type enNewMode = im::tf::DetectMode::DETECT_MODE_ROUTINE;
        auto it = m_mapDevDetectMode.find(strDevSn);
        if (it != m_mapDevDetectMode.end())
        {
            enOldMode = im::tf::DetectMode::type(it->second);
        }

        // 获取设备检测模式
        pItem = ui->DetModeTbl->item(iRow, Dmh_RoutineMode);
        if (pItem != Q_NULLPTR && !pItem->icon().isNull())
        {
            enNewMode = im::tf::DetectMode::DETECT_MODE_ROUTINE;
        }

        // 获取设备检测模式
        pItem = ui->DetModeTbl->item(iRow, Dmh_QuickMode);
        if (pItem != Q_NULLPTR && !pItem->icon().isNull())
        {
            enNewMode = im::tf::DetectMode::DETECT_MODE_QUICK;
        }

        // 获取设备检测模式
        pItem = ui->DetModeTbl->item(iRow, Dmh_StatMode);
        if (pItem != Q_NULLPTR && !pItem->icon().isNull())
        {
            enNewMode = im::tf::DetectMode::DETECT_MODE_STAT;
        }

        // 如果检测模式没有更新，则忽略
        if (enNewMode == enOldMode)
        {
            continue;
        }

        // 否则更新模式
        m_mapDevDetectMode[strDevSn] = enNewMode;
        vecAllUpdateDevSn.push_back(strDevSn);

        // 如果是STAT模式改变，则记入更新
        if ((enNewMode == im::tf::DetectMode::DETECT_MODE_STAT) || (enOldMode == im::tf::DetectMode::DETECT_MODE_STAT))
        {
            vecUpdateDevSn.push_back(strDevSn);
            mapUpdateInfo.insert(QString::fromStdString(strDevSn), enNewMode);
        }
    }

    if (!DictionaryQueryManager::SaveDetectionConfig(m_detectionSetting))
    {
        ULOG(LOG_ERROR, "Encode detectionSetting Failed");
        return;
    }

	// 含有免疫设备
	if ((CommonInformationManager::GetInstance()->GetSoftWareType() & IMMUNE) != 0)
	{
		// 保存检测配置成功时发送信息到下位机
		im::LogicControlProxy::DetectModeUpdateByUI();
	}

	// 如果模式发生改变记录日志 add by chenjialin 20231215
	AddEnterSampleModeOptLog(oldDetectSet, m_detectionSetting, strTestModName);	

    // 获取所有设备列表
	QString strLog = tr("修改%1(内容：").arg(ui->label_3->text()), strLogItem("");
	// 遍历修改
    for (const auto& strDevSn : vecAllUpdateDevSn)
    {
        // 查找对应的设备信息
        auto stuTfDevInfo = CommonInformationManager::GetInstance()->GetDeviceInfo(strDevSn);

        // 轨道则跳过
        if (stuTfDevInfo->deviceType == tf::DeviceType::DEVICE_TYPE_TRACK)
        {
            continue;
        }

        // 不为免疫则跳过
        if (stuTfDevInfo->deviceClassify != tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE)
        {
            continue;
        }

        // 获取对应模式
        auto itMode = m_mapDevDetectMode.find(strDevSn);
        if (itMode == m_mapDevDetectMode.end())
        {
            continue;
        }

        // 获取扩展信息
        ImDeviceOtherInfo devOtherInfo;
        if (!DecodeJson(devOtherInfo, stuTfDevInfo->otherInfo))
        {
            ULOG(LOG_ERROR, "%s DecodeJson ImDeviceOtherInfo Failed", stuTfDevInfo->name);
            continue;
        }

        // 更新检测模式
        devOtherInfo.iDetectMode = itMode->second;

        // 编码
        std::string strOtherInfo;
        if (!Encode2Json(strOtherInfo, devOtherInfo))
        {
            ULOG(LOG_ERROR, "%s Encode2Json ImDeviceOtherInfo Failed", stuTfDevInfo->name);
            continue;
        }

        // 执行更新设备信息
        tf::DeviceInfo stuDevInfoUpdate;
        stuDevInfoUpdate.__set_deviceSN(stuTfDevInfo->deviceSN);
        stuDevInfoUpdate.__set_otherInfo(strOtherInfo);
        if (!CommonInformationManager::GetInstance()->ModifyDeviceInfo(stuDevInfoUpdate))
        {
            ULOG(LOG_ERROR, "ModifyDeviceInfo Failed");
            return;
        }
		// 记录操作日志
		auto atIt = m_mapImDetectModeName.find(devOtherInfo.iDetectMode);
		if (atIt != m_mapImDetectModeName.end())
		{
			if (!strLogItem.isEmpty())
			{
				strLogItem += ",";
			}
			strLogItem += tr("%1选择%2").arg(QString::fromStdString(stuTfDevInfo->name)).arg(atIt->second);
		}
    }
	if (vecAllUpdateDevSn.size() > 0)
	{
		strLog += strLogItem;
		strLog += (")");
		// 修改检测模式
		AddOptLog(::tf::OperationType::MOD, strLog);
	}

	// 通知其他界面
    POST_MESSAGE(MSG_ID_DETECTION_UPDATE, m_detectionSetting);

    // 如果STAT模式更新信息不为空
    if (!vecUpdateDevSn.empty())
    {
        // 通知后台检测模式已更新
        if (!im::LogicControlProxy::TestModeUpdateByUI(vecUpdateDevSn))
        {
            ULOG(LOG_ERROR, "TestModeUpdateByUI Failed");
        }

        POST_MESSAGE(MSG_ID_DETECT_MODE_UPDATE, mapUpdateInfo);
    }

	// 关闭窗口
    close();
}

///
/// @brief
///     表格单元格被点击
///
/// @param[in]  index  点击单元格索引
///
/// @par History:
/// @li 4170/TangChuXian，2023年10月26日，新建函数
///
void DetectionSetDlg::OnDetectTableItemClicked(const QModelIndex& index)
{
    // 参数判断
    if (index.column() != Dmh_RoutineMode &&
        index.column() != Dmh_QuickMode &&
        index.column() != Dmh_StatMode)
    {
        // 不是选择模式则忽略
        return;
    }

    // 获取点击项
    QTableWidgetItem* pItem = ui->DetModeTbl->item(index.row(), index.column());
    if (pItem != Q_NULLPTR && !pItem->icon().isNull())
    {
        // 已经被勾选，则忽略
        return;
    }

    // 检查是否可以运行修改参数
    if (!EnableChangeDetectMode(index))
    {
        return;
    }

    // 遍历所在行
    for (int iCol = Dmh_RoutineMode; iCol <= Dmh_StatMode; iCol++)
    {
        // 获取对应项
        QTableWidgetItem* pItem = ui->DetModeTbl->item(index.row(), iCol);
        if (pItem == Q_NULLPTR)
        {
            continue;
        }

        // 如果不是被点击项，取消勾选
        if (iCol != index.column())
        {
            pItem->setIcon(QIcon());
        }
        else
        {
            // 设置勾选
            pItem->setIcon(QIcon(":/Leonis/resource/image/icon-select.png"));
        }
    }
}

void DetectionSetDlg::UpdateCtrlsEnabled()
{
    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    auto& uim = UserInfoManager::GetInstance();

    // 进样模式
    ui->groupBox->setEnabled(uim->IsPermisson(PSM_TESTSET_SAMPLEIN));

    // 功能开关-扫描样本管条码
    ui->scan_tube_barcode_ckbox->setEnabled(!ui->barcode_rdbtn_0->isChecked()
        && uim->IsPermisson(PSM_TESTSET_FUNCSWITCH));
    // 功能开关-旋转样本管
    ui->rotate_tube_ckbox->setEnabled(uim->IsPermisson(PSM_TESTSET_FUNCSWITCH));

    // 检测模式
    ui->DetModeTbl->setEnabled(uim->IsPermisson(PSM_TESTSET_TESTMODE));
}

///
/// @brief 是否允许修改参数
///
/// @param[in]   
///
/// @return 
///
/// @par History:
/// @li 1556/Chenjianlin，2023年8月14日，新建函数
///
bool DetectionSetDlg::EnableModifyConfig()
{
    // 遍历表格，检查加载检测模式
    std::map<std::string, int> mapDevDetectType;
    for (int iRow = 0; iRow < ui->DetModeTbl->rowCount(); iRow++)
    {
        QTableWidgetItem* pItem = Q_NULLPTR;
        pItem = ui->DetModeTbl->item(iRow, Dmh_Device);
        if (pItem == Q_NULLPTR)
        {
            continue;
        }

        // 获取设备序列号
        std::string strDevSn = pItem->data(Qt::UserRole).toString().toStdString();

        // 获取原来的模式
        im::tf::DetectMode::type enOldMode = im::tf::DetectMode::DETECT_MODE_ROUTINE;
        im::tf::DetectMode::type enNewMode = im::tf::DetectMode::DETECT_MODE_ROUTINE;
        auto it = m_mapDevDetectMode.find(strDevSn);
        if (it != m_mapDevDetectMode.end())
        {
            enOldMode = (im::tf::DetectMode::type)it->second;
        }

        // 获取设备检测模式
        pItem = ui->DetModeTbl->item(iRow, Dmh_RoutineMode);
        if (pItem != Q_NULLPTR && !pItem->icon().isNull())
        {
            enNewMode = im::tf::DetectMode::DETECT_MODE_ROUTINE;
        }

        // 获取设备检测模式
        pItem = ui->DetModeTbl->item(iRow, Dmh_QuickMode);
        if (pItem != Q_NULLPTR && !pItem->icon().isNull())
        {
            enNewMode = im::tf::DetectMode::DETECT_MODE_QUICK;
        }

        // 获取设备检测模式
        pItem = ui->DetModeTbl->item(iRow, Dmh_StatMode);
        if (pItem != Q_NULLPTR && !pItem->icon().isNull())
        {
            enNewMode = im::tf::DetectMode::DETECT_MODE_STAT;
        }

        // 如果检测模式相等，则忽略，如果检测模式不相等，说明已经修改了
        if (enOldMode == enNewMode)
        {
            continue;
        }

        // 如果在加样停或运行状态，则不允许修改
        auto spStuTfDevInfo = CommonInformationManager::GetInstance()->GetDeviceInfo(strDevSn);
        if (spStuTfDevInfo == Q_NULLPTR)
        {
            continue;
        }

        // 如果状态不为运行和加样停，则忽略
        if (!devIsRun(*spStuTfDevInfo) &&
            spStuTfDevInfo->status != ::tf::DeviceWorkState::DEVICE_STATUS_MAINTAIN)
        {
            continue;
        }

        // 弹框提示不允许修改
        // 现在不允许保存
        TipDlg(tr("保存失败"), tr("请在待机或停机状态下修改检测模式！"), TipDlgType::SINGLE_BUTTON).exec();
        return false;
    }

	return true;
}

///
/// @brief
///     是否允许改变检测模式
///
/// @param[in]   index  检测模式表格索引
///
/// @return true表示允许
///
/// @par History:
/// @li 4170/TangChuXian，2023年11月3日，新建函数
///
bool DetectionSetDlg::EnableChangeDetectMode(const QModelIndex& index)
{
    // 检查参数
    if (!index.isValid())
    {
        return false;
    }

    QTableWidgetItem* pItem = ui->DetModeTbl->item(index.row(), Dmh_Device);
    if (pItem == Q_NULLPTR)
    {
        return false;
    }

    // 获取设备序列号
    std::string strDevSn = pItem->data(Qt::UserRole).toString().toStdString();

    // 获取原来的模式
    im::tf::DetectMode::type enOldMode = im::tf::DetectMode::DETECT_MODE_ROUTINE;
    im::tf::DetectMode::type enNewMode = im::tf::DetectMode::DETECT_MODE_ROUTINE;
    auto it = m_mapDevDetectMode.find(strDevSn);
    if (it != m_mapDevDetectMode.end())
    {
        enOldMode = (im::tf::DetectMode::type)it->second;
    }

    // 获取设备检测模式
    if (index.column() == Dmh_RoutineMode)
    {
        enNewMode = im::tf::DetectMode::DETECT_MODE_ROUTINE;
    }
    else if (index.column() == Dmh_QuickMode)
    {
        enNewMode = im::tf::DetectMode::DETECT_MODE_QUICK;
    }
    else if (index.column() == Dmh_StatMode)
    {
        enNewMode = im::tf::DetectMode::DETECT_MODE_STAT;
    }

    // 如果在加样停或运行状态，则不允许修改
    auto spStuTfDevInfo = CommonInformationManager::GetInstance()->GetDeviceInfo(strDevSn);
    if (spStuTfDevInfo == Q_NULLPTR)
    {
        return false;
    }

    // 如果状态不为运行和加样停，则忽略
    if (!devIsRun(*spStuTfDevInfo) &&
        spStuTfDevInfo->status != ::tf::DeviceWorkState::DEVICE_STATUS_MAINTAIN)
    {
        return true;
    }

    // 弹框提示不允许修改
    // 现在不允许保存
    TipDlg(tr("保存失败"), tr("请在待机或停机状态下修改检测模式！"), TipDlgType::SINGLE_BUTTON).exec();
    return false;
}

///
/// @brief 添加进样模式操作日志，日志需要组装，所以提函数
///
/// @param[in]  oldSet       之前的设置
/// @param[in]  uiSet		 现在的设置
/// @param[in]  strModeName  模式的名称
///
/// @return 
///
/// @par History:
/// @li 1556/Chenjianlin，2024年4月1日，新建函数
///
void DetectionSetDlg::AddEnterSampleModeOptLog(const DetectionSetting& oldSet, const DetectionSetting& uiSet, const QString& strModeName)
{
	if (oldSet == uiSet)
	{
		return;
	}


	// 组装日志
	QString strLog = tr("修改%1").arg(ui->groupBox->title()), strSplit=tr(","), strLogTemp("");
	strLog += "(";
	strLog += tr("内容：");
	// 检查进样模式
	if (oldSet.testMode != uiSet.testMode)
	{
		strLogTemp += tr("选择%1").arg(strModeName);
	}
	if (oldSet.scanTubeBarcode != uiSet.scanTubeBarcode)
	{
		if (!strLogTemp.isEmpty())
		{
			strLogTemp += strSplit;
		}
		if (uiSet.scanTubeBarcode)
		{
			strLogTemp += tr("开启");
		}
		else
		{
			strLogTemp += tr("关闭");
		}
		strLogTemp += ui->scan_tube_barcode_ckbox->text();
	}
	if (oldSet.rotateTube != uiSet.rotateTube)
	{
		if (!strLogTemp.isEmpty())
		{
			strLogTemp += strSplit;
		}
		if (uiSet.rotateTube)
		{
			strLogTemp += tr("开启");
		}
		else
		{
			strLogTemp += tr("关闭");
		}
		strLogTemp += ui->rotate_tube_ckbox->text();
	}
	strLog += strLogTemp;
	strLog += ")";
	// 修改进样模式
	AddOptLog(::tf::OperationType::MOD, strLog);
}
