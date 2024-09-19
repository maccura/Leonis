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

#include "QReagentScanDialog.h"
#include "ui_QReagentScanDialog.h"
#include <QRegExpValidator>
#include <QGridLayout>
#include <QSpacerItem>
#include <QButtonGroup>
#include "src/leonis/thrift/DcsControlProxy.h"
#include "src/leonis/thrift/im/ImLogicControlProxy.h"
#include "src/leonis/thrift/ch/c1005/C1005LogicControlProxy.h"
#include "src/leonis/thrift/im/i6000/I6000LogicControlProxy.h"
#include "src/common/defs.h"
#include "src/common/Mlog/mlog.h"

#include "uidcsadapter/uidcsadapter.h"
#include "shared/CommonInformationManager.h"
#include "shared/ReagentCommon.h"
#include "shared/uidef.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/tipdlg.h"
#include "uidcsadapter/uidcsadapter.h"

QReagentScanDialog::QReagentScanDialog(QWidget *parent)
    : BaseDlg(parent),
      m_ciDevGridRowCnt(2),
      m_ciDevGridColCnt(5),
      m_ciStrechRowDis(40),
      m_ciVSpace(20),
      m_ciHSpace(30),
      m_ciCheckBoxFixHeight(20)
{
    ui = new Ui::QReagentScanDialog();
    ui->setupUi(this);

    // 单选框互斥
    QButtonGroup* pButtonGroup = new QButtonGroup(this);
    pButtonGroup->setExclusive(true);
    pButtonGroup->addButton(ui->scan_selected_btn);
    pButtonGroup->addButton(ui->select_all_btn);

    // 获取所有存在试剂盘的生化免疫设备
    m_devices = CommonInformationManager::GetInstance()->GetDeviceFromType(
        std::vector<tf::DeviceType::type>({ tf::DeviceType::DEVICE_TYPE_C1000, tf::DeviceType::DEVICE_TYPE_I6000 }));

    // 隐藏余量检查，当勾选生化设备时显示
    ui->widget_margin_detect->hide();

    // 根据设备，显示界面设备勾选框
    QGridLayout* qLay = new QGridLayout(this);
    qLay->setMargin(0);
    qLay->setHorizontalSpacing(m_ciHSpace);
    qLay->setVerticalSpacing(m_ciVSpace);
    QCheckBox *allChk = new QCheckBox(tr("全部"));
    QLabel*    pModuleNameLab = new QLabel(tr("模块"));
    allChk->setObjectName(QStringLiteral("reagentscan_alldevice"));
    allChk->setFixedHeight(m_ciCheckBoxFixHeight);
    connect(allChk, SIGNAL(stateChanged(int)), this, SLOT(OnDeviceCheckboxCheck(int)));
    qLay->addWidget(pModuleNameLab, 0, 0);
    qLay->addWidget(allChk, 0, 1);
    m_deviceCheckboxs.clear();
    int iDevIdx = 0;
    int iRealRowCnt = m_devices.size() < m_ciDevGridColCnt ? 1 : m_ciDevGridRowCnt;
    for (int iRow = 0; iRow < iRealRowCnt; iRow++)
    {
        for (int iCol = 1; iCol <= m_ciDevGridColCnt; iCol++)
        {
            // 首个复选框为“全部”且已添加，直接跳过
            if ((iRow == 0) && (iCol == 1))
            {
                continue;
            }

            iDevIdx = iRow * m_ciDevGridColCnt + iCol - 2;
            if (iDevIdx >= m_devices.size())
            {
                qLay->addItem(new QSpacerItem(0, 0), iRow, iCol);
                continue;
            }

            std::shared_ptr<const tf::DeviceInfo>& dv = m_devices.at(iDevIdx);
            QCheckBox* ck = new QCheckBox(QString::fromStdString(dv->groupName) + QString::fromStdString(dv->name), this);
            ck->setObjectName(QString::fromStdString(dv->deviceSN));
            ck->setFixedHeight(m_ciCheckBoxFixHeight);
            connect(ck, SIGNAL(stateChanged(int)), this, SLOT(OnDeviceCheckboxCheck(int)));
            m_deviceCheckboxs.push_back(ck);
            qLay->addWidget(ck, iRow, iCol);
        }
    }

    // 设置伸缩比例
    for (int iRow = 0; iRow < iRealRowCnt; iRow++)
    {
        qLay->setRowStretch(iRow, 1);
    }
    for (int iCol = 0; iCol <= m_ciDevGridColCnt; iCol++)
    {
        if (iCol == 0)
        {
            qLay->setColumnStretch(iCol, 0);
            continue;
        }

        qLay->setColumnStretch(iCol, 1);
    }
    ui->widgetDevices->setLayout(qLay);

    // 缩进
    int istrechHeight = (m_ciDevGridRowCnt - iRealRowCnt) * m_ciStrechRowDis;

    // 如果是单机版，则不显示设备选择
    if (gUiAdapterPtr()->WhetherSingleDevMode())
    {
        ui->widgetDevices->setVisible(false);
        allChk->setChecked(true);
        istrechHeight *= 2;
        ui->BottomFrame->move(ui->BottomFrame->x(), ui->BottomFrame->y() - istrechHeight);
        this->setFixedHeight(this->height() - istrechHeight);
    }
    else if (istrechHeight > 0)
    {
        ui->widgetDevices->setFixedHeight(ui->widgetDevices->height() - istrechHeight);
        ui->BottomFrame->move(ui->BottomFrame->x(), ui->BottomFrame->y() - istrechHeight);
        this->setFixedHeight(this->height() - istrechHeight);
    }

    Init();
}

QReagentScanDialog::~QReagentScanDialog()
{
}

///
/// @brief 初始化试剂扫描对话框
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月7日，新建函数
///
void QReagentScanDialog::Init()
{
    SetTitleName(tr("试剂扫描"));

    // 关闭对话框
    connect(ui->cancel_btn, &QPushButton::clicked, this, [&]() {this->close(); });
    // 扫描选中试剂位
    connect(ui->scan_selected_btn, &QPushButton::clicked, this, [&]()
    {
        ui->select_edit->setEnabled(true);
    });

    // 扫描全部试剂位
    connect(ui->select_all_btn, &QPushButton::clicked, this, [&]()
    {
        ui->select_edit->setEnabled(false);
    });

	// 余量检测
	ui->rb_fast->setEnabled(true);
	ui->rb_sync->setEnabled(true);

	// 默认勾选余量探测(方式为同步余量探测)
	ui->allowance_btn->setChecked(true);
	ui->rb_sync->setChecked(true);

	connect(ui->allowance_btn, &QCheckBox::clicked, this, [=](bool ck) {
		if (ck)
		{
			ui->rb_fast->setEnabled(true);
			ui->rb_sync->setEnabled(true);
		}
		else
		{
			ui->rb_fast->setEnabled(false);
			ui->rb_sync->setEnabled(false);
		}
	});

    // 确定按钮信号槽
    connect(ui->ok_btn, SIGNAL(clicked()), this, SLOT(OnScanReagent()));
    ui->select_all_btn->setChecked(true);
    ui->select_edit->setEnabled(false);

    // 设置位置的正则表达式，限制位置信息只能输入"数字、逗号、-"，验证工作留到点击确定的时候
	QRegExp rxSymbols(UI_REAGENT_POS);
	QRegExpValidator* validator = new QRegExpValidator(rxSymbols, this);

    ui->select_edit->setValidator(validator);
    ui->select_edit->setAttribute(Qt::WA_InputMethodEnabled, false);
}

bool QReagentScanDialog::GetSelectedDeviceSn(std::set<std::string>& devicesSn)
{
    for (QCheckBox* ck : m_deviceCheckboxs)
    {
        if (ck->isChecked())
        {
            devicesSn.insert(ck->objectName().toStdString());
        }
    }

    return devicesSn.size() != 0;
}

///
/// @brief 确定按钮槽函数
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月7日，新建函数
///
void QReagentScanDialog::OnScanReagent()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 获取选择的设备SN
    std::set<std::string> deviveSns;
    if (!GetSelectedDeviceSn(deviveSns))
    {
        TipDlg(tr("无法执行试剂扫描操作，未选择执行试剂扫描的设备。")).exec();
        return;
    }

    // 查询所有免疫设备
    // 查询当前选中项设备状态
    tf::DeviceInfoQueryResp devInfoResp;
    tf::DeviceInfoQueryCond devInfoCond;
    devInfoCond.__set_deviceType(tf::DeviceType::DEVICE_TYPE_I6000);

    // 执行查询
    if (!DcsControlProxy::GetInstance()->QueryDeviceInfo(devInfoResp, devInfoCond))
    {
        ULOG(LOG_WARN, "QueryDeviceInfo failed!");
        return;
    }

    // 获取选中设备信息
    for (const auto& stuTfDevInfo : devInfoResp.lstDeviceInfos)
    {
        // 对比设备序列号，查看是否选中设备
        if (std::find(deviveSns.begin(), deviveSns.end(), stuTfDevInfo.deviceSN) == deviveSns.end())
        {
            continue;
        }

        // 判断是否允许执行
        if (PopRgntScanForbiddenTip(stuTfDevInfo))
        {
            return;
        }
    }

    // 获取手动输入的试剂位置
    std::vector<int> reagentPositionsManuSet;
    if (ui->scan_selected_btn->isChecked() && 
		!ParserInputPositions(ui->select_edit->text(),reagentPositionsManuSet))
    {
        ULOG(LOG_WARN, "Failed to parser imput positions string.");
        return;
    }

    // 扫描
    for (auto dev : m_devices)
    {
        // 没选择的设备不进行扫描
        if (deviveSns.find(dev->deviceSN) == deviveSns.end())
        {
            ULOG(LOG_WARN, "Reagent scan skiped, device:%s(device not be contain.)", dev->deviceSN);
            continue;
        }
        QString qstrDevName = QString::fromStdString(dev->name);
        QString qstrDevGroupName = QString::fromStdString(dev->groupName);

        // 当前设备对应的试剂位总数
        int slotNum = gUiAdapterPtr(qstrDevName,qstrDevGroupName)->GetReagentSlotNumber();

        // 获取扫描位置
        std::vector<int> scanPositions;
        if (ui->select_all_btn->isChecked())
        {
            // 采用全部试剂位
            for (int i = 1; i <= slotNum; i++)
            {
                scanPositions.push_back(i);
            }
        }
        else
        {
            // 采用手动填写的试剂位
            scanPositions = reagentPositionsManuSet;

            // 检测手动输入的值是否合法
            for (int posItem : scanPositions)
            {
                if (posItem > slotNum)
                {
                    ULOG(LOG_WARN, "The input value exceeds the maximum slot range.");
                    QString failedstr = tr("输入试剂位号超出允许范围（1-%1），请重新输入！").arg(slotNum);
                    TipDlg(failedstr).exec();
                    return;
                }
            }
        }

        // 免疫设备的试剂扫描调用
        if (dev->deviceType == tf::DeviceType::DEVICE_TYPE_I6000)
        {
            // 过滤掉不合法的位置
            std::vector<int> imPos;
            for (int iPos : scanPositions)
            {
                if (iPos < 1 || iPos > slotNum)
                {
                    continue;
                }

                imPos.push_back(iPos);
            }

            // 通知试剂指定位置扫描开始
            ::tf::MaintainItemType::type enScanType = ::tf::MaintainItemType::type::MAINTAIN_ITEM_REAGENT_SCAN;
            if (ui->scan_selected_btn->isChecked())
            {
                enScanType = ::tf::MaintainItemType::type::MAINTAIN_ITEM_REAGENT_SCAN_POS;
                SEND_MESSAGE(MSG_ID_REAGENT_SCAN_POS_START, qstrDevName, imPos);
            }

            // 试剂扫描维护调用了SetReagentScan。
            if (!im::LogicControlProxy::SingleMaintance(enScanType, dev->deviceSN, imPos))
            {
                ULOG(LOG_ERROR, "Im, SetReagentScan failed.");
                TipDlg(tr("%1 执行维护操作失败！").arg(qstrDevName)).exec();
                return;
            }
        }
        // 生化设备的试剂扫描调用
        else
        {
            std::vector<::ch::tf::SuppliesPosition> srcPos;
            for (auto pos : scanPositions)
            {
                ::ch::tf::SuppliesPosition scan;
                scan.area = ch::tf::SuppliesArea::type::SUPPLIES_AREA_REAGENT_DISK1;
                scan.pos = pos;
                srcPos.push_back(scan);
            }

            // 是否余量检测
            bool probeResidual = ui->allowance_btn->isChecked();
			// 余量检测方式 true: 快速余量检测 false:同步余量检测
			bool isFastMode = ui->rb_fast->isChecked();
            if (!ch::c1005::LogicControlProxy::ScanSuppliesInfo(dev->deviceSN, srcPos, probeResidual,isFastMode))
            {
                ULOG(LOG_ERROR, "Ch, SetReagentScan failed.");
                TipDlg(tr("%1 执行维护操作失败！").arg(qstrDevName)).exec();
                return;
            }
        }
    }

    close();
}

void QReagentScanDialog::OnDeviceCheckboxCheck(int stat)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    QCheckBox* qChk = qobject_cast<QCheckBox*>(this->sender());
    if (qChk == Q_NULLPTR)
    {
        ULOG(LOG_WARN, "Object from sender is null.");
        return;
    }

    // 处理全选状态
    QString objName = qChk->objectName();
    bool isCheck = qChk->isChecked();
    if (objName == "reagentscan_alldevice")
    {
        for (QCheckBox* ck : m_deviceCheckboxs)
        {
            ck->setChecked(isCheck);
        }
    }

    // 当所以设备框都勾选时，自动勾选全选
    bool allChecked = true;
    bool checkedCh = false; // 是否勾选生化设备
    for (QCheckBox* chk : m_deviceCheckboxs)
    {
        if (!chk->isChecked())
        {
            allChecked = false;
        }
        else
        {
            if (checkedCh)
            {
                continue;
            }
            auto deviceObj = CommonInformationManager::GetInstance()->GetDeviceInfo(chk->objectName().toStdString());
            if (deviceObj && deviceObj->deviceType != ::tf::DeviceType::DEVICE_TYPE_I6000)
            {
                checkedCh = true;
            }
        }
    }
    QCheckBox* allChk = ui->widgetDevices->findChild<QCheckBox*>("reagentscan_alldevice");
    if (allChk != nullptr)
    {
        allChk->blockSignals(true);
        allChk->setChecked(allChecked);
        allChk->blockSignals(false);
    }

    // 如果勾选了生化设备，则显示余量检查
    checkedCh ? ui->widget_margin_detect->show() : ui->widget_margin_detect->hide();
}
