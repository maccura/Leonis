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
/// @file     ImQCaliBrateResult.cpp
/// @brief    校准结果
///
/// @author   8090/YeHuaNing
/// @date     2022年11月29日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月29日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "ImQCaliBrateResult.h"
#include "ui_ImQCaliBrateResult.h"
#include <QStandardItemModel>
#include <QFileDialog>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsTextItem>
#include <QDateTime>
#include <QBuffer>

#include "../ch/CaliBrateCommom.h"
#include "thrift/im/i6000/I6000LogicControlProxy.h"

#include "shared/msgiddef.h"
#include "shared/messagebus.h"
#include "shared/tipdlg.h"
#include "shared/uicommon.h"
#include "shared/ReagentCommon.h"
#include "shared/FileExporter.h"
#include "shared/datetimefmttool.h"
#include "shared/CommonInformationManager.h"
#include "shared/ThriftEnumTransform.h"

#include "src/common/common.h"
#include "src/common/TimeUtil.h"
#include "src/common/Mlog/mlog.h"
#include "manager/UserInfoManager.h"
#include "manager/DictionaryQueryManager.h"
#include "Serialize.h"
#include "printcom.hpp"

ImQCaliBrateResult::ImQCaliBrateResult(QWidget *parent)
    : BaseDlg(parent),
    m_calibrate(nullptr),
    m_curCurve(nullptr),
    m_masterCurve(nullptr),
    m_currentIndex(-1),
    m_bDispMaster(false)
{
    ui = new Ui::ImQCaliBrateResult();
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

ImQCaliBrateResult::~ImQCaliBrateResult()
{
}

///
/// @brief 显示当前曲线
///
/// @param[in]  reagent		试剂信息
/// @param[in]  curveStore  曲线的绘制信息
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月29日，新建函数
///
std::shared_ptr<im::tf::CaliCurve> ImQCaliBrateResult::SetCurrentCurve(im::tf::ReagentInfoTable& reagent, std::vector<curveSingleDataStore>& curveStore)
{
    // 获取当前使用曲线
    GetCurrentCurve(reagent);
    auto currentCurve = m_curCurve;
    if (!currentCurve)
    {
        ULOG(LOG_ERROR, "Can't find the current calibrate curve %s()", __FUNCTION__);
        return nullptr;
    }
    ui->CaliExportBtn->setEnabled(true);
    ui->CaliPrintBtn->setEnabled(true);
    ui->restore_btn->setEnabled(true);

    // 主标曲
    if (m_bDispMaster)
    {
        m_masterCurves.clear();
        m_masterCurves.resize(2);
        m_masterCurves[0].colorType = ELEVEN;
        m_masterCurves[0].scatterMarkerShape = CIRCLE;
        m_masterCurves[1].colorType = ELEVEN;
        m_masterCurves[1].scatterMarkerShape = CALIBRATION;

        // 显示当前曲线
        ShowCurve(m_masterCurve, m_masterCurves);
    }

    // 当前曲线参数
    curveStore.clear();
    curveStore.resize(2);
    curveStore[0].colorType = TYPEEND;
    curveStore[0].scatterMarkerShape = CIRCLE;
    curveStore[1].colorType = TYPEEND;
    curveStore[1].scatterMarkerShape = CALIBRATION;

    // 显示当前曲线
    ShowCurve(currentCurve, curveStore);

    auto status = GetCaliStatusShow(reagent, currentCurve);
    if (status.has_value())
    {
        int i = 0;
        for (const auto& lable : m_labelMap)
        {
            (status.value().size() > i) ? lable.second->setText(ToCfgFmtDateTime(status.value()[i++])) : (void(0));
        }
    }

    // 设置工作曲线的状态信息
    {
        // 设置试剂批号
        ui->lot_lable->setText(QString::fromStdString(currentCurve->reagentLot));
        // 设置校准时间
        ui->time_lable->setText(ToCfgFmtDateTime(QString::fromStdString(currentCurve->caliExcuteDate)));
    }

    // 校准参数
    int row = 0;

    // 校准数据
    for (const auto& data : currentCurve->actualRLU.caliCurveRlu)
    {
        int column = 0;
		QString strLevel = "L" + QString::number(row+1);
		//水平
		m_calibrate->setItem(row, column++, CenterAligmentItem(strLevel));
        // 信号值
        m_calibrate->setItem(row, column++, CenterAligmentItem(QString::number(data.rlu, 'f', 0)));
		// 浓度
        QString strConc = "*";
        // 定量项目
        if (currentCurve->caliType == 1)
        {
            strConc = QString::number(CommonInformationManager::GetInstance()->ToCurrentUnitValue(currentCurve->assayCode, data.conc), 'f', 2);           
        }
        m_calibrate->setItem(row, column++, CenterAligmentItem(strConc));

        row++;
    }

    return currentCurve;
}

///
/// @brief 历史曲线的选择槽函数
///
/// @param[in]    index 历史曲线index
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月29日，新建函数
///
void ImQCaliBrateResult::OnHistoryCurve(int index)
{
    m_currentIndex = index;
    if (index > m_historyCurve.size())
    {
        return;
    }

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

    // 显示当前曲线
    ShowCurve(m_historyCurve[index], m_historyCurvesStore);
    
    if (m_historyCurve[index])
    {
        ui->lot_lable_his->setText(QString::fromStdString(m_historyCurve[index]->reagentLot));
        ui->time_lable_his->setText(QString::fromStdString(m_historyCurve[index]->caliExcuteDate));
        /*ui->im_history_btn->setStyleSheet("#im_history_btn{background: url(:/Leonis/resource/image/im/im-history-on-btn.png) no-repeat;"
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
/// @li 8090/YeHuaNing，2022年11月29日，新建函数
///
void ImQCaliBrateResult::OnUpdateWorkCurve()
{
    // 判断是否选中有历史曲线
    if (m_currentIndex < 0 || m_currentIndex >= m_historyCurve.size())
    {
        return;
    }

    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("确定要更新工作曲线?"), TipDlgType::TWO_BUTTON));
    if (pTipDlg->exec() == QDialog::Rejected)
    {
        ULOG(LOG_INFO, "Cancel mask!");
        return;
    }

    auto curve = m_historyCurve[m_currentIndex];
    // 设置试剂组的工作曲线
    if (!im::i6000::LogicControlProxy::UpdateCurentCurve(*curve, m_reagent))
        return;

    // 更新试剂校准信息
    m_reagent.__set_curveId(curve->id);
    m_reagent.__set_curveType((curve->reagentSN == m_reagent.reagentSN) ? 0 : 1);
    m_reagent.__set_curveExcuteDate(curve->caliExcuteDate);

    // 更新历史曲线
	StartDialog(m_reagent);
}

///
/// @brief  重新获取曲线进行显示
///
/// @par History:
/// @li 1226/zhangjing，2023年10月31日，新建函数
///
void ImQCaliBrateResult::initShow(void)
{
    // 清空详细信息
    for (const auto& lable : m_labelMap)
    {
        lable.second->setText("");
    }

    CfgToolImParam cfgTool;
    DictionaryQueryManager::GetInstance()->GetCfgToolImParamConfig(cfgTool);
    m_bDispMaster = cfgTool.bCalRltShowMc;
    if (m_bDispMaster)
    {
        ui->legend_label_5->show();
        ui->legend_text_fitting_2->show();
        ui->master_curve_legend_label->show();
    }
    else
    {
        ui->legend_label_5->hide();
        ui->legend_text_fitting_2->hide();
        ui->master_curve_legend_label->hide();
    }

    // 清空当前曲线的批号和时间
    {
        ui->lot_lable->setText("");
        ui->time_lable->setText("");
        ui->lot_lable_his->setText("");
        ui->time_lable_his->setText("");
        /*ui->im_history_btn->setStyleSheet("#im_history_btn{background: url(:/Leonis/resource/image/im/im-history-btn.png) no-repeat;"
            "border: 0;}");*/
        ui->widget->setVisible(false);
        ui->text_choose_curve_label->setVisible(true);
        ui->text_history_curve_label->setVisible(false);
    }

    m_calibrate->clear();
    // 横向
    m_calibrate->setHorizontalHeaderLabels({ tr("校准品水平"),tr("信号值"), tr("浓度") });

    //设置间隔
    ui->calibrate_result->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // 清除曲线
    ui->curve_widget->clearCurve();
    m_historyCurve.clear();

    // 获取当前使用曲线
    SetCurrentCurve(m_reagent, m_currentCurves);

    // 获取和设置历史曲线
    GetHistoryCurves(m_reagent);
    // 无曲线可切换或权限不足， 更新按钮灰化
    if (m_historyCurve.size() > 0)
    {
        ui->update_btn->setEnabled(true);
    }
    else
    {
        ui->update_btn->setEnabled(false);
    }
}

///
/// @brief 启动校准详情对话框
///
/// @param[in]  reagent  实际详细信息
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月29日，新建函数
///
void ImQCaliBrateResult::StartDialog(im::tf::ReagentInfoTable& reagent)
{
    std::shared_ptr<UserInfoManager> userPms = UserInfoManager::GetInstance();
    ui->update_btn->setVisible(userPms->IsPermisson(PMS_CALI_RESULT_MANUAL_UPDATE_CUR));
    ui->CaliExportBtn->setVisible(userPms->IsPermisson(PSM_IM_PRINTEXPORT_CALIRESULT));

    m_reagent = reagent;
    if (m_calibrate == nullptr)
    {
        m_calibrate = new QStandardItemModel(this);
        ui->calibrate_result->setModel(m_calibrate);
		ui->calibrate_result->setSelectionBehavior(QAbstractItemView::SelectRows);
        ui->calibrate_result->setSelectionMode(QAbstractItemView::SingleSelection);
        ui->calibrate_result->setEditTriggers(QAbstractItemView::NoEditTriggers);
    }

    initShow();
}

///
/// @brief 获取单例
///
///
/// @return 单例句柄
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月29日，新建函数
///
ImQCaliBrateResult& ImQCaliBrateResult::GetInstance()
{
    static ImQCaliBrateResult instance;
    return instance;
}

///
/// @brief 初始化
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月29日，新建函数
///
void ImQCaliBrateResult::Init()
{
    // 关闭按钮
    connect(ui->close_btn, &QPushButton::clicked, this, [&]() {this->close(); });
    connect(ui->restore_btn, &QPushButton::clicked, this, [this] {
        ui->curve_widget->setDefaultViewScale();
    });
    // 历史曲线选择
    connect(ui->im_history_btn, SIGNAL(BComboxSelected(int)), this, SLOT(OnHistoryCurve(int)));
    // 更新工作曲线
    connect(ui->update_btn, SIGNAL(clicked()), this, SLOT(OnUpdateWorkCurve()));
    // 打印按钮
    connect(ui->CaliPrintBtn, SIGNAL(clicked()), this, SLOT(OnPrintBtnClicked()));
    // 导出按钮
    connect(ui->CaliExportBtn, SIGNAL(clicked()), this, SLOT(OnExportBtnClicked()));

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
        cosFont.setPixelSize(16);
        cosFont.setFamily("HarmonyOS Sans SC");
        //sence->setFont(cosFont);
        //sence->setForegroundBrush(QColor::fromRgb(0x56,0x56,0x56));
        auto textItem = sence->addText(tr("信号值"), cosFont);
        textItem->setDefaultTextColor(QColor::fromRgb(0x56, 0x56, 0x56));
        ui->graphicsLabel->setScene(sence);
        ui->graphicsLabel->rotate(-90);
    }

    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);
    // 监听校准完成
    REGISTER_HANDLER(MSG_ID_IM_CALI_END, this, OnCaliEnd);
}

///
/// @brief 获取试剂的当前校准曲线
///
/// @param[in]  reagent  试剂信息
///
/// @return 校准曲线
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月29日，新建函数
///
void ImQCaliBrateResult::GetCurrentCurve(im::tf::ReagentInfoTable& reagent)
{
    m_curCurve = nullptr;
    m_masterCurve = nullptr;
    ::im::tf::CaliCurveQueryResp CurveRsp;
    if (!im::i6000::LogicControlProxy::GetReagentCurrentCurveInfo(CurveRsp, reagent) ||
        CurveRsp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS ||
        CurveRsp.lstCurveConfigs.empty())
        return;

	for (auto re: CurveRsp.lstCurveConfigs)
	{
		if (re.curUse == 1)
		{
            m_curCurve = make_shared<im::tf::CaliCurve>(re);
            continue; 
		}
        m_masterCurve = make_shared<im::tf::CaliCurve>(re);
	}

    return;
}

///
/// @brief 获取试剂的历史校准曲线(历史曲线信息的来源1：项目曲线，2：批曲线，3：所有本瓶试剂的校准曲线)
///
/// @param[in]  reagent  试剂信息
///
/// @return 
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月29日，新建函数
///
void ImQCaliBrateResult::GetHistoryCurves(im::tf::ReagentInfoTable& reagent)
{
    // 获取历史曲线
    ::im::tf::CaliCurveQueryResp _return;
    if (!im::i6000::LogicControlProxy::GetReagentCurveInfo(_return, reagent) ||
        _return.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS) {
        return;
    }

    QStringList curveList/*({ "--", "--", "--" })*/;
    std::vector<::im::tf::CaliCurveQueryCond> curveQueryConds;

    int curCurveIdNo = -1;
    for (const auto& historyCurve : _return.lstCurveConfigs)
    {
        // 忽略已经不是批曲线的当前曲线
		if (historyCurve.curUse == 1 && historyCurve.reagentSN != reagent.reagentSN && historyCurve.lot != 1)
		{
			continue;
        }

        if (m_curCurve && m_curCurve->id == historyCurve.id)
            curCurveIdNo = curveList.size() / 3;

        m_historyCurve.emplace_back(std::make_shared<im::tf::CaliCurve>(historyCurve));
        // 试剂批号
        curveList += QString::fromStdString(historyCurve.reagentLot);
        // 校准时间
        curveList += ToCfgFmtDateTime(QString::fromStdString(historyCurve.caliExcuteDate)); ;
        // 工作曲线
        if (historyCurve.reagentLot == reagent.reagentLot && reagent.reagentSN == historyCurve.reagentSN)
            curveList += tr("瓶曲线");
        else
            curveList += tr("批曲线");

        // 曲线以及排好序了，只取前50条记录-共3列需要除3，-bug23102
        if (curveList.size() / 3 >= 50)
        {
            break;
        }
    }

    // 根据上面的内容，给对话框赋值
    QStringList headerList;
    // 表头信息
    headerList << tr("试剂批号") << tr("校准时间") << tr("曲线类型");
    ComboxInfo curveCombox;
    curveCombox.headList = headerList;
    // 表的内容信息
    curveCombox.ContentList = curveList;
    // 清除标签信息
    ui->im_history_btn->ClearLabelsInfo();
    // 设置当前曲线所在行
    ui->im_history_btn->SetCurrentCurveNo(curCurveIdNo);
    // 设置信息到表头
    ui->im_history_btn->setComboxInfo(curveCombox);
}

///
/// @brief 设置历史曲线
///
/// @param[in]  caliCurves  历史曲线组
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月29日，新建函数
///
void ImQCaliBrateResult::SetHistoryCurve(std::vector<std::shared_ptr<im::tf::CaliCurve>>& caliCurves)
{

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
/// @li 8090/YeHuaNing，2022年11月29日，新建函数
///
bool ImQCaliBrateResult::ShowCurve(std::shared_ptr<im::tf::CaliCurve>& curveData, std::vector<curveSingleDataStore>& curves)
{
    if (!curveData || curves.empty())
    {
        return false;
    }

	auto assayCode = curveData->assayCode;
   
    // 拟合点信息（空心圆）
    int i = 1;
    for (const auto& cv : curveData->actualRLU.caliCurveRlu)
    {
        RealDataStruct tempDate;
        //定性只显示1,2
        tempDate.xData = i++;
        if (curveData->caliType == 1)
        {
            tempDate.xData = CommonInformationManager::GetInstance()->ToCurrentUnitValue(assayCode, cv.conc);
        }       
        tempDate.yData = cv.rlu;
        tempDate.type = CIRCLE;
        tempDate.color = curves[0].colorType;
        tempDate.hollow = true;
        curves[0].dataContent.append(tempDate);
    }

    // 校准点信息（实心圆），优先显示校准点信息
    i = 0;
    for (const auto& cv : curveData->actualRLU.caliDocRlu)
    {
        RealDataStruct tempDate;
        //定性只显示1,2
        tempDate.xData = i+1;
        if (curveData->caliType == 1)
        {
            tempDate.xData = CommonInformationManager::GetInstance()->ToCurrentUnitValue(assayCode, cv.conc);
        }
        tempDate.yData = cv.meanRlu;
        tempDate.type = CIRCLE;
        tempDate.color = curves[0].colorType;
        tempDate.hollow = false;
        if (m_bDispMaster && curveData->caliJudgeInfo.delta.size() >= i+1)
        {
            double tip = abs(curveData->caliJudgeInfo.delta[i]-1)*100;
            tempDate.tip = (QString::number(tip, 'f', 1) + "%").toStdString();
        }
        i++;
        curves[0].dataContent.append(tempDate);
    }

    curves[0].curveId = ui->curve_widget->addCurveData(curves[0]);

    if (curves.size() > 1)
    {
        //获取算法计算的曲线
        for (const auto& calcRlu : curveData->vtCalcRlu)
        {
            //每次取多少点来画曲线
            RealDataStruct tempdata;
            tempdata.xData = CommonInformationManager::GetInstance()->ToCurrentUnitValue(assayCode, calcRlu.conc);
            tempdata.yData = calcRlu.rlu;
            curves[1].dataContent.append(tempdata);
        }

        // 如果只有两个点，又没有连接线，就直接连为直线
        if (curves[1].dataContent.empty() && curveData->actualRLU.caliCurveRlu.size() == 2)
        {
            curves[1].dataContent.append(curves[0].dataContent);
        }

        curves[1].curveId = ui->curve_widget->addCurveData(curves[1]);
    }

    return true;
}

///
/// @brief 设置校准状态
///
///
/// @par History:
/// @li 8090/YeHuaNing，2022年11月29日，新建函数
///
void ImQCaliBrateResult::InitCaliStatus()
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
/// @li 8090/YeHuaNing，2022年11月29日，新建函数
///
boost::optional<QStringList> ImQCaliBrateResult::GetCaliStatusShow(im::tf::ReagentInfoTable& reagent, std::shared_ptr<im::tf::CaliCurve>& curve)
{
    if (!curve)
    {
        return boost::none;
    }
    auto spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(curve->assayCode);
    if (spAssay == nullptr)
    {
        return boost::none;
    }

    // 1:项目名称
    QStringList statusList;
    statusList += QString::fromStdString(spAssay->assayName);

    // 2:单位
    QString curUnit = "";
    for (const tf::AssayUnit& unit : spAssay->units)
    {
        if (unit.isCurrent)
        {
			curUnit = QString::fromStdString(unit.name);
            break;
        }
    }
    statusList += curUnit;

	QString strConc = tr("浓度") + "(" + curUnit + ")";
	ui->label_conc->setText(strConc);
	//ui->label_rlu->setText(tr("信号值"));

    // 3:试剂批号
    statusList += QString::fromStdString(curve->reagentLot);
    // 4:试剂瓶号
    statusList += QString::fromStdString(curve->reagentSN);
    // 模块
    statusList += QString::fromStdString(CommonInformationManager::GetInstance()->GetDeviceName(curve->deviceSN));
    // 5:校准品批号
    statusList += QString::fromStdString(curve->calibratorLot);
    // 6:校准时间
    statusList += QString::fromStdString(curve->caliExcuteDate);
    // 7:数据报警 - bug22450
    statusList += QString::fromStdString(curve->alarmInfo);

    return boost::make_optional(statusList);
}


///
/// @brief 响应导出按钮
///
/// @par History:
/// @li 6889/ChenWei，2023年3月30日，新建函数
///
void ImQCaliBrateResult::OnExportBtnClicked()
{
    // 获取数据
    CaliResultInfo Info;
    if (!GetPrintExportInfo(Info, false))
    {
        ULOG(LOG_ERROR, "Can not get  Calibrate Result");
        return;
    }

    // 弹出保存文件对话框
    QString strFilepath = QFileDialog::getSaveFileName(this, tr("保存为..."), QString(), tr("EXCEL files (*.xlsx);;CSV files (*.csv);;PDF files (*.pdf)"));
    if (strFilepath.isEmpty())
    {
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
        QString strUserTmplatePath = strDirPath + "/resource/exportTemplate/ExportCalibratorResult.lrxml";
        int irect = printcom::printcom_async_assign_export(strInfo, strFilepath.toStdString(), strUserTmplatePath.toStdString(), unique_id);
        ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);
        bRect = true;
    }
    else
    {
        std::shared_ptr<FileExporter> pFileEpt(new FileExporter());
        bRect = pFileEpt->ExportCaliInfo(Info, strFilepath);
    }

    // 弹框提示导出
    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(bRect ? tr("导出完成!") : tr("导出失败！")));
    pTipDlg->exec();
}

///
/// @brief 响应打印按钮
///
/// @par History:
/// @li 6889/ChenWei，2023年3月30日，新建函数
///
void ImQCaliBrateResult::OnPrintBtnClicked()
{
    // 转换数据
    CaliResultInfo Info;
    if (!GetPrintExportInfo(Info, true))
    {
        ULOG(LOG_ERROR, "Can not get  Calibrate Result");
        return;
    }

    // 调用打印接口
    std::string strInfo = GetJsonString(Info);
    ULOG(LOG_INFO, "Print datas : %s", strInfo);
    std::string unique_id = " ";
    int irect = printcom::printcom_async_print(strInfo, unique_id);
    ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);

    // 弹框提示打印结果
    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("数据已发送到打印机!")));
    pTipDlg->exec();
} 

///
/// @brief 获取打印导出数据
///
/// @par History:
/// @li 6889/ChenWei，2023年12月19日，新建函数
///
bool ImQCaliBrateResult::GetPrintExportInfo(CaliResultInfo& Info, bool bIsPrint)
{
    std::shared_ptr<tf::UserInfo> pLoginUserInfo = UserInfoManager::GetInstance()->GetLoginUserInfo();
    if (pLoginUserInfo == nullptr)
    {
        ULOG(LOG_ERROR, "Can't get UserInfo.");
        return false;
    }

    bool bGetAllInfo = true;                                    // 根据权限判断是否输出所有数据
    if (pLoginUserInfo->type <= tf::UserType::USER_TYPE_ADMIN)   // 普通用户不能导出全部数据
    {
        bGetAllInfo = false;
    }

    auto curve = m_curCurve;
    auto assayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(m_reagent.assayCode);
    if (curve == nullptr || assayInfo == nullptr)
    {
        ULOG(LOG_ERROR, "Can not find assay info or no curves");
        return false;
    }

    // 转换数据
    QString strPrintTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    Info.strPrintTime = strPrintTime.toStdString();                                         ///< 打印时间
    Info.strDeviceNum = " ";                                                                ///< 仪器编号（留空 from 张浩楠）
    Info.strSoftVersion = DictionaryQueryManager::GetInstance()->GetSoftwareVersion();      ///< 软件版本号

    if (bIsPrint)
    {
        Info.strName = assayInfo->printName;                                                    ///< 项目名称
    }
    else
    {
        // 导出的时候用简称
        Info.strName = assayInfo->assayName;                                                    ///< 项目名称
    }

    Info.strCalibrateDate = curve->caliExcuteDate;                                          ///< 校准日期
    Info.strModelName = CommonInformationManager::GetDeviceName(m_reagent.deviceSN);                        ///< 模块名
    Info.strReagentLot = curve->reagentLot;                                                 ///< 试剂批号
    Info.strReagentSN = curve->reagentSN;                                                   ///< 试剂瓶号-与界面一致
    Info.strCalibratorLot = curve->calibratorLot;                                           ///< 校准品批号
    Info.strCalibratorPos = curve->sampPoses;                                                            ///< 校准品位置
    if (curve->__isset.supplyInfos)
    {
        // bug20086 显示为底物B，应该为底物A的信息
        Info.strSubstrateLot = curve->supplyInfos.substrateABatchNo;    ///< 底物批号
        Info.strSubstrateSN = curve->supplyInfos.substrateASerialNo;    ///< 底物瓶号
        Info.strCleanFluidLot = curve->supplyInfos.cleanFluidBatchNo;   ///< 清洗液批号(缓冲)
        Info.strCleanFluidSN = curve->supplyInfos.cleanFluidSerialNo;    ///< 清洗液编号(缓冲)
        Info.strCupLot = curve->supplyInfos.assayCupBatchNo;             ///< 反应杯批号
        Info.strCupSN = curve->supplyInfos.assayCupSerialNo;             ///< 反应杯编号
    }

    auto caliJudge = curve->caliJudgeInfo;
    Info.strCutoff = " ";                  ///< cutoff值（定量项目留空）
    if (curve->caliType == 0 && caliJudge.__isset.cutoff && bGetAllInfo)//定性
    {
        Info.strCutoff = to_string(caliJudge.cutoff);
    }

    Info.strCaliResult = tr("校准成功").toStdString();                  ///< 校准结果
    Info.strAlarm = curve->alarmInfo;                       ///< 报警
    Info.strOperater = " ";                    ///< 检测者

    int index = 0;
    for (const auto& rlu : curve->actualRLU.caliDocRlu)
    {
        CaliRLUInfo cali;
        cali.strCalibrator = QString::number(index + 1).toStdString();    // 校准品序号
        QString strInfo = QString::number(CommonInformationManager::GetInstance()->ToCurrentUnitValue(curve->assayCode, rlu.conc), 'f', 2);
        cali.strConc = strInfo.toStdString();                   ///< 浓度
        strInfo = QString::number(rlu.rlu1, 'f', 0);
        cali.strRLU1 = strInfo.toStdString();                   ///< RLU1
        strInfo = QString::number(rlu.rlu2, 'f', 0);
        cali.strRLU2 = strInfo.toStdString();                   ///< RLU2
        strInfo = QString::number(rlu.meanRlu, 'f', 0);
        cali.strRLU = strInfo.toStdString();                    ///< RLU

        if (bGetAllInfo)
        {
            if (curve->__isset.caliJudgeInfo && caliJudge.cv.size() > index && caliJudge.k.size() > 0 && caliJudge.delta.size() > 1)
            {
                strInfo = QString::number(caliJudge.cv[index], 'f', 4);///< CV
                cali.strCV = strInfo.toStdString();
                strInfo = QString::number(caliJudge.delta[0], 'f', 4);///< K1
                cali.strK1 = strInfo.toStdString();
                strInfo = QString::number(caliJudge.delta[1], 'f', 4);///< K2
                cali.strK2 = strInfo.toStdString();
                strInfo = QString::number(caliJudge.k[0], 'f', 4);///< K
                cali.strK = strInfo.toStdString();
            }
        }

        Info.vecCaliRLUInfo.push_back(cali);
        index++;
    }

    if (bGetAllInfo)
    {
        int i = 0;
        for (const auto& rlu : curve->actualRLU.caliCurveRlu)
        {
            CaliResultData caliData;
            caliData.strLevel = "L" + std::to_string(++i);                // 校准品水平

            // 信号值
            caliData.strSign = QString::number(rlu.rlu, 'f', 0).toStdString();

            // 浓度值
            caliData.strConc = QString::number(CommonInformationManager::GetInstance()->ToCurrentUnitValue(curve->assayCode, rlu.conc), 'f', 2).toStdString();

            Info.vecCaliResultData.push_back(caliData);
        }
    }

    // 获取校准曲线
    QImage printImg(ui->curve_widget->size(), QImage::Format_RGB32);
    ui->curve_widget->render(&printImg);

    QByteArray ba;
    QBuffer buff(&ba);
    buff.open(QIODevice::WriteOnly);
    printImg.save(&buff, "PNG");
    QString strPrintImg(ba.toBase64());
    Info.strCaliCurveImage = strPrintImg.toStdString();

    return true;
}

///
/// @bref
///		权限变化响应
///
/// @par History:
/// @li 8276/huchunli, 2023年9月22日，新建函数
///
void ImQCaliBrateResult::OnPermisionChanged()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    std::shared_ptr<UserInfoManager> userPms = UserInfoManager::GetInstance();

    // 手动更新校准曲线的权限限制
    userPms->IsPermisson(PMS_CALI_RESULT_MANUAL_UPDATE_CUR) ? ui->update_btn->show() : ui->update_btn->hide();

    // 导出
    userPms->IsPermisson(PSM_IM_PRINTEXPORT_CALIRESULT) ? ui->CaliExportBtn->show() : ui->CaliExportBtn->hide();
}

///
/// @brief  响应校准完成
///
/// @param[in]  cv  曲线信息
///
/// @par History:
/// @li 1226/zhangjing，2023年10月31日，新建函数
///
void ImQCaliBrateResult::OnCaliEnd(const im::tf::CaliCurve cv)
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 同项目同批次需要进行更新-bug22586
    if (cv.failedFlag == 0 && m_reagent.assayCode == cv.assayCode && m_reagent.reagentLot == cv.reagentLot)
    {
        initShow();
    }
}
