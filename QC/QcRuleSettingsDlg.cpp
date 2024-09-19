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
/// @file     QcRuleSettingsDlg.cpp
/// @brief    质控规则设置界面
///
/// @author   8276/huchunli
/// @date     2022年12月23日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8276/huchunli，2022年12月23日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QcRuleSettingsDlg.h"
#include "ui_QcRuleSettingsDlg.h"
#include "QHeaderView"
#include <QStylePainter>
#include <QStandardItemModel>

#include "shared/CommonInformationManager.h"
#include "shared/tipdlg.h"
#include "shared/uicommon.h"
#include "shared/QComDelegate.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/DataManagerQc.h"

#include "QC/QcRules.h"
#include "src/thrift/ch/gen-cpp/ch_constants.h"
#include "src/thrift/ise/gen-cpp/ise_constants.h"
#include "src/common/Mlog/mlog.h"

Q_DECLARE_METATYPE(std::shared_ptr<QcRuleItem>)
Q_DECLARE_METATYPE(tf::GeneralAssayInfo)
Q_DECLARE_METATYPE(tf::QcDoc)


#define COL_RULE_SELECTION  3 // 质控规则列表的选择列序号
#define COL_RULE_STATUS     2 // 状态列（标记为告警或是失控）

QcRuleSettingsDlg::QcRuleSettingsDlg(QWidget *parent)
    : BaseDlg(parent)
{
    ui = new Ui::QcRuleSettingsDlg();
    ui->setupUi(this);
    m_currentRuleList = nullptr;
    m_bInit = false;

    this->SetTitleName(tr("失控规则"));

    // 项目列表
    QStringList qHeader = { tr("序号"), tr("项目") };
    ui->tableWidget->setHorizontalHeaderLabels(qHeader);
    ui->tableWidget->setColumnCount(2);
    ui->tableWidget->setHorizontalHeaderLabels(qHeader);
    ui->tableWidget->verticalHeader()->setVisible(false);
    ui->tableWidget->setColumnWidth(0, 70);
    ui->tableWidget->setColumnWidth(1, 160);
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->horizontalHeader()->setHighlightSections(false);

    ui->tabWidget_ruleSet->setTabText(0, tr("失控规则设置"));
    ui->tabWidget_ruleSet->setTabText(1, tr("联合质控设置"));

    // 规则列表
    QStringList qheaderRules = { tr("规则"), tr("规则说明"), tr("标记状态"), tr("选择") };
    m_model = new QStandardItemModel();
    m_model->setHorizontalHeaderLabels(qheaderRules);
    ui->tableView_rules->setModel(m_model);

    ui->tableView_rules->verticalHeader()->setVisible(false);
    ui->tableView_rules->setColumnWidth(0, 135);
    ui->tableView_rules->setColumnWidth(1, 568);
    ui->tableView_rules->setColumnWidth(2, 160);
    ui->tableView_rules->setColumnWidth(3, 105); 
    ui->tableView_rules->setItemDelegateForColumn(3, new CheckBoxDelegate(this));
    ui->tableView_rules->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView_rules->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView_rules->horizontalHeader()->setHighlightSections(false);

    // 初始化项目列表
    if (InitAssayList())
    {
        InitQcRulesTable();
    }

    // 点击项目
    connect(ui->tableWidget->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QcRuleSettingsDlg::OnAssayTableCellClicked);
    connect(ui->tableView_rules, SIGNAL(clicked(const QModelIndex&)), this, SLOT(OnRuleTableCellClicked(const QModelIndex&)));
    // 保存
    connect(ui->pushButtonSave, SIGNAL(clicked()), this, SLOT(OnSaveSettings()));
    // 取消
    connect(ui->pushButton_Cancel, SIGNAL(clicked()), this, SLOT(OnCancelSettings()));

    connect(ui->tableView_rules->horizontalHeader(), SIGNAL(sectionDoubleClicked(int)), this, SLOT(OnDoubleClickedHeader(int)));

    connect(ui->tabWidget_ruleSet, SIGNAL(currentChanged(int)), this, SLOT(OnChangeTab(int)));

    connect(ui->comboBoxQcX, SIGNAL(currentIndexChanged(int)), this, SLOT(OnUnionQcDocXChanged(int)));
    connect(ui->comboBoxQcY, SIGNAL(currentIndexChanged(int)), this, SLOT(OnUnionQcDocYChanged(int)));

    // 默认选中第一行
    QTableWidgetItem* currItem = ui->tableWidget->item(0, 1);
    if (currItem != nullptr)
    {
        UpdateRuleSelected(currItem->data(Qt::UserRole).value<tf::GeneralAssayInfo>());
    }
}

QcRuleSettingsDlg::~QcRuleSettingsDlg()
{
}

bool QcRuleSettingsDlg::InitAssayList()
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);

    // 获取项目信息
    auto mapAssayInfo = CommonInformationManager::GetInstance()->GetGeneralAssayCodeMap();
    if (mapAssayInfo.empty())
    {
        ULOG(LOG_ERROR, "Empty assay container returned by GetGeneralAssayCodeMap.");
        return false;
    }
    ui->tableWidget->clearContents();
    int rowCnt = 1;
    QTableWidget *tb = ui->tableWidget;
    for (auto it = mapAssayInfo.begin(); it != mapAssayInfo.end(); ++it)
    {
        // 过滤掉血清指数
        if ((it->second->assayClassify == tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY)
            && (it->first >= ch::tf::g_ch_constants.MAX_REAGENT_CODE))
        {
            continue;
        }

        // 过滤ISE组合名
        if ((it->second->assayClassify == tf::AssayClassify::ASSAY_CLASSIFY_ISE) && (it->first == ::ise::tf::g_ise_constants.ASSAY_CODE_ISE))
        {
            continue;
        }

        int rowIdx = rowCnt - 1;
		ui->tableWidget->setRowCount(rowCnt);
        tb->setItem(rowIdx, 0, new QTableWidgetItem(QString::number(rowCnt)));
        tb->setItem(rowIdx, 1, new QTableWidgetItem(QString::fromStdString(it->second->assayName)));

        tb->item(rowIdx, 1)->setData(Qt::UserRole, QVariant::fromValue(*it->second));
        rowCnt++;
    }

    // 居中内容
    SetTblTextAlign(tb, Qt::AlignCenter);
    return true;
}

void QcRuleSettingsDlg::InitQcRulesTable()
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);

    std::vector<std::shared_ptr<QcRuleItem>> qcRuls = QcRules::GetInstance()->GetAllRules();
    std::vector<QString> &qcMarks = QcRules::GetInstance()->GetQcMarkList();

    for (int i = 0; i < qcRuls.size(); ++i)
    {
        QList<QStandardItem*> rowItems = {
            new QStandardItem(qcRuls[i]->m_name),
            new QStandardItem(qcRuls[i]->m_desc),
            new QStandardItem(""),
            new QStandardItem("")
        };
        m_model->appendRow(rowItems);

        // 标记的下拉框
        CenterComboBox *markComb = new CenterComboBox(ui->tableView_rules);
        markComb->installEventFilter(this);
        for (int m = 0; m < qcMarks.size(); ++m)
        {
            markComb->addItem(qcMarks[m]);
        }
        connect(markComb, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(OnStatComboxChanged(const QString&)));
        markComb->setCurrentText(qcRuls[i]->m_mark);
        ui->tableView_rules->setIndexWidget(m_model->index(i, COL_RULE_STATUS), markComb);

        // 缓存当前数据
        m_model->item(i, COL_RULE_SELECTION)->setData(QVariant::fromValue<std::shared_ptr<QcRuleItem>>(qcRuls[i]), Qt::UserRole);
    }
}

void QcRuleSettingsDlg::UpdateUnionQcDocUi(const tf::GeneralAssayInfo& assayInfo)
{
    ULOG(LOG_INFO, "%s, assayCode:%d.", __FUNCTION__, assayInfo.assayCode);

    // 清除联合质控设置界面
    ui->comboBoxQcX->clear();
    ui->comboBoxQcY->clear();

    // 根据assayCode查询对应的质控文档
    std::vector<std::shared_ptr<tf::QcDoc>> vecDocs;
    DataManagerQc::GetInstance()->GetQcDocs(vecDocs, assayInfo.assayCode);
    if (vecDocs.size() == 0)
    {
        ULOG(LOG_ERROR, "Empty QcDoc query by assayCode:%d.", assayInfo.assayCode);
        return;
    }

    // 填充联合质控文档选择的下拉框
    std::map<int64_t, int> qcDocIndex;
    ui->comboBoxQcX->blockSignals(true);
    ui->comboBoxQcY->blockSignals(true);
    ui->comboBoxQcX->addItem("");
    ui->comboBoxQcY->addItem("");
    for (int i = 0; i < vecDocs.size(); ++i)
    {
        // 下拉选择框中以质控品名称+质控品编号+质控品批号的形式显示
        std::shared_ptr<tf::QcDoc> &qcDoc = vecDocs[i];
        if (qcDoc == nullptr)
        {
            continue;
        }
        QVariant temp = QVariant::fromValue(*qcDoc);

        QString qcItem;
        qcItem.sprintf("%s_%s_%s", qcDoc->name.c_str(), qcDoc->sn.c_str(), qcDoc->lot.c_str());
        ui->comboBoxQcX->addItem(qcItem);
        ui->comboBoxQcX->setItemData(i + 1, temp);
        ui->comboBoxQcY->addItem(qcItem);
        ui->comboBoxQcY->setItemData(i + 1, temp);

        qcDocIndex.insert(std::pair<int64_t, int>(qcDoc->id, i + 1));
    }
    ui->comboBoxQcX->blockSignals(false);
    ui->comboBoxQcY->blockSignals(false);

    // 回填X和Y的历史设置的数据
    if (m_currentRuleList->m_qcX != -1)
    {
        std::map<int64_t, int>::iterator it = qcDocIndex.find(m_currentRuleList->m_qcX);
        if (it != qcDocIndex.end())
        {
            ui->comboBoxQcX->setCurrentIndex(it->second);
        }
    }
    if (m_currentRuleList->m_qcY != -1)
    {
        std::map<int64_t, int>::iterator it = qcDocIndex.find(m_currentRuleList->m_qcY);
        if (it != qcDocIndex.end())
        {
            ui->comboBoxQcY->setCurrentIndex(it->second);
        }
    }
}

void QcRuleSettingsDlg::OnAssayTableCellClicked(const QItemSelection& selected, const QItemSelection& deselected)
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);

    QModelIndex qModel = ui->tableWidget->currentIndex();
    if (selected.size() <= 0 || !qModel.isValid())
    {
        ULOG(LOG_ERROR, "Invalid QModelIndex.");
        return;
    }
    QTableWidgetItem* tbItem = ui->tableWidget->item(qModel.row(), 1);
    if (tbItem == nullptr)
    {
        ULOG(LOG_WARN, "Invalid clicked index.");
        return;
    }
    QVariant assayObj = tbItem->data(Qt::UserRole);

    const tf::GeneralAssayInfo &ass = assayObj.value<tf::GeneralAssayInfo>();
    UpdateRuleSelected(ass);

    // 如果当前界面是联合质控设置界面，当更改当前项目时，更新界面
    UpdateUnionQcDocUi(ass);
    // 选中项可见
    ui->tableWidget->scrollTo(qModel);
}

void QcRuleSettingsDlg::UpdateRuleSelected(const tf::GeneralAssayInfo& assayInfo)
{
    ULOG(LOG_INFO, "%s, assayCode:%d.", __FUNCTION__, assayInfo.assayCode);

    if (!assayInfo.__isset.assayClassify)
    {
        ULOG(LOG_ERROR, "Not set assayClassify.");
        return;
    }
    // 检测重入
    std::map<int, std::shared_ptr<AssayRules>>::iterator sIter = m_ruleSelected.find(assayInfo.assayCode);
    if (sIter != m_ruleSelected.end())
    {
        m_currentRuleList = sIter->second;
    }
    else
    {
        // 缓存界面显示的待修改项目的规则数据
        std::shared_ptr<AssayRules> tempRule(new AssayRules());
        tempRule->m_assayInfo = std::move(assayInfo);
        m_ruleSelected.insert(std::pair<int, std::shared_ptr<AssayRules>>(assayInfo.assayCode, tempRule));

        tempRule->m_qcX = assayInfo.qcRules.XQCDocID;
        tempRule->m_qcY = assayInfo.qcRules.YQCDocID;
        for (int i = 0; i < assayInfo.qcRules.listItem.size(); ++i)
        {
            const tf::QcJudgeParamItem& ruleItem = assayInfo.qcRules.listItem[i];
            RuleStat rs(ruleItem.useFlag, ruleItem.ruleStat);
            tempRule->m_assayRules.insert(std::pair<int, RuleStat>(ruleItem.ruleType, rs));
        }

        m_currentRuleList = tempRule;
    }

    // 填写界面状态
    if (m_currentRuleList != nullptr)
    {
        UpdateRuleSelectedUi(m_currentRuleList->m_assayRules);
    }
}

tf::QcStat::type QcRuleSettingsDlg::TransefromQcStat(const QString & strQcStat)
{
    std::vector<QString> qcMarks = QcRules::GetInstance()->GetQcMarkList();
    if (strQcStat == qcMarks[0])
    {
        return tf::QcStat::QC_STAT_WARN;
    }
    return tf::QcStat::QC_STAT_UNCONTROLLABLE;
}

void QcRuleSettingsDlg::UpdateRuleSelectedUi(const std::map<int, RuleStat>& ruleStat)
{
    ULOG(LOG_INFO, "%s, ruleStat size:%d.", __FUNCTION__, ruleStat.size());

    int ruleCnt = QcRules::GetInstance()->GetAllRules().size();
    for (int i = 0; i < ruleCnt; ++i)
    {
        QVariant qValue = m_model->item(i, COL_RULE_SELECTION)->data(Qt::UserRole);
        std::shared_ptr<QcRuleItem> ruleItem = qValue.value<std::shared_ptr<QcRuleItem>>();

        std::map<int, RuleStat>::const_iterator rIter = ruleStat.find(ruleItem->m_id);
        if (ruleItem == nullptr || rIter == ruleStat.end())
        {
            m_model->setData(m_model->index(i, COL_RULE_SELECTION), 0, Qt::UserRole + 1);

            // 设置默认标记
            tf::QcStat::type defaultType = tf::QcStat::QC_STAT_UNCONTROLLABLE;
            if (i == 0)
            {
                defaultType = tf::QcStat::QC_STAT_WARN;
            }
            CheckStat(i, defaultType);
            continue;
        }
        // 显示勾选图标
        m_model->setData(m_model->index(i, COL_RULE_SELECTION), rIter->second.m_selected, Qt::UserRole + 1);

        // 标记修改
        CheckStat(i, rIter->second.m_stat);
    }
}

void QcRuleSettingsDlg::CheckStat(int rowIndex, tf::QcStat::type qcType)
{
    ULOG(LOG_INFO, "%s, rowidx:%d, type:%d.", __FUNCTION__, rowIndex, qcType);

    std::vector<QString> qcMarks = QcRules::GetInstance()->GetQcMarkList();

    // 标记修改
    QWidget* pCurWidget = ui->tableView_rules->indexWidget(m_model->index(rowIndex, COL_RULE_STATUS));
    if (pCurWidget == Q_NULLPTR)
    {
        ULOG(LOG_WARN, "Null widget returned by indexWidget.");
        return;
    }
    QComboBox* comb = (QComboBox*)pCurWidget;
    switch (qcType)
    {
    case tf::QcStat::QC_STAT_WARN:
        comb->setCurrentText(qcMarks[0]); // 告警
        break;
    default:
        comb->setCurrentText(qcMarks[1]); // 失控
        break;
    }
}

bool QcRuleSettingsDlg::FindQcComposition(const tf::QcDoc &qcDoc, int assayCode, tf::QcComposition& composition)
{
    for (int i = 0; i < qcDoc.compositions.size(); ++i)
    {
        if (qcDoc.compositions[i].assayCode == assayCode)
        {
            composition = std::move(qcDoc.compositions[i]);
            return true;
        }
    }
    return false;
}

void QcRuleSettingsDlg::OnRuleTableCellClicked(const QModelIndex & qModel)
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);

    if (!qModel.isValid() || m_currentRuleList == nullptr)
    {
        ULOG(LOG_ERROR, "Invlaid index or current rulelist.");
        return;
    }

    int curRow = qModel.row();

    // 勾选状态列
    if (qModel.column() == COL_RULE_SELECTION)
    {
        ChoiceOneRow(curRow);
    }
}

void QcRuleSettingsDlg::OnStatComboxChanged(const QString& strText)
{
    ULOG(LOG_INFO, "%s, text:%s.", __FUNCTION__, strText.toStdString().c_str());

    if (m_currentRuleList == nullptr)
    {
        ULOG(LOG_WARN, "Null current rulelist.");
        return;
    }
    QComboBox* comb = dynamic_cast<QComboBox*>(this->sender());
    if (comb == nullptr)
    {
        ULOG(LOG_ERROR, "Invalid object from sender.");
        return;
    }
    QRect combRect = comb->frameGeometry();
    QModelIndex combIndex = ui->tableView_rules->indexAt(QPoint(combRect.x(), combRect.y()));

    // 获取状所在规则
    QStandardItem* tbItem = m_model->item(combIndex.row(), COL_RULE_SELECTION);
    if (tbItem == nullptr)
    {
        ULOG(LOG_ERROR, "Failed to get tbItem at(%d, %d).", combIndex.row(), COL_RULE_SELECTION);
        return;
    }
    std::shared_ptr<QcRuleItem> qcRule = tbItem->data(Qt::UserRole).value<std::shared_ptr<QcRuleItem>>();
    if (qcRule == nullptr)
    {
        ULOG(LOG_ERROR, "Invlaid qc rule.");
        return;
    }

    // 缓存规则对应的状态
    tf::QcStat::type currType = TransefromQcStat(strText);
    std::map<int, RuleStat>::iterator rIter = m_currentRuleList->m_assayRules.find(qcRule->m_id);
    if (rIter == m_currentRuleList->m_assayRules.end())
    {
        m_currentRuleList->m_assayRules.insert(std::pair<int, RuleStat>(qcRule->m_id, RuleStat(false, currType)));
    }
    else
    {
        rIter->second.m_stat = currType;
    }
}

void QcRuleSettingsDlg::OnSaveSettings()
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);

    // 判断联合设置的合法性
    for (const auto& item : m_ruleSelected)
    {
        if ((item.second->m_qcX <= 0 && item.second->m_qcY > 0) || \
            (item.second->m_qcX > 0 && item.second->m_qcY <= 0))
        {
            TipDlg msg(tr("保存失败！") + QString::fromStdString(item.second->m_assayInfo.assayName) + tr("联合质控配对设置不完整！"));
            msg.exec();
            return;
        }
    }

    // 开始存储设定的规则
    std::shared_ptr<CommonInformationManager> assayMgr = CommonInformationManager::GetInstance();
    std::map<int, std::shared_ptr<AssayRules>>::iterator ruleIter = m_ruleSelected.begin();
    for (; ruleIter != m_ruleSelected.end(); ++ruleIter)
    {
        tf::QcJudgeParam qcRule;
        qcRule.__set_XQCDocID(ruleIter->second->m_qcX);
        qcRule.__set_YQCDocID(ruleIter->second->m_qcY);

        std::vector<tf::QcJudgeParamItem> ruleList;
        std::map<int, RuleStat>::iterator it = ruleIter->second->m_assayRules.begin();
        for (; it != ruleIter->second->m_assayRules.end(); ++it)
        {
            tf::QcJudgeParamItem qitem;
            qitem.__set_ruleType((tf::QcRuleType::type)it->first);
            qitem.__set_useFlag(it->second.m_selected);
            qitem.__set_ruleStat(it->second.m_stat);
            ruleList.push_back(qitem);
        }
        qcRule.__set_listItem(ruleList);
        assayMgr->UpdateQcRules(ruleIter->first, qcRule);
    }
    close();

    // 发送质控规则更新消息
    POST_MESSAGE(MSG_ID_QC_RULE_UPDATE);
}

void QcRuleSettingsDlg::OnCancelSettings()
{
    m_ruleSelected.clear();
    if (m_currentRuleList != nullptr)
    {
        m_currentRuleList->ClearRules();
        UpdateRuleSelectedUi(m_currentRuleList->m_assayRules);
    }
    close();
}

void QcRuleSettingsDlg::ChoiceOneRow(int curRow)
{
    std::shared_ptr<QcRuleItem> qcRule = m_model->data(m_model->index(curRow, COL_RULE_SELECTION), Qt::UserRole).value<std::shared_ptr<QcRuleItem>>();
    if (qcRule == nullptr)
    {
        ULOG(LOG_ERROR, "Invlaid qc rule.");
        return;
    }
    QWidget* pCurWidget = ui->tableView_rules->indexWidget(m_model->index(curRow, COL_RULE_STATUS));
    if (pCurWidget == Q_NULLPTR)
    {
        ULOG(LOG_WARN, "Null widget.");
        return;
    }
    QComboBox* comb = (QComboBox*)pCurWidget;
    tf::QcStat::type currType = TransefromQcStat(comb->currentText());

    std::map<int, RuleStat> &rlist = m_currentRuleList->m_assayRules;
    std::map<int, RuleStat>::iterator rit = rlist.find(qcRule->m_id);
    if (rit != rlist.end())
    {
        rit->second.m_selected = !rit->second.m_selected;
        rit->second.m_stat = currType;
    }
    else
    {
        rlist.insert(std::pair<int, RuleStat>(qcRule->m_id, RuleStat(true, currType)));
    }

    // 如果没有存在选择，ui没选中则进行选中
    std::map<int, RuleStat>::iterator curIt = rlist.find(qcRule->m_id);
    if (curIt != rlist.end())
    {
        QModelIndex selectIdx = m_model->index(curRow, COL_RULE_SELECTION);
        m_model->setData(selectIdx, curIt->second.m_selected, Qt::UserRole + 1);
    }
}

void QcRuleSettingsDlg::showEvent(QShowEvent *event)
{
    // 第一次显示时初始化
    if (!m_bInit)
    {
        m_bInit = true;
        ui->tableWidget->selectRow(0);
    }
}

bool QcRuleSettingsDlg::eventFilter(QObject *obj, QEvent *event)
{
    if (obj->inherits("QComboBox") && event->type() == QEvent::Wheel)
    {
        return true;
    }

    return BaseDlg::eventFilter(obj, event);
}

void QcRuleSettingsDlg::OnDoubleClickedHeader(int logicalIndex)
{
    // 如果是选择列，双击则全反选
    if (logicalIndex == COL_RULE_SELECTION)
    {
        if (m_currentRuleList == nullptr)
        {
            return;
        }

        int totalRow = m_model->rowCount();
        for (int curRow = 0; curRow < totalRow; ++curRow)
        {
            ChoiceOneRow(curRow);
        }
    }
}

void QcRuleSettingsDlg::OnChangeTab(int sel)
{
    ULOG(LOG_INFO, "%s, sel:%d.", __FUNCTION__, sel);

    // 联合质控品设置tab页
    if (sel == 1 && m_currentRuleList != nullptr)
    {
        UpdateUnionQcDocUi(m_currentRuleList->m_assayInfo);
    }
}

void QcRuleSettingsDlg::ProcessUnionChanged(QComboBox* curComb, int sel)
{
    ULOG(LOG_INFO, "%s, sel:%d.", __FUNCTION__, sel);

    if (m_currentRuleList == nullptr)
    {
        ULOG(LOG_ERROR, "Null current assayinfo.");
        return;
    }

    // 区分当前控件
    bool isX = (curComb == ui->comboBoxQcX);
    int64_t* targetId = isX ? &m_currentRuleList->m_qcX : &m_currentRuleList->m_qcY;
    QLineEdit* qTarg = isX ? ui->lineEditTagetX : ui->lineEditTargetY;
    QLineEdit* qSd = isX ? ui->lineEditSdX : ui->lineEditSdY;

    qTarg->setText(QStringLiteral(""));
    qSd->setText(QStringLiteral(""));

    // 如果未选择，则不进行缓存值的更改
    if (sel == -1)
    {
        return;
    }

    // 如果选择空，则赋予默认值-1，并清空目标值与SD
    if (sel == 0)
    {
        *targetId = -1;
        return;
    }

    // 判断是否X、Y所选质控品相同
    QString textX = ui->comboBoxQcX->currentText();
    QString textY = ui->comboBoxQcY->currentText();
    if (!textX.isEmpty() && textX == textY)
    {
        TipDlg msg(tr("所选择的质控品X与质控品Y不能相同!"));
        msg.exec();
        curComb->setCurrentIndex(0);
        return;
    }

    tf::QcDoc curData = curComb->itemData(sel).value<tf::QcDoc>();
    *targetId = curData.id;

    // 显示浓度值、SD值
    tf::QcComposition qcComp;
    if (!FindQcComposition(curData, m_currentRuleList->m_assayInfo.assayCode, qcComp))
    {
        ULOG(LOG_ERROR, "Not find qc composition by assayCode:%d", \
            m_currentRuleList->m_assayInfo.assayCode);
        return;
    }
    int prv = m_currentRuleList->m_assayInfo.decimalPlace;
    qTarg->setText(QString::number(qcComp.targetValue, 'f', prv));
    qSd->setText(QString::number(qcComp.sd, 'f', prv));
}

void QcRuleSettingsDlg::OnUnionQcDocXChanged(int sel)
{
    ProcessUnionChanged(ui->comboBoxQcX, sel);
}

void QcRuleSettingsDlg::OnUnionQcDocYChanged(int sel)
{
    ProcessUnionChanged(ui->comboBoxQcY, sel);
}
