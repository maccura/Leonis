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
/// @file     QCaliBrateResult.cpp
/// @brief    校准结果
///
/// @author   5774/WuHongTao
/// @date     2022年7月18日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月18日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QCaliBrateResult.h"
#include "ui_QCaliBrateResult.h"
#include "CaliBrateCommom.h"
#include "src/common/common.h"
#include "src/common/Mlog/mlog.h"
#include "src/public/DictionaryKeyName.h"

#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/tipdlg.h"
#include "shared/ReagentCommon.h"
#include "shared/FileExporter.h"
#include "shared/datetimefmttool.h"
#include "shared/CommonInformationManager.h"
#include "Serialize.h"
#include "printcom.hpp"

#include "thrift/ch/c1005/C1005LogicControlProxy.h"
#include "manager/UserInfoManager.h"
#include "manager/DictionaryQueryManager.h"
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QFileDialog>
#include <QDateTime>
#include <QStandardItemModel>

#define MAXHISTORY 50

QCaliBrateResult::QCaliBrateResult(QWidget *parent)
	: BaseDlg(parent),
    m_calibrate(nullptr),
    m_curCurve(nullptr),
	m_parameter(nullptr),
	m_currentIndex(-1)
{
    ui = new Ui::QCaliBrateResult();
	ui->setupUi(this);
	SetTitleName(tr("校准结果"));
	Init();

    // 设置鼠标点击转发
    ui->widget->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->text_history_curve_label->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->text_choose_curve_label->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->time_lable_his->setAttribute(Qt::WA_TransparentForMouseEvents);
    ui->lot_lable_his->setAttribute(Qt::WA_TransparentForMouseEvents);
}

QCaliBrateResult::~QCaliBrateResult()
{
}

///
/// @brief 显示当前曲线
///
/// @param[in]  reagent		试剂信息
/// @param[in]  curveStore  曲线的绘制信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月18日，新建函数
///
std::shared_ptr<ch::tf::CaliCurve> QCaliBrateResult::SetCurrentCurve(ch::tf::ReagentGroup& reagent, std::vector<curveSingleDataStore>& curveStore)
{
    auto spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(reagent.assayCode);
    if (spAssay == nullptr)
        return nullptr;

	// 当前曲线参数
	curveStore.clear();
	curveStore.resize(2);
	curveStore[0].colorType = TYPEEND;
	curveStore[0].scatterMarkerShape = CIRCLE;
	curveStore[1].colorType = TYPEEND;
	curveStore[1].scatterMarkerShape = CALIBRATION;

	// 获取当前使用曲线
	auto currentCurve = GetCurrentCurve(reagent);
	if (!currentCurve)
	{
		ULOG(LOG_ERROR, "Can't find the current calibrate curve %s()", __FUNCTION__);
        SetPrintAndExportEnabled(false);
		return nullptr;
	}
    else
    {
        SetPrintAndExportEnabled(true);
    }

	// 根据曲线参数获取曲线
	shared_ptr<ch::ICalibrateCurve> currentCurveParamter = CaliBrateCommom::GetCurve(*currentCurve);
	if (currentCurveParamter == nullptr)
	{
		ULOG(LOG_ERROR, "creat the caliCurve failed %s()", __FUNCTION__);
		return nullptr;
	}

	// 显示当前曲线
	ShowCurve(currentCurve, currentCurveParamter, curveStore);

	auto status = GetCaliStatusShow(reagent, currentCurve);
	if (status.has_value())
	{
		int i = 0;
		for (const auto& lable : m_labelMap)
		{
			(status.value().size() > i) ? lable.second->setText(status.value()[i++]) : (void(0));
		}
	}

	// 设置工作曲线的状态信息
	{
		// 设置试剂批号
		ui->lot_lable->setText(QString::fromStdString(currentCurve->reagentLot));
		// 设置校准时间
		ui->time_lable->setText(ToCfgFmtDateTime(QString::fromStdString(currentCurve->caliTime)));
	}

	// 校准参数
	int row = 0;
	// 表格竖直方向的标题
	for (const auto& parameterData : currentCurveParamter->GetModelParameters())
	{
        m_parameter->setItem(row, 0, CenterAligmentItem(QString::fromStdString(parameterData.first)));
		m_parameter->setItem(row++, 1, CenterAligmentItem(QString::number(parameterData.second)));
	}

	// 校准数据
	row = 0;
    const auto& info = currentCurveParamter->GetAlgorithmConfigs();
    auto points = info.origPoints;
    if (info.cali_mode == ch::CALI_MODE::CALI_MODE_BLANK &&
        info.spanedPoints.size() > 0 && points.size() > 0)
    {
        points[0] = info.spanedPoints[0];
    }

    for (auto curveAbsorb : points)
	{
        int column = 0;
        //校准品
        m_calibrate->setItem(row, column++, CenterAligmentItem(tr("校准品%1").arg(row+1)));
        //吸光度
        m_calibrate->setItem(row, column++, CenterAligmentItem(QString::number(std::get<1>(curveAbsorb))));
		//浓度
        auto curFacValue = CommonInformationManager::GetInstance()->ToCurrentUnitValue(reagent.assayCode, std::get<0>(curveAbsorb));
		m_calibrate->setItem(row++, column++, CenterAligmentItem(QString::number(curFacValue, 'f', spAssay->decimalPlace)));
	}

    // 补齐到六个水平(Mantis12273缺陷建议按照实际数量显示，此处不再补齐)
    /*while (row < 6)
    {
        //吸光度
        m_calibrate->setItem(row,0, CenterAligmentItem(tr("校准品%1").arg(row+1)));
        ++row;
    }*/

	return currentCurve;
}

void QCaliBrateResult::SetTableViewFontSize(QTableView* table, int fontSize /*= 14*/)
{
    if (table == nullptr || table->model() == nullptr)
        return;

    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(table->model());
    int row = model->rowCount();
    int column = model->columnCount();
    QFont cosFont = font();
    cosFont.setPixelSize(fontSize);
    cosFont.setFamily("HarmonyOS Sans SC");

    auto func = [&fontSize, &cosFont](QStandardItem* item)
    {
        if (item == nullptr)
            return;

        item->setData(cosFont, Qt::FontRole);
    };

    /*for (int i = 0; i < row; ++i)
    {
        func(model->verticalHeaderItem(i));
    }
    for (int j = 0; j < column; ++j)
    {
        func(model->horizontalHeaderItem(j));
    }*/
    for (int i = 0; i < row; ++i)
    {
        for (int j = 0; j < column; ++j)
        {
            func(model->item(i,j));
        }
    }
}

///
/// @brief 历史曲线的选择槽函数
///
/// @param[in]    index 历史曲线index
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月18日，新建函数
///
void QCaliBrateResult::OnHistoryCurve(int index)
{
	m_currentIndex = index;
	if (index > m_historyCurve.size())
	{
		return;
	}

    // 如果选择了历史曲线，则可以点击更新按钮
    ui->update_btn->setEnabled(true);

	for (const auto& curveStore : m_historyCurvesStore)
	{
		ui->curve_widget->removeCurve(curveStore.curveId);
	}

	// 当前曲线参数
	m_historyCurvesStore.clear();
	m_historyCurvesStore.resize(2);
	m_historyCurvesStore[0].colorType = TEN;
	m_historyCurvesStore[0].scatterMarkerShape = CIRCLE;
	m_historyCurvesStore[0].curveId = -1;
	m_historyCurvesStore[1].colorType = TEN;
	m_historyCurvesStore[1].scatterMarkerShape = CALIBRATION;
	m_historyCurvesStore[1].curveId = -1;

	// 根据曲线参数获取曲线
	shared_ptr<ch::ICalibrateCurve> currentCurveParamter = CaliBrateCommom::GetCurve(*m_historyCurve[index]);
	if (currentCurveParamter == nullptr)
	{
		ULOG(LOG_ERROR, "creat the caliCurve failed %s()", __FUNCTION__);
		return;
	}

	// 显示当前曲线
	ShowCurve(m_historyCurve[index], currentCurveParamter, m_historyCurvesStore);

    if (m_historyCurve[index])
    {
        ui->lot_lable_his->setText(QString::fromStdString(m_historyCurve[index]->reagentLot));
        ui->time_lable_his->setText(ToCfgFmtDateTime(QString::fromStdString(m_historyCurve[index]->caliTime)));
        /*ui->history_btn->setStyleSheet("#history_btn{background: url(:/Leonis/resource/image/ch/c1005/ch-history-on-btn.png) no-repeat;"
            "border: 0;}");*/
        ui->widget->setVisible(true);
        ui->text_choose_curve_label->setVisible(false);
        ui->text_history_curve_label->setVisible(true);
    }
}

///
/// @brief 更新工作曲线
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月18日，新建函数
///
void QCaliBrateResult::OnUpdateWorkCurve()
{
	// 判断是否选中有历史曲线
	if (m_currentIndex < 0 || m_currentIndex >= m_historyCurve.size())
	{
		return;
	}

    const auto& curve = m_historyCurve[m_currentIndex];

    // 校准参数变更后，不能更新此历史曲线为工作曲线
    if (curve->caliParasChanged)
    {
        TipDlg(tr("不可更新为工作曲线！")).exec();
        return;
    }

	std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("更新工作曲线会影响测定结果，是否确认更新?"), TipDlgType::TWO_BUTTON));
	if (pTipDlg->exec() == QDialog::Rejected)
	{
		ULOG(LOG_INFO, "Cancel mask!");
		return;
	}
	
    ch::tf::ReagentGroup newReagent;
    newReagent.__set_id(m_reagent.id);
	// 设置试剂组的工作曲线
    newReagent.__set_caliCurveId(curve->id);

    // 设置曲线类型
    if (m_reagent.reagentKeyInfos == curve->reagentKeyInfos)
    {
        newReagent.__set_caliCurveSource(tf::CurveSource::CURVE_SOURCE_BOTTLE);
    }
    else if ((curve->lotCurve == ch::tf::CurveUsage::CURVE_USAGE_MAIN ||
        (curve->lotCurve == ch::tf::CurveUsage::CURVE_USAGE_STANDBY &&
            curve->assayCurve != ch::tf::CurveUsage::CURVE_USAGE_MAIN)) &&
        curve->reagentKeyInfos.front().lot == m_reagent.reagentKeyInfos.front().lot)
    {
        newReagent.__set_caliCurveSource(tf::CurveSource::CURVE_SOURCE_LOT);
    }
    else /*if (curve->assayCurve == ch::tf::CurveUsage::CURVE_USAGE_MAIN ||
        curve->assayCurve == ch::tf::CurveUsage::CURVE_USAGE_STANDBY)*/
    {
        newReagent.__set_caliCurveSource(tf::CurveSource::CURVE_SOURCE_ASSAY);
    }
    

	if (!ch::c1005::LogicControlProxy::ModifyReagentGroup(newReagent))
	{
		ULOG(LOG_ERROR, "QueryReagentGroup() failed");
		return;
	}

    // 更新历史曲线
    m_reagent.__set_caliCurveId(curve->id);
    StartDialog(m_reagent);
}

///
/// @brief 启动校准详情对话框
///
/// @param[in]  reagent  实际详细信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月15日，新建函数
///
void QCaliBrateResult::StartDialog(ch::tf::ReagentGroup& reagent)
{
    ULOG(LOG_INFO, "%s, reagent=%s.", __FUNCTION__, ToString(reagent));

    std::shared_ptr<UserInfoManager> userPms = UserInfoManager::GetInstance();
    ui->update_btn->setVisible(userPms->IsPermisson(PMS_CALI_RESULT_MANUAL_UPDATE_CUR));
    ui->export_none_flat_btn->setVisible(userPms->IsPermisson(PSM_IM_PRINTEXPORT_CALIRESULT));
    ui->update_btn->setEnabled(false);

	m_reagent = reagent;
	if (m_calibrate == nullptr)
	{
		m_calibrate = new QStandardItemModel(this);
		ui->calibrate_result->setModel(m_calibrate);
		ui->calibrate_result->setSelectionMode(QAbstractItemView::SingleSelection);
		ui->calibrate_result->setEditTriggers(QAbstractItemView::NoEditTriggers);
	}

	// 清空详细信息
	for (const auto& lable : m_labelMap)
	{
		lable.second->setText("");
	}

	// 清空当前曲线的批号和时间
	{
		ui->lot_lable->setText("");
        ui->time_lable->setText("");
        ui->lot_lable_his->setText("");
        ui->time_lable_his->setText("");
        /*ui->history_btn->setStyleSheet("#history_btn{background: url(:/Leonis/resource/image/ch/c1005/ch-history-btn.png) no-repeat;"
            "border: 0;}");*/
        ui->widget->setVisible(false);
        ui->text_choose_curve_label->setVisible(true);
        ui->text_history_curve_label->setVisible(false);
    }

	m_calibrate->clear();

	// 横向
	m_calibrate->setHorizontalHeaderLabels({ tr("校准品水平"), tr("吸光度"), tr("浓度") });
	ui->calibrate_result->setColumnWidth(0, 90);
	ui->calibrate_result->setColumnWidth(1, 110);

	if (m_parameter == nullptr)
	{
		m_parameter = new QStandardItemModel(this);
		ui->calibrate_parameter->setModel(m_parameter);
		ui->calibrate_parameter->setSelectionMode(QAbstractItemView::SingleSelection);
        ui->calibrate_parameter->setEditTriggers(QAbstractItemView::NoEditTriggers);
	}

	m_parameter->clear();

	// 横向
	m_parameter->setHorizontalHeaderLabels({  tr("校准参数"), tr("结果") });
	ui->calibrate_parameter->setColumnWidth(0, 113);

	// 清除曲线
	ui->curve_widget->clearCurve();
	// 获取当前使用曲线
    m_curCurve = SetCurrentCurve(reagent, m_currentCurves);
	// 获取和设置历史曲线
	m_historyCurve = GetHistoryCurves(reagent);
	SetHistoryCurve(m_historyCurve);

    SetTableViewFontSize(ui->calibrate_result);
    SetTableViewFontSize(ui->calibrate_parameter);
}

///
/// @brief 获取单例
///
///
/// @return 单例句柄
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月18日，新建函数
///
QCaliBrateResult& QCaliBrateResult::GetInstance()
{
	static QCaliBrateResult instance;
	return instance;
}

///
/// @brief 初始化
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月18日，新建函数
///
void QCaliBrateResult::Init()
{
	// 关闭按钮
	connect(ui->close_btn, &QPushButton::clicked, this, [&]() {this->close(); });
	connect(ui->restore_btn, &QPushButton::clicked, this, [this] {
		ui->curve_widget->setDefaultViewScale();
	});
	// 历史曲线选择
	connect(ui->history_btn, SIGNAL(BComboxSelected(int)), this, SLOT(OnHistoryCurve(int)));
	// 更新工作曲线
	connect(ui->update_btn, SIGNAL(clicked()), this, SLOT(OnUpdateWorkCurve()));
	// 打印按钮被点击
	connect(ui->print_none_flat_btn, &QPushButton::clicked, this, &QCaliBrateResult::OnPrintBtnClicked);
	// 导出按钮被点击
	connect(ui->export_none_flat_btn, &QPushButton::clicked, this, &QCaliBrateResult::OnExportBtnClicked);
	// 初始化校准的默认状态
	InitCaliStatus();
	// 设置坐标系
	{
		//初始化控件最开始的大小
		CoordinateDataType tmpSetData = ui->curve_widget->GetCurveAttrbute();
		// 改变类型
		tmpSetData.scaleType = 1;
		// 改变单位
		tmpSetData.unit = QString("");
		// 坐标系小数点余数
		tmpSetData.retainedNumber = 2;
		// 设置坐标属性
		ui->curve_widget->setAttribute(tmpSetData);
    }

    // 吸光度文字旋转
    {
        QGraphicsScene* sence = new QGraphicsScene;
        QFont cosFont = font();
        cosFont.setPixelSize(14);
        cosFont.setFamily("HarmonyOS Sans SC");
        //sence->setFont(cosFont);
        //sence->setForegroundBrush(QColor::fromRgb(0x56,0x56,0x56));
        auto textItem = sence->addText(tr("吸光度"), cosFont);
        textItem->setDefaultTextColor(QColor::fromRgb(0x56, 0x56, 0x56));
        ui->graphicsLabel->setScene(sence);
        ui->graphicsLabel->rotate(-90);
    }
}

///
/// @brief 获取试剂的当前校准曲线
///
/// @param[in]  reagent  试剂信息
///
/// @return 校准曲线
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月15日，新建函数
///
std::shared_ptr<ch::tf::CaliCurve> QCaliBrateResult::GetCurrentCurve(ch::tf::ReagentGroup& reagent)
{
	::ch::tf::CaliCurveQueryResp CurveRsp;
	::ch::tf::CaliCurveQueryCond CurveQuery;
	CurveQuery.__set_id(reagent.caliCurveId);
	CurveQuery.__set_checkDependecy(true);

	// 查询曲线数据
	ch::c1005::LogicControlProxy::QueryCaliCurve(CurveRsp, CurveQuery);
	if (CurveRsp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS || CurveRsp.lstCaliCurves.empty() || CurveRsp.lstCaliCurves[0].isEditCurve)
	{
		return nullptr;
	}

	return make_shared<ch::tf::CaliCurve>(CurveRsp.lstCaliCurves[0]);
}

///
/// @brief 获取试剂的历史校准曲线(历史曲线信息的来源1：项目曲线，2：批曲线，3：所有本瓶试剂的校准曲线)
///
/// @param[in]  reagent  试剂信息
///
/// @return 校准曲线列表
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月15日，新建函数
///
std::vector<std::shared_ptr<ch::tf::CaliCurve>> QCaliBrateResult::GetHistoryCurves(ch::tf::ReagentGroup& reagent)
{
	std::vector<::ch::tf::CaliCurveQueryCond> curveQueryConds;

    tf::LimitCond limtidCon;
    limtidCon.__set_offset(0);
    limtidCon.__set_count(MAXHISTORY);
    ::tf::OrderByCond orderCnd;
    orderCnd.__set_asc(false);
    orderCnd.__set_fieldName("caliTime");

    // 1：获取试剂组对应的项目曲线
	::ch::tf::CaliCurveQueryCond CurveQuery;
	CurveQuery.__set_deviceSN(std::vector<std::string>({ reagent.deviceSN }));
	CurveQuery.__set_assayCode(reagent.assayCode);
	CurveQuery.__set_version(std::to_string(reagent.reagentVersion));
    CurveQuery.__set_successed(true);
    CurveQuery.__set_isEditCurve(false);
    CurveQuery.__set_assayCurve({ ::ch::tf::CurveUsage::CURVE_USAGE_MAIN, ::ch::tf::CurveUsage::CURVE_USAGE_STANDBY });
    CurveQuery.__set_checkDependecy(false);
    CurveQuery.__set_limit(limtidCon);
    CurveQuery.orderByConds.push_back(orderCnd);
	curveQueryConds.push_back(CurveQuery);

	// 批号为空，则后续查找无法执行
	if (reagent.reagentKeyInfos.empty())
	{
		return CaliBrateCommom::GetCurvesByCond(curveQueryConds);
	}

	// 2：获取试剂组对应的批曲线
	::ch::tf::CaliCurveQueryCond CurveLotQuery;
	CurveLotQuery.__set_deviceSN(std::vector<std::string>({ reagent.deviceSN }));
	CurveLotQuery.__set_assayCode(reagent.assayCode);
	CurveLotQuery.__set_reagentLot(reagent.reagentKeyInfos[0].lot);
    CurveLotQuery.__set_version(std::to_string(reagent.reagentVersion));
    CurveLotQuery.__set_successed(true);
    CurveLotQuery.__set_isEditCurve(false);
	CurveLotQuery.__set_lotCurve({ ::ch::tf::CurveUsage::CURVE_USAGE_MAIN, ::ch::tf::CurveUsage::CURVE_USAGE_STANDBY });
    CurveLotQuery.__set_checkDependecy(false);
    CurveLotQuery.__set_limit(limtidCon);
    CurveLotQuery.orderByConds.push_back(orderCnd);
	curveQueryConds.push_back(CurveLotQuery);

	// 3：获取本试剂的校准曲线
	::ch::tf::CaliCurveQueryCond CurveBottleQuery;
	CurveBottleQuery.__set_deviceSN(std::vector<std::string>({ reagent.deviceSN }));
	CurveBottleQuery.__set_assayCode(reagent.assayCode);
    CurveBottleQuery.__set_version(std::to_string(reagent.reagentVersion));
    CurveBottleQuery.__set_successed(true);
    CurveBottleQuery.__set_isEditCurve(false);
	CurveBottleQuery.__set_reagentKeyInfos(reagent.reagentKeyInfos);
    CurveBottleQuery.__set_checkDependecy(false);
    CurveBottleQuery.__set_limit(limtidCon);
    CurveBottleQuery.orderByConds.push_back(orderCnd);
	curveQueryConds.push_back(CurveBottleQuery);

    auto&& lstResults = CaliBrateCommom::GetCurvesByCond(curveQueryConds);
    if (lstResults.size() > 1)
    {
        std::sort(lstResults.begin(), lstResults.end(), [](std::shared_ptr<ch::tf::CaliCurve> first, std::shared_ptr<ch::tf::CaliCurve> second) {
            return first->caliTime > second->caliTime;
        });
    }

	return lstResults;
}

///
/// @brief 设置历史曲线
///
/// @param[in]  caliCurves  历史曲线组
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月18日，新建函数
///
void QCaliBrateResult::SetHistoryCurve(std::vector<std::shared_ptr<ch::tf::CaliCurve>>& caliCurves)
{
	QStringList curveList;
    ch::tf::SuppliesInfoQueryResp qryResp;
    ch::tf::SuppliesInfoQueryCond qryCond;
    qryCond.__set_pos(m_reagent.posInfo.pos);
    qryCond.__set_area(m_reagent.posInfo.area);
    qryCond.__set_suppliesCode(m_reagent.assayCode);
    qryCond.__set_deviceSN({ m_reagent.deviceSN });

    // 查询曲线数据
    ch::c1005::LogicControlProxy::QuerySuppliesInfo(qryResp, qryCond);

    int count = 0;
    int curCurveIdNo = -1;
	for(const auto& historyCurve : caliCurves)
	{
        // 最多只获取50个历史曲线
        if (count >= MAXHISTORY)
            break;

        if (historyCurve->isEditCurve)
            continue;

        if (m_curCurve && m_curCurve->id == historyCurve->id)
            curCurveIdNo = curveList.size() / 3;

		// 试剂批号
		curveList += historyCurve->reagentLot.c_str();
		// 校准时间
		curveList += ToCfgFmtDateTime(historyCurve->caliTime.c_str());
        // 瓶曲线
        if (historyCurve->caliMode == ::tf::CaliMode::CALI_MODE_INTELLIGENT)
        {
            curveList += ("AC");
        }
        else if (historyCurve->reagentKeyInfos == m_reagent.reagentKeyInfos)
        {
            curveList += tr("瓶曲线");
        }
        else if ((historyCurve->lotCurve == ch::tf::CurveUsage::CURVE_USAGE_MAIN ||
                (historyCurve->lotCurve == ch::tf::CurveUsage::CURVE_USAGE_STANDBY &&
                historyCurve->assayCurve != ch::tf::CurveUsage::CURVE_USAGE_MAIN)) &&
                historyCurve->reagentKeyInfos.front().lot == m_reagent.reagentKeyInfos.front().lot)
        {
            curveList += tr("批曲线");
        }
        else
        {
            //curveList += tr("项目曲线");
            curveList += "";
        }

        ++count;
	}

	// 根据上面的内容，给对话框赋值
	QStringList headerList;
	// 表头信息
	headerList <<  tr("试剂批号") << tr("校准时间") << tr("曲线类型");
	ComboxInfo curveCombox;
	curveCombox.headList = headerList;
	// 表的内容信息
	curveCombox.ContentList = curveList;
    // 清除标签信息
    ui->history_btn->ClearLabelsInfo();
    // 设置当前曲线所在行
    ui->history_btn->SetCurrentCurveNo(curCurveIdNo);
	// 设置信息到表头
	ui->history_btn->setComboxInfo(curveCombox);
}

///
/// @brief 根据参数获取显示曲线
///
/// @param[in]  curveData  曲线参数
/// @param[in]  curveAlg   算法曲线相关参数
/// @param[in]  curves     曲线可绘制参数
///
/// @return		成功则true
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月18日，新建函数
///
bool QCaliBrateResult::ShowCurve(std::shared_ptr<ch::tf::CaliCurve>& curveData, shared_ptr<ch::ICalibrateCurve>& curveAlg, std::vector<curveSingleDataStore>& curves)
{
	if (!curveData || !curveAlg || curves.empty())
	{
		return false;
	}

    // 将x轴的浓度坐标转为当前单位的值
    auto FuncToCurUnitConc = [this](curveSingleDataStore& curve)
    {
        for (auto& data : curve.dataContent)
        {
            data.xData = CommonInformationManager::GetInstance()->ToCurrentUnitValue(m_reagent.assayCode, data.xData);
        }
    };

	// 曲线创建不成功，或者数据为空，则返回
	if (!CaliBrateCommom::CreatScatterCurve(curves[0], curveAlg) || curves[0].dataContent.empty())
	{
		ULOG(LOG_ERROR, "caliCurve data is empty %s()", __FUNCTION__);
		return false;
	}
    FuncToCurUnitConc(curves[0]);
	curves[0].curveId = ui->curve_widget->addCurveData(curves[0]);

	if (curves.size() <= 1)
	{
		return true;
	}

	//获取算法计算的曲线
	if (!CaliBrateCommom::CreatContinueCurve(curves[1], curves[0], curveAlg) || curves[1].dataContent.empty())
	{
		ULOG(LOG_ERROR, "caliCurve data is empty %s()", __FUNCTION__);
		return false;
	}


    // 如果只有两个点，又没有连接线，就直接连为直线
    if (curves[1].dataContent.empty() && curves[0].dataContent.size() == 2)
    {
        curves[1].dataContent.append(curves[0].dataContent);
    }

	curves[1].curveId = ui->curve_widget->addCurveData(curves[1]);
	return true;
}

///
/// @brief 设置校准状态
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月18日，新建函数
///
void QCaliBrateResult::InitCaliStatus()
{
	// 清除表中内容
	QList<QLabel*> memberLables = ui->widget1->findChildren<QLabel*>();
	for (auto meber : memberLables)
	{
		int index = CaliBrateCommom::GetKeyValueFromWidgetName(meber);
		if (index == -1)
		{
			continue;
		}

		m_labelMap.insert(std::make_pair(index, meber));
		meber->setText("");
	}
}

///
/// @brief 根据试剂和校准曲线信息获取校准状态
///
/// @param[in]  reagent  试剂信息
/// @param[in]  curve    曲线状态
///
/// @return 校准状态信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月18日，新建函数
///
boost::optional<QStringList> QCaliBrateResult::GetCaliStatusShow(ch::tf::ReagentGroup& reagent, std::shared_ptr<ch::tf::CaliCurve>& curve)
{
	if (!curve)
	{
		return boost::none;
	}

	QStringList statusList;

	auto spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(curve->assayCode);
    if (spAssay == nullptr)
        return boost::none;

    // 项目名称
	statusList += QString::fromStdString(spAssay->assayName);
    // 单位
    statusList += CommonInformationManager::GetInstance()->GetCurrentUnit(spAssay->assayCode);
    // 试剂批号
    statusList += QString::fromStdString(curve->reagentLot);
    // 试剂瓶号
    statusList += QString::fromStdString(curve->reagentKeyInfos[0].sn);
    // 模块
    statusList += QString::fromStdString(CommonInformationManager::GetDeviceName(reagent.deviceSN));
	// 校准品批号
	statusList += QString::fromStdString(curve->calibratorLot);
	// 校准方法
	statusList += CaliBrateCommom::GetCalibrateType(CaliBrateCommom::GetCurve(*curve));
	// 校准时间
	statusList += ToCfgFmtDateTime(QString::fromStdString(curve->caliTime));
    // 数据报警
    statusList += QString::fromStdString(curve->failedReason);
    // 执行方法
    statusList += CaliBrateCommom::GetCaliBrateMode(curve->caliMode);

	return boost::make_optional(statusList);
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
void QCaliBrateResult::OnPrintBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    CaliResultInfoCh Info;
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
void QCaliBrateResult::OnExportBtnClicked()
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
        CaliResultInfoCh Info;
        GetPrintExportInfo(Info);
        QString strPrintTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
        Info.strPrintTime = strPrintTime.toStdString();
        Info.strExportTime = strPrintTime.toStdString();
        std::string strInfo = GetJsonString(Info);
        ULOG(LOG_INFO, "Print datas : %s", strInfo);
        std::string unique_id;
        QString strDirPath = QCoreApplication::applicationDirPath();
        QString strUserTmplatePath = strDirPath + "/resource/exportTemplate/ExportCalibratorResultCh.lrxml";
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
bool QCaliBrateResult::SetCaliInfo(QStringList& strExportTextList)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 创建表头
	QString strHeader("");
	strHeader += tr("项目名称") + "\t";
	strHeader += tr("单位") + "\t";
	strHeader += tr("试剂批号") + "\t";
	strHeader += tr("试剂瓶号") + "\t";
    strHeader += tr("模块") + "\t";
    strHeader += tr("酸性清洗液批号") + "\t";
    strHeader += tr("碱性清洗液批号") + "\t";
	strHeader += tr("校准品批号") + "\t";
	strHeader += tr("校准方法") + "\t";
    strHeader += tr("校准时间") + "\t";
    strHeader += tr("执行方法") + "\t";
    strHeader += tr("数据报警") + "\t";
    strHeader += tr("仪器编号") + "\t";
    strHeader += tr("软件版本") + "\t";
    strHeader += tr("校准结果");
	strExportTextList.push_back(std::move(strHeader));

	// 获取当前曲线信息
	auto currentCurve = GetCurrentCurve(m_reagent);
	if (!currentCurve)
	{
		ULOG(LOG_ERROR, "Can't find the current calibrate curve %s()", __FUNCTION__);
		return false;
	}

	QString rowData = "";

	// 项目名称
	auto spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(currentCurve->assayCode);
    if (spAssay == nullptr)
    {
        ULOG(LOG_ERROR, "Can't GetAssayInfo %d()", currentCurve->assayCode);
        return false;
    }
	rowData += QString::fromStdString(spAssay->assayName);
	rowData += "\t";

	// 单位
	rowData += std::move(CommonInformationManager::GetInstance()->GetCurrentUnit(currentCurve->assayCode));
	rowData += "\t";

	// 试剂批号
	rowData += m_reagent.reagentKeyInfos.empty() ? "" : QString::fromStdString(currentCurve->reagentLot);
	rowData += "\t";

	// 试剂瓶号
	rowData += QString::fromStdString(currentCurve->reagentKeyInfos.empty() ? "" : currentCurve->reagentKeyInfos[0].sn);
	rowData += "\t";

	// 模块
	rowData += QString::fromStdString(CommonInformationManager::GetDeviceName(currentCurve->deviceSN));
    rowData += "\t";

    // 酸性清洗液批号
    rowData += "";
    rowData += "\t";

    // 碱性清洗液批号
    rowData += "";
    rowData += "\t";

	// 校准品批号
	rowData += QString::fromStdString(currentCurve->calibratorLot);
	rowData += "\t";

	// 校准方法
	rowData += CaliBrateCommom::GetCalibrateType(CaliBrateCommom::GetCurve(*currentCurve));
	rowData += "\t";

	// 校准时间
	rowData += QString::fromStdString(currentCurve->caliTime);
    rowData += "\t";

    // 执行方法
    rowData += CaliBrateCommom::GetCaliBrateMode(currentCurve->caliMode);
    rowData += "\t";

    // 数据报警
    rowData += QString::fromStdString(currentCurve->failedReason);
    rowData += "\t";

    // 仪器编号
    rowData += QString::fromStdString(currentCurve->deviceSN);
    rowData += "\t";

    // 软件版本
    rowData += QString::fromStdString(DictionaryQueryManager::GetInstance()->GetSoftwareVersion());
    rowData += "\t";

    // 校准结果(最新一次校准结果)
    rowData += true ? tr("成功") : tr("失败");

	strExportTextList.push_back(std::move(rowData));

	// 换行
	strExportTextList.push_back("");

	// 设置校准品测试结果信息
	QString strCaliTestInfoLevel(tr("校准品水平") + "\t");
    QString strCaliTestInfoFirstCpus(tr("首次杯号") + "\t");
    QString strCaliTestInfoFirstPreAssay(tr("同杯前反应") + "\t");
    QString strCaliTestInfoSecCpus(tr("第二次杯号") + "\t");
    QString strCaliTestInfoSecPreAssay(tr("同杯前反应") + "\t");
    QString strCaliTestInfoAbs_1(tr("吸光度1") + "\t");
    QString strCaliTestInfoAbs_2(tr("吸光度2") + "\t");
    QString strCaliTestInfoAbs(tr("吸光度均值") + "\t");
    QString strCaliTestInfoConc(tr("浓度") + "\t");

    auto cupInfos = std::move(CaliBrateCommom::GetCurveCupInfos(*currentCurve));
    for (int64_t i = 1; i <= 6; ++i)
    {
        if (cupInfos.find(i) == cupInfos.end())
        {
            strCaliTestInfoFirstCpus += "\t";
            strCaliTestInfoFirstPreAssay += "\t";
            strCaliTestInfoSecCpus += "\t";
            strCaliTestInfoSecPreAssay += "\t";
        }
        else
        {
            strCaliTestInfoFirstCpus += QString::fromStdString(cupInfos[i][0]) + "\t";
            strCaliTestInfoFirstPreAssay += QString::fromStdString(cupInfos[i][1]) + "\t";
            strCaliTestInfoSecCpus += QString::fromStdString(cupInfos[i][2]) + "\t";
            strCaliTestInfoSecPreAssay += QString::fromStdString(cupInfos[i][3]) + "\t";
        }
    }

	// 校准数据
	shared_ptr<ch::ICalibrateCurve> currentCurveParamter = CaliBrateCommom::GetCurve(*currentCurve);
	if (currentCurveParamter == nullptr)
	{
		ULOG(LOG_ERROR, "creat the caliCurve failed %s()", __FUNCTION__);
		return false;
	}

	int ctiRow = 1;
	QString caliTestInfoRow = "";
    const auto& info = currentCurveParamter->GetAlgorithmConfigs();
    auto points = info.origPoints;
    if (info.cali_mode == ch::CALI_MODE::CALI_MODE_BLANK &&
        info.spanedPoints.size() > 0 && points.size() > 0)
    {
        points[0] = info.spanedPoints[0];
    }
	for (auto curveAbsorb : points)
	{
        // 校准水平
        strCaliTestInfoLevel += QObject::tr("校准品") + QString::number(ctiRow) + "\t";
        //吸光度1
        strCaliTestInfoAbs_1 += QString::number(std::get<2>(curveAbsorb)[0]) + "\t";
        //吸光度2
        strCaliTestInfoAbs_2 += QString::number(std::get<2>(curveAbsorb)[1]) + "\t";
        //吸光度均值
        strCaliTestInfoAbs += QString::number(std::get<1>(curveAbsorb)) + "\t";
        // 浓度
        auto curFacValue = CommonInformationManager::GetInstance()->ToCurrentUnitValue(currentCurve->assayCode, std::get<0>(curveAbsorb));
        strCaliTestInfoConc += QString::number(curFacValue, 'f', spAssay->decimalPlace) + "\t";

		ctiRow++;
    }

    // 导出校准品水平信息
    strExportTextList.push_back(std::move(strCaliTestInfoLevel));

    // 导出校准品位置
    strExportTextList.push_back(tr("校准品位置"));

    // 导出首次杯号
    strExportTextList.push_back(strCaliTestInfoFirstCpus);

    // 导出同杯前反应
    strExportTextList.push_back(strCaliTestInfoFirstPreAssay);
    
    // 导出第二次杯号
    strExportTextList.push_back(strCaliTestInfoSecCpus);

    // 导入第二次同杯前反应
    strExportTextList.push_back(strCaliTestInfoSecPreAssay);

    // 导出吸光度1
    strExportTextList.push_back(std::move(strCaliTestInfoAbs_1));

    // 导出吸光度2
    strExportTextList.push_back(std::move(strCaliTestInfoAbs_2));

    // 导出吸光度均值
    strExportTextList.push_back(std::move(strCaliTestInfoAbs));
    
    // 导出浓度
    strExportTextList.push_back(std::move(strCaliTestInfoConc));
    strExportTextList.push_back("");    // 换行

	// 校准参数
	QString strCaliParamsInfoPara("");
	strCaliParamsInfoPara += QObject::tr("校准参数") + "\t";
    QString strCaliParamsInfoResult("");
    strCaliParamsInfoResult += QObject::tr("结果") + "\t";

	// 校准参数
	QString caliParamsInfoRow = "";
	for (const auto& parameterData : currentCurveParamter->GetModelParameters())
	{
        strCaliParamsInfoPara += QString::fromStdString(parameterData.first) + "\t";
        strCaliParamsInfoResult += QString::number(parameterData.second) + "\t";
	}

    // 导出校准参数
    strExportTextList.push_back(std::move(strCaliParamsInfoPara));

    // 导出参数结果
    strExportTextList.push_back(std::move(strCaliParamsInfoResult));

	return true;
}

void QCaliBrateResult::SetPrintAndExportEnabled(bool hasCurve)
{
    // 暂时只是用是否有当前工作曲线
    ui->print_none_flat_btn->setEnabled(hasCurve);
    ui->export_none_flat_btn->setEnabled(hasCurve);
}

bool QCaliBrateResult::GetPrintExportInfo(CaliResultInfoCh & info)
{
    // 获取当前曲线信息
    auto currentCurve = GetCurrentCurve(m_reagent);
    if (!currentCurve)
    {
        ULOG(LOG_ERROR, "Can't find the current calibrate curve %s()", __FUNCTION__);
        return false;
    }

    auto spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(currentCurve->assayCode);
    if (!spAssay)
        return false;

    int decimal = spAssay->decimalPlace;

    // 模块
    info.strModelName = CommonInformationManager::GetDeviceName(m_reagent.deviceSN);

    // 项目名称
    info.strItemName = spAssay->assayName;

    // 单位
    QString mainUnit = "";
    info.strUnit =  CommonInformationManager::GetInstance()->GetCurrentUnit(currentCurve->assayCode).toStdString();

    // 仪器编号
    info.strDeviceNum = m_reagent.deviceSN;

    // 软件版本
    info.strSoftVersion = DictionaryQueryManager::GetInstance()->GetSoftwareVersion();

    // 试剂批号
    info.strReagentLot = currentCurve->reagentLot;

    // 试剂瓶号
    info.strReagentSN = currentCurve->reagentKeyInfos[0].sn;

    // 校准品批号
    info.strCalibratorLot = currentCurve->calibratorLot;

    // 校准时间
    info.strCalibrateDate = currentCurve->caliTime;

    // 酸性清洗液批号

    // 碱性清洗液批号

    // 校准方法
    info.strCaliType = CaliBrateCommom::GetCalibrateType(CaliBrateCommom::GetCurve(*currentCurve)).toStdString();

    // 执行方法
    info.strCaliMode = CaliBrateCommom::GetCaliBrateMode(currentCurve->caliMode).toStdString();

    // 数据报警
    info.strAlarm = currentCurve->failedReason;

    auto spUserInfo = UserInfoManager::GetInstance()->GetLoginUserInfo();
    // 导出人
    if (spUserInfo)
        info.strExportBy = spUserInfo->nickname;

    // 打印人
    info.strPrintBy = info.strExportBy;

    // 设置校准品测试结果信息
    QString strCaliTestInfoLevel(tr("校准品水平") + "\t");
    QString strCaliTestInfoAbs_1(tr("吸光度1") + "\t");
    QString strCaliTestInfoAbs_2(tr("吸光度2") + "\t");
    QString strCaliTestInfoAbs(tr("吸光度均值") + "\t");
    QString strCaliTestInfoConc(tr("浓度") + "\t");

    // 校准数据
    shared_ptr<ch::ICalibrateCurve> currentCurveParamter = CaliBrateCommom::GetCurve(*currentCurve);
    if (currentCurveParamter == nullptr)
    {
        ULOG(LOG_ERROR, "creat the caliCurve failed %s()", __FUNCTION__);
        return false;
    }

    auto cupInfos = std::move(CaliBrateCommom::GetCurveCupInfos(*currentCurve));
    int ctiRow = 1;
    const auto& tinfo = currentCurveParamter->GetAlgorithmConfigs();
    auto points = tinfo.origPoints;
    if (tinfo.cali_mode == ch::CALI_MODE::CALI_MODE_BLANK &&
        tinfo.spanedPoints.size() > 0 && points.size() > 0)
    {
        points[0] = tinfo.spanedPoints[0];
    }
    for (auto curveAbsorb : points)
    {
        CHCALIRESULTDATA caliResultData;
        if (cupInfos.find(ctiRow) != cupInfos.end())
        {
            // 首次杯号
            caliResultData.strFirstCup = cupInfos[ctiRow][0];
            // 同杯前反应
            caliResultData.strFirstCupPreAssay = cupInfos[ctiRow][1];
            // 第二次杯号
            caliResultData.strSecCup = cupInfos[ctiRow][2];
            // 同杯前反应
            caliResultData.strSecCupPreAssay = cupInfos[ctiRow][3];
        }
        // 校准水平
        caliResultData.strLevel = QObject::tr("校准品%1").arg(ctiRow).toStdString();
        //吸光度1
        caliResultData.strAbs1 = QString::number(std::get<2>(curveAbsorb)[0]).toStdString();
        //吸光度2
        caliResultData.strAbs2 = QString::number(std::get<2>(curveAbsorb)[1]).toStdString();
        //吸光度均值
        caliResultData.strAvgAbs = QString::number(std::get<1>(curveAbsorb)).toStdString();
        // 浓度
        auto curFacValue = CommonInformationManager::GetInstance()->ToCurrentUnitValue(currentCurve->assayCode, std::get<0>(curveAbsorb));
        caliResultData.strConc = QString::number(curFacValue, 'f', decimal).toStdString();

        info.vecCaliResultData.push_back(std::move(caliResultData));
        ctiRow++;
    }
    
    // 校准参数
    QString caliParamsInfoRow = "";
    for (const auto& parameterData : currentCurveParamter->GetModelParameters())
    {
        CHCALIPARAMETERS paras;
        paras.strCaliParameter  = parameterData.first;
        paras.strParameterValue = QString::number(parameterData.second).toStdString();
        info.vecCaliParameters.push_back(std::move(paras));
    }

    return true;
}
