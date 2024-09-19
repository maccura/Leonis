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
/// @file     ReagentShowList.h
/// @brief    试剂界面
///
/// @author   4170/TangChuXian
/// @date     2020年5月14日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2020年5月14日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include <QWidget>
#include <qabstractitemmodel>
#include <boost/any.hpp>
#include "src/thrift/ch/gen-cpp/ch_types.h"
#include "src/thrift/im/gen-cpp/im_types.h"
#include "src/thrift/gen-cpp/defs_types.h"
#include "src/public/ConfigDefine.h"
#include "PrintExportDefine.h"
#include "shared/CommonInformationManager.h"

class QReagentScanDialog;
class QSelectConditionDialog;
class QStandardItem;
class QStandardItemModel;
class QSortFilterProxyModel;
class QRegisterSupplyInfo;
struct SelectConditions;
class CommonInformationManager;
class QItemSelection;
class SortHeaderView;
class QTableView;
namespace Ui { class ReagentListShow; };

enum TblItemState;
struct RowCellInfo; 

///
/// @bref
///		记录一行的基本信息
///
struct ReagRowInfor
{
    ReagRowInfor(){}
    ReagRowInfor(const std::shared_ptr<const tf::DeviceInfo>& dev, int pos)
        : m_dev(dev), m_rowPos(pos)
    {}

    std::shared_ptr<const tf::DeviceInfo> m_dev;
    int m_rowPos;
};


// 生化试剂、生化耗材、免疫试剂、免疫稀释液
enum ReagentType { RT_CH, RT_CHSUP, RT_IM, RT_IMDIT };
class BaseReagData
{
public:

    ///
    /// @bref
    ///		界面行的数据
    ///
    struct RowContentInfo
    {
        RowContentInfo();

        QString m_pos;              // 位置
        QString m_assayName;        // 项目名称
        std::shared_ptr<RowCellInfo> m_avaliableTimes;   // 可用测试数（项目报警
        std::shared_ptr<RowCellInfo> m_remainCount;      // 剩余测试数（瓶报警）
        std::shared_ptr<RowCellInfo> m_useStatus;        // 试剂状态
        std::shared_ptr<RowCellInfo> m_caliStatus;       // 校准状态
        QString m_lot;              // 批号
        QString m_sn;               // 编号
        std::shared_ptr<RowCellInfo> m_caliCurveExp;     // 校准曲线有效期
        std::shared_ptr<RowCellInfo> m_openBottleExp;    // 试剂开瓶有效期（剩余天数）
        std::shared_ptr<RowCellInfo> m_registerTime;     // 上机时间
        std::shared_ptr<RowCellInfo> m_reagentExp;       // 试剂有效期

        // 获取唯一标识
        inline QString GetUniqKey() { return m_pos + m_assayName; };

        void GetStandardItemList(QList<QStandardItem*>& itemList);

    private:
        QStandardItem* MakeStandardItem(const std::shared_ptr<RowCellInfo>& cellInfo);
    };

    ///
    /// @bref
    ///		对打印结构赋值（dbId、assayCode、printName）
    ///
    /// @param[out] info 输出的赋值后的打印结构
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年10月27日，新建函数
    ///
    virtual void AssignPrintInfor(ReagentItem& info) = 0;
    virtual void ProcShieldReagent() = 0;
    virtual void ProcPriorityReagent() = 0;

    virtual bool IsMaskReagent() = 0;

    ///
    /// @bref
    ///		获取试剂列表显示的一行内容
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年11月2日，新建函数
    ///
    virtual std::shared_ptr<RowContentInfo> GetRowContent(const std::vector<std::string>& devSNs = {}) = 0;

public:
    ReagentType m_myType;
    std::shared_ptr<RowContentInfo> m_rowContent;

    boost::any m_reagent;           // 试剂、试剂组
    boost::any m_reagentSupply;     // 耗材（用于扩展CH试剂信息）
};

// 生化试剂
class ReagDataCh : public BaseReagData
{
public:
    ReagDataCh(const ch::tf::ReagentGroup& reagentInfo, const ch::tf::SuppliesInfo& supplyInfo);
    
    ///
    /// @bref
    ///		生化对打印结构赋值（dbId、assayCode、printName）
    ///
    /// @param[out] info 输出的赋值后的打印结构
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年10月27日，新建函数
    ///
    virtual void AssignPrintInfor(ReagentItem& info) override;

    virtual void ProcShieldReagent() override;
    virtual bool IsMaskReagent() override;
    virtual void ProcPriorityReagent() override;

    virtual std::shared_ptr<RowContentInfo> GetRowContent(const std::vector<std::string>& devSNs = {}) override;
};

// 生化耗材
class ReagDataChSup : public BaseReagData
{
public:
    ReagDataChSup(const ch::tf::SuppliesInfo& supplyInfo);

    ///
    /// @bref
    ///		生化耗材对打印结构赋值（dbId、assayCode、printName）
    ///
    /// @param[out] info 输出的赋值后的打印结构
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年10月27日，新建函数
    ///
    virtual void AssignPrintInfor(ReagentItem& info) override;

    virtual void ProcShieldReagent() override;
    virtual bool IsMaskReagent() override { return false; };
    virtual void ProcPriorityReagent() override;

    virtual std::shared_ptr<RowContentInfo> GetRowContent(const std::vector<std::string>& devSNs = {}) override;
};

// 免疫
class ReagDataIm : public BaseReagData
{
public:
    ReagDataIm(const ::im::tf::ReagentInfoTable& reagentInfo);

    ///
    /// @bref
    ///		免疫对打印结构赋值（dbId、assayCode、printName）
    ///
    /// @param[out] info 输出的赋值后的打印结构
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年10月27日，新建函数
    ///
    virtual void AssignPrintInfor(ReagentItem& info) override;

    virtual void ProcShieldReagent() override;
    virtual bool IsMaskReagent() override;

    virtual void ProcPriorityReagent() override;

    virtual std::shared_ptr<RowContentInfo> GetRowContent(const std::vector<std::string>& devSNs = {}) override;
};

// 免疫稀释液
class ReagDataImDilu : public BaseReagData
{
public:
    ReagDataImDilu(const ::im::tf::DiluentInfoTable& reagentInfo);

    ///
    /// @bref
    ///		免疫稀释对打印结构赋值（dbId、assayCode、printName）
    ///
    /// @param[out] info 输出的赋值后的打印结构
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年10月27日，新建函数
    ///
    virtual void AssignPrintInfor(ReagentItem& info) override;

    virtual void ProcShieldReagent() override;
    virtual bool IsMaskReagent() override;
    virtual void ProcPriorityReagent() override;

    virtual std::shared_ptr<RowContentInfo> GetRowContent(const std::vector<std::string>& devSNs = {}) override;
};


///
/// @brief
///     缓存与管理界面不同的数据类型
///
class ReagentCacheMgr
{
public:
    // 通过位置列和项目名列获取缓存中的数据
    std::shared_ptr<BaseReagData> GetReagData(const QString& strPos, const QString& strName);

    // 通过位置删除缓存
    void EraseByPositionString(const QString& strPos);

    // 获取免疫试剂的关联试剂,return strPos
    QString GetImRelativeReagent(const ::im::tf::ReagentInfoTable& targetReag);

public:
    std::map<QString, std::shared_ptr<BaseReagData>> m_reagentInfoCache; ///< 缓存试剂信息<Model+Pos+Name, Reagent> （Model+Pos）必须是界面唯一
};

///
/// @brief
///     缓存生化的试剂组查询，用于减少访问数据库的次数
///
class ReagentGroupCacheCh
{
public:
    ///
    /// @bref
    ///		初始化数据集
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年3月4日，新建函数
    ///
    bool InitCache(const std::vector<std::string>& deviceSn);

    ///
    /// @bref
    ///		根据调解获取指定的试剂列表
    ///
    /// @param[out] reagents 获取到的试剂组列表
    /// @param[in] devSn 设备SN
    /// @param[in] pos 位置
    /// @param[in] supplyCode 对应耗材ID
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年3月4日，新建函数
    ///
    void GetReagetnGroup(std::vector<ch::tf::ReagentGroup>& reagents, const std::string& devSn, int pos, int supplyCode);

private:
    std::vector<ch::tf::ReagentGroup> m_vecReagentGroups;
};

class ReagentShowList : public QWidget
{
    Q_OBJECT

public:

	/// 试剂列表中行所存储数据的属性
	enum ReagentAttribute
	{
		ISBLANK = 1,	/// 是否空行
		ISREAGENTMASK,  /// 是否试剂屏蔽
		REANGTTYPE,		/// 试剂类型
        DEVICESN		/// 设备sn
	};

    ReagentShowList(QWidget *parent = Q_NULLPTR);
    ~ReagentShowList();

    ///
    /// @brief刷新设备试剂列表
    ///     
    ///
    /// @param[in]  spDevModule  s设备列表
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年1月21日，新建函数
    ///
    void UpdateModeDevices(const std::vector<std::shared_ptr<const tf::DeviceInfo>>& spDevModule);

	///
	/// @brief 设备状态变更
	///
	/// @return 
	///
	/// @par History:
	/// @li 1556/Chenjianlin，2023年8月2日，新建函数
	///
	void DevStateChange();

public slots:
    ///
    /// @brief
    ///     重新刷新试剂列表
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月22日，新建函数
    ///
    void OnRefreshReagentList();

    // 筛选标签关闭
    void OnFilterLableClosed();

    ///
    /// @brief 响应打印按钮
    ///
    /// @par History:
    /// @li 6889/ChenWei，2023年3月31日，新建函数
    ///
    void OnPrintBtnClicked();

    ///
    /// @brief 响应打印按钮
    ///
    /// @par History:
    /// @li 6889/ChenWei，2023年3月31日，新建函数
    ///
    void OnExportBtnClicked();

    ///
    /// @brief
    ///     需求计算按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年6月26日，新建函数
    ///
    void OnRequireCalcBtnClicked();

    ///
    /// @brief
    ///     表格列宽改变
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年10月20日，新建函数
    ///
    void OnTblColResized();

    ///
    /// @bref
    ///		权限变化响应
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年10月31日，新建函数
    ///
    void OnPermisionChanged();

	///
	/// @brief  设备otherinfo信息更新
	///
	/// @param[in]  deviceSN      设备序列号
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年11月17日，新建函数
	///
	void OnUpdateChUnloadReagentBtn(const QString &deviceSN);

	///
	/// @brief 更新加载器状态
	///
	/// @param[in]  deviceSN      设备序列号
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2023年11月24日，新建函数
	///
	void OnLoaderStatusUpdate(QString deviceSN);

	///
	/// @brief 响应申请试剂结果消息，0-取消，1-接受，2-结束
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2024年1月11日，新建函数
	///
	void OnManualHandleReagResult(const QString& deviceSN, const int result);

    ///
    /// @brief
    ///     校准过期提醒打开或关闭
    ///
    /// @param[in]  bOn  是否打开
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年3月12日，新建函数
    ///
    void OnCaliExpireNoticeOnOrOff(bool bOn);

Q_SIGNALS:
    ///
    /// @brief
    ///     发送条件
    ///
    /// @param[in]  conditions  条件数目
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年2月9日，新建函数
    ///
    void SendConditionNumber(int conditions);

    ///
    /// @brief
    ///     发送筛选条件
    ///
    /// @param[in]  msg  信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月11日，新建函数
    ///
    void SendCondMessage(QString msg);

    void SentHideFilterLable();

protected:
    // 该行数据更新方式，插入、覆盖
    enum UiRowUpdateType{ Ur_Insert, Ur_Cover};

    ///
    /// @brief
    ///     更新按钮显示（根据当前选择设备）
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年5月29日，新建函数
    ///
    void UpdateBtnVisible();

    ///
    /// @brief 显示试剂的信息到页面上
    ///     
    ///
    /// @param[in]  supplyInfo  耗材信息
    /// @param[in]  reagentInfo  试剂信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年1月21日，新建函数
    ///
    void ShowReagnetItem(UiRowUpdateType updateType, const ch::tf::SuppliesInfo& supplyInfo, \
        const ch::tf::ReagentGroup& reagentInfo);

	///
	/// @brief  当试剂组信息不存在时，覆盖一行耗材信息
	///
	/// @param[in]  pos  位置
	/// @param[in]  supplyInfo  耗材信息
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年2月26日，新建函数
	///
	void CoverChSupplies(const int pos, const ch::tf::SuppliesInfo& supplyInfo);
    void ShowReagnetItemImm(const ::im::tf::ReagentInfoTable& stuRgntInfo);

    ///
    /// @brief
    ///     显示耗材项目
    ///
    /// @param[in]  pos  耗材位置
    /// @param[in]  supplyInfo  耗材信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年2月14日，新建函数
    ///
    void ShowSupplyItem(const ch::tf::SuppliesInfo& supplyInfo);

    ///
    /// @brief
    ///     获取剩余量的状态
    ///
    /// @param[in]  supplyInfo  耗材信息
    /// @param[in]  reagent     试剂信息
    ///
    /// @return 耗材状态控件
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年2月21日，新建函数
    ///
    QStandardItem* GetBlanceStatus(const ch::tf::SuppliesInfo& supplyInfo, const ch::tf::ReagentGroup& reagent);

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
    void ShowBlankLine(int& currentPos, int lastPos, int endPostion, const std::string& devicesn);

    ///
    /// @brief
    ///     设置显示空行
    ///
    /// @param[in]  stuRgntInfo  试剂信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年6月7日，新建函数
    ///
    void ShowBlankLine(const im::tf::ReagentInfoTable& stuRgntInfo);

    ///
    /// @brief
    ///     设置显示空行
    ///
    /// @param[in]  stuDltInfo  稀释液信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年6月7日，新建函数
    ///
    void ShowBlankLine(const im::tf::DiluentInfoTable& stuDltInfo);

	///
	/// @brief 显示扫描失败的项目
	///
	/// @param[in]  devicesn    设备序列号
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年6月10日，新建函数
	///
    QString ShowScanFailedLine(int showPostion, const QString& devicesn);

	///
	/// @brief 根据位置获取行号
	///
	/// @param[in]  postion  位置
	/// @param[in]  devicesn 设备编号
	///
	/// @return 行号列表
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年6月16日，新建函数
	///
	std::vector<int> GetRowByPostion(int postion, const std::string& devicesn);

	///
	/// @brief 根据设备号和项目名称获取对应的行号
	///
	/// @param[in]  assayName  项目名称
	/// @param[in]  device     设备号
	///
	/// @return 包括实际名称和相同设备的行
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年7月29日，新建函数
	///
	std::vector<int> GetRowsByAssayInfo(const QString& assayName, const std::string& device);

	///
	/// @brief 处理双项同测的试剂的切换（从双项同测切换到单试剂，或者从单试剂切换到双项目）
	///
	/// @param[in]  reagentGroups  试剂组
	/// @param[in]  rows           行列表
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年6月23日，新建函数
	///
	void DoChangeAssays(const ch::tf::SuppliesInfo& supplyInfo, const std::vector<ch::tf::ReagentGroup>& reagentGroups, const std::vector<int>& rows);

    ///
    /// @brief
    ///     是否是数字列
    ///
    /// @param[in]  iColumn  列号
    ///
    /// @return true表示是数字列
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月28日，新建函数
    ///
    bool IsNumberColumn(int iColumn);

    ///
    /// @brief
    ///     从数据库中加载显示设置
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年4月13日，新建函数
    ///
    void LoadDisplayCfg();

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
    /// @brief 页面显示时，处理事件
    ///
    /// @param[in]  event  事件
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年9月26日，新建函数
    ///
    void showEvent(QShowEvent *event);

	///
	/// @brief 设置试剂扫描按钮是否允许状态，只针对免疫单机
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 1556/Chenjianlin，2023年8月2日，新建函数
	///
	void SetEnableOfReagSacnBtn();

    ///
    /// @brief
    ///     初始化后表格列宽自适应
    ///
    /// @param[in]  pTblView  表格
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年9月6日，新建函数
    ///
    void ResizeTblColToContentAfterInit(QTableView* pTblView);

    ///
    /// @brief
    ///     无位置选择卸载试剂
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年11月10日，新建函数
    ///
    void UnloadRgntInNoSelPos();

    ///
    /// @brief
    ///     免疫合并加卸载按钮为试剂申请按钮
    ///
    /// @param[in]  bCombine  是否合并
    /// @param[in]  strDevSn  免疫设备序列号
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年6月28日，新建函数
    ///
    void CombineLoadUnloadBtn(bool bCombine, const std::string& strDevSn = "");

    ///
    /// @brief
    ///     免疫合并加卸载按钮为试剂申请按钮
    ///
    /// @param[in]  vDevSn  免疫设备序列号
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年6月28日，新建函数
    ///
    void CombineLoadUnloadBtn(const std::vector<std::string>& vDevSn);

protected Q_SLOTS:
    
    // 界面刷新
    void RefreshModeReagentList();

    ///
    /// @brief
    ///     更新试剂信息
    ///
    /// @param[in]  deviceSN  设备编号
    /// @param[in]  positions  位置信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年12月20日，新建函数
    ///
    void UpdateReagentInformation(const std::vector<ch::tf::SupplyUpdate, std::allocator<ch::tf::SupplyUpdate>>& supplyUpdates);

    ///
    /// @brief
    ///     免疫试剂信息更新
    ///
    /// @param[in]  stuRgntInfo  试剂信息
    /// @param[in]  changeType   更新方式
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年1月5日，新建函数
    ///
    void UpdateImReagentChanged(const im::tf::ReagentInfoTable& stuRgntInfo, im::tf::ChangeType::type changeType);

    ///
    /// @brief
    ///     免疫稀释液信息更新
    ///
    /// @param[in]  stuDltInfo   试剂信息
    /// @param[in]  changeType   更新方式
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年1月5日，新建函数
    ///
    void UpdateImDltChanged(const im::tf::DiluentInfoTable& stuDltInfo, im::tf::ChangeType::type changeType);

    ///
    /// @brief
    ///     清空试剂信息
    ///
    /// @param[in]  strDevSn		设备序列号
	/// @param[in]  devModuleIndex  设备模块号（ISE维护专用，0表示所有模块，≥1表示针对指定的模块）
	/// @param[in]  lGrpId			维护组ID
    /// @param[in]  mit				维护项目类型
    /// @param[in]  enPhase			阶段
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年02月24日，新建函数
    ///
    void OnClearReagentInfo(QString strDevSn, int32_t devModuleIndex, int64_t lGrpId, tf::MaintainItemType::type mit, tf::MaintainResult::type enPhase);
    
    ///
    /// @brief
    ///     筛选按钮点击事件
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年2月9日，新建函数
    ///
    void OnSelectConditionDialog();

    ///
    /// @brief
    ///     根据条件更新界面的试剂列表数据
    ///
    /// @param[in]  searchCondtion  查询条件
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年2月10日，新建函数
    ///
    void OnConditionSearchUpdate(std::shared_ptr<SelectConditions> searchCondtion);

    ///
    /// @brief
    ///     试剂屏蔽
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月21日，新建函数
    ///
    void OnShieldReagent();

    ///
    /// @brief
    ///     优先使用
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年1月13日，新建函数
    ///
    void OnPriorityReagent();

    ///
    /// @brief
    ///     试剂卸载
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年1月13日，新建函数
    ///
    void OnReagentUnload();

    ///
    /// @bref
    ///		免疫的试剂卸载
    ///
    /// @param[in] stuTfDevInfo 设备
    /// @param[in] rowIdx 试剂位置
    /// @param[in] strReg 试剂名称
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年10月7日，新建函数
    ///
    void UnloadReagentIm(const std::shared_ptr<const tf::DeviceInfo>& stuTfDevInfo, int rowIdx, const QString& strReg);

    ///
    /// @bref
    ///		生化的试剂卸载
    ///
    /// @param[in] stuTfDevInfo 设备
    /// @param[in] rowIdx 试剂位置
    /// @param[in] strReg 试剂名称
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年10月7日，新建函数
    ///
    void UnloadReagentCh(const std::shared_ptr<const tf::DeviceInfo>& stuTfDevInfo, int rowIdx, const QString& strReg);

    ///
    /// @brief  加载试剂
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 7702/WangZhongXin，2022年8月10日，新建函数
    /// @li 8276/huchunli，2023年1月3日，从ImReagentShowList移植
    void OnReagentUpload();

    ///
    /// @brief
    ///     试剂扫描
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年1月13日，新建函数
    ///
    void OnScanReagent();

	///
	/// @brief 信息录入槽函数
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年6月13日，新建函数
	///
	void onRegisterClicked();

    ///
    /// @brief
    ///     磁珠混匀按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年10月17日，新建函数
    ///
    void OnMixBtnClicked();

    ///
    /// @brief
    ///     装载装置复位按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年7月4日，新建函数
    ///
    void OnLoadPartResetBtnClicked();

    ///
    /// @bref
    ///		更新免疫试剂选中时，关联的预处理试剂同时选中
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年1月12日，新建函数
    ///
    void SelectRelatedReagentIm(int rowIndex);

    ///
    /// @brief
    ///     表格选中项改变
    ///
    /// @par History:
    /// @li 8276/huchunli，2023年8月31日，新建函数
    /// @li 4170/TangChuXian，2023年8月31日，参数修改
    ///
    void OnTabelSelectChanged();
    void OnTableViewClicked(const QModelIndex& qIndex);

    ///
    /// @brief
    ///     试剂指定位置扫描开始
    ///
    /// @param[in]  strDevName  设备名
    /// @param[in]  vecPos      指定位置
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月22日，新建函数
    ///
    void OnRgntScanPosStart(QString strDevName, std::vector<int, std::allocator<int>> vecPos);

    ///
    /// @brief
    ///     通过显示设置更新试剂信息表
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年4月13日，新建函数
    ///
    void UpdateRgntTblByDisplayCfg();

    ///
    /// @brief
    ///     显示设置更新
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年4月13日，新建函数
    ///
    void OnDisplayCfgUpdate();

    ///
    /// @brief
    ///     试剂仓加卸载是否正在运行
    ///
    /// @param[in]  strDevSn  设备序列号
    /// @param[in]  bRunning  是否正在运行
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年8月30日，新建函数
    ///
    void OnRgntLoadStatusChanged(QString strDevSn, bool bRunning);

    ///
    /// @brief
    ///     设备状态改变
    ///
    /// @param[in]  deviceInfo  设备状态信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年9月14日，新建函数
    ///
    void OnDevStateChange(tf::DeviceInfo deviceInfo);

    ///
    /// @brief
    ///     设备温度异常状态改变
    ///
    /// @param[in]  strDevSn    设备序列号
    /// @param[in]  bErr        是否温度异常
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年10月30日，新建函数
    ///
    void OnDevTemperatureErrChanged(QString strDevSn, bool bErr);

    ///
    /// @brief
    ///     检测模式更新处理
    ///
    /// @param[in]  mapUpdateInfo  检测模式更新信息（设备序列号-检测模式）
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年11月4日，新建函数
    ///
    void OnDetectModeChanged(QMap<QString, im::tf::DetectMode::type> mapUpdateInfo);

    ///
    /// @brief
    ///     更新按钮使能状态
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年8月31日，新建函数
    ///
    void UpdateBtnEnabled();

    ///
    /// @brief
    ///     根据配置更新表格显示
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年9月6日，新建函数
    ///
    void UpdateTblDisplayByCfg();

    ///
    /// @brief	设备信息更新
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2023年6月5日，新建函数
    /// @li 4170/TangChuXian，2024年5月17日，修改为申请更换试剂专用
    ///
    void UpdateDeviceStatus(tf::DeviceInfo deviceInfo);

private:

    void UpdateSingleRegntInfo(const ::im::tf::ReagentInfoTable& stuRgntInfo);

    ///
    /// @brief 界面显示前初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年11月5日，新建函数
    ///
    void InitBeforeShow();

    ///
    /// @brief
    ///     更新耗材表
    ///
    /// @param[in]  deviceSnList  设备名称列表
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2021年8月26日，新建函数
    ///
    void UpdateConsumeTableCh(const std::vector<std::string>& deviceSnList);
    void UpdateConsumeTableImm(const std::vector<std::string>& deviceSnList);
    void UpdateChSuppliesToUI(const std::map<std::string, std::set<int>> &mapUpdateDevPos);

	///
	/// @brief  更新某一行的生化耗材信息
	///
	/// @param[in]  devSN  设备序列号
	/// @param[in]  supplyInfo  耗材信息
	/// @param[in]  reagent  试剂信息
	///
	/// @return 
	///
	/// @par History:
	/// @li 7997/XuXiaoLong，2024年5月15日，新建函数
	///
	void UpdateOneRowChSupplyInfo(const std::string& devSN, const ch::tf::SuppliesInfo& supplyInfo,
		const std::vector<::ch::tf::ReagentGroup>& reagentInfos);

    ///
    /// @bref
    ///		设置界面指定行为空
    ///
    /// @param[in] deviceSn 设备SN
    /// @param[in] reagentPos 试剂位置
    ///
    /// @par History:
    /// @li 8276/huchunli, 2022年12月30日，新建函数（来自重构提取）
    ///
    void UpdateRowsToEmpty(const std::string& deviceSn, int reagentPos);

    ///
    /// @brief
    ///     更新免疫稀释液信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年10月28日，新建函数
    ///
    void UpdateImDltInfo(const std::vector<std::string>& deviceSn);

    ///
    /// @brief
    ///     更新单条稀释液信息
    ///
    /// @param[in]  stuDltInfo  稀释液信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年10月28日，新建函数
    ///
    void ShowSingleDltInfoImm(const ::im::tf::DiluentInfoTable& stuDltInfo);

    ///
    /// @bref
    ///		获取界面当前选择行的行号，如果获取失败或行号不合法则返回-1
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年1月3日，新建函数
    ///
    int  GetCurrentNoEmptyRowIndex();

    ///
    /// @brief
    ///     获取当前选中行
    ///
    /// @return 当前选中行
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年12月6日，新建函数
    ///
    int  GetCurrentRowIndex();

    // 通过第一列信息，解析出设备SN和试剂位置信息(deviceSN-reagentPos)
    bool GetPositionInfo(int rowIndex, std::string& deviceSn, int& reagentPos);

    // 根据筛选条件更新界面
    void UpdateBySelectedCondidionsCh(const std::shared_ptr<SelectConditions>& cond, std::vector<std::string>& deviceSn);
    void UpdateBySelectedCondidionsIm(const std::shared_ptr<SelectConditions>& cond, std::vector<std::string>& deviceSn);

    void SetStanterdRowStyle(QList<QStandardItem*>& rowItems, bool isReagMask, bool isCaliMask);

    ///
    /// @bref
    ///		清空Table列表
    ///
    /// @param[in] chDevicesSn 从device中筛选返回的生化设备
    /// @param[in] imDevicesSn 从device中筛选返回的免疫设备
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年2月23日，新建函数（来自函数重构）
    ///
    void CleanTableview();
    
    // 获取分类的设备SN（生免分类）
    void GetClassfiedDeviceSn(std::vector<std::string>& chDevicesSn, std::vector<std::string>& imDevicesSn);

    // 填充打印导出数据
    void FillExportContainer(ReagentItemVector &vecReagent, bool bPrint);

    ///
    /// @bref
    ///		选中指定的行
    ///
    /// @param[in] mList 待选中的行
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年5月24日，新建函数
    ///
    void SelectByModelIndex(const QModelIndexList& mList);

	///
	/// @brief 试剂加载机构试剂槽是否存在试剂盒
	///
	/// @param[oyut]	bExist  true-存在，false-不存在
	/// @param[in]		deviceSN  设备编号
	///
	/// @return true-获取成功，false-获取失败
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年9月4日，新建函数
	///
	bool ReagentLoadExistReag(bool& bExist, std::string& deviceSN);

    inline std::shared_ptr<const tf::DeviceInfo> GetDeviceByPositionName(const QString& posName)
    {
        std::map<QString, ReagRowInfor>::iterator it = m_rowInforMap.find(posName);
        return it != m_rowInforMap.end() ? it->second.m_dev : nullptr;
    }

    // 设置排序值
    void SetSortValue(QList<QStandardItem*>& itemList, int pos);

    ///
    /// @bref
    ///		从缓存结构中获取数据
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年11月3日，新建函数
    ///
    std::shared_ptr<BaseReagData> GetReagCache(int rowIdx);

    ///
    /// @bref
    ///		获取一行的唯一标记字符串
    ///
    /// @param[in] iRow 行号
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年5月9日，新建函数
    ///
    QString GetUniqIdentify(int iRow);

private:
    Ui::ReagentListShow*            ui;                         // ui对象指针
    QStandardItemModel*				m_reagentModel;             // 试剂的模式
    QSortFilterProxyModel*          m_sortProxyModel;           // 试剂的模式对应的排序模型
    bool                            m_bInit;                    // 是否初始化

    int                             m_iSortOrder;               // 排序方式
    int                             m_iSortColIdx;              // 排序列索引
    SortHeaderView*                 m_pSortHeader;              // 表格的排序头

    QSelectConditionDialog*         m_selectDialog;             // 筛选对话框句柄
    QReagentScanDialog*				m_scanDialog;				// 试剂扫描对话框
    QRegisterSupplyInfo*            m_registerDialog;           // 登记对话框

    bool                            m_needUpdate;               // 需要更新与否
    std::vector<std::shared_ptr<const tf::DeviceInfo>> m_deivces;		// 设备详细信息

    QString                         m_curSelectedKeyString;     // 当前选中行的试剂位+名称（免疫多选时用于确定点击的是哪个
    std::shared_ptr<SelectConditions> m_selectCondition;        // 筛选条件
    std::shared_ptr<CommonInformationManager> m_commAssayMgr;   // 项目管理单列

    ReagentCacheMgr                 m_reagCacheMgr;             // 缓存界面显示的数据
    std::map<QString, ReagRowInfor> m_rowInforMap;///< Model+Pos+Name, Dev> 

    DisplaySet                      m_stuDisplayCfg;            // 显示设置
};
