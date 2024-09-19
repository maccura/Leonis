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
/// @file     dbgparamfittingwidget.cpp
/// @brief    应用界面->工程师调试->参数（免疫）拟合
///
/// @author   4170/TangChuXian
/// @date     2020年8月24日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2020年8月24日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "dbgparamfittingwidget.h"
#include "ui_dbgparamfittingwidget.h"
#include <map>
#include <QVBoxLayout>
#include <QStandardItemModel>
#include "shared/uidef.h"
#include "shared/tipdlg.h"
#include "shared/uicommon.h"
#include "UtilityCommonFunctions.h"
#include "src/common/Mlog/mlog.h"

#define  DEFAULT_EMPTY_ROW_CNT 10 // 默认留10行空行

// 当发生错误时，弹出消息框后退出当前函数
#define OnErr(bErr, msg) \
if ((bErr))\
{\
	TipDlg(msg).exec();\
	return false;\
}

DbgParamFittingWidget::DbgParamFittingWidget(QWidget *parent)
    : QWidget(parent),
      m_bInit(false)
{
    ui = new Ui::DbgParamFittingWidget();
    ui->setupUi(this);

    m_mainCurvPtModel = new QStandardItemModel();
    ui->ConcSgTbl->setModel(m_mainCurvPtModel);
    m_sengModel = new QStandardItemModel();
    ui->StartEndTbl->setModel(m_sengModel);

    m_btnGroup = new QButtonGroup(this);
    m_btnGroup->setExclusive(true);
    m_btnGroup->addButton(ui->QuantityCaliRB);
    m_btnGroup->addButton(ui->QualityCaliRB);

    // 设置表头
    m_mainCurvPtModel->setHorizontalHeaderLabels({ tr("序号"), tr("浓度"), tr("信号值") });
    m_sengModel->setHorizontalHeaderLabels({ tr("序号"), tr("起始点"), tr("终止点") , tr("拟合公式") });
    
    m_mapFitType = {
        { im::FitType::IMM_FIT_LINEAR, tr("线性拟合") },
        { im::FitType::IMM_FIT_LOGITLOG3, tr("logitlog3拟合") },
        { im::FitType::IMM_FIT_LOGITLOG4, tr("logitlog4拟合") },
        { im::FitType::IMM_FIT_LOGITLOG5, tr("logitlog5拟合") },
        { im::FitType::IMM_FIT_EXPONENT5, tr("exponent5拟合（指数）") },
        { im::FitType::IMM_FIT_QUADRATIC, tr("二次多项式拟合") },
        { im::FitType::IMM_FIT_CUBIC, tr("三次多项式拟合") },
        { im::FitType::IMM_FIT_POLY4, tr("四次多项式") },
        { im::FitType::IMM_FIT_POLY5, tr("五次多项式") },
        { im::FitType::IMM_FIT_POLY6, tr("六次多项式") },
        { im::FitType::IMM_FIT_POW, tr("幂函数拟合") },
        { im::FitType::IMM_FIT_LN, tr("对数拟合") },
        { im::FitType::IMM_INT_SPLINE, tr("样条法") },
        { im::FitType::IMM_INT_AKIMA, tr("Akima样条法") },
        { im::FitType::IMM_INT_PCHIP, tr("PCHIP") }
    };

    // 设置事件捕获
    ui->LotEdit->installEventFilter(this);
}

DbgParamFittingWidget::~DbgParamFittingWidget()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    delete ui;
}

void DbgParamFittingWidget::LoadAnalysisParam(const utilcomm::CaliCurvParam& curvParm, const std::string& curLot)
{
    // 清空界面
    CleanUi();

    // 批号
    ui->LotEdit->setText(QString::fromStdString(curLot));

    // 是否是定量分析
    bool isQuantitativeAnalsysis = (curvParm.isQuantita); // 0:定性， 1：定量
    ui->QuantityCaliRB->setChecked(isQuantitativeAnalsysis);
    ui->QualityCaliRB->setChecked(!isQuantitativeAnalsysis);
    ui->QualityFrame->setEnabled(!isQuantitativeAnalsysis); // 是否使能定性分析的控件

    // 主标曲点数
    ui->MajorCurvePtCntEdit->setText(curvParm.calibratePoints == nullptr ? "" : QString::number(curvParm.calibratePoints->size()));
    AssignMasterCurvPointTb(curvParm.calibratePoints);
    // 分段数
    ui->SegCntEdit->setText(curvParm.fit2Segment == nullptr ? "" : QString::number(curvParm.fit2Segment->size()));
    AssignSegmentTb(curvParm.fit2Segment);

    // cutoff 参数
    if (curvParm.cutoff != nullptr)
    {
        ui->FormulaVal1Edit->setText(QString::number(curvParm.cutoff->A1));
        ui->FormulaVal2Edit->setText(QString::number(curvParm.cutoff->A2));
        ui->FormulaVal3Edit->setText(QString::number(curvParm.cutoff->B1));
        ui->FormulaVal4Edit->setText(QString::number(curvParm.cutoff->B2));
        ui->FormulaVal5Edit->setText(QString::number(curvParm.cutoff->C));

        // 阴阳性判断
        QString strPos = (curvParm.cutoff->withUpperEqual ? ">=" : ">") + QString::number(curvParm.cutoff->upper);
        QString strNeg = (curvParm.cutoff->withLowerEqual ? "<=" : "<") + QString::number(curvParm.cutoff->lower);
        if (!curvParm.cutoff->upperSidePositive)
        {
            std::swap(strPos, strNeg);
        }
        ui->Judge1CndEdit->setText(strPos);
        ui->Judge2CndEdit->setText(strNeg);
    }
}

void DbgParamFittingWidget::SetCaliLotsList(const std::vector<std::string>& vecLots)
{
    m_vecLots = vecLots;
}

bool DbgParamFittingWidget::TakeCaliFittingInfo(utilcomm::CaliCurvParam& curvParam)
{
    // 是否是定量 
    curvParam.isQuantita = ui->QuantityCaliRB->isChecked();

    // 浓度-信号值
    std::shared_ptr<im::CalibratePoints> tempPoints(new im::CalibratePoints());
    int iCaliPt = m_mainCurvPtModel->rowCount();
    for (int i=0; i<iCaliPt; ++i)
    {
        QStandardItem* pItemConc = m_mainCurvPtModel->item(i, 1);
        if (pItemConc == Q_NULLPTR || pItemConc->text().isEmpty())
        {
            break;
        }
        QStandardItem* pItemSig = m_mainCurvPtModel->item(i, 2);
        double dSig = (pItemSig == Q_NULLPTR) ? 0 : pItemSig->text().toDouble();
        tempPoints->push_back(std::pair<double, double>(pItemConc->text().toDouble(), dSig));
    }
    curvParam.calibratePoints = tempPoints;

    // 段信息，拟合公式
    std::shared_ptr<im::Fit2Segment> lstPt(new im::Fit2Segment());
    int iSegCnt = m_sengModel->rowCount();
    for (int i=0; i<iSegCnt; ++i)
    {
        im::SegmentFitElement tempSeg;
        QStandardItem* pItemStart = m_sengModel->item(i, 1);
        if (pItemStart == Q_NULLPTR || pItemStart->text().isEmpty())
        {
            break;
        }
        tempSeg.startEnd.start = pItemStart->text().toUInt();

        QStandardItem* pItemStop = m_sengModel->item(i, 2);
        tempSeg.startEnd.end = pItemStop == Q_NULLPTR ? 0 : pItemStop->text().toUInt();

        QStandardItem* pItemFit = m_sengModel->item(i, 3);
        if (pItemFit != Q_NULLPTR)
        {
            QString strFit = pItemFit->text();
            std::map<im::FitType, QString>::iterator it = m_mapFitType.begin();
            for (; it != m_mapFitType.end(); ++it)
            {
                if (it->second == strFit)
                {
                    tempSeg.fitType = it->first;
                    break;
                }
            }
        }
        lstPt->push_back(tempSeg);
    }
    curvParam.fit2Segment = lstPt;

    // 定性信息
    std::shared_ptr<im::CutOffParams> cutoff(new im::CutOffParams());
    cutoff->A1 = ui->FormulaVal1Edit->text().toDouble();
    cutoff->A2 = ui->FormulaVal2Edit->text().toDouble();
    cutoff->B1 = ui->FormulaVal3Edit->text().toDouble();
    cutoff->B2 = ui->FormulaVal4Edit->text().toDouble();
    cutoff->C = ui->FormulaVal5Edit->text().toDouble();

    QString strPositive = ui->Judge1CndEdit->text();
    cutoff->upperSidePositive = (strPositive.indexOf(">") >= 0);
    cutoff->withUpperEqual = (strPositive.indexOf("=") >= 0);
    QString strPositiveNum = cutoff->withUpperEqual ? strPositive.right(strPositive.size() - 2) : strPositive.right(strPositive.size() - 1);
    cutoff->upper = strPositiveNum.toDouble();

    QString strNegtive = ui->Judge2CndEdit->text();
    cutoff->withLowerEqual = (strNegtive.indexOf("=") >= 0);
    QString strNegtiveNum = cutoff->withLowerEqual ? strNegtive.right(strNegtive.size() - 2) : strNegtive.right(strNegtive.size() - 1);
    cutoff->lower = strNegtiveNum.toDouble();

    curvParam.cutoff = cutoff;

    return true;
}

///
/// @brief 界面显示后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2020年8月24日，新建函数
///
void DbgParamFittingWidget::InitAfterShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 表头
    ui->ConcSgTbl->verticalHeader()->setVisible(false);
    ui->StartEndTbl->verticalHeader()->setVisible(false);

    // 设置表格选中模式为行选中，不可多选
    ui->ConcSgTbl->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->ConcSgTbl->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->StartEndTbl->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->StartEndTbl->setSelectionMode(QAbstractItemView::ExtendedSelection);

    // 列宽自适应
    ResizeTblColToContent(ui->ConcSgTbl);
    ResizeTblColToContent(ui->StartEndTbl);
}

///
/// @brief 窗口显示事件
///
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 4170/TangChuXian，2020年8月24日，新建函数
///
void DbgParamFittingWidget::showEvent(QShowEvent *event)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 让基类处理事件
    QWidget::showEvent(event);

    // 第一次显示时初始化
    if (!m_bInit)
    {
        m_bInit = true;
        InitAfterShow();
    }
}

bool DbgParamFittingWidget::eventFilter(QObject* target, QEvent* event)
{
    if (target == ui->LotEdit)
    {
        if (event->type() == QEvent::MouseButtonDblClick)
        {
            QStringList lotList;
            for (const std::string& str : m_vecLots)
            {
                lotList.append(QString::fromStdString(str));
            }
            if (lotList.length() > 1)
            {
                SelectListDlg pDg(this, lotList);
                pDg.exec();
                m_funSetCaliLot(pDg.GetSelected());
            }
            return true;
        }
    }

    return QWidget::eventFilter(target, event);
}

void DbgParamFittingWidget::AssignMasterCurvPointTb(const std::shared_ptr<im::CalibratePoints>& lstPt)
{
    m_mainCurvPtModel->removeRows(0, m_mainCurvPtModel->rowCount());

    // 点信息
    if (lstPt != nullptr)
    {
        int rowId = 0;
        for (const auto& pt : *lstPt)
        {
            m_mainCurvPtModel->appendRow({
                new QStandardItem(QString::number(rowId)),
                new QStandardItem(QString::number(pt.first)),
                new QStandardItem(QString::number(pt.second)) });
            rowId++;
        }
    }
    // 增加10行空行在末尾
    for (int i = 0; i < DEFAULT_EMPTY_ROW_CNT; ++i)
    {
        m_mainCurvPtModel->appendRow({ new QStandardItem(""), new QStandardItem(""), new QStandardItem("") });
    }
}

void DbgParamFittingWidget::AssignSegmentTb(const std::shared_ptr<im::Fit2Segment>& lstPt)
{
    m_sengModel->removeRows(0, m_sengModel->rowCount());

    // 段信息
    if (lstPt != nullptr)
    {
        int rowId = 0;
        for (const auto& seg : *lstPt)
        {
            QString strFitWay;
            std::map<im::FitType, QString>::iterator fIt = m_mapFitType.find(seg.fitType);
            if (fIt != m_mapFitType.end())
            {
                strFitWay = fIt->second;
            }

            m_sengModel->appendRow({
                new QStandardItem(QString::number(rowId)),
                new QStandardItem(QString::number(seg.startEnd.start)),
                new QStandardItem(QString::number(seg.startEnd.end)),
                new QStandardItem(strFitWay)
            });
            rowId++;
        }
    }
    // 增加10行空行在末尾
    for (int i = 0; i < DEFAULT_EMPTY_ROW_CNT; ++i)
    {
        m_sengModel->appendRow({ new QStandardItem(""), new QStandardItem(""),
            new QStandardItem(""),new QStandardItem("") });
    }
}

void DbgParamFittingWidget::CleanUi()
{
    ui->LotEdit->clear();
    ui->QuantityCaliRB->setChecked(false);
    ui->QualityCaliRB->setChecked(false);

    ui->MajorCurvePtCntEdit->clear();
    AssignMasterCurvPointTb(nullptr);
    ui->SegCntEdit->clear();
    AssignSegmentTb(nullptr);

    ui->FormulaVal1Edit->clear();
    ui->FormulaVal2Edit->clear();
    ui->FormulaVal3Edit->clear();
    ui->FormulaVal4Edit->clear();
    ui->FormulaVal5Edit->clear();
    ui->Judge1CndEdit->clear();
    ui->Judge2CndEdit->clear();
}

SelectListDlg::SelectListDlg(QWidget* prant, const QStringList& contents)
    : QDialog(prant)
{
    QVBoxLayout *pV = new QVBoxLayout(this);
    QListWidget* pList = new QListWidget(this);
    pList->addItems(contents);
    pV->addWidget(pList);

    this->setLayout(pV);
    this->adjustSize();

    connect(pList, SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(ItemDoubleClick(QListWidgetItem *)));
}

void SelectListDlg::ItemDoubleClick(QListWidgetItem *item)
{
    m_selectString = item->text();
    close();
}
