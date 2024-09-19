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
/// @file     QSampleAssayModel.h
/// @brief    样本模块module
///
/// @author   5774/WuHongTao
/// @date     2022年7月20日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年7月20日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QQcSampleModel.h"
#include <QTableView>
#include <QDateTime>
#include <QFontMetrics>

#include "shared/uicommon.h"
#include "shared/uidef.h"
#include "shared/msgiddef.h"
#include "shared/messagebus.h"
#include "shared/CommonInformationManager.h"
#include "shared/ThriftEnumTransform.h"
#include "shared/CReadOnlyDelegate.h"
#include "shared/datetimefmttool.h"
#include "shared/DataManagerQc.h"

#include "thrift/DcsControlProxy.h"
#include "src/common/defs.h"
#include "src/common/common.h"
#include "src/common/TimeUtil.h"
#include "src/common/Mlog/mlog.h"

#define INVALIDCODE -1
#define DEFAULTASSAY 20
#define ROW_EMPTY_SETTINGS 16 // 质控品列表末尾附加空行数
#define ROW_DEFAULT_AMOUNT 999 // 默认显示的质控品数量

#define DEFAULT_PRICISION 0.01 // sd、cv、td使用默认精度，不使用项目对应的精度（BUG0023875）


QQcSampleModel::QQcSampleModel() : m_bMaskExpiredTime(false)
{
    Init();
}

QQcSampleModel::~QQcSampleModel()
{
}

///
/// @brief 设置view
///
/// @param[in]  view  view句柄
///
/// @par History:
/// @li 5774/WuHongTao，2022年8月18日，新建函数
///
void QQcSampleModel::SetView(QTableView* view)
{
    m_tableView = view;
}

///
/// @brief 根据索引获取对应的质控对象
///
/// @param[in]  index  索引
///
/// @return 质控对象
///
/// @par History:
/// @li 5774/WuHongTao，2022年8月18日，新建函数
///
boost::optional<::tf::QcDoc> QQcSampleModel::GetDocByIndex(const QModelIndex& qcIndex)
{
    QVariant cellValue = m_tableView->model()->index(qcIndex.row(), 0).data();
    if (!cellValue.isValid())
    {
        return boost::none;
    }
    int seq = cellValue.toString().replace("QC", "").toInt();

    for (const std::shared_ptr<QcRowData>& rItem : m_qcDocVec)
    {
        if (rItem && rItem->qcItem && rItem->qcItem.value().index == seq)
        {
            return rItem->qcItem;
        }
    }

    return boost::none;
}

int QQcSampleModel::GenerateQcIndex(const QModelIndex& qcIndex)
{
    // 使用序号列单元格中的内容作
    QVariant cellValue = m_tableView->model()->index(qcIndex.row(), 0).data();
    if (cellValue.isValid())
    {
        return cellValue.toString().replace("QC", "").toInt();
    }

    // 条件筛选时，序号列被值为空，进行遍历查找递增序号
    if (!m_qcFilter.IsEmptyCondition())
    {
        for (int i = 1; i < ROW_DEFAULT_AMOUNT; ++i)
        {
            if (m_allQcDocIndex.find(i) == m_allQcDocIndex.end())
            {
                return i;
            }
        }
    }

    // 使用额外的序号
    int rowIndex = qcIndex.row();
    rowIndex++;
    while (m_qcDocIndexSet.find(rowIndex) != m_qcDocIndexSet.end())
    {
        rowIndex++;
    }
    return rowIndex;
}

///
/// @brief 获取单例对象
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月30日，新建函数
///
QQcSampleModel& QQcSampleModel::Instance()
{
    static QQcSampleModel model;
    return model;
}

///
/// @brief
///     获取单元格属性(可编辑、可选择)
///
/// @param[in]    index      当前单元格索引
///
/// @return       QVariant   包装的数据
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月30日，新建函数
///
Qt::ItemFlags QQcSampleModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

///
/// @brief 删除多行
///
/// @param[in]  indexs  行数
/// @param[out]  failedDelete  删除失败的行序列集合
///
/// @return true代表成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年8月23日，新建函数
///
bool QQcSampleModel::removeQcRow(const std::set<int>& qcIndex, std::vector<int>& failedDelete)
{
    ULOG(LOG_INFO, "%s, %d", __FUNCTION__, qcIndex.size());
    bool alreadyDeleted = false;

    // 获取删除主键
    std::vector<int64_t> ids;
    std::map<int64_t, std::shared_ptr<QcRowData>> deletItem;
    for (auto& rItem : m_qcDocVec)
    {
        if (rItem && rItem->qcItem && qcIndex.find(rItem->qcItem->index) != qcIndex.end())
        {
            ids.push_back(rItem->qcItem.value().id);
            deletItem.insert(std::pair<int64_t, std::shared_ptr<QcRowData>>(rItem->qcItem.value().id, rItem));
        }
    }

    // 执行删除
    std::vector<int64_t> failedDocID;
    ::tf::QcDocQueryCond cdgq;
    cdgq.__set_ids(ids);
    if (!DcsControlProxy::GetInstance()->DeleteQcDoc(cdgq, failedDocID))
    {
        ULOG(LOG_WARN, "Failed to delete qcdoc.");
        return false;
    }
    else
    {
        std::set<int64_t> setFailedDocId;
        for (int64_t si : failedDocID)
        {
            setFailedDocId.insert(si);
        }

        std::map<int64_t, std::shared_ptr<QcRowData>>::iterator it = deletItem.begin();
        for (; it != deletItem.end(); ++it)
        {
            if (setFailedDocId.find(it->first) != setFailedDocId.end())
            {
                failedDelete.push_back(it->second->rowSequence);
                continue;
            }
            else
            {
                // 更新质控品对应的联合设置
                CommonInformationManager::GetInstance()->DeleteUnionQcSettings(it->first);

                // 删除Model数据
                UpdateDeletedQcdoc(it->second->rowSequence);
            }
            it->second->qcItem = boost::none;
        }
    }

    return true;
}

///
/// @brief 更新模式里面的数据
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月30日，新建函数
///
bool QQcSampleModel::ReloadQcdoc()
{
    ULOG(LOG_INFO, __FUNCTION__);

    std::vector<std::shared_ptr<tf::QcDoc>> vecQcDoc;
    DataManagerQc::GetInstance()->GetQcDocs(vecQcDoc);

    beginResetModel();

    m_qcDocVec.clear();
    m_qcDocIndexSet.clear();
    m_allQcDocIndex.clear();
    m_allQcDocNames.clear();
    m_filtedRowIndex.clear();

    if (m_qcFilter.IsEmptyCondition())
    {
        for (const auto& qcDoc : vecQcDoc)
        {
            if (qcDoc == nullptr)
            {
                continue;
            }
            std::shared_ptr<QcRowData> ptrData = std::make_shared<QcRowData>(boost::optional<tf::QcDoc>(*qcDoc));
            m_qcDocVec.push_back(ptrData);
            m_qcDocIndexSet[qcDoc->index] = ptrData;

            m_allQcDocIndex.insert(qcDoc->index);
            m_allQcDocNames.push_back(qcDoc->name);
        }


#if 0 // bugfix[0022482]不允许支持自增，所以弃用超过限定最大行数后可自增的支持
        int maxRow = m_qcDocIndexSet.size() > 0 ? m_qcDocIndexSet.rbegin()->first + ROW_EMPTY_SETTINGS : ROW_EMPTY_SETTINGS;
        maxRow = maxRow > ROW_DEFAULT_AMOUNT ? maxRow : ROW_DEFAULT_AMOUNT; // 最低已默认数量显示
#else

        int maxRow = ROW_DEFAULT_AMOUNT;
#endif
        for (int i = 1; i <= maxRow; ++i)
        {
            if (m_qcDocIndexSet.find(i) == m_qcDocIndexSet.end())
            {
                std::shared_ptr<QcRowData> ptrData = std::make_shared<QcRowData>(QcRowData(i));
                m_qcDocIndexSet[i] = ptrData;
                m_qcDocVec.push_back(ptrData);
            }
        }
    }
    else
    {
        int iRealIdx = 1;
        for (const auto& qcDoc : vecQcDoc)
        {
            if (qcDoc == nullptr)
            {
                continue;
            }
            // 对质控品按过滤设置进行过滤，注册类型为空代表不对注册类型进行过滤
            if (m_qcFilter.IsFitCondition(qcDoc->name, qcDoc->registType))
            {
                std::shared_ptr<QcRowData> ptrData = std::make_shared<QcRowData>(boost::optional<tf::QcDoc>(*qcDoc));
                m_qcDocVec.push_back(ptrData);
                m_qcDocIndexSet[qcDoc->index] = ptrData;
                m_allQcDocNames.push_back(qcDoc->name);

                m_filtedRowIndex.insert(std::pair<int, int>(iRealIdx, qcDoc->index));
                iRealIdx++;
            }
        }
    }

    endResetModel();

    // 发送信息
    emit displayDataChanged();

    return true;
}

bool QQcSampleModel::Update(const std::vector<int64_t>& qcDocIds)
{
    ULOG(LOG_INFO, "%s, size:%d.", __FUNCTION__, qcDocIds.size());

    ::tf::QcDocQueryResp _return;
    ::tf::QcDocQueryCond cdqc;
    cdqc.__set_ids(qcDocIds);
    if (!DcsControlProxy::GetInstance()->QueryQcDoc(_return, cdqc)
        || _return.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "Failed to queryqcdoc.");
        return false;
    }

    beginResetModel();
    for (const auto& qcDoc : _return.lstQcDocs)
    {
        // 对质控品按过滤设置进行过滤，注册类型为空代表不对注册类型进行过滤
        if (m_qcFilter.IsFitCondition(qcDoc.name, qcDoc.registType))
        {
            std::map<int, std::shared_ptr<QcRowData>>::iterator it = m_qcDocIndexSet.find(qcDoc.index);
            if (it != m_qcDocIndexSet.end())
            {
                it->second->qcItem = qcDoc;
            }
        }
        // 更新筛选名字
        if (std::find(m_allQcDocNames.begin(), m_allQcDocNames.end(), qcDoc.name) == m_allQcDocNames.end())
        {
            m_allQcDocNames.push_back(qcDoc.name);
        }
    }
    endResetModel();

    // 发送信息
    emit displayDataChanged();

    return true;
}

void QQcSampleModel::UpdateDeletedQcdoc(int deletedQcIndex)
{
    beginResetModel();

    // 删除索引数据
    std::map<int, std::shared_ptr<QcRowData>>::iterator it = m_qcDocIndexSet.find(deletedQcIndex);
    if (it != m_qcDocIndexSet.end() && it->second != nullptr && it->second->qcItem.has_value())
    {
        // 删除names缓存
        std::string strName = it->second->qcItem->name;
        for (std::vector<std::string>::iterator nameIter = m_allQcDocNames.begin(); nameIter != m_allQcDocNames.end(); ++nameIter)
        {
            if (*nameIter == strName)
            {
                m_allQcDocNames.erase(nameIter);
                break;
            }
        }
        if (!m_qcFilter.IsEmptyCondition())
        {
            // 删除set索引
            it->second->qcItem.reset();
            m_qcDocIndexSet.erase(it);
        }
        else
        {
            it->second->qcItem.reset();
        }
    }

    // 删除数据
    for (std::vector<std::shared_ptr<QcRowData>>::iterator vIter = m_qcDocVec.begin(); vIter != m_qcDocVec.end(); ++vIter)
    {
        std::shared_ptr<QcRowData> pTemp = *vIter;
        if (pTemp != nullptr && pTemp->rowSequence == deletedQcIndex)
        {
            (*vIter)->qcItem.reset();

            // 删除顺序记录
            if (!m_qcFilter.IsEmptyCondition())
            {
                m_qcDocVec.erase(vIter);
            }

            break;
        }
    }

    // 重新组织真实行号-index的映射
    int iRealIdx = 1;
    m_filtedRowIndex.clear();
    for (std::vector<std::shared_ptr<QcRowData>>::iterator vIter = m_qcDocVec.begin(); vIter != m_qcDocVec.end(); ++vIter)
    {
        if (!(*vIter)->qcItem.has_value())
        {
            continue;
        }
        m_filtedRowIndex.insert(std::pair<int, int>(iRealIdx, (*vIter)->qcItem->index));
        iRealIdx++;
    }

    endResetModel();

    // 发送信息
    emit displayDataChanged();
}

int QQcSampleModel::rowCount(const QModelIndex &parent) const
{
    return m_qcDocVec.size();
}

int QQcSampleModel::columnCount(const QModelIndex &parent) const
{
    return m_heads.size();
}

int QQcSampleModel::FindRowByQcIndex(int qcIndex)
{
    for (int i = 0; i < m_qcDocVec.size(); ++i)
    {
        QVariant curVari = m_tableView->model()->index(i, 0).data();
        if (!curVari.isValid())
        {
            continue;
        }

        // 序号列单元格中的序号与入参qcIndex是否相等的判断
        if (curVari.toString().replace("QC", "").toInt() == qcIndex)
        {
            return i;
        }
    }

    return -1;
}

int QQcSampleModel::FindRowByIndexName(QString indexName)
{
    for (int i = 0; i < m_qcDocVec.size(); ++i)
    {
        std::shared_ptr<QcRowData> &curdt = m_qcDocVec[i];
        if (curdt == nullptr || !curdt->qcItem.has_value())
        {
            continue;
        }
        int seq = indexName.replace("QC", "").toInt();
        if (seq == curdt->qcItem->index)
        {
            return i;
        }
    }

    return -1;
}

///
/// @brief 初始化质控
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年8月17日，新建函数
///
void QQcSampleModel::Init()
{
    ULOG(LOG_INFO, __FUNCTION__);

    // 初始化试管类型位图表
    m_tubePixMap[tf::TubeType::TUBE_TYPE_MICRO] = QPixmap(QString(":/Leonis/resource/image/icon-micro-cup-hv.png"));
    m_tubePixMap[tf::TubeType::TUBE_TYPE_NORMAL] = QPixmap(QString(":/Leonis/resource/image/icon-normal-cup-hv.png"));
    m_tubePixMap[tf::TubeType::TUBE_TYPE_ORIGIN] = QPixmap(QString(":/Leonis/resource/image/icon-ori-hv.png"));
    m_tubePixMap[tf::TubeType::TUBE_TYPE_STORE] = QPixmap(QString(":/Leonis/resource/image/icon-store-hv.png"));

    m_focusLine = INVALIDCODE;
    m_heads.clear();
    m_heads << tr("文档号") << tr("质控品编号") << tr("质控品名称") << tr("质控品简称") << tr("质控品类型") << \
        tr("质控品水平") << tr("质控品批号") << tr("失效日期") << tr("登记方式") << tr("位置/样本管");

    ReloadQcdoc();
    m_tableView = nullptr;
}

///
/// @brief 获取校准品的位置和杯类型
///
/// @param[in]  doc  校准品信息
///
/// @return 位置和类型字符串
///
/// @par History:
/// @li 5774/WuHongTao，2022年8月23日，新建函数
///
QString QQcSampleModel::GetPosAndCupType(const ::tf::QcDoc& doc) const
{
    //ULOG(LOG_INFO, __FUNCTION__); // called by data, too frequency.

    // rack + postion
    QString strPostion = doc.rack.empty() ? tr("请设置位置") : (QString::fromStdString(doc.rack) + "-" + QString::number(doc.pos));

    return strPostion;
}

QVariant QQcSampleModel::data(const QModelIndex& index, int role) const
{
    // 范围之外，退出
    int rowIndex = index.row() + 1;

    // 如果存在过滤，转换为数据所在的索引
    if (!m_qcFilter.IsEmptyCondition())
    {
        std::map<int, int>::const_iterator idxIt = m_filtedRowIndex.find(rowIndex);
        if (idxIt != m_filtedRowIndex.end())
        {
            rowIndex = idxIt->second;
        }
    }

    std::map<int, std::shared_ptr<QcRowData>>::const_iterator qcIter = m_qcDocIndexSet.find(rowIndex);
    if (qcIter == m_qcDocIndexSet.end())
    {
        return QVariant();
    }
    if (qcIter->second && qcIter->second->qcItem == boost::none)
    {
        if (role == Qt::DisplayRole && (COLSAMPLE)index.column() == COLSAMPLE::SEQNO)
        {
            if (!m_qcFilter.IsEmptyCondition())
            {
                return QVariant();
            }
            return QString("QC%1").arg(qcIter->second->rowSequence, 3, 10, QChar('0'));
        }
        if (role == Qt::TextAlignmentRole && (COLSAMPLE)index.column() == COLSAMPLE::SEQNO)
        {
            return QVariant::fromValue<int>(Qt::AlignHCenter | Qt::AlignVCenter);
        }
        return QVariant();
    }

    tf::QcDoc qcData = qcIter->second->qcItem.value();
    switch (role)
    {
        // 获取数据
    case Qt::DisplayRole:
    {
        switch ((COLSAMPLE)index.column())
        {
        case COLSAMPLE::SEQNO:
            return QString("QC%1").arg(qcData.index, 3, 10, QChar('0'));
        case COLSAMPLE::SERIALNUMBER:
            return QString::fromStdString(qcData.sn);
        case COLSAMPLE::NAME:
            return QString::fromStdString(qcData.name);
        case COLSAMPLE::ABBREVIATION:
            return QString::fromStdString(qcData.shortName);
        case COLSAMPLE::TYPE:
            return ThriftEnumTrans::GetSourceTypeName(qcData.sampleSourceType);
        case COLSAMPLE::LEVEL:
            return QString::number(qcData.level);
        case COLSAMPLE::LOT:
            return QString::fromStdString(qcData.lot);
        case COLSAMPLE::EXPIRED:
        {
            if (qcData.expireTime.empty() || m_bMaskExpiredTime)
            {
                return QVariant();
            }
            return DateTimeFmtTool::GetInstance()->GetCfgFmtDateStr(QString::fromStdString(qcData.expireTime));
        }
        case COLSAMPLE::REGTYPE:
            return ThriftEnumTrans::GetRegistType(qcData.registType);
        case COLSAMPLE::POS_TUBE:
            return GetPosAndCupType(qcData);
        default:
            return QVariant();
        }
        break;
    }
    // 对齐方式
    case Qt::TextAlignmentRole:
        return QVariant::fromValue<int>(Qt::AlignHCenter | Qt::AlignVCenter);

        // 使用的CReadOnlyDelegate中的paint颜色绘制
    case (Qt::UserRole + DELEGATE_DATATIME_EXPIRE):
        if ((COLSAMPLE)index.column() == COLSAMPLE::EXPIRED)
        {
            return QString::fromStdString(qcData.expireTime);
        }
        break;
        // 设置图标
    case (Qt::UserRole + DELEGATE_POSITION_OFFSET_POS):
    {
        if ((COLSAMPLE)index.column() == COLSAMPLE::EXPIRED)
        {
            QString strExpireTime = QString::fromStdString(qcData.expireTime);
            QDateTime dtExprity = QDateTime::fromString(strExpireTime, UI_DATE_TIME_FORMAT);
            return dtExprity < QDateTime::currentDateTime() ? QColor(UI_REAGENT_WARNFONT) : QVariant();
        }
        else
        {
            if (!qcData.rack.empty())
            {
                std::map<tf::TubeType::type, QPixmap>::const_iterator tpIter = m_tubePixMap.find(qcData.tubeType);
                if (tpIter != m_tubePixMap.end())
                {
                    return tpIter->second;
                }
            }
        }
    }
    break;
    // 文本提示
    case Qt::ToolTipRole:
    {
        QVariant qVri = index.data();
        if (qVri.isValid())
        {
            QFontMetrics fm(m_tableView->fontMetrics());
            int iCharc = fm.width(qVri.toString());
            int iColum = m_tableView->columnWidth(index.column());
            if (iCharc + 19 > iColum) // 19：字体外的边框冗余
            {
                return qVri.toString();
            }
        }
    }
    break;
    // 此处设置背景颜色不生效(优先使用qss了)
    case Qt::BackgroundRole:
        break;
    default:
        break;
    }

    return QVariant();
}

///
/// @brief
///     重写设置数据函数
///
/// @param[in]  index   索引
/// @param[in]  value   值
/// @param[in]  role    角色
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2023年9月20日，新建函数
///
bool QQcSampleModel::setData(const QModelIndex &index, const QVariant &value, int role /*= Qt::EditRole*/)
{
    if (role == Qt::DisplayRole && index.isValid() && index.column() == (int)COLSAMPLE::EXPIRED)
    {
        QString text = value.toString();
        m_bMaskExpiredTime = text.isEmpty();
    }

    return QAbstractTableModel::setData(index, value, role);
}

QVariant QQcSampleModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {
            return m_heads.at(section);
        }

        if (role == Qt::ForegroundRole && (m_heads.size() - 1) == section)
        {
            QBrush brush(UI_TEXT_COLOR_HEADER_MARK);
            return brush;
        }
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

///
/// @bref
///		通过Index获取打印数据
///
/// @param[in] Index 输入的Index
///
/// @par History:
/// @li 6889/ChenWei, 2023年12月14日，新建函数
///
QString QQcSampleModel::GetPrintData(const QModelIndex &Index)
{
    // 范围之外，退出
    std::map<int, std::shared_ptr<QcRowData>>::const_iterator qcIter = m_qcDocIndexSet.find(Index.row() + 1);
    if (qcIter == m_qcDocIndexSet.end())
    {
        return "";
    }
    if (qcIter->second && qcIter->second->qcItem == boost::none)
    {
        return "";
    }

    tf::QcDoc qcData = qcIter->second->qcItem.value();
    switch ((COLSAMPLE)Index.column())
    {
    case COLSAMPLE::SEQNO:
        return QString("QC%1").arg(qcData.index, 3, 10, QChar('0'));
    case COLSAMPLE::SERIALNUMBER:
        return QString::fromStdString(qcData.sn);
    case COLSAMPLE::NAME:
        return QString::fromStdString(qcData.name);
    case COLSAMPLE::ABBREVIATION:
        return QString::fromStdString(qcData.shortName);
    case COLSAMPLE::TYPE:
        return ThriftEnumTrans::GetSourceTypeName(qcData.sampleSourceType);
    case COLSAMPLE::LEVEL:
        return QString::number(qcData.level);
    case COLSAMPLE::LOT:
        return QString::fromStdString(qcData.lot);
    case COLSAMPLE::EXPIRED:
    {
        if (qcData.expireTime.empty() || m_bMaskExpiredTime)
        {
            return QString();
        }
        tm expritime = ConvertSimpleString2Tm(qcData.expireTime);
        char tempstr[128] = {};
        sprintf(tempstr, "%d/%d/%d", expritime.tm_year + 1900, expritime.tm_mon + 1, expritime.tm_mday);

        return QString::fromStdString(tempstr);
    }
    case COLSAMPLE::REGTYPE:
        return ThriftEnumTrans::GetRegistType(qcData.registType);
    case COLSAMPLE::POS_TUBE:
    {
        if (!qcData.rack.empty())
        {
            QString strPostion = (QString::fromStdString(qcData.rack) + "-" + QString::number(qcData.pos)) + " ";
            strPostion += ConvertTfEnumToQString(qcData.tubeType);
            return strPostion;
        }

        return QString("*");
    }

    default:
        return QString();
    }
}


QQcAssayModel& QQcAssayModel::Instance()
{
    static QQcAssayModel module;
    return module;
}

bool QQcAssayModel::SetData(const std::vector<::tf::QcComposition>& data)
{
    ULOG(LOG_INFO, "%s, data size is:%d.", __FUNCTION__, data.size());

    std::shared_ptr<CommonInformationManager> insCm = CommonInformationManager::GetInstance();
    beginResetModel();

    // 过滤掉已被删除的项目，避免空行
    m_compositionCv.clear();
    m_assayData.clear();
    std::vector<::tf::QcComposition> validCompoistions;
    AssayIndexCodeMaps generalAssays = insCm->GetGeneralAssayCodeMap();
    for (const ::tf::QcComposition& qcCom : data)
    {
        if (generalAssays.find(qcCom.assayCode) != generalAssays.end())
        {
            double dRate = insCm->GetUnitChangeRate(qcCom.assayCode);
            m_assayData.push_back(qcCom);

            // 转换成当前单位倍率
            ::tf::QcComposition &tmp = m_assayData[m_assayData.size() - 1];
            tmp.sd *= dRate;
            tmp.targetValue *= dRate;
        }
    }

    // 按assayCode从小到大排序
    std::sort(m_assayData.begin(), m_assayData.end(), [](
        const ::tf::QcComposition& a, const ::tf::QcComposition& b) {
        return a.assayCode < b.assayCode;
    });

    endResetModel();

    return true;
}

int QQcAssayModel::rowCount(const QModelIndex &parent) const
{
    //// 越界判断
    //auto rows = m_assayData.size();
    //if (rows < DEFAULTASSAY)
    //{
    //   return DEFAULTASSAY;
    //}

    return m_assayData.size();
}

int QQcAssayModel::columnCount(const QModelIndex &parent) const
{
    return m_heads.size();
}

///
/// @brief 初始化代码
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年8月18日，新建函数
///
bool QQcAssayModel::Init()
{
    return true;
}

Qt::ItemFlags QQcAssayModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant QQcAssayModel::data(const QModelIndex &index, int role) const
{
    if (index.row() >= m_assayData.size() || index.row() < 0)
    {
        return QVariant();
    }

    const tf::QcComposition& assayData = m_assayData.at(index.row());
    auto spCompostion = CommonInformationManager::GetInstance()->GetAssayInfo(assayData.assayCode);
    if (spCompostion == nullptr)
    {
        return QVariant();
    }

    // 获取数据
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (COLASSAY(index.column()))
        {
        case COLASSAY::NAME:
            return QString::fromStdString(spCompostion->assayName);
        case COLASSAY::TARGETVALUE:
            if (abs(assayData.targetValue) < PRECESION_VALUE)
            {
                std::map<int, double>::const_iterator cvIter = m_compositionCv.find(index.row());
                // 根据CV和SD 求 Target
                return abs(assayData.sd) < PRECESION_VALUE || cvIter == m_compositionCv.end() ? "" : \
                    QString::number((assayData.sd * 100) / cvIter->second, 'f', spCompostion->decimalPlace);
            }
            else
            {
                return QString::number(assayData.targetValue, 'f', spCompostion->decimalPlace);
            }
        case COLASSAY::STANDARD:
            if (abs(assayData.sd) < PRECESION_VALUE)
            {
                std::map<int, double>::const_iterator cvIter = m_compositionCv.find(index.row());
                // 根据CV和Target 求SD
                return abs(assayData.targetValue) < PRECESION_VALUE || cvIter == m_compositionCv.end() ? "" : \
                    QString::number((cvIter->second  * assayData.targetValue) / 100, 'f', spCompostion->decimalPlace);
            }
            else
            {
                return QString::number(assayData.sd, 'f', spCompostion->decimalPlace);
            }
        case COLASSAY::CV:
        {
            std::map<int, double>::const_iterator cvIt = m_compositionCv.find(index.row());
            if (cvIt == m_compositionCv.end())
            {
                // 如果CV为空且存在SD和Target，则显示SD和Target计算的CV
                return abs(assayData.targetValue) < PRECESION_VALUE || abs(assayData.sd) < PRECESION_VALUE ? "" : \
                    QString::number(double(assayData.sd) / double(assayData.targetValue) * 100, 'f', spCompostion->decimalPlace);
            }
            else
            {
                if (abs(cvIt->second < PRECESION_VALUE))
                {
                    return "";
                }
                return QString::number(cvIt->second, 'f', spCompostion->decimalPlace);
            }
        }
        case COLASSAY::UNIT:
            for (const auto& unit : spCompostion->units)
            {
                if (unit.isCurrent)
                {
                    return QString::fromStdString(unit.name);
                }
            }

            return "";
        default:
            return QVariant();
            break;
        }
    }
    // 对齐方式
    else if (role == Qt::TextAlignmentRole) {
        return QVariant::fromValue<int>(Qt::AlignHCenter | Qt::AlignVCenter);
    }
    else
    {
        return QVariant();
    }
}

QVariant QQcAssayModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {
            return m_heads.at(section);
        }

        if (role == Qt::ForegroundRole && (int(COLASSAY::TARGETVALUE) == section ||
            int(COLASSAY::STANDARD) == section ||
            int(COLASSAY::CV) == section))
        {
            return QVariant();
        }
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

QQcAssayModel::QQcAssayModel()
{
    m_heads << tr("项目名称") << tr("靶值") << tr("标准差") << tr("CV%") << tr("单位");
    m_preEditCol = COLASSAY::NAME;
}

bool QQcAssayEditModel::SetData(const std::vector<::tf::QcComposition>& data)
{
    ULOG(LOG_INFO, "%s, data size is:%d", __FUNCTION__, data.size());

    std::shared_ptr<CommonInformationManager> insCm = CommonInformationManager::GetInstance();

    m_compositionCv.clear();
    beginResetModel();

    // Clean data.
    for (auto& compostion : m_assayData)
    {
        compostion.__set_targetValue(0);
        compostion.__set_sd(0);
    }

    // Assign
    std::vector<int> notExistAssay;
    for (const auto& dataCompostion : data)
    {
        bool existFlag = false;
        for (auto& compostion : m_assayData)
        {
            if (dataCompostion.assayCode == compostion.assayCode)
            {
                existFlag = true;
                compostion = dataCompostion;

                // 换算成现有单位的值用于显示
                double dRate = insCm->GetUnitChangeRate(compostion.assayCode);
                compostion.sd *= dRate;
                compostion.targetValue *= dRate;
            }
        }

        // 没有匹配到已有试剂
        if (!existFlag)
        {
            notExistAssay.push_back(dataCompostion.assayCode);
        }
    }
    endResetModel();

    // Print log.
    if (notExistAssay.size() > 0)
    {
        ULOG(LOG_WARN, "Not match assay code %s.", GetContainerJoinString(notExistAssay, "/"));
    }

    return true;
}

bool QQcAssayEditModel::AssignTargetValue(tf::QcComposition& assayData, const QVariant& sTargetValue, int iRow)
{
    // 设置targetValue信息
    double dTv = sTargetValue.toDouble();
    if (dTv < DEFAULT_PRICISION)
    {
        if (sTargetValue.toString().size() == 0)
        {
            assayData.__set_targetValue(0); // 小于0 则清零
            return true;
        }
        return false;
    }

    // 如果存在SD且上回不是刚填的CV则重新计算CV
    std::map<int, double>::iterator cvIter = m_compositionCv.find(iRow);
    if (cvIter != m_compositionCv.end())
    {
        if (abs(assayData.sd) > PRECESION_VALUE)
        {
            // 计算CV
            if (m_preEditCol != QQcAssayModel::COLASSAY::CV && m_preEditCol != QQcAssayModel::COLASSAY::TARGETVALUE)
            {
                double dCv = double(assayData.sd) / dTv * 100;
                if (dCv < DEFAULT_PRICISION)
                {
                    return false;
                }
                cvIter->second = dCv;
            }
            // 计算SD
            else
            {
                assayData.__set_sd(dTv * cvIter->second / 100);
            }
        }
        // 计算SD
        else
        {
            assayData.__set_sd(dTv * cvIter->second / 100);
        }
    }
    assayData.__set_targetValue(sTargetValue.toDouble());
    return true;
}

bool QQcAssayEditModel::AssignStandardValue(tf::QcComposition& assayData, const QVariant& vStandardValue, int iRow)
{
    // 设置sd信息
    double dSd = vStandardValue.toDouble();
    if (dSd < DEFAULT_PRICISION)
    {
        if (vStandardValue.toString().size() == 0)
        {
            assayData.__set_sd(0);
            return true;
        }
        return false;
    }

    std::map<int, double>::iterator cvIter = m_compositionCv.find(iRow);
    if (cvIter != m_compositionCv.end())
    {
        // 如果存在Target则重新计算CV
        if (abs(assayData.targetValue) > PRECESION_VALUE)
        {
            if (m_preEditCol != QQcAssayModel::COLASSAY::CV && m_preEditCol != QQcAssayModel::COLASSAY::STANDARD)
            {
                double dCv = dSd / double(assayData.targetValue) * 100;
                if (dCv < DEFAULT_PRICISION)
                {
                    return false;
                }
                cvIter->second = dCv;
            }
            else
            {
                assayData.__set_targetValue((dSd / cvIter->second) * 100);
            }
        }
        // 计算target
        else
        {
            assayData.__set_targetValue((dSd / cvIter->second) * 100);
        }
    }

    assayData.__set_sd(dSd);
    return true;
}

bool QQcAssayEditModel::AssignCvValue(tf::QcComposition& assayData, const QVariant& vCvValue, int iRow)
{
    double dCv = vCvValue.toDouble();
    // 如果CV清空，则sd、target也清空
    if (dCv < DEFAULT_PRICISION)
    {
        if (vCvValue.toString().size() == 0)
        {
            std::map<int, double>::iterator cvIter = m_compositionCv.find(iRow);
            if (cvIter != m_compositionCv.end())
            {
                m_compositionCv.erase(cvIter);
            }
            assayData.__set_sd(0);
            assayData.__set_targetValue(0);
            return true;
        }
        return false;
    }

    // 记录不为CV的操作列
    static QQcAssayModel::COLASSAY preDiff = m_preEditCol;
    if (m_preEditCol != QQcAssayModel::COLASSAY::CV)
    {
        preDiff = m_preEditCol;
    }

    bool existTgV = (abs(assayData.targetValue) > PRECESION_VALUE);
    bool existSdV = (abs(assayData.sd) > PRECESION_VALUE);
    if (existTgV && existSdV) // 
    {
        if (preDiff == QQcAssayModel::COLASSAY::STANDARD)
        {
            double calTarget = (assayData.sd / vCvValue.toDouble()) * 100;
            assayData.__set_targetValue(calTarget);
        }
        else
        {
            double sd = assayData.targetValue * vCvValue.toDouble() / 100;
            assayData.__set_sd(sd);
        }
    }
    // 如果靶值有有值，则计算sd的值
    if (existTgV)
    {
        double sd = assayData.targetValue * dCv / 100;
        if (sd < DEFAULT_PRICISION)
        {
            return false;
        }
        assayData.__set_sd(sd);
    }
    // 如果sd有值且靶值为空，则自动填充靶值
    else if (existSdV)
    {
        double calTarget = (assayData.sd / dCv) * 100;
        if (calTarget < DEFAULT_PRICISION)
        {
            return false;
        }
        assayData.__set_targetValue(calTarget);
    }
    m_compositionCv[iRow] = dCv;
    return true;
}

///
/// @brief
///     设置单元格数据 供视图调用
///
/// @param[in]    index      当前单元格索引
/// @param[in]    value      单元格的值
/// @param[in]    role       被修改的角色类型
///
/// @return       bool       数据处理成功返回true
///
/// @par History:
/// @li 5774/WuHongTao，2022年8月19日，新建函数
///
bool QQcAssayEditModel::setData(const QModelIndex &index, const QVariant &value, int role /*= Qt::EditRole*/)
{
    if (role != Qt::EditRole)
    {
        return false;
    }

    // 获取对应行的数据
    int row = index.row();
    int column = index.column();
    if (row >= m_assayData.size())
    {
        return false;
    }
    tf::QcComposition& assayData = m_assayData.at(row);

    // 进行赋值
    bool ret = true;
    switch (COLASSAY(column))
    {
    case QQcAssayModel::COLASSAY::NAME:
        break;
    case QQcAssayModel::COLASSAY::TARGETVALUE:
    {
        ret = AssignTargetValue(assayData, value, row);
        m_preEditCol = QQcAssayModel::COLASSAY::TARGETVALUE;
        break;
    }
    case QQcAssayModel::COLASSAY::STANDARD:
    {
        ret = AssignStandardValue(assayData, value, row);
        m_preEditCol = QQcAssayModel::COLASSAY::STANDARD;
        break;
    }
    case QQcAssayModel::COLASSAY::CV:
    {
        ret = AssignCvValue(assayData, value, row);
        m_preEditCol = QQcAssayModel::COLASSAY::CV;
        break;
    }
    break;
    case QQcAssayModel::COLASSAY::UNIT:
        break;
    default:
        break;
    }

    return ret;
}

///
/// @brief 获取选择的项目成分
///
///
/// @return 项目成分列表
///
/// @par History:
/// @li 5774/WuHongTao，2022年8月22日，新建函数
///
void QQcAssayEditModel::GetCompostion(std::vector<::tf::QcComposition>& outComp, std::vector<std::string>& notComplate)
{
    auto funcIsExsitCv = [this](int rowIdx) ->bool {
        std::map<int, double>::iterator it = m_compositionCv.find(rowIdx);
        if (it == m_compositionCv.end())
        {
            return false;
        }
        return it->second > 0;
    };

    std::shared_ptr<CommonInformationManager> insCom = CommonInformationManager::GetInstance();

    // 收集设置不完整的项目，用于提示
    int rowIdx = 0;
    for (auto& com : m_assayData)
    {
        if (com.targetValue > 0 && com.sd > 0)
        {
            if (m_assayType == QmCh)
            {
                bool isIse = com.assayCode >= ::ise::tf::g_ise_constants.ASSAY_CODE_ISE && com.assayCode <= ::ise::tf::g_ise_constants.ASSAY_CODE_CL;
                com.__set_deviceType(isIse ? tf::DeviceType::DEVICE_TYPE_ISE1005 : tf::DeviceType::DEVICE_TYPE_C1000);
            }
            else
            {
                com.__set_deviceType(tf::DeviceType::DEVICE_TYPE_I6000);
            }
            double dRate = insCom->GetUnitChangeRate(com.assayCode);
            outComp.push_back(com);

            // 转换成原始倍率存储
            ::tf::QcComposition &tmp = outComp[outComp.size() - 1];
            tmp.sd /= dRate;
            tmp.targetValue /= dRate;
        }
        else if (com.targetValue > 0 || com.sd > 0 || funcIsExsitCv(rowIdx))
        {
            auto pAssay = insCom->GetAssayInfo(com.assayCode);
            notComplate.push_back(pAssay == nullptr ? std::to_string(com.assayCode) : pAssay->assayName);
        }

        rowIdx++;
    }
}

///
/// @brief 初始化代码
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年8月18日，新建函数
///
bool QQcAssayEditModel::Init(const std::vector<int32_t>& assayCode)
{
    ULOG(LOG_INFO, "%s, assayCode number:%d.", __FUNCTION__, assayCode.size());

    // 排除应版本而产生的相同assayCode
    std::set<int32_t> uniqAssay;

    m_assayData.clear();
    for (const auto& assay : assayCode)
    {
        if (uniqAssay.find(assay) != uniqAssay.end())
        {
            continue;
        }
        uniqAssay.insert(assay);

        ::tf::QcComposition compostion;
        compostion.__set_assayCode(assay);
        compostion.__set_targetValue(0);
        compostion.__set_sd(0);

        m_assayData.push_back(compostion);
    }

    return true;
}

///
/// @brief
///     获取单元格属性(可编辑、可选择)
///
/// @param[in]    index      当前单元格索引
///
/// @return       QVariant   包装的数据
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月30日，新建函数
///
Qt::ItemFlags QQcAssayEditModel::flags(const QModelIndex &index) const
{
    if (index.column() == 1
        || index.column() == 2
        || index.column() == 3)
    {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
    }
    else
    {
        return Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    }
}

///
/// @brief
///     表头显示正常颜色
///
/// @param[in]  section      段
/// @param[in]  orientation  方向
/// @param[in]  role         角色
///
/// @return 数据
///
/// @par History:
/// @li 4170/TangChuXian，2023年7月28日，新建函数
///
QVariant QQcAssayEditModel::headerData(int section, Qt::Orientation orientation, int role /*= Qt::DisplayRole*/) const
{
    if (orientation == Qt::Horizontal) {
        if (role == Qt::DisplayRole) {
            return m_heads.at(section);
        }

        if (role == Qt::ForegroundRole && (int(COLASSAY::TARGETVALUE) == section ||
            int(COLASSAY::STANDARD) == section ||
            int(COLASSAY::CV) == section))
        {
            QBrush brush(UI_TEXT_COLOR_HEADER_MARK);
            return brush;
        }
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

QcRowData::QcRowData(const boost::optional<tf::QcDoc>& qcData)
{
    if (qcData)
    {
        rowSequence = qcData->index;
        qcItem = qcData;
    }
}

QcRowData::QcRowData(int seq)
{
    rowSequence = seq;
    qcItem = boost::none;
}

QQcSampleModel::QcFilter::QcFilter(const std::string& nfilter, tf::DocRegistType::type regfilter)
{
    m_nameFilter = nfilter;
    m_registerTypeFilter = regfilter;
}

bool QQcSampleModel::QcFilter::IsFitCondition(const std::string& qcName, tf::DocRegistType::type typeReg)
{
    // 不满足名称过滤
    if (!m_nameFilter.empty() && qcName.find(m_nameFilter) == std::string::npos)
    {
        return false;
    }
    // 不满足登记方式过滤
    if (m_registerTypeFilter != tf::DocRegistType::REG_EMTPY && m_registerTypeFilter != typeReg)
    {
        return false;
    }

    return true;
}
