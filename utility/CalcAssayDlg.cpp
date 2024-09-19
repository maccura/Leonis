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
/// @file     CalcAssayDlg.cpp
/// @brief    应用--系统--计算项目
///
/// @author   7951/LuoXin
/// @date     2022年10月25日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2022年10月25日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "CalcAssayDlg.h"
#include "ui_CalcAssayDlg.h" 
#include <cfloat>
#include <QRegExp>
#include <QDateTime>
#include <QCheckBox>
#include <QMouseEvent>
#include <QStandardItemModel>
#include <QFontMetrics>
#include "RangParaSetWidget.h"

#include "shared/tipdlg.h"
#include "shared/uidef.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/uicommon.h"
#include "shared/CommonInformationManager.h"
#include "shared/ThriftEnumTransform.h"

#include "src/thrift/ch/gen-cpp/ch_constants.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/defs.h"
#include "manager/UserInfoManager.h"
#include "manager/OperationLogManager.h"

#define SAMPLE_ROLE                         (Qt::UserRole + 1)  // 缓存样本类型
#define ASSAY_PREFIX					    "assay"			    // 公式中项目前缀
#define ASSAY_COUNT						    (36)			    // 项目按钮一页的个数

Q_DECLARE_METATYPE(::tf::AssayReferenceItem);

// 页面索引
enum TableWidgetIndex
{
    CALC_FORMULA_INDEX,					// 计算公式页面
    REF_RANGE_INDEX 				    // 参考区间页面
};

// 计算项目表格各信息所在列
enum CalcTableViewColunm
{
    CALC_COLUNM_NUM,					// 序号
    CALC_COLUNM_NAME,					// 名称
    CALC_COLUNM_ID,						// id
    CALC_COLUNM_CODE				    // code
};

// 参考区间表格各信息所在列
enum RefTableViewColunm
{
    REF_COLUNM_NUM,						// 序号
    REF_COLUNM_AUTO,					// 是否缺省
    REF_COLUNM_SAMPLE,					// 样本类型
    REF_COLUNM_LOW,						// 低值
    REF_COLUNM_UP,						// 高值
    REF_COLUNM_REF,						// 参考范围
    REF_COLUNM_LOW_CRISIS,				// 危急低值
    REF_COLUNM_UP_CRISIS,				// 危急高值
    REF_COLUNM_GENDER,					// 性别
    REF_COLUNM_AGE_LOW,					// 年龄下限
    REF_COLUNM_AGE_UP,					// 年龄上限
    REF_COLUNM_UNIT						// 单位
};

// 当发生错误时，弹出消息框后退出当前函数
#define ReportErr(bErr, msg) \
if ((bErr))\
{\
	TipDlg(msg).exec();\
	return false;\
}

CalcAssayDlg::CalcAssayDlg(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::CalcAssayDlg)
    , m_calcMode(new QStandardItemModel)
    , m_refMode(new QStandardItemModel)
    , m_currentSampleSource(-1)
    , m_rangParaSetWidget(new CRangParaSetWidget(CALC, this))
{
    ui->setupUi(this);
    m_assayMgr = CommonInformationManager::GetInstance();
    InitCtrls();
}

CalcAssayDlg::~CalcAssayDlg()
{

}

void CalcAssayDlg::LoadDataToCtrls()
{
    // 清理控件中的数据
    ClearCtrls();

	// 清理项目列表
	m_calcMode->removeRows(0, m_calcMode->rowCount());

    // 加载普通项目信息
    LoadAssayData();

    // 加载计算项目信息到控件
    LoadCalcAssayToTableview();

    ui->assay_name_edit->setFocus();
}

void CalcAssayDlg::showEvent(QShowEvent *event)
{
    QWidget::showEvent(event);
 
    // 清理上次选中的项目名称
    m_lastAssayName.clear();

    LoadDataToCtrls();
}

void CalcAssayDlg::InitCtrls()
{
    // 初始化样本类型下拉框	
    ui->sample_type_cmbox->addItem(tr("全部"), -1);
    UiCommon::Instance()->AssignSampleCombox(ui->sample_type_cmbox);

    // 设置表格的model
    m_calcMode->setHorizontalHeaderLabels({ tr("序号"), tr("项目名"), ("id"), ("code")});
    m_refMode->setHorizontalHeaderLabels({ tr("序号") ,tr("是否缺省") ,tr("样本类型"),
        tr("低值") ,tr("高值") ,tr("参考值范围"),tr("危急低值"),tr("危急高值")
        ,tr("性别") ,tr("年龄下限") ,tr("年龄上限"),tr("单位") });

    ui->tableView_calc->setModel(m_calcMode);
	ui->tableView_calc->setColumnWidth(0, 75);
	ui->tableView_calc->horizontalHeader()->setStretchLastSection(true);

    ui->tableView_ref->setModel(m_refMode);
    ui->tableView_ref->horizontalHeader()->setStretchLastSection(true);
	ui->tableView_ref->setColumnWidth(0, 90);
	ui->tableView_ref->setColumnWidth(1, 115);
	ui->tableView_ref->setColumnWidth(2, 160);
	ui->tableView_ref->setColumnWidth(3, 90);
	ui->tableView_ref->setColumnWidth(4, 90);
	ui->tableView_ref->setColumnWidth(6, 160);
	ui->tableView_ref->horizontalHeader()->setMinimumSectionSize(90);

    // 保存按钮
    connect(ui->flat_save, &QPushButton::clicked, this, [&] {
        if (m_assayMgr->IsExistDeviceRuning())
        {
            TipDlg(tr("仪器在运行或暂停状态时不能新增或修改计算项目！")).exec();
            return;
        }

        OnSaveBtnClicked();
    });

    // 新增按钮
    connect(ui->flat_new_add, SIGNAL(clicked()), this, SLOT(OnAddBtnClicked()));
    // 删除按钮
    connect(ui->del_Button, SIGNAL(clicked()), this, SLOT(OnDeleteBtnClicked()));
    // 新增参考区间按钮
    connect(ui->add_ref_btn, SIGNAL(clicked()), this, SLOT(OnAddRefBtnClicked()));
    // 删除参考区间按钮
    connect(ui->del_ref_btn, &QPushButton::clicked, this, &CalcAssayDlg::OnDeleteRefBtnClicked);
    // 修改参考区间按钮
    connect(ui->modify_ref_btn, SIGNAL(clicked()), this, SLOT(OnModifyRefBtnClicked()));
    // 参考区间保存按钮被点击
    connect(m_rangParaSetWidget->getSaveBtn(), SIGNAL(clicked()), this, SLOT(OnUpdateRefRange()));
    // 计算项目信息表格选中行改变
    connect(ui->tableView_calc->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
        this, SLOT(OnSelectRowChanged(QItemSelection, QItemSelection)));
    // 参考区间表格选中行改变
    connect(ui->tableView_ref->selectionModel(), SIGNAL(selectionChanged(QItemSelection, QItemSelection)),
        this, SLOT(OnTabelSelectChanged(QItemSelection, QItemSelection)));

    // 与需求确认，参考范围页面不显示保存按钮
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, [&] (int index){
        ui->flat_save->setVisible(REF_RANGE_INDEX != index);

    });

    // 项目类型（生化、免疫）radio button按钮事件
    connect(ui->radioButtonChim, &QRadioButton::toggled, this, [&] {LoadAssayData(); });
    connect(ui->radioButtonImmy, &QRadioButton::toggled, this, [&] {LoadAssayData(); });
    // 项目界面列宽度
    connect(ui->tableView_calc->horizontalHeader(), &QHeaderView::sectionResized, this, &CalcAssayDlg::OnColumnResized);
    
    // 保存完成
    connect(this, &CalcAssayDlg::saveCompleted, this, [&](int calcCode) {
        // 缓存项目名称
        m_lastAssayName = ui->assay_name_edit->text().toStdString();

        // 保存成功后刷新当前行的项目名称
        LoadCalcAssayToTableview();

        // 通知数据浏览界面刷新项目参数
        POST_MESSAGE(MSG_ID_USER_DEF_ASSAY_PARAM_UPDATE, calcCode);
        
        POST_MESSAGE(MSG_ID_SHOW_ERROR_TEXT, tr("保存成功"));
    });

    // 删除完成
    connect(this, &CalcAssayDlg::deleteCompleted, this, [&] {
        // 保存成功后刷新当前行的项目名称
        LoadCalcAssayToTableview();

        POST_MESSAGE(MSG_ID_SHOW_ERROR_TEXT, tr("删除成功"));
    });

    // 计算器按钮
    for (auto btn : ui->calc_widget->findChildren<QPushButton*>()) {
        if (btn->objectName().startsWith("page_"))
        {
            connect(btn, SIGNAL(clicked()), this, SLOT(OnCurrentPageChange()));
        }
        else
        {
            connect(btn, SIGNAL(clicked()), this, SLOT(OnCalculatorBtnClicked()));
        }
    }

    // 隐藏列
    ui->tableView_calc->hideColumn(CALC_COLUNM_ID);
    ui->tableView_calc->hideColumn(CALC_COLUNM_CODE);

    // 通道号输入限制
    ui->code_edit->setValidator(new QRegExpValidator(QRegExp(UI_REG_POSITIVE_INT)));

    // 边框
    for (auto fram : QWidget::findChildren<QFrame*>())
    {
        fram->setStyle(QApplication::style());
    }
    // 按钮
    for (auto btn : QWidget::findChildren<QPushButton*>())
    {
        btn->setStyle(QApplication::style());
    }

    // 设置生化免疫按钮是否可见
    SOFTWARE_TYPE type = m_assayMgr->GetSoftWareType();
    ui->radioButtonImmy->setChecked(type == IMMUNE);
    ui->groupBox_3->setVisible(type == CHEMISTRY_AND_IMMUNE);

    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, UpdateCtrlsEnabled);
}

void CalcAssayDlg::ClearCtrls()
{
    // 清理所有的输入框
    for (auto edit : QWidget::findChildren<QLineEdit*>())
    {
        edit->clear();
    }

    // 清理所有的下拉框
    for (auto cmbox : QWidget::findChildren<QComboBox*>())
    {
        cmbox->setCurrentIndex(-1);
    }

    // 清理所有的勾选框
    for (auto ckbox : QWidget::findChildren<QCheckBox*>())
    {
        ckbox->setChecked(false);
    }

    // 清理公式
    ui->formula_textedit->setText("");
    m_vecFormula.clear();

    // 清理参考区间
    m_refMode->removeRows(0, m_refMode->rowCount());

    // 清空提醒标签
    ui->attention_label->clear();

    ui->del_Button->setEnabled(false);
    ui->flat_save->setEnabled(false);
    ui->code_edit->setDisabled(false);
}

void CalcAssayDlg::DisableAllCtrls(bool bDisable)
{
    for (auto edit : QWidget::findChildren<QLineEdit*>())
    {
        edit->setDisabled(bDisable);
    }
    for (auto cmbox : QWidget::findChildren<QComboBox*>())
    {
        cmbox->setDisabled(bDisable);
    }
    for (auto ckbox : QWidget::findChildren<QCheckBox*>())
    {
        ckbox->setDisabled(bDisable);
    }

    // 清理公式
    ui->formula_textedit->setDisabled(bDisable);

    ui->del_Button->setDisabled(bDisable);
    ui->flat_save->setDisabled(bDisable);
    ui->code_edit->setDisabled(bDisable);

    // 参考范围按钮
    ui->add_ref_btn->setDisabled(bDisable);
    ui->modify_ref_btn->setDisabled(bDisable);
    ui->del_ref_btn->setDisabled(bDisable);
}

void CalcAssayDlg::LoadAssayDataChim(std::vector<std::pair<int, QString>>& assayData)
{
    // 获得通用项目信息
    auto& generalAssayMap = m_assayMgr->GetGeneralAssayCodeMap();

    // 查询生化已启用的普通项目
    auto& chAssayIndexCodeMaps = m_assayMgr->GetChAssayIndexCodeMaps();
    for (const auto& each : chAssayIndexCodeMaps)
    {
        // 获得血清指数项目信息
        if (ch::tf::g_ch_constants.ASSAY_CODE_SIND == each.second->assayCode)
        {
            for (int i = 1; i < 4; i++)
            {
                int code = ch::tf::g_ch_constants.ASSAY_CODE_SIND + i;
                assayData.push_back(std::pair<int, QString>(code, QString::fromStdString(generalAssayMap.at(code)->assayName)));
            }

            continue;
        }

        if (each.second->enable && generalAssayMap.count(each.first))
        {
            QString strAssayName = QString::fromStdString(generalAssayMap.at(each.first)->assayName);
            assayData.push_back(std::pair<int, QString>(each.first, strAssayName));
        }
    }

    // 获取已启用的ise项目
    if (m_assayMgr->HasIseDevice())
    {
        auto& iseAssayIndexCodeMaps = m_assayMgr->GetIseAssayIndexCodeMaps();
        for (const auto& each : iseAssayIndexCodeMaps)
        {
            if (each.second->enable && generalAssayMap.count(each.first))
            {
                assayData.push_back(std::pair<int, QString>(each.first, \
                    QString::fromStdString(generalAssayMap.at(each.first)->assayName)));
            }
        }
    }
}

void CalcAssayDlg::LoadAssayDataImmy(std::vector<std::pair<int, QString>>& assayData)
{
    const auto& generalAssayMap = m_assayMgr->GetGeneralAssayCodeMap();

    // 查询免疫已启用的普通项目
    const auto& imAssayIndexCodeMaps = m_assayMgr->GetImAssayIndexCodeMaps();
    for (const auto& genaral : generalAssayMap)
    {
        // 多版本的取最高版本的项目
        std::shared_ptr<im::tf::GeneralAssayInfo> curAssay = nullptr;
        std::string curVer;
        auto lowIter = imAssayIndexCodeMaps.lower_bound(genaral.first);
        auto uppIter = imAssayIndexCodeMaps.upper_bound(genaral.first);
        for (; lowIter != uppIter; lowIter++)
        {
            // 此处不能使用usingFlag过滤，因为usingFlag被初始化为0，用于项目设置初始显示版本标记。
            if (lowIter->second == nullptr)
            {
                continue;
            }
            if (lowIter->second->version > curVer)
            {
                curVer = lowIter->second->version;
                curAssay = lowIter->second;
            }
        }

        // 存在赋值
        if (curAssay != nullptr)
        {
            QString strAssayName = QString::fromStdString(genaral.second->assayName);
            assayData.push_back(std::pair<int, QString>(genaral.first, strAssayName));
        }
    }
}

void CalcAssayDlg::LoadAssayData()
{
    // 加载对应的项目名
    m_vecAssayInfo.clear();
    if (ui->radioButtonChim->isChecked())
    {
        LoadAssayDataChim(m_vecAssayInfo);
    }
    else
    {
        LoadAssayDataImmy(m_vecAssayInfo);
    }

    // 设置最大页码
    int maxPage = m_vecAssayInfo.size() / ASSAY_COUNT;
    maxPage += (m_vecAssayInfo.size() % ASSAY_COUNT) > 0 ? 1 : 0;
    ui->page_label->setText("1/" + QString::number(maxPage));

    // 设置翻页按钮状态
    ui->page_up_btn->setEnabled(false);
    ui->page_down_btn->setEnabled(m_vecAssayInfo.size() > ASSAY_COUNT);

    ShowAssayNameToCtrls();
}

void CalcAssayDlg::ShowAssayNameToCtrls()
{
    // 清理按钮文本
    for (auto btn : ui->calc_widget->findChildren<QPushButton*>())
    {
        if (btn->objectName().startsWith("assay_btn_"))
        {
            btn->setText("");
        }
    }

    // 显示项目名称
    int currentPage = ui->page_label->text().split("/")[0].toInt() - 1;
    int pageOffset = ASSAY_COUNT * currentPage;
    for (int i = 1; i < ASSAY_COUNT + 1; i++)
    {
        if (m_vecAssayInfo.size() < (i + pageOffset))
        {
            break;
        }

        int iPos = (i - 1) + pageOffset;
        if (iPos < m_vecAssayInfo.size())
        {
            QPushButton* pCurBtn = QWidget::findChild<QPushButton*>("assay_btn_" + QString::number(i));
            if (pCurBtn != Q_NULLPTR)
            {
                QString strText = m_vecAssayInfo[iPos].second;
                pCurBtn->setText(strText);

                QFontMetrics qMtr(pCurBtn->fontMetrics());
                if (qMtr.width(strText) +19 > pCurBtn->width())
                {
                    pCurBtn->setToolTip(strText);
                }
            }
        }
    }
}

void CalcAssayDlg::LoadCalcAssayToTableview()
{
    // 清理表格
    m_calcMode->removeRows(0, m_calcMode->rowCount());

    // 查询计算项目
    std::vector<std::shared_ptr<tf::CalcAssayInfo>> allCalAssayInfo;
    m_assayMgr->GetCalcAssayInfo(allCalAssayInfo);

    int iRows = 0;
    int selectRow = 0;
    for (const auto& item : allCalAssayInfo)
    {
        // 加载序号到表格
        AddTextToTableView(m_calcMode, iRows, CALC_COLUNM_NUM, QString::number(iRows + 1));
        // 加载项目名称到表格
        AddTextToTableView(m_calcMode, iRows, CALC_COLUNM_NAME, QString::fromStdString(item->name));
        // 加载id到表格                
        AddTextToTableView(m_calcMode, iRows, CALC_COLUNM_ID, QString::number(item->id));
        // 加载code到表格                
        AddTextToTableView(m_calcMode, iRows, CALC_COLUNM_CODE, QString::number(item->assayCode));

        // 获得上一次的选中行
        if (item->name == m_lastAssayName)
        {
            selectRow = iRows;
        }

        iRows++;
    }

    if (iRows == 0)
    {
        OnAddBtnClicked();
    }
    else
    {
        ui->tableView_calc->selectRow(0);
    }

    // 还原选中状态
    ui->tableView_calc->selectRow(selectRow);  
}

void CalcAssayDlg::ShowFormula()
{
    // 清理公式
    ui->formula_textedit->setText("");

    // 检查公式队列
    if (m_vecFormula.empty())
    {
        return;
    }

    // 查找项目
    if (m_assayMgr == nullptr)
    {
        ULOG(LOG_ERROR, "%s : CommonInformationManager::GetInstance() Failed", __FUNCTION__);
        return;
    }

    // 获得通用项目信息
    auto& generalAssayMap = m_assayMgr->GetGeneralAssayCodeMap();
    if (generalAssayMap.empty())
    {
        ULOG(LOG_ERROR, "Query Get General Assay Info Failed !");
        return;
    }

    // 显示公式
    QString tempdata;
    for (const auto& data : m_vecFormula)
    {
        if (data.startsWith(ASSAY_PREFIX))
        {
            int assayCode = data.split("y").at(1).toInt();
            AssayIndexCodeMaps::const_iterator gIter = generalAssayMap.find(assayCode);
            if (gIter == generalAssayMap.end())
            {
                ULOG(LOG_ERROR, "Error! Not exist assayCode:%d in general Assay table.", assayCode);
                return;
            }
            tempdata.append(QString::fromStdString(gIter->second->assayName));
        }
        else
        {
            tempdata.append(data);
        }
    }

    ui->formula_textedit->setText(tempdata);
}

void CalcAssayDlg::DecodeFormula(QString formula)
{
    // 清空计算器输入信息列表
    m_vecFormula.clear();

    // 检查输入参数
    if (formula.isEmpty())
    {
        return;
    }

    formula = formula.toLower();

    // 解析公式
    QString temp;
    for (int i = 0; i < formula.size(); i++)
    {
        if (formula[i] == '+' || formula[i] == '-' || formula[i] == '*' || formula[i] == '/'
            || formula[i] == ')' || formula[i] == '.' || formula[i] == '^')
        {
            if (!temp.isEmpty())
            {
                m_vecFormula.push_back(temp);
                temp.clear();
            }

            m_vecFormula.push_back(QString(formula[i]));
        }
        else if (formula[i] == '(')
        {
            m_vecFormula.push_back(temp.append('('));
            temp.clear();
        }
        else if (formula[i] >= '0' && formula[i] <= '9')
        {
            if (temp.startsWith(ASSAY_PREFIX))
            {
                temp += QString(formula[i]);
            }
            else
            {
                if (!temp.isEmpty())
                {
                    m_vecFormula.push_back(temp);
                    temp.clear();
                }
                m_vecFormula.push_back(QString(formula[i]));
            }
        }
        else
        {
            temp += QString(formula[i]);
        }
    }

    if (!temp.isEmpty())
    {
        m_vecFormula.push_back(temp);
    }
}

bool CalcAssayDlg::CheckUserInputData()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    ReportErr(ui->assay_name_edit->text().isEmpty(), tr("请填写项目名称"));
    ReportErr(ui->print_name_edit->text().isEmpty(), tr("请填写打印名称"));
    ReportErr(ui->code_edit->text().isEmpty(), tr("请填写项目通道号"));
    ReportErr(ui->display_cmbox->currentIndex() == -1, tr("请选择是否显示"));
    ReportErr(ui->unit_cmbox->currentIndex() == -1, tr("请选择单位"));
    ReportErr(ui->decimal_digit_cmbox->currentIndex() == -1, tr("请选择小数位数"));

    ReportErr(ui->sample_type_cmbox->currentText().isEmpty(), tr("请选择样本类型"));

    bool falg = false;
    int code = ui->code_edit->text().toInt(&falg);
    ReportErr(!falg || (::tf::AssayCodeRange::CALC_RANGE_MIN > code ||
        ::tf::AssayCodeRange::CALC_RANGE_MAX < code), tr("通道号错误，3001≤通道号≤4000"));

    // 检查当前通道号是否已经存在
    ReportErr(ui->code_edit->isEnabled() && m_assayMgr->GetCalcAssayInfo(code) != nullptr, tr("通道号错误:当前通道号已经存在"));

    std::string assayName = ui->assay_name_edit->text().toStdString();
    std::string printName = ui->print_name_edit->text().toStdString();

    // 检测测试项目同名
    auto assayInfoMap = m_assayMgr->GetGeneralAssayMap();
    for (const auto& iter : assayInfoMap)
    {
        if (iter.second->assayCode != code)
        {
            ReportErr((iter.second->assayName == assayName || iter.second->assayFullName == assayName), tr("项目名称不能与其他项目的名称重复！"));
            ReportErr((iter.second->printName == assayName), tr("项目名称不能与其他项目的打印名称重复！"));
            ReportErr((iter.second->assayName == printName || iter.second->assayFullName == printName), tr("打印名称不能与其他项目的名称重复！"));
            ReportErr((iter.second->printName == printName), tr("打印名称不能与其他项目的打印名称重复！"));
        }
    }
    // 检测计算项目同名
    auto calcInfoMap = m_assayMgr->GetCalcAssayInfoMap();
    for (const auto& iter : calcInfoMap)
    {
        if (iter.second->assayCode != code)
        {
            ReportErr((iter.second->name == assayName), tr("项目名称不能与其他计算项目的名称重复！"));
            ReportErr((iter.second->printName == assayName), tr("项目名称不能与其他计算项目的打印名称重复！"));
            ReportErr((iter.second->name == printName), tr("打印名称不能与其他计算项目的名称重复！"));
            ReportErr((iter.second->printName == printName), tr("打印名称不能与其他计算项目的打印名称重复！"));
        }
    }

    return true;
}

void CalcAssayDlg::GetReferrenceByRow(int row, ::tf::AssayReferenceItem& referrence)
{
    referrence.bGenderRelevent = true;
    referrence.bAgeRelevant = true;

    // 是否缺省
    referrence.bAutoDefault = m_refMode->data(m_refMode->index(row, REF_COLUNM_AUTO)
    ).toString() == STR_AUTO_DEFAULT_Y;
    // 样本类型
    referrence.sampleSourceType = m_refMode->data(m_refMode->index(row, REF_COLUNM_SAMPLE), SAMPLE_ROLE).toInt();

    // 参考区间
    QString strLower = m_refMode->data(m_refMode->index(row, REF_COLUNM_LOW)).toString();
    referrence.__set_fLowerRefRang(strLower.isEmpty() ? std::numeric_limits<double>::lowest() : strLower.toDouble());
    QString strUpper = m_refMode->data(m_refMode->index(row, REF_COLUNM_UP)).toString();
    referrence.__set_fUpperRefRang(strUpper.isEmpty() ? DBL_MAX : strUpper.toDouble());

    // 危急低值
    QString tempStr = m_refMode->data(m_refMode->index(row, REF_COLUNM_LOW_CRISIS)).toString();
    referrence.__set_fLowerCrisisRang(tempStr.isEmpty() ? std::numeric_limits<double>::lowest() : tempStr.toDouble());

    // 危急高值
    tempStr = m_refMode->data(m_refMode->index(row, REF_COLUNM_UP_CRISIS)).toString();
    referrence.__set_fUpperCrisisRang(tempStr.isEmpty() ? DBL_MAX : tempStr.toDouble());

    // 性别
    auto tempText = m_refMode->data(m_refMode->index(row, REF_COLUNM_GENDER)).toString();
    int iGender = ThriftEnumTrans::TransGender(tempText);
    if (iGender != -1)
    {
        referrence.__set_enGender((tf::Gender::type)iGender);
    }

    // 年龄下限
    tempStr = m_refMode->data(m_refMode->index(row, REF_COLUNM_AGE_LOW)).toString();
    referrence.__set_iLowerAge(tempStr.isEmpty() ? INT_MIN : tempStr.toDouble());

    // 年龄上限
    tempStr = m_refMode->data(m_refMode->index(row, REF_COLUNM_AGE_UP)).toString();
    referrence.__set_iUpperAge(tempStr.isEmpty() ? INT_MAX : tempStr.toDouble());

    // 年龄单位
    tempStr = m_refMode->data(m_refMode->index(row, REF_COLUNM_UNIT)).toString();
    int iAgeUnit = ThriftEnumTrans::TransAgeUnit(tempStr);
    if (iAgeUnit != -1)
    {
        referrence.__set_enAgeUnit((tf::AgeUnit::type)iAgeUnit);
    }
}

void CalcAssayDlg::ShowReferrence(int row, const ::tf::AssayReferenceItem& item)
{
    // 序号
    QStandardItem* sque = new QStandardItem(QString::number(row + 1));
    sque->setData(QVariant::fromValue<::tf::AssayReferenceItem>(item), Qt::UserRole);
    sque->setData(Qt::AlignCenter, Qt::TextAlignmentRole);
    m_refMode->setItem(row, REF_COLUNM_NUM, sque);

    // 是否缺省
    AddTextToTableView(m_refMode, row, REF_COLUNM_AUTO, (item.bAutoDefault ? STR_AUTO_DEFAULT_Y : STR_AUTO_DEFAULT_N))

    // 样本类型
    QString strSampleType = (item.sampleSourceType < 0) ? tr("全部") : ConvertTfEnumToQString((::tf::SampleSourceType::type)item.sampleSourceType);
    QStandardItem* si = new QStandardItem(strSampleType);
    si->setData(item.sampleSourceType, SAMPLE_ROLE);
    si->setData(Qt::AlignCenter, Qt::TextAlignmentRole);
    m_refMode->setItem(row, REF_COLUNM_SAMPLE, si);

    // 低值
    QString strLowerRef = item.fLowerRefRang == std::numeric_limits<double>::lowest()  ? "" : UiCommon::DoubleToQString(item.fLowerRefRang);
    AddTextToTableView(m_refMode, row, REF_COLUNM_LOW, strLowerRef);

    // 高值
    QString strUpperRef = item.fUpperRefRang == DBL_MAX ? "" : UiCommon::DoubleToQString(item.fUpperRefRang);
    AddTextToTableView(m_refMode, row, REF_COLUNM_UP, strUpperRef);

    // 参考范围
    QString strLowerUpper = (strLowerRef.isEmpty() && strUpperRef.isEmpty()) ? "" : (strLowerRef + "-" + strUpperRef);
    AddTextToTableView(m_refMode, row, REF_COLUNM_REF, strLowerUpper);

    // 危急低值
    AddTextToTableView(m_refMode, row, REF_COLUNM_LOW_CRISIS, item.fLowerCrisisRang == std::numeric_limits<double>::lowest()  
                        ? "" : UiCommon::DoubleToQString(item.fLowerCrisisRang));
    // 危急高值
    AddTextToTableView(m_refMode, row, REF_COLUNM_UP_CRISIS, item.fUpperCrisisRang == DBL_MAX 
                            ? "" : UiCommon::DoubleToQString(item.fUpperCrisisRang));
    // 性别
    AddTextToTableView(m_refMode, row, REF_COLUNM_GENDER, ConvertTfEnumToQString(item.enGender));

    // 年龄下限
    AddTextToTableView(m_refMode, row, REF_COLUNM_AGE_LOW,item.iLowerAge == INT_MIN ? "" : QString::number(item.iLowerAge));

    // 年龄上限
    AddTextToTableView(m_refMode, row, REF_COLUNM_AGE_UP,(item.iUpperAge == INT_MAX ? "" : QString::number(item.iUpperAge)));

    // 单位
    AddTextToTableView(m_refMode, row, REF_COLUNM_UNIT,(item.iLowerAge == INT_MIN ? "" : ConvertTfEnumToQString(item.enAgeUnit)));
}

bool CalcAssayDlg::TakeCalcAssayInfo(::tf::CalcAssayInfo& cai)
{
    // 项目名称
    cai.__set_name(ui->assay_name_edit->text().toUtf8().toStdString());
    // 是否显示
    cai.__set_display(ui->display_cmbox->currentIndex() == 0);
    // 打印名称
    cai.__set_printName(ui->print_name_edit->text().toUtf8().toStdString());
    // 单位
    cai.__set_resultUnit(ui->unit_cmbox->currentText().toStdString());
    // 通道号
    cai.__set_assayCode(ui->code_edit->text().toInt());
    // 小数位数
    cai.__set_decimalDigit(ui->decimal_digit_cmbox->currentIndex());
    // 样本源类型
    int iSampleType = ui->sample_type_cmbox->currentText().isEmpty() ? -1 : ui->sample_type_cmbox->currentData().toInt();
    cai.__set_sampleSourceType(iSampleType);

    // 公式
    QString formula;
    std::vector<int> codes;
    for (const QString& data : m_vecFormula)
    {
        if (data.startsWith(ASSAY_PREFIX))
        {
            codes.push_back(data.split("y").at(1).toInt());
        }
        formula.append(data);
    }

    if (codes.empty())
    {
        POST_MESSAGE(MSG_ID_SHOW_ERROR_TEXT, tr("保存失败，计算项目的公式中必须包含常规项目"));
        return false;
    }

    formula.replace("lg(", "Lg(");
    formula.replace("ln(", "Ln(");
    formula.replace("sqrt(", "Sqrt(");

    // 括号必须成对,结尾不能是计算符号
    if ((formula.count("(") != formula.count(")")) || formula.endsWith("+")
        || formula.endsWith("-") || formula.endsWith("*") || formula.endsWith("/")
        || formula.endsWith(".") || formula.endsWith("^") || ui->formula_textedit->toPlainText().isEmpty())
    {
        POST_MESSAGE(MSG_ID_SHOW_ERROR_TEXT, tr("保存失败，当前项目的公式不合法"));
        return false;
    }
    cai.__set_formula(formula.toStdString());

    // 公式中包含的通道号
    cai.__set_vecAssayCode(std::move(codes));

    // 参考区间
    ::tf::AssayReference referrence;
    referrence.bUsing = true;
    for (int i = 0; i < m_refMode->rowCount(); i++)
    {
        ::tf::AssayReferenceItem refItem;
        GetReferrenceByRow(i, refItem);
        referrence.Items.push_back(refItem);
    }
    cai.__set_vecRefRange(referrence);

    if (m_assayMgr->GetCalcAssayInfo(cai.assayCode) != nullptr)
    {
        // 设置id
        cai.__set_id(m_calcMode->data(m_calcMode->index(ui->tableView_calc->currentIndex().row(), CALC_COLUNM_ID)).toInt());
    }
    else
    {
        // 公式创建时间	
        cai.__set_oleCreateDate(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss").toStdString());
    }

    return true;
}

void CalcAssayDlg::ProcSaveRef(bool bRefChange)
{
    // 显示等待弹窗
    POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_OPEN, tr("正在保存项目..."), true);

    ::tf::CalcAssayInfo cai;
    if (!TakeCalcAssayInfo(cai))
    {
        ULOG(LOG_WARN, "Failed to get calassayinfo from ui.");
        return;
    }
	// 仅参考范围发生变化，计算项目未变化，仅将参考范围变动添加到操作日志
	// 参考范围和计算项目均发生变化，都需添加到操作日志
	bool bSaveOptLog = true;
	if (bRefChange && !IsChangedOfCalcAssay(cai))
	{
		bSaveOptLog = false;
	}

    // 操作日志
    QString optStr(tr("计算项目："));
    optStr += QString::fromStdString(cai.name);
    optStr += tr("、是否显示：") + (cai.display ? tr("是") : tr("否"));
    optStr += tr("、样本类型：") + (cai.sampleSourceType == -1 ? tr("全部") : ConvertTfEnumToQString((::tf::SampleSourceType::type)cai.sampleSourceType));
    optStr += tr("、打印名称：") + QString::fromStdString(cai.printName);
    optStr += tr("、结果单位：") + QString::fromStdString(cai.resultUnit);
    optStr += tr("、通道号：") + QString::number(cai.assayCode);
    optStr += tr("、小数位数：") + QString::number(cai.decimalDigit);
    optStr += tr("、公式：") + ui->formula_textedit->toPlainText();

    std::shared_ptr<CommonInformationManager> assayMgr = CommonInformationManager::GetInstance();
    if (assayMgr->GetCalcAssayInfo(cai.assayCode) != nullptr)
    {
        if (!assayMgr->ModifyCalcAssayInfo(cai))
        {
            ULOG(LOG_ERROR, "ModifyCalcAssayInfo Failed, assayCode:%d.", cai.assayCode);
            POST_MESSAGE(MSG_ID_SHOW_ERROR_TEXT, tr("修改失败"));
            return;
        }

		if (bSaveOptLog)
		{
			if (!AddOptLog(::tf::OperationType::MOD, tr("修改") + optStr))
			{
				ULOG(LOG_ERROR, "Add add calc assay operate log failed !");
			}
		}

    }
    else
    {
        if (!assayMgr->AddCalcAssayInfo(cai))
        {
            ULOG(LOG_ERROR, "AddCalcAssayInfo Failed, assayCode:%d.", cai.assayCode);
            POST_MESSAGE(MSG_ID_SHOW_ERROR_TEXT, tr("保存失败"));
            return;
        }

        if (!AddOptLog(::tf::OperationType::ADD, tr("新增") + optStr))
        {
            ULOG(LOG_ERROR, "Add modify calc assay operate log failed !");
        }
    }

    emit saveCompleted(cai.assayCode);
}

void CalcAssayDlg::deleteCalcAssay(const std::map<int, int>& calcIdAndCode)
{
    // 显示等待弹窗
    POST_MESSAGE(MSG_ID_PROGRESS_DIALOG_OPEN, tr("正在删除项目..."), true);

    const auto& CIM = CommonInformationManager::GetInstance();
    for (auto& item : calcIdAndCode)
    {
        // 操作日志
        auto calcInfo = CIM->GetCalcAssayInfo(item.second);
        QString optStr(tr("删除计算项目："));
        if (calcInfo)
        {
            optStr += QString::fromStdString(calcInfo->name);
            optStr += tr("、是否显示：") + (calcInfo->display ? tr("是") : tr("否"));
            optStr += tr("、样本类型：") + (calcInfo->sampleSourceType == -1 ? tr("全部") : ConvertTfEnumToQString((::tf::SampleSourceType::type)calcInfo->sampleSourceType));
            optStr += tr("、打印名称：") + QString::fromStdString(calcInfo->printName);
            optStr += tr("、结果单位：") + QString::fromStdString(calcInfo->resultUnit);
            optStr += tr("、通道号：") + QString::number(calcInfo->assayCode);
            optStr += tr("、小数位数：") + QString::number(calcInfo->decimalDigit);
            optStr += tr("、公式：") + QString::fromStdString(calcInfo->formula);
        }

        if (!CIM->DeleteCalcAssayInfoById(item.first, item.second))
        {
            POST_MESSAGE(MSG_ID_SHOW_ERROR_TEXT, tr("删除失败"));
            return;
        }

        if (!AddOptLog(::tf::OperationType::DEL, optStr))
        {
            ULOG(LOG_ERROR, "Add delete calc assay operate log failed !");
        }

        // 通知数据浏览界面刷新项目参数
        POST_MESSAGE(MSG_ID_USER_DEF_ASSAY_PARAM_UPDATE, item.second);
    }

    emit deleteCompleted();
}

void CalcAssayDlg::OnColumnResized(int column, int oldWidth, int newWidth)
{
    if (column != CALC_COLUNM_NAME)
    {
        return;
    }
    // 显示不全时使用tooltip
    int iRowCnt = m_calcMode->rowCount();
    for (int iRows =0; iRows<iRowCnt; ++iRows)
    {
        QStandardItem* pNameItem = m_calcMode->item(iRows, CALC_COLUNM_NAME);
        if (pNameItem != Q_NULLPTR)
        {
            QFontMetrics pMtr(ui->tableView_calc->fontMetrics());
            bool needTip = pMtr.width(pNameItem->text()) + 10 > newWidth;
            pNameItem->setToolTip(needTip ? pNameItem->text() : "");
        }
    }
}

void CalcAssayDlg::UpdateCtrlsEnabled()
{
    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    QItemSelectionModel* pSelectionModel = ui->tableView_ref->selectionModel();
    if (pSelectionModel == Q_NULLPTR)
    {
        ULOG(LOG_WARN, "Null selection model.");
        return;
    }
    DisableAllCtrls(false);

    QModelIndexList qSelectedList = ui->tableView_calc->selectionModel()->selectedRows();
    if (qSelectedList.size() > 0)
    {
        // 选中空行; 新增项目时项目编号的编辑框才可编辑 
        QString strAssayCode = ui->code_edit->text();
        if (strAssayCode.isEmpty() || (m_assayMgr->GetCalcAssayInfo(strAssayCode.toInt()) == nullptr))
        {
            // 计算项目按钮
            ui->flat_new_add->setEnabled(true);
            ui->del_Button->setEnabled(false);
            ui->flat_save->setEnabled(true);
        }
        else
        {
            ui->code_edit->setDisabled(true);
            const auto& uim = UserInfoManager::GetInstance();

            // 参考范围按钮
            ui->add_ref_btn->setEnabled(uim->IsPermisson(PSM_IM_CALCULATEASSAY_SETRANGE));
            ui->modify_ref_btn->setEnabled(uim->IsPermisson(PSM_IM_CALCULATEASSAY_SETRANGE));
            ui->del_ref_btn->setEnabled(uim->IsPermisson(PSM_IM_CALCULATEASSAY_SETRANGE));

            // 计算项目按钮
            //末尾还为空行时表示正在新增行，需置灰新增按钮
            QStandardItem* lastRowCell = m_calcMode->item(m_calcMode->rowCount() - 1, CALC_COLUNM_NAME);
            ui->flat_new_add->setEnabled(lastRowCell != Q_NULLPTR && uim->IsPermisson(PSM_IM_CALCULATEASSAY_ADD));

            ui->del_Button->setEnabled(uim->IsPermisson(PSM_IM_CALCULATEASSAY_DEL));
            ui->flat_save->setEnabled(uim->IsPermisson(PSM_IM_CALCULATEASSAY_MODIFY));
        }
    } 
    else
    {
        DisableAllCtrls(true);
    }
}

void CalcAssayDlg::OnSaveBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    if (m_assayMgr->IsExistDeviceRuning())
    {
        TipDlg(tr("仪器运行中禁止修改计算项目参数")).exec();
        return;
    }

    if (!CheckUserInputData())
    {
        ULOG(LOG_INFO, "CheckUserInputData Invalid !");
        return;
    }

    // 启动线程,避免界面卡顿
    std::thread saveThread(std::bind(&CalcAssayDlg::ProcSaveRef, this, false));
    saveThread.detach();
}

void CalcAssayDlg::OnCalculatorBtnClicked()
{
    // 清空提醒标签
    ui->attention_label->clear();

    QPushButton *btn = qobject_cast<QPushButton*>(sender());
    QRegExp rx("[0-9]");
    if (btn->objectName() == "c_btn")	// C按钮,情况全部
    {
        m_vecFormula.clear();
    }
    else if (btn->objectName() == "ce_btn" && !m_vecFormula.empty())	// CE按钮,清空上次输入
    {
        m_vecFormula.pop_back();
    }
    else if (btn->objectName() == "right_btn" && !m_vecFormula.empty())	// ")"按钮
    {
        // ")"前必须是数字或者")"
        QString formual;
        for (auto item : m_vecFormula)
        {
            formual += item;
        }

        if (!rx.exactMatch(m_vecFormula[m_vecFormula.size() - 1])
            && (formual.count("(") == formual.count(")")))
        {
            ui->attention_label->setText(tr("当前公式后不能输入 ") + btn->text());
            return;
        }

        // 括号中必须包含运算符和项目
        bool flag = false;
        for (int i = m_vecFormula.size() - 1; i >= 0; i--)
        {
            ui->attention_label->setText(tr("当前公式后不能输入 ") + btn->text());

            if (m_vecFormula[i] == "(")
            {
                break;
            }
            else
            {
                if (m_vecFormula[i] == "+" || m_vecFormula[i] == "-" || m_vecFormula[i] == "*"
                    || m_vecFormula[i] == "/" || m_vecFormula[i] == "^" || m_vecFormula[i].endsWith("("))
                {
                    ui->attention_label->clear();
                    m_vecFormula.push_back(btn->text());
                    break;
                }
            }
        }
    }
    else if (btn->objectName().startsWith("math_fun") || btn->objectName() == "left_btn")		// 数学符号按钮
    {
        if (!m_vecFormula.empty())
        {
            QString data = m_vecFormula[m_vecFormula.size() - 1];
            if (rx.exactMatch(data) || data == "." || data == ")" || data.startsWith(ASSAY_PREFIX))
            {
                ui->attention_label->setText(tr("当前公式后不能输入 ") + btn->text());
                return;
            }
        }

        (btn->objectName() == "left_btn") ? m_vecFormula.push_back(btn->text())
            : m_vecFormula.push_back(btn->text() + "(");
    }
    else if (btn->objectName().startsWith("symbols_"))		// 四则运算符号按钮
    {
        if (!m_vecFormula.empty())
        {
            QString data = m_vecFormula[m_vecFormula.size() - 1];

            // 除减号外前面不能是"("
            if (data.endsWith("(") && btn->objectName() != "symbols_minus_btn")
            {
                ui->attention_label->setText(tr("当前公式后不能输入 ") + btn->text());
                return;
            }

            // "."前面只能是数字
            if (btn->objectName() == "symbols_spot_btn")
            {
                if (!rx.exactMatch(data))
                {
                    ui->attention_label->setText(tr("当前公式后不能输入 ") + btn->text());
                    return;
                }
            }

            // 计算符号前面不能是计算符号
            for (auto syBtn : ui->calc_widget->findChildren<QPushButton*>())
            {
                if ((syBtn->objectName().startsWith("symbols_")) && (syBtn->text() == data))
                {
                    ui->attention_label->setText(tr("当前公式后不能输入 ") + btn->text());
                    return;
                }
            }

        }
        else
        {	// 公式开始允许是减号
            if (btn->objectName() != "symbols_minus_btn")
            {
                ui->attention_label->setText(tr("当前公式后不能输入 ") + btn->text());
                return;
            }
        }

        m_vecFormula.push_back(btn->text());

    }
    else if (btn->objectName().startsWith("num_"))		// 数字按钮
    {
        // 数字前面不能是")"、项目
        if (!m_vecFormula.empty())
        {
            QString data = m_vecFormula[m_vecFormula.size() - 1];
            if (data.endsWith(")") || data.startsWith(ASSAY_PREFIX))
            {
                ui->attention_label->setText(tr("当前公式后不能输入 ") + btn->text());
                return;
            }
        }

        m_vecFormula.push_back(btn->text());
    }
    else if (btn->objectName().startsWith("assay_btn_"))		// 项目选择按钮
    {
        if (btn->text().isEmpty())
        {
            return;
        }

        // 项目前面不能是")"和".",也不能是项目和数字
        if (!m_vecFormula.empty())
        {
            QString data = m_vecFormula[m_vecFormula.size() - 1];
            if (data == ")" || data == "." || data.startsWith(ASSAY_PREFIX) || rx.exactMatch(data))
            {
                ui->attention_label->setText(tr("当前公式后不能输入 ") + btn->text());
                return;
            }
        }

        // 加载选择的项目编号到公式队列
        auto iter = find_if(m_vecAssayInfo.begin(), m_vecAssayInfo.end(),
            [&](const std::pair<int, QString>& data)
        {
            return btn->text() == data.second;
        });
        m_vecFormula.push_back(ASSAY_PREFIX + QString::number(iter->first));
    }

    // 重新显示公式
    ShowFormula();
}

///
/// @brief 新增按钮槽函数
///
///
/// @return 
///
/// @par History:
/// @li 1556/Chenjianlin，2023年8月15日，新建函数
///
void CalcAssayDlg::OnAddBtnClicked()
{
    // 清理控件中的数据
    ClearCtrls();
    // 设置通道号可编辑
    ui->code_edit->setDisabled(false);

    // 创建新行
    QStandardItem * pItem = new QStandardItem(QString::number(m_calcMode->rowCount() + 1));
    if (nullptr != pItem)
    {
        // 设置居中 	0021404: [应用] 计算项目界面新增计算项目后点击“新增”按钮，界面项目列表中序号未对齐
        pItem->setData(Qt::AlignCenter, Qt::TextAlignmentRole);
        m_calcMode->setItem(m_calcMode->rowCount(), CALC_COLUNM_NUM, pItem);
    }
    // 选中新增行
    ui->tableView_calc->selectRow(m_calcMode->rowCount() - 1);
}

void CalcAssayDlg::OnDeleteBtnClicked()
{
    ULOG(LOG_INFO, __FUNCTION__);

    if (m_assayMgr->IsExistDeviceRuning())
    {
        TipDlg(tr("仪器在运行或暂停状态时不能执行计算项目的删除操作！")).exec();
        return;
    }

    if(TipDlg(tr("删除"), tr("即将删除选中的计算项目"), TipDlgType::TWO_BUTTON).exec() == QDialog::Rejected)
    {
        return;
    }

    std::map<int, int> calcIdAndCode;
    for (const QModelIndex& rowIndex : ui->tableView_calc->selectionModel()->selectedRows())
    {
        // 判断当前选中的是否是空行
        int currentRowIndex = rowIndex.row();
        if (!m_calcMode->index(currentRowIndex, CALC_COLUNM_NAME).data().isNull())
        {
            int calcId = m_calcMode->index(currentRowIndex, CALC_COLUNM_ID).data().toInt();
            calcIdAndCode[calcId] = m_calcMode->index(currentRowIndex, CALC_COLUNM_CODE).data().toInt();           
        }
    }

    // 启动线程,避免界面卡顿
    std::thread deleteThread(std::bind(&CalcAssayDlg::deleteCalcAssay, this, calcIdAndCode));
    deleteThread.detach();
}

void CalcAssayDlg::OnAddRefBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    if (m_assayMgr->IsExistDeviceRuning())
    {
        TipDlg(tr("仪器运行中禁止修改计算项目参数")).exec();
        return;
    }

    m_rangParaSetWidget->ShowDlg(Et_Add);
    m_rangParaSetWidget->SetSampleTypeCommbox(m_currentSampleSource);
}

void CalcAssayDlg::OnModifyRefBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    if (m_assayMgr->IsExistDeviceRuning())
    {
        TipDlg(tr("仪器运行中禁止修改计算项目参数")).exec();
        return;
    }

    m_rangParaSetWidget->ShowDlg(Et_Modify);
    m_rangParaSetWidget->SetSampleTypeCommbox(m_currentSampleSource);

    ::tf::AssayReferenceItem refItem;
    GetReferrenceByRow(ui->tableView_ref->currentIndex().row(), refItem);
    m_rangParaSetWidget->loadRangParaSetWidget(refItem);
}

void CalcAssayDlg::OnDeleteRefBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    if (m_assayMgr->IsExistDeviceRuning())
    {
        TipDlg(tr("仪器运行中禁止修改计算项目参数")).exec();
        return;
    }

    if (TipDlg(tr("删除"), tr("即将删除选中的参考区间"), TipDlgType::TWO_BUTTON).exec() == QDialog::Rejected)
    {
        return;
    }

    // 刷新行号并移除当前行
    auto rowIndexs = ui->tableView_ref->selectionModel()->selectedRows();
    QStringList rowList;
    for (const QModelIndex & index : rowIndexs)
    {
        QModelIndex seqIdx = m_refMode->index(index.row(), 0);
        if (seqIdx.isValid())
        {
            rowList.push_back(seqIdx.data().toString());
        }
    }

    // 按序号删除选中行
    std::set<int> deletedRows;
    for (const QString& row : rowList)
    {
        for (int i = 0; i < m_refMode->rowCount(); i++)
        {
            auto rowSeqItem = m_refMode->item(i, REF_COLUNM_NUM);
            if (rowSeqItem == nullptr)
            {
                continue;
            }

            if (rowSeqItem->text() == row)
            {
                deletedRows.insert(i);
                m_refMode->removeRow(i);
                ui->tableView_ref->selectRow(i - 1);
                continue;
            }
        }
    }

    // 重置行序号
    int optCurrentRow = 0;
    while (optCurrentRow < m_refMode->rowCount())
    {
        if (!m_refMode->item(optCurrentRow, 1)->text().isEmpty())
        {
            QStandardItem* pItem = m_refMode->item(optCurrentRow, 0);
            if (pItem != Q_NULLPTR)
            {
                pItem->setText(QString::number(optCurrentRow + 1)); // 修改序号
            }
        }
        optCurrentRow++;
    }

    // 启动线程,避免界面卡顿
    std::thread saveThread(std::bind(&CalcAssayDlg::ProcSaveRef, this, true));
    saveThread.detach();

	// 添加操作日志 add by chenjianlin 20231214
	int rowIndex = ui->tableView_calc->currentIndex().row();
	if (!m_calcMode->data(m_calcMode->index(rowIndex, CALC_COLUNM_NAME)).isNull())
	{
		QString strCalcName = m_calcMode->data(m_calcMode->index(rowIndex, CALC_COLUNM_NAME)).toString();
		COperationLogManager::GetInstance()->AddRangeParamOperationLog(tf::OperationType::type::DEL, strCalcName.toStdString());
	}
}

void CalcAssayDlg::OnSelectRowChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    // 清理控件中的数据
    ClearCtrls();

    QItemSelectionModel* pSelectionModel = ui->tableView_calc->selectionModel();
    if (pSelectionModel == Q_NULLPTR)
    {
        ULOG(LOG_WARN, "Null qitemselectionmodel.");
        return;
    }
    auto rowList = pSelectionModel->selectedRows();
    if (rowList.empty())
    {
        UpdateCtrlsEnabled();
        return;
    }

    // 根据ID获取CalcAssayInfo
    int caclItemId = m_calcMode->data(m_calcMode->index(rowList[rowList.size()-1].row(), CALC_COLUNM_ID)).toInt();
    std::shared_ptr<tf::CalcAssayInfo> curCalcItem = m_assayMgr->GetCalcAssayInfoById(caclItemId);
    if (curCalcItem == nullptr)
    {
        ULOG(LOG_INFO, "Null result caclassayinfo get by id:%d.", caclItemId);
        UpdateCtrlsEnabled();
        return;
    }

    // 设置项目类型Radio按钮状态
    tf::CalcAssayInfo currCalcAssay = *curCalcItem;

    // 项目名称
    ui->assay_name_edit->setText(QString::fromStdString(currCalcAssay.name));
    // 是否显示
    ui->display_cmbox->setCurrentIndex(currCalcAssay.display ? 0 : 1);
    // 打印名称
    ui->print_name_edit->setText(QString::fromStdString(currCalcAssay.printName));
    // 单位
    ui->unit_cmbox->setCurrentText(QString::fromStdString(currCalcAssay.resultUnit));
    // 通道号
    ui->code_edit->setText(QString::number(currCalcAssay.assayCode));
    ui->code_edit->setDisabled(true);
    // 小数位数
    ui->decimal_digit_cmbox->setCurrentIndex(currCalcAssay.decimalDigit);
    // 样本源类型
    ui->sample_type_cmbox->setCurrentIndex(ui->sample_type_cmbox->findData(currCalcAssay.sampleSourceType));
    m_currentSampleSource = currCalcAssay.sampleSourceType;

    // 公式
    DecodeFormula(QString::fromStdString(currCalcAssay.formula));
    ShowFormula();

    // 参考区间
    int iRow = 0;
    for (const auto& item : currCalcAssay.vecRefRange.Items)
    {
        ShowReferrence(iRow, item);
        iRow++;
    }

    // 设置默认选中行
    if (iRow > 0)
    {
        ui->tableView_ref->selectRow(0);
    }

    UpdateCtrlsEnabled();
}

void CalcAssayDlg::OnCurrentPageChange()
{
    // 获取信号发送按钮
    auto btn = dynamic_cast<QPushButton*>(sender());

    // 获取当前页
    auto pagetext = ui->page_label->text().split("/");
    int page = pagetext[0].toInt();

    // 修改页码
    page += btn->objectName().contains("up") ? -1 : 1;
    pagetext[0] = QString::number(page);

    // 设置翻页按钮的状态
    ui->page_up_btn->setEnabled(pagetext[0] != "1");
    ui->page_down_btn->setEnabled(pagetext[0] != pagetext[1]);

    ui->page_label->setText(pagetext[0] + "/" + pagetext[1]);

    // 刷新项目选择按钮文本
    ShowAssayNameToCtrls();
}

void CalcAssayDlg::OnUpdateRefRange()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    int currentIdx = ui->tableView_ref->currentIndex().row();
    int row = m_rangParaSetWidget->GetEditType() == Et_Add ? m_refMode->rowCount() : currentIdx;

    ::tf::AssayReferenceItem item;
    if (!m_rangParaSetWidget->getSetParaData(item))
    {
        ULOG(LOG_INFO, "getSetParaData Failed !");
        return;
    }

    // 检查参考区间
    ::tf::AssayReference referrence;
    for (int row = 0; row < m_refMode->rowCount(); row++)
    {
        ::tf::AssayReferenceItem refItem;
        GetReferrenceByRow(row, refItem);
        referrence.Items.push_back(std::move(refItem));
    }

    // 编辑时，先移除编辑前的行数据
    if (m_rangParaSetWidget->GetEditType() == Et_Modify)
    {
        referrence.Items.erase(referrence.Items.begin() + ui->tableView_ref->currentIndex().row());
    }

    referrence.Items.push_back(item);
    if (!UiCommon::CheckAssayReferenceIsValid(referrence))
    {
        return;
    }

    ShowReferrence(row, item);
    ui->tableView_ref->selectRow(row);

    // 启动线程,避免界面卡顿
    std::thread saveThread(std::bind(&CalcAssayDlg::ProcSaveRef, this, true));
    saveThread.detach();

    m_rangParaSetWidget->close();

	// 添加操作日志 add by chenjianlin 20231214
	int rowIndex = ui->tableView_calc->currentIndex().row();
	if (!m_calcMode->data(m_calcMode->index(rowIndex, CALC_COLUNM_NAME)).isNull())
	{
		QString strCalcName = m_calcMode->data(m_calcMode->index(rowIndex, CALC_COLUNM_NAME)).toString();
		COperationLogManager::GetInstance()->AddRangeParamOperationLog(m_rangParaSetWidget->GetEditType() == Et_Modify ? tf::OperationType::type::MOD : tf::OperationType::type::ADD, strCalcName.toStdString());
	}
}

void CalcAssayDlg::OnTabelSelectChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    UpdateCtrlsEnabled();
}

bool CalcAssayDlg::IsChangedOfCalcAssay(const ::tf::CalcAssayInfo& calci)
{
	QItemSelectionModel* pSelectionModel = ui->tableView_calc->selectionModel();
	if (pSelectionModel == Q_NULLPTR)
	{
		ULOG(LOG_WARN, "Null qitemselectionmodel.");
		return true;
	}

	auto rowList = pSelectionModel->selectedRows();
	if (rowList.empty())
	{
		return true;
	}

	// 根据ID获取CalcAssayInfo
	int caclItemId = m_calcMode->data(m_calcMode->index(rowList[rowList.size() - 1].row(), CALC_COLUNM_ID)).toInt();
	std::shared_ptr<tf::CalcAssayInfo> curCalcItem = m_assayMgr->GetCalcAssayInfoById(caclItemId);
	if (curCalcItem == nullptr)
	{
		return true;
	}

	if (calci.assayCode != curCalcItem->assayCode ||
		calci.decimalDigit != curCalcItem->decimalDigit ||
		calci.display != curCalcItem->display ||
		calci.formula != curCalcItem->formula ||
		calci.name != curCalcItem->name ||
		calci.printName != curCalcItem->printName ||
		calci.resultUnit != curCalcItem->resultUnit || 
		calci.sampleSourceType != curCalcItem->sampleSourceType)
	{
		return true;
	}

	return false;
}
