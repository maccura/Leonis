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
/// @file     SampleRackAllocationDlg.h
/// @brief    样本架分配弹窗
///
/// @author   7951/LuoXin
/// @date     2022年9月16日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2022年9月16日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include <QStandardItemModel>
#include "shared/basedlg.h"
#include "src/thrift/gen-cpp/defs_types.h"

struct SampleRackAllocation;
struct RackRange;

namespace Ui {
    class SampleRackAllocationDlg;
};

// 样本架分配的自定义表格
class NumericTableModel : public QStandardItemModel
{
    Q_OBJECT

public:
    NumericTableModel(QObject *parent = nullptr):QStandardItemModel(parent) {}

    virtual ~NumericTableModel() {}

    void SetColunmColor(int row, QColor color);
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

private:
    bool validateInput(const QString& input) const;

protected:
    std::map<int, QColor>            m_mapRowAndColor;          // 设置表头字体颜色的行和对应的颜色
};

class SampleRackAllocationDlg : public BaseDlg
{
	Q_OBJECT

public:
	SampleRackAllocationDlg(QWidget *parent = Q_NULLPTR);
	~SampleRackAllocationDlg();

	///
	/// @brief	加载显示数据
	///     
	/// @par History:
	/// @li 7951/LuoXin，2022年9月16日，新建函数
	///
	void LoadDataToDlg();

protected Q_SLOTS:
	///
	/// @brief
	///     保存按钮被点击
	/// @par History:
	/// @li 7951/LuoXin，2022年9月17日，新建函数
	///
	void OnSaveBtnClicked();

    ///
    /// @brief	更新控件的显示和编辑权限
    ///    
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年3月27日，新建函数
    ///
    void UpdateCtrlsEnabled();

private:
	///
	/// @brief	初始化控件
	///     
	/// @par History:
	/// @li 7951/LuoXin，2022年9月17日，新建函数
	///
	void Initctrls();

	///
	/// @brief	检查当前配置的架号范围是否重复
	///     
	///
	/// @return 不重复返回true
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年9月17日，新建函数
	///
	bool CheckRackRang(const std::vector<RackRange>& vecRackRange);

	///
	/// @brief	初始化样本源数据，用于过滤免疫不支持的样本源
	///     
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年8月11日，新建函数
	///
	void InitSampleSourceTypeVec();

	///
	/// @brief	调整UI显示
	///     
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年8月11日，新建函数
	///
	void AdjustUIDisplay();

    bool GetUserInputData(bool isStat, SampleRackAllocation& sra);

private:
	Ui::SampleRackAllocationDlg*					ui;
    NumericTableModel*								m_tabViewMode;			 // 列表的model
	QVector<::tf::SampleSourceType::type>			m_sampleSourceTypesVec;  // 样本源类型列表，因为用于过滤免疫不支持的类型
};
