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

#pragma once

#include <QAbstractTableModel>
#include "boost/optional.hpp"
#include "src/thrift/gen-cpp/defs_types.h"
#include "src/thrift/ch/gen-cpp/ch_types.h"

using DataPair = std::pair<std::shared_ptr<ch::tf::ReagentGroup>, std::shared_ptr<::tf::QcDoc>>;
class ShareClass : public QObject
{
	Q_OBJECT
public:

    ///
    /// @brief 获取质控原因
    ///
    /// @param[in]  reason  质控原因
    ///
    /// @return 原因字符
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年09月05日，新建函数
    ///
    static QString GetQcReason(const int32_t reason);

    ///
    /// @brief 获取质控申请信息
    ///
    /// @param[in]  mode  模式
    /// @param[in]  data  质控文档和复合质控品
    ///
    /// @return 质控申请信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年09月05日，新建函数
    ///
    static boost::optional<ch::tf::QcApply> GetQcApplyInfo(int mode, const DataPair& data);
};

// 自动更新
template<typename T>
struct autoCleaner
{
    autoCleaner(T* appModule)
    {
        m_moudle = appModule;
        (appModule != nullptr) ? appModule->beginResetModel() : void(0);
    }

    ~autoCleaner()
    {
        (m_moudle != nullptr) ? m_moudle->endResetModel() : void(0);
    }

    T* m_moudle;
};

class QQcSampleAppModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum class COLSAMPLE
    {
        MODULE = 0,            ///< 模块
        NAME,				   ///< 项目名称
        SAMPLENAME,            ///< 质控品名称
        LEVEL,                 ///< 质控品水平
        SN,				       ///< 质控品编号
        LOT,				   ///< 质控品批号
        TYPE,			       ///< 质控品类型
        REASON,				   ///< 质控品原因
        SELECTED			   ///< 选择
    };

    enum class MODE
    {
        ONUSE = 0,              ///< 在用试剂选择模式
        DEFAULT                 ///< 默认质控
    };

    QQcSampleAppModel(MODE mode = MODE::ONUSE);
    virtual ~QQcSampleAppModel();

    friend class autoCleaner<QQcSampleAppModel>;

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
    /// @li 5774/WuHongTao，2022年9月1日，新建函数
    ///
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    ///
    /// @brief 设置页面显示关键信息
    ///
    /// @param[in]  mode     模式
    /// @param[in]  devices  设备信息
    /// @param[in]  qcDoc    复合质控品
    ///
    /// @return true表示显示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年8月31日，新建函数
    ///
    bool SetData(MODE mode, std::vector<std::shared_ptr<const tf::DeviceInfo>>& devices, ::tf::QcDoc& qcDoc);

    ///
    /// @brief 获取单例对象
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年8月31日，新建函数
    ///
/*    static QQcSampleAppModel& Instance();*/

protected:
    void Init();
    Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual int rowCount(const QModelIndex &parent) const;
    virtual int columnCount(const QModelIndex &parent) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

private:
    QStringList						m_heads;								///< 列表头
    std::vector<DataPair>           m_data;                                 ///<
    MODE                            m_mode;                                 ///< 当前模式
};

class QQcSpareAppModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    enum class COLSAMPLE
    {
        MODULE = 0,            ///< 模块
        NAME,				   ///< 项目名称
        REAGENTSN,             ///< 试剂瓶号
        REAGENTLOT,            ///< 试剂批号
        SAMPLENAME,            ///< 质控品名称
        LEVEL,                 ///< 质控品水平
        SN,				       ///< 质控品编号
        LOT,				   ///< 质控品批号
        TYPE,			       ///< 质控品类型
        REASON,				   ///< 质控品原因
        SELECTED			   ///< 选择
    };

    QQcSpareAppModel();
    ~QQcSpareAppModel();

    friend class autoCleaner<QQcSpareAppModel>;

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
    /// @li 5774/WuHongTao，2022年9月6日，新建函数
    ///
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    ///
    /// @brief 设置页面显示关键信息
    ///
    /// @param[in]  spdevices设备信息
    /// @param[in]  qcDoc    复合质控品
    ///
    /// @return true表示显示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年9月6日，新建函数
    ///
    bool SetData(std::vector<std::shared_ptr<const tf::DeviceInfo>>& spdevices, ::tf::QcDoc& qcDoc);

protected:
    Qt::ItemFlags flags(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
private:
    QStringList						m_heads;								///< 列表头
    std::vector<DataPair>           m_data;                                 ///<
};
