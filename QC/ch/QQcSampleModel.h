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
#pragma once
#include <map>
#include <QAbstractTableModel>
#include "src/thrift/gen-cpp/defs_types.h"
#include <boost/optional.hpp>

// 模型对应的类型（生化、免疫
enum QcSampleModelDevType{ QmCh, QmIm};

class QTableView;

// 一行质控数据，其中rowSequence与行号+1对应
struct QcRowData
{
    ///
    /// @bref
    ///		构造
    ///
    /// @param[in] qcData 质控品
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年5月30日，新建函数
    ///
    QcRowData(const boost::optional<tf::QcDoc>& qcData);

    ///
    /// @bref
    ///		构造
    ///
    /// @param[in] seq 序号
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年5月30日，新建函数
    ///
    QcRowData(int seq);

    ///
    /// @bref
    ///		构造
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年5月30日，新建函数
    ///
    QcRowData() {};

    int rowSequence;
    boost::optional<tf::QcDoc> qcItem;
};

class QQcSampleModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum class COLSAMPLE
    {
        SEQNO = 0,            ///< 序号
        SERIALNUMBER,         ///< 质控品编号
        NAME,				  ///< 质控品名称
        ABBREVIATION,         ///< 质控品简称
        TYPE,				  ///< 质控品类型
        LEVEL,                ///< 质控水平
        LOT,				  ///< 质控品批号
        EXPIRED,			  ///< 失效日期
        REGTYPE,              ///< 登记方式
        POS_TUBE,			  ///< 位置和样本杯类型
    };

    ///
    /// @brief 用于质控品筛选
    ///
    struct QcFilter
    {
        ///
        /// @bref
        ///		构造质控过滤条件
        ///
        /// @param[in] nfilter 质控名字的过滤条件
        /// @param[in] regfilter 登记方式的过滤条件
        ///
        /// @par History:
        /// @li 8276/huchunli, 2023年10月10日，新建函数
        ///
        QcFilter(const std::string& nfilter, tf::DocRegistType::type regfilter);

        QcFilter() {}

        ///
        /// @bref
        ///		判断质控品是否满足过滤条件
        ///
        /// @param[in] qcName 质控品名
        /// @param[in] typeReg 质控品登记方式
        ///
        /// @par History:
        /// @li 8276/huchunli, 2023年10月10日，新建函数
        ///
        bool IsFitCondition(const std::string& qcName, tf::DocRegistType::type typeReg);

        ///
        /// @bref
        ///		判断过滤条件是否为空
        ///        
        /// @par History:
        /// @li 8276/huchunli, 2023年10月10日，新建函数
        ///
        inline bool IsEmptyCondition() const { return (m_nameFilter.empty() && m_registerTypeFilter == tf::DocRegistType::REG_EMTPY); }

    private:
        std::string m_nameFilter;
        tf::DocRegistType::type m_registerTypeFilter;
    };

    virtual ~QQcSampleModel();

    // 获取质控名
    inline std::vector<std::string>& GetQcNames() {
        return m_allQcDocNames;
    }
    inline void SetQcFilter(const QcFilter& qcFilter) { m_qcFilter = qcFilter; }

    ///
    /// @brief 设置view
    ///
    /// @param[in]  view  view句柄
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年8月18日，新建函数
    ///
    void SetView(QTableView* view);

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
    boost::optional<::tf::QcDoc> GetDocByIndex(const QModelIndex& qcIndex);

    ///
    /// @bref
    ///		根据当前行号，生成Qc的index字段值
    ///
    /// @param[in] curIndex 当前行号
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年5月11日，新建函数
    ///
    int GenerateQcIndex(const QModelIndex& qcIndex);

    ///
    /// @brief 获取单例对象
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年6月30日，新建函数
    ///
    static QQcSampleModel& Instance();

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
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    ///
    /// @brief 删除多行
    ///
    /// @param[in]  indexs  待删除的行序列集合
    /// @param[out]  failedDelete  删除失败的行序列集合
    /// @return true代表成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年8月23日，新建函数
    ///
    bool removeQcRow(const std::set<int>& qcIndex, std::vector<int>& failedDelete);

    ///
    /// @brief 更新模式里面的数据
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年6月30日，新建函数
    ///
    bool ReloadQcdoc();

    ///
    /// @bref
    ///		更新指定行
    ///
    /// @param[in] qcDocIds 质控品ID集
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年10月25日，新建函数
    ///
    bool Update(const std::vector<int64_t>& qcDocIds);

    ///
    /// @bref
    ///		当删除质控品时，对界面Model的数据更新
    ///
    /// @param[in] deletedQcIndex 被删除数据的行号
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年1月22日，新建函数
    ///
    void UpdateDeletedQcdoc(int deletedQcIndex);

    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;

    ///
    /// @bref
    ///		通过qcIndex字段找到其对应所在的行号
    ///
    /// @param[in] qcIndex 输入的qcIndex
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年5月11日，新建函数
    ///
    int FindRowByQcIndex(int qcIndex);
    int FindRowByIndexName(QString indexName);

    ///
    /// @bref
    ///		通过Index获取打印数据
    ///
    /// @param[in] Index 输入的Index
    ///
    /// @par History:
    /// @li 6889/ChenWei, 2023年12月14日，新建函数
    ///
    QString GetPrintData(const QModelIndex &Index);

protected:
    ///
    /// @brief 初始化质控
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年8月17日，新建函数
    ///
    void Init();

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
    QString GetPosAndCupType(const ::tf::QcDoc& doc) const;

    QVariant data(const QModelIndex &index, int role) const override;

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
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QQcSampleModel();

Q_SIGNALS:
    ///
    /// @brief
    ///     显示数据改变
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年8月7日，新建函数
    ///
    void displayDataChanged();

private:
    std::vector<std::shared_ptr<QcRowData>>	m_qcDocVec;					    ///< 质控文档信息列表
    std::map<int, std::shared_ptr<QcRowData>> m_qcDocIndexSet;              ///< 缓存所有质控品的Index，用于生成新index时唯一性判断
    std::set<int>                   m_allQcDocIndex;                        ///< 缓存所有质控文档号
    std::vector<std::string>        m_allQcDocNames;                        ///< 缓存所有质控文档的名字

    QTableView*						m_tableView;							///< 归属view
    QStringList						m_heads;								///< 列表头
    int								m_focusLine;							///< 当前关注行

    std::map<int, int>              m_filtedRowIndex;                       ///< 筛选过的真实行号与当前行号关系 <qc data index, current real row index>
    QcFilter                        m_qcFilter;                             ///< 质控品名筛选
    std::map<tf::TubeType::type, QPixmap> m_tubePixMap;                     ///< 试管类型位图表

    bool                            m_bMaskExpiredTime;                     // 屏蔽过期时间
};

class QQcAssayModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    enum class COLASSAY
    {
        NAME = 0,			  ///< 项目名称
        TARGETVALUE,          ///< 靶值
        STANDARD,			  ///< 标准差
        CV,					  ///< CV%
        UNIT				  ///< 单位
    };

    ///
    /// @brief 获取单例对象
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年6月30日，新建函数
    ///
    static QQcAssayModel& Instance();

    virtual bool SetData(const std::vector<::tf::QcComposition>& data);
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;

Q_SIGNALS:
    ///
    /// @brief
    ///     显示数据改变
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年8月7日，新建函数
    ///
    void displayDataChanged();

protected:
    ///
    /// @brief 初始化代码
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年8月18日，新建函数
    ///
    virtual bool Init();
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    QQcAssayModel();

    QStringList						   m_heads;								///< 列表头
    std::vector<::tf::QcComposition> m_assayData;							///< 项目列表

protected:
    std::map<int, double> m_compositionCv; ///< 由于tf::QcComposition中没有地方保存CV值，所以在此处缓存CV值，用于界面先输入CV时处理
    QQcAssayModel::COLASSAY m_preEditCol;  ///< 用于记录前编辑位置
};


class QQcAssayEditModel : public QQcAssayModel
{
    Q_OBJECT

public:
    QQcAssayEditModel(QcSampleModelDevType aType) 
        :m_assayType(aType)
    {};
    ~QQcAssayEditModel() {};

    ///
    /// @brief 初始化代码
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年8月18日，新建函数
    ///
    virtual bool Init(const std::vector<int32_t>& assayCode);

    bool SetData(const std::vector<::tf::QcComposition>& data);

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
    virtual bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    ///
    /// @brief 获取选择的项目成分
    ///
    ///
    /// @return 项目成分列表
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年8月22日，新建函数
    ///
    void GetCompostion(std::vector<::tf::QcComposition>& outComp, std::vector<std::string>& notComplate);

Q_SIGNALS:
    ///
    /// @brief
    ///     显示数据改变
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年8月7日，新建函数
    ///
    void displayDataChanged();

protected:

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
    Qt::ItemFlags flags(const QModelIndex &index) const override;

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
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    bool AssignTargetValue(tf::QcComposition& assayData, const QVariant& vTargetValue, int iRow);
    bool AssignStandardValue(tf::QcComposition& assayData, const QVariant& vStandardValue, int iRow);
    bool AssignCvValue(tf::QcComposition& assayData, const QVariant& vCvValue, int iRow);

private:
    QcSampleModelDevType m_assayType; // 区分生免
};
