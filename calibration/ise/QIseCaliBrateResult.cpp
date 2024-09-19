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
/// @file     QIseCaliBrateResult.cpp
/// @brief    校准结果
///
/// @author   8090/YeHuaNing
/// @date     2023年4月10日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8090/YeHuaNing，2022年7月18日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QIseCaliBrateResult.h"
#include "ui_QIseCaliBrateResult.h"
#include <QFileDialog>
#include <QDateTime>
#include <QStandardItemModel>
#include "../ch/CaliBrateCommom.h"
#include "thrift/ise/IseLogicControlProxy.h"
#include "thrift/ise/ise1005/Ise1005LogicControlProxy.h"
#include "src/thrift/ise/gen-cpp/ise_constants.h"
#include "manager/UserInfoManager.h"
#include "manager/DictionaryQueryManager.h"
#include "curveDraw/common.h"

#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/tipdlg.h"
#include "shared/FileExporter.h"
#include "shared/ReagentCommon.h"
#include "shared/datetimefmttool.h"
#include "shared/CommonInformationManager.h"
#include "Serialize.h"
#include "printcom.hpp"

#include "src/common/Mlog/mlog.h"
#include "src/common/common.h"


QIseCaliBrateResult::QIseCaliBrateResult(QWidget *parent)
	: BaseDlg(parent),
    m_curModel(nullptr),
    m_hisModel(nullptr),
	m_currentIndex(-1)
{
    ui = new Ui::QIseCaliBrateResult();
	ui->setupUi(this);
	SetTitleName(tr("校准结果"));
	Init();
}

QIseCaliBrateResult::~QIseCaliBrateResult()
{
}

void QIseCaliBrateResult::OnUpdateWorkCurve()
{
	// 判断是否选中有历史曲线
	if (m_currentIndex <= 0 || m_currentIndex > m_historyCurve.size())
	{
		return;
	}

	std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("更新工作曲线会影响测定结果，是否确认更新?"), TipDlgType::TWO_BUTTON));
	if (pTipDlg->exec() == QDialog::Rejected)
	{
		ULOG(LOG_INFO, "Cancel mask!");
		return;
	}

	auto curve = m_historyCurve[m_currentIndex - 1];
    // 使用新的成员去修改，避免客户端将其它状态修改了
    ::ise::tf::IseModuleInfo imi;
    imi.__set_id(m_module.id);
    imi.__set_caliCurveId(curve.id);
    
    // 重设当前工作曲线信息
	if (!ise::LogicControlProxy::ModifyIseModuleInfo(imi))
	{
		ULOG(LOG_ERROR, "QueryReagentGroup() failed");
        std::shared_ptr<TipDlg> pFailTipDlg(new TipDlg(tr("更新失败！"), TipDlgType::SINGLE_BUTTON));
        pFailTipDlg->exec();
		return;
	}

    m_module.caliCurveId = curve.id;
    StartDialog(m_module);
    // 通知界面其它界面更新
    //POST_MESSAGE(MSG_ID_ISE_CALI_APP_UPDATE, std::vector<ise::tf::IseModuleInfo>{m_module});
}

void QIseCaliBrateResult::StartDialog(ise::tf::IseModuleInfo& module)
{
    std::shared_ptr<UserInfoManager> userPms = UserInfoManager::GetInstance();
    ui->update_btn->setVisible(userPms->IsPermisson(PMS_CALI_RESULT_MANUAL_UPDATE_CUR));
    ui->export_none_flat_btn->setVisible(userPms->IsPermisson(PSM_IM_PRINTEXPORT_CALIRESULT));
    ui->update_btn->setEnabled(false);

    m_module = module;
    
    // 清空标签信息和表格数据
    InitCaliStatus();
    InitTableModel();

    // 获取当前校准和历史校准信息
    auto curCurve = GetCurrentCurve();
    GetHistoryCurves();

    // 设置当前曲线信息
    if (curCurve)
    {
        ShowCaliCurveStatus(GetCaliStatusShow(*curCurve));
        ShowCurveCoreData(m_curModel, GetCurveData(*curCurve));
        SetPrintAndExportEnabled(true);
    }
    else
    {
        SetPrintAndExportEnabled(false);
    }

    // 设置历史校准时间数据
    SetCaliTimesModelData();
}

void QIseCaliBrateResult::Init()
{
	// 关闭按钮
	connect(ui->close_btn, &QPushButton::clicked, this, [&]() {this->close(); });
	// 更新工作曲线
	connect(ui->update_btn, SIGNAL(clicked()), this, SLOT(OnUpdateWorkCurve()));
	// 打印按钮被点击
	connect(ui->print_none_flat_btn, &QPushButton::clicked, this, &QIseCaliBrateResult::OnPrintBtnClicked);
	// 导出按钮被点击
	connect(ui->export_none_flat_btn, &QPushButton::clicked, this, &QIseCaliBrateResult::OnExportBtnClicked);
    // 选择校准时间
    connect(ui->curve_his_times, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), [=](int index) { 
        m_currentIndex = index;
        InitHisCurveModel();
        if (index > 0 && index <= m_historyCurve.size())
        {
            ShowCurveCoreData(m_hisModel, GetCurveData(m_historyCurve[index - 1]));

            // 选择了历史曲线后，可以点击更新曲线按钮
            ui->update_btn->setEnabled(true);
        }
        else
        {
            // 禁用更新曲线按钮
            ui->update_btn->setEnabled(false);
        }
    });

    InitCaliStatus();

    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);
}

std::shared_ptr<ise::tf::CaliCurve> QIseCaliBrateResult::GetCurrentCurve()
{
    ise::tf::IseCaliCurveQueryResp qryResp;
    ise::tf::IseCaliCurveQueryCond qryCond;
    qryCond.__set_id(m_module.caliCurveId);

    ise::ise1005::LogicControlProxy::QueryIseCaliCurve(qryResp, qryCond);

    if (!qryResp.lstIseCaliCurves.empty())
        return std::make_shared<ise::tf::CaliCurve>(qryResp.lstIseCaliCurves[0]);

    return nullptr;
}

void QIseCaliBrateResult::InitCurCurveModel()
{
    QStringList hTitles;
    QStringList vTitles;
    hTitles << tr("低值") << tr("基准") << tr("高值") << tr("基准") << tr("斜率") << tr("稀释倍数") << tr("数据报警");
    vTitles << tr("Na") << tr("K") << tr("Cl");

    // 清空当前结果
    if (m_curModel == nullptr)
    {
        m_curModel = new QStandardItemModel(this);
        ui->curve_cur->setModel(m_curModel);
        ui->curve_cur->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->curve_cur->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->curve_cur->verticalHeader()->setDefaultAlignment(Qt::AlignCenter);
    }

    m_curModel->clear();
    // 纵向标题
    m_curModel->setHorizontalHeaderLabels(hTitles);
    // 横向
    m_curModel->setVerticalHeaderLabels(vTitles);
}

void QIseCaliBrateResult::InitHisCurveModel()
{
    QStringList hTitles;
    QStringList vTitles;
    hTitles << tr("低值") << tr("基准") << tr("高值") << tr("基准") << tr("斜率") << tr("稀释倍数") << tr("数据报警");
    vTitles << tr("Na") << tr("K") << tr("Cl");

    // 清空历史结果
    if (m_hisModel == nullptr)
    {
        m_hisModel = new QStandardItemModel(this);
        ui->curve_his->setModel(m_hisModel);
        ui->curve_his->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->curve_his->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        ui->curve_his->verticalHeader()->setDefaultAlignment(Qt::AlignCenter);
    }

    m_hisModel->clear();
    // 纵向标题
    m_hisModel->setHorizontalHeaderLabels(hTitles);
    // 横向
    m_hisModel->setVerticalHeaderLabels(vTitles);
}

void QIseCaliBrateResult::InitCaliTimesModel()
{
    // 清空历史结果校准时间数据
    ui->curve_his_times->clear();
}

void QIseCaliBrateResult::GetHistoryCurves()
{
    ise::tf::IseCaliCurveQueryResp qryResp;
    ise::tf::IseCaliCurveQueryCond qryCond;
    tf::LimitCond limtidCon;
    limtidCon.__set_offset(0);
    limtidCon.__set_count(50);
    ::tf::OrderByCond orderCnd;
    orderCnd.__set_asc(false);
    orderCnd.__set_fieldName("caliTime");

    qryCond.__set_deviceSn({ m_module.deviceSn });
    qryCond.__set_moduleIndex(m_module.moduleIndex);
    qryCond.__set_sampleType(m_module.sampleType);
    qryCond.__set_successed(true);
    qryCond.__set_limit(limtidCon);
    qryCond.orderByConds.push_back(orderCnd);

    ise::ise1005::LogicControlProxy::QueryIseCaliCurve(qryResp, qryCond);

    m_historyCurve.clear();
    m_historyCurve = std::move(qryResp.lstIseCaliCurves);
}

void QIseCaliBrateResult::InitCaliStatus()
{
	static std::once_flag once;
	std::call_once(once, [&]{
        // 清除表中内容
        QList<QLabel*> memberLables = ui->layoutWidget->findChildren<QLabel*>();
        for (auto meber : memberLables)
        {
            int index = CaliBrateCommom::GetKeyValueFromWidgetName(meber);
            if (index == -1)
            {
                continue;
            }

            m_labelMap.insert(std::make_pair(index, meber));
        }
    });

    // 清空详细信息
    for (const auto& lable : m_labelMap)
    {
        lable.second->setText("");
    }
}

void QIseCaliBrateResult::InitTableModel()
{
    InitCurCurveModel();
    InitHisCurveModel();
    InitCaliTimesModel();
}

QStringList QIseCaliBrateResult::GetCaliStatusShow(ise::tf::CaliCurve& curve)
{
	QStringList statusList;

    if (curve.__isset.id)
    {
        const auto& devInfo =CommonInformationManager::GetInstance()->GetDeviceInfo(m_module.deviceSn);

        // 1:项目名称(ISE-S)
        statusList << QString("ISE-") + GetIseSampleName(m_module.sampleType);
        // 2:项目位置(ISE-A)
        QString moduleName;
        if (devInfo != nullptr && devInfo->moduleCount > 1)
            moduleName = GetIseModuleName(m_module.moduleIndex);
        statusList << QString::fromStdString(devInfo->name) + moduleName;
        // 3:校准时间
        statusList << ToCfgFmtDateTime(QString::fromStdString(curve.caliTime));
        // 4:单位(以Na的单位代替所有的电解质单位)
        statusList << CommonInformationManager::GetInstance()->GetCurrentUnit(ise::tf::g_ise_constants.ASSAY_CODE_NA);
        // 5:校准品批号
        statusList << QString::fromStdString(curve.calibratorLot);
        // 6:内部标准液批号
        statusList << QString::fromStdString(curve.IS_Lot);
        // 7:AT缓冲液批号
        statusList << QString::fromStdString(curve.diluentLot);
    }

	return statusList;
}

QStringList QIseCaliBrateResult::GetCurveData(ise::tf::CaliCurve & curve)
{
    QStringList rsts;

    
    rsts << GetCoreData(curve.caliData.Na, ise::tf::g_ise_constants.ASSAY_CODE_NA, curve); 
    rsts << GetCoreData(curve.caliData.K, ise::tf::g_ise_constants.ASSAY_CODE_K, curve);
    rsts << GetCoreData(curve.caliData.Cl, ise::tf::g_ise_constants.ASSAY_CODE_CL, curve);

    return rsts;
}

QStringList QIseCaliBrateResult::GetCoreData(ise::tf::CaliDataCore core, int assayCode, const ise::tf::CaliCurve& curve)
{
    QStringList  datas;

    // 低值
    datas << QString::number(core.lStdEMF.sampleEMF);
    // 基准
    datas << QString::number(core.lStdEMF.baseEMF);
    // 高值
    datas << QString::number(core.hStdEMF.sampleEMF);
    // 基准
    datas << QString::number(core.hStdEMF.baseEMF);
    // 斜率
    datas << QString::number(core.slope);
    // 稀释倍数
    datas << QString::number(core.dilutionRatio);
    // 数据报警
    datas << GetAlarmInfo(assayCode, curve);

    return datas;
}

QString QIseCaliBrateResult::GetAlarmInfo(int assayCode, const ise::tf::CaliCurve & curve)
{
    QString alarmInfo;
    // 约定按照Na、K、Cl的顺序放在置报警，并且用 | 分割
    const auto& alarmList = QString::fromStdString(curve.failedReason).split("|");
    int alarmListSize = alarmList.size();

    if (assayCode == ise::tf::g_ise_constants.ASSAY_CODE_NA && alarmListSize > 0)
    {
        alarmInfo <= alarmList[0];
    }
    else if (assayCode == ise::tf::g_ise_constants.ASSAY_CODE_K && alarmListSize > 1)
    {
        alarmInfo <= alarmList[1];
    }
    else if (assayCode == ise::tf::g_ise_constants.ASSAY_CODE_CL && alarmListSize > 2)
    {
        alarmInfo <= alarmList[2];
    }
    else
    {
        alarmInfo <= ""; // QString::number(core.itemStatus);
    }

    return alarmInfo;
}

void QIseCaliBrateResult::ShowCaliCurveStatus(const QStringList & data)
{
    for (const auto& lb : m_labelMap)
    {
        if (lb.first >= 0 && lb.first < data.size() && lb.second != nullptr)
        {
            lb.second->setText(data.at(lb.first));
        }
    }
}

void QIseCaliBrateResult::ShowCurveCoreData(QStandardItemModel * model, const QStringList & data)
{
    int i = 0;
    int rowCnt = model->rowCount();
    int colCnt = model->columnCount();

    for (const auto& d : data)
    {
        if (i >= rowCnt*colCnt)
        {
            ULOG(LOG_ERROR, "error loop!");
            return;
        }

        model->setItem(i / colCnt, i % colCnt, CenterAligmentItem(d));

        ++i;
    }
}

void QIseCaliBrateResult::SetCaliTimesModelData()
{
    QStringList data;
    data << "";

    for (const auto& c : m_historyCurve)
    {
        data << ToCfgFmtDateTime(QString::fromStdString(c.caliTime));
    }

    ui->curve_his_times->addItems(data);
}

///
/// @brief 打印按钮被点击
///
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年9月8日，新建函数
///
void QIseCaliBrateResult::OnPrintBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    CaliResultInfoIse Info;
    QString strPrintTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    Info.strPrintTime = strPrintTime.toStdString();
    GetPrintExportInfo(Info);

    std::string strInfo = GetJsonString(Info);
    ULOG(LOG_INFO, "Print datas : %s", strInfo);
    std::string unique_id;
    int irect = printcom::printcom_async_print(strInfo, unique_id);
    ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);
}

///
/// @brief 导出按钮被点击
///
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年9月8日，新建函数
///
void QIseCaliBrateResult::OnExportBtnClicked()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 弹出保存文件对话框
	QString fileName = QFileDialog::getSaveFileName(this, tr("保存为..."), QString(), tr("EXCEL files (*.xlsx);;CSV files (*.csv);;PDF files (*.pdf)"));
	if (fileName.isNull() || fileName.isEmpty())
	{
		//std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("保存文件错误！")));
		//pTipDlg->exec();
		return;
	}

    QFileInfo file(fileName);
    bool bRet = true;

    if (file.suffix() == "pdf")
    {
        CaliResultInfoIse Info;
        GetPrintExportInfo(Info);
        QString strPrintTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
        Info.strPrintTime = strPrintTime.toStdString();
        Info.strExportTime = strPrintTime.toStdString();
        std::string strInfo = GetJsonString(Info);
        ULOG(LOG_INFO, "Print datas : %s", strInfo);
        std::string unique_id;
        QString strDirPath = QCoreApplication::applicationDirPath();
        QString strUserTmplatePath = strDirPath + "/resource/exportTemplate/ExportCalibratorResultIse.lrxml";
        int irect = printcom::printcom_async_assign_export(strInfo, fileName.toStdString(), strUserTmplatePath.toStdString(), unique_id);
        ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);
        bRet = true;
    }
    else
    {
        QStringList strExportTextList;

        // 校准项目信息
        if (!SetCaliInfo(strExportTextList))
        {
            // 弹框提示导出失败
            std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("数据获取失败！")));
            pTipDlg->exec();

            return;
        }

        // 导出文件
        FileExporter fileExporter;
        bRet = fileExporter.ExportInfoToFile(strExportTextList, fileName);
    }

	// 弹框提示导出失败
	std::shared_ptr<TipDlg> pTipDlg(new TipDlg(bRet ? tr("导出成功！") : tr("导出失败！")));
	pTipDlg->exec();

}

void QIseCaliBrateResult::OnPermisionChanged()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    std::shared_ptr<UserInfoManager> userPms = UserInfoManager::GetInstance();

    // 手动更新校准曲线的权限限制
    userPms->IsPermisson(PMS_CALI_RESULT_MANUAL_UPDATE_CUR) ? ui->update_btn->show() : ui->update_btn->hide();

    // 导出
    userPms->IsPermisson(PSM_IM_PRINTEXPORT_CALIRESULT) ? ui->export_none_flat_btn->show() : ui->export_none_flat_btn->hide();
}

///
/// @brief 设置校准信息（导出使用）
///
/// @param[in] strExportTextList   导出的数据
///
/// @return true:设置成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年9月8日，新建函数
///
bool QIseCaliBrateResult::SetCaliInfo(QStringList& strExportTextList)
{
	// 创建表头
	QString strHeader("");
	strHeader += QObject::tr("项目名称") + "\t";
	strHeader += QObject::tr("位置") + "\t";
	strHeader += QObject::tr("校准时间") + "\t";
	strHeader += QObject::tr("单位") + "\t";
    strHeader += QObject::tr("校准品批号") + "\t";
    strHeader += QObject::tr("高浓度校准品位置") + "\t";
    strHeader += QObject::tr("低浓度校准品位置") + "\t";
	strHeader += QObject::tr("IC批号") + "\t";
    strHeader += QObject::tr("IC批号") + "\t";
    strHeader += QObject::tr("仪器编号") + "\t";
    strHeader += QObject::tr("软件版本") + "\t";
    strHeader += QObject::tr("校准结果") + "\t";
	strExportTextList.push_back(std::move(strHeader));

	// 获取当前曲线信息
	auto currentCurve = GetCurrentCurve();
	if (!currentCurve)
	{
		ULOG(LOG_ERROR, "Can't find the current calibrate curve %s()", __FUNCTION__);
		return false;
	}

	QString rowData = "";
	
	// 项目名称
    // 1:项目名称(ISE-S)
	rowData += QString("ISE-") + GetIseSampleName(m_module.sampleType);
	rowData += "\t";

	// 位置
    QString moduleName;
    const auto& devInfo = CommonInformationManager::GetInstance()->GetDeviceInfo(m_module.deviceSn);
    if (devInfo != nullptr && devInfo->moduleCount > 1)
        moduleName = GetIseModuleName(m_module.moduleIndex);
    rowData += QString::fromStdString(devInfo->name) + moduleName;
	rowData += "\t";

	// 校准时间
	rowData += QString::fromStdString(currentCurve->caliTime);
	rowData += "\t";

	// 单位
	rowData += CommonInformationManager::GetInstance()->GetCurrentUnit(ise::tf::g_ise_constants.ASSAY_CODE_NA);
	rowData += "\t";

	// 校准品批号
	rowData += QString::fromStdString(currentCurve->calibratorLot);
    rowData += "\t";

    // 高浓度校准品位置
    rowData +="";
    rowData += "\t";

    // 低浓度校准品位置
    rowData += "";
    rowData += "\t";

	// 内部标准液批号
	rowData += QString::fromStdString(currentCurve->IS_Lot);
	rowData += "\t";

	// AT缓冲液批号
	rowData += QString::fromStdString(currentCurve->diluentLot);
    rowData += "\t";

    // 仪器编号
    rowData += QString::fromStdString(currentCurve->deviceSN);
    rowData += "\t";

    // 软件版本
    rowData += QString::fromStdString(DictionaryQueryManager::GetInstance()->GetSoftwareVersion());
    rowData += "\t";

    // 校准结果
    rowData += true ? tr("成功") : tr("失败");
    rowData += "\t";
	strExportTextList.push_back(std::move(rowData));

	// 换行
	strExportTextList.push_back("");

	// 设置校准品测试结果信息
	QString strCaliResultHeader("");
	strCaliResultHeader +=  "\t";
	strCaliResultHeader += QObject::tr("低值") + "\t";
	strCaliResultHeader += QObject::tr("基准") + "\t";
	strCaliResultHeader += QObject::tr("高值") + "\t";
	strCaliResultHeader += QObject::tr("基准") + "\t";
	strCaliResultHeader += QObject::tr("斜率") + "\t";
	strCaliResultHeader += QObject::tr("稀释倍数") + "\t";
	strCaliResultHeader += QObject::tr("数据报警") + "\t";
	strExportTextList.push_back(std::move(strCaliResultHeader));

	// 校准结果信息
	auto caliResultInfoRow = [&](const ise::tf::CaliDataCore& core, int assayCode, std::shared_ptr<ise::tf::CaliCurve> curve)
	{
		QString rowData = "";
		// 低值
		rowData += QString::number(core.lStdEMF.sampleEMF) + "\t";
		// 基准
		rowData += QString::number(core.lStdEMF.baseEMF) + "\t";
		// 高值
		rowData += QString::number(core.hStdEMF.sampleEMF) + "\t";
		// 基准
		rowData += QString::number(core.hStdEMF.baseEMF) + "\t";
		// 斜率
		rowData += QString::number(core.slope) + "\t";
		// 稀释倍数
		rowData += QString::number(core.dilutionRatio) + "\t";
		// 状态
		rowData += GetAlarmInfo(assayCode, *curve) + "\t";

		return rowData;
	};

	QString caliResultInfoRowNa = "Na\t";
	caliResultInfoRowNa += caliResultInfoRow(currentCurve->caliData.Na, ise::tf::g_ise_constants.ASSAY_CODE_NA, currentCurve);
	strExportTextList.push_back(std::move(caliResultInfoRowNa));

	QString caliResultInfoRowK = "K\t";
	caliResultInfoRowK += caliResultInfoRow(currentCurve->caliData.K, ise::tf::g_ise_constants.ASSAY_CODE_K, currentCurve);
	strExportTextList.push_back(std::move(caliResultInfoRowK));

	QString caliResultInfoRowCl = "Cl\t";
	caliResultInfoRowCl += caliResultInfoRow(currentCurve->caliData.Cl, ise::tf::g_ise_constants.ASSAY_CODE_CL, currentCurve);
	strExportTextList.push_back(std::move(caliResultInfoRowCl));

	return true;

}

void QIseCaliBrateResult::SetPrintAndExportEnabled(bool hasCurve)
{
    // 暂时只是用是否有当前工作曲线
    ui->print_none_flat_btn->setEnabled(hasCurve);
    ui->export_none_flat_btn->setEnabled(hasCurve);
}

bool QIseCaliBrateResult::GetPrintExportInfo(CaliResultInfoIse & Info)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    /******校准信息******/
    auto currentCurve = GetCurrentCurve();
    if (!currentCurve)
    {
        ULOG(LOG_ERROR, "Can't find the current calibrate curve %s()", __FUNCTION__);
        return false;
    }

    // 1:项目名称
    Info.strReagentName = ui->label_0->text().toStdString();

    // 2:位置
    Info.strPos = ui->label_1->text().toStdString();
    Info.strModelName = Info.strPos; // 模块

    // 3:校准时间
    Info.strCalibrateDate = currentCurve->caliTime;

    // 4:单位
    Info.strUnit = CommonInformationManager::GetInstance()->GetCurrentUnit(ise::tf::g_ise_constants.ASSAY_CODE_NA).toStdString();

    // 5:校准品批号
    Info.strCalibratorLot = currentCurve->calibratorLot;

    // 高浓度校准品位置
    Info.strCalibrator1;
    // 低浓度校准品位置
    Info.strCalibrator2;

    // 6:内部标准液批号
    Info.strIsLot = currentCurve->IS_Lot;

    // 7:AT缓冲液批号
    Info.strDiluLot = currentCurve->diluentLot;

    // 仪器编号
    Info.strDeviceNum = currentCurve->deviceSN;

    // 软件版本
    Info.strSoftVersion = DictionaryQueryManager::GetInstance()->GetSoftwareVersion();

    auto spUserInfo = UserInfoManager::GetInstance()->GetLoginUserInfo();
    // 导出人
    if (spUserInfo)
        Info.strExportBy = spUserInfo->nickname;

    // 打印人
    Info.strPrintBy = Info.strExportBy;

    // 校准结果
    Info.strCaliResult = tr("成功").toStdString();

    // 校准结果信息
    auto setIseCaliData = [&](const ise::tf::CaliDataCore& core, const std::string& assayName, int assayCode, std::shared_ptr<ise::tf::CaliCurve> curve)
    {
        IseCaliResultData iseCaliData;
        // 名称
        iseCaliData.strAssayName = assayName;
        // 低值
        iseCaliData.strLowSamleEmf = QString::number(core.lStdEMF.sampleEMF).toStdString();
        // 基准
        iseCaliData.strLowBaseEmf = QString::number(core.lStdEMF.baseEMF).toStdString();
        // 高值
        iseCaliData.strHighSamleEmf = QString::number(core.hStdEMF.sampleEMF).toStdString();
        // 基准
        iseCaliData.strHighBaseEmf = QString::number(core.hStdEMF.baseEMF).toStdString();
        // 斜率
        iseCaliData.strSlope = QString::number(core.slope).toStdString();
        // 稀释倍数
        iseCaliData.strDilutionRatio = QString::number(core.dilutionRatio).toStdString();
        // 状态
        iseCaliData.strAlarm = GetAlarmInfo(assayCode, *curve).toStdString();

        Info.vecCaliResultData.emplace_back(iseCaliData);
    };

    setIseCaliData(currentCurve->caliData.Na, CommonInformationManager::GetInstance()->GetAssayNameByCode(ise::tf::g_ise_constants.ASSAY_CODE_NA), ise::tf::g_ise_constants.ASSAY_CODE_NA, currentCurve);
    setIseCaliData(currentCurve->caliData.K, CommonInformationManager::GetInstance()->GetAssayNameByCode(ise::tf::g_ise_constants.ASSAY_CODE_K), ise::tf::g_ise_constants.ASSAY_CODE_K, currentCurve);
    setIseCaliData(currentCurve->caliData.Cl, CommonInformationManager::GetInstance()->GetAssayNameByCode(ise::tf::g_ise_constants.ASSAY_CODE_CL), ise::tf::g_ise_constants.ASSAY_CODE_CL, currentCurve);
    return true;
}
