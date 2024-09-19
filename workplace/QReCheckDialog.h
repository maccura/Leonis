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

#include <QComboBox>
#include <boost/optional.hpp>
#include "shared/basedlg.h"
#include "src/thrift/gen-cpp/defs_types.h"

class QStandardItemModel;

namespace Ui {
    class QReCheckDialog;
    class QReCheckSimpleDialog;
};


class QReCheckDialog : public BaseDlg
{
    Q_OBJECT

public:
    enum class Mode
    {
        simple_mode,    ///< 简单模式
        dilu_mode       ///< 稀释模式
    };

	enum class ASSAYTYPE
	{
		CHEMISTRY,    ///< 生化项目
		IMMUNE,       ///< 免疫项目
		SPECIAL		  ///< 特殊项目（ISE ,LHI ,糖化）
	};

    using ValueType = QMap<ASSAYTYPE, QMap<int, QString>>;
    QReCheckDialog(QWidget *parent = Q_NULLPTR);
    ~QReCheckDialog();

    ///
    /// @brief 设置样本Id
    ///
    /// @param[in]  sampleId  样本Id
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年11月22日，新建函数
    ///
    void SetSample(tf::SampleInfo& sampleInput);

    ///
    /// @brief 设置模式
    ///
    /// @param[in]  mode  模式
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年11月23日，新建函数
    ///
    void SetMode(Mode mode);

    ///
    /// @brief 保存当前数据
    ///
    ///
    /// @return true 保存成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年1月13日，新建函数
    ///
    bool OnSaveData();

protected:
    ///
    /// @brief 初始化
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年11月22日，新建函数
    ///
    void Init();

	bool IsSpecialAssay(int assayCode);

    ///
    /// @brief 获取样本位置
    ///
    ///
    /// @return 样本位置
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年12月27日，新建函数
    ///
    int GetSamplePostion();

    ///
    /// @brief 清除状态
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年1月9日，新建函数
    ///
    void ClearStatus();

	///
	/// @brief 设置项目的稀释属性
	///
	/// @param[in]  testItem  
	///
	/// @return 
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年1月25日，新建函数
	///
	void SetTestItemAttribute(tf::TestItem& testItem, const QComboBox* duliCombox);

	///
	/// @brief 获取comBox根据信息
	///
	/// @param[in]  spTestItem  项目信息
	/// @param[in]  dataMap		类型信息
	///
	/// @return true combox信息
	/// @par History:
	/// @li 5774/WuHongTao，2022年11月23日，新建函数
	///
	QComboBox* GetComBoxFromTestItem(std::shared_ptr<tf::TestItem> spTestItem, const ValueType& dataMap);

private:
    Ui::QReCheckDialog*             ui;
    Ui::QReCheckSimpleDialog*       ui_simple;
    QStandardItemModel*             m_recheckMode;
	std::map<int64_t, std::shared_ptr<tf::TestItem>> m_selectedItems;
    boost::optional<tf::SampleInfo> m_sampleInfo;
    ValueType                       m_dataMap;
    Mode                            m_mode;
	std::vector<QPushButton*>		m_postionButton;
};
