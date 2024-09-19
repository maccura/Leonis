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
/// @file     AddAssayDlg.h
/// @brief    项目追加弹窗
///
/// @author   7951/LuoXin
/// @date     2022年8月16日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2022年8月16日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include "shared/basedlg.h"
#include "src/thrift/gen-cpp/defs_types.h"
#include "src/thrift/ch/gen-cpp/ch_constants.h"

namespace Ui {
    class AddAssayDlg;
};

// 开放项目校验信息
struct OpenAssayCodeCheck;

// 追加项目信息
struct AddAssagData
{
public:
	std::string					strAssayName;					// 项目名称
	int							iAssayCode = -1;				// 项目编号
	std::string					strUnit;						// 结果单位
	std::vector<int>			sampleSources;					// 样本源类型列表
    std::string                 version = "1";                  // 参数版本

	bool operator==(const AddAssagData& rhs)
	{
		return this->strAssayName == rhs.strAssayName
			&& this->iAssayCode == rhs.iAssayCode
			&& this->strUnit == rhs.strUnit
			&& this->sampleSources == rhs.sampleSources
            && this->version == rhs.version;
	}
};

class AddAssayDlg : public BaseDlg
{
	Q_OBJECT

public:
	AddAssayDlg(QWidget *parent = Q_NULLPTR);
	~AddAssayDlg();

	///
	/// @brief
	///     清空界面上出项目信息列表外的信息
	/// @par History:
	/// @li 7951/LuoXin，2022年8月22日，新建函数
	///
	void ClearCtrls();

	///
	/// @brief
	///     加载项目信息到表格
	///
	/// @param[in]  VecsData  项目信息
	/// @par History:
	/// @li 7951/LuoXin，2022年8月16日，新建函数
	///
	void LoadDataToDlg(AddAssagData data);

    ///
    /// @brief
    ///     获取用户修改的数据
    ///
    /// @return 是否异常，false：异常
    ///
    /// @par History:
    /// @li 7951/LuoXin，2022年8月19日，新建函数
    ///
    bool GetModifyData(AddAssagData& data);

    ///
    /// @brief
    ///     获取保存按钮的指针
    /// @par History:
    /// @li 7951/LuoXin，2024年2月21日，新建函数
    ///
    QPushButton* GetSaveBtnPtr();

protected slots:

	///
	/// @brief 当校验码输入后，主动解析，并填充信息
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年5月22日，新建函数
	///
	void OnCheckCodeFinished();

    ///
    /// @brief 更新样本源类型勾选框
    ///
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年6月12日，新建函数
    ///
    void OnUpdateCheckBox();

    ///
    /// @bref
    ///		权限变化响应
    ///
    /// @par History:
    /// @li 7951/LuoXin, 2023年10月31日，新建函数
    ///
    void OnPermisionChanged();
private:

	///
	/// @brief
	///     检查用户输入的数据是否合法
	///
	/// @return		数据合法返回true;
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年8月22日，新建函数
	///
	bool CheckUserData();

	///
	/// @brief
	///     保存开放项目记录
	///
	/// @param[in]  assayCode  项目通道号
	/// @param[in]  checkCode  校验码
	///
	/// @return	成功返回true;
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年12月6日，新建函数
	///
	bool SaveOpenAssayRecord(const int& assayCode, const QString& checkCode);

	///
	/// @brief
	///     检查开放项目校验码
	///
	/// @return		数据合法返回true;
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年12月6日，新建函数
	///
	bool CheckCode(QString& info);

	///
	/// @brief 获取开放项目校验码信息
	///
	/// @param[in]  code  校验码
	///
	/// @return 校验码信息
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年5月22日，新建函数
	///
	OpenAssayCodeCheck GetCheckCodeInfo(const std::string& code);

private:
	Ui::AddAssayDlg*								ui;
	bool											m_aCRecorded;		///< 开放项目编号是否已经有记录
};
