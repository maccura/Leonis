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
/// @file     QCalibrateOverView.h
/// @brief    校准总览
///
/// @author   5774/WuHongTao
/// @date     2022年2月17日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月17日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QWidget>
#include <QSet>
#include <QSortFilterProxyModel>
#include "boost/optional.hpp"
#include "src/thrift/gen-cpp/defs_types.h"
#include "src/thrift/ch/gen-cpp/ch_types.h"
#include "src/thrift/ise/gen-cpp/ise_types.h"
#include "src/thrift/im/gen-cpp/im_types.h"

namespace Ui {
    class QCalibrateOverView;
};
class QCalibrationFactorEdit;

typedef std::map<tf::AssayClassify::type, std::vector<std::string>> DeviceMap;
// 免疫试剂配对信息
// key: 项目编号|批号|瓶号   value:配对的两个试剂（0:对应bottleIdx为0位置的， 1：对应bottleIdx为1位置的)
typedef QMap<QString, QVector<im::tf::ReagentInfoTable>> ImPairReagent;

class QCaliBrateMethodApp;
class QReactionCurve;
class QCaliBrateResult;
class ImQCaliBrateResult;
class QIseCaliBrateResult;
class QMachineFactor;
class QCaliRackOverview;
class QIseCaliAppManager;
class QCalibrateOverFilterDataModule;
class QStandardItemModel;
class QStandardItem;


class QCalibrateOverView : public QWidget
{
    Q_OBJECT

public:
    QCalibrateOverView(QWidget *parent = Q_NULLPTR);
    ~QCalibrateOverView();

    // 存储自定义数据
    enum UserDataRoles {
        ReagentInfo = 2,        // 试剂信息
        DatabaseId,             // 数据库id
        AssayClassify,          // 项目分类
        DefaultSort = 6         // 默认排序 (枚举值5被CReadnOnlyDegate占用)
    };

    // 单元格状态
    enum TblItemState
    {
        STATE_NORMAL = 0,           // 正常
        STATE_NOTICE,               // 提醒
        STATE_WARNING,              // 警告
        STATE_SCAN_FAILED           // 扫描失败
    };

    ///
    /// @brief刷新设备试剂列表
    ///     
    ///
    /// @param[in]  devices  设备列表
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年2月17日，新建函数
    ///
    void RefreshPage(std::vector<std::shared_ptr<const tf::DeviceInfo>>& devices);

protected:
    // 重写隐藏事件
    virtual void hideEvent(QHideEvent *event) override;

    ///
    /// @brief 按照第一列进行升序排序
    ///
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年8月30日，新建函数
    ///
    void SetAscSortByFirstColumn();

private:
    ///
    /// @brief
    ///     初始化
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年2月17日，新建函数
    ///
    void Init();

    ///
    /// @brief
    ///     清空内容
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年2月17日，新建函数
    ///
    void ClearContent();

    ///
    /// @brief
    ///     更新试剂的校准状态
    ///
    /// @param[in]  devices  设备信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年2月17日，新建函数
    ///
    void UpdateCalibrateStatusTable(DeviceMap& devices);

    ///
    /// @brief
    ///     显示生化试剂校准状态信息
    ///
    /// @param[in]  reagentInfo  试剂信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年2月17日，新建函数
    ///
    void ShowReagentStatusItem(bool addAction, int pos, const ch::tf::ReagentGroup& reagentInfo);

    ///
    /// @brief 免疫试剂校准状态信息
    ///
    /// @param[in]  addAction  true表示插入数据,false表示覆盖
    /// @param[in]  pos  数据写入行数（插入时，重新计算位置）
    /// @param[in]  reagentInfo  试剂信息
    /// @param[in]  findDB  是否重新查数据库
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 1226/zhangjing，2023年3月1日，新建函数
    ///
    void ShowImReagentStatusItem(bool addAction, int pos, const im::tf::ReagentInfoTable& reagentInfo, bool findDB = true);

    ///
    /// @brief 
    ///
    /// @param[in]  devSn  试剂的设备序列号
    /// @param[in]  itemList  数据列表
    /// @param[in]  rowIndex  需要写入表中的行（插入模式此数据无效需重新计算
    /// @param[in]  regeantPos  试剂盘中的位置
    /// @param[in]  action  true表示插入数据,false表示覆盖
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月24日，新建函数
    ///
    void SetRowItemData(const std::string& devSn, const QList<QStandardItem*>& itemList, int rowIndex, int reagentPos, bool action);

    ///
    /// @brief 确认当前试剂信息在表格中的写入方式
    ///
    /// @param[in]  pos  试剂位置
    /// @param[in]  deviceSn  设备序列号
    /// @param[in]  maxPos  保存的最大位置
    /// @param[in]  devList  已经出现过的设备信息
    ///
    /// @return true表示需要插入数据，false表示覆盖数据
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月24日，新建函数
    ///
    bool GetInsertMode(int pos, const std::string& deviceSn, int& maxPos, QVector<std::string>& devList);

	///
	/// @brief 根据位置获取行号
	///
	/// @param[in]  postion  位置
	/// @param[in]  devicesn 设备编号
	///
	/// @return 行号
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年6月16日，新建函数
	///
	std::vector<int> GetRowByPostion(int postion, const std::string& devicesn);

    ///
    /// @brief 页面显示时，处理事件
    ///
    /// @param[in]  event  事件
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年9月26日，新建函数
    ///
    void showEvent(QShowEvent *event);

	///
	/// @brief 获取表格中被选中的第一行
	///
	///
	/// @return 第一行的行索引
	///
	/// @par History:
	/// @li 8090/YeHuaNing，2022年10月11日，新建函数
	///
    int GetFirstIndex();

    ///
    /// @brief 更新按钮
    ///
    /// @param[in]  bSel  选择按钮
    /// @param[in]  bApp  执行方法
    /// @param[in]  bResult  校准结果
    /// @param[in]  bReaction  反应曲线
    /// @param[in]  bUnMaskButton  校准屏蔽
    /// @param[in]  bCaliFactorEdit  校准系数编辑
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2024年6月5日，新建函数
    ///
    void UpdateButtonStatus(bool bSel, bool bApp, bool bResult, bool bReaction, bool bUnMaskButton, bool bCaliFactorEdit);

    ///
    /// @brief 获取制定行的设备分类
    ///
    /// @param[in]  row  行号
    ///
    /// @return 设备类型
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月25日，新建函数
    ///
    tf::AssayClassify::type GetItemClassify(int row);

    ///
    /// @brief 获取选择行执行选择按钮时的动作（需要自行判断是否有选择项）
    ///（选择行中有任意一个没有选择，则执行选择）
    ///
    /// @return true：表示执行选择 false:表示取消选择
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月26日，新建函数
    ///
    bool GetSelectAction(const QModelIndexList& selList);

    ///
    /// @brief 获取指定行的用户自定义数据
    ///
    /// @param[in]  rowIndex  行号
    /// @param[in]  role  规则
    ///
    /// @return 返回值
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月25日，新建函数
    ///
    template<typename T>
    boost::optional<T> GetItemUserDataInfo(int rowIndex, UserDataRoles role)
    {
        if (rowIndex < m_caliBrateMode->rowCount())
        {
            QStandardItem* item = m_caliBrateMode->item(rowIndex);
            if (item != nullptr)
            {
                return boost::make_optional(item->data(Qt::UserRole + (int)role).value<T>());
            }
        }

        return boost::none;
    }

	///
	/// @brief
	///     判断该试剂组是否拥有校准曲线
	///
	/// @param[in]  reagent  待判断试剂组
	/// 
	/// 
	/// @return true:表示有校准曲线  false:表示没有校准曲线
	///
	/// @par History:
	/// @li 8580/GongzhiQiang，2022年12月9日，新建函数
	///
	bool isContansCaliCurve(const boost::optional<ch::tf::ReagentGroup>& reagent);

	///
	/// @brief
	///     初始化校准申请界面中的查询名称
	/// 
	/// 
	///
	/// @par History:
	/// @li 8580/GongzhiQiang，2022年12月19日，新建函数
	///
	void initCaliProjectNameBox(DeviceMap&  devices);

    ///
    /// @brief 保存免疫试剂配对信息
    ///
    /// @param[in]  reagent  试剂信息
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年6月5日，新建函数
    ///
    void StorePairedReagent(const im::tf::ReagentInfoTable& reagent);

    ///
    /// @brief 获取配对试剂的map表的key
    ///
    /// @param[in]  reagent  试剂信息
    ///
    /// @return map表主键
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年6月5日，新建函数
    ///
    QString GetImPairReagentMapKey(const im::tf::ReagentInfoTable& reagent);

    ///
    /// @brief 设置表格列固定宽度
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年6月6日，新建函数
    ///
    void SetTableColumnFixedWidth();

    ///
    /// @brief 处理瓶索引为1的试剂（使用此函数之前，需要保证m_imPairedReagent已经正确更新）
    ///
    ///
    /// @return 瓶索引为1的是否已经正确匹配（false时，需要将此位置显示到列表中）
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年6月6日，新建函数
    ///
    bool DealImPairedReagentOfBottleIdx_1(const im::tf::ReagentInfoTable& reagent);

    ///
    /// @brief 界面操作后，需要更新单条记录时，设置配对试剂的信息
    ///
    /// @param[in]  reagent  试剂信息
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年6月6日，新建函数
    ///
    void SetPairedInfo(const im::tf::ReagentInfoTable& reagent);

    ///
    /// @brief 通过表格行移除配对试剂
    ///
    /// @param[in]  row  行号
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年6月6日，新建函数
    ///
    void RemovePairedReagentsByRow(int row);

    ///
    /// @brief 移除
    ///
    /// @param[in]  reagent  试剂信息
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年6月6日，新建函数
    ///
    void RemovePairedReagent(const im::tf::ReagentInfoTable& reagent);

    ///
    /// @brief 设置配对试剂的位置
    ///
    /// @param[in]  reagent  index为1的试剂
    /// @param[in]  r  行号
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年6月8日，新建函数
    ///
    void SetPairedReagentPos(const im::tf::ReagentInfoTable& reagent, int r);

    ///
    /// @brief 增加项目到筛选下拉框
    ///
    /// @param[in]  assayCode  项目编号
    ///
    /// @return 是否增加
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年6月8日，新建函数
    ///
    bool AddAssayToSearchList(int assayCode);

    //
    /// @brief 从筛选下拉框移除某个项目
    ///
    /// @param[in]  assayCode  项目编号
    ///
    /// @return 是否删除
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年6月8日，新建函数
    ///
    bool RmAssayFromSearchList(int assayCode);

    //
    /// @brief 增加项目到筛选下拉框
    ///
    /// @param[in]  assayCode  项目编号
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年6月8日，新建函数
    ///
    void SetSearchList();

    ///
    /// @brief 设置纯生化按钮是否显示
    ///
    /// @param[in]  visible  true:显示
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年6月27日，新建函数
    ///
    void SetChBtnVisible(bool visible);

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

private slots:

    ///
    /// @brief
    ///     校准申请对话框显示
    ///
    /// @param[in]  index  索引
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月2日，新建函数
    ///
    void OnShowAppDialog();

	///
	/// @brief 校准屏蔽解除
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 8090/YeHuaNing，2022年9月6日，新建函数
	///
	void UnMasckCali();

    ///
    /// @brief 执行选择功能
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月25日，新建函数
    ///
    void OnSelectBtn();

	///
	/// @brief 显示反应曲线
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年7月19日，新建函数
	///
	void OnShowReactCurve();

	///
	/// @brief 显示校准结果
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年7月18日，新建函数
	///
	void OnShowCalibrateResult();

	///
	/// @brief 显示校准系数编辑对话框
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年1月6日，新建函数
	///
	void OnShowCaliFactorEditDialog();

    ///
    /// @brief 显示校准架概况对话框
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年4月25日，新建函数
    ///
    void OnShowCaliRackOverviewDlg();

    ///
    /// @brief
    ///     改变方法
    ///
    /// @param[in] method		方法 
	/// @param[in] isSelected   是否选择 
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月2日，新建函数
    ///
    void OnChangeCaliMethod(int method, bool isSelected);

	///
	/// @brief 校准文档杯选择与否
	///
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年6月15日，新建函数
	///
	void OnCaliDocSelected(const QModelIndex& index);

    ///
    /// @brief 表格选择变化消息处理
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年10月31日，新建函数
    ///
    void OnSelectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

	///
	/// @brief 处理双项同测的试剂的切换（从双项同测切换到单试剂，或者从单试剂切换到双项目）
	///
	/// @param[in]  reagentGroups  试剂组
	/// @param[in]  rows           行列表
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年6月23日，新建函数
	///
	void DoChangeAssays(const std::vector<ch::tf::ReagentGroup>& reagentGroups, const std::vector<int>& rows);

	///
	/// @brief 更新试剂的校准状态信息
	///  
	/// @param[in]  supplyUpdates  位置信息
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年6月20日，新建函数
	///
    void UpdateCaliSatus(class std::vector<class ch::tf::SupplyUpdate, class std::allocator<class ch::tf::SupplyUpdate>> supplyUpdates);

    ///
    /// @brief 更新电解质的校准状态信息
    ///  
    /// @param[in]  iseAppUpdates  更新信息
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年12月30日，新建函数
    ///
    void UpdateCaliAppSatus(class std::vector<class ise::tf::IseModuleInfo, class std::allocator<class ise::tf::IseModuleInfo>> iseAppUpdates);

    ///
    /// @brief
    ///     更新单条校准申请信息
    ///
    /// @param[in]  stuRgntInfo  试剂信息
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年11月29日，新建函数
    ///
    void UpdateSingleCaliReqInfo(const im::tf::ReagentInfoTable& stuRgntInfo, const im::tf::ChangeType::type type);

    ///
    /// @brief
    ///     更新单条校准申请信息
    ///
    /// @param[in]  reagentInfo  试剂信息
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年12月1日，新建函数
    ///
    void UpdateSingleReagentGroup(const ch::tf::ReagentGroup& reagentInfo);

    ///
    /// @brief 更新校准选择状态
    ///
    /// @param[in]  row  触发行的索引
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年12月1日，新建函数
    ///
    void UpdateCaliSelect(int row);

	///
	/// @brief 执行方法双击切换
	///
	/// @param[in]  index  索引
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2022年12月9日，新建函数
	///
	void OnCaliModeChanged(const QModelIndex& index);


	///
	/// @brief 校准项目查询
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2022年12月19日，新建函数
	///
	void OnCaliProjectNameSearch();


	///
	/// @brief 校准项目查询重置
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2022年12月19日，新建函数
	///
	void OnCaliProjectNameReset();

    ///
    /// @brief 根据过滤条件判断此项是否需要显示
    ///
    /// @param[in]  data  一行记录
    ///
    /// @return true:显示此行数据，false:不显示此行数据
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2023年5月8日，新建函数
    ///
    bool IsShowLineByFilter(const QList<QStandardItem*>& data);

	///
	/// @brief 项目配置发生变化
	///
	/// @par History:
	/// @li 1226/zhangjing，2023年6月28日，新建函数
	///
	void OnAssayUpdated();

    ///
    /// @brief  新的一天刷新界面
    ///
    /// @par History:
    /// @li 1226/zhangjing，2023年8月29日，新建函数
    ///
    void OnRefreshCaliInfoList();

    ///
    /// @bref
    ///		权限变更响应
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年10月18日，新建函数
    ///
    void OnPermisionChanged();

    ///
    /// @brief
    ///     检测模式更新处理
    ///
    /// @param[in]  mapUpdateInfo  检测模式更新信息（设备序列号-检测模式）
    ///
    /// @par History:
    /// @li 1226/zhangjing，2023年11月8日，新建函数
    ///
    void OnDetectModeChanged(QMap<QString, im::tf::DetectMode::type> mapUpdateInfo);

    ///
    /// @brief 校准提示发生变化
    ///
    /// @param[in]  bOn  开关状态
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2024年4月11日，新建函数
    ///
    void OnCaliExpireNoticeChanged(bool bOn);

signals:
    ///
    /// @brief 选择按钮状态变化
    ///
    /// @param[in]  sel  是否选择
    ///
    /// @par History:
    /// @li 8090/YeHuaNing，2022年12月14日，新建函数
    ///
    void sigSelBtnStateChanged(bool sel);

private:
    Ui::QCalibrateOverView*          ui;
    QStandardItemModel*             m_caliBrateMode;            // 校准的模式
	QCalibrateOverFilterDataModule* m_caliBrateModeProxyModel;  // 校准的模式代理
    DeviceMap                       m_devs;                     // 对应设备列表
    QCaliBrateMethodApp*            m_CaliAppDialog;            // 校准申请对话框
	QReactionCurve*					m_reactionDialog;			// 反应曲线对话框
	QCaliBrateResult*				m_calibrateResultDialog;	// 校准结果对话框
    ImQCaliBrateResult*             m_imCaliRestDlg;            // 免疫校准结果对话框
    QIseCaliBrateResult*            m_iseCaliRestDlg;           // 电解质校准结果对话框
	QMachineFactor*					m_machineFactorDlg;			// 仪器系数对话框
    QCaliRackOverview*              m_caliRackOverviewDlg;      // 校准架概况对话框
    QIseCaliAppManager*             m_iseCaliAppManager;        // ise 校准申请数据管理器
	QCalibrationFactorEdit*         m_caliFactorEditDlg;		// 校准系数编辑对话框
    bool                            m_needUpdate;               // 需要更新与否
    bool                            m_selItem;                  // 根据选择的行，记录执行选择还是取消选择功能（默认为执行选择时，为取消选择（即false））
	bool							m_blankMethodEnable;		// 执行方法多行选择时，是否需要禁止空白选择
    QString                         m_filterAssayName;          // 筛选项目名称
    ImPairReagent                   m_imPairedReagent;          // 免疫试剂配对
    QSet<QString>                   m_loadeAssays;              // 在机项目
    bool                            m_bIsSearching;                // 是否正在筛选
};

///////////////////////////////////////////////////////////////////////////
/// @file     QCalibrateOverView.h
/// @brief 	 排序代理
///
/// @author   7656/zhang.changjiang
/// @date      2023年4月11日
/// @version  0.1
///
/// @par Copyright(c):
///     2022 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7656/zhang.changjiang，2023年4月11日，新建文件
///
///////////////////////////////////////////////////////////////////////////
class QCalibrateOverFilterDataModule : public QSortFilterProxyModel
{
public:
	QCalibrateOverFilterDataModule(QObject* parent = nullptr);
	~QCalibrateOverFilterDataModule();
	virtual bool lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const override;
};
Q_DECLARE_METATYPE(ch::tf::ReagentGroup);
Q_DECLARE_METATYPE(im::tf::ReagentInfoTable);
Q_DECLARE_METATYPE(tf::AssayClassify::type)


