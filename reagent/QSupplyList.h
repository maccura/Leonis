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
/// @file     QSupplyList.h
/// @brief    耗材列表
///
/// @author   5774/WuHongTao
/// @date     2021年10月19日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2021年10月19日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QWidget>
#include <vector>
#include <string>
#include <memory>
#include <boost/any.hpp>
#include "src/public/im/ImConfigDefine.h"
#include "src/thrift/im/gen-cpp/im_types.h"
#include "src/thrift/ch/gen-cpp/ch_types.h"
#include "src/thrift/ise/gen-cpp/ise_types.h"
#include "PrintExportDefine.h"

class QRecordISESupply;
class QStandardItemModel;
class QStandardItem;
class QPushButton;
class QItemSelection;
class SortHeaderView;                       // 三序排序表头
class QSampleDetectDlg;
class QIsePerfusionDlg;
namespace Ui { class SupplyList; };

enum SupplyType{ ST_CH, ST_ISE, ST_IM};


// 记录每行对应的数据结构
class  SuppyDataItem
{
public:
    // 生化
    SuppyDataItem(const ch::tf::SuppliesInfo& supplyItem);
    // ISE
    SuppyDataItem(const ise::tf::SuppliesInfo& supplyItem);
    // 免疫
    SuppyDataItem(const im::tf::SuppliesInfoTable& supplyItem);

    SupplyType m_supplyType;
    boost::any m_supplyItem;
};

class QSupplyList : public QWidget
{
    Q_OBJECT

public:
    // 单元格状态
    enum TblItemState
    {
        STATE_NORMAL = 0,           // 正常
        STATE_NOTICE,               // 提醒
        STATE_WARNING,              // 警告
        STATE_SCAN_FAILED           // 扫描失败
    };

public:
    QSupplyList(QWidget *parent = Q_NULLPTR);
    ~QSupplyList();

    ///
    /// @brief刷新设备试剂列表
    ///     
    ///
    /// @param[in]  spDevModule  s设备列表
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年1月21日，新建函数
    ///
	using Devices = std::vector<std::shared_ptr<const tf::DeviceInfo>>;
    void RefreshSupplyList(Devices& spDevModule);

	///
	/// @brief  更新ISE液路灌注按钮使能
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年11月16日，新建函数
	///
	void updatePerfusionBtnEnable();

private:
    ///
    /// @brief
    ///     更新按钮显示（根据当前选择设备）
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年5月29日，新建函数
    ///
    void UpdateBtnVisible();

    ///
    /// @brief 通过模块描述获取的设备名称
    ///
    /// @param[in]  device  模块描述
    ///
    /// @return 设备名称
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年11月15日，新建函数
    ///
    QString GetDeviceNameByModuleDsc(const QString& deviceModuleDescrip);

    ///
    /// @brief
    ///     初始化
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年2月16日，新建函数
    ///
    void Init();

    ///
    /// @brief
    ///     更新耗材信息
    ///
    /// @param[in]  deviceSnList  设备名称列表
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年2月16日，新建函数
    ///
    void UpdateConsumeTable();

	///
	/// @brief
	///     设置显示空行
	///
	/// @param[out]  currentPos  当前的行数
	/// @param[in]  lastPos      上次的位置
	/// @param[in]  endPostion   本次的位置
	/// @param[in]  devicesn     设备编号
	///
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年3月24日，新建函数
	///
	void ShowBlankLine(int& currentPos, int lastPos, int endPostion, const QString& devicesn);

    ///
    /// @brief
    ///     显示耗材信息
    ///
    /// @param[in]  pos  位置信息
    /// @param[in]  supplyInfo  耗材信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年2月16日，新建函数
    ///
    void ShowSupplyItemCh(const ch::tf::SuppliesInfo& supplyInfo, std::vector<QList<QStandardItem*>>& tableContent);
    void ShowSupplyItemISE(const ise::tf::SuppliesInfo& supplyInfo, std::vector<QList<QStandardItem*>>& tableContent);
    bool ShowSupplyItemIm(const im::tf::SuppliesInfoTable& stuSplInfo, std::vector<QList<QStandardItem*>>& tableContent);
    ///
    /// @bref
    ///		查询耗材信息并更新耗材列表
    ///
    /// @param[in] deviceSn 设备序列号
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年1月4日，新建函数
    ///
    void UpdateTableListCh(const std::vector<std::string>& deviceSn, std::vector<QList<QStandardItem*>>& tableContent);
    void UpdateTableListIse(const std::vector<std::string>& deviceSn, std::vector<QList<QStandardItem*>>& tableContent);
    void UpdateTableListIm(const std::vector<std::string>& deviceSn, std::vector<QList<QStandardItem*>>& tableContent);

    // 选中关联的一组耗材
    void SelectRelatedSupplyIm(int rowIdx);

    ///
    /// @brief
    ///     是否选中反应杯
    ///
    /// @return true表示是
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年12月19日，新建函数
    ///
    bool IsSelectReaction();

    ///
    /// @brief
    ///     获取选中设备序列号
    ///
    /// @return 选中项的设备序列号，没有时返回空
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年1月8日，新建函数
    ///
    QString GetSelectDevSn();

	///
	/// @brief 根据位置和模块名称查找对应的位置
	///
	/// @param[in]  postion  位置
	/// @param[in]  devicesn  模块号
	///
	/// @return list中的位置
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年6月20日，新建函数
	///
	int GetRowByPostion(int postion, const std::string& devicesn);

    ///
    /// @bref
    ///		获取当前模块的所有行号;（TODO: 免疫耗材结构中没有pos字段，暂时从头开始显示）
    ///
    /// @param[in] devicesn 模块SN
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年1月5日，新建函数
    ///
    bool GetRowsByModuleName(const std::string& devicesn, std::vector<int>& rows);

	///
	/// @brief 根据设备列表获取设备名称列表
	///
	/// @param[in]  deviceModules  设备模块列表
	///
	/// @return 设备名称列表
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年7月13日，新建函数
	///
	std::vector<std::string> GetDeviceSnVec(Devices& deviceModules);

    ///
    /// @brief
    ///     设置表格单元格状态
    ///
    /// @param[in]  pItem    表格单元项
    /// @param[in]  enState  状态
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年6月28日，新建函数
    ///
    void SetTblItemState(QStandardItem* pItem, TblItemState enState);

    ///
    /// @brief 页面显示时，处理事件s
    ///
    /// @param[in]  event  事件
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年9月26日，新建函数
    ///
    void showEvent(QShowEvent *event);

    ///
    /// @brief
    ///     更新排序值
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年10月9日，新建函数
    ///
    void UpdateSortValue();

private slots:
	///
	/// @brief 更新耗材试剂表
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年7月1日，新建函数
	///
	void OnUpdateAssayTable() { UpdateConsumeTable(); }

    ///
    /// @brief
    ///     耗材信息更新
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年1月12日，新建函数
    ///
    void OnSplInfoUpdate();

	///
	/// @brief 优先使用耗材
	///
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年6月14日，新建函数
	///
	void OnPrioritySupply();
    void UpdatePrioritySupplyCh(const std::string& deviceSn, int supplyCode, int pos);

	///
	/// @brief 更新耗材信息
	///   
	/// @param[in]  supplyUpdates  需要更新的耗材的详细信息（位置等）
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年6月14日，新建函数
	///
	void OnUpdateSupplyInfo(std::vector<ch::tf::SupplyUpdate, std::allocator<ch::tf::SupplyUpdate>> supplyUpdates);

	///
	/// @brief 注册ISE耗材
	///
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年7月11日，新建函数
	///
	void OnApplyIseSupply();

    ///
    /// @brief
    ///     免疫耗材信息更新
    ///
    /// @param[in]  stuSplInfo   耗材信息
    /// @param[in]  changeType   更新方式
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年1月5日，新建函数
    ///
    void UpdateImSplChanged(const im::tf::SuppliesInfoTable& stuSplInfo, im::tf::ChangeType::type changeType);

    ///
    /// @brief
    ///     耗材管理设置改变
    ///
    /// @param[in]  strDevSn    设备序列号
    /// @param[in]  stuSplMng   耗材管理
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月27日，新建函数
    ///
    void OnSplMngCfgChanged(QString strDevSn, ImDeviceOtherInfo stuSplMng);

    void OnTabelSelectChanged(const QItemSelection& currIndex, const QItemSelection& prevIndex);

    ///
    /// @bref
    ///		自定义排序
    ///
    /// @param[in] columNm 目标列
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年2月8日，新建函数
    ///
    void OnSortByColumn(int columNm);

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
    /// @brief
    ///     加载反应杯按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年4月3日，新建函数
    ///
    void OnAddReactBtnClicked();

    ///
    /// @brief
    ///     反应杯扫描结束
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年12月23日，新建函数
    ///
    void OnReactCupScanFinished();


	///
	/// @brief  点击余量探测按钮
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年10月27日，新建函数
	///
	void OnRemainDetect();

	///
	/// @brief  开始余量探测
	///  
	/// @param[in]  devs  探测的设备
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年10月27日，新建函数
	///
	void StartRemainDetect(std::vector<std::pair<std::string, int>> devs);

    ///
    /// @bref
    ///		权限变化响应
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年10月31日，新建函数
    ///
    void OnPermisionChanged();

	///
	/// @brief  点击液路灌注按钮
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年11月16日，新建函数
	///
	void OnISEPerfusion();

	///
	/// @brief  ISE耗材更新
	///
	/// @param[in]  supplyUpdates  更新的耗材信息
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年8月31日，新建函数
	///
	void OnUpdateSupplyForISE(std::vector<ise::tf::SupplyUpdate, std::allocator<ise::tf::SupplyUpdate>> supplyUpdates);

	///
	/// @brief   设备OtherInfo信息更新
	///
	/// @param[in]  devSn  设备序列号
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年3月20日，新建函数
	///
	void OnDeviceOtherInfoUpdate(const QString& devSn);

private:

    ///
    /// @bref
    ///		填充打印导出数据
    ///
    /// @param[out] vecSupply 输出的填充好的数据
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年8月29日，新建函数
    ///
    void FillExportContainer(SupplyItemVector &vecSupply);

    ///
    /// @bref
    ///		组装开瓶有效期
    ///
    /// @param[in] openBottleExp 开瓶有效期
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年3月3日，新建函数
    ///
    QStandardItem* OpenBottleExpireItem(const std::string& openBottleExp);
    QStandardItem* ExpireItem(const std::string& expireTime);

    // 不在界面显示的类型
    bool IsExceptSupplyTypeIm(im::tf::SuppliesType::type supplyType);

    // 存在备用的耗材类型
    inline bool IsExistStandbyType(im::tf::SuppliesType::type supplyType)
    {
        return supplyType == im::tf::SuppliesType::SUPPLIES_TYPE_SUBSTRATE_A ||
            supplyType == im::tf::SuppliesType::SUPPLIES_TYPE_SUBSTRATE_B;
    }

    // 获取选择行号
    std::vector<int> GetSelectedRows();

    ///
    /// @bref
    ///		对免疫的耗材按照指定的顺序进行排序
    ///
    /// @param[in/out] srcSupplies 输入的耗材列表
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年6月19日，新建函数
    ///
    void SortSuppliesIm(const std::vector<im::tf::SuppliesInfoTable>& srcSupplies, std::vector<im::tf::SuppliesInfoTable>& outSorted);

private:
    Ui::SupplyList*                 ui;                         ///< ui对象指针
    QStandardItemModel*             m_supplyMode;               ///< 耗材的模式
	Devices							m_deivceVec;
	QRecordISESupply*				m_regIseDialog;				///< 登记ISE的对话框
    bool                            m_needUpdate;               ///< 需要更新与否
    bool                            m_bInit;                    //   是否已经初始化
	QSampleDetectDlg*				m_sampleDetectDlg;			///< 余量探测弹窗
    std::pair<int, Qt::SortOrder>   m_sortedHistory;            ///< 记录排序
    std::map<QString, std::shared_ptr<SuppyDataItem>> m_supplyItemCache; //  暂存的每行耗材信息

    SortHeaderView*                 m_pSortHeader;              // 表格的排序头
	QIsePerfusionDlg*				m_isePerfusionDlg;			// ISE液路灌注
};
