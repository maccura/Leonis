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
/// @file     QAssayParamDownloadDlg.h
/// @brief    应用-项目设置-参数下载的对话框
///
/// @author   8276/huchunli
/// @date     2023年8月1日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8276/huchunli，2023年8月1日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#ifndef _INCLUDE_QASSAYPARAMDOWNLOADDLG_H_
#define _INCLUDE_QASSAYPARAMDOWNLOADDLG_H_

#pragma once
#include <QMap>
#include "src/thrift/cloud/gen-cpp/cloud_adapter_types.h"
#include "src/thrift/ch/gen-cpp/ch_types.h"
#include "shared/basedlg.h"

namespace Ui { class QAssayParamDownloadDlg; };
class QStandardItemModel;
class QTableView;
class SortHeaderView;

///
/// @brief	参数下载模型
///
class QParamDownloadModel : public QObject
{
	Q_OBJECT
public:

	// 下载设备类型(界面区分)
	enum DownloadClassify
	{
		DEV_INVALID = 0,
		DEV_IM,					// 免疫
		DEV_CH					// 生化
	};

	// 下载方式
	enum DownloadWay
	{
		OTHER_WAY = 0,
		FILE_WAY,				// 文件
		NET_WAY					// 网络
	};

	QParamDownloadModel(QObject* parent = nullptr);
	~QParamDownloadModel() {};


	///
	/// @brief 初始化模型
	///
	/// @param[in]  downloadDevType  下载类型
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年2月21日，新建函数
	///
	void InitModel(const DownloadClassify& downloadClassify);

	///
	/// @brief 设置参数下载方式
	///
	/// @param[in]  downloadWay  参数下载方式
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年2月21日，新建函数
	///
	void SetDownloadWay(DownloadWay downloadWay);

	///
	/// @brief 读取文件数据
	///
    /// @param[in]  path  文件路径
    /// @param[out]  errInfo  错误信息
	///
	/// @return true:成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年2月27日，新建函数
	///
	bool ReadParamsFileData(const QString& path, QStringList& errInfo);

	///
	/// @brief 获取全部文件导入数据信息
	///
	///
	/// @return 文件导入数据信息
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年2月27日，新建函数
	///
	QMap<int, ::adapter::tf::CloudAssayParam>& GetFileImportInfo();

	///
	/// @brief 获取全部网络下载数据信息
	///
	///
	/// @return 网络下载数据信息
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年2月27日，新建函数
	///
	QMap<int, ::adapter::tf::CloudAssayParam>& GetNetDownloadInfo();

	///
	/// @brief 根据查找索引导入文件参数
	///
	/// @param[in]  indexVec  信息查找索引
	///
	/// @return true:成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年2月27日，新建函数
	///
	bool ImportFileParams(const QVector<int>& indexVec);

	///
	/// @brief 通过项目名称从网络上查询
	///
	/// @param[in]  assayName  项目名称
	///
	/// @return true:查询成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年2月27日，新建函数
	///
	bool SearchNetParams(const QString& assayName);

	///
	/// @brief 通过信息查找索引从网络下载参数
	///
	/// @param[in]  indexVec  信息查找索引
	///
	/// @return true:成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年2月27日，新建函数
	///
	bool DownloadNetParams(const QVector<int>& indexVec);

	///
	/// @brief 获取状态
	///
	///
	/// @return true:正在请求
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年4月8日，新建函数
	///
	bool GetWorkStatus() { return m_isRequesting; };

	///
	/// @brief 获取跳过标识
	///
	///
	/// @return true:有跳过标识
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年4月8日，新建函数
	///
	bool GetSkipFlag() { return m_skipFlag; };

	///
	/// @brief 设置状态
	///
	/// @param[in]  isRequesting  是否正在请求
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年4月8日，新建函数
	///
	void SetWorkStatus(bool isRequesting);

private:

	///
	/// @brief 保存项目参数
	///
	/// @param[in]  info  项目信息
	///
	/// @return true:保存成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年2月27日，新建函数
	///
	bool SaveAssayInfo(const ::adapter::tf::CloudAssayParam& info);

	///
	/// @brief 保存生化项目参数
	///
	/// @param[in]  assayInfo  项目信息
	///
	/// @return true:保存成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年2月27日，新建函数
	///
	bool SaveChAssayInfo(const std::string& assayInfo);

	///
	/// @brief 保存免疫项目参数
	///
	/// @param[in]  assayInfo  项目信息
	///
	/// @return true:保存成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年2月27日，新建函数
	///
	bool SaveImAssayInfo(const std::string& assayInfo);

	///
	/// @brief 通用项目信息表导入
	///		   项目通道号号相同则覆盖，没有则新增
	///
	/// @param[in]  gai  通用项目信息
	///
	/// @return true:导入成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年2月27日，新建函数
	///
	bool ImportGeneralAssayParams(const ::tf::GeneralAssayInfo& gai);

	///
	/// @brief 生化通用项目信息表导入
	///		   项目通道号，仪器类型，版本信息相同则覆盖，没有则新增
	///
	/// @param[in]  cgai  生化通用项目信息
	///
	/// @return true:导入成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年2月27日，新建函数
	///
	bool ImportChGeneralAssayParams(const ch::tf::GeneralAssayInfo& cgai);

	///
	/// @brief 生化特殊项目信息表导入
	///		   项目通道号,仪器类型,参数版本, 样本源类型相匹配则覆盖，没有则新增
	///
	/// @param[in]  csai  生化特殊项目信息
	///
	/// @return true:导入成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年2月27日，新建函数
	///
	bool ImportChSpecialAssayParams(const ch::tf::SpecialAssayInfo& csai);

	///
	/// @brief 将JSON信息转换为项目参数信息
	///
	/// @param[in]  json  源JSON信息
	/// @param[in]  paramStu  参数信息
	///
	/// @return true:转换成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年2月27日，新建函数
	///
	bool DecodeJson2ParamStu(const QString& json, ::adapter::tf::CloudAssayParam& paramStu);

private:
	DownloadClassify							 m_downloadClassify;	///< 当前下载设备
	DownloadWay									 m_downloadWay;			///< 下载方式
	bool										 m_isRequesting;		///< 是否正在请求
	bool										 m_skipFlag;			///< 跳过标识（当存在相同项目信息时，提示跳过）

	QMap<int, ::adapter::tf::CloudAssayParam>	m_fileImportInfo;		///< 文件导入缓存[信息查找索引，参数信息]，表格存储为PARAM_TABLE_INDEX_ROLE
	QMap<int, ::adapter::tf::CloudAssayParam>	m_netDownloadInfo;		///< 网络下载缓存[信息查找索引，参数信息]，表格存储为PARAM_TABLE_INDEX_ROLE

};

///
/// @brief	参数下载界面
///
class QAssayParamDownloadDlg : public BaseDlg
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

    QAssayParamDownloadDlg(QParamDownloadModel::DownloadClassify downloadClassify, QWidget *parent = Q_NULLPTR);
    ~QAssayParamDownloadDlg();

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
	/// @bref
	///		初始化信号槽
	///
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年8月10日，新建函数
	///
	void InitConnect();

	///
	/// @brief
	///     初始化子控件
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年8月10日，新建函数
	///
	void InitChildCtrl();

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

Q_SIGNALS:
	///
	/// @brief 查询信号
	///
	/// @param[in]  isSuccess  是否成功
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年4月8日，新建函数
	///
	void searchResult(bool isSuccess);

	///
	/// @brief 下载信号
	///
	/// @param[in]  isSuccess  是否成功
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年4月8日，新建函数
	///
	void downloadResult(bool isSuccess);

	///
	/// @brief 导入信号
	///
	/// @param[in]  isSuccess  是否成功
	/// @param[in]  firstAssayName  第一个项目名称
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年5月16日，新建函数
	///
	void importResult(bool isSuccess, QString firstAssayName);

protected slots:
    ///
    /// @bref
    ///	对话框底部的按钮显示与隐藏的处理
    ///
    /// @param[in] pageIdx 页索引
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年8月2日，新建函数
    ///
    void OnTabPageChange(int pageIdx);

	///
	/// @bref
	///	路径选择槽函数
	///
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年8月10日，新建函数
	///
	void OnPushButtonFileSelectClicked();

	///
	/// @bref
	///		文件导入项目参数
	///
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年8月10日，新建函数
	///
	void OnPushButtonImportClicked();

	///
	/// @bref
	///		文件导入项目参数
	///
	/// @param[in] index 改变的索引
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年8月10日，新建函数
	///
	void OnComboBoxAssayNameChanged(int index);

	///
	/// @bref
	///		网络查询按钮点击
	///
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年8月10日，新建函数
	///
	void OnPushButtonSearchClicked();

	///
	/// @bref
	///		网络下载按钮点击
	///
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年8月10日，新建函数
	///
	void OnPushButtonDownloadClicked();

    ///
    /// @bref
    ///		清空按钮点击
    ///
    ///
    /// @par History:
    /// @li 1226/ZhangJing，2024年5月15日，新建函数
    ///
    void OnPushButtonClearClicked();

	///
	/// @brief 查询处理接口
	///
	/// @param[in]  isSuccess  是否成功
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年4月8日，新建函数
	///
	void OnSearchHander(bool isSuccess);

	///
	/// @brief 下载处理接口
	///
	/// @param[in]  isSuccess  是否成功
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年4月8日，新建函数
	///
	void OnDownloadHander(bool isSuccess);

	///
	/// @brief 导入处理接口
	///
	/// @param[in]  isSuccess  是否成功
	/// @param[in]  firstAssayName  第一个项目名称
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年5月16日，新建函数
	///
	void OnImportHander(bool isSuccess, QString firstAssayName);
private:
    ///
    /// @bref
    ///		初始化参数表格
    ///
    /// @param[in] tbView 表格控件
    /// @param[in] tbModel 表格模型
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年8月2日，新建函数
    ///
    void InitTableView(QTableView* tbView, QStandardItemModel* tbModel);

	///
	/// @brief 添加一行数据到表格
	///
	/// @param[in]  TableView   表格
	/// @param[in]  TableModel  表格模型
	/// @param[in]  index		信息查找索引
	/// @param[in]  info		数据信息
	///
	/// @return true:成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年2月27日，新建函数
	///
	void AddRowToTableView(QTableView* TableView, QStandardItemModel* TableModel, 
		const int& index, const ::adapter::tf::CloudAssayParam& info);
	
	///
	/// @brief 获取表格中杯选中的信息索引
	///
	/// @param[in]  TableView	表格
	/// @param[in]  TableModel  表格模型
	/// @param[in]  indexVec	返回的信息索引
	///
	/// @return	true:成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年2月27日，新建函数
	///
	bool GetTableViewSelected(QTableView* TableView, QStandardItemModel* TableModel, QVector<int>& indexVec);

	///
	/// @bref
	///		初始化文件下载参数表格数据
	///
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年8月10日，新建函数
	///
	bool InitFileImportTableData();

	///
	/// @bref
	///		初始化网络下载参数表格数据
	///
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年8月10日，新建函数
	///
	bool InitNetImportTableData();

	///
	/// @brief
	///     设置为默认排序
	///
	/// @param[in]  pTableModel  表格控件
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年12月15日，新建函数
	///
	void SetDefaultSort(QStandardItemModel* pTableModel);

	///
	/// @brief 设置下载提示信息
	///
	/// @param[in] isSuccess 是否下载成功
	/// @param[in] noticeInfo 提示信息
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年8月10日，新建函数
	///
	void UpdateNoticeInfo(bool isSuccess, QString noticeInfo);

private:
    Ui::QAssayParamDownloadDlg *ui;
	bool                                        m_bInit;                    ///< 是否已经初始化

    QStandardItemModel*							m_tbModelFileImport;		///< 文件下载
    QStandardItemModel*							m_tbModelNetDownload;		///< 网络下载

	QParamDownloadModel							m_paramDownloadModel;		///< 参数下载模型
};

#endif //_INCLUDE_QASSAYPARAMDOWNLOADDLG_H_
