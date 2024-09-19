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
/// @file     QCalibrateSet.h
/// @brief    校准设置
///
/// @author   5774/WuHongTao
/// @date     2022年2月24日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月24日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QCalibrateSet.h"
#include "ui_QCalibrateSet.h"
#include <random>
#include <QIcon>
#include <QStandardItemModel>
#include <QTextDocument>
#include <QStyle>
#include <QPainter>
#include <QTextBlock>
#include <QFileDialog>
#include <QAbstractTextDocumentLayout>

#include "shared/QPostionEdit.h"
#include "shared/CommonInformationManager.h"
#include "shared/tipdlg.h"
#include "shared/QComDelegate.h"
#include "shared/ReagentCommon.h"
#include "shared/CReadOnlyDelegate.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/FileExporter.h"
#include "shared/uicommon.h"
#include "manager/UserInfoManager.h"
#include "Serialize.h"
#include "printcom.hpp"

#include "thrift/ch/c1005/C1005LogicControlProxy.h"
#include "thrift/ch/ChLogicControlProxy.h"
#include "thrift/DcsControlProxy.h"

#include "src/common/defs.h"
#include "src/common/TimeUtil.h"
#include "src/common/common.h"
#include "src/common/Mlog/mlog.h"
#include "src/thrift/ch/gen-cpp/ch_constants.h"

#include "CaliBrateCommom.h"
#include "SortHeaderView.h"
#include "CaliBrateRecordDialog.h"

void RichTextDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem op(option);
    QStyledItemDelegate::initStyleOption(&op, index);
    if (op.state.testFlag(QStyle::State_HasFocus))
        op.state = op.state ^ QStyle::State_HasFocus;

    // 保存需要绘制的富文本内容
    QString content = op.text;
    QStyle* pStyle = op.widget ? op.widget->style() : QApplication::style();

    // 先绘制文字以外的内容
    op.text = "";
    pStyle->drawControl(QStyle::CE_ItemViewItem, &op, painter, op.widget);

    // 设置富文本
    QTextDocument doc;
    doc.setHtml(content);

    // 设置字体
    const auto& dFont = op.widget->font();
    doc.setDefaultFont(dFont);

    // 设置文字选项
    const auto& elide = op.textElideMode;
    QTextOption textOp = doc.defaultTextOption();
    // 设置文字是否居中，居中还需要设置 QTextDocument 的宽度
    textOp.setAlignment(Qt::AlignCenter);
    textOp.setWrapMode(QTextOption::NoWrap);
    doc.setDefaultTextOption(textOp);

    const auto& fontHeight = dFont.pixelSize();
    QAbstractTextDocumentLayout::PaintContext ctx;
    ctx.palette = option.palette;

    // 获取文字项的区域，此区域已经在单元格内因style偏移了
    QRect textRect = pStyle->subElementRect(QStyle::SE_ItemViewItemText, &op);
    painter->save();
    
    // 移动到需要绘制的单元格左上角位置
    painter->translate(textRect.topLeft().x(), textRect.y() + 5);
    // 设置截断区域，超出区域不需要显示
    const auto& clipRect = textRect.translated(-textRect.topLeft());
    painter->setClipRect(QRect(clipRect.x(), clipRect.y(), clipRect.width() - 5, clipRect.height()));
    // 设置文档宽度，否则Alignment居中会不生效
    doc.setTextWidth(clipRect.width());

    // 设置文字超长省略号
    QFontMetrics fm = op.widget->fontMetrics();
    QPoint testPoint(clipRect.right(), fm.height() /2);
    int pos = doc.documentLayout()->hitTest(testPoint, Qt::ExactHit);
    int dotWidth = fm.width("...");
    
    // 默认采用右侧省略 (Qt::ElideRight)
    {
        qreal cutPos = doc.documentLayout()->hitTest(QPointF(clipRect.width() - dotWidth, fm.height() / 2), Qt::ExactHit);
        if (cutPos >= 0)
        {
            QTextCursor cursor(&doc);
            cursor.setPosition(cutPos);
            cursor.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
            cursor.insertText("...", cursor.block().charFormat());
            cursor.select(QTextCursor::Document);
        }
    }

    // 绘制
    doc.documentLayout()->draw(painter, ctx);
    painter->restore();
}

QCalibrateSet::QCalibrateSet(QWidget *parent)
    : QWidget(parent),
    m_addDocDialog(nullptr),
    m_modifyDocDialog(nullptr),
    m_SelectModel(nullptr),
    m_CaliBrateGroup(nullptr),
    m_CaliPostionEditDialog(nullptr)
{
    ui = new Ui::QCalibrateSet();
    ui->setupUi(this);
    ResetCaliBrateGroup();

    // 装载搜索选项框
    initCaliProjectNameBox();

    // 监听试剂更新信息
    REGISTER_HANDLER(MSG_ID_CALI_GRP_INFO_UPDATE, this, OnCaliGroupUpdate);
    // 监听项目更新
    REGISTER_HANDLER(MSG_ID_ASSAY_CODE_MANAGER_UPDATE, this, OnAssayUpdated);
    // 设备状态的更新
    REGISTER_HANDLER(MSG_ID_DEVS_STATUS_CHANGED, this, OnDeviceUpdate);
    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, OnPermisionChanged);
    // 登记
    connect(ui->flat_regist, &QPushButton::clicked, this, [this] {
        OnAddNewCalibrateGroup();
    });
    // 修改
    connect(ui->flat_modify, &QPushButton::clicked, this, &QCalibrateSet::OnEditCalibrateGroup);
    // 删除校准组
    connect(ui->del_Button, SIGNAL(clicked()), this, SLOT(OnDeleteCaliBrateGroup()));
    // 项目名称查询
    connect(ui->pushButton_search, &QPushButton::clicked, this, &QCalibrateSet::OnCaliProjectNameSearch);
    /*connect(ui->comboBox_projectName, static_cast<void(QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
        [=](const QString &text) { OnCaliProjectNameSearch(); });*/
    connect(ui->CaliGroup_View->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this](const QItemSelection &selected, const QItemSelection &deselected) {
        Q_UNUSED(selected);
        Q_UNUSED(deselected);

        if (!m_SelectModel)
        {
            return;
        }

        const auto& selecIndexed = m_SelectModel->selectedRows();

        if (selecIndexed.isEmpty())
        {
            ui->flat_modify->setEnabled(false);
            ui->del_Button->setEnabled(false);
        }
        else if (selecIndexed.size() > 1)
        {
            ui->flat_modify->setEnabled(false);
            //ui->del_Button->setEnabled(!m_isRunningStatus);
            ui->del_Button->setEnabled(true);
        }
        else
        {
            ui->flat_modify->setEnabled(true);
            ui->del_Button->setEnabled(true);
           // ui->del_Button->setEnabled(!m_isRunningStatus);
        }

        // 清空序号保持列表
        m_docAssayCodeVec.clear();
        if (m_CaliBrateGroup == nullptr)
        {
            return;
        }

        ch::tf::CaliDocGroup group;
        if (!GetCaliGroupInfo(ui->CaliGroup_View->currentIndex(), group))
            return;

        int column = 1;
        ch::tf::CaliDoc docSave;
        // 清除上次的校准文档的详细信息
        m_CurrentCalis.clear();
        for (ch::tf::CaliDocKeyInfo docKeyInfo : group.caliDocKeyInfos)
        {
            // 查询成功
            if (ShowDocInfo(group.id, docKeyInfo, docSave))
            {
                m_CurrentCalis.push_back(docSave);
                column++;
            }
        }
    });

    // 初始状态筛选按钮不可用，有文字的时候才可以用
    ui->pushButton_search->setEnabled(false);
    connect(ui->comboBox_projectName, &QComboBox::editTextChanged, this, [this] {
        if (ui->comboBox_projectName->lineEdit()->text().isEmpty())
            ui->pushButton_search->setEnabled(false);
        else
            ui->pushButton_search->setEnabled(true);
    });
    connect(ui->comboBox_projectName, &QFilterComboBox::ResetText, this, [this] {
        OnCaliProjectNameSearch();
    });
    connect(ui->export_btn, &QPushButton::clicked, this, &QCalibrateSet::OnExport);
    connect(ui->PrintBtn, &QPushButton::clicked, this, &QCalibrateSet::OnPrint);

    // 默认修改和删除都是禁用的
    ui->flat_modify->setEnabled(false);
    ui->del_Button->setEnabled(false);
    // 最多只能输入50个字符
    ui->comboBox_projectName->lineEdit()->setMaxLength(50);

    RefreshPage();
}

QCalibrateSet::~QCalibrateSet()
{
}

///
/// @brief
///     初始化校准品组
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月24日，新建函数
///
void QCalibrateSet::ResetCaliBrateGroup()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    if (m_CaliBrateGroup == nullptr)
    {
        m_CaliBrateGroup = new QToolTipModel(this);
        m_CaliBrateGroup->clear();

        // 升序 降序 原序
        m_pCaliBrateGroupHeader = new SortHeaderView(Qt::Horizontal, ui->CaliGroup_View);
        //m_pCaliBrateGroupHeader->setStretchLastSection(true);

        ui->CaliGroup_View->setHorizontalHeader(m_pCaliBrateGroupHeader);
        connect(m_pCaliBrateGroupHeader, &SortHeaderView::SortOrderChanged, this, [this](int logicIndex, SortHeaderView::SortOrder order) {
            m_iSortOrder = order;
            m_iSortColIdx = logicIndex;
            QTableView *view = ui->CaliGroup_View;
            //无效index或NoOrder就设置为默认未排序状态
            if (logicIndex < 0 || order == SortHeaderView::NoOrder) {
                m_iSortColIdx = -1;
                SetAscSortByFirstColumn();
            }
            else
            {
                Qt::SortOrder qOrderFlag = order == SortHeaderView::DescOrder ? Qt::DescendingOrder : Qt::AscendingOrder;
                int role = logicIndex == 0 ? Qt::UserRole + 1 : Qt::DisplayRole;
                m_CaliBrateGroup->setSortRole(role);
                view->sortByColumn(logicIndex, qOrderFlag);
            }
        });
        //去掉排序三角样式
        ui->CaliGroup_View->horizontalHeader()->setSortIndicator(-1, Qt::DescendingOrder);
        //-1则还原model默认顺序
        //m_CaliBrateGroupProxyModel->sort(-1, Qt::DescendingOrder);

        ui->CaliGroup_View->setModel(m_CaliBrateGroup);
        // 多选模式
        m_SelectModel = new QItemSelectionModel(m_CaliBrateGroup);
        ui->CaliGroup_View->setSelectionModel(m_SelectModel);
        ui->CaliGroup_View->setItemDelegateForColumn(3, new RichTextDelegate(this));
        ui->CaliGroup_View->setItemDelegateForColumn(4, new CReadOnlyDelegate(this));
        ui->CaliGroup_View->setSelectionMode(QAbstractItemView::ExtendedSelection);
        ui->CaliGroup_View->setSelectionBehavior(QAbstractItemView::SelectRows);
        ui->CaliGroup_View->setEditTriggers(QAbstractItemView::NoEditTriggers);

        ui->CaliGroup_View->setSortingEnabled(true);
        // 选择每一行的槽函数
        connect(ui->CaliGroup_View, SIGNAL(clicked(QModelIndex)), this, SLOT(OnSelectGroup(QModelIndex)));

        // 设置校准品代理
        ui->CaliGroup_View->setItemDelegateForColumn(5, new QPositionDelegate(this));
        ui->CaliGroup_View->setItemDelegateForColumn(6, new QPositionDelegate(this));
        ui->CaliGroup_View->setItemDelegateForColumn(7, new QPositionDelegate(this));
        ui->CaliGroup_View->setItemDelegateForColumn(8, new QPositionDelegate(this));
        ui->CaliGroup_View->setItemDelegateForColumn(9, new QPositionDelegate(this));
        ui->CaliGroup_View->setItemDelegateForColumn(10, new QPositionDelegate(this));

    }

    
    QStringList horheadList;
    horheadList << tr("序号") << tr("校准品名称") << tr("校准品批号") << tr("校准项目") << tr("失效日期") <<
        tr("校准品1") << tr("校准品2") << tr("校准品3") << tr("校准品4") << tr("校准品5") << tr("校准品6");
    m_CaliBrateGroup->setHorizontalHeaderLabels(horheadList);
    m_CaliBrateGroup->setRowCount(20);
    for (int s = 5; s < horheadList.size(); ++s)
        m_CaliBrateGroup->setHeaderData(s, Qt::Horizontal, QColor::fromRgb(2, 91, 199), Qt::ForegroundRole);

	    ui->CaliGroup_View->setColumnWidth(0, 80);
	    ui->CaliGroup_View->setColumnWidth(1, 230);
	    ui->CaliGroup_View->setColumnWidth(2, 145);
		ui->CaliGroup_View->setColumnWidth(3, 322);
	    ui->CaliGroup_View->setColumnWidth(4, 160); 
		ui->CaliGroup_View->setColumnWidth(5, 160);
	    ui->CaliGroup_View->setColumnWidth(6, 160);
	    ui->CaliGroup_View->setColumnWidth(7, 160);
	    ui->CaliGroup_View->setColumnWidth(8, 160);
	    ui->CaliGroup_View->setColumnWidth(9, 160);
	   //m_pCaliBrateGroupHeader->setSectionResizeMode(DOCASSAY, QHeaderView::Stretch);
		ui->CaliGroup_View->horizontalHeader()->setStretchLastSection(true);
}

///
/// @brief刷新设备试剂列表
///     
///
/// @param[in]  devices  设备列表
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月24日，新建函数
///
void QCalibrateSet::RefreshPage(/*std::vector<std::shared_ptr<const tf::DeviceInfo>>& devices*/)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    //Q_UNUSED(devices);

    SetAscSortByFirstColumn();
    // 清除旧数据
    m_CaliGroupSave.clear();


    if (m_CaliBrateGroup == nullptr)
    {
        return;
    }
    m_CaliBrateGroup->removeRows(0, m_CaliBrateGroup->rowCount());

    //ResetCaliBrateGroup();
    ::ch::tf::CaliDocGroupQueryResp ResultDocGroup;
    ::ch::tf::CaliDocGroupQueryCond queryDocGroupCondition;

    if (!ch::LogicControlProxy::QueryCaliDocGroup(ResultDocGroup, queryDocGroupCondition)
        || ResultDocGroup.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
        || ResultDocGroup.lstCaliDocGroup.empty())
    {
        SetPrintAndExportEnabled(false);
        return;
    }

    // 清除历史存储编号
    m_caliDocGroupIndex.clear();
    m_caliAssayIndex.clear();
    SetPrintAndExportEnabled(true);

    int row = 0;
    // 依次显示校准品组的信息
    for (const auto& docGroup : ResultDocGroup.lstCaliDocGroup)
    {
        
		if (!SetTableRowData(row, docGroup))
			continue;
		
        row++;

        // 保存当前批次出现过的序号，用于后续生成序号时 本地序号去重检测
        m_caliDocGroupIndex[docGroup.lot].push_back(docGroup.sn);
    }

    emit m_pCaliBrateGroupHeader->SortOrderChanged(m_iSortColIdx, (SortHeaderView::SortOrder)(m_iSortOrder));
}

///
/// @brief 更新界面，置空输入框
///     
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2022年12月21日，新建函数
///
void QCalibrateSet::ReInitSerachCombox()
{
    ui->comboBox_projectName->setCurrentIndex(-1);
}

///
/// @brief
///     显示校准文档信息
///
/// @param[in]  pos         文档显示的列序
/// @param[in]  docKeyInfo  校准文档的关键信息
/// @param[out]  doc        校准文档的详细信息
///
/// @return true表示查询成功
/// @par History:
/// @li 5774/WuHongTao，2022年2月24日，新建函数
///
bool QCalibrateSet::ShowDocInfo(int64_t groupDb, ch::tf::CaliDocKeyInfo& docKeyInfo, ch::tf::CaliDoc& doc)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 找不到数据就去数据库中获取，否则就从缓存中获取
    if (m_caliDocIndex.find(groupDb) == m_caliDocIndex.end())
    {
        ::ch::tf::CaliDocQueryResp ResultDocs;
        ::ch::tf::CaliDocQueryCond QueryCondition;
        //QueryCondition.__set_level(docKeyInfo.level);
        QueryCondition.__set_lot(docKeyInfo.lot);
        QueryCondition.__set_sn(docKeyInfo.sn);

        if (!ch::LogicControlProxy::QueryCaliDoc(ResultDocs, QueryCondition)
            || ResultDocs.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
            || ResultDocs.lstCaliDocs.empty())
        {
            return false;
        }

        m_caliDocIndex[groupDb] = ResultDocs.lstCaliDocs;
    }

    bool bFound = false;
    for (const auto& d : m_caliDocIndex[groupDb])
    {
        // 匹配校准品组
        if (d.level != docKeyInfo.level ||
            d.lot != docKeyInfo.lot ||
            d.sn != docKeyInfo.sn)
            continue;

        bFound = true;
        doc = d;
        // 依次显示校准品组的信息
        for (const auto& compostion : doc.compositions)
        {
            // 判断是否是未添加的项目，并添加项目
            if (!m_docAssayCodeVec.contains(compostion.assayCode))
            {
                std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(compostion.assayCode);

                // 不存在的项目直接跳过
                if (spAssayInfo == nullptr)
                {
                    ULOG(LOG_ERROR, "Can not find the assay item, assayCode=%d", compostion.assayCode);
                    continue;
                }

                // 放入列表并获取最新index;
                m_docAssayCodeVec.append(compostion.assayCode);
            }
        }
    }

    return bFound;
}

const std::map<int64_t, ch::tf::CaliDocGroup>& QCalibrateSet::GetAllCaliGroups()
{
    return m_CaliGroupSave;
}

const std::vector<ch::tf::CaliDoc>& QCalibrateSet::GetCaliDocsByGroupId(int64_t groupId)
{
    static std::vector<ch::tf::CaliDoc> nullVec;
    if (m_caliDocIndex.find(groupId) != m_caliDocIndex.end())
        return m_caliDocIndex[groupId];

    return nullVec;
}

///
/// @brief
///     点击校准组名称
///
/// @param[in]  index  索引
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月24日，新建函数
///
void QCalibrateSet::OnSelectGroup(QModelIndex index)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 清空序号保持列表
    m_docAssayCodeVec.clear();
    if (m_CaliBrateGroup == nullptr)
    {
        return;
    }

    ch::tf::CaliDocGroup group;
    if (!GetCaliGroupInfo(ui->CaliGroup_View->currentIndex(), group))
        return;

    int column = 1;
    ch::tf::CaliDoc docSave;
    // 清除上次的校准文档的详细信息
    m_CurrentCalis.clear();
    for (ch::tf::CaliDocKeyInfo docKeyInfo : group.caliDocKeyInfos)
    {
        // 查询成功
        if (ShowDocInfo(group.id, docKeyInfo, docSave))
        {
            m_CurrentCalis.push_back(docSave);
            column++;
        }
    }

    if (index.column() - EXPIRYEDATE - (int)group.systemWater > 0)
    {
        // 只有需要设置校准品位置的单元格才响应消息
        if (m_CurrentCalis.size() >= (index.column() - EXPIRYEDATE))
            onCaliDocViewDoubleClicked(index);
    }
}

///
/// @brief
///     修改或者增加校准品组(点击在空行为增加否则为修改)
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月25日，新建函数
///
void QCalibrateSet::OnEditCalibrateGroup()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    QModelIndex index = ui->CaliGroup_View->currentIndex();

    // 允许修改
    if ((index.row() >= 0) && (index.row() < m_CaliGroupSave.size()))
    {
        if (m_modifyDocDialog == nullptr)
        {
            m_modifyDocDialog = new CaliBrateRecordDialog(this);
            m_modifyDocDialog->setMode(SHOWMODE::MODIFY_MODE);
            connect(m_modifyDocDialog, &CaliBrateRecordDialog::closeWindow, this, [=](int64_t dbId) {
                OnCaliInfoUpdatedByUser(dbId);
            });
        }
        ch::tf::CaliDocGroup group;
        if (!GetCaliGroupInfo(ui->CaliGroup_View->currentIndex(), group))
            return;

        m_modifyDocDialog->setInputMode(Input_Modify);
        m_modifyDocDialog->detailShow(m_CurrentCalis, group);
        m_modifyDocDialog->show();
    }
}

///
/// @brief
///     录入校准品组
///
/// @param[in]  InputType  录入方式
///
/// @par History:
/// @li 8090/YeHuaNing，2022年8月24日，新建函数
///
void QCalibrateSet::OnAddNewCalibrateGroup()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    if (m_addDocDialog == nullptr)
    {
        m_addDocDialog = new CaliBrateRecordDialog(this);
        m_addDocDialog->setMode(SHOWMODE::NEW_MODE);
        connect(m_addDocDialog, &CaliBrateRecordDialog::closeWindow, this, [=](int64_t dbId) {
            OnNewCaliGroupAdded(dbId);
        });

        // 手动输入时的序号生成响应
        connect(m_addDocDialog, &CaliBrateRecordDialog::getNewSn, this, [this](const std::string& lot, std::string& sn) {
            // 生成一个当前批号不重复的 编号
            for (size_t i = ::ch::tf::g_ch_constants.MIN_OPEN_REAGENT_CODE; i <= ::ch::tf::g_ch_constants.MAX_REAGENT_CODE; ++i)
            {
                std::string genNum = QString::number(i).toStdString();
                if ((m_caliDocGroupIndex.find(lot) == m_caliDocGroupIndex.end()) || std::find(m_caliDocGroupIndex[lot].begin(), m_caliDocGroupIndex[lot].end(), genNum) == m_caliDocGroupIndex[lot].end())
                {
                    sn = genNum;
                }
            }
        }, Qt::DirectConnection);
    }
    // 初始化设置为扫描登记
    m_addDocDialog->setInputMode(InputType::Input_Scan);
    m_addDocDialog->show();
}

///
/// @brief
///     更新校准数据
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月25日，新建函数
///
void QCalibrateSet::OnUpdateCaliBrateData()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    //std::vector<std::shared_ptr<const tf::DeviceInfo>> devices;
    // 刷新
    RefreshPage(/*devices*/);
}

///
/// @brief
///     校准品文档表格双击信号响应
///
/// @param[in]  index  位置索引
///
/// @par History:
/// @li 8090/YeHuaNing，2022年8月29日，新建函数
///
void QCalibrateSet::onCaliDocViewDoubleClicked(const QModelIndex & index)
{
    // 非第一行，不处理
    // 必须满足位置的编辑条件
    if (!index.isValid() || index.column() < 1
        || m_CaliBrateGroup->rowCount() == 0)
    {
        return;
    }

    if (m_CaliPostionEditDialog == nullptr)
    {
        m_CaliPostionEditDialog = new QPostionEdit(this);
        connect(m_CaliPostionEditDialog, &QPostionEdit::ModifyCaliPostion, this, &QCalibrateSet::OnRefreshPostion);
    }

    m_CaliPostionEditDialog->SetCaliDoc(m_CurrentCalis, index.column() - EXPIRYEDATE);
    m_CaliPostionEditDialog->show();
}

///
/// @brief 
///     当位置信息设置后
///
/// @par History:
/// @li 8090/YeHuaNing，2022年8月29日，新建函数
///
void QCalibrateSet::OnRefreshPostion()
{
    auto docs = m_CaliPostionEditDialog->GetCaliDoc();
    int64_t groupId = -1;
    if (docs && docs->size() > 0)
    {
        QList<ch::tf::CaliDoc>& caliDocs = docs.value();

        // 原则上修改架号不影响原来的校准品内存存放顺序，此处直接顺序赋值新的架号、杯型、位置
        if (caliDocs.size() != m_CurrentCalis.size())
        {
            return;
        }

        ch::tf::CaliDocGroup group;
        if (!GetCaliGroupInfo(ui->CaliGroup_View->currentIndex(), group))
            return;

        groupId = group.id;
        // 如果校准品1使用了系统水， 那么
        if (caliDocs[0].caliDocGroupId == group.id && group.__isset.systemWater && group.systemWater)
        {
            if (caliDocs.size() > 1 && caliDocs[1].__isset.rack)
            {
                caliDocs[0].__set_rack(caliDocs[1].rack);
                caliDocs[0].__set_pos(0 - caliDocs[1].pos);
            }
        }

        for (const auto& doc : caliDocs)
        {
            if (!ch::LogicControlProxy::ModifyCaliDoc(doc))
                ULOG(LOG_ERROR, "ModifyCaliDoc failed! doc db id is %lld.", doc.id);
        }

        POST_MESSAGE(MSG_ID_CH_CALI_SET_UPDATE, tf::UpdateType::UPDATE_TYPE_MODIFY, group, caliDocs);
        OnSelectGroup(ui->CaliGroup_View->currentIndex());
    }

    // 修改了数据,刷新所在行数据
    OnCaliInfoUpdatedByUser(groupId);
}

///
/// @brief 校准品组更新通知
///  
/// @param[in]  infos  更新的校准品组信息
///
/// @par History:
/// @li 8090/YeHuaNing，2022年10月19日，新建函数
///
void QCalibrateSet::OnCaliGroupUpdate(std::vector<ch::tf::CaliGroupUpdate, std::allocator<ch::tf::CaliGroupUpdate>> infos)
{
    FUNC_ENTRY_LOG();
    std::vector<int64_t> changedDbs;

    for (const auto& info : infos)
    {
        ULOG(LOG_INFO, "info=%s", ToString(info));

        if (info.updateType == tf::UpdateType::UPDATE_TYPE_DELETE)
        {
            // 删除保存的校准品组信息
            if (m_CaliGroupSave.find(info.db) != m_CaliGroupSave.end())
            {
                POST_MESSAGE(MSG_ID_CH_CALI_SET_UPDATE, info.updateType, m_CaliGroupSave[info.db], QList<ch::tf::CaliDoc>{});
            }
            
            ClearCacheByGroupId(info.db);
        }
        else if (info.updateType == tf::UpdateType::UPDATE_TYPE_MODIFY)
        {
            // 清理缓存数据
            ClearCacheByGroupId(info.db);

            ch::tf::CaliDocGroup caliGroup;
            if (!GetSingleCaliGroupFromDb(info.db, caliGroup))
            {
                ULOG(LOG_ERROR, "GetSingleCaliGroupFromDb failed! db id is %lld.", info.db);
                continue;
            }

            m_CaliGroupSave[info.db] = std::move(caliGroup);
            changedDbs.emplace_back(info.db);
        }
        else if (info.updateType == tf::UpdateType::UPDATE_TYPE_ADD)
        {
            ch::tf::CaliDocGroup caliGroup;
            if (!GetSingleCaliGroupFromDb(info.db, caliGroup))
            {
                ULOG(LOG_ERROR, "GetSingleCaliGroupFromDb failed! db id is %lld.", info.db);
                continue;
            }

            m_CaliGroupSave[info.db] = std::move(caliGroup);
            changedDbs.emplace_back(info.db);
        }
    }

    // 显示缓存数据
    UpdateCacheCaliInfoToTable();
    for (const auto& db : changedDbs)
    {
        if (m_CaliGroupSave.find(db) == m_CaliGroupSave.end() ||
            m_caliDocIndex.find(db) == m_caliDocIndex.end())
        continue;

        POST_MESSAGE(MSG_ID_CH_CALI_SET_UPDATE, tf::UpdateType::UPDATE_TYPE_MODIFY, m_CaliGroupSave[db], m_caliDocIndex[db]);
    }
}

///
/// @brief
///     删除校准品组
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月25日，新建函数
///
void QCalibrateSet::OnDeleteCaliBrateGroup()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    if (CommonInformationManager::GetInstance()->IsExistDeviceRuning(tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY) ||
        CommonInformationManager::GetInstance()->IsExistDeviceRuning(tf::AssayClassify::ASSAY_CLASSIFY_ISE))
    {
        // 删除之前先弹出一个对话框，然后进行选择
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("提示"), tr("检测中不能删除校准品！"), TipDlgType::SINGLE_BUTTON));
        pTipDlg->exec();
        return;
    }

    // 删除之前先弹出一个对话框，然后进行选择
    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("删除"), tr("确定删除选中数据?"), TipDlgType::TWO_BUTTON));
    if (QDialog::Rejected == pTipDlg->exec())
    {
        return;
    }

    std::set<int, std::greater<int>> uiRows;
    if (!m_SelectModel)
    {
        return;
    }

    QModelIndexList curIndexList = m_SelectModel->selectedRows();
    for(const auto& index : curIndexList)
    {
        uiRows.insert(index.row());
    }

    for (auto row : uiRows)
    {
        if (row >= m_CaliGroupSave.size())
        {
            continue;
        }
        ch::tf::CaliDocGroup group;
        if (!GetCaliGroupInfo(m_CaliBrateGroup->index(row, 0), group))
            continue;
        // 删除对应的校准品组
        ch::tf::CaliDocGroupQueryCond caliGroupDeleteCondition;
        caliGroupDeleteCondition.__set_id(group.id);
        // 删除对应的
        if (!ch::LogicControlProxy::DeleteCaliDocGroup(caliGroupDeleteCondition))
        {
            ULOG(LOG_ERROR, "DeleteCaliDocGroup failed! db id is %lld.", group.id);
            return;
        }

        POST_MESSAGE(MSG_ID_CH_CALI_SET_UPDATE, tf::UpdateType::UPDATE_TYPE_DELETE, group, QList<ch::tf::CaliDoc>{});

        // 删除表格中的数据
        m_CaliBrateGroup->removeRow(row);
        // 清除缓存数据
        ClearCacheByGroupId(group.id);

        // 更新操作日志
        QString record = tr("删除了校准品组，校准品组名称：") + QString::fromStdString(group.name) \
            + tr("，校准品组批号：") + QString::fromStdString(group.lot) \
            + tr("，校准品组编号：") + QString::fromStdString(group.sn);

        ::tf::ResultLong _return;
        ::tf::OperationLog ol;
        auto spUser = UserInfoManager::GetInstance()->GetLoginUserInfo();
        if (spUser)
        {
            ol.__set_user(spUser->nickname);
        }
        //ol.__set_deviceSN("4");
        ol.__set_deviceClassify(tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY);
        ol.__set_operationType(tf::OperationType::DEL);
        ol.__set_operationTime(QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss").toStdString());
        ol.__set_operationRecord(record.toStdString());
        if (!DcsControlProxy::GetInstance()->AddOperationLog(_return, ol))
        {
            ULOG(LOG_ERROR, "AddOperationLog failed, lot:%s, sn:%s.", group.lot, group.sn);
        }
    }
    
    // 重新更新一次显示
    UpdateCacheCaliInfoToTable();
    SetPrintAndExportEnabled(!m_CaliGroupSave.empty());
}


///
/// @brief 校准项目查询
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2022年12月19日，新建函数
///
void QCalibrateSet::OnCaliProjectNameSearch()
{
    //获取需要查找的项目信息
    auto&& projectName = ui->comboBox_projectName->currentText();

    // 如果在空的时候进行查找，直接返回
    if (projectName != m_filterAssayName)
    {
        m_filterAssayName = projectName;
        UpdateCacheCaliInfoToTable();
    }
}

///
/// @brief 根据项目名称，判断是否是目标校准组，也就是该项目组是否使用了该校准组
///
/// @param[in]  projectName  项目名称
/// @param[in]  docGroup     待判断的校准组
///
/// @return  true:该项目使用了该校准组   false:该项目没有使用该校准组
///
/// @par History:
/// @li 8580/GongZhiQiang，2022年12月21日，新建函数
///
bool QCalibrateSet::isAimCaliDocGroup(const std::string &projectName, const ch::tf::CaliDocGroup &docGroup)
{
    // 获取目标校准项目名称的code
    std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(projectName);
    // 判断是否为空指针
    if (spAssayInfo == nullptr)
    {
        return false;
    }

    for (const ch::tf::CaliDocKeyInfo& docKeyInfo : docGroup.caliDocKeyInfos) {

        ::ch::tf::CaliDocQueryResp ResultDocs;
        ::ch::tf::CaliDocQueryCond QueryCondition;
        QueryCondition.__set_level(docKeyInfo.level);
        QueryCondition.__set_lot(docKeyInfo.lot);
        QueryCondition.__set_sn(docKeyInfo.sn);

        if (!ch::LogicControlProxy::QueryCaliDoc(ResultDocs, QueryCondition)
            || ResultDocs.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
            || ResultDocs.lstCaliDocs.empty())
        {
            continue;
        }


        for (const auto& compostion : ResultDocs.lstCaliDocs[0].compositions) {
            if (compostion.assayCode == spAssayInfo->assayCode)	return true;
        }

    }

    return false;
}

void QCalibrateSet::SetAscSortByFirstColumn()
{
    m_CaliBrateGroup->setSortRole(Qt::UserRole + 1);
    ui->CaliGroup_View->sortByColumn(0, Qt::AscendingOrder);
    ui->CaliGroup_View->horizontalHeader()->setSortIndicator(-1, Qt::DescendingOrder); 
    //去掉排序三角样式;
    //pCaliBrateGroupHeader->setSortIndicator(-1, Qt::DescendingOrder); //去掉排序三角样式
    //m_iSortOrder = SortHeaderView::NoOrder;
    //m_iSortColIdx = -1;


    // 第一列按位置升序排列
    //m_CaliBrateGroup->setSortRole(Qt::UserRole+1);
    //ui->CaliGroup_View->sortByColumn(0, Qt::AscendingOrder);
    ////去掉排序三角样式
    //ui->CaliGroup_View->horizontalHeader()->setSortIndicator(-1, Qt::DescendingOrder);
    ////-1则还原model默认顺序
    //m_CaliBrateGroupProxyModel->sort(-1, Qt::DescendingOrder);
}

bool QCalibrateSet::GetCaliGroupInfo(const QModelIndex & index, ch::tf::CaliDocGroup & caliGroup)
{
    if (!index.isValid())
        return false;

    // 获取序号列显示数据
    const auto& varData = m_CaliBrateGroup->data(m_CaliBrateGroup->index(index.row(), 0),Qt::UserRole + 1);
    if (!varData.isValid())
        return false;

    // 将序号转为数组索引
    int64_t dbId = varData.toLongLong();

    // 获取校准品组信息
    caliGroup = m_CaliGroupSave[dbId];
    return true;
}

QVector<int> QCalibrateSet::GetInitColWidth()
{
    QVector<int> vecColWidth;

    vecColWidth.append(100);
    vecColWidth.append(120);
    vecColWidth.append(150);
    vecColWidth.append(350);
    vecColWidth.append(150);
    vecColWidth.append(150);
    vecColWidth.append(150);
    vecColWidth.append(150);
    vecColWidth.append(150);
    vecColWidth.append(150);
    vecColWidth.append(150);

    return vecColWidth;
}

QVector<int> QCalibrateSet::GetCurColWidth()
{
    QVector<int> vecColWidth;

    int tableColCount = m_CaliBrateGroup->columnCount();

    for (int i = 0; i < tableColCount; ++i)
    {
        vecColWidth.append(ui->CaliGroup_View->columnWidth(i));
    }

    return vecColWidth;
}

void QCalibrateSet::SetColWidth(const QVector<int>& vecWidth)
{
    if (vecWidth.isEmpty())
        return;

    int vecSize = vecWidth.size();
    int tableColCount = m_CaliBrateGroup->columnCount();

    for (int i = 0; i < tableColCount; ++i)
    {
        if (i >= vecSize)
            break;

        ui->CaliGroup_View->setColumnWidth(i, vecWidth[i]);
    }

    //ui->CaliGroup_View->horizontalHeader()->setStretchLastSection(true);
}

void QCalibrateSet::SetPrintAndExportEnabled(bool enabled)
{
    ui->PrintBtn->setEnabled(enabled);
    ui->export_btn->setEnabled(enabled);
}

void QCalibrateSet::OnCaliInfoUpdatedByUser(int64_t dbId)
{
    ch::tf::CaliDocGroup caliGroup;
    const auto& index = ui->CaliGroup_View->currentIndex();

    // 非正常数据，不处理
    if (!GetCaliGroupInfo(index, caliGroup))
        return;

    ClearCacheByGroupId(dbId);
    GetSingleCaliGroupFromDb(dbId, caliGroup);

    if (!SetTableRowData(index.row(), caliGroup))
        ULOG(LOG_ERROR, "SetTableRowData failed! db id is %lld.", dbId);

    ui->CaliGroup_View->selectRow(index.row());
    // 重置一下缓存的信息
    OnSelectGroup(ui->CaliGroup_View->currentIndex());
    SetPrintAndExportEnabled(!m_CaliGroupSave.empty());
}

void QCalibrateSet::OnNewCaliGroupAdded(int64_t dbId)
{
    ch::tf::CaliDocGroup caliGroup;
    if (!GetSingleCaliGroupFromDb(dbId, caliGroup) || !SetTableRowData(m_CaliBrateGroup->rowCount(), caliGroup))
        ULOG(LOG_ERROR, "SetTableRowData failed! db id is %lld.", dbId);

    // 排序
    emit m_pCaliBrateGroupHeader->SortOrderChanged(m_iSortColIdx, (SortHeaderView::SortOrder)(m_iSortOrder));
    SetPrintAndExportEnabled(!m_CaliGroupSave.empty());
}

void QCalibrateSet::hideEvent(QHideEvent * event)
{
    SetAscSortByFirstColumn();

    QWidget::hideEvent(event);
}

void QCalibrateSet::showEvent(QShowEvent * event)
{
    // 基类处理
    QWidget::showEvent(event);
    POST_MESSAGE(MSG_ID_CURRENT_MODEL_NAME, QString("> ") + tr("校准") + QString(" > ") + tr("校准设置"));
}

void QCalibrateSet::OnAssayUpdated()
{
    // 装载搜索选项框
    initCaliProjectNameBox();

    UpdateCacheCaliInfoToTable();
}

void QCalibrateSet::OnDeviceUpdate(class tf::DeviceInfo deviceInfo)
{
    if (CommonInformationManager::GetInstance()->IsExistDeviceRuning(tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY) ||
        CommonInformationManager::GetInstance()->IsExistDeviceRuning(tf::AssayClassify::ASSAY_CLASSIFY_ISE))
    {
        //ui->del_Button->setEnabled(false);
        m_isRunningStatus = true;
        return;
    }

    // 设备运行状态变化后，需要选中了项目才能置为可以点击
    //ui->del_Button->setEnabled(!(ui->CaliGroup_View->selectionModel()->selectedIndexes().isEmpty()));
    m_isRunningStatus = false;
}

void QCalibrateSet::OnPermisionChanged()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    std::shared_ptr<UserInfoManager> userPms = UserInfoManager::GetInstance();
    ui->export_btn->setVisible(userPms->IsPermisson(PSM_CALI_EXPORT_CURVE));
}



void QCalibrateSet::OnExport()
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

    bool bRet = false;
    QFileInfo FileInfo(fileName);
    QString strSuffix = FileInfo.suffix();
    if (strSuffix == "pdf")
    {
        ChCaliRecordInfo Info;
        GetPrintExportInfo(Info);
        QString strPrintTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
        Info.strPrintTime = strPrintTime.toStdString();
        std::string strInfo = GetJsonString(Info);
        ULOG(LOG_INFO, "Print datas : %s", strInfo);
        std::string unique_id;
        QString strDirPath = QCoreApplication::applicationDirPath();
        QString strUserTmplatePath = strDirPath + "/resource/exportTemplate/ExportCaliRecordInfoCh.lrxml";
        int irect = printcom::printcom_async_assign_export(strInfo, fileName.toStdString(), strUserTmplatePath.toStdString(), unique_id);
        ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);
        bRet = true;
    }
    else
    {
        QStringList strExportTextList;

        // 导出校准品信息
        if (!ExportCalibratorInfo(strExportTextList))
        {
            ULOG(LOG_ERROR, "ExportCaliInfo failed!");
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

void QCalibrateSet::OnPrint()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
    ChCaliRecordInfo Info;
    QString strPrintTime = QDateTime::currentDateTime().toString("yyyy/MM/dd hh:mm:ss");
    Info.strInstrumentModel = "C 1000";
    Info.strPrintTime = strPrintTime.toStdString();
    GetPrintExportInfo(Info);

    std::string strInfo = GetJsonString(Info);
    ULOG(LOG_INFO, "Print datas : %s", strInfo);
    std::string unique_id;
    int irect = printcom::printcom_async_print(strInfo, unique_id);
    ULOG(LOG_INFO, "Print datas retrun value: %d ID: %s", irect, unique_id);
}

void QCalibrateSet::UpdateCacheCaliInfoToTable()
{
    if (m_CaliBrateGroup == nullptr)
    {
        return;
    }
    m_CaliBrateGroup->removeRows(0, m_CaliBrateGroup->rowCount());

    SetAscSortByFirstColumn();
    int row = 0;
    // 依次显示校准品组的信息
    for (const auto& docGroup : m_CaliGroupSave)
    {
        const auto& group = docGroup.second;
        if (!SetTableRowData(row, group))
            continue;

        row++;

        // 保存当前批次出现过的序号，用于后续生成序号时 本地序号去重检测
        m_caliDocGroupIndex[group.lot].push_back(group.sn);
    }

    emit m_pCaliBrateGroupHeader->SortOrderChanged(m_iSortColIdx, (SortHeaderView::SortOrder)(m_iSortOrder));
}

bool QCalibrateSet::SetTableRowData(int row, const ch::tf::CaliDocGroup & docGroup)
{
    bool showLine = false;

    // 如果关键信息时空，则下一个
    if (docGroup.caliDocKeyInfos.empty() || !docGroup.__isset.id || docGroup.id <= 0)
    {
        return false;
    }

    int column = 0;
    QString seq = QString::number(row + 1);
    // 新增项目
    if (row >= m_CaliBrateGroup->rowCount())
    {
        auto seqItem = CenterAligmentItem(seq);
        m_CaliBrateGroup->setItem(row, column, seqItem);
    }
    // 修改项目（防止表头列宽变化）
    else
    {
        m_CaliBrateGroup->setData(m_CaliBrateGroup->index(row, column), seq, Qt::DisplayRole);
    }
    m_CaliBrateGroup->setData(m_CaliBrateGroup->index(row, column++), static_cast<qlonglong>(docGroup.id), Qt::UserRole + 1);

    // 校准组名称
    m_CaliBrateGroup->setData(m_CaliBrateGroup->index(row, column++), QString::fromStdString(docGroup.name), Qt::DisplayRole);
    // 校准品组批号
    m_CaliBrateGroup->setData(m_CaliBrateGroup->index(row, column++), QString::fromStdString(docGroup.lot), Qt::DisplayRole);
    // 校准项目
    column++;
    // 有效期（只显示日期，不显示时间）
    // 判断日期是否已经失效(判断标准：当天过0点就算过期了，剩余的几小时舍去)
    if (TimeStringToPosixTime(docGroup.expiryTime).date() <= boost::posix_time::second_clock::local_time().date())
    {
        //m_CaliBrateGroup->setData(m_CaliBrateGroup->index(row, column), UI_REAGENT_WARNCOLOR, Qt::UserRole + DELEGATE_COLOR_OFFSET_POS);
        m_CaliBrateGroup->setData(m_CaliBrateGroup->index(row, column), QColor(UI_REAGENT_WARNFONT), Qt::ForegroundRole);
        m_CaliBrateGroup->setData(m_CaliBrateGroup->index(row, column), QColor(UI_REAGENT_WARNFONT), Qt::UserRole + 1);
    }
    else
    {
        //m_CaliBrateGroup->setData(m_CaliBrateGroup->index(row, column), m_CaliBrateGroup->index(row, column-1).data(Qt::UserRole + DELEGATE_COLOR_OFFSET_POS), Qt::UserRole + DELEGATE_COLOR_OFFSET_POS);
        m_CaliBrateGroup->setData(m_CaliBrateGroup->index(row, column), m_CaliBrateGroup->index(row, column - 1).data(Qt::ForegroundRole), Qt::ForegroundRole);
        m_CaliBrateGroup->setData(m_CaliBrateGroup->index(row, column), m_CaliBrateGroup->index(row, column - 1).data(Qt::UserRole + 1), Qt::UserRole + 1);
    }
    // 有效期（只显示日期，不显示时间）
    const QString& date = QString::fromStdString(boost::gregorian::to_iso_extended_string(TimeStringToPosixTime(docGroup.expiryTime).date()));
    const auto& strDate = QString::fromStdString(boost::gregorian::to_iso_extended_string(TimeStringToPosixTime(docGroup.expiryTime).date()));
    m_CaliBrateGroup->setData(m_CaliBrateGroup->index(row, column++), strDate, Qt::DisplayRole);

    m_CaliGroupSave[docGroup.id] = docGroup;
    ch::tf::CaliDoc doc;

    // 显示校准品信息
    for (ch::tf::CaliDocKeyInfo docKeyInfo : docGroup.caliDocKeyInfos)
    {
        m_docAssayCodeVec.clear();

        // 查询成功
        if (ShowDocInfo(docGroup.id, docKeyInfo, doc))
        {
            if ((int)docGroup.systemWater == doc.level)
            {
                // 设置校准品的架号信息等
                m_CaliBrateGroup->setData(m_CaliBrateGroup->index(row, (int)EXPIRYEDATE + doc.level), tr("系统水"), Qt::DisplayRole);
                m_CaliBrateGroup->setData(m_CaliBrateGroup->index(row, (int)EXPIRYEDATE + doc.level), QVariant(), Qt::UserRole + DELEGATE_POSITION_OFFSET_POS);
            }
            else if (!doc.rack.empty())   //架号为空，则异常; 设置校准品的架号信息等
            {
                QString rack_pos = QString::fromStdString(doc.rack) + "-" + QString::number(doc.pos);
                m_CaliBrateGroup->setData(m_CaliBrateGroup->index(row, (int)EXPIRYEDATE + doc.level), rack_pos, Qt::DisplayRole);
                if (doc.tubeType == tf::TubeType::TUBE_TYPE_MICRO)
                    m_CaliBrateGroup->setData(m_CaliBrateGroup->index(row, (int)EXPIRYEDATE + doc.level), QPixmap(QString(":/Leonis/resource/image/icon-micro-cup-hv.png")), Qt::UserRole + DELEGATE_POSITION_OFFSET_POS);
                else if (doc.tubeType == tf::TubeType::TUBE_TYPE_NORMAL)
                    m_CaliBrateGroup->setData(m_CaliBrateGroup->index(row, (int)EXPIRYEDATE + doc.level), QPixmap(QString(":/Leonis/resource/image/icon-normal-cup-hv.png")), Qt::UserRole + DELEGATE_POSITION_OFFSET_POS);
                else if (doc.tubeType == tf::TubeType::TUBE_TYPE_ORIGIN)
                    m_CaliBrateGroup->setData(m_CaliBrateGroup->index(row, (int)EXPIRYEDATE + doc.level), QPixmap(QString(":/Leonis/resource/image/icon-ori-hv.png")), Qt::UserRole + DELEGATE_POSITION_OFFSET_POS);
                else if (doc.tubeType == tf::TubeType::TUBE_TYPE_STORE)
                    m_CaliBrateGroup->setData(m_CaliBrateGroup->index(row, (int)EXPIRYEDATE + doc.level), QPixmap(QString(":/Leonis/resource/image/icon-store-hv.png")), Qt::UserRole + DELEGATE_POSITION_OFFSET_POS);
            }
            else
            {
                m_CaliBrateGroup->setData(m_CaliBrateGroup->index(row, (int)EXPIRYEDATE + doc.level), tr("请设置位置"), Qt::DisplayRole);
                m_CaliBrateGroup->setData(m_CaliBrateGroup->index(row, (int)EXPIRYEDATE + doc.level), QVariant(), Qt::UserRole + DELEGATE_POSITION_OFFSET_POS);
            }

            column++;
        }
    }

    // 清理多余的数据
    while (column <= CALILEVEL6)
    {
        m_CaliBrateGroup->setData(m_CaliBrateGroup->index(row, column), QVariant(), Qt::DisplayRole);
        m_CaliBrateGroup->setData(m_CaliBrateGroup->index(row, column), QVariant(), Qt::UserRole + DELEGATE_POSITION_OFFSET_POS);

        column++;
    }

    QStringList assays;
    bool hasHtml = false; //判定是否存在html标记
    for (const auto& assayCode : m_docAssayCodeVec)
    {
        // 不需要检查项目，容器内的编号都是已经检查过的才存入的
        std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(assayCode);
        if (!spAssayInfo || spAssayInfo->assayName.empty())
            continue;
        QString name;
        const auto& assayName = QString::fromStdString(spAssayInfo->assayName);
        if (docGroup.disableAssayCodes.find(assayCode) != docGroup.disableAssayCodes.end())
        {
            name = "<span style=\"color:#a0a0a4\">" + assayName + "</span>";
            hasHtml = true;
        }
        else
            name = assayName;

        assays.push_back(name);
        if (!showLine && (m_filterAssayName.isEmpty() || assayName.contains(m_filterAssayName, Qt::CaseInsensitive)))
            showLine = true;
    }

    // 如果不显示当前行，直接删除当前行
    if (!showLine)
    {
        m_CaliBrateGroup->removeRow(row);
        return false;
    }

    if (!assays.empty())
    {
        // 如果没有标记，则增加一个html标记
        // 用于tooltip可以自动换行
        if (!hasHtml)
        {
            assays[0] = "<span style=\"background-color:transparent\">" + assays[0] + "</span>";
        }
        const auto& assayNames = assays.join(tr("、"));
        // 保存校准品组项目，用于筛选(但是有富文本信息，暂时不适合用于优化，方案还在探索中)
        m_caliAssayIndex[docGroup.id] = assays;
        m_CaliBrateGroup->setData(m_CaliBrateGroup->index(row, DOCASSAY), assayNames, Qt::DisplayRole);
        m_CaliBrateGroup->setData(m_CaliBrateGroup->index(row, DOCASSAY), assayNames, Qt::ToolTipRole);
        //m_CaliBrateGroup->setData(m_CaliBrateGroup->index(row, DOCASSAY), "<span style=\"background-color:transparent\">" + assayNames + "</span>", Qt::ToolTipRole);
        //m_CaliBrateGroup->setData(m_CaliBrateGroup->index(row, DOCASSAY), "<span style=\"color:#565656\">" + assayNames + "</span>", Qt::ToolTipRole);
    }

    return true;
}

bool QCalibrateSet::ClearCacheByGroupId(int64_t groupId)
{
    // 删除保存的校准品组信息
    if (m_CaliGroupSave.find(groupId) != m_CaliGroupSave.end())
    {
        m_CaliGroupSave.erase(groupId);
    }
    // 删除校准品组中的名称
    if (m_caliAssayIndex.find(groupId) != m_caliAssayIndex.end())
    {
        m_caliAssayIndex.erase(groupId);
    }
    // 删除保存校准品组的校准文档数据
    if (m_caliDocIndex.find(groupId) != m_caliDocIndex.end())
    {
        m_caliDocIndex.erase(groupId);
    }

    return true;
}

bool QCalibrateSet::GetSingleCaliGroupFromDb(int64_t groupId, ch::tf::CaliDocGroup& docGroup)
{
    ::ch::tf::CaliDocGroupQueryResp ResultDocGroup;
    ::ch::tf::CaliDocGroupQueryCond queryDocGroupCondition;
    queryDocGroupCondition.__set_id(groupId);

    if (!ch::LogicControlProxy::QueryCaliDocGroup(ResultDocGroup, queryDocGroupCondition)
        || ResultDocGroup.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS
        || ResultDocGroup.lstCaliDocGroup.empty())
    {
        return false;
    }

    docGroup = ResultDocGroup.lstCaliDocGroup[0];

    return true;
}

bool QCalibrateSet::ExportCalibratorInfo(QStringList& strExportTextList)
{
    //序号	校准品名称	校准品批号	校准项目	失效日期	校准品1	校准品2	校准品3	校准品4	校准品5	校准品6
    strExportTextList.push_back(QString(tr("序号") + "\t" + tr("校准品名称") + "\t" + tr("校准品批号") + "\t" + 
        tr("校准项目") + "\t" + tr("失效日期") + "\t" + tr("校准品1") + "\t" + tr("校准品2") + "\t" + 
        tr("校准品3") + "\t" + tr("校准品4") + "\t" + tr("校准品5") + "\t" + tr("校准品6")));

    for (int i= 0; i < m_CaliBrateGroup->rowCount(); ++i)
    {
        QString rowData;
        ch::tf::CaliDocGroup group;
        if (!GetCaliGroupInfo(m_CaliBrateGroup->index(i , 0), group))
            continue;
        // 序号
        rowData += m_CaliBrateGroup->index(i, 0).data().toString();
        // 校准品名称
        rowData += "\t" + QString::fromStdString(group.name);
        // 校准品批号
        rowData += "\t" + QString::fromStdString(group.lot);
        // 校准项目
        QTextDocument textDoc;
        textDoc.setHtml(m_CaliBrateGroup->index(i, DOCASSAY).data().toString());
        rowData += "\t" + textDoc.toPlainText();
        // 失效日期
        rowData += "\t" + QString::fromStdString(boost::gregorian::to_iso_extended_string(TimeStringToPosixTime(group.expiryTime).date()));

        // 能获取到校准品信息
        if (m_caliDocIndex.find(group.id) != m_caliDocIndex.end() && !m_caliDocIndex[group.id].empty())
        {
            const auto& caliDocs = m_caliDocIndex[group.id];
            int docSize = caliDocs.size();
            for (int i = 0; i < 6; ++i)
            {
                rowData += "\t";
                if (group.systemWater && i == 0)
                {
                    rowData += tr("系统水");
                }
                else if (i >= docSize || caliDocs[i].rack.empty())
                {
                    //rowData += "\t";
                }
                else
                {
                    rowData += QString::fromStdString(caliDocs[i].rack);
                    rowData += "-" + QString::number(caliDocs[i].pos);
                    QString tubeType = (caliDocs[i].tubeType != tf::TubeType::TUBE_TYPE_INVALID ? ConvertTfEnumToQString(caliDocs[i].tubeType) : QString(""));
                    if (!tubeType.isEmpty())
                    {
                        rowData += " / ";
                        rowData += tubeType;
                    }
                }
            }
        }
        else
        {
            // 校准品1-6
            rowData += "\t\t\t\t\t\t";
        }

        strExportTextList.push_back(rowData);
    }

    return true;
}

void QCalibrateSet::GetPrintExportInfo(ChCaliRecordInfo& info)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    int iRow = m_CaliBrateGroup->rowCount();
    for (int i = 0; i < iRow; ++i)
    {
        ch::tf::CaliDocGroup group;
        if (!GetCaliGroupInfo(m_CaliBrateGroup->index(i, 0), group))
            continue;

        ChCaliRecord Record;
        vector<std::string*> lvs = { &Record.strCalibrator1, &Record.strCalibrator2, &Record.strCalibrator3,
            &Record.strCalibrator4, &Record.strCalibrator5, &Record.strCalibrator6 };

        // 序号
        Record.strIndex = m_CaliBrateGroup->index(i, 0).data().toString().toStdString();
        // 校准品名称
        Record.strCaliName = group.name;
        // 校准品批号
        Record.strCaliLot = group.lot;
        // 校准项目
        QTextDocument textDoc;
        textDoc.setHtml(m_CaliBrateGroup->index(i, DOCASSAY).data().toString());
        Record.strCaliAssays = textDoc.toPlainText().toStdString();
        // 失效日期
        Record.strExpirationDate = boost::gregorian::to_iso_extended_string(TimeStringToPosixTime(group.expiryTime).date());

        // 能获取到校准品信息
        if (m_caliDocIndex.find(group.id) != m_caliDocIndex.end() && !m_caliDocIndex[group.id].empty())
        {
            const auto& caliDocs = m_caliDocIndex[group.id];
            int docSize = caliDocs.size();
            for (int i = 0; i < 6; ++i)
            {
                QString rowData;
                if (group.systemWater && i == 0)
                {
                    rowData += tr("系统水");
                }
                else if (i >= docSize || caliDocs[i].rack.empty())
                {
                }
                else
                {
                    rowData += QString::fromStdString(caliDocs[i].rack);
                    rowData += "-" + QString::number(caliDocs[i].pos);
                    QString tubeType = (caliDocs[i].tubeType != tf::TubeType::TUBE_TYPE_INVALID ? ConvertTfEnumToQString(caliDocs[i].tubeType) : QString(""));
                    if (!tubeType.isEmpty())
                    {
                        rowData += " / ";
                        rowData += tubeType;
                    }
                }

                *lvs[i] = rowData.toStdString();
            }
        }
        else
        {
            // 校准品1-6
            Record.strCalibrator1 = "*";
            Record.strCalibrator2 = "*";
            Record.strCalibrator3 = "*";
            Record.strCalibrator4 = "*";
            Record.strCalibrator5 = "*";
            Record.strCalibrator6 = "*";
        }

        info.vecRecord.push_back(Record);
    }
}

///
/// @brief
///     初始化生化校准设置界面中的查询名称
/// 
/// @return 
///
/// @par History:
/// @li 8580/GongzhiQiang，2022年12月19日，新建函数
///
void QCalibrateSet::initCaliProjectNameBox()
{

    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 用一个有序容器进行存储项目名称
    QStringList projectNames;
    projectNames.clear();


    // 获取所有项目信息
    auto assayMap = CommonInformationManager::GetInstance()->GetGeneralAssayCodeMap();

    // 获取生化项目信息
    auto chAssayMap = CommonInformationManager::GetInstance()->GetChAssayIndexCodeMaps();
    // 依次添加项目名称到容器中
    for (const auto& chAssay : chAssayMap)
    {
        std::map<int, std::shared_ptr<tf::GeneralAssayInfo>>::iterator aIter = assayMap.find(chAssay.second->assayCode);
        if (chAssay.second->assayCode != ch::tf::g_ch_constants.ASSAY_CODE_SIND && aIter != assayMap.end() && aIter->second != nullptr)
        {
            projectNames.append(QString::fromStdString(aIter->second->assayName));
        }
    }

    // 确认是否存在ise设备，暂时以ise1005为标记
    const auto& iseDevs = CommonInformationManager::GetInstance()->GetDeviceFromType({ tf::DeviceType::type::DEVICE_TYPE_ISE1005 });
    if (!iseDevs.empty())
    {
        auto iseAssayMap = CommonInformationManager::GetInstance()->GetIseAssayIndexCodeMaps();
        for (const auto& iseAssay: iseAssayMap)
        {
            auto iter = assayMap.find(iseAssay.second->assayCode);
            if (iter != assayMap.end() && iter->second != nullptr)
            {
                projectNames.append(QString::fromStdString(iter->second->assayName));
            }
        }
        
    }

    // 将名称进行排序
    projectNames.sort();

    // 重新装载
    ui->comboBox_projectName->clear();
    ui->comboBox_projectName->addItems(projectNames);

    //初始化为空
    ReInitSerachCombox();
}
