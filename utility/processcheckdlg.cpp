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
/// @file     processcheckdlg.cpp
/// @brief    流程检查对话框
///
/// @author   4170/TangChuXian
/// @date     2024年1月15日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月15日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "processcheckdlg.h"
#include "ui_processcheckdlg.h"
#include "shared/uicommon.h"
#include "shared/tipdlg.h"
#include "shared/basedlg.h"
#include "shared/uidef.h"
#include "shared/msgiddef.h"
#include "shared/messagebus.h"
#include "shared/CommonInformationManager.h"
#include "shared/FileExporter.h"
#include "uidcsadapter/uidcsadapter.h"
#include "../thrift/im/ImLogicControlProxy.h"
#include "thrift/DcsControlProxy.h"
#include "src/common/Mlog/mlog.h"
#include "src/public/DictionaryKeyName.h"
#include "src/thrift/im/gen-cpp/im_types.h"
#include <boost/math/tools/stats.hpp>
#include <boost/accumulators/accumulators.hpp>
#include <boost/accumulators/statistics/stats.hpp>
#include <boost/accumulators/statistics/variance.hpp>
#include <boost/accumulators/statistics/density.hpp>
#include <QTimer>
#include <QRadioButton>
#include <QScrollBar>
#include <QFileDialog>
#include <algorithm>

#define ATTEN_FACTOR_PERCISION              2               // 系数精度默认位数

using Accumulators = boost::accumulators::accumulator_set<double, boost::accumulators::features<boost::accumulators::tag::mean, boost::accumulators::tag::variance>>;
ProcessCheckDlg::ProcessCheckDlg(QWidget *parent)
    : BaseDlg(parent),
      m_bInit(false),
      m_ciDefaultRowCnt(10),
      m_ciDefaultColCnt(2),
      m_enTfMtType(tf::MaintainItemType::type::MAINTAIN_ITEM_DETECT_TEST)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 初始化Ui对象
    ui = new Ui::ProcessCheckDlg();
    ui->setupUi(this);

    // 界面显示前初始化
    InitBeforeShow();
}

ProcessCheckDlg::~ProcessCheckDlg()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    delete ui;
}

///
/// @brief
///     界面显示前初始化
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月15日，新建函数
///
void ProcessCheckDlg::InitBeforeShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 磁分离隐藏加液量
    ui->AddRgntVolLab1->setVisible(false);
    ui->AddRgntVolEdit1->setVisible(false);
    ui->RgntUnitLab1->setVisible(false);
    ui->MagneticNoLab->move(160, ui->MagneticNoLab->y());
    ui->MagneticNoCombo->move(160, ui->MagneticNoCombo->y());

    // 构造列表
    QList<QTableWidget*> allTblLst;
    allTblLst.push_back(ui->SigValTbl11);
    allTblLst.push_back(ui->SigValTbl12);
    allTblLst.push_back(ui->SigValTbl21);
    allTblLst.push_back(ui->SigValTbl22);
    allTblLst.push_back(ui->SigValTbl31);
    allTblLst.push_back(ui->SigValTbl32);

    // 构造表头
    QStringList strHeaderList;
    strHeaderList << tr("序号") << tr("信号值");

    // 遍历所有表格
    for (QTableWidget* pTbl : allTblLst)
    {
        // 设置表格行列数
        pTbl->setRowCount(m_ciDefaultRowCnt);
        pTbl->setColumnCount(m_ciDefaultColCnt);

        // 构造表头
        pTbl->setHorizontalHeaderLabels(strHeaderList);
        pTbl->verticalHeader()->setVisible(false);

        // 设置表格选中模式为行选中，不可多选
        pTbl->setSelectionBehavior(QAbstractItemView::SelectRows);
        pTbl->setSelectionMode(QAbstractItemView::ExtendedSelection);
        pTbl->setEditTriggers(QAbstractItemView::NoEditTriggers);

        // 表格列宽自适应
        ResizeTblColToContent(pTbl);
    }

    // 添加下拉框数据
    // 下拉框1
	AddTfEnumItemToComBoBox(ui->CheckProcessCombo1, ::im::tf::MaintProcCheckSubType::MODULE_DETECT_NO_CUP);
	AddTfEnumItemToComBoBox(ui->CheckProcessCombo1, ::im::tf::MaintProcCheckSubType::MODULE_DETECT_NULL_CUP);
	AddTfEnumItemToComBoBox(ui->CheckProcessCombo1, ::im::tf::MaintProcCheckSubType::MODULE_DETECT_SUB_CUP);
	AddTfEnumItemToComBoBox(ui->CheckProcessCombo1, ::im::tf::MaintProcCheckSubType::MODULE_DETECT_SUB_A_ACCURACY);
	AddTfEnumItemToComBoBox(ui->CheckProcessCombo1, ::im::tf::MaintProcCheckSubType::MODULE_DETECT_SUB_B_ACCURACY);

    // 下拉框2
    AddTfEnumItemToComBoBox(ui->CheckProcessCombo2, ::im::tf::MaintProcCheckSubType::MODULE_MAGTRAY_INJECT_ACCURACY);
    AddTfEnumItemToComBoBox(ui->CheckProcessCombo2, ::im::tf::MaintProcCheckSubType::MODULE_MAGTRAY_SUCK_ACCURACY);

    // 下拉框3
	AddTfEnumItemToComBoBox(ui->CheckProcessCombo3, ::im::tf::MaintProcCheckSubType::MODULE_SAMPLE_PMT);
	AddTfEnumItemToComBoBox(ui->CheckProcessCombo3, ::im::tf::MaintProcCheckSubType::MODULE_SAMPLE_ACCURACY);
	AddTfEnumItemToComBoBox(ui->CheckProcessCombo3, ::im::tf::MaintProcCheckSubType::MODULE_DILUTE_ACCURACY);
	AddTfEnumItemToComBoBox(ui->CheckProcessCombo3, ::im::tf::MaintProcCheckSubType::MODULE_REAGENT_PMT);
	AddTfEnumItemToComBoBox(ui->CheckProcessCombo3, ::im::tf::MaintProcCheckSubType::MODULE_REAGENT_ACCURACY);

    // 重复次数
    QStringList strItemLst1;
    QStringList strItemLst2;
    QStringList strItemLst3;
    strItemLst1 << "1" << "5" << "10" << "20" << "50" << "100";
    strItemLst2 << "1" << "2" << "3" << "4" << "5";
    strItemLst3 << "1" << "2" << "3" << "5" << "10" << "20" << "50" << "100";
    ui->RepeatTimeCombo1->addItems(strItemLst1);
    ui->RepeatTimeCombo2->addItems(strItemLst2);
    ui->RepeatTimeCombo3->addItems(strItemLst3);

    ui->MagneticNoCombo->addItem(QString::number(1));
    ui->MagneticNoCombo->addItem(QString::number(2));

    ui->RGntHandNCombo->addItem(QString::number(1));
    ui->RGntHandNCombo->addItem(QString::number(2));

    // 根据设备列表创建单选框
    QHBoxLayout* pHlayout = new QHBoxLayout(ui->DevListFrame);
    pHlayout->setMargin(0);
    pHlayout->setSpacing(10);
    pHlayout->addStretch(1);

    // 获取设备列表
    if (!gUiAdapterPtr()->GetGroupDevNameList(m_strDevNameList, tf::DeviceType::DEVICE_TYPE_I6000))
    {
        ULOG(LOG_ERROR, "%s(), GetDevNameList() failed", __FUNCTION__);
        return;
    }

    // 依次添加具体设备
    QRadioButton* pFirstRBtn = Q_NULLPTR;
    for (const auto& strDevName : m_strDevNameList)
    {
        QRadioButton* pRBtn = new QRadioButton(strDevName);
        if (strDevName == m_strDevNameList.front())
        {
            m_strCurDevName = strDevName;
            pRBtn->setChecked(true);
            pFirstRBtn = pRBtn;
        }

        connect(pRBtn, SIGNAL(clicked()), this, SLOT(OnDevRBtnClicked()));
        pHlayout->addWidget(pRBtn);
    }

    // 第一个按钮检查
    if (pFirstRBtn == Q_NULLPTR)
    {
        ULOG(LOG_INFO, "%s(), device Empty", __FUNCTION__);
        return;
    }

    // 如果仅一个设备，不允许点击
    if (m_strDevNameList.size() <= 1)
    {
        pFirstRBtn->setEnabled(false);
    }

    // 设置编辑框左对齐且冒泡提示文本
    QList<QLineEdit*> lstRltEdit;
    lstRltEdit.push_back(ui->MaxValEdit);
    lstRltEdit.push_back(ui->NoiseEdit);
    lstRltEdit.push_back(ui->AvgValEdit1);
    lstRltEdit.push_back(ui->CvEdit1);
    lstRltEdit.push_back(ui->RelativeSubValEdit1);
    lstRltEdit.push_back(ui->AvgValEdit2);
    lstRltEdit.push_back(ui->CvEdit2);
    lstRltEdit.push_back(ui->RelativeSubValEdit2);
    for (QLineEdit* pRltEdit : lstRltEdit)
    {
        pRltEdit->setAlignment(Qt::AlignLeft);
        connect(pRltEdit, SIGNAL(textChanged(const QString&)), this, SLOT(OnRltEditTextChanged(const QString&)));
    }
}

///
/// @brief
///     界面显示后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月15日，新建函数
///
void ProcessCheckDlg::InitAfterShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 初始化字符串资源
    InitStrResource();

    // 初始化子控件
    InitChildCtrl();

    // 初始化信号槽连接
    InitConnect();
}

///
/// @brief
///     初始化字符串资源
///
/// @par History:
/// @li 4170/TangChuXian，2022年1月17日，新建函数
///
void ProcessCheckDlg::InitStrResource()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
}

///
/// @brief
///     初始化信号槽连接
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月15日，新建函数
///
void ProcessCheckDlg::InitConnect()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 关闭按钮被点击
    connect(ui->CloseBtn, SIGNAL(clicked()), this, SLOT(reject()));

	// 执行按钮被点击
	connect(ui->ExcuteBtn, SIGNAL(clicked()), this, SLOT(OnExcuteBtnClicked()));

	// 停止按钮被点击
	connect(ui->StopBtn, SIGNAL(clicked()), this, SLOT(OnStopBtnClicked()));

    // 试剂针/样本针切换
    connect(ui->CheckProcessCombo3, SIGNAL(activated(int)), this, SLOT(OnCheckProcessCombo3Changed()));

    // 导出按钮被点击
    connect(ui->ExportDataBtn, SIGNAL(clicked()), this, SLOT(OnExportBtnClicked()));

    // 清除按钮被点击
    connect(ui->ClearBtn, SIGNAL(clicked()), this, SLOT(OnClearBtnClicked()));

    // 选中执行项改变
    connect(ui->DectCB, SIGNAL(clicked()), this, SLOT(OnExeModuleClicked()));
    connect(ui->MagneticCB, SIGNAL(clicked()), this, SLOT(OnExeModuleClicked()));
    connect(ui->RefuelCB, SIGNAL(clicked()), this, SLOT(OnExeModuleClicked()));

    // 选中执行项改变
    connect(ui->SigValTbl12->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(OnScrollTbl()));
    connect(ui->SigValTbl22->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(OnScrollTbl()));
    connect(ui->SigValTbl32->verticalScrollBar(), SIGNAL(valueChanged(int)), this, SLOT(OnScrollTbl()));

    // 注册发光剂消息
    REGISTER_HANDLER(MSG_ID_MAINTAIN_DETECT_UPDATE, this, ShowDetectSignals);
}

///
/// @brief
///     初始化子控件
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月15日，新建函数
///
void ProcessCheckDlg::InitChildCtrl()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 设置标题
    SetTitleName(tr("流程检查"));

    // 根据选择样本针还是试剂针进行不同显示
    OnCheckProcessCombo3Changed();

//     QStringList strHeaderList;
//     strHeaderList << tr("序号") << tr("登录名") << tr("用户名") << tr("用户权限") << tr("自动登录");
//     ui->UserTbl->setHorizontalHeaderLabels(strHeaderList);
//     ui->UserTbl->verticalHeader()->setVisible(false);
// 
//     // 设置表格选中模式为行选中，不可多选
//     ui->UserTbl->setSelectionBehavior(QAbstractItemView::SelectRows);
//     ui->UserTbl->setSelectionMode(QAbstractItemView::SingleSelection);
//     ui->UserTbl->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 列宽设置
    //ui->UserTbl->setColumnWidth(0, 200);

    // 设置表格列宽比
//     QVector<double> vScale;
//     vScale << 1.0 << 1.0 << 1.0 << 1.0 << 1.0 << 1.0;
//     SetTblColWidthScale(ui->DefaultQcTbl, vScale);


    // 设置绝对列宽
//     QVector<double> vColWidth;
//     vColWidth << 162.0 << 160.0 << 161.0 << 160.0 << 161.0 << 131.0;
//     SetTblColAbsWidthScale(ui->DefaultQcTbl, vColWidth);

    // 更新用户信息表
    //UpdateUserInfoTbl();
}

///
/// @brief
///     窗口显示事件
///
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月12日，新建函数
///
void ProcessCheckDlg::showEvent(QShowEvent *event)
{
    // 调用基类接口
    BaseDlg::showEvent(event);

    // 第一次显示时初始化
    if (!m_bInit)
    {
        m_bInit = true;
        InitAfterShow();
    }
}

///
/// @brief
///     重置表格
///
/// @par History:
/// @li 4170/TangChuXian，2024年4月9日，新建函数
///
void ProcessCheckDlg::ResetTbl()
{
    // 设置表格行列数
    ui->SigValTbl11->setRowCount(m_ciDefaultRowCnt);
    ui->SigValTbl11->clearContents();
    ui->SigValTbl12->setRowCount(m_ciDefaultRowCnt);
    ui->SigValTbl12->clearContents();
    ui->SigValTbl21->setRowCount(m_ciDefaultRowCnt);
    ui->SigValTbl21->clearContents();
    ui->SigValTbl22->setRowCount(m_ciDefaultRowCnt);
    ui->SigValTbl22->clearContents();
    ui->SigValTbl31->setRowCount(m_ciDefaultRowCnt);
    ui->SigValTbl31->clearContents();
    ui->SigValTbl32->setRowCount(m_ciDefaultRowCnt);
    ui->SigValTbl32->clearContents();

    // 清空编辑框
    ui->MaxValEdit->clear();
    ui->NoiseEdit->clear();
    ui->AvgValEdit1->clear();
    ui->CvEdit1->clear();
    ui->RelativeSubValEdit1->clear();
    ui->AvgValEdit2->clear();
    ui->CvEdit2->clear();
    ui->RelativeSubValEdit2->clear();
}

///
/// @brief
///     更新信号值表和编辑框
///
/// @par History:
/// @li 4170/TangChuXian，2024年4月9日，新建函数
///
void ProcessCheckDlg::UpdateSigValTblAndEdit()
{
    // 结果为空直接返回
    if (m_vSigVal.isEmpty())
    {
        return;
    }

    // 匹配对应控件
    QLineEdit* pMaxValEdit = Q_NULLPTR;
    QLineEdit* pAvgValEdit = Q_NULLPTR;
    QLineEdit* pCvValEdit = Q_NULLPTR;
    QLineEdit* pRSubValEdit = Q_NULLPTR;
    QLineEdit* pNoiseEdit = Q_NULLPTR;
    QTableWidget* pSigValTbl1 = Q_NULLPTR;
    QTableWidget* pSigValTbl2 = Q_NULLPTR;

    // 判断选中模块
    if (m_enTfMtType == tf::MaintainItemType::type::MAINTAIN_ITEM_DETECT_TEST)
    {
        pMaxValEdit = ui->MaxValEdit;
        pNoiseEdit = ui->NoiseEdit;
        pSigValTbl1 = ui->SigValTbl11;
        pSigValTbl2 = ui->SigValTbl12;
    }
    else if (m_enTfMtType == tf::MaintainItemType::type::MAINTAIN_ITEM_MAGTRAY_ACCURACY_TEST)
    {
        pAvgValEdit = ui->AvgValEdit1;
        pCvValEdit = ui->CvEdit1;
        pRSubValEdit = ui->RelativeSubValEdit1;
        pSigValTbl1 = ui->SigValTbl21;
        pSigValTbl2 = ui->SigValTbl22;
    }
    else if (m_enTfMtType == tf::MaintainItemType::type::MAINTAIN_ITEM_INJECT_NEEDLE_TEST)
    {
        pAvgValEdit = ui->AvgValEdit2;
        pCvValEdit = ui->CvEdit2;
        pRSubValEdit = ui->RelativeSubValEdit2;
        pSigValTbl1 = ui->SigValTbl31;
        pSigValTbl2 = ui->SigValTbl32;
    }
    else
    {
        return;
    }

    // 算出最大值和平均值
    auto itMax = std::max_element(m_vSigVal.begin(), m_vSigVal.end());
    if (pMaxValEdit != Q_NULLPTR && itMax != m_vSigVal.end())
    {
        pMaxValEdit->setText(QString::number(*itMax, 'f', ATTEN_FACTOR_PERCISION));
    }

    // 平均值
    double dAvgVal = 0.0;
    if (!m_vSigVal.isEmpty())
    {
        // 规避除0风险
        dAvgVal = std::accumulate(m_vSigVal.begin(), m_vSigVal.end(), 0.0) / m_vSigVal.size();
    }

    // 平均值为0则返回
    if (dAvgVal <= DBL_EPSILON)
    {
        return;
    }

    // 填入编辑框
    if (pAvgValEdit != Q_NULLPTR)
    {
        pAvgValEdit->setText(QString::number(dAvgVal, 'f', ATTEN_FACTOR_PERCISION));
    }

    // 绝对极差
    auto itMin = std::min_element(m_vSigVal.begin(), m_vSigVal.end());
    if (pRSubValEdit != Q_NULLPTR && itMax != m_vSigVal.end() && itMin != m_vSigVal.end())
    {
        double dRSubVal = (*itMax - *itMin) * 100 / dAvgVal;
        pRSubValEdit->setText(QString::number(dRSubVal, 'f', ATTEN_FACTOR_PERCISION));
    }

    // 先计算标准差
    double dSD = 0.0;
    double dVariance = 0.0;
    for (double dSignal : m_vSigVal)
    {
        dVariance += std::pow(dSignal - dAvgVal, 2);
    }
    dVariance = dVariance / m_vSigVal.size();
    dSD = std::sqrt(dVariance);

    // 变异系数
    double dCV = dSD * 100 / dAvgVal;
    if (pCvValEdit != Q_NULLPTR)
    {
        pCvValEdit->setText(QString::number(dCV, 'f', ATTEN_FACTOR_PERCISION));
    }

    // 仪器噪声
    double dNoiseVal = dAvgVal + (2 * dSD);
    if (pNoiseEdit != Q_NULLPTR)
    {
        pNoiseEdit->setText(QString::number(dNoiseVal, 'f', ATTEN_FACTOR_PERCISION));
    }

    // 更新表格
    int iSeqNo = 1;
    int iRow = 0;
    QTableWidget* pCurTbl = Q_NULLPTR;
    for (double dSigVal : m_vSigVal)
    {
        if (iSeqNo % 2 == 1)
        {
            pCurTbl = pSigValTbl1;
        }
        else
        {
            pCurTbl = pSigValTbl2;
        }

        // 行自增
        if (iRow >= pCurTbl->rowCount())
        {
            pSigValTbl1->setRowCount(iRow + 1);
            pSigValTbl2->setRowCount(iRow + 1);
        }

        // 插入数据
        pCurTbl->setItem(iRow, 0, new QTableWidgetItem(QString::number(iSeqNo)));
        pCurTbl->setItem(iRow, 1, new QTableWidgetItem(QString::number(dSigVal, 'f', 0)));

        // 在第二个表则行号自增
        if (iSeqNo % 2 == 0)
        {
            ++iRow;
        }

        // 序号自增
        ++iSeqNo;
    }
}

///
/// @brief
///     执行按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月12日，新建函数
///
void ProcessCheckDlg::OnExcuteBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 设备序列号
    QString strDevSn = gUiAdapterPtr()->GetDevSnByName(m_strCurDevName);

	// 仪器状态判断
    // 获取选中设备信息
    auto spDevInfo = CommonInformationManager::GetInstance()->GetDeviceInfo(strDevSn.toStdString());
    if (spDevInfo == Q_NULLPTR)
    {
        ULOG(LOG_INFO, "%s(), spDevInfo == Q_NULLPTR", __FUNCTION__);
        return;
    }

    // 如果不是待机或预热状态，则返回
    if ((::tf::DeviceWorkState::DEVICE_STATUS_WARMUP != spDevInfo->status) &&
        (::tf::DeviceWorkState::DEVICE_STATUS_STANDBY != spDevInfo->status))
    {
        ULOG(LOG_INFO, "%s(), device status do not suport!", __FUNCTION__);
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("当前仪器状态不允许执行!")));
        pTipDlg->exec();
        return;
    }

	// 填充参数 ...
	std::vector<int32_t> lstParam;

	::tf::MaintainItemType::type maintType;
	::im::tf::MaintProcCheckSubType::type subType;
	if (ui->DectCB->isChecked())
	{
		maintType = ::tf::MaintainItemType::type::MAINTAIN_ITEM_DETECT_TEST;
		subType	= (::im::tf::MaintProcCheckSubType::type)ui->CheckProcessCombo1->currentData().toInt();
		lstParam.emplace_back(subType);
		lstParam.emplace_back(ui->RepeatTimeCombo1->currentText().toUInt());
	}
	else if(ui->MagneticCB->isChecked())
	{
		maintType = ::tf::MaintainItemType::type::MAINTAIN_ITEM_MAGTRAY_ACCURACY_TEST;
		subType = (::im::tf::MaintProcCheckSubType::type)ui->CheckProcessCombo2->currentData().toInt();
		lstParam.emplace_back(subType);
		lstParam.emplace_back(ui->RepeatTimeCombo2->currentText().toUInt());
		lstParam.emplace_back(ui->AddRgntVolEdit1->text().toUInt());
		lstParam.emplace_back(ui->MagneticNoCombo->currentText().toUInt());
	}
	else if (ui->RefuelCB->isChecked())
	{
		maintType = ::tf::MaintainItemType::type::MAINTAIN_ITEM_INJECT_NEEDLE_TEST;
		subType = (::im::tf::MaintProcCheckSubType::type)ui->CheckProcessCombo3->currentData().toInt();
		lstParam.emplace_back(subType);
		lstParam.emplace_back(ui->RepeatTimeCombo3->currentText().toUInt());
		lstParam.emplace_back(ui->AddRgntVolEdit2->text().toUInt());
		lstParam.emplace_back(ui->RGntHandNCombo->currentText().toUInt());
	}
	else
	{
		// 都没有选中选项卡
		return;
	}

    // 清空结果记录
    m_vSigVal.clear();

    // 记录执行的维护类型
    m_enTfMtType = maintType;

	std::vector<std::string> deviceSNs;
    deviceSNs.push_back(strDevSn.toStdString());
	im::LogicControlProxy::ProcessCheck(maintType, deviceSNs, lstParam);

    // 重置表
    ResetTbl();

    // 记录执行的设备名
    m_strLastExeDevName = m_strCurDevName;
}

///
/// @brief
/// .....停止按钮被点击
///
/// @return 
///
/// @par History:
/// @li 4058/WangZhiNang，2024年3月6日，新建函数
///
void ProcessCheckDlg::OnStopBtnClicked()
{
    // 设备序列号
    QString strDevSn = gUiAdapterPtr()->GetDevSnByName(m_strCurDevName);

	// 获取勾选的按钮列表和设备SN
	std::vector<::tf::DevicekeyInfo> SnAndModelIndexs;
	::tf::DevicekeyInfo dk;
	dk.sn			= strDevSn.toStdString();
	dk.modelIndex	= 1;
	SnAndModelIndexs.emplace_back(dk);

	// 执行维护
	if (!DcsControlProxy::GetInstance()->StopMaintain(SnAndModelIndexs))
	{
		ULOG(LOG_ERROR, "%s(), Maintain() failed", __FUNCTION__);
		return;
	}
}

///
/// @brief
///     样本针/试剂针选项改变
///
/// @par History:
/// @li 4170/TangChuXian，2024年7月9日，新建函数
///
void ProcessCheckDlg::OnCheckProcessCombo3Changed()
{
    // 默认显示试剂臂号
    ui->RGntHandNoLab->setVisible(true);
    ui->RGntHandNCombo->setVisible(true);

    // 获取当前选中项
    int iData = ui->CheckProcessCombo3->currentData().toInt();
    if (iData < 0)
    {
        ULOG(LOG_ERROR, "%s(), iData < 0", __FUNCTION__);
        return;
    }

    // 如果选中的是样本针，则隐藏
    ::im::tf::MaintProcCheckSubType::type enType = ::im::tf::MaintProcCheckSubType::type(iData);
    if (enType == ::im::tf::MaintProcCheckSubType::MODULE_SAMPLE_PMT ||
        enType == ::im::tf::MaintProcCheckSubType::MODULE_SAMPLE_ACCURACY ||
        enType == ::im::tf::MaintProcCheckSubType::MODULE_DILUTE_ACCURACY)
    {
        ui->RGntHandNoLab->setVisible(false);
        ui->RGntHandNCombo->setVisible(false);
    }
}

///
/// @brief
///     执行模块被点击
///
/// @par History:
/// @li 4170/TangChuXian，2024年4月9日，新建函数
///
void ProcessCheckDlg::OnExeModuleClicked()
{
    // 获取信号发送者
    QCheckBox* pCkBox = qobject_cast<QCheckBox*>(sender());
    if (pCkBox == Q_NULLPTR)
    {
        return;
    }

    // 被点击的选中，其他取消勾选
    ui->DectCB->setChecked(ui->DectCB == pCkBox);
    ui->MagneticCB->setChecked(ui->MagneticCB == pCkBox);
    ui->RefuelCB->setChecked(ui->RefuelCB == pCkBox);
}

///
/// @brief
///     设备单选框被点击
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月28日，新建函数
///
void ProcessCheckDlg::OnDevRBtnClicked()
{
    // 获取信号发送者
    QRadioButton* pRBtn = qobject_cast<QRadioButton*>(sender());
    if (pRBtn == Q_NULLPTR)
    {
        return;
    }

    // 如果更新当前设备名
    m_strCurDevName = pRBtn->text();

    // 判断当前设备是否为上一次执行设备
    if (m_strCurDevName == m_strLastExeDevName)
    {
        // 是则刷新表格数据
        UpdateSigValTblAndEdit();
    }
    else
    {
        // 重置表
        ResetTbl();
    }
}

///
///  @brief:	显示信号值
///
///  @param[in]   mit    维护项类型
///  @param[in]   sigValue  信号值
///
///  @par History: 
///  @li 7656/zhang.changjiang，2022年7月27日，新建函数
///
void ProcessCheckDlg::ShowDetectSignals(tf::MaintainItemType::type mit, double sigValue)
{
    // 如果不是发光剂、磁分离、注样流程，则返回
    if (mit != tf::MaintainItemType::type::MAINTAIN_ITEM_DETECT_TEST &&
        mit != tf::MaintainItemType::type::MAINTAIN_ITEM_MAGTRAY_ACCURACY_TEST &&
        mit != tf::MaintainItemType::type::MAINTAIN_ITEM_INJECT_NEEDLE_TEST)
    {
        return;
    }

    // 记录信号值
    m_vSigVal.push_back(sigValue);

    // 更新界面显示
    if (m_strCurDevName == m_strLastExeDevName)
    {
        UpdateSigValTblAndEdit();
    }
}

///
/// @brief
///     滚动表格
///
/// @par History:
/// @li 4170/TangChuXian，2024年4月9日，新建函数
///
void ProcessCheckDlg::OnScrollTbl()
{
    QScrollBar* pScrollBar = qobject_cast<QScrollBar*>(sender());
    if (pScrollBar == Q_NULLPTR)
    {
        return;
    }

    QScrollBar* pRelateBar = Q_NULLPTR;
    if (pScrollBar == ui->SigValTbl12->verticalScrollBar())
    {
        pRelateBar = ui->SigValTbl11->verticalScrollBar();
    }
    if (pScrollBar == ui->SigValTbl22->verticalScrollBar())
    {
        pRelateBar = ui->SigValTbl21->verticalScrollBar();
    }
    if (pScrollBar == ui->SigValTbl32->verticalScrollBar())
    {
        pRelateBar = ui->SigValTbl31->verticalScrollBar();
    }
    else
    {
        return;
    }

    if (pRelateBar == Q_NULLPTR)
    {
        return;
    }

    // 滚动条联动
    pRelateBar->setValue(pScrollBar->value());
}

///
/// @brief
///     导出按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2024年4月9日，新建函数
///
void ProcessCheckDlg::OnExportBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 弹出保存文件对话框
    QString strFileName = QFileDialog::getSaveFileName(this, tr("保存为..."), QString(), tr("CSV files (*.csv)"));
    if (strFileName.isEmpty())
    {
        ULOG(LOG_INFO, "File path is empty!");
        return;
    }

    // 记录导出文本
    QStringList strExportTextList;
    QString strInfo;
    QLabel* pLab1 = Q_NULLPTR;
    QLabel* pLab2 = Q_NULLPTR;
    QLabel* pLab3 = Q_NULLPTR;
    QLineEdit* pEdit1 = Q_NULLPTR;
    QLineEdit* pEdit2 = Q_NULLPTR;
    QLineEdit* pEdit3 = Q_NULLPTR;

    // 判断选中模块
    if (ui->DectCB->isChecked())
    {
        pEdit1 = ui->MaxValEdit;
        pEdit2 = ui->NoiseEdit;
        pLab1 = ui->MaxValLab;
        pLab2 = ui->NoiseLab;
    }
    else if (ui->MagneticCB->isChecked())
    {
        pEdit1 = ui->AvgValEdit1;
        pEdit2 = ui->CvEdit1;
        pEdit3 = ui->RelativeSubValEdit1;
        pLab1 = ui->AvgValLab1;
        pLab2 = ui->CvLab1;
        pLab3 = ui->RelativeSubValLab1;
    }
    else if (ui->RefuelCB->isChecked())
    {
        pEdit1 = ui->AvgValEdit2;
        pEdit2 = ui->CvEdit2;
        pEdit3 = ui->RelativeSubValEdit2;
        pLab1 = ui->AvgValLab2;
        pLab2 = ui->CvLab2;
        pLab3 = ui->RelativeSubValLab2;
    }
    else
    {
        return;
    }

    // 逐个添加信息
    if (pEdit1 != Q_NULLPTR && pLab1 != Q_NULLPTR)
    {
        strInfo = pLab1->text() + "\t" + pEdit1->text();
        strExportTextList.push_back(strInfo);
    }

    if (pEdit2 != Q_NULLPTR && pLab2 != Q_NULLPTR)
    {
        strInfo = pLab2->text() + "\t" + pEdit2->text();
        strExportTextList.push_back(strInfo);
    }

    if (pEdit3 != Q_NULLPTR && pLab3 != Q_NULLPTR)
    {
        strInfo = pLab3->text() + "\t" + pEdit3->text();
        strExportTextList.push_back(strInfo);
    }

    for (int iIndex = 0; iIndex < m_vSigVal.size() && iIndex < m_vSigVal.size(); iIndex++)
    {
        auto sigValue = m_vSigVal.at(iIndex);
        strInfo = tr("第") + QString::number(iIndex + 1) + tr("次检测信号值为：") + "\t" + QString::number(sigValue, 'f', 0);
        strExportTextList.push_back(strInfo);
    }

    // 导出
    std::shared_ptr<FileExporter> pFileEpt(new FileExporter());
    bool bRect = pFileEpt->ExportInfoToFile(strExportTextList, strFileName);

    // 弹框提示导出
    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(bRect ? tr("导出完成!") : tr("导出失败！")));
    pTipDlg->exec();
}

///
/// @brief
///     清除按钮被点击
///
/// @par History:
/// @li 4170/TangChuXian，2024年4月12日，新建函数
///
void ProcessCheckDlg::OnClearBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 清空数据
    m_vSigVal.clear();

    // 重置表格
    ResetTbl();
}


///
/// @brief
///     结果编辑框文本改变
///
/// @param[in]  strNewText  新的文本
///
/// @par History:
/// @li 4170/TangChuXian，2024年6月13日，新建函数
///
void ProcessCheckDlg::OnRltEditTextChanged(const QString& strNewText)
{
    // 获取信号发送者
    QLineEdit* pEdit = qobject_cast<QLineEdit*>(sender());
    if (pEdit == Q_NULLPTR)
    {
        return;
    }

    // 设置冒泡文本
    pEdit->setToolTip(strNewText);
    pEdit->setCursorPosition(0);
}
