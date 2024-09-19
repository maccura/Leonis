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
/// @file     DataDictionaryDlg.cpp
/// @brief    应用--系统--数据字典
///
/// @author   7951/LuoXin
/// @date     2022年11月22日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2022年11月22日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "DataDictionaryDlg.h"
#include "ui_DataDictionaryDlg.h"
#include <QStandardItemModel>
#include <QLineEdit>
#include "thrift/DcsControlProxy.h"
#include "manager/DictionaryQueryManager.h"
#include "manager/UserInfoManager.h"
#include "src/common/common.h"
#include "src/common/Mlog/mlog.h"
#include "src/thrift/gen-cpp/defs_types.h"
#include "src/public/ConfigSerialize.h"
#include "src/public/SerializeUtil.hpp"
#include "shared/uicommon.h"
#include "shared/uidef.h"
#include "shared/tipdlg.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/CReadOnlyDelegate.h"
#include "shared/CommonInformationManager.h"

#define MAX_CONTENT_LENGGHT 1024 // 内最大长度（TODO：confirm）

// 名称表格各信息所在列
enum NameTableViewColunm
{
	NAME_COLUNM,						// 名称
	DB_NAME_COLUNM						// 数据库字段名称
};

// 名称表格各信息所在列
enum InfoTableViewColunm
{
	NO_COLUNM,							// 序号
	DATA_COLUNM,						// 数据
	REMARKS_COLUNM,						// 备注
	MNEMONIC_COLUNM						// 助记符
};

class LimitLenDelegate : public QStyledItemDelegate
{
public:
    LimitLenDelegate(int maxLen, QObject* parent = nullptr)
        : m_maxLen(maxLen)
        , QStyledItemDelegate(parent)
    {

    }

    virtual QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QLineEdit* lineEdit = new QLineEdit(parent);
        lineEdit->setMaxLength(m_maxLen);
        return lineEdit;
    }

    void setLimitLen(int len) { m_maxLen = len; }

private:
    int                             m_maxLen;                   ///< 可输入的最大长度
};

DataDictionaryDlg::DataDictionaryDlg(QWidget *parent)
	: QWidget(parent)
    , ui(new Ui::DataDictionaryDlg)
    , m_nameModel(new QToolTipModel(true))
	, m_infoModel(new QAutoSeqNoModel(Qt::AlignCenter))
    , m_bKeyExisted(false)
    , m_bDataInfoChanged(false)
    , m_bNameEditted(false)
    , m_bPermit(false)
    , m_edittingType(-1)
    , m_limitLenDelegate(new LimitLenDelegate(1024, this))
{
	ui->setupUi(this);
	InitCtrls();
}

DataDictionaryDlg::~DataDictionaryDlg()
{

}

void DataDictionaryDlg::LoadDataToCtrls()
{
	OnNameTableViewCurrentChanged(GetNameTabCurrentIndex());
}

void DataDictionaryDlg::SetDictData(DisplaySet data)
{
    static QVector<tf::PatientFields::type> filterTypes = {
        /*tf::PatientFields::PATIENT_TYPE,*/ tf::PatientFields::PATIENT_DIAGNOSIS, tf::PatientFields::PATIENT_INSPECT_DEPARTMENT, 
        tf::PatientFields::PATIENT_INSPECT_DOCTOR, tf::PatientFields::PATIENT_INSPECTOR, tf::PatientFields::PATIENT_COST_TYPE,
        tf::PatientFields::PATIENT_AUDITOR, tf::PatientFields::PATIENT_INPATIENT_AREA, tf::PatientFields::PATIENT_ATTENDING_DOCTOR, 
        tf::PatientFields::PATIENT_COMMENT };

    // 清除标记
    m_bDataInfoChanged = false;
    m_bKeyExisted = false;
    m_bNameEditted = false;
    m_changedDict.clear();
    m_edittingName.clear();
    m_edittingType = -1;


    int currentRow = GetNameTabCurrentRow();
    //disconnect(m_nameModel, &QStandardItemModel::itemChanged, this, &DataDictionaryDlg::OnNameTableItemChanged);
    int rowIndex = 0;
    m_storedSet = data;

    // 存储5个自定义项目
    m_cosSettingData.clear();
    auto& cosSettingData = m_cosSettingData;
    cosSettingData.resize(5);
    m_nameCheckList.clear();
    m_dsItems.clear();

    for (auto& d : data.displayItems)
    {
        m_dsItems[d.type] = d;

        // 自定义类型
        if (d.type > ::tf::PatientFields::PATIENT_CUSTOM_BASE && (d.type - ::tf::PatientFields::PATIENT_CUSTOM_BASE) <= cosSettingData.size())
        {
            cosSettingData[(d.type - ::tf::PatientFields::PATIENT_CUSTOM_BASE - 1)] = QString::fromStdString(d.name);
        }
        // 内置类型（且有需要自定义数据字典的类型）
        else if (filterTypes.indexOf((tf::PatientFields::type)d.type) >= 0)
        {
            QString name = ConvertTfEnumToQString((::tf::PatientFields::type)d.type);

            AddTextToTableView(m_nameModel, rowIndex, NAME_COLUNM, name);
            AddTextToTableView(m_nameModel, rowIndex++, DB_NAME_COLUNM, QString::number(d.type));
            m_nameCheckList.append(name);
        }
    }

    // 保存自定义项目的其实位置
    m_customStartRow = rowIndex;

    // 设置5个自定义设置()
    for (int i = 1; i <= 5; ++i)
    {
        auto& name = cosSettingData[i - 1];

        // 获取显示名称
        QString str;
        if (name.isEmpty())
            str = std::move(QString(tr("自定义设置%1")).arg(i));
        else
        {
            str = name;
            m_nameCheckList.append(name);
        }

        AddTextToTableView(m_nameModel, rowIndex, NAME_COLUNM, str);
        if (cosSettingData[i - 1].isEmpty())
            m_nameModel->item(rowIndex, NAME_COLUNM)->setForeground(Qt::gray);
        AddTextToTableView(m_nameModel, rowIndex++, DB_NAME_COLUNM, QString::number(::tf::PatientFields::PATIENT_CUSTOM_BASE + i));
    }

    //connect(m_nameModel, &QStandardItemModel::itemChanged, this, &DataDictionaryDlg::OnNameTableItemChanged);
    ui->name_table_view->selectRow(currentRow);
    LoadDataToCtrls();
}

bool DataDictionaryDlg::isExistUnsaveData()
{
    // 上一次选择的页面发生过变化，提示是否需要保存
    if ((m_bNameEditted&& m_edittingType != GetCurrentType() && m_edittingName != ui->name_edit->text() && !m_edittingName.isEmpty()) || m_bDataInfoChanged)
    {
        return true;
    }

    return false;
}

void DataDictionaryDlg::hideEvent(QHideEvent * event)
{
    m_bDataInfoChanged = false;
    m_changedDict.clear();
    // 界面切换时，清除未保存的数据
    SetDictData(m_storedSet);

    QWidget::hideEvent(event);
}

void DataDictionaryDlg::InitCtrls()
{
	// 初始化名称列表
	m_nameModel->setHorizontalHeaderLabels({tr("名称"),tr("数据库字段名")});
	ui->name_table_view->setModel(m_nameModel);
    m_nameModel->setRowCount(5);
	ui->name_table_view->hideColumn(DB_NAME_COLUNM);
    ui->name_table_view->selectRow(0);

	// 初始化信息表格
	m_infoModel->setHorizontalHeaderLabels({ tr("序号") ,tr("内容") , tr("描述") , tr("助记符")});
    ui->info_table_view->setModel(m_infoModel);
    // 设置表头字体颜色
    m_infoModel->setHeaderData(DATA_COLUNM, Qt::Horizontal, QColor(UI_HEADER_FONT_COLOR), Qt::ForegroundRole);
    m_infoModel->setHeaderData(REMARKS_COLUNM, Qt::Horizontal, QColor(UI_HEADER_FONT_COLOR), Qt::ForegroundRole);
    m_infoModel->setHeaderData(MNEMONIC_COLUNM, Qt::Horizontal, QColor(UI_HEADER_FONT_COLOR), Qt::ForegroundRole);
	// 序号列不可编辑
	ui->info_table_view->setItemDelegateForColumn(NO_COLUNM, new CReadOnlyDelegate(this));
    ui->info_table_view->setItemDelegateForColumn(DATA_COLUNM, m_limitLenDelegate);

	ui->info_table_view->setColumnWidth(0, 175);
	ui->info_table_view->setColumnWidth(1, 505);
	ui->info_table_view->setColumnWidth(2, 360);
	ui->info_table_view->horizontalHeader()->setStretchLastSection(true);
	ui->info_table_view->horizontalHeader()->setMinimumSectionSize(80);

	// 连接槽函数
	connect(ui->flat_save, SIGNAL(clicked()), this, SLOT(OnSaveBtnClicked()));
    connect(ui->info_del_btn, SIGNAL(clicked()), this, SLOT(OnDelBtnClicked()));
    connect(ui->flat_clear_custom, &QPushButton::clicked, this, &DataDictionaryDlg::OnClearCustomBtnClicked);
	//connect(ui->name_table_view, SIGNAL(clicked(const QModelIndex&)), this, SLOT(OnNameTableViewCurrentChanged(const QModelIndex&)));
    connect(ui->name_table_view->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this](const QItemSelection &selected, const QItemSelection &deselected) {
        OnNameTableViewCurrentChanged(GetNameTabCurrentIndex());
    });
    //connect(m_nameModel, &QStandardItemModel::itemChanged, this, &DataDictionaryDlg::OnNameTableItemChanged);
    connect(m_infoModel, &QStandardItemModel::itemChanged, this, &DataDictionaryDlg::OnInfoTableItemChanged);
    connect(ui->info_table_view->selectionModel(), &QItemSelectionModel::selectionChanged, this, &DataDictionaryDlg::OnSelectionChanged);
    //connect(ui->name_table_view, &QTableView::doubleClicked, this, &DataDictionaryDlg::OnNameTableDoubleClicked);
    connect(ui->name_edit, &QLineEdit::textEdited, this, &DataDictionaryDlg::OnNameTextChanged);

    // 注册患者信息页面数据更新消息
    REGISTER_HANDLER(MSG_ID_PATINET_INFO_POSITION_UPDATE, this, OnDataDictUpdated);
    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);
}

void DataDictionaryDlg::UpdateBtnEnable()
{
    // 设置删除按钮的可用状态
    const auto& indexes = ui->info_table_view->selectionModel()->selectedRows();
    int rowCnt = m_infoModel->rowCount();

    ui->info_del_btn->setEnabled(true);

    // 设置按钮禁用
    if (indexes.empty() || (indexes.size() == 1 && indexes.at(0).row() == rowCnt - 1))
    {
        ui->info_del_btn->setEnabled(false);
    }

    // 设置清空自定义按钮的可用状态
    int nameTableRow = GetNameTabCurrentRow();

    bool isPermit = UserInfoManager::GetInstance()->IsPermisson(PSM_DISPLAYSET_DICTIONARY);
    bool bDisbaleCustom = (nameTableRow < 0 || nameTableRow < m_customStartRow) || !isPermit;
    ui->flat_clear_custom->setDisabled(bDisbaleCustom);
}

QList<QString> DataDictionaryDlg::IsNameTableDuplicate()
{
    QList<QString> checkList;
    int curRow = GetNameTabCurrentRow();
    for (int r = 0; r < m_nameModel->rowCount(); ++r)
    {
        // 获取名称 和 类型
        auto nameItem = m_nameModel->item(r);
        auto typeItem = m_nameModel->item(r, DB_NAME_COLUNM);
                
        if (nameItem == nullptr || typeItem == nullptr)
        {
            ULOG(LOG_ERROR, "nameItem or typeItem is nullptr, row is %d", r);
            return std::move(QList<QString>());
        }

        QString name;
        int type = typeItem->text().toInt();
        // 被编辑的项目名称，需要使用右侧输入框中的文字来判断是否重复
        if (type == m_edittingType)
            name = ui->name_edit->text();
        else
            name = nameItem->text();

        if (name.isEmpty())
            return std::move(QList<QString>());

        // 内置的和已经设置过的自定义项目能够找到
        if (m_dsItems.find(type) != m_dsItems.end())
            m_dsItems[type].name = name.toStdString();
        // 如果没有找到，当前名称又发生变化，则为新增加的
        else if (name != nameItem->text())
        {
            DisplaySetItem dsItem;
            dsItem.name = name.toStdString();
            dsItem.type = type;
            dsItem.enable = false;
            dsItem.postion = -1;

            m_dsItems[type] = dsItem;
        }
        else continue;

        checkList.append(name);
    }

    auto&& listToSet = QSet<QString>::fromList(checkList);

    if (listToSet.size() == checkList.size())
        return std::move(checkList);

    return std::move(QList<QString>()); 
}

int DataDictionaryDlg::ShowFaliedReasonDlg(const QString & reason)
{
    return TipDlg(tr("保存失败"), reason).exec();
}

void DataDictionaryDlg::UpdateNameTableDB()
{
    DisplaySet dsSet;
    dsSet.type = PATIENTTYPE;
    auto i = m_dsItems.constBegin();
    while (i != m_dsItems.constEnd()) {
        dsSet.displayItems.emplace_back(i.value());
        ++i;
    }

    std::vector<DisplaySet> savedata;
    if (!DictionaryQueryManager::GetUiDispalySet(savedata))
    {
        ULOG(LOG_ERROR, "Failed to get DisplaySet.");
        return;
    }

    // 更新数据
    for (auto& data : savedata)
    {
        if (data.type == dsSet.type)
        {
            data = dsSet;
        }
    }

    if (!DictionaryQueryManager::SaveUiDisplaySet(savedata))
    {
        ULOG(LOG_ERROR, "ModifyDictionaryInfo Failed");
        SetDictData(m_storedSet);
    }
    else
    {
        SetDictData(dsSet);
        // 通知其他模块数据信息更新
        POST_MESSAGE(MSG_ID_PATINET_DATA_DICT_ITEM_UPDATE, dsSet);
    }
    // 有未保存数据，切换左侧病人类型数据项目时，保存的概率较小;
    // 此处不重新保存数据
    // m_edittingName = GetCurrentName();
}

void DataDictionaryDlg::GetCurrentDictKey()
{
    int type = GetCurrentType();
    if (type >= 0 && m_changedDict.empty())
    {
        m_changedDict = "DataDict_" + QString::number(type).toStdString();
    }
}

void DataDictionaryDlg::OnSaveBtnClicked()
{
    QString newName = ui->name_edit->text();
    if (newName.isEmpty())
    {
        ShowFaliedReasonDlg(tr("未填写自定义项名称！"));
        return;
    }

    // 获取当前选中名称
    QString name = GetCurrentName();

    // 数据信息发生变化
    if (m_bDataInfoChanged &&  !m_changedDict.empty())
    {
        // 存储数据，用于判断是否有重复内容
        std::vector<CustomSetRowData> saveData;
        QList<QString> contentCheckList;
        QList<QString> descriptionCheckList;
        QList<QString> symbolCheckList;

        int rowCnt = m_infoModel->rowCount();

        for (int currentRow = 0; currentRow < rowCnt; currentRow++)
        {
            CustomSetRowData cr;
            QString&& content = m_infoModel->data(m_infoModel->index(currentRow, DATA_COLUNM)).toString();
            QString&& description = m_infoModel->data(m_infoModel->index(currentRow, REMARKS_COLUNM)).toString();
            QString&& symbol = m_infoModel->data(m_infoModel->index(currentRow, MNEMONIC_COLUNM)).toString();

            // 长度限制
            if (content.length() > MAX_CONTENT_LENGGHT || description.length() > MAX_CONTENT_LENGGHT || symbol.length() > MAX_CONTENT_LENGGHT)
            {
                ShowFaliedReasonDlg(tr("第%1行数据存在过长内容，限制长度不大于").arg(currentRow + 1) + QString::number(MAX_CONTENT_LENGGHT));
                return;
            }

            // 内容
            cr.strData = content.toStdString();
            // 描述
            cr.strRemarks = description.toStdString();
            // 助记符
            cr.strMnemonic = symbol.toStdString();

            // bug3128:只需要检查内容是不是为空
            if ((content.isEmpty() /*|| description.isEmpty() || symbol.isEmpty()*/) && currentRow < rowCnt - 1)
            {
                ShowFaliedReasonDlg(tr("第%1行内容未填写！").arg(currentRow + 1));
                return;
            }
            // 内容列存在相同数据
            else if (contentCheckList.contains(content))
            {
                ShowFaliedReasonDlg(tr("第%1列数据有重复数据！").arg(DATA_COLUNM + 1));
                return;
            }
            // 描述列存在相同数据
            else if (descriptionCheckList.contains(description))
            {
                ShowFaliedReasonDlg(tr("第%1列数据有重复数据！").arg(REMARKS_COLUNM + 1));
                return;
            }
            // 助记符列存在相同数据
            else if (symbolCheckList.contains(symbol))
            {
                ShowFaliedReasonDlg(tr("第%1列数据有重复数据！").arg(MNEMONIC_COLUNM + 1));
                return;
            }
            // 最后一行允许空行
            else if (currentRow == rowCnt - 1)
            {
                break;
            }

            contentCheckList.append(content);
            if(!description.isEmpty())
                descriptionCheckList.append(description);
            if (!symbol.isEmpty())
                symbolCheckList.append(symbol);
            saveData.push_back(std::move(cr));
        }

        // 不需要判断saveData是否为空，可能用户就是想清空列表

        std::string value;
        if (!Encode2Json(value, saveData))
        {
            ULOG(LOG_ERROR, "Encode CustomSetData Failed");
            ShowFaliedReasonDlg(tr("编码失败"));
            return;
        }

        // 组装修改的数据
        tf::DictionaryInfo di;
        tf::ResultLong ret;
        di.__set_keyName(m_changedDict);
        di.__set_value(value);

        // 修改
        if (m_bKeyExisted)
        {
            if (!DcsControlProxy::GetInstance()->ModifyDictionaryInfo(di))
            {
                ULOG(LOG_ERROR, "ModifyDictionaryInfo Failed");
                ShowFaliedReasonDlg(tr("修改数据失败"));
                return;
            }
        }
        // 新增
        else
        {
            DcsControlProxy::GetInstance()->AddDictionaryInfo(di, ret);
            if (ret.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
            {
                ULOG(LOG_ERROR, "AddDictionaryInfo Failed");
                ShowFaliedReasonDlg(tr("新增数据失败"));
                return;
            }
            m_bKeyExisted = true;
        }

        // 通知其他模块数据信息更新
        POST_MESSAGE(MSG_ID_PATINET_DATA_DICT_INFO_UPDATE/*, m_changedDict*/);
    }

    // 数据信息已经成功处理，重置标识
    m_changedDict.clear();
    m_bDataInfoChanged = false;


    // 名称是否发生过变化
    if (m_bNameEditted && m_edittingName != ui->name_edit->text())
    {
        // 以下代码处理数据项名称的保存逻辑
        auto&& list = IsNameTableDuplicate();
        m_edittingName.clear();
        m_edittingType = -1;
        // 名称规则检查失败，不可保存
        if (list.isEmpty())
        {
                ShowFaliedReasonDlg(tr("数据项名称不能有重复或者为空。"));
            // 恢复为保存前的数据
            SetDictData(m_storedSet);
        }
        // 需要更新名称表的数据
        else if (list != m_nameCheckList)
        {
            UpdateNameTableDB();
            TipDlg(tr("提示"), tr("保存成功")).exec();
        }
    }
    else
    {
        TipDlg(tr("提示"), tr("保存成功")).exec();
    }
}

void DataDictionaryDlg::OnDelBtnClicked()
{
    if (TipDlg(tr("删除"), tr("确认删除选中数据信息？"), TipDlgType::TWO_BUTTON).exec() != QDialog::Accepted)
        return;

    auto&& rowModelIndexList = ui->info_table_view->selectionModel()->selectedRows();
    if (rowModelIndexList.empty())
        return;

    std::vector<int> rows;
    std::transform(rowModelIndexList.begin(), rowModelIndexList.end(), std::back_inserter(rows), [](auto& item)->int {return item.row(); });
    // 从大到小来排序
    std::sort(rows.begin(), rows.end(), std::greater<int>());
    for (auto row : rows)
    {
        // 移除行
        m_infoModel->removeRow(row);
    }

    m_bDataInfoChanged = true;
    GetCurrentDictKey();
}

void DataDictionaryDlg::OnClearCustomBtnClicked()
{
    if (TipDlg(tr("清空自定义"), tr("确认清空选中数据内容？"), TipDlgType::TWO_BUTTON).exec() != QDialog::Accepted)
        return;

    int type = GetCurrentType();
    if (type < 0)
    {
        ULOG(LOG_ERROR, "type is less than 0");
        return;
    }

    const auto& dbKey = QString("DataDict_%1").arg(type);
    tf::DictionaryInfoQueryCond di;
    tf::ResultLong ret;
    di.__set_keyName(dbKey.toStdString());

    if (m_bKeyExisted && !DcsControlProxy::GetInstance()->DeleteDictionaryInfo(di))
        ShowFaliedReasonDlg("清空自定义数据失败");

    // 删除并保存
    if (m_dsItems.find(type) != m_dsItems.end())
    {
        int pos = m_dsItems[type].postion;
        // 确认是否需要重新排序一次
        if (pos > -1)
        {
            auto i = m_dsItems.begin();
            while (i != m_dsItems.end()) {
                if (i.value().postion > pos)
                   i.value().postion--;
                ++i;
            }
        }

        m_dsItems.remove(type);
        UpdateNameTableDB();
    }
    else
        SetDictData(m_storedSet);
}

void DataDictionaryDlg::OnNameTableViewCurrentChanged(const QModelIndex& index)
{
    // 上一次选择的页面发生过变化，提示是否需要保存
    if (isExistUnsaveData())
    {
        if (TipDlg(tr("保存提示"), tr("是否保存变更内容"), TipDlgType::TWO_BUTTON).exec() == QDialog::Accepted)
        {
            OnSaveBtnClicked();
        }
    }

    m_bKeyExisted = false;
    m_bDataInfoChanged = false;
    m_bNameEditted = false;
    m_edittingName = GetCurrentName();
    m_edittingType = GetCurrentType();
    m_changedDict.clear();

    UpdateBtnEnable();
    disconnect(m_infoModel, &QStandardItemModel::itemChanged, this, &DataDictionaryDlg::OnInfoTableItemChanged);

	// 清空信息表格
	m_infoModel->removeRows(0, m_infoModel->rowCount());

    // 显示名称
    QString name = GetCurrentName();
    // 如果是没有设置名称的自定义项，就不设置名称
    if (GetCurrentType() > ::tf::PatientFields::PATIENT_CUSTOM_BASE && !m_cosSettingData.contains(name))
        name.clear();
    
    // 获取对应的患者信息定义值
    disconnect(ui->name_edit, &QLineEdit::textEdited, this, &DataDictionaryDlg::OnNameTextChanged);
    ui->name_edit->setText(name);
    connect(ui->name_edit, &QLineEdit::textEdited, this, &DataDictionaryDlg::OnNameTextChanged);

    // 获取查询条件
    int type = GetCurrentType();
    if (type >= tf::PatientFields::PATIENT_CUSTOM_BASE)
        ui->name_edit->setEnabled(m_bPermit);
    else
        ui->name_edit->setEnabled(false);

    switch (tf::PatientFields::type(type))
    {
        // 源自禅道bug:3165
        case tf::PatientFields::PATIENT_COMMENT:
            m_limitLenDelegate->setLimitLen(15);
            break;
        // 源自免疫Mantis缺陷:18082
        case tf::PatientFields::PATIENT_NAME:
        case tf::PatientFields::PATIENT_RECORD_NO:
        case tf::PatientFields::PATIENT_BED_NO:
        case tf::PatientFields::PATIENT_PHONE_NO:
        case tf::PatientFields::PATIENT_ADMISSION_NO:
        case tf::PatientFields::PATIENT_ACCOUNT_NO:
            m_limitLenDelegate->setLimitLen(20);
            break;
        default:
            m_limitLenDelegate->setLimitLen(1024);
            break;
    }

    // 先判断是否需要填充默认值，如果查到了对应的字典，则重新删除覆盖
    std::vector<CustomSetRowData> csrDatas;
    if (CommonInformationManager::GetInstance()->GetDefaultDictData(type, csrDatas))
    {
        int row = 0;
        for (const auto& d : csrDatas)
        {
            m_infoModel->setItem(row, NO_COLUNM, new QStandardItem(QString::number(row + 1)));
            m_infoModel->setItem(row, DATA_COLUNM, new QStandardItem(QString::fromStdString(d.strData)));
            m_infoModel->setItem(row, REMARKS_COLUNM, new QStandardItem(QString::fromStdString(d.strRemarks)));
            m_infoModel->setItem(row, MNEMONIC_COLUNM, new QStandardItem(QString::fromStdString(d.strMnemonic)));
            row++;
        }
    }
	// 构造查询条件
	::tf::DictionaryInfoQueryCond   qryCond;
	::tf::DictionaryInfoQueryResp   qryResp;
	qryCond.__set_keyName("DataDict_" + QString::number(type).toStdString());

	// 查询显示设置
	if (!DcsControlProxy::GetInstance()->QueryDictionaryInfo(qryResp, qryCond)
		|| qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
        ULOG(LOG_ERROR, "QueryDictionaryInfo Failed");
        AddInfoTableEmptyRow();
        connect(m_infoModel, &QStandardItemModel::itemChanged, this, &DataDictionaryDlg::OnInfoTableItemChanged);
		return;
    }

	if (qryResp.lstDictionaryInfos.empty())
	{
        ULOG(LOG_INFO, "QueryDictionaryInfo Result Is Empty");
        AddInfoTableEmptyRow();
        connect(m_infoModel, &QStandardItemModel::itemChanged, this, &DataDictionaryDlg::OnInfoTableItemChanged);
		return;
    }

    // 清空信息表格
    m_infoModel->removeRows(0, m_infoModel->rowCount());

    m_bKeyExisted = true;

	// 解析字符串
	std::vector<CustomSetRowData>  veccs;
	if (!DecodeJson(veccs, qryResp.lstDictionaryInfos[0].value))
	{
        ULOG(LOG_ERROR, "DecodeJson CustomSetData Failed");
        AddInfoTableEmptyRow();
        connect(m_infoModel, &QStandardItemModel::itemChanged, this, &DataDictionaryDlg::OnInfoTableItemChanged);
		return;
	}

	int row = 0;

	for (const auto& item : veccs)
	{
		m_infoModel->setItem(row, NO_COLUNM, new QStandardItem(QString::number(row+1)));
		m_infoModel->setItem(row, DATA_COLUNM, new QStandardItem(QString::fromStdString(item.strData)));
		m_infoModel->setItem(row, REMARKS_COLUNM, new QStandardItem(QString::fromStdString(item.strRemarks)));
		m_infoModel->setItem(row, MNEMONIC_COLUNM, new QStandardItem(QString::fromStdString(item.strMnemonic)));
		row++;
	}

    AddInfoTableEmptyRow();

    ui->info_del_btn->setEnabled(row != 0);

	// 默认选中第一行
    ui->info_table_view->selectRow(-1);

    connect(m_infoModel, &QStandardItemModel::itemChanged, this, &DataDictionaryDlg::OnInfoTableItemChanged);
    UpdateBtnEnable();
}

void DataDictionaryDlg::OnSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{
    UpdateBtnEnable();
}

/*void DataDictionaryDlg::OnNameTableItemChanged(QStandardItem * item)
{
    if (item && item->row() >= m_customStartRow && item->row() < m_nameModel->rowCount())
    {
        
    }
}*/

void DataDictionaryDlg::OnInfoTableItemChanged(QStandardItem * item)
{
    if (item)
    {
        int row = item->row();
        int rowCnt = m_infoModel->rowCount();
        // 增加一行数据
        if (row == rowCnt - 1 && !item->text().isEmpty())
        {
            AddInfoTableEmptyRow();
            UpdateBtnEnable();
        }
        // 优化（编辑信息表以后，删除最后的空行，只保留一个空行）
        else
        {
            for (int loopRow = rowCnt - 2; loopRow >= 0; --loopRow)
            {
                bool removeRow = true;
                // 检查是否是空行
                for (int startCol = 1; startCol < m_infoModel->columnCount(); ++startCol)
                {
                    auto item = m_infoModel->item(loopRow, startCol);
                    if (item && !item->text().isEmpty())
                    {
                        removeRow = false;
                        break;
                    }
                }

                if (removeRow)
                    m_infoModel->removeRow(loopRow);
                else
                    break;
            }
        }

        m_bDataInfoChanged = true;
        GetCurrentDictKey();
    }
}

/*void DataDictionaryDlg::OnNameTableDoubleClicked(const QModelIndex & index)
{
    if (!index.isValid())
        return;

    int idxRow = index.row();

    if (idxRow >= m_customStartRow && idxRow < m_nameModel->rowCount())
    {
        ui->name_table_view->edit(index);
    }
}*/

void DataDictionaryDlg::OnDataDictUpdated(DisplaySet dsSet)
{
    SetDictData(dsSet);
}

void DataDictionaryDlg::OnNameTextChanged(const QString & text)
{
    m_bNameEditted = true;
}

void DataDictionaryDlg::OnPermisionChanged()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    m_bPermit = UserInfoManager::GetInstance()->IsPermisson(PSM_DISPLAYSET_DICTIONARY);

    ui->info_table_view->setDisabled(!m_bPermit);
    ui->info_del_btn->setDisabled(!m_bPermit);
    ui->flat_save->setDisabled(!m_bPermit);

    // 自定义清空
    int nameTableRow = GetNameTabCurrentRow();
    bool bDisbaleCustom = (nameTableRow < 0 || nameTableRow < m_customStartRow) || !m_bPermit;
    ui->flat_clear_custom->setDisabled(bDisbaleCustom);
    ui->name_edit->setDisabled(bDisbaleCustom);
}

void DataDictionaryDlg::AddInfoTableEmptyRow()
{
    m_infoModel->setRowCount(m_infoModel->rowCount() + 1);
}

QString DataDictionaryDlg::GetCurrentName()
{
    return m_nameModel->data(m_nameModel->index(
        GetNameTabCurrentRow(), NAME_COLUNM)).toString();
}

int DataDictionaryDlg::GetCurrentType()
{
    auto&& index = GetNameTabCurrentIndex();
    if (!index.isValid())
        return -1;
    QString strData = m_nameModel->data(m_nameModel->index(index.row(), DB_NAME_COLUNM)).toString();
    if (strData.isEmpty())
        return -1;

    return strData.toInt();
}

QModelIndex DataDictionaryDlg::GetNameTabCurrentIndex()
{
    const auto& indexes = ui->name_table_view->selectionModel()->selectedIndexes();

    if (!indexes.isEmpty())
        return indexes[0];

    return QModelIndex();
}

int DataDictionaryDlg::GetNameTabCurrentRow()
{
    const auto& index = GetNameTabCurrentIndex();
    if (!index.isValid())
        return -1;

    return index.row();
}
