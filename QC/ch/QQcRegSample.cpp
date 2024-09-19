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

#include "QQcRegSample.h"
#include "ui_QQcRegSample.h"
#ifdef _WIN32
#include <Windows.h>
#endif
#include <QDateTime>
#include <QTimer>
#include <QKeyEvent>
#include "QQcSampleModel.h"
#include "QQcSampleAppModel.h"
#include "src/thrift/ise/gen-cpp/ise_constants.h"

#include "shared/QComDelegate.h"
#include "shared/CReadOnlyDelegate.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/uidef.h"
#include "shared/tipdlg.h"
#include "shared/CommonInformationManager.h"
#include "shared/uicommon.h"
#include "src/common/defs.h"
#include "src/common/common.h"
#include "src/common/Mlog/mlog.h"
#include "thrift/DcsControlProxy.h"
#include "manager/OperationLogManager.h"
#include "manager/DictionaryQueryManager.h"
#ifdef __unix__
#include "safe_str_lib.h"
#endif

#define COL_TARGET      1 // 界面成分表格的靶值列序号
#define COL_SD          2 // 界面成分表格的标准差
#define COL_CV          3 // 界面成分表格的变异系数

// 质控Compositon设置值的验证
bool ValidateCompositionInput(const QString& strText)
{
    if (strText.isEmpty())
    {
        return true;
    }

    double inputNum = strText.toDouble();
    if (inputNum < 0.01)
    {
        TipDlg tipMsg(QObject::tr("输入数据不可小于0.01"));
        tipMsg.exec();
        return false;
    }

    return true;
}

///
/// @bref
///		设置右侧表格单元格的验证事件函数
///
/// @param[in] tbView 表格控件
/// @param[in] rowIdx 列序号
/// @param[in] vfunc 验证方法
///
/// @par History:
/// @li 8276/huchunli, 2023年5月23日，新建函数
///
#define InitLineEdit(tbView, rowIdx, vfunc) do \
{\
    PositiveNumDelegate* edt = new PositiveNumDelegate(this); \
    connect(edt, SIGNAL(textChanged(const QString&)), this, SLOT(OnTextChanged(const QString&))); \
    if (vfunc != nullptr) { edt->SetValidateFunc(vfunc); } \
    tbView->setItemDelegateForColumn(rowIdx, edt); \
} while (0);


QQcRegSample::QQcRegSample(QWidget *parent)
    : QWidget(parent)
{
    m_CompositionModeChim = nullptr;
    m_CompositionModeImmy = nullptr;
    m_qcDoc = nullptr;
    m_uiBarInfo = nullptr;
    m_rowIndex = -1;

    ui = new Ui::QQcRegSample();
    ui->setupUi(this);
    ui->label_errorshow->setWordWrap(true);
    m_module = DIALOGMODULE::MANNUAL;
    // 取消
    connect(ui->buttonQcCancel, SIGNAL(clicked()), this, SLOT(OnCancalButton()));
    // 确认
    connect(ui->buttonQcOk, SIGNAL(clicked()), this, SLOT(OnSaveQcDoc()));

    // 编辑质控名称时长度限制输入
    connect(ui->name_Edit, SIGNAL(textEdited(const QString&)), this, SLOT(OnQcNameTextEditted(const QString&)));

    // 根据设备类型不同对界面的初始化
    InitByDiffDivece();

    // 二维码长度值
    m_fixedBarcodeLength = DictionaryQueryManager::GetInstance()->GetQcBarcodeLengthConfig();
    m_fixLengthSn = DictionaryQueryManager::GetInstance()->GetQcSnLengthConfig();
    m_fixLengthLot = DictionaryQueryManager::GetInstance()->GetQcLotLengthConfig();

    // 初始化水平下拉框
    int levelCnt = DictionaryQueryManager::GetInstance()->GetQcLevelConfig();
    for (int ev = 1; ev <= levelCnt; ++ev)
    {
        ui->level_Combo->addItem(QString::number(ev), ev);
    }

    // 限制批号瓶号长度
    QString strRegSn = QString("[\\d]{%1}").arg(m_fixLengthSn);
    QString stdRegLot = QString("[\\d]{%1}").arg(m_fixLengthLot);
    ui->sn_Edit->setValidator(new QRegExpValidator(QRegExp(strRegSn), this));
    ui->lot_Edit->setValidator(new QRegExpValidator(QRegExp(stdRegLot), this));
    ui->shortName_Edit->setValidator(new QRegExpValidator(QRegExp("[a-z,A-Z,\\d]{0,10}"), this));
    ui->name_Edit->setValidator(new QRegExpValidator(QRegExp("\\S+"), this));

    ui->bar_Edit->setAttribute(Qt::WA_InputMethodEnabled, false);
    ui->buttonQcOk->setFocusPolicy(Qt::NoFocus);
    ui->buttonQcCancel->setFocusPolicy(Qt::NoFocus);
}

QQcRegSample::~QQcRegSample()
{
}

void QQcRegSample::InitTableView(QTableView* tbView, DIALOGMODULE dlgType)
{
    ULOG(LOG_INFO, __FUNCTION__);

    tbView->setSelectionBehavior(QAbstractItemView::SelectRows);
    tbView->horizontalHeader()->setHighlightSections(false);
    tbView->verticalHeader()->setVisible(false);

    InitLineEdit(tbView, COL_TARGET, ValidateCompositionInput); // 靶值
    InitLineEdit(tbView, COL_SD, ValidateCompositionInput);     // sd
    InitLineEdit(tbView, COL_CV, ValidateCompositionInput); // cv
}

void QQcRegSample::InitByDiffDivece()
{
    SOFTWARE_TYPE devType = CommonInformationManager::GetInstance()->GetSoftWareType();

    if (devType == SOFTWARE_TYPE::IMMUNE || devType == SOFTWARE_TYPE::CHEMISTRY)
    {
        // 只有免疫(移除生化成分页签:0);只有生化(移除免疫成分页签:1)
        int tabIdx = devType == SOFTWARE_TYPE::IMMUNE ? 0 : 1;
        ui->tabWidget_qcReg->removeTab(tabIdx);

        // 隐藏QTabBar
        QList<QTabBar*> qtabBar = ui->tabWidget_qcReg->findChildren<QTabBar*>();
        if (qtabBar.size() > 0)
        {
            qtabBar[0]->hide();
        }

        // 上边线上移,左右边线上移
        ui->line_h_3->move(ui->line_h_3->x(), 5);
        ui->line_v_2->move(ui->line_v_2->x(), 5);
        ui->line_v_2->setFixedHeight(654);
        ui->line_v_3->move(ui->line_v_3->x(), 5);
        ui->line_v_3->setFixedHeight(654);

        // 调整表格高度
        ui->tableView_chim->setFixedHeight(655);
        ui->tableView_immy->setFixedHeight(655);
    }

    // 初始化样本类型下拉框(如果存在生化设备，则多添加 脑脊液、浆膜腔积液 两种类型)
    UiCommon::Instance()->AssignSampleCombox(ui->qcType_Combo);
}

void QQcRegSample::UpdateComposition(QTableView* tb, std::shared_ptr<QQcAssayEditModel>& cModel, \
    const std::vector<int32_t>& assayCodes, const std::vector<tf::QcComposition>& qcCompositions)
{
    if (cModel == nullptr)
    {
        QcSampleModelDevType cqAssayType = (tb == ui->tableView_chim) ? QmCh : QmIm;
        cModel = std::shared_ptr<QQcAssayEditModel>(new QQcAssayEditModel(cqAssayType));
        cModel->setHeaderData((int)QQcAssayModel::COLASSAY::TARGETVALUE, Qt::Horizontal, QBrush(UI_TEXT_COLOR_HEADER_MARK), Qt::TextColorRole);
        cModel->setHeaderData((int)QQcAssayModel::COLASSAY::STANDARD, Qt::Horizontal, QBrush(UI_TEXT_COLOR_HEADER_MARK), Qt::TextColorRole);
        cModel->setHeaderData((int)QQcAssayModel::COLASSAY::CV, Qt::Horizontal, QBrush(UI_TEXT_COLOR_HEADER_MARK), Qt::TextColorRole);
        tb->setModel(cModel.get());
    }
    cModel->Init(assayCodes);
    cModel->SetData(qcCompositions);
    tb->viewport()->update();
    ResizeTblColToContent(tb);
}

void QQcRegSample::InitCompositionTableMode(const std::vector<tf::QcComposition>& qcCompositions)
{
    ULOG(LOG_INFO, "%s, compostion list size is:%d.", __FUNCTION__, qcCompositions.size());

    std::vector<int32_t> chimAssayCode;
    std::vector<int32_t> immyAssayCode;
    TakeChimAndImmyAssayCode(chimAssayCode, immyAssayCode);

    // 生化
    UpdateComposition(ui->tableView_chim, m_CompositionModeChim, chimAssayCode, qcCompositions);

    // 免疫
    UpdateComposition(ui->tableView_immy, m_CompositionModeImmy, immyAssayCode, qcCompositions);


    // 提示输入的项目未匹配到项目列表assayCode的通道号
    std::set<int32_t> totalAssayCode;
    for (int a : chimAssayCode)
    {
        totalAssayCode.insert(a);
    }
    for (int a : immyAssayCode)
    {
        totalAssayCode.insert(a);
    }
    std::vector<int> unmatched;
    for (const auto& cp : qcCompositions)
    {
        if (totalAssayCode.find(cp.assayCode) == totalAssayCode.end())
        {
            unmatched.push_back(cp.assayCode);
        }
    }

    if (m_uiBarInfo != nullptr)
    {
        m_uiBarInfo->m_notExistAssayCodes = unmatched;
    }
}

void QQcRegSample::TakeChimAndImmyAssayCode(std::vector<int32_t>& chimCode, std::vector<int32_t>& immyCode)
{
    ULOG(LOG_INFO, __FUNCTION__);

    std::shared_ptr<CommonInformationManager> assayMgr = CommonInformationManager::GetInstance();

    // 排除掉找不到名字的项目
    auto funcAssignCode = [&](const std::vector<int32_t>& srcVec, std::vector<int32_t>& outputCode) {
        outputCode.clear();
        for (int32_t iAssay : srcVec)
        {
            if (assayMgr->GetAssayInfo(iAssay) != nullptr)
            {
                outputCode.push_back(iAssay);
            }
        }
    };

    // 获取生化的项目编号
    std::vector<int32_t> tempChim;
    assayMgr->GetAssayCodeListCh(tf::DeviceType::DEVICE_TYPE_C1000, tempChim);
    assayMgr->GetAssayCodeListIse(tf::DeviceType::DEVICE_TYPE_ISE1005, tempChim);
    funcAssignCode(tempChim, chimCode);

    // 获取免疫的项目编号
    std::vector<int32_t> tempImm;
    assayMgr->GetAssayCodeListIm(tf::DeviceType::DEVICE_TYPE_I6000, tempImm);
    funcAssignCode(tempImm, immyCode);
}

bool QQcRegSample::StartModify(int rowIndex, const std::shared_ptr<tf::QcDoc>& curQcDoc)
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);

    if (curQcDoc == nullptr)
    {
        return false;
    }

    m_qcDoc = curQcDoc;
    m_rowIndex = rowIndex;
    m_module = DIALOGMODULE::MODIFY;

    ui->name_Edit->setText(QString::fromStdString(m_qcDoc->name));
    ui->shortName_Edit->setText(QString::fromStdString(m_qcDoc->shortName));
    ui->level_Combo->setCurrentIndex(ui->level_Combo->findData(m_qcDoc->level));

    // 如果是4位SN，则只显示前三位，不显示末尾的level部分
    QString strSn = QString::fromStdString(m_qcDoc->sn);
    ui->sn_Edit->setText(strSn.size() > m_fixLengthSn ? strSn.mid(0, m_fixLengthSn) : strSn);

    ui->lot_Edit->setText(QString::fromStdString(m_qcDoc->lot));
    ui->qcType_Combo->setCurrentIndex(ui->qcType_Combo->findData(m_qcDoc->sampleSourceType));

    ui->label->hide();
    ui->bar_Edit->hide();

    DisableNScanEdit(true);

    // 对成分表进行初始化
    InitTableView(ui->tableView_chim, m_module);
    InitTableView(ui->tableView_immy, m_module);

    QDateTime expDateTime = QDateTime::fromString(QString::fromStdString(m_qcDoc->expireTime), UI_DATE_TIME_FORMAT);
    ui->expire_Edit->setDate(expDateTime.date());
    InitCompositionTableMode(m_qcDoc->compositions);

    // 显示当前设备页面对应的项目tab
    ShowCurrentDeviceTab();

    return true;
}

void QQcRegSample::ClearQcbaseInfoUi()
{
    ULOG(LOG_INFO, __FUNCTION__);

    ui->name_Edit->setText("");
    ui->shortName_Edit->setText("");
    ui->level_Combo->setCurrentIndex(-1);
    ui->sn_Edit->setText("");
    ui->lot_Edit->setText("");
    ui->qcType_Combo->setCurrentIndex(-1);
    ui->label_errorshow->setText("");
    ui->expire_Edit->setNull();
}

void QQcRegSample::DisableNScanEdit(bool isSetDisable)
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 防止切换扫描手工下载时，timer没有被停止的情况
    if (m_barCodeTimer != nullptr && m_barCodeTimer->isActive())
    {
        m_barCodeTimer->stop();
        ChangeScanEffectedFocs(Qt::StrongFocus);
    }
    ui->buttonQcOk->setEnabled(true);
    ui->buttonQcCancel->setEnabled(true);

    auto funcDisAll = [this](bool disCtr) {
        ui->name_Edit->setDisabled(disCtr);
        //ui->shortName_Edit->setDisabled(disCtr);
        ui->level_Combo->setDisabled(disCtr);
        ui->sn_Edit->setDisabled(disCtr);
        ui->lot_Edit->setDisabled(disCtr);
        ui->qcType_Combo->setDisabled(disCtr);
        ui->expire_Edit->setDisabled(disCtr);
    };
    funcDisAll(isSetDisable);

    // 如果选中行有质控品信息
    if (m_qcDoc && m_module == DIALOGMODULE::MODIFY)
    {
        // 若选中质控品是扫描二维码/网络下载添加的，则只有名称、简称可修改
        if (m_qcDoc->registType == ::tf::DocRegistType::REG_SCAN ||
            m_qcDoc->registType == ::tf::DocRegistType::REG_DOWNLOAD)
        {
            ui->name_Edit->setDisabled(false);
            ui->shortName_Edit->setDisabled(false);
        }

        // 若选中质控品是手动添加的
        if (m_qcDoc->registType == ::tf::DocRegistType::REG_MANUAL)
        {
            // 若其已用其进行过质控，则只有名称、简称、失效日期可修改
            tf::SampleInfoQueryCond cond;
            cond.__set_qcDocId(m_qcDoc->id);
            cond.__set_statuses({ tf::SampleStatus::SAMPLE_STATUS_TESTING, tf::SampleStatus::SAMPLE_STATUS_TESTED });
            tf::SampleInfoQueryResp resp;
            if (DcsControlProxy::GetInstance()->QuerySampleInfo(cond, resp) && resp.lstSampleInfos.size() > 0)
            {
                ui->name_Edit->setDisabled(false);
                ui->shortName_Edit->setDisabled(false);
                ui->expire_Edit->setDisabled(false);
            }
            // 若其未用其进行过质控，则编号、名称、简称、类型、水平、批号、失效日期均可修改
            else
            {
                funcDisAll(false);
            }
        }
    }

    ui->label_errorshow->setText("");
}

///
/// @brief 进行新质控品的添加操作
///
/// @param[in]  newDoc  
///
/// @return 
///
/// @par History:
/// @li 1556/Chenjianlin，2023年7月29日，新建函数
///
int64_t QQcRegSample::ProcessQcDocAdd(const tf::QcDoc& newDoc)
{
    ULOG(LOG_INFO, __FUNCTION__);

    int64_t vKey = -1;
    // 如果是在原有质控品选中的情况下添加质控品
    if (m_qcDoc)
    {
        if (m_qcDoc->sn != newDoc.sn)
        {
            TipDlg(tr("当前质控品与修改后的质控品的编号存在差异，不能执行覆盖操作！")).exec();
            return -1;
        }
        tf::QcDoc tempDoc = newDoc;
        tempDoc.__set_id(m_qcDoc->id);
        tf::ResultLong ret;
        if (!DcsControlProxy::GetInstance()->ModifyQcDoc(ret, tempDoc) || ret.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            TipDlg(tr("覆盖操作执行失败！")).exec();
            return -1;
        }
        // 添加操作日志
        COperationLogManager::GetInstance()->AddQCOperationLog(::tf::OperationType::type::MOD, newDoc, m_qcDoc);
        vKey = m_qcDoc->id;
    }
    else
    {
        tf::ResultLong ret;
        DcsControlProxy::GetInstance()->AddQcDoc(ret, newDoc);
        if (ret.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            TipDlg(tr("新增质控品操作执行失败！")).exec();
            return -1;
        }
        // 添加操作日志
        COperationLogManager::GetInstance()->AddQCOperationLog(::tf::OperationType::type::ADD, newDoc);
        vKey = ret.value;
    }

    return vKey;
}

void QQcRegSample::ShowCurrentDeviceTab()
{
    auto deviceMap = CommonInformationManager::GetInstance()->GetDeviceMaps();
    for (auto it = deviceMap.begin(); it != deviceMap.end(); ++it)
    {
        if (it->second->deviceClassify == tf::AssayClassify::ASSAY_CLASSIFY_OTHER)
        {
            continue;
        }
        int currTab = it->second->deviceClassify == tf::AssayClassify::ASSAY_CLASSIFY_IMMUNE ? 1 : 0;
        ui->tabWidget_qcReg->setCurrentIndex(currTab);
        break;
    }
}

bool QQcRegSample::StartManual(int rowIndex, const std::shared_ptr<tf::QcDoc>& curQcDoc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    m_qcDoc = curQcDoc;
    m_rowIndex = rowIndex;

    m_module = DIALOGMODULE::MANNUAL;

    ui->label->hide();
    ui->bar_Edit->hide();

    DisableNScanEdit(false);
    ClearQcbaseInfoUi();

    // 对成分表进行初始化
    InitTableView(ui->tableView_chim, m_module);
    InitTableView(ui->tableView_immy, m_module);

    std::vector<::tf::QcComposition> emptyData;
    InitCompositionTableMode(emptyData);

    // 显示当前设备页面
    ShowCurrentDeviceTab();

    // 设置焦点默认位置为编号框
    ui->sn_Edit->setFocus();

    return true;
}

void QQcRegSample::PaserScanBarCodeToUi(const QString& barCode)
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 已固定长度分割粘连的码，挨个解析
    int curPosition = 0;
    QString strCurBar;
    do
    {
        strCurBar = barCode.mid(curPosition, m_fixedBarcodeLength);
        curPosition += strCurBar.size();
        if (strCurBar.isEmpty())
        {
            break;
        }
        PaserScanBarCodeOneBar(strCurBar);
    } while (!strCurBar.isEmpty());

    InitCompositionTableMode(m_uiBarInfo->GetCompositions());
}

void QQcRegSample::PaserScanBarCodeOneBar(const QString& strOneBar)
{
    ULOG(LOG_INFO, __FUNCTION__);

    if (strOneBar.size() != m_fixedBarcodeLength )
    {
        ULOG(LOG_WARN, "Barcode length invalid, %d/%d.", strOneBar.size(), m_fixedBarcodeLength);
        return;
    }
    ChangeScanEffectedFocs(Qt::StrongFocus);

    // 解析条码并填充界面框
    ::tf::QcBarCodeInfo qcDoc;
    if (!ParseScanBarCode(strOneBar, qcDoc))
    {
        ClearQcbaseInfoUi();
        ui->label_errorshow->setText(tr("二维码解析错误！"));
        ui->label_errorshow->setStyleSheet("color:red;font-size:16;");

        ui->bar_Edit->clear();
        InitCompositionTableMode({});

        return;
    }

    ui->label_errorshow->setText(tr("扫描成功！"));
    ui->label_errorshow->setStyleSheet("color:green;font-size:16;");

    // BUG0024739,扫描成功后需要清空条码框
    ui->bar_Edit->clear();

    // 汇聚二维码中的composition
    if (m_uiBarInfo == nullptr)
    {
        m_uiBarInfo = std::shared_ptr<UiBarCodeInfo>(new UiBarCodeInfo());
    }
    m_uiBarInfo->AddItem(qcDoc);
}

bool QQcRegSample::ParseScanBarCode(const QString& barCode, ::tf::QcBarCodeInfo &qcDoc)
{
    ULOG(LOG_INFO, "%s, barcode:%s", __FUNCTION__, barCode.toStdString());

    // 条码解析接口调用
    if (!DcsControlProxy::GetInstance()->ParseQCBarcodeInfo(qcDoc, barCode.toStdString()))
    {
        ULOG(LOG_ERROR, "Failed to pharse qc barcodeinfo.");
        return false;
    }

    // 把解析的质控信息填写入界面
    ::tf::QcDoc &curQcDoc = qcDoc.docInfo;
    if (curQcDoc.__isset.name)
    {
        ui->name_Edit->setText(QString::fromStdString(curQcDoc.name));
    }
    if (curQcDoc.__isset.shortName)
    {
        ui->shortName_Edit->setText(QString::fromStdString(curQcDoc.shortName));
    }
    if (curQcDoc.__isset.sampleSourceType)
    {
        ui->qcType_Combo->setCurrentIndex(ui->qcType_Combo->findData(curQcDoc.sampleSourceType));
    }
    if (curQcDoc.__isset.level)
    {
        ui->level_Combo->setCurrentIndex(ui->level_Combo->findData(curQcDoc.level));
    }
    if (curQcDoc.__isset.sn)
    {
        // 原始SN如果是4位，则是3 SN + 1 Leve
        QString origSn = QString::fromStdString(curQcDoc.sn);
        ui->sn_Edit->setText(origSn.mid(0, m_fixLengthSn));
        if (origSn.length() > m_fixLengthSn)
        {
            ui->level_Combo->setCurrentIndex(ui->level_Combo->findData(origSn.mid(m_fixLengthSn).toInt()));
        }
    }
    if (curQcDoc.__isset.lot)
    {
        ui->lot_Edit->setText(QString::fromStdString(curQcDoc.lot));
    }
    if (curQcDoc.__isset.expireTime)
    {
        QDateTime expDateTime = QDateTime::fromString(QString::fromStdString(curQcDoc.expireTime), UI_DATE_TIME_FORMAT);
        ui->expire_Edit->setDate(expDateTime.date());
    }

    return true;
}

void QQcRegSample::InitScanEnvr()
{
    ULOG(LOG_INFO, __FUNCTION__);

    m_preBarcodeText.clear();
    ui->bar_Edit->clear();
    ui->label_errorshow->setText(tr(""));
    ui->label_errorshow->setStyleSheet("color:green;font-size:16;");
    ui->bar_Edit->setFocusPolicy(Qt::StrongFocus);

    // 条码框Timer
    m_barCodeTimer = std::shared_ptr<QTimer>(new QTimer(this));
    m_barCodeTimer->setInterval(400);
    connect(m_barCodeTimer.get(), SIGNAL(timeout()), this, SLOT(OnBarCodeTimerEvent()));
    m_barCodeTimer->start();
    ChangeScanEffectedFocs(Qt::NoFocus);

    connect(ui->bar_Edit, SIGNAL(textChanged(const QString&)), this, SLOT(OnBarCodeEditChanged(const QString&)));
    connect(ui->bar_Edit, SIGNAL(editingFinished()), this, SLOT(OnBarFinished()));
}

bool QQcRegSample::StartScan(int rowIndex, const std::shared_ptr<tf::QcDoc>& curQcDoc)
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);

    m_rowIndex = rowIndex;
    m_qcDoc = curQcDoc;
    m_module = DIALOGMODULE::SCAN;

    // 重置用于存储多二维码的结构
    m_uiBarInfo = std::shared_ptr<UiBarCodeInfo>(new UiBarCodeInfo(curQcDoc));

    ClearQcbaseInfoUi();

    ui->bar_Edit->show();
    ui->bar_Edit->setFocus();
    ui->bar_Edit->setText("");
    ui->label->show();

    // 禁用除扫描框外的输入控件
    DisableNScanEdit(true);

    // 初始化扫描相关信号
    InitScanEnvr();

    // 对成分表进行初始化
    InitTableView(ui->tableView_chim, m_module);
    InitTableView(ui->tableView_immy, m_module);

    InitCompositionTableMode(m_uiBarInfo->GetCompositions());

    // 显示当前设备页面
    ShowCurrentDeviceTab();

    return true;
}

tf::DocRegistType::type QQcRegSample::GetCurrentRegistType()
{
    switch (m_module)
    {
    case QQcRegSample::DIALOGMODULE::MANNUAL:
        return ::tf::DocRegistType::REG_MANUAL;
    case QQcRegSample::DIALOGMODULE::SCAN:
        return ::tf::DocRegistType::REG_SCAN;
    case QQcRegSample::DIALOGMODULE::MODIFY:
        break;
    default:
        break;
    }

    return m_qcDoc ? m_qcDoc->registType : ::tf::DocRegistType::REG_MANUAL;
}

bool QQcRegSample::CheckRepeatQc(const ::tf::QcDoc& saveDoc)
{
    ULOG(LOG_INFO, __FUNCTION__);

    tf::QcDocQueryResp resp;
    tf::QcDocQueryCond cond;
    cond.__set_lot(saveDoc.lot);
    cond.__set_sn(saveDoc.sn);
    cond.__set_level(saveDoc.level);
    if (DcsControlProxy::GetInstance()->QueryQcDoc(resp, cond) && resp.result == tf::ThriftResult::THRIFT_RESULT_SUCCESS && \
        resp.lstQcDocs.size() > 0)
    {
        // 当编辑行原来为空，或原来的id非编辑后的id，则认为是重复了
        if (m_qcDoc == nullptr || \
            (m_qcDoc->__isset.id && resp.lstQcDocs[0].__isset.id && m_qcDoc->id != resp.lstQcDocs[0].id))
        {
            TipDlg(tr("相同编号、批号和水平的质控品信息已存在，请重新输入！")).exec();
            return false;
        }
    }

    return true;
}

std::string QQcRegSample::CombineAssayCodes(const std::vector<int>& vecAssayCode)
{
#define TEMP_STRING_BUFF_LENGTH 32

    std::string strCodes;
    int iCount = 0;
    for (int iCode : vecAssayCode)
    {
        char temp[TEMP_STRING_BUFF_LENGTH] = { 0 };
        sprintf_s(temp, TEMP_STRING_BUFF_LENGTH, "%d,", iCode);
        strCodes += std::string(temp);

        // 每十个编号换一行
        if (++iCount % 10 == 0)
        {
            strCodes.pop_back();
            strCodes += "\n";
        }
    }

    // 去掉最后一个多余的分割符
    if (strCodes.length() > 0)
    {
        strCodes.pop_back();
    }

    return strCodes;
}

bool QQcRegSample::CheckCompositionCondition(const ::tf::QcDoc& newQcDoc)
{
    if (m_qcDoc == nullptr || \
        m_module != QQcRegSample::DIALOGMODULE::MODIFY || \
        !CommonInformationManager::GetInstance()->IsExistDeviceRuning())
    {
        return true;
    }
    // 判断composition
    auto funcCompareComposition = [](const tf::QcComposition& compA, const tf::QcComposition& compB)->bool {
        return (compA.assayCode == compB.assayCode && IsEqual(compA.sd, compB.sd) && IsEqual(compA.targetValue, compB.targetValue) && \
            compA.autoCtrlSet == compB.autoCtrlSet);
    };

    // 如果composition存在变化，则不能修改
    if (m_qcDoc->compositions.size() != newQcDoc.compositions.size())
    {
        return false;
    }
    else
    {
        for (int i = 0; i < m_qcDoc->compositions.size(); ++i)
        {
            if (!funcCompareComposition(m_qcDoc->compositions[i], newQcDoc.compositions[i]))
            {
                return false;
            }
        }
    }

    return true;
}

void QQcRegSample::TakeQcDocInfoFromUI(::tf::QcDoc& saveQcDoc, std::vector<std::string>& notComplate)
{
    QDateTime expDateTime(ui->expire_Edit->date());
    saveQcDoc.__set_name(ui->name_Edit->text().toStdString());
    saveQcDoc.__set_shortName(ui->shortName_Edit->text().toStdString());
    saveQcDoc.__set_level(ui->level_Combo->currentData().toInt());

    // 如果SN为3位，则需要把level加在末尾
    QString strSn = ui->sn_Edit->text();
    if (strSn.size() == m_fixLengthSn)
    {
        strSn += ui->level_Combo->currentData().toString();
    }
    saveQcDoc.__set_sn(strSn.toStdString());

    saveQcDoc.__set_lot(ui->lot_Edit->text().toStdString());
    saveQcDoc.__set_sampleSourceType(ui->qcType_Combo->currentData().toInt());
    saveQcDoc.__set_expireTime(expDateTime.toString(UI_DATE_TIME_FORMAT).toStdString());
    saveQcDoc.__set_registType(GetCurrentRegistType());

    if (m_qcDoc)
    {
        saveQcDoc.__set_index(m_qcDoc->index);
        saveQcDoc.__set_id(m_qcDoc->id);
    }
    else
    {
        saveQcDoc.__set_index(m_rowIndex);
    }

    // 存储一维码
    std::string strOneCode = "8" + saveQcDoc.sn + saveQcDoc.lot + std::to_string(saveQcDoc.level);
    saveQcDoc.__set_oneDimensionalCode(strOneCode);

    // 成分信息
    std::vector<tf::QcComposition> curCompositions;
    if (m_CompositionModeChim != nullptr)
    {
        m_CompositionModeChim->GetCompostion(curCompositions, notComplate);
    }
    if (m_CompositionModeImmy != nullptr)
    {
        m_CompositionModeImmy->GetCompostion(curCompositions, notComplate);
    }
    saveQcDoc.__set_compositions(curCompositions);
}

void QQcRegSample::PostUpdateDocInfo(int64_t keyValue)
{
    std::vector<tf::QcDocUpdate> vecQcDocUpdate;
    tf::QcDocUpdate qcDocUpdate;
    qcDocUpdate.__set_index(m_rowIndex);
    qcDocUpdate.__set_db(keyValue);

    if (m_module == DIALOGMODULE::SCAN || m_module == DIALOGMODULE::MANNUAL)
    {
        qcDocUpdate.__set_updateType(tf::UpdateType::UPDATE_TYPE_ADD);
    }
    else if (m_module == DIALOGMODULE::MODIFY)
    {
        qcDocUpdate.__set_updateType(tf::UpdateType::UPDATE_TYPE_MODIFY);
    }
    vecQcDocUpdate.push_back(qcDocUpdate);
    POST_MESSAGE(MSG_ID_QC_DOC_INFO_UPDATE, vecQcDocUpdate);
}

bool QQcRegSample::CheckBarcodeInfoValid()
{
    if (m_module == QQcRegSample::DIALOGMODULE::SCAN && m_uiBarInfo)
    {
        // 检查条码扫描模式下，还存在未扫描的条码
        if (!m_uiBarInfo->FullScaned())
        {
            TipDlg(tr("该质控品存在多个二维码，请扫描完所有二维码再保存该质控品信息！")).exec();
            return false;
        }

        // 提示不存在的项目编号
        if (m_uiBarInfo->m_notExistAssayCodes.size() > 0)
        {
            std::string strCodes = CombineAssayCodes(m_uiBarInfo->m_notExistAssayCodes);
            QString strMsg = tr("未找到项目通道号为%1 的测试项目，是否继续添加存在的测试项目?").arg(QString::fromStdString(strCodes));
            TipDlg tipMsg(strMsg, TipDlgType::TWO_BUTTON);
            if (tipMsg.exec() == QDialog::Rejected)
            {
                return false;
            }
        }
    }

    return true;
}

///
/// @bref 保存前ISE特殊检测
///		  ISE(包含Na, K, CL项目)样本类型只支持“血清血浆”或“尿液”
///
/// @par History:
/// @li 8580/GongZhiQiang, 2023年11月10日，新建函数
///
bool QQcRegSample::IseSpecialCheck(const ::tf::QcDoc& saveQcDoc)
{
    // 遍历检查
    bool isContansIse = false;
    for (const auto& qDC : saveQcDoc.compositions)
    {
        if (qDC.assayCode == ise::tf::g_ise_constants.ASSAY_CODE_NA ||
            qDC.assayCode == ise::tf::g_ise_constants.ASSAY_CODE_K ||
            qDC.assayCode == ise::tf::g_ise_constants.ASSAY_CODE_CL)
        {
            isContansIse = true;
            break;
        }
    }

    // 检查样本类型
    if (isContansIse &&
        (saveQcDoc.sampleSourceType != ::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_XQXJ &&
            saveQcDoc.sampleSourceType != ::tf::SampleSourceType::SAMPLE_SOURCE_TYPE_NY))
    {
        return false;
    }

    return true;
}

void QQcRegSample::ChangeScanEffectedFocs(Qt::FocusPolicy focusPolicy)
{
    ui->sn_Edit->setFocusPolicy(focusPolicy);
    ui->name_Edit->setFocusPolicy(focusPolicy);
    ui->shortName_Edit->setFocusPolicy(focusPolicy);
    ui->lot_Edit->setFocusPolicy(focusPolicy);
    ui->label_errorshow->setFocusPolicy(focusPolicy);
    ui->tableView_chim->setFocusPolicy(focusPolicy);
    ui->tableView_immy->setFocusPolicy(focusPolicy);
}

///
/// @brief
///     获取文本的字符数
///
/// @param[in]  strText  文本
///
/// @return 文本字符数
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月31日，新建函数
///
int QQcRegSample::GetCharCnt(const QString& strText)
{
    int iRetLen = 0;
    for (const auto& qChar : strText)
    {
        if (qChar.unicode() >= 0x4e00 && qChar.unicode() <= 0x9fa5)
        {
            iRetLen += 2;
        }
        else
        {
            ++iRetLen;
        }
    }

    return iRetLen;
}

void QQcRegSample::OnSaveQcDoc()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 验证空
    if (ui->name_Edit->text().isEmpty()
        || ui->level_Combo->currentText().isEmpty()
        || ui->qcType_Combo->currentText().isEmpty()
        || ui->lot_Edit->text().isEmpty()
        || ui->sn_Edit->text().isEmpty()
		|| ui->expire_Edit->isNull())
    {
        TipDlg(tr("添加质控品信息时编号、名称、类型、水平、批号、失效日期都不能为空，请重新输入！")).exec();
        return;
    }

    // 验证长度
    if (ui->name_Edit->text().length() > QC_MAX_NAME ||
        ui->shortName_Edit->text().length() > QC_MAX_NAME ||
        ui->sn_Edit->text().length() != m_fixLengthSn ||
        ui->lot_Edit->text().length() != m_fixLengthLot)
    {
        TipDlg(tr("添加或修改质控品信息时，需选择水平，编号长度为3个字符，名称长度不能超过30个字符，批号长度为7个字符，请重新输入！")).exec();
        return;
    }

    // 验证二维码输入完整性
    if (!CheckBarcodeInfoValid())
    {
        return;
    }

    // 构造用于存储的质控品对象
    ::tf::QcDoc saveQcDoc;
    if (m_module == QQcRegSample::DIALOGMODULE::MODIFY && m_qcDoc != nullptr)
    {
        saveQcDoc = *m_qcDoc;
    }
    std::vector<std::string> notComplate;
    TakeQcDocInfoFromUI(saveQcDoc, notComplate);
    if (notComplate.size() > 0)
    {
        TipDlg temDlg(tr("确认"), tr("项目：%1 的靶值、标准差或CV%填写不完整，如果继续保存，不完整的信息将丢失，是否继续保存？")
            .arg(QString::fromStdString(GetContainerJoinString(notComplate, "、"))), TipDlgType::TWO_BUTTON);
        if (temDlg.exec() == QDialog::Rejected)
        {
            return;
        }
    }

    // ISE特殊检查，只要包含（Na,K,Cl）其中一个项目，样本类型就必须选择“血清/血浆”或“尿液”
    if (!IseSpecialCheck(saveQcDoc))
    {
        TipDlg(tr("ISE（Na,K,Cl）项目的样本类型只支持“血清/血浆”或“尿液”!")).exec();
        return;
    }

    // 判断是否重复
    if (!CheckRepeatQc(saveQcDoc))
    {
        ULOG(LOG_ERROR, "Repeat qc error.");
        return;
    }

    // 判断运行或暂停时不能改的“靶值/标准差/变异系数”
    if (!CheckCompositionCondition(saveQcDoc))
    {
        TipDlg(tr("运行或暂停时不能修改靶值、标准差、变异系数！")).exec();
        return;
    }

    int64_t keyValue = -1;
    switch (m_module)
    {
    case QQcRegSample::DIALOGMODULE::SCAN:
    case QQcRegSample::DIALOGMODULE::MANNUAL:
    {
        keyValue = ProcessQcDocAdd(saveQcDoc);
        if (keyValue == -1)
        {
            return;
        }
    }
    break;
    case QQcRegSample::DIALOGMODULE::MODIFY:
    {
        tf::ResultLong ret;
        if (!DcsControlProxy::GetInstance()->ModifyQcDoc(ret, saveQcDoc) || ret.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            QString strMsg = ret.result == tf::ThriftResult::THRIFT_RESULT_MODIFY_USEING ? \
                tr("修改失败，修改的质控品正在使用中！") : \
                tr("修改操作执行失败！");
            TipDlg(strMsg).exec();
            return;
        }
        keyValue = saveQcDoc.id;

        // 添加操作日志
        COperationLogManager::GetInstance()->AddQCOperationLog(::tf::OperationType::type::MOD, saveQcDoc, m_qcDoc);
    }
    break;
    default:
        break;
    }

    emit SigClosed();
    emit Update();

    PostUpdateDocInfo(keyValue);
}

void QQcRegSample::OnCancalButton()
{
    DisableNScanEdit(false);
    emit SigClosed();
}

void QQcRegSample::OnBarCodeEditChanged(const QString& barCode)
{
    // 重新开启定时器
    if (!m_barCodeTimer->isActive())
    {
        m_barCodeTimer->start();
        ChangeScanEffectedFocs(Qt::NoFocus);
    }
}

void QQcRegSample::OnBarFinished()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 兼容设置条码枪后缀换行符情况，当有换行结束符条码输入完后，则记录当前结果，清空扫描框为后续其它条码输入准备
    QString currentBar = ui->bar_Edit->text();
    if (!currentBar.isEmpty())
    {
        m_barCodeTimer->stop();
        ChangeScanEffectedFocs(Qt::StrongFocus);

        ui->buttonQcOk->setEnabled(true);
        ui->buttonQcCancel->setEnabled(true);

        // 填充0，尝试解析
        while ((currentBar.size() % m_fixedBarcodeLength) != 0)
        {
            currentBar.append("0");
        }

        PaserScanBarCodeToUi(currentBar);
    }
}

///
/// @brief
///     质控文本被编辑
///
/// @param[in]  strQcDoc  编辑后的质控文本
///
/// @par History:
/// @li 4170/TangChuXian，2024年8月13日，新建函数
///
void QQcRegSample::OnQcNameTextEditted(const QString& strQcDoc)
{
    if (GetCharCnt(strQcDoc) > QC_MAX_NAME)
    {
        ui->name_Edit->backspace();
    }
}

void QQcRegSample::OnBarCodeTimerEvent()
{
    if (m_module != DIALOGMODULE::SCAN)
    {
        return;
    }

    // 如果条码框内容没有变化，则不进行解析
    QString currentBar = ui->bar_Edit->text();
    if (currentBar.isEmpty())
    {
        ui->buttonQcOk->setEnabled(true);
        ui->buttonQcCancel->setEnabled(true);
        return;
    }

    // 扫码框无变化，表示条码录入完成
    static int timeoutCont = 0;
    if (m_preBarcodeText == currentBar)
    {
        if (currentBar.size() % m_fixedBarcodeLength == 0)
        {
            m_barCodeTimer->stop();
        }
        else
        {
            // 如果条码不完整，则尝试等待2.4s后解析
            if (timeoutCont > 6) // 400 * 6, 2.4 s 后退出
            {
                timeoutCont = 0;
                m_barCodeTimer->stop();

                // 填充0，尝试解析
                while ((currentBar.size() % m_fixedBarcodeLength) != 0)
                {
                    currentBar.append("0");
                }
            }
            timeoutCont++;
        }
        
        // 解析条码并填充界面框
        PaserScanBarCodeToUi(currentBar);

        ui->buttonQcOk->setEnabled(true);
        ui->buttonQcCancel->setEnabled(true);
    }
    else
    {
        timeoutCont = 0;
        if ((currentBar.size() % m_fixedBarcodeLength) != 0)
        {
            ui->label_errorshow->setText(tr("正在接收条码[%1]... (%2/%3)").arg(
                (currentBar.size() / m_fixedBarcodeLength) + 1).arg(currentBar.size() % m_fixedBarcodeLength).arg(m_fixedBarcodeLength));
            ui->label_errorshow->setStyleSheet("color:green;font-size:16;");
        }
        else
        {
            PaserScanBarCodeToUi(currentBar);
        }

        ui->buttonQcOk->setEnabled(false);
        ui->buttonQcCancel->setEnabled(false);

        m_preBarcodeText = currentBar;
    }
}

void QQcRegSample::OnTextChanged(const QString& ctrText)
{
    // 获取当前tab页
    bool isChim = ui->tabWidget_qcReg->currentWidget() == ui->tab_qcreg_chim;
    const std::shared_ptr<QQcAssayEditModel>& currModel = isChim ? m_CompositionModeChim : m_CompositionModeImmy;
    QTableView* curTable = isChim ? ui->tableView_chim : ui->tableView_immy;

    currModel->setData(curTable->currentIndex(), ctrText);
    curTable->viewport()->update();
}

UiBarCodeInfo::UiBarCodeInfo(const std::shared_ptr<tf::QcDoc>& curQcDoc)
{
    if (curQcDoc != nullptr)
    {
        m_lot = curQcDoc->lot;
        m_sn = curQcDoc->sn;
        m_level = curQcDoc->level;

        std::shared_ptr<tf::QcBarCodeInfo> origQc(new tf::QcBarCodeInfo);
        origQc->__set_docInfo(*curQcDoc);
        m_barCodes[-1] = origQc;
    }
}

void UiBarCodeInfo::AddItem(const tf::QcBarCodeInfo& qcItem)
{
    ULOG(LOG_INFO, "%s, Lot:%s Sn:%s Level:%d, curPage/toutalPage:%d/%d", __FUNCTION__, \
        qcItem.docInfo.lot, qcItem.docInfo.sn, qcItem.docInfo.level, qcItem.curPage, qcItem.totalPage);

    // 如果添加的质控品与之前的不同，则重置存储结构
    bool sameQc = (qcItem.docInfo.lot == m_lot && qcItem.docInfo.sn == m_sn && qcItem.docInfo.level == m_level);
    if (!sameQc)
    {
        m_barCodes.clear();
        m_lot = qcItem.docInfo.lot;
        m_sn = qcItem.docInfo.sn;
        m_level = qcItem.docInfo.level;
    }

    m_totalPage = qcItem.totalPage;
    m_barCodes[qcItem.curPage] = std::make_shared<tf::QcBarCodeInfo>(qcItem);
}

std::vector<tf::QcComposition> UiBarCodeInfo::GetCompositions()
{
    std::vector<tf::QcComposition> allComposition;
    std::map<int, std::shared_ptr<tf::QcBarCodeInfo>>::iterator it = m_barCodes.begin();
    for (; it != m_barCodes.end(); ++it)
    {
        if (!it->second->docInfo.__isset.compositions)
        {
            continue;
        }
        allComposition.insert(allComposition.end(), \
            it->second->docInfo.compositions.begin(), it->second->docInfo.compositions.end());
    }

    return allComposition;
}

bool UiBarCodeInfo::FullScaned()
{
    ULOG(LOG_INFO, "%s, stored page cnt:%d, total page cnt:%d", __FUNCTION__, m_barCodes.size(), m_totalPage);

    // -1代表传入的原来存在的qcDoc信息
    return m_barCodes.find(-1) == m_barCodes.end() ? m_barCodes.size() == m_totalPage : m_barCodes.size() - 1 == m_totalPage;
}
