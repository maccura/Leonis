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
/// @file     AssayListModel.h
/// @brief    项目列表模型（应用->分析参数）
///
/// @author   6950/ChenFei
/// @date     2022年5月16日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 6950/ChenFei，2022年5月16日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include <QAbstractTableModel>

class QUtilitySortFilterProxyModel;
class QTableView;

#define	INVALID_ASSAY_CODE			(-1)						/// 无效的项目代号


class AssayListModel : public QAbstractTableModel
{
	Q_OBJECT

public:
	///
	/// @brief
	///     项目列表列定义
	///
	enum class ASSAY_LIST_COL
	{
		INDEX = 0,                  /// 序号
		NAME,                       /// 项目名
		MODULE,                     /// 模块名
		SAMPLE_TYPE,                /// 样本源
		COUNT                       /// 列数
	};

	///
	/// @brief
	///     显示数据结构
	///
	struct StAssayListRowItem
	{
		int32_t assayCode = INVALID_ASSAY_CODE;/// 项目代号
        QString name;                          /// 项目名
        int32_t model;                         /// 设备类型
        int32_t sampleType;                    /// 样本源
		QString version;			           /// 参数版本
        int32_t assayClassify;                 /// 项目类别
        
        std::string extractData;               /// 额外携带的数据
											   
        /// @brief
        ///     重载operator==操作符
        ///
		bool operator==(const StAssayListRowItem& rhs) const
		{
			return this->assayCode == rhs.assayCode && this->name == rhs.name && this->model == rhs.model 
                && this->version == rhs.version && this->sampleType == rhs.sampleType;
		}
	};

public:
	AssayListModel(QTableView* pView, QObject *parent = nullptr);

	virtual ~AssayListModel();

    ///
    /// @brief
    ///     设置数据到模型
    ///
    ///@param[in]    data 显示数据
    ///
    /// @par History:
    /// @li 6950/ChenFei，2022年05月16日，新建函数
    ///
	void SetData(const std::vector<StAssayListRowItem>& data);

    ///
    /// @brief
    ///     获取数据
    ///
    /// @return 被显示的数据
    ///
    /// @par History:
    /// @li 6950/ChenFei，2022年05月16日，新建函数
    ///
	const std::vector<StAssayListRowItem>& GetData();

    ///
    /// @brief
    ///     设置当前选中行的版本参数
    ///
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年12月21日，新建函数
    ///
    void SetRowData(int row, const StAssayListRowItem& rowData);
    int GetRowAssayCode(int row);

    ///
    /// @brief  设置排序模型
    ///  
    inline void SetSortModel(QUtilitySortFilterProxyModel* sortModel) { m_sortModel = sortModel; }

    ///
    /// @brief
    ///     设置当前选中行的样本类型
    ///
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年12月21日，新建函数
    ///
    void SetCurrentRowItemSampleSourceType(int row, int SampleSourceType);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

protected:

	int columnCount(const QModelIndex &parent) const override;
	QVariant data(const QModelIndex &index, int role) const override;
	QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
	QStringList                     m_headerNames;              /// 表头名称集合

    QTableView*                     m_tbView;                   /// Model对应的View
    QUtilitySortFilterProxyModel*   m_sortModel;                /// 表格排序的model
	std::vector<StAssayListRowItem>	m_data;                     /// 显示的数据
};
