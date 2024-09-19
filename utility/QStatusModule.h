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

#include <QWidget>
#include <QStandardItemModel>
#include "PrintExportDefine.h"

namespace tf
{
	class UnitStatusInfo;
	class TemperautureInfo;
	class LiquidPathInfo;
	class UnitInfo;
	class CountInfo;
}

namespace Ui {
    class QStatusModule;
};

class ResetCountInfoDlg;

using namespace std;

class QStatusModule : public QWidget
{
    Q_OBJECT

public:
    QStatusModule(QWidget *parent = Q_NULLPTR);
    ~QStatusModule();
    void Init();

protected:
    ///
    /// @brief 窗口显示事件
    ///     
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年8月10日，新建函数
    ///
    void showEvent(QShowEvent *event);

private:

	///
	/// @brief  更新温度信息
	///
	/// @param[in]    devName:设备名称
	/// @param[in]    temperautureInfo:温度信息
	/// 
	/// @return void
	///
	/// @par History:
	/// @li 7951/LuoXin，2023年2月13日，新建函数
	///
	void UpdateTemperautureInfo(const QString& devName, const std::vector<tf::TemperautureInfo>& temperautureInfo);

    ///
    /// @brief  设置液路信息单元格
    ///     
    /// @param[in]  name    液路信息名称 
    /// @param[in]  status  状态
    /// @param[in]  status  单元格
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年7月6日，新建函数
    ///
    void SetLiquidPathInfoItem(const QString& name, bool status, QStandardItem* item);

	///
	/// @brief  更新液路信息
	///
	/// @param[in]    devName:设备名称
	/// @param[in]    liquidPathInfo:液路信息
	/// 
	/// @return void
	///
	/// @par History:
	/// @li 7951/LuoXin，2023年2月13日，新建函数
	///
	void UpdateLiquidPathInfo(const QString& devName, const std::vector<tf::LiquidPathInfo>& liquidPathInfo);

	///
	/// @brief  更新单元信息
	///
	/// @param[in]    devName:设备名称
	/// @param[in]    unitInfo:单元信息
	/// 
	/// @return void
	///
	/// @par History:
	/// @li 7951/LuoXin，2023年2月13日，新建函数
	///
	void UpdateUnitInfo(const QString& devName, const std::vector<tf::UnitInfo>& unitInfo);

	///
	/// @brief  更新计数信息
	///
	/// @param[in]    devName:当前选中设备名称
	/// @param[in]    unitName:当前选中单元名称
	/// 
	/// @return void
	///
	/// @par History:
	/// @li 7951/LuoXin，2023年2月13日，新建函数
	///
	void UpdateCountInfo(const QString& devName, const QString& unitName);

    ///
    /// @brief  获取温度日志
    /// 
    /// @return void
    ///
    /// @par History:
    /// @li 6889/ChenWei，2023年12月15日，新建函数
    ///
    void GetTemperatureLog(TemperatureLog& Log);

    ///
    /// @brief  获取取消分割后的部件名称
    /// 
    /// tfCountInfoName thrift结构体信息
    /// 
    /// @return std::string
    ///
    /// @par History:
    /// @li 7915/LeiDingXiang，2023年12月15日，新建函数
    ///
    std::string GetUnitName(const std::string& tfCountInfoName);

    ///
    /// @brief  获取计数信息
    /// 
    /// @return void
    ///
    /// @par History:
    /// @li 6889/ChenWei，2023年12月15日，新建函数
    ///
    void GetCountInfo(StatisticalInfo& Info);

    ///
    /// @brief  获取打印/导出的液路状态信息
    /// 
    /// @return void
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年04月02日，新建函数
    ///
    void GetLiquidInfo(LiquidInfo& info);

    ///
    /// @brief  获取下位机单元日志
    /// 
    /// @return void
    ///
    /// @par History:
    /// @li 6889/ChenWei，2023年12月15日，新建函数
    ///
    void GetUnitLog(LowerCpLog& Log);

    ///
    /// @brief  初始化温度界面
    /// 
    /// @param[in]    types:设备类型列表
    /// @param[in]    devNameAndTypes    map<<设备名称，设备序列号>，设备类型>
    /// 
    /// @return void
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年8月12日，新建函数
    ///
    void InitTemperautureTableView(const std::vector<::tf::DeviceType::type>& types,
        const std::map<QString, ::tf::DeviceType::type>& devNameAndTypes);

    ///
    /// @brief  初始化液路界面
    /// 
    /// @param[in]    types:设备类型列表
    /// @param[in]    devNameAndTypes    map<<设备名称，设备序列号>，设备类型>
    /// 
    /// @return void
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年1月22日，新建函数
    ///
    void InitLiquidPathTableView(const std::vector<::tf::DeviceType::type>& types,
        const std::map<QString, ::tf::DeviceType::type>& devNameAndTypes);

    ///
    /// @brief  初始化单元界面
    /// 
    /// @param[in]    types:设备类型列表
    /// @param[in]    devNameAndTypes    map<<设备名称，设备序列号>，设备类型>
    /// 
    /// @return void
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年1月22日，新建函数
    ///
    void InitUnitTableView(const std::vector<::tf::DeviceType::type>& types,
        const std::map<QString, ::tf::DeviceType::type>& devNameAndTypes);

    ///
    /// @brief  初始化界面左侧的表格
    /// 
    /// @param[in]    types:设备类型列表
    /// @param[in]    devNameAndTypes    map<<设备名称，设备序列号>，设备类型>
    /// 
    /// @return void
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年1月22日，新建函数
    ///
    void InitCountTableView(const std::vector<::tf::DeviceType::type>& types, 
        const std::map<QString, ::tf::DeviceType::type>& devNameAndTypes);

    ///
    /// @brief  初始化界面表格
    /// 
    /// @return void
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年08月09日，新建函数
    ///
    void InitStatusInfoUpdate();

private slots:

    ///
    /// @brief  更新状态信息
    ///
    /// @param[in]    statusInfo:设备状态信息
    /// 
    /// @return void
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年2月13日，新建函数
    ///
    void OnStatusInfoUpdate(const tf::UnitStatusInfo& statusInfo);

    ///
    /// @bref
    ///		权限变化响应
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年7月26日，新建函数
    ///
    void OnPermisionChanged();

    ///
    /// @bref
    ///		选中的设备改变
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年8月10日，新建函数
    ///
    void OnSelcetDevChanged();

    ///
    /// @bref
    ///		按设备名称筛选需要显示的行
    ///
    /// @param[in]    devName:设备名称
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年8月14日，新建函数
    ///
    void FilterViewRowByDevName(const QString& devName);

    ///
    /// @bref
    ///		Tab页切换
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年10月20日，新建函数
    ///
    void OnTabWidgetChangePage();

    ///
    /// @brief 响应打印按钮
    ///
    /// @par History:
    /// @li 6889/ChenWei，2023年3月31日，新建函数
    ///
    void OnPrintBtnClicked();

    ///
    /// @brief 响应导出按钮
    ///
    /// @par History:
    /// @li 6889/ChenWei，2023年3月31日，新建函数
    ///
    void OnExportBtnClicked();

    ///
    /// @brief 响应重置按钮
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年1月24日，新建函数
    ///
    void OnResetBtnClicked();

    ///
    /// @brief 重置界面保存
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年1月24日，新建函数
    ///
    void OnResetDlgSaveClicked();

    ///
    /// @brief 响应取消重置按钮
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年1月24日，新建函数
    ///
    void OnCancelResetBtnClicked();

    ///
    /// @bref
    ///		根据权限和页签对导出和打印按钮做显示控制
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年4月7日，新建函数
    ///
    void PermissionOfPrintExportBtn();

private:
    Ui::QStatusModule*       ui;
    ResetCountInfoDlg*       m_pResetCountInfoDlg;          // 重置弹窗
	QStandardItemModel       m_temperautureItemModel;		// 温度数据模型
	QStandardItemModel       m_liquidPathItemModel;			// 液路数据模型
	QStandardItemModel       m_unitItemModel;				// 单元数据模型
	QStandardItemModel       m_countItemModel_right;		// 计数右侧数据模型
    QStandardItemModel       m_countItemModel_left;			// 计数左侧数据模型
    map<pair<QString, QString>, pair<string,vector<tf::CountInfo>>> m_data; // 计数页面数据缓存<<设备名称，单元名称>，<设备序列号,<计数信息>>>
    map<QString, QString>    m_tempData;                    // 温度页面数据缓存<设备名称，设备序列号>
    QString                  m_strCurDevName;               // 当前选中的设备名
};
