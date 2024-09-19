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

#include "assaydbgparamtabwidget.h"
#include "dbgparamanalysiswidget.h"
#include "dbgparamprehandlewidget.h" 
#include "dbgparamfittingwidget.h"
#include "dbgparamcaliwidget.h"
#include "UtilityCommonFunctions.h"

#include "thrift/im/i6000/I6000LogicControlProxy.h"
#include "shared/tipdlg.h"
#include "src/common/defs.h"
#include "src/common/Mlog/mlog.h"


// 当发生错误时，弹出消息框后退出当前函数
#define OnErr(bErr, msg) \
if ((bErr))\
{\
	TipDlg(msg).exec();\
	return false;\
}

#define ADD_TAB(tabType, tabWidget) do {\
    TabItem tb(tabType, tabWidget);\
    addTab(tabWidget, tb.GetName());\
    m_tabInfo.push_back(tb);\
} while (false);


AssayDbgParamTabWidget::AssayDbgParamTabWidget(QWidget *parent)
    : QTabWidget(parent),
    m_bInit(false)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    m_caliCurvCache = std::shared_ptr<utilcomm::CaliCurvCache>(new utilcomm::CaliCurvCache());
    m_origCurvParam = nullptr;
    m_existPrehandleAssay = false;

    m_pPrehandleWidget = new DbgParamPrehandleWidget(this);
	m_pAnalysisWidget = new DbgParamAnalysisWidget(this);
    m_pFittingWidget = new DbgParamFittingWidget(this);
    m_pFittingWidget->BindSetCaliLotCallBack(std::bind(&AssayDbgParamTabWidget::CaliLotChanged, this, std::placeholders::_1));
    m_pCaliWidget = new DbgParamCaliWidget(this);

	connect(this, SIGNAL(currentChanged(int)), this, SLOT(OnCurrentChanged(int)));
}

AssayDbgParamTabWidget::~AssayDbgParamTabWidget()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
}

void AssayDbgParamTabWidget::LoadAssayInfo(int assayCode)
{
    m_pAnalysisWidget->LoadAnalysisParam(assayCode);    
    
    // 预处理页签的处理
    if (DbgParamPrehandleWidget::IsExistPreAssay(assayCode))
    {
        QWidget *curTab = this->widget(0);
        if (curTab != Q_NULLPTR)
        {
            if (curTab->objectName() != "DbgParamPrehandleWidget")
            {
                this->insertTab(0, m_pPrehandleWidget, tr("预处理"));
            }
        }
        m_pPrehandleWidget->LoadAnalysisParam(assayCode);
        m_existPrehandleAssay = true;
    }
    else
    {
        // 移除预处理页签
        for (int ti = 0; ti < this->count(); ++ti)
        {
            QWidget *curTab = this->widget(ti);
            if (curTab != Q_NULLPTR)
            {
                if (curTab->objectName() == "DbgParamPrehandleWidget")
                {
                    this->removeTab(ti);
                    break;
                }
            }
        }
        m_existPrehandleAssay = false;
    }

    // 初始化校准曲线缓存
    m_caliCurvCache->Init(assayCode);
    std::string strLot;
    m_caliCurvCache->GetLateastCurve(m_origCurv, strLot);

    utilcomm::CaliCurvParam caliParams;
    if (m_origCurv != nullptr)
    {
        utilcomm::ParseCaliJson(m_origCurv->caliCurveInfo, caliParams);
        m_origCurvParam = std::make_shared<utilcomm::CaliCurvParam>(caliParams);
    }

    // 加载校准信息
    m_pCaliWidget->LoadAnalysisParam(caliParams);

    // 加载拟合信息 
    std::vector<std::string> vecLots;
    m_caliCurvCache->GetLotList(vecLots);
    m_pFittingWidget->SetCaliLotsList(vecLots);
    m_pFittingWidget->LoadAnalysisParam(caliParams, strLot);
}

void AssayDbgParamTabWidget::SaveAssayInfo()
{
    QStringList strNoticeSucc;
    QStringList strNoticeFail;
    if (m_pAnalysisWidget->SaveAnalysisInfo())
    {
        strNoticeSucc.append(tr("分析参数"));
    }
    else
    {
        strNoticeFail.append(tr("分析参数"));
    }

    if (m_existPrehandleAssay)
    {
        if (m_pPrehandleWidget->SavePrehandleInfo())
        {
            strNoticeSucc.append(tr("预处理参数"));
        }
        else
        {
            strNoticeFail.append(tr("预处理参数"));
        }
    }

    utilcomm::CaliCurvParam uiCaliParam;
    m_pFittingWidget->TakeCaliFittingInfo(uiCaliParam);
    m_pCaliWidget->TakeCaliInfo(uiCaliParam);

    if (IsCaliCurvChanged(*m_origCurvParam, uiCaliParam))
    {
        // 保存修改
        utilcomm::ModifyCaliJson(m_origCurv->caliCurveInfo, uiCaliParam);
        if (::im::i6000::LogicControlProxy::ModifyCaliCurve(*m_origCurv))
        {
            strNoticeSucc.append(tr("校准曲线"));
        }
        else
        {
            ULOG(LOG_ERROR, "Failed to save cali curve from ui.");
            strNoticeFail.append(tr("校准曲线"));
        }
    }

    // 弹框提示
    QString strNotice;
    if (strNoticeSucc.length() > 0 && strNoticeFail.length() > 0)
    {
        strNotice += strNoticeSucc.join("、");
        strNotice += tr("保存成功;\n");
        strNotice += strNoticeFail.join("、");
        strNotice += tr("保存失败;\n");
    }
    else if (strNoticeSucc.length() > 0)
    {
        strNotice = tr("保存成功！");
    }
    else if (strNoticeFail.length() > 0)
    {
        strNotice = tr("保存失败！");
    }
    TipDlg(strNotice).exec();
}

void AssayDbgParamTabWidget::InitAfterShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    this->clear();
    m_tabInfo.clear();

    // 添加Tab页
    ADD_TAB(TabPreHandle, m_pPrehandleWidget);      // 预处理
    ADD_TAB(TabAnalysis, m_pAnalysisWidget);        // 分析
    ADD_TAB(TabFitting, m_pFittingWidget);          // 拟合
    ADD_TAB(TabCali, m_pCaliWidget);                // 校准
}

void AssayDbgParamTabWidget::showEvent(QShowEvent *event)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 让基类处理事件
    QTabWidget::showEvent(event);

    // 第一次显示时初始化
    if (!m_bInit)
    {
        m_bInit = true;
        InitAfterShow();
    }
}

void AssayDbgParamTabWidget::OnCurrentChanged(int index)
{
}

void AssayDbgParamTabWidget::CaliLotChanged(const QString& strLot)
{
    if (strLot.isEmpty())
    {
        return;
    }

    std::string slot = strLot.toStdString();
    std::shared_ptr<::im::tf::CaliCurve> pCur = m_caliCurvCache->GetCurveByLot(slot);
    utilcomm::CaliCurvParam caliParams;
    if (pCur != nullptr)
    {
        utilcomm::ParseCaliJson(pCur->caliCurveInfo, caliParams);
        m_origCurvParam = std::make_shared<utilcomm::CaliCurvParam>(caliParams);
    }

    // 加载校准信息
    m_pCaliWidget->LoadAnalysisParam(caliParams);

    // 加载拟合信息
    m_pFittingWidget->LoadAnalysisParam(caliParams, slot);
}

QString AssayDbgParamTabWidget::TabItem::GetName() const
{
    switch (m_tbType)
    {
    case AssayDbgParamTabWidget::TabPreHandle:
        return tr("预处理");
    case AssayDbgParamTabWidget::TabAnalysis:
        return tr("分析");
    case AssayDbgParamTabWidget::TabFitting:
        return tr("拟合");
    case AssayDbgParamTabWidget::TabCali:
        return tr("校准");
    default:
        break;
    }

    return "";
}

bool AssayDbgParamTabWidget::IsCaliCurvChanged(const utilcomm::CaliCurvParam& uiParam, const utilcomm::CaliCurvParam& origCaliCurv)
{
    if (uiParam.isQuantita != origCaliCurv.isQuantita)
    {
        return true;
    }
    if (uiParam.calibratePoints != nullptr && origCaliCurv.calibratePoints != nullptr)
    {
        if (uiParam.calibratePoints->size() != origCaliCurv.calibratePoints->size())
        {
            return true;
        }
        for (int i = 0; i < uiParam.calibratePoints->size(); ++i)
        {
            if (abs((*(uiParam.calibratePoints))[i].first - (*(origCaliCurv.calibratePoints))[i].first) > PRECESION_VALUE ||
                abs((*(uiParam.calibratePoints))[i].second - (*(origCaliCurv.calibratePoints))[i].second) > PRECESION_VALUE)
            {
                return true;
            }
        }
    }
    if (uiParam.cutoff != nullptr && origCaliCurv.cutoff != nullptr)
    {
        const std::shared_ptr<im::CutOffParams>& uiC = uiParam.cutoff;
        const std::shared_ptr<im::CutOffParams>& ogC = origCaliCurv.cutoff;
        if (uiC->upperSidePositive != ogC->upperSidePositive ||
            uiC->withLowerEqual != ogC->withLowerEqual ||
            uiC->withUpperEqual != ogC->withUpperEqual ||
            abs(uiC->A1 - ogC->A1) > PRECESION_VALUE ||
            abs(uiC->A2 - ogC->A2) > PRECESION_VALUE ||
            abs(uiC->B1 - ogC->B1) > PRECESION_VALUE ||
            abs(uiC->B2 - ogC->B2) > PRECESION_VALUE ||
            abs(uiC->C - ogC->C) > PRECESION_VALUE ||
            abs(uiC->lower - ogC->lower) > PRECESION_VALUE ||
            abs(uiC->upper - ogC->upper) > PRECESION_VALUE)
        {
            return true;
        }
    }
    if (uiParam.fit2Segment != nullptr && origCaliCurv.fit2Segment != nullptr)
    {
        if (uiParam.fit2Segment->size() != origCaliCurv.fit2Segment->size())
        {
            return true;
        }
        for (int i = 0; i < uiParam.fit2Segment->size(); ++i)
        {
            const im::SegmentFitElement& uiS = (*(uiParam.fit2Segment))[i];
            const im::SegmentFitElement& ogS = (*(origCaliCurv.fit2Segment))[i];
            if (uiS.fitType != ogS.fitType ||
                uiS.startEnd.start != ogS.startEnd.start ||
                uiS.startEnd.end != ogS.startEnd.end)
            {
                return true;
            }
        }
    }
    if (uiParam.spanFix2Segment != nullptr && origCaliCurv.spanFix2Segment != nullptr)
    {
        if (uiParam.spanFix2Segment->size() != origCaliCurv.spanFix2Segment->size())
        {
            return true;
        }
        for (int i = 0; i < uiParam.spanFix2Segment->size(); ++i)
        {
            const im::SegmentSpanElement& uiE = (*(uiParam.spanFix2Segment))[i];
            const im::SegmentSpanElement& ogE = (*(origCaliCurv.spanFix2Segment))[i];
            if (uiE.spanFixType != ogE.spanFixType ||
                uiE.startEnd.start != ogE.startEnd.start ||
                uiE.startEnd.end != ogE.startEnd.end ||
                uiE.pointIndex.size() != ogE.pointIndex.size())
            {
                return true;
            }
            for (int pi = 0; pi < uiE.pointIndex.size(); ++pi)
            {
                if (uiE.pointIndex[pi] != ogE.pointIndex[pi])
                {
                    return true;
                }
            }
        }
    }

    return false;
}
