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
/// @file     qcapplymodel.cpp
/// @brief    质控申请数据模型
///
/// @author   4170/TangChuXian
/// @date     2024年3月21日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月21日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "qcapplymodel.h"
#include "shared/uidef.h"
#include <QTimer>
#include <QTableView>

// 注册内部类型
Q_DECLARE_METATYPE(QC_APPLY_INFO)

// 初始化静态成员变量
QMap<QcApplyModel*, QMap<int, QList<QStandardItem *>>> QcApplyModel::sm_mapPrepInsertBuffer;

///
/// @brief 获取日期Item,如果过期，则用红色填充单元格，并且字体也为红色
///
/// @param[in]  StrDate  yyyy-MM-dd固定格式的日期字符串
///
/// @return 设置好的item
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年11月14日，新建函数
///
static QStandardItem* GetDateItem(const QString & StrDate)
{
    QStandardItem * item = new QStandardItem(StrDate);
    item->setTextAlignment(Qt::AlignCenter);

    // 失效日期
    QDate expDate = QDate::fromString(StrDate, "yyyy-MM-dd");

    // 是否过期
    if (expDate.isValid() && expDate <= QDate::currentDate())
    {
        item->setData(UI_REAGENT_WARNCOLOR, Qt::UserRole + 5);
        item->setData(QColor(UI_REAGENT_WARNFONT), Qt::ForegroundRole);
    }

    return item;
}

QcApplyModel::QcApplyModel(QObject *parent, QcApplyType enQcType)
    : QStandardItemModel(parent),
      m_cenQcType(enQcType),
      m_pTblView(Q_NULLPTR),
      m_bOldERder(true)
{
    //m_pItemUpdateTimer = new QTimer(this);
    //m_pItemUpdateTimer->setSingleShot(true);
    //connect(m_pItemUpdateTimer, SIGNAL(timeout()), this, SLOT(InsertItems()));
    //m_pItemUpdateTimer->start(200);
}

QcApplyModel::~QcApplyModel()
{
}

///
/// @brief
///     覆盖item
///
/// @param[in]  row     行号
/// @param[in]  column  列号
///
/// @return 对应item项
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月22日，新建函数
///
// QStandardItem* QcApplyModel::item(int iRow, int iCol /*= 0*/) const
// {
//     auto it = sm_mapPrepInsertBuffer.find((QcApplyModel*)this);
//     if (it == sm_mapPrepInsertBuffer.end())
//     {
//         return Q_NULLPTR;
//     }
// 
//     // 找对应行
//     auto itData = it.value().find(iRow);
//     if (itData == it.value().end())
//     {
//         return Q_NULLPTR;
//     }
// 
//     // 查找对应列
//     if (iCol < 0 || iCol >= itData.value().size())
//     {
//         return Q_NULLPTR;
//     }
// 
//     // 返回对应项
//     return itData.value().at(iCol);
// }

///
/// @brief
///     获取列数
///
/// @param[in]  parent  父节点索引
///
/// @return 列数
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月20日，新建函数
///
int QcApplyModel::columnCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
    return QStandardItemModel::columnCount(parent);
}

///
/// @brief
///     获取行数
///
/// @param[in]  parent  父节点索引
///
/// @return 行数
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月20日，新建函数
///
int QcApplyModel::rowCount(const QModelIndex &parent /*= QModelIndex()*/) const
{
    return m_lstDataBuffer.size();
}

///
/// @brief
///     获取数据
///
/// @param[in]  index   索引
/// @param[in]  role    角色
///
/// @return 数据
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月20日，新建函数
///
QVariant QcApplyModel::data(const QModelIndex &index, int role /*= Qt::DisplayRole*/) const
{
    // 只可以取显示行和选中行
    if (m_pTblView == Q_NULLPTR)
    {
        return QStandardItemModel::data(index, role);
    }

    // 参数检查
    if (!index.isValid())
    {
        return QStandardItemModel::data(index, role);
    }

    // 对齐方式
    if (role == Qt::TextAlignmentRole)
    {
        return QVariant::fromValue<int>(Qt::AlignHCenter | Qt::AlignVCenter);
    }

    // 只对特定行有效
    int iMinRow = m_pTblView->rowAt(0);
    unsigned int iMaxRow = static_cast<unsigned int>(m_pTblView->rowAt(m_pTblView->height()));
    int iSelRow = -1;
    QModelIndex curIndex = m_pTblView->currentIndex();
    if (curIndex.isValid())
    {
        iSelRow = curIndex.row();
    }

    // 判断是否需要显示
    int iRow = index.row();
    if ((iRow < iMinRow || iRow > iMaxRow) && (iRow != iSelRow))
    {
        return QStandardItemModel::data(index, role);
    }

    // 没有对应行，则跳过
    if ((iRow < 0) || (iRow >= m_lstDataBuffer.size()))
    {
        // 返回结果
        return QStandardItemModel::data(index, role);
    }

    // 获取对应行数据
    const QC_APPLY_INFO& stuQcApplyItem = m_lstDataBuffer[iRow];

    // 选中ID匹配则选中
    if (m_strSelID == stuQcApplyItem.strID && iSelRow != iRow)
    {
        m_pTblView->selectRow(iRow);
    }

    // 筛选匹配则显示
    if (IsMatchFilter(stuQcApplyItem) && m_pTblView->isRowHidden(iRow))
    {
        m_pTblView->showRow(iRow);
    }

    if (!IsMatchFilter(stuQcApplyItem) && !m_pTblView->isRowHidden(iRow))
    {
        m_pTblView->hideRow(iRow);
    }

    // 分别处理
    switch (m_cenQcType)
    {
    case QC_APPLY_TYPE_ON_USE:
        return OnUseQcData(index, role);
    case QC_APPLY_TYPE_DEFAULT:
        return DefaultQcData(index, role);
    case QC_APPLY_TYPE_BACKUP:
        return BackupQcData(index, role);
    default:
        break;
    }

    return QStandardItemModel::data(index, role);


    // 是否数据已经缓存
//     if (m_setInsertRows.contains(iRow))
//     {
//         return QStandardItemModel::data(index, role);
//     }

    // 通知插入缓存数据
    //m_pItemUpdateTimer->start(100)

    // 是否在准备插入的数据缓存中
//     int iCol = index.column();
//     QStandardItem* pItem = item(iRow, iCol);
//     if (pItem != Q_NULLPTR)
//     {
//         return pItem->data(role);
//     }
// 
//     // 加载缓存数据
//     bool bLoadRlt = false;
//     QList<QStandardItem *> rowBuffer;
//     switch (m_cenQcType)
//     {
//     case QC_APPLY_TYPE_ON_USE:
//         LoadOneRowOnUseQcApplyInfo(iRow, rowBuffer);
//         break;
//     case QC_APPLY_TYPE_DEFAULT:
//         LoadOneRowDefaultQcApplyInfo(iRow, rowBuffer);
//         break;
//     case QC_APPLY_TYPE_BACKUP:
//         LoadOneRowBackupQcApplyInfo(iRow, rowBuffer);
//         break;
//     default:
        //break;
//     }
// 
//     // 往缓存中插入数据
//     auto it = sm_mapPrepInsertBuffer.find((QcApplyModel*)this);
//     if (it == sm_mapPrepInsertBuffer.end())
//     {
//         sm_mapPrepInsertBuffer.insert((QcApplyModel*)this, QMap<int, QList<QStandardItem *>>());
//         it = sm_mapPrepInsertBuffer.find((QcApplyModel*)this);
//         it.value().insert(iRow, rowBuffer);
//     }
//     else
//     {
//         it.value().insert(iRow, rowBuffer);
//     }
// 
//     // 获取缓存插入项
//     pItem = item(iRow, iCol);
//     if (pItem != Q_NULLPTR)
//     {
//         return pItem->data(role);
//     }
// 
//     // 返回结果
//     return QStandardItemModel::data(index, role);
}

///
/// @brief
///     获取在用质控数据
///
/// @param[in]  index   索引
/// @param[in]  role    角色
///
/// @return 数据
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月23日，新建函数
///
QVariant QcApplyModel::OnUseQcData(const QModelIndex &index, int role /*= Qt::DisplayRole*/) const
{
    // 模式检查
    if (m_cenQcType != QC_APPLY_TYPE_ON_USE)
    {
        // 返回结果
        return QStandardItemModel::data(index, role);
    }

    // 判断是否需要显示
    int iRow = index.row();
    int iCol = index.column();

    // 没有对应行，则跳过
    if ((iRow < 0) || (iRow >= m_lstDataBuffer.size()))
    {
        // 返回结果
        return QStandardItemModel::data(index, role);
    }

    // 获取对应行数据
    const QC_APPLY_INFO& stuQcApplyItem = m_lstDataBuffer[iRow];

    // 显示文本
    if (role == Qt::DisplayRole)
    {
        switch (iCol)
        {
        case Qah_Module:
            return stuQcApplyItem.strModuleName;
        case Qah_AssayName:
            return stuQcApplyItem.strAssayName;
        case Qah_QcNo:
            return stuQcApplyItem.strQcNo;
        case Qah_QcName:
            return stuQcApplyItem.strQcName;
        case Qah_QcBriefName:
            return stuQcApplyItem.strQcBriefName;
        case Qah_QcType:
            return stuQcApplyItem.strQcSourceType;
        case Qah_QcLevel:
            return stuQcApplyItem.strQcLevel;
        case Qah_QcLot:
            return stuQcApplyItem.strQcLot;
        case Qah_QcReason:
            return stuQcApplyItem.strQcReason;
        case Qah_Selected:
            return QString("");
        case Qah_DbIndex:
            return stuQcApplyItem.strID;
        default:
            break;
        }
    }

    // 显示勾选图标
    if (role == Qt::DecorationRole && iCol == Qah_Selected)
    {
        return stuQcApplyItem.bSelected ? QIcon(":/Leonis/resource/image/icon-select.png") : QIcon();
    }

    return QStandardItemModel::data(index, role);
}

///
/// @brief
///     获取默认质控数据
///
/// @param[in]  index   索引
/// @param[in]  role    角色
///
/// @return 数据
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月23日，新建函数
///
QVariant QcApplyModel::DefaultQcData(const QModelIndex &index, int role /*= Qt::DisplayRole*/) const
{
    // 模式检查
    if (m_cenQcType != QC_APPLY_TYPE_DEFAULT)
    {
        // 返回结果
        return QStandardItemModel::data(index, role);
    }

    // 判断是否需要显示
    int iRow = index.row();
    int iCol = index.column();

    // 没有对应行，则跳过
    if ((iRow < 0) || (iRow >= m_lstDataBuffer.size()))
    {
        // 返回结果
        return QStandardItemModel::data(index, role);
    }

    // 获取对应行数据
    const QC_APPLY_INFO& stuQcApplyItem = m_lstDataBuffer[iRow];

    // 构造数据
    // 构造沟渠日期
    QStringList strDateTimeList = stuQcApplyItem.strQcExpDate.split(" ");
    QString strDate("");
    if (strDateTimeList.size() >= 1)
    {
        strDate = strDateTimeList.front();
    }

    // 显示文本
    if (role == Qt::DisplayRole)
    {
        switch (iCol)
        {
        case Dqh_Module:
            return stuQcApplyItem.strModuleName;
        case Dqh_AssayName:
            return stuQcApplyItem.strAssayName;
        case Dqh_QcNo:
            return stuQcApplyItem.strQcNo;
        case Dqh_QcName:
            return stuQcApplyItem.strQcName;
        case Dqh_QcBriefName:
            return stuQcApplyItem.strQcBriefName;
        case Dqh_QcType:
            return stuQcApplyItem.strQcSourceType;
        case Dqh_QcLevel:
            return stuQcApplyItem.strQcLevel;
        case Dqh_QcLot:
            return stuQcApplyItem.strQcLot;
        case Dqh_ExpDate:
            return strDate;
        case Dqh_Selected:
            return QString("");
        case Dqh_DbIndex:
            return stuQcApplyItem.strID;
        default:
            break;
        }
    }

    // 失效日期颜色
    if (role == Qt::TextColorRole && iCol == Dqh_ExpDate)
    {
        QDate expDate = QDate::fromString(strDate, "yyyy-MM-dd");

        // 是否过期
        if (expDate.isValid() && expDate <= QDate::currentDate())
        {
            return QColor(UI_REAGENT_WARNFONT);
        }

        return QStandardItemModel::data(index, role);
    }

    // 显示勾选图标
    if (role == Qt::DecorationRole && iCol == Dqh_Selected)
    {
        return stuQcApplyItem.bSelected ? QIcon(":/Leonis/resource/image/icon-select.png") : QIcon();
    }

    return QStandardItemModel::data(index, role);
}

///
/// @brief
///     获取备用瓶质控数据
///
/// @param[in]  index   索引
/// @param[in]  role    角色
///
/// @return 数据
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月23日，新建函数
///
QVariant QcApplyModel::BackupQcData(const QModelIndex &index, int role /*= Qt::DisplayRole*/) const
{
    // 模式检查
    if (m_cenQcType != QC_APPLY_TYPE_BACKUP)
    {
        // 返回结果
        return QStandardItemModel::data(index, role);
    }

    // 判断是否需要显示
    int iRow = index.row();
    int iCol = index.column();

    // 没有对应行，则跳过
    if ((iRow < 0) || (iRow >= m_lstDataBuffer.size()))
    {
        // 返回结果
        return QStandardItemModel::data(index, role);
    }

    // 获取对应行数据
    const QC_APPLY_INFO& stuQcApplyItem = m_lstDataBuffer[iRow];

    // 构造数据
    // 构造沟渠日期
    QStringList strDateTimeList = stuQcApplyItem.strQcExpDate.split(" ");
    QString strDate("");
    if (strDateTimeList.size() >= 1)
    {
        strDate = strDateTimeList.front();
    }

    // 显示文本
    if (role == Qt::DisplayRole)
    {
        switch (iCol)
        {
        case Bqh_Pos:
            return stuQcApplyItem.strPos;
        case Bqh_AssayName:
            return stuQcApplyItem.strAssayName;
        case Bqh_RgntLot:
            return stuQcApplyItem.strRgntLot;
        case Bqh_RgntNo:
            return stuQcApplyItem.strRgntNo;
        case Bqh_UseStatus:
            return stuQcApplyItem.strRgntUseStatus;
        case Bqh_QcNo:
            return stuQcApplyItem.strQcNo;
        case Bqh_QcName:
            return stuQcApplyItem.strQcName;
        case Bqh_QcBriefName:
            return stuQcApplyItem.strQcBriefName;
        case Bqh_QcType:
            return stuQcApplyItem.strQcSourceType;
        case Bqh_QcLevel:
            return stuQcApplyItem.strQcLevel;
        case Bqh_QcLot:
            return stuQcApplyItem.strQcLot;
        case Bqh_ExpDate:
            return strDate;
        case Bqh_Selected:
            return QString("");
        case Bqh_DbIndex:
            return stuQcApplyItem.strID;
        default:
            break;
        }
    }

    // 失效日期颜色
    if (role == Qt::TextColorRole && iCol == Bqh_ExpDate)
    {
        QDate expDate = QDate::fromString(strDate, "yyyy-MM-dd");

        // 是否过期
        if (expDate.isValid() && expDate <= QDate::currentDate())
        {
            return QColor(UI_REAGENT_WARNFONT);
        }

        return QStandardItemModel::data(index, role);
    }

    // 显示勾选图标
    if (role == Qt::DecorationRole && iCol == Bqh_Selected)
    {
        return stuQcApplyItem.bSelected ? QIcon(":/Leonis/resource/image/icon-select.png") : QIcon();
    }

    return QStandardItemModel::data(index, role);
}

///
/// @brief
///     写入数据
///
/// @param[in]  index   索引
/// @param[in]  value   值
/// @param[in]  role    角色
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月22日，新建函数
///
bool QcApplyModel::setData(const QModelIndex &index, const QVariant &value, int role /*= Qt::EditRole*/)
{
    // 参数检查
    if (!index.isValid())
    {
        return false;
    }

    // 图标图书处理
    if (role != Qt::DecorationRole)
    {
        return false;
    }

// 
//     QStandardItem* pItem = item(index.row(), index.column());
//     if (pItem == Q_NULLPTR)
//     {
//         return false;
//     }
// 
//     pItem->setData(value, role);

    // 图标转换
    bool bSel = false;
    if (value.canConvert<QIcon>())
    {
        QIcon selIcon = value.value<QIcon>();
        bSel = !selIcon.isNull();
    }

    // 判断是否需要显示
    int iRow = index.row();
    int iCol = index.column();

    // 模式判断
    if ((m_cenQcType == QC_APPLY_TYPE_ON_USE && iCol == Qah_Selected) ||
        (m_cenQcType == QC_APPLY_TYPE_DEFAULT && iCol == Dqh_Selected) || 
        (m_cenQcType == QC_APPLY_TYPE_BACKUP && iCol == Bqh_Selected))
    {
        // 获取对应行数据
        QC_APPLY_INFO& stuQcApplyItem = m_lstDataBuffer[iRow];
        stuQcApplyItem.bSelected = bSel;

        return true;
    }

    return false;
}

///
/// @brief
///     获取项目标志
///
/// @param[in]  index  索引
///
/// @return 项目标志
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月20日，新建函数
///
Qt::ItemFlags QcApplyModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

///
/// @brief
///     设置选中ID
///
/// @param[in]  strQcNameFilter  筛选质控名
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月23日，新建函数
///
void QcApplyModel::SetQcNameFilter(const QString& strQcNameFilter)
{
    // 否则更新筛选条件
    m_strQcNameFilter = strQcNameFilter;

    // 如果没有对应的表则返回
    if (m_pTblView == Q_NULLPTR)
    {
        return;
    }

    // 将所有行置为显示
    for (int iRow = 0; iRow < m_lstDataBuffer.size(); iRow++)
    {
        if (IsMatchFilter(m_lstDataBuffer[iRow]))
        {
            m_pTblView->showRow(iRow);
        }
        else
        {
            m_pTblView->hideRow(iRow);
        }
    }
}

///
/// @brief
///     排序
///
/// @param[in]  column  列号
/// @param[in]  order   排序方式
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月20日，新建函数
///
void QcApplyModel::sort(int column, Qt::SortOrder order /*= Qt::AscendingOrder*/)
{
    auto ShowAllRow = [this]()
    {
        // 如果没有对应的表则返回
        if (m_pTblView == Q_NULLPTR)
        {
            return;
        }

        // 将所有行置为显示
        for (int iRow = 0; iRow < m_lstDataBuffer.size(); iRow++)
        {
            if (IsMatchFilter(m_lstDataBuffer[iRow]))
            {
                m_pTblView->showRow(iRow);
            }
            else
            {
                m_pTblView->hideRow(iRow);
            }
        }
    };

    // 如果没有数据，直接跳过
    if (m_lstDataBuffer.isEmpty() || column >= columnCount())
    {
        return;
    }

    if ((column != columnCount() - 1) &&  m_bOldERder)
    {
        RecordOldOrder();
    }

    if ((column == columnCount() - 1))
    {
        if (m_cenQcType == QC_APPLY_TYPE_BACKUP && !m_bOldERder)
        {
            sortBackupQcApplyInfo(Bqh_Pos, Qt::AscendingOrder);
            m_bOldERder = true;
            ShowAllRow();
            UpdateLaout();
        }

        if (!m_bOldERder)
        {
            RecoverOldOrder();
            m_bOldERder = true;
            ShowAllRow();
            UpdateLaout();
        }
        return;
    }

    // 判断质控类型
    switch (m_cenQcType)
    {
    case QC_APPLY_TYPE_ON_USE:
        sortOnUseQcApplyInfo(column, order);
        break;
    case QC_APPLY_TYPE_DEFAULT:
        sortDefaultQcApplyInfo(column, order);
        break;
    case QC_APPLY_TYPE_BACKUP:
        sortBackupQcApplyInfo(column, order);
        break;
    default:
        break;
    }

    m_bOldERder = false;
    ShowAllRow();
    UpdateLaout();
}

///
/// @brief
///     指定质控类型排序(在用)
///
/// @param[in]  column  列号
/// @param[in]  order   排序方式
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月21日，新建函数
///
void QcApplyModel::sortOnUseQcApplyInfo(int column, Qt::SortOrder order /*= Qt::AscendingOrder*/)
{
    // 判断排序列
    switch (column)
    {
    case Qah_Module:
        qSort(m_lstDataBuffer.begin(), m_lstDataBuffer.end(), 
            [order](const QC_APPLY_INFO& stuApplyInfo1, const QC_APPLY_INFO& stuApplyInfo2)
        {return order == Qt::AscendingOrder ? stuApplyInfo1.strModuleName < stuApplyInfo2.strModuleName : stuApplyInfo1.strModuleName > stuApplyInfo2.strModuleName; });
        break;
    case Qah_AssayName:
        qSort(m_lstDataBuffer.begin(), m_lstDataBuffer.end(),
            [order](const QC_APPLY_INFO& stuApplyInfo1, const QC_APPLY_INFO& stuApplyInfo2)
        {return order == Qt::AscendingOrder ? stuApplyInfo1.strAssayName.toLower() < stuApplyInfo2.strAssayName.toLower() : stuApplyInfo1.strAssayName.toLower() > stuApplyInfo2.strAssayName.toLower(); });
        break;
    case Qah_QcNo:
        qSort(m_lstDataBuffer.begin(), m_lstDataBuffer.end(),
            [order](const QC_APPLY_INFO& stuApplyInfo1, const QC_APPLY_INFO& stuApplyInfo2)
        {return order == Qt::AscendingOrder ? stuApplyInfo1.strQcNo < stuApplyInfo2.strQcNo : stuApplyInfo1.strQcNo > stuApplyInfo2.strQcNo; });
        break;
    case Qah_QcName:
        qSort(m_lstDataBuffer.begin(), m_lstDataBuffer.end(),
            [order](const QC_APPLY_INFO& stuApplyInfo1, const QC_APPLY_INFO& stuApplyInfo2)
        {return order == Qt::AscendingOrder ? stuApplyInfo1.strQcName < stuApplyInfo2.strQcName : stuApplyInfo1.strQcName > stuApplyInfo2.strQcName; });
        break;
    case Qah_QcBriefName:
        qSort(m_lstDataBuffer.begin(), m_lstDataBuffer.end(),
            [order](const QC_APPLY_INFO& stuApplyInfo1, const QC_APPLY_INFO& stuApplyInfo2)
        {return order == Qt::AscendingOrder ? stuApplyInfo1.strQcBriefName < stuApplyInfo2.strQcBriefName : stuApplyInfo1.strQcBriefName > stuApplyInfo2.strQcBriefName; });
        break;
    case Qah_QcType:
        qSort(m_lstDataBuffer.begin(), m_lstDataBuffer.end(),
            [order](const QC_APPLY_INFO& stuApplyInfo1, const QC_APPLY_INFO& stuApplyInfo2)
        {return order == Qt::AscendingOrder ? stuApplyInfo1.strQcSourceType < stuApplyInfo2.strQcSourceType : stuApplyInfo1.strQcSourceType > stuApplyInfo2.strQcSourceType; });
        break;
    case Qah_QcLevel:
        qSort(m_lstDataBuffer.begin(), m_lstDataBuffer.end(),
            [order](const QC_APPLY_INFO& stuApplyInfo1, const QC_APPLY_INFO& stuApplyInfo2)
        {return order == Qt::AscendingOrder ? stuApplyInfo1.strQcLevel < stuApplyInfo2.strQcLevel : stuApplyInfo1.strQcLevel > stuApplyInfo2.strQcLevel; });
        break;
    case Qah_QcLot:
        qSort(m_lstDataBuffer.begin(), m_lstDataBuffer.end(),
            [order](const QC_APPLY_INFO& stuApplyInfo1, const QC_APPLY_INFO& stuApplyInfo2)
        {return order == Qt::AscendingOrder ? stuApplyInfo1.strQcLot < stuApplyInfo2.strQcLot : stuApplyInfo1.strQcLot > stuApplyInfo2.strQcLot; });
        break;
    case Qah_QcReason:
        qSort(m_lstDataBuffer.begin(), m_lstDataBuffer.end(),
            [order](const QC_APPLY_INFO& stuApplyInfo1, const QC_APPLY_INFO& stuApplyInfo2)
        {return order == Qt::AscendingOrder ? stuApplyInfo1.strQcReason < stuApplyInfo2.strQcReason : stuApplyInfo1.strQcReason > stuApplyInfo2.strQcReason; });
        break;
    case Qah_Selected:
        qSort(m_lstDataBuffer.begin(), m_lstDataBuffer.end(),
            [order](const QC_APPLY_INFO& stuApplyInfo1, const QC_APPLY_INFO& stuApplyInfo2)
        {return order == Qt::AscendingOrder ? stuApplyInfo1.bSelected < stuApplyInfo2.bSelected : stuApplyInfo1.bSelected > stuApplyInfo2.bSelected; });
        break;
    default:
        break;
    }
}

///
/// @brief
///     指定质控类型排序(默认)
///
/// @param[in]  column  列号
/// @param[in]  order   排序方式
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月21日，新建函数
///
void QcApplyModel::sortDefaultQcApplyInfo(int column, Qt::SortOrder order /*= Qt::AscendingOrder*/)
{
    // 判断排序列
    switch (column)
    {
    case Dqh_Module:
        qSort(m_lstDataBuffer.begin(), m_lstDataBuffer.end(),
            [order](const QC_APPLY_INFO& stuApplyInfo1, const QC_APPLY_INFO& stuApplyInfo2)
        {return order == Qt::AscendingOrder ? stuApplyInfo1.strModuleName < stuApplyInfo2.strModuleName : stuApplyInfo1.strModuleName > stuApplyInfo2.strModuleName; });
        break;
    case Dqh_AssayName:
        qSort(m_lstDataBuffer.begin(), m_lstDataBuffer.end(),
            [order](const QC_APPLY_INFO& stuApplyInfo1, const QC_APPLY_INFO& stuApplyInfo2)
        {return order == Qt::AscendingOrder ? stuApplyInfo1.strAssayName.toLower() < stuApplyInfo2.strAssayName.toLower() : stuApplyInfo1.strAssayName.toLower() > stuApplyInfo2.strAssayName.toLower(); });
        break;
    case Dqh_QcNo:
        qSort(m_lstDataBuffer.begin(), m_lstDataBuffer.end(),
            [order](const QC_APPLY_INFO& stuApplyInfo1, const QC_APPLY_INFO& stuApplyInfo2)
        {return order == Qt::AscendingOrder ? stuApplyInfo1.strQcNo < stuApplyInfo2.strQcNo : stuApplyInfo1.strQcNo > stuApplyInfo2.strQcNo; });
        break;
    case Dqh_QcName:
        qSort(m_lstDataBuffer.begin(), m_lstDataBuffer.end(),
            [order](const QC_APPLY_INFO& stuApplyInfo1, const QC_APPLY_INFO& stuApplyInfo2)
        {return order == Qt::AscendingOrder ? stuApplyInfo1.strQcName < stuApplyInfo2.strQcName : stuApplyInfo1.strQcName > stuApplyInfo2.strQcName; });
        break;
    case Dqh_QcBriefName:
        qSort(m_lstDataBuffer.begin(), m_lstDataBuffer.end(),
            [order](const QC_APPLY_INFO& stuApplyInfo1, const QC_APPLY_INFO& stuApplyInfo2)
        {return order == Qt::AscendingOrder ? stuApplyInfo1.strQcBriefName < stuApplyInfo2.strQcBriefName : stuApplyInfo1.strQcBriefName > stuApplyInfo2.strQcBriefName; });
        break;
    case Dqh_QcType:
        qSort(m_lstDataBuffer.begin(), m_lstDataBuffer.end(),
            [order](const QC_APPLY_INFO& stuApplyInfo1, const QC_APPLY_INFO& stuApplyInfo2)
        {return order == Qt::AscendingOrder ? stuApplyInfo1.strQcSourceType < stuApplyInfo2.strQcSourceType : stuApplyInfo1.strQcSourceType > stuApplyInfo2.strQcSourceType; });
        break;
    case Dqh_QcLevel:
        qSort(m_lstDataBuffer.begin(), m_lstDataBuffer.end(),
            [order](const QC_APPLY_INFO& stuApplyInfo1, const QC_APPLY_INFO& stuApplyInfo2)
        {return order == Qt::AscendingOrder ? stuApplyInfo1.strQcLevel < stuApplyInfo2.strQcLevel : stuApplyInfo1.strQcLevel > stuApplyInfo2.strQcLevel; });
        break;
    case Dqh_QcLot:
        qSort(m_lstDataBuffer.begin(), m_lstDataBuffer.end(),
            [order](const QC_APPLY_INFO& stuApplyInfo1, const QC_APPLY_INFO& stuApplyInfo2)
        {return order == Qt::AscendingOrder ? stuApplyInfo1.strQcLot < stuApplyInfo2.strQcLot : stuApplyInfo1.strQcLot > stuApplyInfo2.strQcLot; });
        break;
    case Dqh_ExpDate:
        qSort(m_lstDataBuffer.begin(), m_lstDataBuffer.end(),
            [order](const QC_APPLY_INFO& stuApplyInfo1, const QC_APPLY_INFO& stuApplyInfo2)
        {return order == Qt::AscendingOrder ? stuApplyInfo1.strQcExpDate < stuApplyInfo2.strQcExpDate : stuApplyInfo1.strQcExpDate > stuApplyInfo2.strQcExpDate; });
        break;
    case Dqh_Selected:
        qSort(m_lstDataBuffer.begin(), m_lstDataBuffer.end(),
            [order](const QC_APPLY_INFO& stuApplyInfo1, const QC_APPLY_INFO& stuApplyInfo2)
        {return order == Qt::AscendingOrder ? stuApplyInfo1.bSelected < stuApplyInfo2.bSelected : stuApplyInfo1.bSelected > stuApplyInfo2.bSelected; });
        break;
    default:
        break;
    }
}

///
/// @brief
///     指定质控类型排序(备用)
///
/// @param[in]  column  列号
/// @param[in]  order   排序方式
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月21日，新建函数
///
void QcApplyModel::sortBackupQcApplyInfo(int column, Qt::SortOrder order /*= Qt::AscendingOrder*/)
{
    // 判断排序列
    switch (column)
    {
    case Bqh_Pos:
        qSort(m_lstDataBuffer.begin(), m_lstDataBuffer.end(),
            [order](const QC_APPLY_INFO& stuApplyInfo1, const QC_APPLY_INFO& stuApplyInfo2)
        {
            QString strSortVal1 = QString("%1%2%3").arg(stuApplyInfo1.strDevName).arg(stuApplyInfo1.iBackupRgntPos, 2, 10, QChar('0')).arg(stuApplyInfo1.strQcNo);
            QString strSortVal2 = QString("%1%2%3").arg(stuApplyInfo2.strDevName).arg(stuApplyInfo2.iBackupRgntPos, 2, 10, QChar('0')).arg(stuApplyInfo2.strQcNo);
            return order == Qt::AscendingOrder ? strSortVal1 < strSortVal2 : strSortVal1 > strSortVal2;
        });
        break;
    case Bqh_AssayName:
        qSort(m_lstDataBuffer.begin(), m_lstDataBuffer.end(),
            [order](const QC_APPLY_INFO& stuApplyInfo1, const QC_APPLY_INFO& stuApplyInfo2)
        {return order == Qt::AscendingOrder ? stuApplyInfo1.strAssayName.toLower() < stuApplyInfo2.strAssayName.toLower() : stuApplyInfo1.strAssayName.toLower() > stuApplyInfo2.strAssayName.toLower(); });
        break;
    case Bqh_RgntLot:
        qSort(m_lstDataBuffer.begin(), m_lstDataBuffer.end(),
            [order](const QC_APPLY_INFO& stuApplyInfo1, const QC_APPLY_INFO& stuApplyInfo2)
        {return order == Qt::AscendingOrder ? stuApplyInfo1.strRgntLot < stuApplyInfo2.strRgntLot : stuApplyInfo1.strRgntLot > stuApplyInfo2.strRgntLot; });
        break;
    case Bqh_RgntNo:
        qSort(m_lstDataBuffer.begin(), m_lstDataBuffer.end(),
            [order](const QC_APPLY_INFO& stuApplyInfo1, const QC_APPLY_INFO& stuApplyInfo2)
        {return order == Qt::AscendingOrder ? stuApplyInfo1.strRgntNo < stuApplyInfo2.strRgntNo : stuApplyInfo1.strRgntNo > stuApplyInfo2.strRgntNo; });
        break;
    case Bqh_UseStatus:
        qSort(m_lstDataBuffer.begin(), m_lstDataBuffer.end(),
            [order](const QC_APPLY_INFO& stuApplyInfo1, const QC_APPLY_INFO& stuApplyInfo2)
        {return order == Qt::AscendingOrder ? stuApplyInfo1.strRgntUseStatus < stuApplyInfo2.strRgntUseStatus : stuApplyInfo1.strRgntUseStatus > stuApplyInfo2.strRgntUseStatus; });
        break;
    case Bqh_QcNo:
        qSort(m_lstDataBuffer.begin(), m_lstDataBuffer.end(),
            [order](const QC_APPLY_INFO& stuApplyInfo1, const QC_APPLY_INFO& stuApplyInfo2)
        {return order == Qt::AscendingOrder ? stuApplyInfo1.strQcNo < stuApplyInfo2.strQcNo : stuApplyInfo1.strQcNo > stuApplyInfo2.strQcNo; });
        break;
    case Bqh_QcName:
        qSort(m_lstDataBuffer.begin(), m_lstDataBuffer.end(),
            [order](const QC_APPLY_INFO& stuApplyInfo1, const QC_APPLY_INFO& stuApplyInfo2)
        {return order == Qt::AscendingOrder ? stuApplyInfo1.strQcName < stuApplyInfo2.strQcName : stuApplyInfo1.strQcName > stuApplyInfo2.strQcName; });
        break;
    case Bqh_QcBriefName:
        qSort(m_lstDataBuffer.begin(), m_lstDataBuffer.end(),
            [order](const QC_APPLY_INFO& stuApplyInfo1, const QC_APPLY_INFO& stuApplyInfo2)
        {return order == Qt::AscendingOrder ? stuApplyInfo1.strQcBriefName < stuApplyInfo2.strQcBriefName : stuApplyInfo1.strQcBriefName > stuApplyInfo2.strQcBriefName; });
        break;
    case Bqh_QcType:
        qSort(m_lstDataBuffer.begin(), m_lstDataBuffer.end(),
            [order](const QC_APPLY_INFO& stuApplyInfo1, const QC_APPLY_INFO& stuApplyInfo2)
        {return order == Qt::AscendingOrder ? stuApplyInfo1.strQcSourceType < stuApplyInfo2.strQcSourceType : stuApplyInfo1.strQcSourceType > stuApplyInfo2.strQcSourceType; });
        break;
    case Bqh_QcLevel:
        qSort(m_lstDataBuffer.begin(), m_lstDataBuffer.end(),
            [order](const QC_APPLY_INFO& stuApplyInfo1, const QC_APPLY_INFO& stuApplyInfo2)
        {return order == Qt::AscendingOrder ? stuApplyInfo1.strQcLevel < stuApplyInfo2.strQcLevel : stuApplyInfo1.strQcLevel > stuApplyInfo2.strQcLevel; });
        break;
    case Bqh_QcLot:
        qSort(m_lstDataBuffer.begin(), m_lstDataBuffer.end(),
            [order](const QC_APPLY_INFO& stuApplyInfo1, const QC_APPLY_INFO& stuApplyInfo2)
        {return order == Qt::AscendingOrder ? stuApplyInfo1.strQcLot < stuApplyInfo2.strQcLot : stuApplyInfo1.strQcLot > stuApplyInfo2.strQcLot; });
        break;
    case Bqh_ExpDate:
        qSort(m_lstDataBuffer.begin(), m_lstDataBuffer.end(),
            [order](const QC_APPLY_INFO& stuApplyInfo1, const QC_APPLY_INFO& stuApplyInfo2)
        {return order == Qt::AscendingOrder ? stuApplyInfo1.strQcExpDate < stuApplyInfo2.strQcExpDate : stuApplyInfo1.strQcExpDate > stuApplyInfo2.strQcExpDate; });
        break;
    case Bqh_Selected:
        qSort(m_lstDataBuffer.begin(), m_lstDataBuffer.end(),
            [order](const QC_APPLY_INFO& stuApplyInfo1, const QC_APPLY_INFO& stuApplyInfo2)
        {return order == Qt::AscendingOrder ? stuApplyInfo1.bSelected < stuApplyInfo2.bSelected : stuApplyInfo1.bSelected > stuApplyInfo2.bSelected; });
        break;
    default:
        break;
    }
}

///
/// @brief
///     加载一行指定质控类型质控申请数据（在用）
///
/// @param[in]  iRow        行号
/// @param[in]  rowBuffer   行数据缓存
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月21日，新建函数
///
bool QcApplyModel::LoadOneRowOnUseQcApplyInfo(int iRow, QList<QStandardItem *>& rowBuffer) const
{
    // 清空缓存
    rowBuffer.clear();

    // 没有对应行，则跳过
    if ((iRow < 0) || (iRow >= m_lstDataBuffer.size()))
    {
        return false;
    }

    // 获取对应行数据
    const QC_APPLY_INFO& stuQcApplyItem = m_lstDataBuffer[iRow];

    // 构造数据
    // 构造沟渠日期
    QStringList strDateTimeList = stuQcApplyItem.strQcExpDate.split(" ");
    QString strDate("");
    if (strDateTimeList.size() >= 1)
    {
        strDate = strDateTimeList.front();
    }

    rowBuffer.push_back(new QStandardItem(stuQcApplyItem.strModuleName));
    rowBuffer.push_back(new QStandardItem(stuQcApplyItem.strAssayName));
    rowBuffer.push_back(new QStandardItem(stuQcApplyItem.strQcNo));
    rowBuffer.push_back(new QStandardItem(stuQcApplyItem.strQcName));
    rowBuffer.push_back(new QStandardItem(stuQcApplyItem.strQcBriefName));
    rowBuffer.push_back(new QStandardItem(stuQcApplyItem.strQcSourceType));
    rowBuffer.push_back(new QStandardItem(stuQcApplyItem.strQcLevel));
    rowBuffer.push_back(new QStandardItem(stuQcApplyItem.strQcLot));
    rowBuffer.push_back(new QStandardItem(stuQcApplyItem.strQcReason));
    rowBuffer.push_back(new QStandardItem(""));
    rowBuffer.push_back(new QStandardItem(stuQcApplyItem.strID));

    // 设置选中图标
    rowBuffer[Qah_Selected]->setIcon(stuQcApplyItem.bSelected ? QIcon(":/Leonis/resource/image/icon-select.png") : QIcon());

    // 缓存质控申请数据
    rowBuffer[Qah_DbIndex]->setData(QVariant::fromValue<QC_APPLY_INFO>(stuQcApplyItem), Qt::UserRole);

    return true;
}

///
/// @brief
///     加载一行指定质控类型质控申请数据（默认）
///
/// @param[in]  iRow        行号
/// @param[in]  rowBuffer   行数据缓存
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月21日，新建函数
///
bool QcApplyModel::LoadOneRowDefaultQcApplyInfo(int iRow, QList<QStandardItem *>& rowBuffer) const
{
    // 清空缓存
    rowBuffer.clear();

    // 没有对应行，则跳过
    if ((iRow < 0) || (iRow >= m_lstDataBuffer.size()))
    {
        return false;
    }

    // 获取对应行数据
    const QC_APPLY_INFO& stuQcApplyItem = m_lstDataBuffer[iRow];

    // 构造数据
    // 构造沟渠日期
    QStringList strDateTimeList = stuQcApplyItem.strQcExpDate.split(" ");
    QString strDate("");
    if (strDateTimeList.size() >= 1)
    {
        strDate = strDateTimeList.front();
    }

    rowBuffer.push_back(new QStandardItem(stuQcApplyItem.strModuleName));
    rowBuffer.push_back(new QStandardItem(stuQcApplyItem.strAssayName));
    rowBuffer.push_back(new QStandardItem(stuQcApplyItem.strQcNo));
    rowBuffer.push_back(new QStandardItem(stuQcApplyItem.strQcName));
    rowBuffer.push_back(new QStandardItem(stuQcApplyItem.strQcBriefName));
    rowBuffer.push_back(new QStandardItem(stuQcApplyItem.strQcSourceType));
    rowBuffer.push_back(new QStandardItem(stuQcApplyItem.strQcLevel));
    rowBuffer.push_back(new QStandardItem(stuQcApplyItem.strQcLot));
    rowBuffer.push_back(GetDateItem(strDate));
    rowBuffer.push_back(new QStandardItem(""));
    rowBuffer.push_back(new QStandardItem(stuQcApplyItem.strID));

    // 设置选中图标
    rowBuffer[Dqh_Selected]->setIcon(stuQcApplyItem.bSelected ? QIcon(":/Leonis/resource/image/icon-select.png") : QIcon());

    // 缓存质控申请数据
    rowBuffer[Dqh_DbIndex]->setData(QVariant::fromValue<QC_APPLY_INFO>(stuQcApplyItem), Qt::UserRole);

    return true;
}

///
/// @brief
///     加载一行指定质控类型质控申请数据（备用）
///
/// @param[in]  iRow        行号
/// @param[in]  rowBuffer   行数据缓存
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月21日，新建函数
///
bool QcApplyModel::LoadOneRowBackupQcApplyInfo(int iRow, QList<QStandardItem *>& rowBuffer) const
{
    // 清空缓存
    rowBuffer.clear();

    // 没有对应行，则跳过
    if ((iRow < 0) || (iRow >= m_lstDataBuffer.size()))
    {
        return false;
    }

    // 获取对应行数据
    const QC_APPLY_INFO& stuQcApplyItem = m_lstDataBuffer[iRow];

    // 构造数据
    // 构造沟渠日期
    QStringList strDateTimeList = stuQcApplyItem.strQcExpDate.split(" ");
    QString strDate("");
    if (strDateTimeList.size() >= 1)
    {
        strDate = strDateTimeList.front();
    }

    // 添加数据
    rowBuffer.push_back(new QStandardItem(stuQcApplyItem.strPos));
    rowBuffer.push_back(new QStandardItem(stuQcApplyItem.strAssayName));
    rowBuffer.push_back(new QStandardItem(stuQcApplyItem.strRgntUseStatus));
    rowBuffer.push_back(new QStandardItem(stuQcApplyItem.strRgntNo));
    rowBuffer.push_back(new QStandardItem(stuQcApplyItem.strRgntLot));
    rowBuffer.push_back(new QStandardItem(stuQcApplyItem.strQcNo));
    rowBuffer.push_back(new QStandardItem(stuQcApplyItem.strQcName));
    rowBuffer.push_back(new QStandardItem(stuQcApplyItem.strQcBriefName));
    rowBuffer.push_back(new QStandardItem(stuQcApplyItem.strQcSourceType));
    rowBuffer.push_back(new QStandardItem(stuQcApplyItem.strQcLevel));
    rowBuffer.push_back(new QStandardItem(stuQcApplyItem.strQcLot));
    rowBuffer.push_back(GetDateItem(strDate));
    rowBuffer.push_back(new QStandardItem(""));
    rowBuffer.push_back(new QStandardItem(stuQcApplyItem.strID));

    // 位置列添加排序值
    //QString strSortVal = QString("%1%2").arg(stuQcApplyItem.strDevName).arg(stuQcApplyItem.iBackupRgntPos, 2, 10, QChar('0'));
    //pDlg->m_pStdModel->item(iRow, BackupRgntQcDlg::Bqh_Pos)->setData(strSortVal, UI_ITEM_ROLE_SEQ_NO_SORT);

    // 设置选中图标
    rowBuffer[Bqh_Selected]->setIcon(stuQcApplyItem.bSelected ? QIcon(":/Leonis/resource/image/icon-select.png") : QIcon());

    // 缓存质控申请数据
    rowBuffer[Bqh_DbIndex]->setData(QVariant::fromValue<QC_APPLY_INFO>(stuQcApplyItem), Qt::UserRole);

    return true;
}

///
/// @brief
///     更新布局
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月22日，新建函数
///
void QcApplyModel::UpdateLaout()
{
    emit layoutChanged();
}

///
/// @brief
///     设置特定行的质控申请信息
///
/// @param[in]  iRow            行号
/// @param[in]  stuQcApplyInfo  质控申请信息
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月22日，新建函数
///
bool QcApplyModel::SetApplyInfoByRow(int iRow, const QC_APPLY_INFO& stuQcApplyInfo)
{
    if (iRow < 0 || iRow >= m_lstDataBuffer.size())
    {
        return false;
    }

    // 设置对应值
    m_lstDataBuffer[iRow] = stuQcApplyInfo;
    return true;
}

///
/// @brief
///     获取特定行的质控申请信息
///
/// @param[in]  iRow            行号
/// @param[in]  stuQcApplyInfo  质控申请信息
///
/// @return true表示成功
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月22日，新建函数
///
bool QcApplyModel::GetApplyInfoByRow(int iRow, QC_APPLY_INFO& stuQcApplyInfo)
{
    if (iRow < 0 || iRow >= m_lstDataBuffer.size())
    {
        return false;
    }

    // 设置对应值
    stuQcApplyInfo = m_lstDataBuffer[iRow];
    return true;
}

///
/// @brief
///     是否匹配过滤器
///
/// @param[in]  stuQcApplyInfo  质控申请信息
///
/// @return true表示匹配
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月28日，新建函数
///
bool QcApplyModel::IsMatchFilter(const QC_APPLY_INFO& stuQcApplyInfo) const
{
    if (m_strQcNameFilter.isEmpty())
    {
        return true;
    }

    if (stuQcApplyInfo.strQcName.contains(m_strQcNameFilter))
    {
        return true;
    }

    return false;
}

///
/// @brief
///     记录原序
///
/// @par History:
/// @li 4170/TangChuXian，2024年4月24日，新建函数
///
void QcApplyModel::RecordOldOrder()
{
    // 遍历数组
    m_mapOldOrder.clear();
    for (int i = 0; i < m_lstDataBuffer.size(); i++)
    {
        m_mapOldOrder.insert(m_lstDataBuffer[i].strID + m_lstDataBuffer[i].strModuleName, i);
    }
}

///
/// @brief
///     恢复原序
///
/// @par History:
/// @li 4170/TangChuXian，2024年4月24日，新建函数
///
void QcApplyModel::RecoverOldOrder()
{
    qSort(m_lstDataBuffer.begin(), m_lstDataBuffer.end(),
        [this](const QC_APPLY_INFO& stuApplyInfo1, const QC_APPLY_INFO& stuApplyInfo2)
    {
        auto it1 = m_mapOldOrder.find(stuApplyInfo1.strID + stuApplyInfo1.strModuleName);
        auto it2 = m_mapOldOrder.find(stuApplyInfo2.strID + stuApplyInfo2.strModuleName);
        if (it1 == m_mapOldOrder.end() && it2 == m_mapOldOrder.end())
        {
            return false;
        }
        else if (it1 != m_mapOldOrder.end() && it2 == m_mapOldOrder.end())
        {
            return true;
        }
        else if (it1 == m_mapOldOrder.end() && it2 != m_mapOldOrder.end())
        {
            return false;
        }
        else if (it1 != m_mapOldOrder.end() && it2 != m_mapOldOrder.end())
        {
            return it1.value() < it2.value();
        }
        else
        {
            return false;
        }

        return false;
    });
}

///
/// @brief
///     插入行
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月21日，新建函数
///
void QcApplyModel::InsertItems()
{
    // 查找对应需要插入的缓存数据
    auto it = sm_mapPrepInsertBuffer.find(this);
    if (it == sm_mapPrepInsertBuffer.end())
    {
        return;
    }

    // 只对特定行有效
    int iMinRow = m_pTblView->rowAt(0);
    int iMaxRow = m_pTblView->rowAt(m_pTblView->height());
    int iSelRow = -1;
    QModelIndex curIndex = m_pTblView->currentIndex();
    if (curIndex.isValid())
    {
        iSelRow = curIndex.row();
    }

    // 查找行号
    QSet<int> setRemoveRow;
    for (auto itData = it.value().begin(); itData != it.value().end(); itData++)
    {
        // 记录插入成功的行
        int iRow = itData.key();
        if ((iRow < iMinRow || iRow > iMaxRow) && (iRow != iSelRow))
        {
            continue;
        }

        // 插入数据
        insertRow(itData.key(), itData.value());
        m_setInsertRows.insert(itData.key());
        setRemoveRow.insert(iRow);
    }

    // 清空缓存
    for (int iRmRow : setRemoveRow)
    {
        it.value().remove(iRmRow);
    }
}
