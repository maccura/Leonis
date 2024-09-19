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
/// @file     QCaliRackOverview.h
/// @brief    校准架概况（只需要显示校准设置页面登记了位置的校准品信息，和试剂是否在机无关，和试剂是否被选择有关）
///
/// @author   8090/YeHuaNing
/// @date     2022年9月13日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8090/YeHuaNing，2022年9月13日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include "boost/optional.hpp"
#include "shared/basedlg.h"
#include "src/thrift/ch/gen-cpp/ch_types.h"
#include "src/thrift/gen-cpp/defs_types.h"
#include "src/thrift/im/gen-cpp/im_types.h"
#include "src/thrift/ise/gen-cpp/ise_types.h"
#include "PrintExportDefine.h"
#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include <QList>

namespace Ui {
    class QCaliRackOverview;
};


struct CaliRackDetails
{
    CaliRackDetails() : m_concL1(-1), m_concL2(-1), m_concL3(-1), m_concL4(-1),
        m_concL5(-1), m_concL6(-1), m_assayCode(-1)/*, m_bApp(false)*/, m_bShied(false){}

    int64_t                         m_groupId;                   // 数据库主键
    double                          m_concL1;                   // 浓度1(小于0表示未设置，不用显示)
    double                          m_concL2;                   // 浓度2(小于0表示未设置，不用显示)
    double                          m_concL3;                   // 浓度3(小于0表示未设置，不用显示)
    double                          m_concL4;                   // 浓度4(小于0表示未设置，不用显示)
    double                          m_concL5;                   // 浓度5(小于0表示未设置，不用显示)
    double                          m_concL6;                   // 浓度6(小于0表示未设置，不用显示)
    int                             m_assayCode;                // 项目编号
    //bool                            m_bApp;                     // 项目是否勾选校准申请
    bool                            m_bShied;                   // 项目是否在校准品组中被屏蔽
    QString                         m_grpName;                  // 校准品名称
    QString                         m_lot;                      // 校准品批号
    QString                         m_exprite;                  // 失效日期
    QString                         m_rackL1;                   // 位置1
    QString                         m_rackL2;                   // 位置2
    QString                         m_rackL3;                   // 位置3
    QString                         m_rackL4;                   // 位置4
    QString                         m_rackL5;                   // 位置5
    QString                         m_rackL6;                   // 位置6
    
};


class QCaliRackModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    QCaliRackModel(QObject* parent = nullptr);
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    ///
    /// @brief 获取指定索引的 项目名称和是否已经申请校准
    ///
    /// @param[in]  index  索引
    ///
    /// @return 项目名称和是否已经申请校准
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年4月27日，新建函数
    ///
    boost::optional<std::tuple<QString, bool>> GetNameAndIsApp(int sourceRow);

    ///
    /// @brief 获取单元格合并信息
    ///
    ///
    /// @return 合并信息（x:行号   y:需要合并的行数）
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年7月14日，新建函数
    ///
    QList<QPoint> GetTableSpanInfos();

    ///
    /// @brief 更新数据
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年4月27日，新建函数
    ///
    void UpdateModelData();

    ///
    /// @brief 获取已有的项目
    ///
    ///
    /// @return 已有的项目名称
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年5月11日，新建函数
    ///
    const QList<QString> GetAssayNames();

    ///
    /// @brief 更新表格合并
    ///
    /// @param[in]  isApp  是否已选择
    /// @param[in]  filterName  过滤名称
    ///
    ///
    /// @return 返回需要合并的单元格计算列表
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年7月25日，新建函数
    ///
    QVector<int> UpdateTableSpan(bool isApp, const QString& filterName);

	///
	/// @brief 根据表格行号，获取对应行的显示数据
	///
	/// @param[in]  rowInx  表格行号
	///
	/// @return 详细信息
	///
	/// @par History:
	/// @li 8090/YeHuaNing，2023年7月17日，新建函数
	///
    boost::optional<CaliRackDetails> GetCaliRackDetailByRow(int row) const;

    ///
    /// @brief 是否存在数据
    ///
    /// @return true: 表示存在
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年12月22日，新建函数
    ///
    bool HasRackDatas() { return !(m_rackDetails.empty()); }

public slots:
    ///
    /// @brief 更新试剂的校准状态信息
    ///  
    /// @param[in]  supplyUpdates  位置信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年6月20日，新建函数
    ///
    void UpdateCaliSatus(class std::vector<class ch::tf::SupplyUpdate, class std::allocator<class ch::tf::SupplyUpdate>> supplyUpdates);
    void UpdateIseCaliSatus(std::vector<class ise::tf::IseModuleInfo, class std::allocator<class ise::tf::IseModuleInfo>> iseAppUpdates);
    void UpdateImCaliStatus(const im::tf::ReagentInfoTable & stuRgntInfo, const im::tf::ChangeType::type type);


    ///
    /// @brief 刷新数据
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年4月27日，新建函数
    ///
    void RefreshData();

    ///
    /// @brief 当校准设置界面数据发生变化时
    ///
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年8月30日，新建函数
    ///
    void OnImCaliSetUpdated(tf::UpdateType::type upType, im::tf::CaliDocGroup group);

    ///
    /// @brief 生化校准品组变更响应
    ///
    /// @param[in]  upType  更新方式
    /// @param[in]  group  变更的校准品组
    /// @param[in]  docs  变更的校准文档
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2024年7月3日，新建函数
    ///
    void OnChCaliSetUpdated(tf::UpdateType::type upType, ch::tf::CaliDocGroup group, QList<ch::tf::CaliDoc> docs);

    ///
    /// @brief 项目数据发生变化
    ///
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2024年6月21日，新建函数
    ///
    void OnAssayUpdated();

protected:
    ///
    /// @brief 获取在机处于选择状态的试剂编号
    ///
    ///
    /// @return 返回是否发生了表更(true：已变更）
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年4月25日，新建函数
    ///
    bool UpdateChSelectedAssay();
    bool UpdateImSelectedAssay();
    bool UpdateIseSelectedAssay();

    ///
    /// @brief 获取校准架信息
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年4月25日，新建函数
    ///
    void UpdateChCaliRackInfos();
    void UpdateImCaliRackInfos();

    ///
    /// @brief 更新校准架详情
    ///
    /// @param[in]  doc  校准文档
    /// @param[in]  grp  校准品组
    /// @param[out]  details  详细信息
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年4月26日，新建函数
    ///
    void UpdateChRackDetails(const ch::tf::CaliDoc& doc, const ch::tf::CaliDocGroup& grp, QMap<int, CaliRackDetails>& details);

    ///
    /// @brief 更新校准架详情
    ///
    /// @param[in]  grp  免疫校准品组信息
    /// @param[out]  details  详细信息
    ///
    /// @return true 表示至少设置了一个架号位置
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年10月28日，新建函数
    ///
    bool UpdateImRackDetails(const im::tf::CaliDocGroup& grp, QMap<int, CaliRackDetails>& details);

    ///
    /// @brief 获取校准架概况一行数据
    ///
    /// @param[in]  c  项目浓度信息
    /// @param[in]  grp  校准品组信息
    /// @param[out]  details  获取到的一行数据
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2024年7月3日，新建函数
    ///
    bool GetChCaliRackDetail(const ch::tf::CaliDoc & doc, const ch::tf::CaliComposition& c, const ch::tf::CaliDocGroup & grp, CaliRackDetails& detail);
    bool GetImCaliRackDetail(const im::tf::CaliDoc& c, const im::tf::CaliDocGroup & grp, CaliRackDetails& detail);

    ///
    /// @brief 
    ///
    /// @param[in]  mapGroupAssayCache  校准品组项目缓存
    /// @param[in]  detailFromCaliGroup  项目对应的校准架概况图表
    /// @param[in]  groupId  数据库主键
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2024年7月3日，新建函数
    ///
    void AddRackDetails(QMap<int64_t, QSet<int>>& mapGroupAssayCache, const QMap<int, CaliRackDetails>& detailFromCaliGroup, int64_t groupId);

    ///
    /// @brief 
    ///
    /// @param[in]  mapGroupAssayCache  校准品组项目缓存
    /// @param[in]  groupId  数据库主键
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2024年7月3日，新建函数
    ///
    void DeleteRackDetails(QMap<int64_t, QSet<int>>& mapGroupAssayCache, int64_t groupId);

    ///
    /// @brief 
    ///
    /// @param[in]  mapGroupAssayCache  校准品组项目缓存
    /// @param[in]  detailFromCaliGroup  项目对应的校准架概况图表
    /// @param[in]  groupId  数据库主键
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2024年7月3日，新建函数
    ///
    void ModifyRackDetails(QMap<int64_t, QSet<int>>& mapGroupAssayCache, const QMap<int, CaliRackDetails>& detailFromCaliGroup, int64_t groupId);

signals:
    void sigModelUpdated();

private:
    QMap<int, QSet<QString>>        m_appAssays;                ///< 缓存每个被选择过项目，哪些位置被选中了（选中位置的数据库主键）
    QMap<QString, QSet<int>>        m_posIndex;                 ///< 缓存每个被选择的位置，有哪些项目编号（防止双向同侧） key: devenSn|pos value：此位置被选择的项目编号
    QMap<int,QList<CaliRackDetails>> m_rackDetails;             ///< 所有校准架汇总信息 K:项目, V:项目的所有校准品组信息合计
    QMap<int64_t, QSet<int>>        m_mapChGroupAssays;         ///< 缓存生化校准品组中有哪些项目（用于快速更新）
    QMap<int64_t, QSet<int>>        m_mapImGroupAssays;         ///< 缓存生化校准品中有哪些项目（用于快速更新）
    int                             m_rowCnt;                 ///< 表格行数量
    QStringList                     m_hTitle;                   ///< 表格标题
    QModelIndex                     m_topLeftIndex;             ///< 左上角
    QModelIndex                     m_buttomRightIndex;         ///< 右下角
    bool                            m_bExImDev;                 ///< 存在免疫设备
    bool                            m_bExChDev;                 ///< 存在生化设备
    bool                            m_bExIseDev;                ///< 存在电解质设备
};

class QCaliRackFilter : public QSortFilterProxyModel
{
public:
    QCaliRackFilter(QObject* parent = nullptr) : QSortFilterProxyModel(parent), m_bFilterApp(false){}
    ~QCaliRackFilter() {};

    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

    void SetFilterApp(bool app) { m_bFilterApp = app; }
    void SetFilterName(const QString& name) { m_filterName = name; }

    bool GetFilterApp() const { return m_bFilterApp; }
    QString GetFilterName() const { return m_filterName; }

private:
    bool                            m_bFilterApp;               ///< 是否显示已选择 （true：已选择， false：全部）
    QString                         m_filterName;               ///< 过滤的名称
    
};

class QCaliRackOverview : public BaseDlg
{
    Q_OBJECT

public:
	QCaliRackOverview(QWidget *parent = Q_NULLPTR);
    ~QCaliRackOverview();

public slots:

    ///
    /// @bref
    ///		权限变更响应
    ///
    /// @par History:
    /// @li 8090/YeHuaNing, 2024年01月15日，新建函数
    ///
    void OnPermisionChanged();

    ///
    /// @brief 响应搜索框选择项变化
    ///
    /// @param[in]  text  变化后的文字
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年5月11日，新建函数
    ///
    void OnSearchIndexChanged(const QString& text);

protected slots :

	///
	/// @brief 打印按钮被点击
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年9月8日，新建函数
	///
	void OnPrintBtnClicked();

	///
	/// @brief 导出按钮被点击
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年9月8日，新建函数
	///
	void OnExportBtnClicked();

private:
    ///
    /// @brief 初始化表标题
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年9月14日，新建函数
    ///
    void InitTableTitle();

    ///
    /// @brief 初始化信号连接
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年4月27日，新建函数
    ///
    void InitConnect();

    ///
    /// @brief 设置项目筛选列表
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年7月14日，新建函数
    ///
    void SetAssayFilterList();

    ///
    /// @brief 设置表格合并
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年7月14日，新建函数
    ///
    void SetTableViewSpan();

	///
	/// @brief 设置校准架信息（导出使用）
	///
	/// @param[in] strExportTextList   导出的数据
	///
	/// @return true:设置成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年9月8日，新建函数
	///
	bool SetCaliRackInfo(QStringList& strExportTextList);

    ///
    /// @brief 设置打印导出是否可用
    ///
    /// @param[in]  hasCurve  是否存在可用曲线
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年12月22日，新建函数
    ///
    void SetPrintAndExportEnabled(bool enabled);

    ///
    /// @brief 获取校准架概况打印数据（打印使用）
    ///
    /// @param[in] caliRackInfo   校准架概况打印数据
    ///
    /// @return true:获取成功
    ///
    /// @par History:
    /// @li 6889/ChenWei，2023年12月13日，新建函数
    ///
    bool GetCaliRackPrintData(CaliRackInfo& caliRackInfo, bool isPrint);

private:

	Ui::QCaliRackOverview*			ui;
    QCaliRackModel*					m_tableModel;				///< 表数据
    QCaliRackFilter*                m_filterModel;              ///< 筛选器
};
