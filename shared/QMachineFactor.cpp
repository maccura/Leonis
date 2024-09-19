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
/// @file     QMachineFactor.cpp
/// @brief    仪器系数
///
/// @author   8090/YeHuaNing
/// @date     2022年9月13日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8090/YeHuaNing，2022年9月13日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QMachineFactor.h"
#include "ui_QMachineFactor.h"
#include "shared/CReadOnlyDelegate.h"
#include "shared/tipdlg.h"
#include "shared/CommonInformationManager.h"
#include "shared/QSerialModel.h"
#include "shared/QComDelegate.h"
#include "shared/ReagentCommon.h"
#include "shared/uicommon.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/StringUtil.h"

#include <QRadioButton>
#include <QHBoxLayout>
#include "thrift/DcsControlProxy.h"


#define	 DEFAULTA		"1"
#define  DEFAULTB		"0"

QMachineFactor::QMachineFactor(QWidget *parent)
    : BaseDlg(parent)
	, m_tableModel(new QSerialModel)
    , m_assayName(tr("项目名称"))
    , m_factorA(tr("系数(a)"))
    , m_factorB(tr("系数(b)"))

{
    ui = new Ui::QMachineFactor();
    ui->setupUi(this);

    SetTitleName(tr("仪器系数"));
	ui->tableView->setModel(m_tableModel);

    // 限制输入范围
    QDoubleValidator* validatorA = new ChDoubleValidator();
    //validatorA->setBottom(0); //原需求抄袭3000，不应限制系数a的范围。20240611 马骉确认
    validatorA->setNotation(QDoubleValidator::StandardNotation);
    ui->factorA_edit->setValidator(validatorA);

    // 限制输入范围
    QDoubleValidator* validatorB = new ChDoubleValidator();
    validatorB->setNotation(QDoubleValidator::StandardNotation);
    ui->factorB_edit->setValidator(validatorB);

    InitTableTitle();
    InitMachines();

	connect(ui->cls_Button, &QPushButton::clicked, this, [this] { close(); });
	connect(ui->save_btn, &QPushButton::clicked, this, &QMachineFactor::SaveMachineData);
	connect(ui->tableView, &QTableView::clicked, this, &QMachineFactor::onSelectRow);
    connect(ui->tableView->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this](const QItemSelection &selected, const QItemSelection &deselected) {
        if (selected.indexes().empty())
        {
            ui->factorA_edit->setText("");
            ui->factorB_edit->setText("");
            m_mfQryCnd.__set_assayCode(-1);
            m_mfQryCnd.__isset.assayCode = false;
        }
        else
            onSelectRow(selected.indexes()[0]);
    });

    // 监听项目更新
    REGISTER_HANDLER(MSG_ID_ASSAY_CODE_MANAGER_UPDATE, this, OnAssayUpdated);
}

QMachineFactor::~QMachineFactor()
{
}

///
/// @brief 初始化表标题
///
/// @par History:
/// @li 8090/YeHuaNing，2022年9月14日，新建函数
///
void QMachineFactor::InitTableTitle()
{
	QStringList hTitle;
	hTitle << m_assayName << m_factorA << m_factorB;

	m_tableModel->setHorizontalHeaderLabels(hTitle);

    for(int i = 0; i < m_tableModel->columnCount(); ++i)
        ui->tableView->setItemDelegateForColumn(i, new CReadOnlyDelegate(this));
}

///
/// @brief 初始化设备
///
/// @par History:
/// @li 8090/YeHuaNing，2022年9月14日，新建函数
///
void QMachineFactor::InitMachines()
{
    // 首先刷新设备信息
    std::vector<std::shared_ptr<const tf::DeviceInfo>> deviceList = CommonInformationManager::GetInstance()->GetDeviceFromType(\
        std::vector<tf::DeviceType::type>({ tf::DeviceType::DEVICE_TYPE_C1000,
        tf::DeviceType::DEVICE_TYPE_ISE1005, tf::DeviceType::DEVICE_TYPE_C200, tf::DeviceType::DEVICE_TYPE_I6000 }));

    // 设备列表为空，则直接返回
    if (deviceList.empty())
    {
        return;
    }

    // 开始插入仪器选项
    QHBoxLayout* layout = new QHBoxLayout(ui->radioButtons);
	layout->setSpacing(19);

    if (layout == nullptr)
    {
        return;
    }

    QButtonGroup* pButtonGroup = new QButtonGroup(this);
    // 设置互斥
    pButtonGroup->setExclusive(true);
    layout->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    bool isChecked = false;

    for (const auto& device : deviceList)
    {
        for (int i = 1; i <= device->moduleCount; ++i)
        {
            int moduleIndex = 0;
            if (device->deviceClassify == tf::AssayClassify::ASSAY_CLASSIFY_ISE)
            {
                moduleIndex += i;
            }
            QString DevName = CommonInformationManager::GetInstance()->GetDeviceName(device->deviceSN, moduleIndex).c_str();
            QRadioButton *pButton = new QRadioButton(DevName);
            connect(pButton, &QPushButton::clicked, this, [this, device, moduleIndex] { ui->tableView->selectRow(0); onSelectMachine(device->deviceClassify, device->deviceSN, moduleIndex); });
            layout->addWidget(pButton);
            pButtonGroup->addButton(pButton);

            if (!isChecked && device->deviceClassify == tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY)
            {
                pButton->setChecked(true);
                emit pButton->clicked(true);
                isChecked = true;
            }
        }
        
    }

    // 如果没有生化设备被选中，则默认选择第一个
    if (!isChecked && !pButtonGroup->buttons().empty())
    {
        if (pButtonGroup->buttons()[0])
        {
            pButtonGroup->buttons()[0]->setChecked(true);
            emit pButtonGroup->buttons()[0]->clicked(true);
        }
    }

    // 只有一个设备的时候，不显示设备信息-bug23941
    if (deviceList.size() == 1)
    {
        ui->radioButtons->setVisible(false);
        ui->tableView->setGeometry(50, 85, 790, 539);
        ui->line_h->setGeometry(50, 85, 790, 1);
        ui->line_v->setGeometry(50, 85, 1, 539+78);
        ui->line_v_2->setGeometry(839, 85, 1, 539+78);
    }
}

///
/// @brief 执行保存
///
/// @par History:
/// @li 8090/YeHuaNing，2022年9月14日，新建函数
///
void QMachineFactor::SaveMachineData()
{
    QString errorInfo;
	// 校验基础信息
    if (ui->factorA_edit->text().isEmpty() || ui->factorB_edit->text().isEmpty())
    {
        errorInfo = tr("输入框不能为空。");
    }
    else if (!ChDoubleValidator::IsValidNumber(ui->factorA_edit->text())
        || !ChDoubleValidator::IsValidNumber(ui->factorB_edit->text()))
    {
        errorInfo = tr("请输入有效数字。");
    }
    else if (ui->factorA_edit->text().toDouble() == 0.0)
    {
        errorInfo = tr("系数a的值不能为0。");
    }
    else if (!m_mfQryCnd.__isset.assayCode)
    {
        errorInfo = tr("未选择需要修改的项目。");
    }

    if (!errorInfo.isEmpty())
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("参数检查失败"), errorInfo, TipDlgType::SINGLE_BUTTON));
        pTipDlg->exec();
        return;
    }

    // 仪器运行中，禁止修改参数
    auto spDeviceInfo = CommonInformationManager::GetInstance()->GetDeviceInfo(m_mfQryCnd.deviceSN);
    if (spDeviceInfo != nullptr &&  devIsRun(*spDeviceInfo))
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("失败"), tr("当前仪器运行中，禁止修改系数！"), TipDlgType::SINGLE_BUTTON));
        pTipDlg->exec();
        return;
    }

    // 查询数据是否存在
	::tf::MachineFactorQueryResp mfqr;
    double f_A = 1;
    double f_B = 0;
    if (!stringutil::IsDouble(ui->factorA_edit->text().toStdString(), f_A) ||
        !stringutil::IsDouble(ui->factorB_edit->text().toStdString(), f_B))
    {
        TipDlg("系数必须输入为数值").exec();
    }

	if (DcsControlProxy::GetInstance()->QueryMachineFactors(mfqr, m_mfQryCnd) && mfqr.result == tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		// 增加
		if (mfqr.lstMachineFactors.empty())
		{
			tf::MachineFactor mf;
			tf::ResultLong rst;
			mf.__set_assayCode(m_mfQryCnd.assayCode);
            mf.__set_deviceSN(m_mfQryCnd.deviceSN);
            mf.__set_moduleIndex(m_mfQryCnd.moduleIndex);
			mf.__set_factorA(f_A);
			mf.__set_factorB(f_B);

			DcsControlProxy::GetInstance()->AddMachineFactor(rst, mf);
		}
		//修改
		else
		{
			std::vector< ::tf::MachineFactor> mfs;
			tf::MachineFactor mf;
			mf.__set_id(mfqr.lstMachineFactors[0].id);
			mf.__set_assayCode(mfqr.lstMachineFactors[0].assayCode);
            mf.__set_deviceSN(mfqr.lstMachineFactors[0].deviceSN);
            mf.__set_moduleIndex(mfqr.lstMachineFactors[0].moduleIndex);
			mf.__set_factorA(f_A);
			mf.__set_factorB(f_B);

			mfs.push_back(mf);
			DcsControlProxy::GetInstance()->ModifyMachineFactor(mfs);
		}
	}

    // 刷新一次界面
	onSelectMachine(m_curDevType, "", -1);
}

///
/// @brief 当选择设备后，进行刷新
///
/// @param[in]  deviceName  设备名称
///
/// @par History:
/// @li 8090/YeHuaNing，2022年9月14日，新建函数
///
void QMachineFactor::onSelectMachine(tf::AssayClassify::type  devClassify, const std::string& deviceName, int moduleIndex)
{
    ChDoubleValidator* pValiA = static_cast<ChDoubleValidator*>(const_cast<QValidator *>(ui->factorA_edit->validator()));
    ChDoubleValidator* pValiB = static_cast<ChDoubleValidator*>(const_cast<QValidator *>(ui->factorB_edit->validator()));
    auto iSelInd = ui->tableView->currentIndex().row();
    int facAIndex = m_tableModel->GetColIndex(m_factorA);
    int facBIndex = m_tableModel->GetColIndex(m_factorB);    
    m_mfQryCnd.__isset.assayCode = false;           // 重置一次数据，避免错误验证
    ui->factorA_edit->setText("");                   // 初始化
    ui->factorB_edit->setText("");                   // 初始化

    // 如果设备类型改变，则重新获取项目
    if (devClassify != m_curDevType)
    {
        iSelInd = 0;
    }

    m_tableModel->removeRows(0, m_tableModel->rowCount());
    SetTableAssayInfo(devClassify);
    ui->tableView->setCurrentIndex(QModelIndex());   // 取消选择

    // 设置当前仪器类型，调用刷新接口时使用
    m_curDevType = devClassify;

    // 竖向表格标题（不显示)
    QStringList vTitle;
    // 上一次的仪器序号
	static std::string lastDevice;
    static int lastModuleIndex = -1;

	if (!deviceName.empty())
		lastDevice = deviceName;

    if (moduleIndex != -1)
        lastModuleIndex = moduleIndex;

	if (lastDevice.empty() || lastModuleIndex == -1)
	{
        ULOG(LOG_ERROR, "Invalid Parameters:%s, %d", lastDevice, lastModuleIndex);
		return;
	}

	// 查询当前设备已经设置的仪器系数
	::tf::MachineFactorQueryResp mfqr;
	::tf::MachineFactorQueryCond mfqc;
    mfqc.__set_deviceSN(lastDevice);
    // 免疫不需要判定该列，升级前为空
    if (tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE != m_curDevType)
    {
        mfqc.__set_moduleIndex(lastModuleIndex);
    }   

    // 查询当前仪器已经设置过的数据
	if (DcsControlProxy::GetInstance()->QueryMachineFactors(mfqr, mfqc) && mfqr.result == tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		for (const auto& mf : mfqr.lstMachineFactors)
		{
            QString a = std::move(DoubleToString(mf.factorA));
            QString b = std::move(DoubleToString(mf.factorB));
            if (pValiA)
                a = std::move(pValiA->GetValidNumber(a, mf.factorA));

            if (pValiB)
                b = std::move(pValiB->GetValidNumber(b, mf.factorB));

			QStandardItem* facA = CenterAligmentItem(a);
			QStandardItem* facB = CenterAligmentItem(b);
			int rowIndex = m_tableModel->GetRowIndex(QString::number(mf.assayCode));

			m_tableModel->setItem(rowIndex, facAIndex, facA);
			m_tableModel->setItem(rowIndex, facBIndex, facB);
		}
	}

    // 保存查询结果
	m_curMachineFactors.assign(mfqr.lstMachineFactors.begin(), mfqr.lstMachineFactors.end());

	m_mfQryCnd = mfqc;
	ui->tableView->setColumnWidth(0, 360);
	ui->tableView->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
	ui->tableView->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
	ui->tableView->horizontalHeader()->setMinimumSectionSize(80);

    if (m_tableModel->rowCount() > 0 && iSelInd < m_tableModel->rowCount())
    {
        ui->tableView->selectRow(iSelInd);
        onSelectRow(m_tableModel->index(iSelInd, 0));
    }
    
}

///
/// @brief 设置表格中的项目信息
///
/// @param[in]  devClassify  设备类型
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月26日，新建函数
///
void QMachineFactor::SetTableAssayInfo(tf::AssayClassify::type devClassify)
{
    switch (devClassify)
    {
    case tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE:
        SetImAssayInfoToTable();
        break;
    case tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY:
        SetChAssayInfoToTable();
        break;
    case tf::AssayClassify::ASSAY_CLASSIFY_ISE:
        SetIseAssayInfoToTable();
        break;
    default:
        break;
    }
}

///
/// @brief 将当前表格数据设置为生化信息
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月26日，新建函数
///
void QMachineFactor::SetChAssayInfoToTable()
{
    QStringList vTitle;                 //表格竖向标题（不显示）
    QVector<std::tuple<std::string, int>> list;
    ChAssayIndexUniqueCodeMaps AssayMap;
    QList<int> shiledList;             //不需要显示的项目
    shiledList << ch::tf::g_ch_constants.ASSAY_CODE_SIND;

    CommonInformationManager::GetInstance()->GetChAssayMaps(::tf::DeviceType::DEVICE_TYPE_INVALID, AssayMap);

    int row = 0;
    //获取项目信息list
    for (auto assay : AssayMap)
    {
        // 根据项目名称获取项目信息
        std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(assay.first);
        if (spAssayInfo == nullptr)
        {
            return;
        }

        // 过滤不需要的项目信息
        if (shiledList.contains(spAssayInfo->assayCode))
        {
            continue;
        }

        vTitle.push_back(QString::number(spAssayInfo->assayCode));
        m_tableModel->setItem(row, m_tableModel->GetColIndex(m_assayName), CenterAligmentItem(QString::fromStdString(spAssayInfo->assayName)));
        m_tableModel->setItem(row, m_tableModel->GetColIndex(m_factorA), CenterAligmentItem(DEFAULTA));
        m_tableModel->setItem(row++, m_tableModel->GetColIndex(m_factorB), CenterAligmentItem(DEFAULTB));
    }

    m_tableModel->setVerticalHeaderLabels(vTitle);
}

///
/// @brief 将当前表格数据设置为免疫信息
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月26日，新建函数
///
void QMachineFactor::SetImAssayInfoToTable()
{
    QStringList vTitle;                 //表格竖向标题（不显示）
    QVector<std::tuple<std::string, int>> list;
    ImAssayNameMaps AssayMap;
    //QStringList shiledList;             //不需要显示的项目

    CommonInformationManager::GetInstance()->GetImAssayMaps(::tf::DeviceType::DEVICE_TYPE_INVALID, AssayMap);

    int row = 0;
    //获取项目信息list
    for (auto assay : AssayMap)
    {
        // 根据项目名称获取项目信息
        std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(assay.first);
        if (spAssayInfo == nullptr)
        {
            return;
        }
        vTitle.push_back(QString::number(spAssayInfo->assayCode));
        m_tableModel->setItem(row, m_tableModel->GetColIndex(m_assayName), CenterAligmentItem(QString::fromStdString(spAssayInfo->assayName)));
        m_tableModel->setItem(row, m_tableModel->GetColIndex(m_factorA), CenterAligmentItem(DEFAULTA));
        m_tableModel->setItem(row++, m_tableModel->GetColIndex(m_factorB), CenterAligmentItem(DEFAULTB));
    }

    m_tableModel->setVerticalHeaderLabels(vTitle);
}

///
/// @brief 将当前表格数据设置为电解质项目信息
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月26日，新建函数
///
void QMachineFactor::SetIseAssayInfoToTable()
{
    QStringList vTitle;                 //表格竖向标题（不显示）
    QVector<std::tuple<std::string, int>> list;

    const IseAssayIndexCodeMaps& AssayMap = CommonInformationManager::GetInstance()->GetIseAssayIndexCodeMaps();

    int row = 0;
    //获取项目信息list
    for (auto assay : AssayMap)
    {
        // 根据项目名称获取项目信息
        std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(assay.first);
        if (spAssayInfo == nullptr)
        {
            return;
        }

        vTitle.push_back(QString::number(spAssayInfo->assayCode));
        m_tableModel->setItem(row, m_tableModel->GetColIndex(m_assayName), CenterAligmentItem(QString::fromStdString(spAssayInfo->assayName)));
        m_tableModel->setItem(row, m_tableModel->GetColIndex(m_factorA), CenterAligmentItem(DEFAULTA));
        m_tableModel->setItem(row++, m_tableModel->GetColIndex(m_factorB), CenterAligmentItem(DEFAULTB));
    }

    m_tableModel->setVerticalHeaderLabels(vTitle);
}

QString QMachineFactor::DoubleToString(double value)
{
    QString strValue = QString::number(value, 'f');
    while (strValue.endsWith('0'))
    {
        strValue.chop(1);
    }

    while (strValue.endsWith('.'))
    {
        strValue.chop(1);
    }

    return strValue;
}

///
/// @brief 当某一行选择后
///
/// @param[in]  index  被点击的项目
///
/// @return 
///
/// @par History:
/// @li 8090/YeHuaNing，2022年9月14日，新建函数
///
void QMachineFactor::onSelectRow(const QModelIndex &index)
{
	if (!index.isValid() || index.row() < 0 || index.column() < 0)
		return;

	QStandardItem* itemAssay = m_tableModel->item(index.row(), m_tableModel->GetColIndex(m_assayName));
	QStandardItem* itemA = m_tableModel->item(index.row(), m_tableModel->GetColIndex(m_factorA));
	QStandardItem* itemB = m_tableModel->item(index.row(), m_tableModel->GetColIndex(m_factorB));

	if (itemA != nullptr)
	{
		// 获取系数A
		ui->factorA_edit->setText(itemA->text());
	}
	else
	{
		ui->factorA_edit->setText("");
	}

	if (itemB != nullptr)
	{
		// 获取系数B
		ui->factorB_edit->setText(itemB->text());
	}
	else
	{
		ui->factorB_edit->setText("");
	}

	if (itemAssay != nullptr)
	{
		m_mfQryCnd.__set_assayCode(m_tableModel->verticalHeaderItem(index.row())->text().toInt());
	}
}

void QMachineFactor::OnAssayUpdated()
{
    // 刷新一次界面
    onSelectMachine(m_curDevType, "", -1);
}

void QMachineFactor::hideEvent(QHideEvent * event)
{
    onSelectRow(ui->tableView->currentIndex());

    QWidget::hideEvent(event);
}
