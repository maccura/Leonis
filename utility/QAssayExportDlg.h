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
/// @file     QAssayExportDlg.h
/// @brief    项目导出，用户不可见，此功能内部使用
///
/// @author   8580/GongZhiQiang
/// @date     2023年8月10日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月10日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include "shared/basedlg.h"
#include "model/AssayListModel.h"
#include "src/thrift/ise/gen-cpp/ise_types.h"
#include "src/thrift/ch/gen-cpp/ch_types.h"
#include "src/thrift/im/gen-cpp/im_types.h"
#include <QDateTime>

namespace Ui {
    class QAssayExportDlg;
};
class QStandardItemModel;


class QAssayExportDlg : public BaseDlg
{
	Q_OBJECT

public:
	
	enum ASSAY_INFO_COLUNM		    // 项目信息列表
	{
		COLUNM_ASSAY_CHECKBOX = 0,	// 勾选框
		COLUNM_ASSAY_NAME,			// 项目名称
		COLUNM_ASSAY_SAMPLETYPE,	// 样本源类型
		COLUNM_ASSAY_VERSION,		// 参数版本
	};

	// 生化项目信息
	struct ChAssayInfoSpStu
	{
		std::shared_ptr<::tf::GeneralAssayInfo>		spGA   = nullptr;		// 通用项目信息指针
		std::shared_ptr<ch::tf::GeneralAssayInfo>	spChGA = nullptr;		// 生化通用项目信息指针
		std::shared_ptr<ch::tf::SpecialAssayInfo>	spChSA = nullptr;		// 生化特殊项目信息指针

		// 是否不合法
		bool IsInvalid()
		{
			if (spGA == nullptr || spChGA == nullptr || spChSA == nullptr)
			{
				return true;
			}

			return false;
		}
	};

public:
	QAssayExportDlg(QWidget *parent = Q_NULLPTR);
	~QAssayExportDlg();

	
	///
	/// @brief 加载项目信息的数据
	///
	/// @param[in]  assayData  项目信息数据
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年2月24日，新建函数
	///
	void LoadAssayData(const std::vector<AssayListModel::StAssayListRowItem>& assayData);

protected:
	///
	/// @brief
	///     界面显示前初始化
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年8月10日，新建函数
	///
	void InitBeforeShow();

	///
	/// @brief
	///     界面显示后初始化
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年8月10日，新建函数
	///
	void InitAfterShow();

	///
	/// @brief
	///     初始化信号槽连接
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年8月10日，新建函数
	///
	void InitConnect();

	///
	/// @brief
	///     窗口显示事件
	///
	/// @param[in]  event  事件对象
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年8月10日，新建函数
	///
	void showEvent(QShowEvent *event);

protected Q_SLOTS:
	///
    /// @brief
    ///     导出文件位置选择
    ///
    /// @par History:
    /// @li 8580/GongZhiQiang，2023年8月10日，新建函数
    ///
    void OnExportPathBtnClicked();

	///
	/// @brief
	///     确认按钮选择
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年8月10日，新建函数
	///
	void OnExportEnsureClicked();

private:
	
	///
	/// @brief 往表格中添加一行数据
	///
	/// @param[in]  index		 查询索引
	/// @param[in]  assayName    项目名称
	/// @param[in]  sampleType   样本类型
	/// @param[in]  version      参数版本
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年2月24日，新建函数
	///
	void AddRowToTable(const int& index, const QString& assayName, const QString& sampleType, const QString& version);

	///
	/// @brief 初始化导出数据，并进行编码
	///
    /// @param[in]  isEncryption	是否对数据加密
	///
	/// @return true：成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年2月24日，新建函数
	///
	bool InitExportData();

	///
	/// @brief 编码参数
	///
	/// @param[in]  indexVec	界面勾选的数据<数据索引，表格行索引>
	///
	/// @return true：编码成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年2月24日，新建函数
	///
	bool EncodeParams(const QVector<int>& indexVec);

	///
	/// @brief
	///     导出参数结构体
	///
	/// @param[in]  filePath   文件路径
	///
	/// @return 成功返回true
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年8月10日，新建函数
	///
	bool ExportAssayParamsToFile(const QString& filePath);

private:
	Ui::QAssayExportDlg* ui;
	bool                                        m_bInit;                    ///< 是否已经初始化
	QStandardItemModel*							m_assayTableModel;			///< 项目选择表格

	QMap<int, ChAssayInfoSpStu>					m_chAssaySpMap;				///< Ch项目信息映射

	QVector<QByteArray>							m_assayInfoEncryptionJsonVec;///< 加密的项目信息JSON列表(必须用QByteArray,因为QString与std::string会有结尾符问题)

    QVector<QByteArray>							m_assayInfoJsonVec;			///< 未加密的项目信息JSON列表(必须用QByteArray,因为QString与std::string会有结尾符问题)
};
