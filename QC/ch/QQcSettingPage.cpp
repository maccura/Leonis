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

#include "QQcSettingPage.h"
#include "ui_QQcSettingPage.h"

#include "QQcSampleModel.h"
#include "QQcDownloadRegDlg.h"
#include "QQcRegSample.h"
#include "qcdocregdlg.h"
#include <QFileDialog>
#include <QTimer>
#include <thread>

#include "shared/tipdlg.h"
#include "shared/uidef.h"
#include "shared/uicommon.h"
#include "shared/QComDelegate.h"
#include "shared/QPostionEdit.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/CommonInformationManager.h"
#include "shared/CReadOnlyDelegate.h"
#include "shared/FileExporter.h"

#include "SortHeaderView.h"
#include "manager/UserInfoManager.h"
#include "manager/OperationLogManager.h"

#include "thrift/DcsControlProxy.h"
#include "QC/QcRuleSettingsDlg.h"
#include "src/common/common.h"
#include "src/common/Mlog/mlog.h"
#include "Serialize.h"
#include "printcom.hpp"
#include "DataManagerQc.h"


QQcSettingPage::QQcSettingPage(QWidget *parent)
    : QWidget(parent),
    m_bInit(false),
    m_PostionEditDialog(nullptr)
{
    ui = new Ui::QQcSettingPage();
    ui->setupUi(this);
    Init();
}

QQcSettingPage::~QQcSettingPage()
{
}

void QQcSettingPage::Init()
{
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->horizontalHeader()->setHighlightSections(false);
    ui->tableView->verticalHeader()->setVisible(false);

    // 排序设置   
    QQcSampleModel &qcModel = QQcSampleModel::Instance();
    qcModel.SetView(ui->tableView);
    m_qSortModel = new QcSortFilterProxyModel(this);
    m_qSortModel->setSortRole(Qt::InitialSortOrderRole);
    m_qSortModel->setSourceModel(&qcModel);
    ui->tableView->setModel(m_qSortModel);

    ui->compositionView->setModel(&QQcAssayModel::Instance());
    ui->compositionView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->compositionView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->compositionView->horizontalHeader()->setHighlightSections(false);
    ui->compositionView->verticalHeader()->setVisible(false);

    // 设置列宽
    ui->compositionView->setColumnWidth((int)QQcAssayModel::COLASSAY::NAME, 170);
    ui->compositionView->setColumnWidth((int)QQcAssayModel::COLASSAY::TARGETVALUE, 70);
    ui->compositionView->setColumnWidth((int)QQcAssayModel::COLASSAY::STANDARD, 70);
    ui->compositionView->setColumnWidth((int)QQcAssayModel::COLASSAY::CV, 70);

    // 列宽自适应
    ResizeTblColToContent(ui->compositionView);

    // 初始时质控信息表没有选中项，禁能质控品信息操作按钮
    ui->flat_m_regist->setEnabled(false);
    ui->flat_modify->setEnabled(false);
    ui->del_Button->setEnabled(false);

    // Sort Header
    // 升序 降序 原序
    m_pSortHeader = new SortHeaderView(Qt::Horizontal, ui->tableView);
    m_pSortHeader->setStretchLastSection(true);
    ui->tableView->setHorizontalHeader(m_pSortHeader);

    // 默认排序初始化
    m_pSortHeader->setSortIndicator(-1, Qt::DescendingOrder);
    m_qSortModel->sort(-1, Qt::DescendingOrder);

    connect(m_pSortHeader, &SortHeaderView::SortOrderChanged, this, [this](int logicIndex, SortHeaderView::SortOrder order) {
        //无效index或NoOrder就设置为默认未排序状态
        if (logicIndex < 0 || order == SortHeaderView::NoOrder) {
            m_pSortHeader->setSortIndicator(-1, Qt::DescendingOrder); //去掉排序三角样式
            m_qSortModel->sort(-1, Qt::DescendingOrder); //-1则还原model默认顺序
        }
        else
        {
            Qt::SortOrder qOrderFlag = order == SortHeaderView::DescOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
            ui->tableView->sortByColumn(logicIndex, qOrderFlag);
        }
    });
    ui->tableView->setSortingEnabled(true);

    // 监听免疫质控申请更新信息
    REGISTER_HANDLER(MSG_ID_IM_QCAPP_INFO_UPDATE, this, OnImQcApplyInfoUpdate);
    // 监听试剂更新信息
    REGISTER_HANDLER(MSG_ID_QC_DOC_INFO_UPDATE, this, OnQcDocUpdate);
    // 当项目信息发生变化时
    REGISTER_HANDLER(MSG_ID_ASSAY_CODE_MANAGER_UPDATE, this, OnAssayInfoChanged);

    // 位置的设定
    connect(ui->tableView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(OnClickedRow(const QModelIndex&)));
    // 按钮的使能
    connect(ui->tableView->selectionModel(), SIGNAL(currentRowChanged(const QModelIndex&, const QModelIndex&)), \
        this, SLOT(OnTabelSelectChanged(const QModelIndex&, const QModelIndex&)));
    // 监听显示数据的变化
    connect(&qcModel, SIGNAL(displayDataChanged()), this, SLOT(OnDisplayDataChanged()));
    // 手动登记
    connect(ui->flat_m_regist, SIGNAL(clicked()), this, SLOT(OnClickOpRegDoc()));
    // 修改
    connect(ui->flat_modify, SIGNAL(clicked()), this, SLOT(OnClickOpRegDoc()));
    // 删除
    connect(ui->del_Button, SIGNAL(clicked()), this, SLOT(OnDelete()));
    // 打印
    connect(ui->PrintBtn, SIGNAL(clicked()), this, SLOT(OnPrintBtnClicked()));
    // 导出
    connect(ui->export_btn, SIGNAL(clicked()), this, SLOT(OnExportBtnClicked()));

    // 角落筛选条件被重置
    REGISTER_HANDLER(MSG_ID_QC_TAB_CORNER_QRY_COND_RESET, this, OnTabCornerQryCondReset);

    // 初始化置空
    m_downloadRegDlg = nullptr;
    m_settingDialog = nullptr;
    m_pQcRegDlg = nullptr;

    // 失控规则
    connect(ui->flat_qc_rule, SIGNAL(clicked()), this, SLOT(OnQcRuleSettings()));

    // 筛选和重置
    connect(ui->QcDocCombo, SIGNAL(currentIndexChanged(int)), this, SLOT(OnQryCondChanged()));
    connect(ui->QcDocCombo, SIGNAL(ResetText()), this, SLOT(OnQryCondChanged()));
    connect(ui->comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(OnQryCondChanged()));
    connect(ui->comboBox, SIGNAL(ResetText()), this, SLOT(OnQryCondChanged()));
    connect(ui->pushButtonQcDocSearch, SIGNAL(clicked()), this, SLOT(OnClickedQcDocSearch()));
    connect(ui->pushButtonQcDocRest, SIGNAL(clicked()), this, SLOT(OnClickedQcRest()));

    // 设置列宽
    ui->tableView->setColumnWidth((int)QQcSampleModel::COLSAMPLE::SEQNO, 104);
    ui->tableView->setColumnWidth((int)QQcSampleModel::COLSAMPLE::SERIALNUMBER, 121);
    ui->tableView->setColumnWidth((int)QQcSampleModel::COLSAMPLE::NAME, 150);
    ui->tableView->setColumnWidth((int)QQcSampleModel::COLSAMPLE::ABBREVIATION, 121);
    ui->tableView->setColumnWidth((int)QQcSampleModel::COLSAMPLE::TYPE, 115);
    ui->tableView->setColumnWidth((int)QQcSampleModel::COLSAMPLE::LEVEL, 100);
    ui->tableView->setColumnWidth((int)QQcSampleModel::COLSAMPLE::LOT, 122);
    ui->tableView->setColumnWidth((int)QQcSampleModel::COLSAMPLE::EXPIRED, 135);
    ui->tableView->setColumnWidth((int)QQcSampleModel::COLSAMPLE::REGTYPE, 180);
    ui->tableView->setColumnWidth((int)QQcSampleModel::COLSAMPLE::POS_TUBE, 105);

    // 设置代理，格式化日期显示
    ui->tableView->setItemDelegateForColumn((int)QQcSampleModel::COLSAMPLE::EXPIRED, new AutoExpriteDelegate(this));
    qcModel.setHeaderData((int)QQcSampleModel::COLSAMPLE::POS_TUBE, Qt::Horizontal, QBrush(UI_TEXT_COLOR_HEADER_MARK), Qt::ForegroundRole);

    // 位置显示代理
    ui->tableView->setItemDelegateForColumn((int)QQcSampleModel::COLSAMPLE::POS_TUBE, new QPositionDelegate(this));
    ui->tableView->horizontalHeader()->setTextElideMode(Qt::ElideLeft);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);

    ReloadQcDocCombData();

    // 初始化注册类型筛选下拉框
    InitRegisFilterCombox();

    // 最开始默认选中第一行
    ui->tableView->selectRow(0);

    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);

    // 响应子线程的提示信息
    connect(this, SIGNAL(hint(QString)), this, SLOT(OnHint(QString)));
}

///
/// @brief
///     显示后初始化
///
/// @par History:
/// @li 4170/TangChuXian，2023年8月7日，新建函数
///
void QQcSettingPage::InitAfterShow()
{
    // 列宽自适应 // 屏蔽原因：0022562: [质控] 质控设置界面设置质控品名称或者简称字符长度过长，质控品分配的位置显示不全
    //ResizeTblColToContent(ui->tableView);
}

///
/// @brief
///     弹出质控登记页面类型
///
/// @param[in]  enPageType  页面类型
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月12日，新建函数
///
bool QQcSettingPage::SwitchQcRegPage(QcRegPageType enPageType)
{
    // 获取选中的行
    QModelIndex rowIdx = ui->tableView->currentIndex();
    if (rowIdx.row() < 0)
    {
        TipDlg tipDlg("选中行为空！此操作需要选中某一行才能进行。");
        tipDlg.exec();
        ULOG(LOG_WARN, "Invalid current sel.");
        return false;
    }
    InitQcRegDlg(rowIdx);

    auto qcDoc = QQcSampleModel::Instance().GetDocByIndex(rowIdx);
    int newIndexfield = -1;

    std::shared_ptr<tf::QcDoc> qcPtr = nullptr;
    if (qcDoc.has_value())
    {
        qcPtr = std::make_shared<tf::QcDoc>(qcDoc.get());
        newIndexfield = qcDoc->index;
    }
    else
    {
        newIndexfield = QQcSampleModel::Instance().GenerateQcIndex(rowIdx);
    }

    // 判断质控登记页面类型
    switch (enPageType)
    {
    case QQcSettingPage::REG_PAGE_TYPE_SCAN:
        // 扫描登记
        m_settingDialog->StartScan(newIndexfield, qcPtr);
        m_pQcRegDlg->SetCurrentPage(m_settingDialog);
        m_pQcRegDlg->SetModifyQcMode(false);
        m_pQcRegDlg->SetIgnRgntIconVisible(false);
        break;
    case QQcSettingPage::REG_PAGE_TYPE_MANUAL:
        // 手动登记
        m_settingDialog->StartManual(newIndexfield, qcPtr);
        m_pQcRegDlg->SetCurrentPage(m_settingDialog);
        m_pQcRegDlg->SetModifyQcMode(false);
        m_pQcRegDlg->SetIgnRgntIconVisible(false);
        break;
    case QQcSettingPage::REG_PAGE_TYPE_DOWNLOAD:
        // 下载登记
        m_pQcRegDlg->SetCurrentPage(m_downloadRegDlg);
        m_pQcRegDlg->SetModifyQcMode(false);
        m_pQcRegDlg->SetIgnRgntIconVisible(true);
        m_downloadRegDlg->StartDownload(newIndexfield, qcPtr);
        break;
    case QQcSettingPage::REG_PAGE_TYPE_MODIFY:
        // 修改登记
        m_pQcRegDlg->SetCurrentPage(m_settingDialog);
        m_pQcRegDlg->SetModifyQcMode(true);
        m_settingDialog->StartModify(newIndexfield, qcPtr);
        m_pQcRegDlg->SetIgnRgntIconVisible(false);
        break;
    default:
        break;
    }

    return true;
}

void QQcSettingPage::OnClickedRow(const QModelIndex& index)
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);

    // 再次刷新删除按钮（currentRowChanged 在多选之后点击其它地方，获取到的是之前的选中位置)
    UpdateDeleteButtonStatu();

    // 位置行
    if (index.column() != (int)QQcSampleModel::COLSAMPLE::POS_TUBE)
    {
        return;
    }

    auto qcDoc = QQcSampleModel::Instance().GetDocByIndex(index);
    if (!qcDoc)
    {
        ULOG(LOG_WARN, "Null qcDoc get form qcsamplemodel.");
        return;
    }

    if (m_PostionEditDialog == nullptr)
    {
        m_PostionEditDialog = new QPostionEdit(this);
        connect(m_PostionEditDialog, SIGNAL(ModifyQcPostion()), this, SLOT(OnSavePostion()));
    }

    m_PostionEditDialog->SetQcDoc(QList<::tf::QcDoc>({ qcDoc.value() }), qcDoc.value().level);
    m_PostionEditDialog->show();
}

void QQcSettingPage::OnSavePostion()
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);

    if (m_PostionEditDialog == nullptr)
    {
        ULOG(LOG_WARN, "Null position edit dialog.");
        return;
    }

    // 没有数据或者，为空
    auto qc = m_PostionEditDialog->GetQcDoc();
    if (!qc || qc.value().empty())
    {
        ULOG(LOG_ERROR, "Null qcDoc get from positionEditDialog.");
        return;
    }
    tf::QcDoc modifiedDoc = qc.value()[0];

    // 判断是否被占用
    tf::ResultLong ret;
    if (!DcsControlProxy::GetInstance()->ModifyQcDoc(ret, modifiedDoc) || ret.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        QString strMsg = ret.result == tf::ThriftResult::THRIFT_RESULT_MODIFY_USEING ? \
            tr("使用中的质控品信息不被允许修改！") : tr("质控品位置保存失败！");

        TipDlg(strMsg).exec();
        ULOG(LOG_ERROR, "Failed to save qc postion.");
        return;
    }
    // 添加操作日志
    COperationLogManager::GetInstance()->AddQCOperationLog(::tf::OperationType::type::MOD, 
		modifiedDoc, DataManagerQc::GetInstance()->GetQcDocByID(modifiedDoc.id));

    // 刷新通知
    QQcSampleModel::Instance().Update({modifiedDoc.id});

    tf::QcDocUpdate modifed;
    modifed.__set_db(modifiedDoc.id);
    modifed.__set_index(modifiedDoc.index);
    modifed.__set_updateType(tf::UpdateType::UPDATE_TYPE_MODIFY);
    std::vector<tf::QcDocUpdate> vecQcDocUpdate;
    vecQcDocUpdate.push_back(modifed);
    POST_MESSAGE(MSG_ID_QC_DOC_INFO_UPDATE, vecQcDocUpdate);  // 质控品位置更新
}

void QQcSettingPage::UpdateAssay(const QModelIndex& index)
{
    auto qcDoc = QQcSampleModel::Instance().GetDocByIndex(index);
    std::vector<::tf::QcComposition> assaydata;
    if (qcDoc.has_value())
    {
        for (const auto& composition : qcDoc.value().compositions)
        {
            assaydata.push_back(composition);
        }
    }
    QQcAssayModel::Instance().SetData(assaydata);

    // 列宽自适应
    ResizeTblColToContent(ui->compositionView);
}

void QQcSettingPage::UpdateDeleteButtonStatu()
{
    QModelIndexList sItems = ui->tableView->selectionModel()->selectedRows();
    if (sItems.size() == 0)
    {
        // 处理最初时selectRow(0)后 selectedRows()获取不到选中行的问题
        QModelIndex curIdx = ui->tableView->currentIndex();
        if (curIdx.isValid())
        {
            sItems.push_back(curIdx);
        }
    }

    bool existSelectValue = false;
    for (const QModelIndex& mItem : sItems)
    {
        auto sDocItem = QQcSampleModel::Instance().GetDocByIndex(mItem);
        if (sDocItem.has_value())
        {
            existSelectValue = true;
            break;
        }
    }
    ui->del_Button->setEnabled(existSelectValue);

    // 导出按钮
    bool enableExportPrint = (QQcSampleModel::Instance().GetQcNames().size() > 0);
    ui->export_btn->setEnabled(enableExportPrint);
    ui->PrintBtn->setEnabled(enableExportPrint);
}

void QQcSettingPage::InitRegisFilterCombox()
{
    ui->comboBox->addItem(tr("手工登记"), tf::DocRegistType::REG_MANUAL);
    ui->comboBox->addItem(tr("扫描登记"), tf::DocRegistType::REG_SCAN);
    ui->comboBox->addItem(tr("下载登记"), tf::DocRegistType::REG_DOWNLOAD);

	ui->comboBox->setCurrentIndex(-1);
}

void QQcSettingPage::GetJionUnionQcAssay(std::map<int64_t, std::set<std::string>>& qcDocMap)
{
    auto funcAddQcdocid = [&](int64_t curId, const std::string& assayName) {
        std::map<int64_t, std::set<std::string>>::iterator qIter = qcDocMap.find(curId);
        if (qIter != qcDocMap.end())
        {
            qIter->second.insert(assayName);
        }
        else
        {
            std::set<std::string> sNames;
            sNames.insert(assayName);
            qcDocMap.insert(std::pair<int64_t, std::set<std::string>>(curId, sNames));
        }
    };

    qcDocMap.clear();
    AssayIndexCodeMaps assayMap = CommonInformationManager::GetInstance()->GetAssayInfo();
    for (AssayIndexCodeMaps::iterator it = assayMap.begin(); it != assayMap.end(); ++it)
    {
        if (it->second == nullptr)
        {
            continue;
        }
        const tf::QcJudgeParam &qcRule = it->second->qcRules;
        if (qcRule.XQCDocID > 0 && qcRule.YQCDocID > 0)
        {
            funcAddQcdocid(qcRule.XQCDocID, it->second->assayName);
            funcAddQcdocid(qcRule.YQCDocID, it->second->assayName);
        }
    }
}

void QQcSettingPage::OnTabelSelectChanged(const QModelIndex& cur, const QModelIndex& pre)
{
    auto qcDoc = QQcSampleModel::Instance().GetDocByIndex(cur);

    // 设置状态
    ui->flat_m_regist->setEnabled(true);     // 手动登记

    // 判断质控品是否在用
    auto funcIsUsing = [](const boost::optional<tf::QcDoc>& opQcDoc) {
        // 空行不可修改
        if (!opQcDoc.has_value())
        {
            return true;
        }
        tf::SampleInfoQueryCond cond;
        cond.__set_qcDocId(opQcDoc->id);
        cond.__set_statuses({ tf::SampleStatus::SAMPLE_STATUS_TESTING });
        tf::SampleInfoQueryResp resp;
        // 查询失败不可修改
        if (!DcsControlProxy::GetInstance()->QuerySampleInfo(cond, resp))
        {
            ULOG(LOG_ERROR, "Failed to query sampleinfo.");
            return true;
        }
        return (resp.__isset.lstSampleInfos && resp.lstSampleInfos.size() > 0);
    };

    // 修改按钮的状态
    ui->flat_modify->setEnabled(!funcIsUsing(qcDoc));

    // 刷新项目
    UpdateAssay(cur);

    // 可能选中多个
    UpdateDeleteButtonStatu();
}

///
/// @brief
///     查询条件改变
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月10日，新建函数
///
void QQcSettingPage::OnQryCondChanged()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 获取信息发送者
    QFilterComboBox* pCombo = qobject_cast<QFilterComboBox*>(sender());
    if (pCombo == Q_NULLPTR)
    {
        ULOG(LOG_WARN, "%s(), pCombo == Q_NULLPTR", __FUNCTION__);
        return;
    }

    // 如果文本不为空，则忽略
    if (!pCombo->currentText().isEmpty())
    {
        ULOG(LOG_INFO, "%s(), ignore", __FUNCTION__);
        return;
    }

    // 触发筛选
    QTimer::singleShot(0, this, SLOT(OnClickedQcDocSearch()));
}

bool QQcSettingPage::GetCurrentOporationSel(QModelIndex& rowIdx)
{
    // 防止传空序号
    rowIdx = ui->tableView->currentIndex();
    if (rowIdx.row() < 0)
    {
        TipDlg tipDlg("选中行为空！此操作需要选中某一行才能进行。");
        return false;
    }

    return true;
}

void QQcSettingPage::ReloadQcDocCombData()
{
    int curSel = ui->QcDocCombo->currentIndex();
    std::vector<std::string> &qcDocNames = QQcSampleModel::Instance().GetQcNames();

    QStringList qList;
    for (const std::string& qn : qcDocNames)
    {
        qList.append(QString::fromStdString(qn));
    }

    ui->QcDocCombo->blockSignals(true);
    ui->QcDocCombo->clear();
    ui->QcDocCombo->addItems(UiCommon::QStringListFiltSameItem(qList));
    ui->QcDocCombo->setCurrentIndex(curSel);
    ui->QcDocCombo->blockSignals(false);
}

void QQcSettingPage::OnClickOpRegDoc()
{
    QPushButton* btn = qobject_cast<QPushButton*>(sender());
    if (btn == Q_NULLPTR)
    {
        return;
    }

    if (btn == ui->flat_m_regist)
    {
        // 扫描登记
        if (!SwitchQcRegPage(REG_PAGE_TYPE_SCAN))
        {
            return;
        }
    }
    else if (btn == ui->flat_modify)
    {
        // 修改登记
        if (!SwitchQcRegPage(REG_PAGE_TYPE_MODIFY))
        {
            return;
        }
    }

    // 没生成则返回
    if (m_pQcRegDlg == Q_NULLPTR)
    {
        return;
    }

    // 弹出质控登记对话框
    m_pQcRegDlg->exec();
}

///
/// @brief
///     显示数据改变
///
/// @par History:
/// @li 4170/TangChuXian，2023年8月7日，新建函数
///
void QQcSettingPage::OnDisplayDataChanged()
{ 
    // 屏蔽原因：0022562: [质控] 质控设置界面设置质控品名称或者简称字符长度过长，质控品分配的位置显示不全
    //ResizeTblColToContent(ui->tableView);
}

QString QQcSettingPage::GetJoinUnionQcAssayNames(std::vector<::tf::QcDoc>& vecQcDoc)
{
    // 获取联合质控信息，用于检测与提示
    std::map<int64_t, std::set<std::string>> qcDocMap;
    GetJionUnionQcAssay(qcDocMap);

    QString strNotices;
    for (const tf::QcDoc qDoc : vecQcDoc)
    {
        std::map<int64_t, std::set<std::string>>::iterator it = qcDocMap.find(qDoc.id);
        if (it != qcDocMap.end())
        {
            QString strNote = tr("质控品：%1 参与了项目：%2的联合质控")
                .arg(QString::fromStdString(qDoc.name))
                .arg(QString::fromStdString(GetContainerJoinString(it->second, ",")));

            strNotices += strNote;
            strNotices += "\n";
        }
    }
    if (strNotices.endsWith('\n'))
    {
        strNotices.chop(1);
    }

    return strNotices;
}

///
/// @brief 删除质控文档
///
///
/// @return 
///
/// @par History:增加操作日志
/// @li 1556/Chenjianlin，2023年7月29日，新建函数
///
void QQcSettingPage::OnDelete()
{
    // 修复	0021585: [质控] 运行、加样停状态下，质控设置界面可以删除质控品
    if (CommonInformationManager::GetInstance()->IsExistDeviceRuning())
    {
        TipDlg(tr("检测中不能删除质控品！")).exec();
        return;
    }
    QModelIndex currentIdx = ui->tableView->currentIndex();

    std::set<int> beDeleteIndexes;
    std::vector<::tf::QcDoc> vecQcDoc;
    // 获取界面显示的质控文档信息
    if (!GetSelectionQqDoc(beDeleteIndexes, vecQcDoc))
    {
        return;
    }

    // 弹框提示是否确认删除
    QString strNames = GetJoinUnionQcAssayNames(vecQcDoc);
    QString strDelate = tr("确定删除选中质控品？");
    QString strFinNot = strNames.isEmpty() ? strDelate : (strNames + tr("，") + strDelate);
    TipDlg confirmTipDlg(tr("删除"), strFinNot, TipDlgType::TWO_BUTTON);
    if (confirmTipDlg.exec() == QDialog::Rejected)
    {
        ULOG(LOG_INFO, "Canceled.");
        return;
    }

    // 删除对应的QC项
    std::vector<int> faildIndexes;
    if (!QQcSampleModel::Instance().removeQcRow(beDeleteIndexes, faildIndexes))
    {
        TipDlg(tr("执行删除操作失败！")).exec();
        return;
    }
    // 添加操作日志
    std::vector<tf::QcDocUpdate> vecQcDocUpdate;
    for (const tf::QcDoc& atVal : vecQcDoc)
    {
        if (std::find(faildIndexes.begin(), faildIndexes.end(), atVal.index) != faildIndexes.end())
        {
            continue;
        }
        tf::QcDocUpdate upItem;
        upItem.__set_db(atVal.id);
        upItem.__set_index(atVal.index);
        upItem.__set_updateType(tf::UpdateType::UPDATE_TYPE_DELETE);
        vecQcDocUpdate.push_back(upItem);
        COperationLogManager::GetInstance()->AddQCOperationLog(::tf::OperationType::type::DEL, atVal);
    }
    if (faildIndexes.size() > 0)
    {
        QString strNotice = QString::fromStdString(GetContainerJoinString(faildIndexes, ","));
        TipDlg(tr("无法删除序号为：%1的选中行，选中行的质控品被占用或删除操作出现未知异常！").arg(strNotice)).exec();
    }

    POST_MESSAGE(MSG_ID_QC_DOC_INFO_UPDATE, vecQcDocUpdate);

    // 清空质控项目
    std::vector<::tf::QcComposition> assaydata;
    QQcAssayModel::Instance().SetData(assaydata);

    // 列宽自适应
    ResizeTblColToContent(ui->compositionView);

    // 恢复选中行
    if (currentIdx.isValid())
    {
        ui->tableView->selectRow(currentIdx.row());
    }
}

///
/// @brief 获取界面显示的质控文档信息
///
/// @param[in]  vecQcDocUpdate  
/// @param[in]  beDeleteIndexes  
/// @param[in]  vecQcDoc  
///
/// @return 
///
/// @par History:
/// @li 1556/Chenjianlin，2023年7月29日，新建函数
///
bool QQcSettingPage::GetSelectionQqDoc(std::set<int>& beDeleteIndexes, std::vector<::tf::QcDoc>& vecQcDoc)
{
    auto selectModule = ui->tableView->selectionModel();
    if (selectModule == nullptr)
    {
        return false;
    }
    // 遍历选择
    QModelIndexList allSelected = selectModule->selectedRows();
    for (const auto& index : allSelected)
    {
        // 获取当前行的质控品
        auto item = QQcSampleModel::Instance().GetDocByIndex(index);
        if (!item)
        {
            continue;
        }
        beDeleteIndexes.insert(item->index);
        // 名称、批号和编号 0022944: [应用] 日志-操作日志界面查询操作类型为”删除”的操作日志，删除质控品的操作记录显示的质控品信息中缺少质控品编号 modify by chenjianlin 20231214
        vecQcDoc.push_back(item.value());
    }
    return true;
}

void QQcSettingPage::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);

    // 更新菜单位置
    POST_MESSAGE(MSG_ID_CURRENT_MODEL_NAME, QString("> ") + tr("质控") + QString(" > ") + tr("质控设置"));

    // 切换回页面时，变成默认排序
    m_pSortHeader->setSortIndicator(-1, Qt::DescendingOrder); //去掉排序三角样式
    m_pSortHeader->ResetAllIndex();
    m_qSortModel->sort(-1, Qt::DescendingOrder); //-1则还原model默认顺序

    // 显示后初始化
    if (!m_bInit)
    {
        m_bInit = true;
        InitAfterShow();
    }
}

void QQcSettingPage::OnPermisionChanged()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    std::shared_ptr<UserInfoManager> userPms = UserInfoManager::GetInstance();

    // 质控规则
    userPms->IsPermisson(PMS_QC_SET_RULE) ? ui->flat_qc_rule->show() : ui->flat_qc_rule->hide();
    // 删除
    userPms->IsPermisson(PMS_QC_DELETE) ? ui->del_Button->show() : ui->del_Button->hide();
    // 修改
    userPms->IsPermisson(PMS_QC_MODIFY_BASEINFO) ? ui->flat_modify->show() : ui->flat_modify->hide();
    // 注册
    userPms->IsPermisson(PMS_QC_REGIST_MANUAL) ? ui->flat_m_regist->show() : ui->flat_m_regist->hide();
    // 导出
    userPms->IsPermissionShow(PSM_QC_SETTINGS_EXPORT) ? ui->export_btn->show() : ui->export_btn->hide();
}

///
/// @brief 质控文档更新通知
///  
/// @param[in]  infos  更新的质控文档信息
///
/// @par History:
/// @li 8090/YeHuaNing，2022年10月19日，新建函数
///
void QQcSettingPage::OnQcDocUpdate(std::vector<tf::QcDocUpdate, std::allocator<tf::QcDocUpdate>> infos)
{
    QQcSampleModel &qcMode = QQcSampleModel::Instance();
    if (infos.size() == 0)
    {
        int iSelectRow = ui->tableView->currentIndex().row();
        qcMode.ReloadQcdoc();
        ui->tableView->selectRow(iSelectRow);
    }

    // 处理数据更新
    std::vector<int64_t> updatedDbId;
    for (const tf::QcDocUpdate& item : infos)
    {
        if (item.db > 0) // item.__isset.db not effect
        {
            updatedDbId.push_back(item.db);
        }
    }
    qcMode.Update(updatedDbId);

    // 处理选中状态更新
    for (const tf::QcDocUpdate& item : infos)
    {
        if (item.index >= 0)
        {
            int rowIdx = qcMode.FindRowByQcIndex(item.index);
            if (rowIdx != -1)
            {
                ui->tableView->selectRow(rowIdx);
                break;
            }
        }
    }

    // 更新筛选框
    ReloadQcDocCombData();

    // 更新按钮状态
    QModelIndex curModelIdx = ui->tableView->currentIndex();
    OnTabelSelectChanged(curModelIdx, curModelIdx);
}

///
/// @brief
///     角落查询条件被重置
///
/// @par History:
/// @li 4170/TangChuXian，2023年8月16日，新建函数
///
void QQcSettingPage::OnTabCornerQryCondReset()
{
    ULOG(LOG_INFO, __FUNCTION__);
}

///
/// @brief
///     更新条件显示字符串
///
/// @param[in]  strCond  筛选条件字符串
///
/// @par History:
/// @li 4170/TangChuXian，2023年8月16日，新建函数
///
void QQcSettingPage::UpdateCondDisplayStr(QString strCond)
{
    ULOG(LOG_INFO, __FUNCTION__);
    // 通知设备列表更新
    POST_MESSAGE(MSG_ID_QC_TAB_CORNER_DISPLAY_UPDATE, QStringList(), 0);
}

void QQcSettingPage::OnQcRuleSettings()
{
    // 提成成员指针的时候，退出窗口后没有清理全面，暂时使用模态框处理
    QcRuleSettingsDlg ruleDlg;
    ruleDlg.exec();
}

void QQcSettingPage::InitQcRegDlg(const QModelIndex& currSel)
{
    if (m_settingDialog == nullptr)
    {
        m_settingDialog = new QQcRegSample(this);
        connect(m_settingDialog, &QQcRegSample::Update, this, [&]() {
            emit ui->tableView->clicked(currSel); });
    }

    if (m_downloadRegDlg == nullptr)
    {
        m_downloadRegDlg = new QQcDownloadRegDlg(this);
        connect(m_downloadRegDlg, &QQcDownloadRegDlg::Update, this, [&]() {
            emit ui->tableView->clicked(currSel); });
    }

    // 初始化质控登记对话框
    if (m_pQcRegDlg == Q_NULLPTR)
    {
        QList<QWidget*> wgtList;
        wgtList.push_back(m_settingDialog);
        wgtList.push_back(m_downloadRegDlg);
        m_pQcRegDlg = new QcDocRegDlg(this);
        m_pQcRegDlg->SetContentPageList(wgtList);

        // 连接信号槽
        // 页面切换
        connect(m_pQcRegDlg, SIGNAL(SigSwitchScanRegPage()), this, SLOT(OnQcRegDlgSwitchScanRegPage()));
        connect(m_pQcRegDlg, SIGNAL(SigSwitchManualRegPage()), this, SLOT(OnQcRegDlgSwitchManualRegPage()));
        connect(m_pQcRegDlg, SIGNAL(SigSwitchQcDownloadPage()), this, SLOT(OnQcRegDlgSwitchQcDownloadPage()));
    }
}

void QQcSettingPage::UpdateTableView(const std::string& qcDocNameFilter, tf::DocRegistType::type registType)
{
    // 保存当前选中行序号
    QModelIndex rowIdx = ui->tableView->currentIndex();
    auto &qcModel = QQcSampleModel::Instance();
    QModelIndex seqCellIdx = qcModel.index(rowIdx.row(), 0);
    QString strSeqName;
    if (seqCellIdx.isValid() && seqCellIdx.data().isValid())
    {
        strSeqName = seqCellIdx.data().toString();
    }

    qcModel.SetQcFilter(QQcSampleModel::QcFilter(qcDocNameFilter, registType));
    qcModel.ReloadQcdoc();
    ui->tableView->viewport()->update();

    if (!strSeqName.isEmpty())
    {
        int rowIdx = qcModel.FindRowByIndexName(strSeqName);
        if (rowIdx != -1)
        {
            ui->tableView->selectRow(rowIdx);

            // 更新composition表
            UpdateAssay(qcModel.index(rowIdx, 0));
        }
    }
}

void QQcSettingPage::OnClickedQcDocSearch()
{
    // 获取注册类型下拉框内容
    int curRegIdx = ui->comboBox->currentIndex();
	tf::DocRegistType::type curRegistType;
    if (curRegIdx == -1)
    {
		curRegistType = tf::DocRegistType::REG_EMTPY;
    }
	else
	{
		QVariant curValue = ui->comboBox->itemData(curRegIdx);
		if (!curValue.isValid())
		{
			return;
		}
		curRegistType = (tf::DocRegistType::type)curValue.toInt();
	}
	
    // 使用筛选条件进行列表刷新
    UpdateTableView(ui->QcDocCombo->currentText().toStdString(), curRegistType);

	// 如果不是全清则不排序
	if (ui->QcDocCombo->currentIndex() != -1 || ui->comboBox->currentIndex() != -1 || !ui->QcDocCombo->currentText().isEmpty())
	{
		m_pSortHeader->SortOrderChanged(0, SortHeaderView::AscOrder);
	}
}

void QQcSettingPage::OnClickedQcRest()
{
    ui->QcDocCombo->setCurrentIndex(-1);
    ui->comboBox->setCurrentIndex(-1);
    UpdateTableView("", tf::DocRegistType::REG_EMTPY);
}

///
/// @brief
///     质控登记对话框切换扫描登记页面
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月12日，新建函数
///
void QQcSettingPage::OnQcRegDlgSwitchScanRegPage()
{
    // 扫描登记
    SwitchQcRegPage(REG_PAGE_TYPE_SCAN);
}

///
/// @brief
///     质控登记对话框切换手工登记页面
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月12日，新建函数
///
void QQcSettingPage::OnQcRegDlgSwitchManualRegPage()
{
    // 手动登记
    SwitchQcRegPage(REG_PAGE_TYPE_MANUAL);
}

///
/// @brief
///     质控登记对话框切换质控下载页面
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月12日，新建函数
///
void QQcSettingPage::OnQcRegDlgSwitchQcDownloadPage()
{
    // 手动登记
    SwitchQcRegPage(REG_PAGE_TYPE_DOWNLOAD);
}

bool QcSortFilterProxyModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    QString strLeft = source_left.data().toString();
    QString strRight = source_right.data().toString();

    // 空行排到后边
    if (strLeft.isEmpty() && strRight.isEmpty())
    {
        return false;
    }

    if (strLeft.isEmpty())
    {
        return sortOrder() == Qt::AscendingOrder ? false : true;
    }

    if (strRight.isEmpty())
    {
        return sortOrder() == Qt::AscendingOrder ? true : false;
    }

    return strLeft < strRight;
}

///
/// @brief
///     质控申请信息更新(临时)
///
/// @param[in]  qa  质控申请信息
///
/// @par History:
/// @li 1226/zhangjing，2023年8月22日，新建函数
///
void QQcSettingPage::OnImQcApplyInfoUpdate(im::tf::QcApply qa)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    // 获取注册类型下拉框内容
    int curRegIdx = ui->comboBox->currentIndex();
    if (curRegIdx == -1)
    {
        return;
    }
    QVariant curValue = ui->comboBox->itemData(curRegIdx);
    if (!curValue.isValid())
    {
        return;
    }
    tf::DocRegistType::type curRegistType = (tf::DocRegistType::type)curValue.toInt();

    // 使用筛选条件进行列表刷新
    UpdateTableView(ui->QcDocCombo->currentText().toStdString(), curRegistType);
}

void QQcSettingPage::OnAssayInfoChanged()
{
    OnQcDocUpdate({});
}

///
/// @brief 响应打印按钮
///
/// @par History:
/// @li 6889/ChenWei，2023年12月14日，新建函数
///
void QQcSettingPage::OnPrintBtnClicked()
{
    QCItemRecord Info;
    QString strPrintTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    Info.strInstrumentModel = "i 6000";
    Info.strPrintTime = strPrintTime.toStdString();
    GetPrintExportInfo(Info);
    if (Info.vecRecord.empty())
    {
        emit hint(tr("没有可打印的数据！"));
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
void QQcSettingPage::OnExportBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 弹出保存文件对话框
    QString strFilepath = QFileDialog::getSaveFileName(this, tr("保存为..."), QString(), tr("EXCEL files (*.xlsx);;CSV files (*.csv);;PDF files (*.pdf)"));
    if (strFilepath.isEmpty())
    {
        ULOG(LOG_INFO, "Export canceled!");
        return;
    }

    // 耗时操作放子线程
    std::thread t([this, strFilepath]() {
        ULOG(LOG_INFO, "%s()", __FUNCTION__);

        // 打开进度条
        POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_OPEN, tr("正在导出数据，请稍候..."), true);
        QCItemRecord Info;
        GetPrintExportInfo(Info);
        if (Info.vecRecord.empty())
        {
            POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CLOSE);                 // 关闭进度条
            emit hint(tr("没有可导出的数据！"));
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
            QString strUserTmplatePath = strDirPath + "/resource/exportTemplate/ExportQCItemRecord.lrxml";
            int irect = printcom::printcom_async_assign_export(strInfo, strFilepath.toStdString(), strUserTmplatePath.toStdString(), unique_id);
            ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);
            bRect = true;
        }
        else
        {
            std::shared_ptr<FileExporter> pFileEpt(new FileExporter());
            bRect = pFileEpt->ExportQCItemRecord(Info, strFilepath);
        }

        POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_CLOSE);                 // 关闭进度条
        emit hint(bRect ? tr("导出完成!") : tr("导出失败！"));
    });
    t.detach();
}

void QQcSettingPage::GetPrintExportInfo(QCItemRecord& info)
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 获取一个单元格内容
    auto funcValue = [this](int ir, int ic, bool& bIsValue) -> std::string {
        QModelIndex sortIndex = ui->tableView->model()->index(ir, ic);
        QModelIndex sourceIndex = m_qSortModel->mapToSource(sortIndex);
        QString strData = QQcSampleModel::Instance().GetPrintData(sourceIndex);

        // 从第二列开始判断是否有值
        if (ic > 0 && !strData.isEmpty())   
            bIsValue = true;

        return strData.toStdString();
    };

    for (int iR = 0; iR < m_qSortModel->rowCount(); iR++)
    {
        bool bRowValue = false;
        QCRecord Record;
        Record.strDocNo = funcValue(iR, (int)QQcSampleModel::COLSAMPLE::SEQNO, bRowValue);
        Record.strQCID = funcValue(iR, (int)QQcSampleModel::COLSAMPLE::SERIALNUMBER, bRowValue);
        Record.strQCName = funcValue(iR, (int)QQcSampleModel::COLSAMPLE::NAME, bRowValue);
        Record.strQcBriefName = funcValue(iR, (int)QQcSampleModel::COLSAMPLE::ABBREVIATION, bRowValue);
        Record.strQcSourceType = funcValue(iR, (int)QQcSampleModel::COLSAMPLE::TYPE, bRowValue);
        Record.strQcLevel = funcValue(iR, (int)QQcSampleModel::COLSAMPLE::LEVEL, bRowValue);
        Record.strQCLot = funcValue(iR, (int)QQcSampleModel::COLSAMPLE::LOT, bRowValue);
        Record.strExpirationDate = funcValue(iR, (int)QQcSampleModel::COLSAMPLE::EXPIRED, bRowValue);
        Record.strRegMethod = funcValue(iR, (int)QQcSampleModel::COLSAMPLE::REGTYPE, bRowValue);
        Record.strPos = funcValue(iR, (int)QQcSampleModel::COLSAMPLE::POS_TUBE, bRowValue);

        if (bRowValue)
            info.vecRecord.push_back(Record);
    }
}

///
/// @bref
///		响应提示信息信号
///
/// @par History:
/// @li 6889/ChenWei，2024年1月29日，新建函数
///
void QQcSettingPage::OnHint(QString strInfo)
{
    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(strInfo));
    pTipDlg->exec();
}
