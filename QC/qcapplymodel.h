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
/// @file     qcapplymodel.h
/// @brief    质控申请模型
///
/// @author   4170/TangChuXian
/// @date     2024年3月20日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月20日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once

#include "uidcsadapter/adaptertypedef.h"
#include <QStandardItemModel>
#include <QList>
#include <QSet>

// 前置声明
class       QTimer;             // 定时器
class       QTableView;         // 表格视图
class       QTimer;             // 定时器

class QcApplyModel : public QStandardItemModel
{
    Q_OBJECT

public:
    // 模型类型
    enum QcApplyType
    {
        QC_APPLY_TYPE_ON_USE = 0,
        QC_APPLY_TYPE_DEFAULT,
        QC_APPLY_TYPE_BACKUP
    };

public:
    QcApplyModel(QObject *parent, QcApplyType enQcType = QC_APPLY_TYPE_ON_USE);
    ~QcApplyModel();

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
    //QStandardItem* item(int row, int column = 0) const;

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
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

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
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

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
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

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
    QVariant OnUseQcData(const QModelIndex &index, int role = Qt::DisplayRole) const;

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
    QVariant DefaultQcData(const QModelIndex &index, int role = Qt::DisplayRole) const;

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
    QVariant BackupQcData(const QModelIndex &index, int role = Qt::DisplayRole) const;

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
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

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
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    
    ///
    /// @brief
    ///     设置选中ID
    ///
    /// @param[in]  strSelID  选中ID
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年3月23日，新建函数
    ///
    inline void SetSelQcApplyId(const QString& strSelID) 
    {
        m_strSelID = strSelID;
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
    void SetQcNameFilter(const QString& strQcNameFilter);

protected:
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
    void sort(int column, Qt::SortOrder order = Qt::AscendingOrder) override;

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
    void sortOnUseQcApplyInfo(int column, Qt::SortOrder order = Qt::AscendingOrder);

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
    void sortDefaultQcApplyInfo(int column, Qt::SortOrder order = Qt::AscendingOrder);

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
    void sortBackupQcApplyInfo(int column, Qt::SortOrder order = Qt::AscendingOrder);

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
    bool LoadOneRowOnUseQcApplyInfo(int iRow, QList<QStandardItem *>& rowBuffer) const;

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
    bool LoadOneRowDefaultQcApplyInfo(int iRow, QList<QStandardItem *>& rowBuffer) const;

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
    bool LoadOneRowBackupQcApplyInfo(int iRow, QList<QStandardItem *>& rowBuffer) const;

    ///
    /// @brief
    ///     更新布局
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年3月22日，新建函数
    ///
    void UpdateLaout();

    ///
    /// @brief
    ///     
    ///
    /// @param[in]  pView  
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年3月22日，新建函数
    ///
    inline void SetView(QTableView* pView) 
    {
        m_pTblView = pView;
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
    bool SetApplyInfoByRow(int iRow, const QC_APPLY_INFO& stuQcApplyInfo);

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
    bool GetApplyInfoByRow(int iRow, QC_APPLY_INFO& stuQcApplyInfo);

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
    bool IsMatchFilter(const QC_APPLY_INFO& stuQcApplyInfo) const;

    ///
    /// @brief
    ///     记录原序
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年4月24日，新建函数
    ///
    void RecordOldOrder();

    ///
    /// @brief
    ///     恢复原序
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年4月24日，新建函数
    ///
    void RecoverOldOrder();

protected Q_SLOTS:
    ///
    /// @brief
    ///     插入行
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年3月21日，新建函数
    ///
    void InsertItems();

private:
    // 质控申请列表，表头枚举
    enum QcApplyHeader {
        Qah_Module = 0,         // 模块
        Qah_AssayName,          // 项目名称
        Qah_QcNo,               // 质控品编号
        Qah_QcName,             // 质控品名称
        Qah_QcBriefName,        // 质控品简称
        Qah_QcType,             // 质控品类型
        Qah_QcLevel,            // 质控品水平
        Qah_QcLot,              // 质控品批号
        Qah_QcReason,           // 质控品原因
        Qah_Selected,           // 选择
        Qah_DbIndex             // 数据ID
    };

    /// 默认质控列表，表头枚举
    enum DefaultQcHeader {
        Dqh_Module = 0,         // 模块
        Dqh_AssayName,          // 项目名称
        Dqh_QcNo,               // 质控品编号
        Dqh_QcName,             // 质控品名称
        Dqh_QcBriefName,        // 质控品简称
        Dqh_QcType,             // 质控品类型
        Dqh_QcLevel,            // 质控品水平
        Dqh_QcLot,              // 质控品批号
        Dqh_ExpDate,            // 失效日期
        Dqh_Selected,           // 选择
        Dqh_DbIndex             // 数据ID
    };

    // 备用瓶列表，表头枚举
    enum BackupRgntQcHeader {
        Bqh_Pos = 0,            // 位置
        Bqh_AssayName,          // 项目名称
        Bqh_RgntLot,            // 试剂批号
        Bqh_RgntNo,             // 试剂瓶号
        Bqh_UseStatus,          // 使用状态
        Bqh_QcNo,               // 质控品编号
        Bqh_QcName,             // 质控品名称
        Bqh_QcBriefName,        // 质控品简称
        Bqh_QcType,             // 质控品类型
        Bqh_QcLevel,            // 质控品水平
        Bqh_QcLot,              // 质控品批号
        Bqh_ExpDate,            // 失效日期
        Bqh_Selected,           // 选择
        Bqh_DbIndex             // 数据ID
    };

private:
    // 准备插入的缓存数据
    static QMap<QcApplyModel*, QMap<int, QList<QStandardItem *>>> sm_mapPrepInsertBuffer;

    bool                    m_bOldERder;            // 是否是原序
    const QcApplyType       m_cenQcType;            // 质控类型
    QList<QC_APPLY_INFO>    m_lstDataBuffer;        // 数据缓存
    QMap<QString, int>      m_mapOldOrder;          // 原序记录
    QSet<int>               m_setInsertRows;        // 插入行集合

    // 表格
    QString                 m_strSelID;             // 选中ID
    QString                 m_strQcNameFilter;      // 质控名称过滤
    QTableView*             m_pTblView;             // 表格视图
    QTimer*                 m_pItemUpdateTimer;     // 缓存刷新

    // 友元类
    friend class            QcApplyWidget;          // 质控申请界面
    friend class            DefaultQcDlg;           // 声明友元类
    friend class            BackupRgntQcDlg;        // 备用瓶质控
};
