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
/// @file     ImQCaliBrateHistory.cpp
/// @brief    校准历史界面
///
/// @author   4170/TangChuXian
/// @date     2022年10月9日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月5日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "ImQCaliBrateHistory.h"
#include "ui_ImQCaliBrateHistory.h"
#include <QStandardItemModel>
#include <QVBoxLayout>
#include <QScrollBar>
#include <QCheckBox>
#include <QLabel>
#include <QSpacerItem>
#include <QDateTime>
#include <QSortFilterProxyModel>
#include <QFileDialog>
#include <qwt_plot.h>
#include <qwt_symbol.h>
#include <qwt_plot_curve.h>
#include <qwt_scale_draw.h>
#include <qwt_scale_widget.h>
#define _MATH_DEFINES_DEFINED
#include "shared/calhiscurve.h"
#include "shared/QSerialModel.h"
#include "shared/QCustomScaleDraw.h"
#include "shared/mccustomqwtmarker.h"
#include "shared/datetimefmttool.h"
#include "shared/msgiddef.h"
#include "shared/messagebus.h"
#include "shared/uicommon.h"
#include "shared/ReagentCommon.h"
#include "shared/CReadOnlyDelegate.h"
#include "shared/CommonInformationManager.h"
#include "shared/tipdlg.h"
#include "shared/FileExporter.h"
#include "src/common/common.h"

#include "thrift/im/i6000/I6000LogicControlProxy.h"
#include "manager/UserInfoManager.h"
#include "src/common/Mlog/mlog.h"
#include "SortHeaderView.h"
#include "Serialize.h"
#include "printcom.hpp"

ImQCaliBrateHistory::ImQCaliBrateHistory(QWidget *parent)
    : QWidget(parent),
    m_CurveMode(nullptr),
    m_caliDataMode(new QSerialModel),
    m_pCalHisCurve(new CalHisCurve()),
    m_pScrollBar(nullptr),
    m_pCaliSelTipMarker(),
    m_pCaliSelRing(new QwtPlotCurve),
    m_bInit(false),
	m_index(-1)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    ui = new Ui::ImQCaliBrateHistory();
    ui->setupUi(this);

    // 界面显示前初始化
    InitBeforeShow();
}

ImQCaliBrateHistory::~ImQCaliBrateHistory()
{
	ULOG(LOG_INFO, "%s", __FUNCTION__);
    delete ui;
}

///
/// @brief刷新设备试剂列表
///     
///
/// @param[in]  devices  设备列表
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月22日，新建函数
///
QWidget*  ImQCaliBrateHistory::RefreshPage(std::vector<std::shared_ptr<const tf::DeviceInfo>>& devices)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    m_strDevs.clear();
    // 根据模块依次显示
    for (auto device : devices)
    {
        m_strDevs.push_back(device->deviceSN);
    }

    // 设备列表为空则返回
    if (m_strDevs.empty())
    {
        ULOG(LOG_INFO, "deivce is empty");
        return this;
    }

    // 刷新列表
    OnRefreshHistory();

    return this;
}

///
/// @brief
///     重新刷新界面
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月14日，新建函数
///
void ImQCaliBrateHistory::OnRefreshHistory()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 设备列表为空则返回
    if (m_strDevs.empty() || m_strDevs.size() > 1)
    {
        ULOG(LOG_INFO, "deivce is error");
        return;
    }

	if (!(m_CurveMode->rowCount() > 0 && m_CurveMode->item(0) != nullptr
		&& !m_CurveMode->item(0)->text().isEmpty()))
    {
        // 清空表格内容
        ClearCurveContent();

        // 查询生化信息表
        const std::shared_ptr<const tf::DeviceInfo>& dv = CommonInformationManager::GetInstance()->GetDeviceInfo(m_strDevs[0]);
        UpdateCurveView(dv->deviceType);
    }

    ui->AssayNameTbl->setCurrentIndex(QModelIndex());
    ResizeTblColToContent(ui->AssayNameTbl);

    emit ui->AssayNameTbl->clicked(QModelIndex());
}

///
/// @brief 初始化校准量程表格
///
/// @par History:
/// @li 8090/YeHuaNing，2022年9月16日，新建函数
///
void ImQCaliBrateHistory::InitCaliDataModel()
{
    m_caliDataMode->clear();
    ui->CalRltTbl->setModel(m_caliDataMode);
    ui->CalRltTbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->CalRltTbl->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->CalRltTbl->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->CalRltTbl->verticalHeader()->setDefaultAlignment(Qt::AlignCenter);

	ui->CalRltTbl__fail->setModel(m_caliDataMode);
    ui->CalRltTbl__fail->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->CalRltTbl__fail->verticalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->CalRltTbl__fail->verticalHeader()->setDefaultAlignment(Qt::AlignCenter);

    // 重新设置页面
    m_caliDataMode->clear();
    QStringList horHeaderTitle;
    horHeaderTitle << tr("校准品1") << tr("校准品2") << tr("校准品3") << tr("校准品4") << tr("校准品5") << tr("校准品6");
    int index = 1;
    for (auto tile : horHeaderTitle)
    {
        m_mapColums[index++] = tile;
    }
    m_caliDataMode->setHorizontalHeaderLabels(horHeaderTitle);

    QStringList verHeaderTitle;
    verHeaderTitle << tr("信号值1") << tr("信号值2");
    if (UserInfoManager::GetInstance()->IsPermisson(PMS_CALI_HISTORY_SHOW_CONC))
    {
        verHeaderTitle << tr("浓度");
    }
    m_caliDataMode->setVerticalHeaderLabels(verHeaderTitle);

    auto font = ui->CalRltTbl->font();
    font.setPixelSize(16);
    ui->CalRltTbl->setFont(font);
    ui->CalRltTbl__fail->setFont(font);
}

///
/// @brief
///     设置显示曲线的详细信息
///
/// @param[in]  curveInfo  曲线信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月22日，新建函数
///
void ImQCaliBrateHistory::SetCurveDetail(const im::tf::CaliCurve& curveInfo)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    QStringList  infos;

    // 获取项目名称、单位、倍率
    if (curveInfo.__isset.assayCode)
    {
        // 项目名称
		const std::shared_ptr<tf::GeneralAssayInfo>& spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(curveInfo.assayCode);
        if (spAssay)
        {
            infos.push_back(std::move(QString::fromStdString(spAssay->assayName)));           
        }
        infos.push_back(std::move(CommonInformationManager::GetInstance()->GetCurrentUnit(curveInfo.assayCode)));
    }
    else
    {
        infos.push_back("");
        infos.push_back("");
    }

    // 试剂批号
    if (curveInfo.__isset.reagentLot)
    {
        infos.push_back(std::move(QString::fromStdString(curveInfo.reagentLot)));
    }
    else
    {
        infos.push_back("");
    }

    // 校准品批号
    if (curveInfo.__isset.calibratorLot)
    {
        infos.push_back(std::move(QString::fromStdString(curveInfo.calibratorLot)));
    }
    else
    {
        infos.push_back("");
    }

    // 校准瓶号
    if (curveInfo.__isset.reagentSN && !curveInfo.reagentSN.empty())
    {
        infos.push_back(std::move(QString::fromStdString(curveInfo.reagentSN)));
    }
    else
    {
        infos.push_back("");
    }

    // 校准时间
    if (curveInfo.__isset.caliExcuteDate)
    {
        infos.push_back(std::move(QString::fromStdString(curveInfo.caliExcuteDate)));
    }
    else
    {
        infos.push_back("");
    }

    // 校准失败信息
    if (curveInfo.failedFlag > 0)
    {
        ui->stackedWidget->setCurrentIndex(1);
        // 设置项目名称
        SetLableData(ui->AssayNameLab_fail, infos[0]);
        // 浓度单位
        SetLableData(ui->ConcUnitLab_fail, infos[1]);
        // 设置试剂批号
        SetLableData(ui->RgntLotLab_fail, infos[2]);
        // 设置校准品批号
        SetLableData(ui->CalLotLab_fail, infos[3]);
        // 设置校准瓶号
        SetLableData(ui->RgntBnLab_fail, infos[4]);
        // 设置校准时间
        SetLableData(ui->CalTimeLab_fail, infos[5]);
		QString failReason = "";
        // bug19161 不再报未知错误或用户难以理解的校准失败原因，原因除用户可控的CV和DELTA,K值外，分类为参数异常和拟合失败
        QString number = "";
		switch(curveInfo.failedFlag)
		{
        case im::tf::caliFailType::ERR_WRONG_DATA:
        case im::tf::caliFailType::ERR_UNKNOWN_IM_TYPE:
        case im::tf::caliFailType::ERR_SIZE_UNMATCH:
        case im::tf::caliFailType::ERR_OUT_RANGE:
        case im::tf::caliFailType::ERR_UNMATCH_IM_TYPE:
        case im::tf::caliFailType::ERR_PARAM:
            failReason = tr("参数异常。");
            break;
        case im::tf::caliFailType::ERR_WRONG_AFTER_CALI_RLU:
        case im::tf::caliFailType::ERR_WRONG_CON:
		case im::tf::caliFailType::ERR_NO_RLU_ANALYZER:
        case im::tf::caliFailType::ERR_FITTING_FAILURE:
            failReason = tr("拟合失败。");
            break;
		case im::tf::caliFailType::ERR_CV_OVERFLOW:           
            if (curveInfo.caliJudgeInfo.cv.size() > 0)
            {
                int index = (int)curveInfo.caliJudgeInfo.cv[0];
                number = QString::number(index);
            }
            failReason = tr("CV") + number + tr("越界。");
			break;
		case im::tf::caliFailType::ERR_DELTA_OVERFLOW:
            if (curveInfo.caliJudgeInfo.delta.size() > 0)
            {
                int index = (int)curveInfo.caliJudgeInfo.delta[0];
                number = QString::number(index);
            }
            failReason = tr("Δ") + number + tr("越界。");
			break;
		case im::tf::caliFailType::ERR_K_OVERFLOW:
			failReason = tr("K越界。");
			break;
		case im::tf::caliFailType::ERR_UNKNOWN_ERROR:
		default:
			failReason = tr("拟合失败。");
			break;
		}
        // 失败信息
        SetLableData(ui->FailedReason, failReason);

    }
    // 校准成功的信息
    else
    {
        ui->stackedWidget->setCurrentIndex(0);
        // 设置项目名称
        SetLableData(ui->AssayNameLab, infos[0]);
        // 浓度单位
        SetLableData(ui->ConcUnitLab, infos[1]);
        // 设置试剂批号
        SetLableData(ui->RgntLotLab, infos[2]);
        // 设置校准品批号
        SetLableData(ui->CalLotLab, infos[3]);
        // 设置校准瓶号
        SetLableData(ui->RgntBnLab, infos[4]);
        // 设置校准时间
        SetLableData(ui->CalTimeLab, ToCfgFmtDateTime(infos[5]));
    }
}

///
/// @brief 设置控件数据
///
/// @param[in]  lb  需要设置的控件
/// @param[in]  text  数据内容
///
/// @par History:
/// @li 8090/YeHuaNing，2022年9月17日，新建函数
///
void ImQCaliBrateHistory::SetLableData(QLabel * lb, const QString & text)
{
    if (lb == nullptr)
    {
        return;
    }

    int pos = lb->text().indexOf(tr("："));

    if (pos == -1)
    {
        return;
    }

    lb->setText((lb->text().left(pos + 1) + text));
}

///
/// @brief 设置吸光度浓度表
///
/// @param[in]  curveInfo  曲线信息
///
/// @par History:
/// @li 8090/YeHuaNing，2022年9月17日，新建函数
///
void ImQCaliBrateHistory::SetCaliDataModel(const im::tf::CaliCurve & curveInfo)
{
    InitCaliDataModel();

    bool isPermitConn = UserInfoManager::GetInstance()->IsPermisson(PMS_CALI_HISTORY_SHOW_CONC);

    // 汇总各水平的数据
    int col = 1;
    for (const auto& absorbance : curveInfo.actualRLU.caliDocRlu)
    {
        m_caliDataMode->SetItem(tr("信号值1"), m_mapColums[col], CenterAligmentItem(QString::number(absorbance.rlu1, 'f', 0)));
        m_caliDataMode->SetItem(tr("信号值2"), m_mapColums[col], CenterAligmentItem(QString::number(absorbance.rlu2, 'f', 0)));

        if (isPermitConn)
        {
            // 定性项目显示*
            QString connText = "*";
            if (curveInfo.caliType == 1)
            {
                connText = QString::number(CommonInformationManager::GetInstance()->ToCurrentUnitValue(curveInfo.assayCode, absorbance.conc), 'f', 2);
            }
            m_caliDataMode->SetItem(tr("浓度"), m_mapColums[col], CenterAligmentItem(connText));
        }
        // 变行应该在权限判定外bug24888
        ++col;
    }

    ui->CalRltTbl->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->CalRltTbl__fail->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
}

QString ImQCaliBrateHistory::SetCustomAxisInfo_X_Bottom(double value)
{
    // 默认成功的曲线都是瓶曲线
    QString curveMark = "B";

    // 1.如果校准失败了，curveMark为空
    // 2.如果索引大于曲线范围，也为空
    if (m_curCaliCurves.size() < (int)value || value <= 0.0 || 
        (m_curCaliCurves[(int)value - 1].__isset.failedFlag && m_curCaliCurves[(int)value - 1].failedFlag > 0))
    {
        curveMark = "";
    }
    // 判断曲线是不是批曲线
    else if (!m_curCaliCurves.empty() && m_curCaliCurves.size() >= (int)value)
    {
        if (m_curCaliCurves[(int)value - 1].__isset.lot && m_curCaliCurves[(int)value - 1].lot >= 1)
            curveMark = ("B/L");
    }

    // 不管curveMark是否为空，都增加一个换行，可以保持视图不变
    return QString::number(value) + "\n" + curveMark;
}

///
/// @bref
///		加载浓度表格数据
///
/// @par History:
/// @li 8276/huchunli, 2023年10月30日，新建函数
///
void ImQCaliBrateHistory::ReloadTableData()
{
    if (m_index >= 0 && m_index < m_curCaliCurves.size())
    {
        // 修改单位倍率
        double factor = 1.0;
        if (CommonInformationManager::GetInstance()->GetUnitFactor(m_curCaliCurves[m_index].assayCode, factor) && m_unitFactor != factor)
        {
            m_unitFactor = factor;
            OnShowQwtCurveTip(false, -1, -1);
        }
        SetCaliDataModel(m_curCaliCurves[m_index]);
        SetCurveDetail(m_curCaliCurves[m_index]);       
    }
    // 范围以外的，直接清空原有数据
    else
    {
        SetCaliDataModel(im::tf::CaliCurve());
        SetCurveDetail(im::tf::CaliCurve());
    }
}

///
/// @brief 显示当前图形选中索引的校准数据
///
/// @param[in]  index  图形上的索引
///
/// @par History:
/// @li 8090/YeHuaNing，2022年12月12日，新建函数
///
void ImQCaliBrateHistory::OnCurrentSelCalHisInfoChanged(int index)
{
    // 图形上的索引需要减1
    --index;

    if (index >= 0 && index < m_curCaliCurves.size())
    {
		m_index = index;
        SetCaliDataModel(m_curCaliCurves[index]);
        SetCurveDetail(m_curCaliCurves[index]);
    }
    // 范围以外的，直接清空原有数据
    else
    {
        SetCaliDataModel(im::tf::CaliCurve());
        SetCurveDetail(im::tf::CaliCurve());
    }

    CreateScaleDraw_X(++index);
}

///
/// @brief
///     清空曲线内容
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月22日，新建函数
///
void ImQCaliBrateHistory::ClearCurveContent()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    if (m_CurveMode == nullptr)
    {
        m_CurveMode = new QToolTipModel(this);
		// 升序 降序 原序
		SortHeaderView *pCurveModeHeader = new SortHeaderView(Qt::Horizontal, ui->AssayNameTbl);
		pCurveModeHeader->setStretchLastSection(true);
		ui->AssayNameTbl->setHorizontalHeader(pCurveModeHeader);
        connect(pCurveModeHeader, &SortHeaderView::SortOrderChanged, this, [this](int logicIndex, SortHeaderView::SortOrder order) {
            m_iSortOrder = order;
            m_iSortColIdx = logicIndex;
            QTableView *view = ui->AssayNameTbl;
            //无效index或NoOrder就设置为默认未排序状态
            if (logicIndex < 0 || order == SortHeaderView::NoOrder) {
                m_iSortColIdx = -1;
                SetAscSortByFirstColumn();
            }
            else
            {
                Qt::SortOrder qOrderFlag = order == SortHeaderView::DescOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
                int role = Qt::DisplayRole;
                m_CurveMode->setSortRole(role);
                view->sortByColumn(logicIndex, qOrderFlag);
                //m_iSortOrder = order;
                //m_iSortColIdx = logicIndex;
            }
        });
		//去掉排序三角样式
		ui->AssayNameTbl->horizontalHeader()->setSortIndicator(-1, Qt::DescendingOrder);
		//-1则还原model默认顺序
        m_CurveMode->sort(-1, Qt::DescendingOrder);

        ui->AssayNameTbl->setModel(m_CurveMode);
        ui->AssayNameTbl->setSelectionBehavior(QAbstractItemView::SelectRows);
        ui->AssayNameTbl->setSelectionMode(QAbstractItemView::SingleSelection);
        ui->AssayNameTbl->setEditTriggers(QAbstractItemView::NoEditTriggers);
        ui->AssayNameTbl->setSortingEnabled(true);
		ui->AssayNameTbl->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

        ResizeTblColToContent(ui->AssayNameTbl);
        //ui->AssayNameTbl->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    }

    // 重新设置页面
    m_CurveMode->clear();
    QStringList supplyHeaderListString;
    supplyHeaderListString << tr("项目名称");
    m_CurveMode->setRowCount(30);
    m_CurveMode->setHorizontalHeaderLabels(supplyHeaderListString);
}

///
/// @brief
///     更新校准曲线表格数据
///
/// @param[in]  curveInfos  校准曲线信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月22日，新建函数
///
bool ImQCaliBrateHistory::UpdateCurveView(tf::DeviceType::type assayInfos)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    ImAssayIndexUniqueCodeMaps codeMaps;
    CommonInformationManager::GetInstance()->GetImAssayMaps(assayInfos, codeMaps);
    if (codeMaps.size() <= 0)
    {
        return false;
    }

    // 依次显示校准曲线的历史的数据信息
    int seq = 0;
    for (const auto& ass : codeMaps)
    {
		if (ass.second == nullptr)
		{
			continue;
		}
        m_CurveMode->setItem(seq, 0, CenterAligmentItem(QString::fromStdString(ass.second->assayName)));
        m_CurveMode->setData(m_CurveMode->index(seq++, 0), ass.second->assayCode, Qt::UserRole + 1);
    }

    return true;
}

///
/// @brief
///     选中曲线--显示曲线
///
/// @param[in]  index  曲线Index
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月22日，新建函数
///
void ImQCaliBrateHistory::OnSelectCurveShow(QModelIndex index)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 清理数据
    m_curCaliCurves.clear();
    InitCaliDataModel();

    // 隐藏显示点
    if (m_pCaliSelRing)
        m_pCaliSelRing->detach();
    if (m_pCaliSelTipMarker)
        m_pCaliSelTipMarker->detach();
    
    if (!index.isValid() || index.row() < 0 || index.column() < 0)
    {
        SetCurveDetail(im::tf::CaliCurve());
        CreateQwtPlotCurveData(std::vector<im::tf::CaliCurve>{});
        return;
    }

    // 获取当前项目的项目配置参数
    const QString& str = m_CurveMode->itemData(index)[Qt::DisplayRole].toString();
    const auto& assay = CommonInformationManager::GetInstance()->GetAssayInfo(str.toStdString());

    // 判断初始条件是否满足
    if (str.isEmpty() || assay == nullptr)
    {
        return;
    }
    // 修改单位倍率
    double factor = 1.0;
    if (CommonInformationManager::GetInstance()->GetUnitFactor(assay->assayCode, factor))
    {
        m_unitFactor = factor;
    }

	// 设备列表为空则返回
	if (m_strDevs.empty())
	{
		ULOG(LOG_INFO, "deivce is error");
		return;
	}
    // 获取历史曲线
    ::im::tf::CaliCurveQueryResp _return;
    im::i6000::LogicControlProxy::GetHistoryCurveInfo(_return, m_strDevs[0], assay->assayCode);

    // 按时间倒序排列(bug22547)
    std::sort(_return.lstCurveConfigs.begin(), _return.lstCurveConfigs.end(), [](const im::tf::CaliCurve& first, const im::tf::CaliCurve& second) {
        return first.caliExcuteDate > second.caliExcuteDate;
    });

    // 准备曲线数据
    CreateQwtPlotCurveData(_return.lstCurveConfigs);
}

void ImQCaliBrateHistory::OnAssayUpdated()
{
	if (m_strDevs.empty() || m_CurveMode == nullptr)
	{
		return;
	}

    auto item = m_CurveMode->item(ui->AssayNameTbl->currentIndex().row());
    QString text;
	if (item != nullptr)
	{
		text = item->text();
	}
           
	// 清空表格内容	
	m_CurveMode->clear();
	QStringList supplyHeaderListString;
	supplyHeaderListString << tr("项目名称");
	m_CurveMode->setRowCount(30);
	m_CurveMode->setHorizontalHeaderLabels(supplyHeaderListString);

    const std::shared_ptr<const tf::DeviceInfo>& dv = CommonInformationManager::GetInstance()->GetDeviceInfo(m_strDevs[0]);
    if (!UpdateCurveView(dv->deviceType))
    {
        return;
    }

    // 确定是否存在已选择的项目
    auto && items = m_CurveMode->findItems(text);
    if (!items.isEmpty())
    {
        const auto& index = items[0]->index();
        ui->AssayNameTbl->setCurrentIndex(index);
        OnSelectCurveShow(index);
    }
	else
	{
		ui->AssayNameTbl->setCurrentIndex(QModelIndex());
        OnSelectCurveShow(QModelIndex());
	}
    ResizeTblColToContent(ui->AssayNameTbl);
}

///
/// @brief
///     显示前初始化
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月4日，新建函数
///
void ImQCaliBrateHistory::InitBeforeShow()
{
    // 项目名称表
    ClearCurveContent();

    // 校准结果表
    InitCaliDataModel();

    // 初始化信号槽连接
    InitConnect();

    // 初始化子控件
    InitChildCtrl();
}

///
/// @brief
///     显示之后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月4日，新建函数
///
void ImQCaliBrateHistory::InitAfterShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
}

///
/// @brief
///     重写显示事件
///
/// @param[in]  event  事件
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月4日，新建函数
///
void ImQCaliBrateHistory::showEvent(QShowEvent* event)
{
    // 基类处理
    QWidget::showEvent(event);
    POST_MESSAGE(MSG_ID_CURRENT_MODEL_NAME, QString("> ") + tr("校准") + QString(" > ") + tr("校准历史"));

    // 如果是第一次显示则初始化
    if (!m_bInit)
    {
        // 显示后初始化
        InitAfterShow();
    }

    ReloadTableData();
}

///
/// @brief
///     初始化子控件
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月17日，新建函数
///
void ImQCaliBrateHistory::InitChildCtrl()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    if (m_pScrollBar == nullptr || m_pCalHisCurve == nullptr)
    {
        QVBoxLayout* pHLayout = new QVBoxLayout();
        m_pScrollBar = new QScrollBar();
        m_pScrollBar->setRange(0, 25);
        m_pScrollBar->setSingleStep(1);
        m_pScrollBar->setOrientation(Qt::Horizontal);
        QObject::connect(m_pScrollBar, SIGNAL(valueChanged(int)), m_pCalHisCurve, SLOT(TestRangeChanged(int)));
        pHLayout->setSpacing(10);
        pHLayout->setMargin(0);
        pHLayout->addWidget(m_pCalHisCurve);
        pHLayout->addWidget(m_pScrollBar);
        ui->CalHisCurveWgt->setLayout(pHLayout);
    }
    CreateScaleDraw_X();
    m_pCalHisCurve->SetAxisTitle(QwtPlot::yLeft, "\n" + tr("校准品1") + "\n");
    m_pCalHisCurve->SetAxisColor(QwtPlot::yLeft, GetLegendAttrs()[0].legendColor);
    m_pCalHisCurve->SetAxisTitle(QwtPlot::yRight, "\n" + tr("校准品2") + "\n");
    m_pCalHisCurve->SetAxisColor(QwtPlot::yRight, GetLegendAttrs()[1].legendColor);
    m_pCalHisCurve->SetYAxisEnable();
    m_pCalHisCurve->GetQwtPlot()->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Backbone, false);
    m_pCalHisCurve->GetQwtPlot()->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Ticks, false);
   // m_pCalHisCurve->GetQwtPlot()->axisScaleDraw(QwtPlot::yLeft)->enableComponent(QwtAbstractScaleDraw::Labels, false);
    m_pCalHisCurve->GetQwtPlot()->axisScaleDraw(QwtPlot::yRight)->enableComponent(QwtAbstractScaleDraw::Backbone, false);
    m_pCalHisCurve->GetQwtPlot()->axisScaleDraw(QwtPlot::yRight)->enableComponent(QwtAbstractScaleDraw::Ticks, false);
    //m_pCalHisCurve->GetQwtPlot()->axisScaleDraw(QwtPlot::yRight)->enableComponent(QwtAbstractScaleDraw::Labels, false);

    // 选中效果圆环
    m_pCaliSelRing = new QwtPlotCurve();
    QwtSymbol* pQcSelSymbol = new QwtSymbol(QwtSymbol::Pixmap);
    pQcSelSymbol->setPixmap(QPixmap(":/Leonis/resource/image/img-qc-graphic-sel-ring.png"));
    m_pCaliSelRing->setSymbol(pQcSelSymbol);

    // 选中标记
    m_pCaliSelTipMarker = new QcGraphicSelTipMaker();
}

///
/// @brief
///     初始化连接
///
/// @par History:
/// @li 4170/TangChuXian，2022年9月17日，新建函数
///
void ImQCaliBrateHistory::InitConnect()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 选择每一行的槽函数
    connect(ui->AssayNameTbl, SIGNAL(clicked(QModelIndex)), this, SLOT(OnSelectCurveShow(QModelIndex)));
    connect(ui->PrintBtn, SIGNAL(clicked()), this, SLOT(OnPrintBtnClicked()));
    connect(ui->export_btn, SIGNAL(clicked()), this, SLOT(OnExportBtnClicked()));
    connect(m_pCalHisCurve, &CalHisCurve::SigCurrentSelCalHisInfoChanged, this, &ImQCaliBrateHistory::OnCurrentSelCalHisInfoChanged);
    connect(m_pCalHisCurve, &CalHisCurve::SigOnSelectPoint, this, &ImQCaliBrateHistory::OnSelectPos);
    REGISTER_HANDLER(MSG_ID_ASSAY_CODE_MANAGER_UPDATE, this, OnAssayUpdated);
    // 监听校准完成
    REGISTER_HANDLER(MSG_ID_IM_CALI_END, this, OnCaliEnd);
    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);
    // 监听检测模式改变
    REGISTER_HANDLER(MSG_ID_DETECT_MODE_UPDATE, this, OnDetectModeChanged);
}

///
/// @brief 创建QwtCurve显示数据
///
/// @param[in]  caliCurves  校准数据
///
/// @par History:
/// @li 8090/YeHuaNing，2022年12月12日，新建函数
///
void ImQCaliBrateHistory::CreateQwtPlotCurveData(const std::vector<im::tf::CaliCurve>& caliCurves)
{
    static std::map<int, QVector<QPointF>>  caliHis;  //各水平的记录
    int hisCount = 0;   // 历史记录条数

    // 清理原有数据
    for (auto& his : caliHis)
    {
        his.second.clear();
    }

    // 汇总各水平的数据
	double min = DBL_MAX;
	double max = std::numeric_limits<double>::lowest();
	bool bSet = false;
    for (const auto& curve : caliCurves)
    {
        for (size_t i = 0; i < curve.actualRLU.caliDocRlu.size(); i++)
        {
            QPointF pointF;
			double pointY = curve.actualRLU.caliDocRlu[i].meanRlu;
			pointF.setX(hisCount + 1);
			pointF.setY(pointY);
			if (pointY > max)
			{
				max = pointY;			
			}
			if (pointY < min)
			{
				min = pointY;
			}
            caliHis[i+1].push_back(pointF);
			bSet = true;
        }
        ++hisCount;
        m_curCaliCurves.emplace_back(curve);
    }
	// 如果没有值则设可用范围的正常值
	if (!bSet)
	{
		min = 0;
		max = 1000;
	}

    auto& legendAttrs = GetLegendAttrs();

    if (!legendAttrs.empty()) {
        for (size_t i = 0; i < legendAttrs.size(); ++i)
        {
            // 如果校准水平不足六个，缺省的部分使用初始值填充
            if (i >= caliHis.size() && legendAttrs[i].curve != nullptr)
                legendAttrs[i].curve->setSamples(QVector<QPointF>{});
            else
                legendAttrs[i].curve->setSamples(caliHis[i+1]);

            legendAttrs[i].curve->attach(m_pCalHisCurve->GetQwtPlot());
        }

        m_pCalHisCurve->SetHisCount(hisCount);
        m_pScrollBar->setValue(0);
        m_pCalHisCurve->TestRangeChanged(0);
		m_pCalHisCurve->SetYRange(min, max);

		if (m_curCaliCurves.size() > 0)
		{
			m_pCalHisCurve->SetSelIndex(1);
		}
    }

    // 默认选择第一个，高亮文字
    if (!caliCurves.empty())
        CreateScaleDraw_X(1);
}

///
/// @brief 创建线条基础数据
///
/// @param[in]  xAxis  x坐标轴位置
/// @param[in]  yAxis  y坐标轴位置
/// @param[in]  brushColor  线条颜色
///
/// @return 曲线对象
///
/// @par History:
/// @li 8090/YeHuaNing，2022年12月12日，新建函数
///
QwtPlotCurve * ImQCaliBrateHistory::CreateInitQwtPlotCurve(int xAxis, int yAxis, const QColor & brushColor)
{
    QwtPlotCurve* pCaliHisCurve = new QwtPlotCurve();

    pCaliHisCurve->setZ(30);

    // 设置坐标轴
    pCaliHisCurve->setAxes(xAxis, yAxis);

    // 设置曲线点符号
    QwtSymbol* pCircleSymbol = new QwtSymbol(QwtSymbol::Ellipse);
    pCircleSymbol->setBrush(QBrush(brushColor));
    pCircleSymbol->setPen(QPen(Qt::NoPen));
    pCircleSymbol->setSize(10);
    pCaliHisCurve->setSymbol(pCircleSymbol);

    // 设置曲线样式为点
    pCaliHisCurve->setStyle(QwtPlotCurve::NoCurve);

    return pCaliHisCurve;
}

///
/// @brief 获取图例数据
///
///
/// @return 图例对象集合
///
/// @par History:
/// @li 8090/YeHuaNing，2022年12月12日，新建函数
///
QVector<ImQCaliBrateHistory::QLegendAttr>& ImQCaliBrateHistory::GetLegendAttrs()
{
    static std::once_flag onceFlag;
    // 每个校准品水平的图例颜色以及数据点颜色
    static QMap<int, QColor> legendColor{
        { 1, QColor(31, 181, 88) },
        { 2, QColor(69, 159, 255) }
    };

    static QVector<ImQCaliBrateHistory::QLegendAttr> legendAttrs(legendColor.size());

    // 初始化每个校准水平 点的属性
    std::call_once(onceFlag, [&] {
        // 创建显示的曲线（一次创建）
        for (size_t i = 1; i <= legendColor.size(); i++)
        {
            auto& legend = legendAttrs[i - 1];
            legend.curve = CreateInitQwtPlotCurve(QwtPlot::xBottom, i % 2 ? QwtPlot::yLeft : QwtPlot::yRight, legendColor[i]);
            legend.legendTitle = tr("校准品") + QString::number(i);
            legend.legendColor = legendColor[i];
        }
    });
 
    return legendAttrs;
}

void ImQCaliBrateHistory::CreateScaleDraw_X(int selIndex)
{
    QCustomScaleDraw* pScaleDrawX = new QCustomScaleDraw(std::bind(&ImQCaliBrateHistory::SetCustomAxisInfo_X_Bottom, this, std::placeholders::_1));
    if (pScaleDrawX)
    {
        pScaleDrawX->enableComponent(QwtAbstractScaleDraw::Backbone, false);
        pScaleDrawX->enableComponent(QwtAbstractScaleDraw::Ticks, false);
        pScaleDrawX->SetSelLabelColor(selIndex, QColor(0xfa, 0x0, 0x0));
        m_pCalHisCurve->GetQwtPlot()->setAxisScaleDraw(QwtPlot::xBottom, pScaleDrawX);
    }
}

void ImQCaliBrateHistory::OnShowQwtCurveTip(bool isShow, int level, int curveIndex, const QPoint& pos)
{
    // 隐藏显示点
    m_pCaliSelRing->detach();
    m_pCaliSelTipMarker->detach();

    // 需要显示高亮
    if (isShow && (curveIndex >= 0 && curveIndex < m_curCaliCurves.size()))
    {
        auto& curve = m_curCaliCurves[curveIndex];

        for (size_t i = 0; i < curve.actualRLU.caliDocRlu.size(); i++)
        {
            // 水平是否相等
            if (i+1 != level)
                continue;
            auto plot = m_pCalHisCurve->GetQwtPlot();
            // 显示高亮圆环
            QPointF pointF;
            double pointY = curve.actualRLU.caliDocRlu[i].meanRlu;
            pointF.setX(curveIndex + 1);
            pointF.setY(pointY);
            m_pCaliSelRing->setSamples({ pointF });
            m_pCaliSelRing->attach(plot);

            // 显示提示框
            // 构造qwtText
            m_strSelPtTextList.clear();
            auto && dateTime = QDateTime::fromString(QString::fromStdString(curve.caliExcuteDate), Qt::ISODate);
            m_strSelPtTextList << QString::number(curve.actualRLU.caliDocRlu[i].conc*m_unitFactor, 'f', 2) << dateTime.time().toString(Qt::TextDate) << dateTime.date().toString(Qt::LocalDate);
            m_pCaliSelTipMarker->SetQcTipContent(m_strSelPtTextList);

            // 根据坐标判断标签位置
            int iAlign = 0;
            if (pos.y() < 120)
            {
                iAlign |= Qt::AlignBottom;
            }
            else
            {
                iAlign |= Qt::AlignTop;
            }

            if (pos.x() < 50)
            {
                iAlign |= Qt::AlignRight;
            }
            else if (plot->canvas()->width() - pos.x() < 50)
            {
                iAlign |= Qt::AlignLeft;
            }
            else
            {
                iAlign |= Qt::AlignHCenter;
            }

            // 构造marker
            m_pCaliSelTipMarker->setValue(pointF);
            m_pCaliSelTipMarker->setLabelAlignment(Qt::Alignment(iAlign), Qt::AlignCenter);
            m_pCaliSelTipMarker->attach(plot);
        }
    }
}

void ImQCaliBrateHistory::SetAscSortByFirstColumn()
{
    m_CurveMode->setSortRole(Qt::UserRole + 1);
    ui->AssayNameTbl->sortByColumn(0, Qt::AscendingOrder);
    ui->AssayNameTbl->horizontalHeader()->setSortIndicator(-1, Qt::DescendingOrder);
}

void ImQCaliBrateHistory::OnSelectPos(const QPoint & pos)
{
    static;
    QVector<QLegendAttr>& legendArr =  GetLegendAttrs();
    int level = 1;
    int minlevel = -1;
    double mind = 9;
    for (const auto& le : legendArr)
    {
        double d;
        if (le.curve)
        {
            // idx 是当前曲线下标索引对应的坐标轴的值
            // 如果曲线中间出现间隔例如(x轴 7后面是10，点击10时，返回的是7）
            int idx = le.curve->closestPoint(pos, &d);
            if (d < mind)
            {
                mind = d;
                minlevel = level;
            }
        }
        ++level;
    }

    if (mind < 9 && minlevel != -1)
    {
        // 显示标记的点和弹框显示记录
        OnShowQwtCurveTip(true, minlevel, m_index, pos);
        return;
    }
    else
    {
        OnShowQwtCurveTip(false, -1, -1);
    }
}

///
/// @brief  响应校准完成
///
/// @param[in]  cv  曲线信息
///
/// @par History:
/// @li 1226/zhangjing，2023年8月28日，新建函数
///
void ImQCaliBrateHistory::OnCaliEnd(const im::tf::CaliCurve cv)
{
    // 获取当前项目的项目配置参数
    auto selIndex = ui->AssayNameTbl->currentIndex();
    //const auto& toSourceIndex = m_CurveModeProxyModel->mapToSource(selIndex);
	auto item = m_CurveMode->item(selIndex.row(), 0);
	if (item == nullptr)
	{
		ULOG(LOG_WARN, " m_CurveMode->item(selIndex.row(), 0) is null.");
		return;
	}
    auto assayCode = item->data(Qt::UserRole + 1).toInt();
    // 不是同项目不用更新
    if (assayCode <= 0 ||assayCode != cv.assayCode)
    {
        return;
    }
    OnSelectCurveShow(selIndex);
}

///
/// @bref
///		权限变化响应
///
/// @par History:
/// @li 8276/huchunli, 2023年10月30日，新建函数
///
void ImQCaliBrateHistory::OnPermisionChanged()
{
    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    std::shared_ptr<UserInfoManager> userPms = UserInfoManager::GetInstance();
    userPms->IsPermisson(PSM_IM_PRINTEXPORT_CALIHISTORY) ? ui->export_btn->show() : ui->export_btn->hide();

    ReloadTableData();
}

///
/// @brief
///     检测模式更新处理
///
/// @param[in]  mapUpdateInfo  检测模式更新信息（设备序列号-检测模式）
///
/// @par History:
/// @li 1226/zhangjing，2023年11月8日，新建函数
///
void ImQCaliBrateHistory::OnDetectModeChanged(QMap<QString, im::tf::DetectMode::type> mapUpdateInfo)
{
    // 获取当前项目的项目配置参数
    auto selIndex = ui->AssayNameTbl->currentIndex();
    auto item = m_CurveMode->item(selIndex.row(), 0);
    if (item == nullptr)
    {
        ULOG(LOG_WARN, " m_CurveMode->item(selIndex.row(), 0) is null.");
        return;
    }
    OnSelectCurveShow(selIndex);
}

///
/// @brief 响应打印按钮
///
/// @par History:
/// @li 6889/ChenWei，2023年12月14日，新建函数
///
void ImQCaliBrateHistory::OnPrintBtnClicked()
{
    CaliHistoryInfo Info;
    QString strPrintTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    Info.strInstrumentModel = "i 6000";
    Info.strPrintTime = strPrintTime.toStdString();
    if (!GetPrintExportInfo(Info))
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("没有可打印的数据!")));
        pTipDlg->exec();
        return;
    }

    std::string strInfo = GetJsonString(Info);
    ULOG(LOG_INFO, "Print datas : %s", strInfo);
    std::string unique_id;
    int irect = printcom::printcom_async_print(strInfo, unique_id);
    ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);

    // 弹框提示打印结果
    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("数据已发送到打印机!")));
    pTipDlg->exec();
}

///
/// @brief 响应打印按钮
///
/// @par History:
/// @li 6889/ChenWei，2023年12月14日，新建函数
///
void ImQCaliBrateHistory::OnExportBtnClicked()
{
    CaliHistoryInfo Info;
    if (!GetPrintExportInfo(Info, false))
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("没有可导出的数据!")));
        pTipDlg->exec();
        return;
    }

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
    if (strSuffix == "pdf")
    {
        QString strPrintTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
        Info.strPrintTime = strPrintTime.toStdString();
        std::string strInfo = GetJsonString(Info);
        ULOG(LOG_INFO, "Print datas : %s", strInfo);
        std::string unique_id;
        QString strDirPath = QCoreApplication::applicationDirPath();
        QString strUserTmplatePath = strDirPath + "/resource/exportTemplate/ExportCaliHistoryInfo.lrxml";
        int irect = printcom::printcom_async_assign_export(strInfo, strFilepath.toStdString(), strUserTmplatePath.toStdString(), unique_id);
        ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);
        bRect = true;
    }
    else
    {
        std::shared_ptr<FileExporter> pFileEpt(new FileExporter());
        bRect = pFileEpt->ExportCaliHistoryInfo(Info, strFilepath);
    }

    // 弹框提示导出完成
    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(bRect ? tr("导出完成!") : tr("导出失败！")));
    pTipDlg->exec();
}

bool ImQCaliBrateHistory::GetPrintExportInfo(CaliHistoryInfo& info, bool isPrint)
{
    // 获取当前项目的项目配置参数
    im::tf::CaliCurve CurveInfo;
    if (m_index >= 0 && m_index < m_curCaliCurves.size())
    {
        CurveInfo = m_curCaliCurves[m_index];
    }
    else
    {
        ULOG(LOG_INFO, "Print Data: Can't find CurveInfo.");
        return false;
    }

    const std::shared_ptr<tf::GeneralAssayInfo>& spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(CurveInfo.assayCode);
    if (isPrint)
    {
        info.strName = spAssay->printName;           ///< 项目名称
    }
    else
    {
        info.strName = spAssay->assayName;           ///< 项目名称
    }
    
    info.strCalibrateDate = CurveInfo.caliExcuteDate;           ///< 校准时间
    info.strModelName = CommonInformationManager::GetInstance()->GetDeviceName(CurveInfo.deviceSN);               ///< 模块
    info.strCalibratorLot = CurveInfo.calibratorLot;           ///< 校准品批号
    info.strReagentLot = CurveInfo.reagentLot;              ///< 试剂批号
    info.strReagentSN = CurveInfo.reagentSN;               ///< 试剂瓶号
    info.strRegisterDate = CurveInfo.supplyInfos.reagentRegisterTime;            ///< 试剂上机时间
    info.strSubstrateLot = CurveInfo.supplyInfos.substrateABatchNo;            ///< 底物液批号
    info.strSubstrateSN = CurveInfo.supplyInfos.substrateASerialNo;             ///< 底物液瓶号
    info.strCleanFluidLot = CurveInfo.supplyInfos.cleanFluidBatchNo;           ///< 清洗缓冲液批号
    info.strCleanFluidSN = CurveInfo.supplyInfos.cleanFluidSerialNo;            ///< 清洗缓冲液瓶号
    info.strCupLot = CurveInfo.supplyInfos.assayCupBatchNo;                  ///< 反应杯批号
    info.strCupSN = CurveInfo.supplyInfos.assayCupSerialNo;                   ///< 反应杯序列号

    // 校准品1
    if (CurveInfo.actualRLU.caliDocRlu.size() > 0)
    {
        const auto& rlu = CurveInfo.actualRLU.caliDocRlu[0];
        QString strInfo = QString::number(CommonInformationManager::GetInstance()->ToCurrentUnitValue(CurveInfo.assayCode, rlu.conc), 'f', 2);
        info.strConc1 = strInfo.toStdString();;                   // Cal-1浓度
        strInfo = QString::number(rlu.rlu1, 'f', 0);
        info.strC1_RUL1 = strInfo.toStdString();                 // Cal-1 RLU1
        strInfo = QString::number(rlu.rlu2, 'f', 0);
        info.strC1_RUL2 = strInfo.toStdString();                 // Cal-1 RLU2
        strInfo = QString::number(rlu.meanRlu, 'f', 0);
        info.strC1_RUL = strInfo.toStdString();                  // Cal-1 RLU
        if (!IsEqual(rlu.masterRlu, 0.0))                        // Cal-1主标信号值
        {
            strInfo = QString::number(rlu.masterRlu, 'f', 0);
            info.strSignalValue1 = strInfo.toStdString();
        }       
    }

    // 校准品2
    if (CurveInfo.actualRLU.caliDocRlu.size() > 1)
    {
        const auto& rlu = CurveInfo.actualRLU.caliDocRlu[1];
        QString strInfo = QString::number(CommonInformationManager::GetInstance()->ToCurrentUnitValue(CurveInfo.assayCode, rlu.conc), 'f', 2);
        info.strConc2 = strInfo.toStdString();;                   // Cal-2浓度
        strInfo = QString::number(rlu.rlu1, 'f', 0);
        info.strC2_RUL1 = strInfo.toStdString();                 // Cal-2 RLU1
        strInfo = QString::number(rlu.rlu2, 'f', 0);
        info.strC2_RUL2 = strInfo.toStdString();                 // Cal-2 RLU2
        strInfo = QString::number(rlu.meanRlu, 'f', 0);
        info.strC2_RUL = strInfo.toStdString();                  // Cal-2 RLU
        if (!IsEqual(rlu.masterRlu, 0.0))                        // Cal-2主标信号值
        {
            strInfo = QString::number(rlu.masterRlu, 'f', 0);
            info.strSignalValue2 = strInfo.toStdString();
        }
    }

    // 需做失败状态判断，失败才需要输出原因
    if (CurveInfo.failedFlag != 0)
    {
        QString failReason;
        switch (CurveInfo.failedFlag)
        {
        case im::tf::caliFailType::ERR_WRONG_DATA:
        case im::tf::caliFailType::ERR_UNKNOWN_IM_TYPE:
        case im::tf::caliFailType::ERR_SIZE_UNMATCH:
        case im::tf::caliFailType::ERR_OUT_RANGE:
        case im::tf::caliFailType::ERR_UNMATCH_IM_TYPE:
        case im::tf::caliFailType::ERR_PARAM:
            failReason = tr("参数异常。");
            break;
        case im::tf::caliFailType::ERR_WRONG_AFTER_CALI_RLU:
        case im::tf::caliFailType::ERR_WRONG_CON:
        case im::tf::caliFailType::ERR_NO_RLU_ANALYZER:
        case im::tf::caliFailType::ERR_FITTING_FAILURE:
            failReason = tr("拟合失败。");
            break;
        case im::tf::caliFailType::ERR_CV_OVERFLOW:
            failReason = tr("CV越界。");
            break;
        case im::tf::caliFailType::ERR_DELTA_OVERFLOW:
            failReason = tr("△越界。");
            break;
        case im::tf::caliFailType::ERR_K_OVERFLOW:
            failReason = tr("K越界。");
            break;
        case im::tf::caliFailType::ERR_UNKNOWN_ERROR:
        default:
            failReason = tr("拟合失败。");
            break;
        }

        info.strFailureReason = tr("校准失败原因:").toStdString() + failReason.toStdString();            // 失败原因
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////
/// @file     ImQCaliBrateHistory.h
/// @brief 	 排序代理
///
/// @author   7656/zhang.changjiang
/// @date      2023年4月12日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年4月12日，新建文件
///
///////////////////////////////////////////////////////////////////////////
QImCurveModeFilterDataModule::QImCurveModeFilterDataModule(QObject * parent)
	:QSortFilterProxyModel(parent)
{
}

QImCurveModeFilterDataModule::~QImCurveModeFilterDataModule()
{
}

bool QImCurveModeFilterDataModule::lessThan(const QModelIndex & source_left, const QModelIndex & source_right) const
{
	// 获取源model
	QStandardItemModel* pSouceModel = dynamic_cast<QStandardItemModel*>(sourceModel());
	if (pSouceModel == nullptr)
	{
		return false;
	}

	QString leftData = pSouceModel->data(source_left).toString();
	QString rightData = pSouceModel->data(source_right).toString();
	return QString::localeAwareCompare(leftData, rightData) < 0;
}

