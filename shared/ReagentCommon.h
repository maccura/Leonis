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
/// @file     ReagentCommon.h
/// @brief    试剂通用文档
///
/// @author   5774/WuHongTao
/// @date     2021年12月2日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2021年12月2日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include <string>
#include <vector>
#include <map>
#include <QString>
#include <boost/optional.hpp>

#include "src/thrift/gen-cpp/defs_types.h"
#include "src/thrift/ch/gen-cpp/ch_types.h"
#include "src/thrift/ise/gen-cpp/ise_types.h"
#include "src/thrift/im/gen-cpp/im_types.h"
#include "src/thrift/ch/gen-cpp/ch_constants.h"

#include "shared/uidef.h"

class QStandardItem;

#define SetItemColor(item, color) {item->setData(color, Qt::UserRole+ 5);}

// 试剂状态
enum USESTATE
{
    INIT,
    NORMAL,
    WARING,
    ERRORSTATE,
    END
};

// 试剂列表单元格状态
enum TblItemState
{
    STATE_NORMAL = 0,           // 正常
    STATE_NOTICE,               // 提醒
    STATE_WARNING,              // 警告
    STATE_SCAN_FAILED           // 扫描失败
};

// 试剂更新结果
enum RgntChangeRlt
{
    RGNT_CHANGE_RLT_CANCEL = 0, // 取消
    RGNT_CHANGE_RLT_ACCEPT,     // 接受
    RGNT_CHANGE_RLT_FINISH,     // 结束
};

// 试剂更换申请状态
enum RgntChangeApplyStatus
{
    RGNT_CHANGE_APPLY_STATUS_ALLOW_APPLY = 0,   // 可申请
    RGNT_CHANGE_APPLY_STATUS_COMMIT_APPLY,      // 提交申请
    RGNT_CHANGE_APPLY_STATUS_APPLYING,          // 申请中
    RGNT_CHANGE_APPLY_STATUS_HANDLE,            // 申请处理中
};

// 试剂卡片显示信息
struct CardShowInfo
{
    // 卡片的编号
    int assayCode;
    // 卡片名称
    std::string assayName;
    // 模块状态的显示字符串组
    std::vector<std::string> showMsg;
    // 每台设备测试余量
    std::multimap<std::string, int> deviceVolumn;
    // 余量
    int lastVolumn;
    // 试剂状态
    USESTATE state;

    CardShowInfo()
    {
        state = NORMAL;
    }

    void ToShowContent()
    {
        lastVolumn = 0;
        for (const auto& volumn : deviceVolumn)
        {
            std::string msg = volumn.first + ":" + std::to_string(volumn.second);
            showMsg.push_back(msg);
            // 增加次数
            lastVolumn += volumn.second;
        }
    }
};

///
/// @brief
///     列表单元格显示信息
///
struct RowCellInfo
{
    QString m_text;             // 显示内容
    QString m_backgroudColor;   // 背景颜色
    QString m_fontColor;        // 字体颜色

    QString m_dataExt;

    // 构造
    RowCellInfo() {};
    RowCellInfo(const QString& strText) { m_text = strText; };
};

///
/// @brief
///     获取试剂校准状态
///
/// @param[in]  reagentInfo  试剂信息
/// @param[in]  existCurveId 存在校准曲线
///
/// @return 试剂校准状态控件
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月21日，新建函数
///
QStandardItem* GetCalibrateShowStatus(::tf::CaliStatus::type reagentCaliStatus, bool existCurveId, QStandardItem* item = nullptr);
void MakeCaliShowStatus(::tf::CaliStatus::type reagentCaliStatus, bool existCurveId, RowCellInfo& cellInfo);

/// @brief
///     试剂组的校准曲线ID是否存在
///
inline bool IsCaliCurveIdValiable(const ch::tf::ReagentGroup& reagentInfo) {
    return (reagentInfo.__isset.caliCurveId && reagentInfo.caliCurveId != -1);
}

///
/// @brief 
///     获取校准剩余有效天数
///
/// @param[in]  reagentInfo  
///
/// @return 校准有效天数控件
///
/// @par History:
/// @li 8090/YeHuaNing，2022年9月2日，新建函数
///
QStandardItem* GetCaliCurveExpirateDate(const ch::tf::ReagentGroup& reagentInfo, QStandardItem* item = nullptr);
QString MakeCaliCurveExpirateDate(const ch::tf::ReagentGroup& reagentInfo, RowCellInfo& cellInfo);

///
/// @brief 根据位置号获取位置控件
///
/// @param[in]  deviceSn		设备编号
/// @param[in]  postionNumber   位置编号
///
/// @return 位置控件
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月13日，新建函数
///
QStandardItem* GetItemFromPostionAndDevice(const std::string& deviceSn, int postionNumber, QStandardItem* item = nullptr);
QString MakePositionString(const std::string& deviceSn, int postionNumber);

///
/// @brief
///     通过设备和位置号获取文本
///
/// @param[in]  deviceSn        设备序列号
/// @param[in]  postionNumber   位置号
///
/// @par History:
/// @li 4170/TangChuXian，2023年8月25日，新建函数
///
QString GetTextFromPostionAndDevice(const std::string& deviceSn, int postionNumber);

///
/// @brief
///     根据耗材和试剂组状态给出显示试剂状态
///
/// @param[in]  supplyInfo  耗材信息
/// @param[in]  reagentInfo  试剂信息
///
/// @return 试剂显示状态控件
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月22日，新建函数
///
QStandardItem* GetReagentShowStatus(const ch::tf::SuppliesInfo& supplyInfo, const ch::tf::ReagentGroup& reagentInfo, QStandardItem* item = nullptr);
void MakeReagentStatusInfo(const ch::tf::SuppliesInfo& supplyInfo, const ch::tf::ReagentGroup& reagentInfo, RowCellInfo& cellInfo);


///
/// @brief  获取显示状态(只有生化需要显示备用瓶，ise没有备用瓶)
///
/// @param[in]  placeStat   放置状态
/// @param[in]  useStat		使用状态
/// @param[in]  supCode		项目编号
/// @param[in]  backupToTal 备用总数
/// @param[in]  backNum     备用编号
/// @param[in]  iseResidualZero		ise耗材余量为0
///
/// @return 耗材状态的item
///
/// @par History:
/// @li 7997/XuXiaoLong，2023年10月20日，新建函数
///
QStandardItem* GetSupplyShowStatus(tf::PlaceStatus::type placeStat, tf::UsageStatus::type useStat, int supCode, 
	 int backupTotal = -1, int backNum = -1, bool iseResidualZero = false);
void MakeSupplyShowStatus(RowCellInfo& cellInfo, tf::PlaceStatus::type placeStat, tf::UsageStatus::type useStat, int supCode, 
	 int backupTotal = -1, int backNum = -1, bool iseResidualZero = false);

/// @brief
///     获取试剂有效期的状态控件
///
/// @param[in]  reagentInfo  试剂信息
///
/// @return 状态控件
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月21日，新建函数
///
QStandardItem* GetReagenExpiredStatus(const ch::tf::ReagentGroup& reagentInfo);
void MakeReagenExpiredStatus(const ch::tf::ReagentGroup& reagentInfo, RowCellInfo& cellInfo);

///
/// @brief
///     获取校准曲线类型
///
/// @param[in]  reagentSourceType  校准曲线类型枚举
///
/// @return 试剂校准曲线类型控件
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月17日，新建函数
///
QStandardItem* TranslateCalibrateCurveType(::tf::CurveSource::type reagentSourceType);

///
/// @brief 
///     获取校准曲线类型简写
///
/// @param[in]  reagentCurveType  校准曲线类型枚举
/// @param[out]  type		      曲线简写字母
///
/// @return 
///
/// @par History:
/// @li 8090/YeHuaNing，2022年9月1日，新建函数 caliCurveSource
///
void GetCalibrateCurveTypeBrif(::tf::CurveSource::type reagentCurveType, QString& type);

///
/// @brief 获取ise样本类型名称
///
/// @param[in]  type  样本类型
///
/// @return 样本名称
///
/// @par History:
/// @li 8090/YeHuaNing，2022年12月27日，新建函数
///
QString GetIseSampleName(const ::ise::tf::SampleType::type& type);

///
/// @brief 将ise模块索引转为名称
///
/// @param[in]  moduleIndex  模块索引
///
/// @return 模块名称
///
/// @par History:
/// @li 8090/YeHuaNing，2022年12月27日，新建函数
///
QString GetIseModuleName(int moduleIndex);

///
/// @brief 获取ISE的名称
///
/// @param[in]  supplyInfo  耗材信息
///
/// @return 名称
///
/// @par History:
/// @li 5774/WuHongTao，2023年5月12日，新建函数
///
QString GetIseName(const ise::tf::SuppliesInfo& supplyInfo);

///
/// @brief 获取校准原因
///
/// @param[in]  reagentInfo  试剂信息
///
/// @return 试剂校准原因控件
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月14日，新建函数
///
QStandardItem* GetCaliBrateReason(const tf::CaliReason::type& caliReason, QStandardItem* item = nullptr);

///
/// @brief 根据校准原因获取对应的中文信息
///
/// @param[in]  caliReason  校准原因
///
/// @return 原因
///
/// @par History:
/// @li 5774/WuHongTao，2024年2月28日，新建函数
///
QString GetCaliBrateReasonInfo(const tf::CaliReason::type& caliReason);

///
/// @brief 获取校准时间
///
/// @param[in]  reagentInfo  试剂组信息
///
/// @return 校准时间控件
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月14日，新建函数
///
QStandardItem* GetCaliTime(const ch::tf::ReagentGroup& reagentInfo);

///
/// @brief 
///     获取校准时间
///
/// @param[in]  reagentInfo  试剂组信息
/// @param[out]  caliTime  校准时间
///
/// @return 
///
/// @par History:
/// @li 8090/YeHuaNing，2022年9月1日，新建函数
///
void GetCaliTime(const ch::tf::ReagentGroup& reagentInfo, QString& caliTime);

///
/// @brief 校准方法
///
/// @param[in]  caliMode  校准方法的枚举
///
/// @return 试剂校准方法控件
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月14日，新建函数
///
QStandardItem* GetCaliBrateMethod(tf::CaliMode::type caliMode, QStandardItem* item = nullptr);

///
/// @brief 获取校准执行方法的名称
///
/// @param[in]  caliMode  校准执行方法
///
/// @return 名称
///
/// @par History:
/// @li 5774/WuHongTao，2024年2月28日，新建函数
///
QString GetCaliBrateMethodInfo(const tf::CaliMode::type caliMode);

///
/// @brief 根据试剂信息获取耗材信息
///
/// @param[in]  reagentGroup  试剂信息
///
/// @return 耗材对象，找不到为空
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月15日，新建函数
///
std::shared_ptr<ch::tf::SuppliesInfo> GetSupplyInfoByReagent(const ch::tf::ReagentGroup& reagentGroup);

///
/// @brief 根据耗材查找对应的试剂组
///
/// @param[in]  supplyInfo  耗材信息
///
/// @return 试剂组信息
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月16日，新建函数
///
bool GetReagentGroupsBySupply(const ch::tf::SuppliesInfo& supplyInfo, std::vector<ch::tf::ReagentGroup>& chReagent);

///
/// @brief
///     获取字符串排序值
///
/// @param[in]  strSort  排序字符串
///
/// @return 字符串排序值
///
/// @par History:
/// @li 4170/TangChuXian，2023年3月27日，新建函数
///
unsigned long long GetStringSortVal(const QString& strSort);

///
/// @brief 根据耗材位置信息查找对应的在线试剂信息（对应的试剂信息大部分为1个，当有两个的时候为双向同测）
///
/// @param[in]  supplyPos  耗材位置信息
///
/// @return 试剂信息
///
/// @par History:
/// @li 5774/WuHongTao，2023年8月19日，新建函数
///
std::vector<ch::tf::ReagentGroup> GetReagentGroupsBySupply(const ch::tf::SupplyUpdate& supplyPos);

///
/// @brief 根据项目编号获取所有的试剂信息
///
/// @param[in]  assaycode  项目编号
///
/// @return 试剂信息列表
///
/// @par History:
/// @li 5774/WuHongTao，2023年8月18日，新建函数
///
std::vector<ch::tf::ReagentGroup> GetReagentGroupByAssayCode(int assaycode);

///
/// @brief 根据ISE编号，获取ISE的耗材信息
///
/// @param[in]  assaycode  ISE耗材编号
///
/// @return 耗材信息列表
///
/// @par History:
/// @li 5774/WuHongTao，2023年8月22日，新建函数
///
std::vector<ise::tf::SuppliesInfo> GetISESupplyByAssayCode(int assaycode);

///
/// @brief 根据项目编号获取所有的试剂信息(免疫)
///
/// @param[in]  assaycode  
///
/// @return 
///
/// @par History:
/// @li 5774/WuHongTao，2023年8月19日，新建函数
///
std::vector<im::tf::ReagentInfoTable> GetImReagentGroupByAssayCode(int assaycode);

///
/// @brief 是否是血清指数
///
/// @param[in]  assayCode  项目编号
///
/// @return true:表示血清指数
///
/// @par History:
/// @li 5774/WuHongTao，2023年8月19日，新建函数
///
bool IsSIND(int assayCode);

///
/// @brief 是否ISE设备
///
/// @param[in]  assayCode  项目编号
///
/// @return true:表示ISE设备
///
/// @par History:
/// @li 5774/WuHongTao，2023年8月22日，新建函数
///
bool IsISE(int assayCode);

///
/// @brief 是否试剂异常
///
/// @param[in]  reagent  试剂信息
///
/// @return true:试剂异常，反之则不是
///
/// @par History:
/// @li 5774/WuHongTao，2023年8月18日，新建函数
///
bool IsReagentAbNormal(const ch::tf::ReagentGroup& reagent);

///
/// @brief 是否试剂异常(免疫)
///
/// @param[in]  reagent  试剂信息
///
/// @return true:试剂异常，反之则不是
///
/// @par History:
/// @li 5774/WuHongTao，2023年8月19日，新建函数
///
bool IsReagentAbNormal(const im::tf::ReagentInfoTable& reagent);

///
/// @brief 是否试剂异常(ISE)
///
/// @param[in]  supply		耗材信息
/// @param[in]  moduleInfo	耗材模块信息（用于判断是否校准）
///
/// @return true:耗材异常，反之则不是
///
/// @par History:
/// @li 5774/WuHongTao，2023年8月22日，新建函数
///
bool IsReagentAbNormal(const ise::tf::SuppliesInfo& supply, std::shared_ptr<ise::tf::IseModuleInfo> moduleInfo);

///
/// @brief 是否校准异常
///
/// @param[in]  reagent  试剂信息
///
/// @return true:校准异常，反之则不是
///
/// @par History:
/// @li 5774/WuHongTao，2023年8月18日，新建函数
///
bool IsCaliStatusAbnormal(const ch::tf::ReagentGroup& reagent);

///
/// @brief 是否校准异常（免疫）
///
/// @param[in]  reagent  试剂信息
///
/// @return true:校准异常，反之则不是
///
/// @par History:
/// @li 5774/WuHongTao，2023年8月18日，新建函数
///
bool IsCaliStatusAbnormal(const im::tf::ReagentInfoTable& reagent);

///
/// @brief ISE的校准信息是否正常
///
/// @param[in]  moduleInfo  ISE的模块信息
///
/// @return true:校准异常，反之则不是
///
/// @par History:
/// @li 5774/WuHongTao，2023年8月22日，新建函数
///
bool IsCaliStatusAbnormal(std::shared_ptr<ise::tf::IseModuleInfo> moduleInfo);

///
/// @brief   获取耗材的余量
///
/// @param[in]  supplyInfo  耗材信息
///
/// @return 耗材余量(单位0.1ul)
///
/// @par History:
/// @li 5774/WuHongTao，2022年2月11日，新建函数
///
int GetSupplyResidual(const ch::tf::SuppliesInfo& supplyInfo);

///
/// @brief 获取对应耗材编号的所有可用测试数
///
/// @param[in]  supplyCode		耗材编号
/// @param[in]  deviceSn        设备序列号
///
/// @return 耗材的所有可用余量(单位0.1ul)
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月10日，新建函数
///
int GetAvailableSupplyTimes(int supplyCode, const std::string& deviceSn);

///
/// @brief 获取编号对应试剂的总可用测试数
///
/// @param[in]  assayCode  试剂编号
/// @param[in]  deviceSn   设备序列号
///
/// @return 总可用试剂次数
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月10日，新建函数
///
int GetAvailableReagentTimes(int assayCode, const std::string& deviceSn);

///
/// @brief
///     比较两个耗材的大小（首先根据设备号，再次根据位置）
///
/// @param[in]  supplyFirst  耗材1
/// @param[in]  supplySecond  耗材2
///
/// @return true代表大于
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月24日，新建函数
///
template<class T>
bool CompareSupplyPriority(const T& supplyFirst, const T& supplySecond)
{
	int ret = supplyFirst.deviceSN.compare(supplySecond.deviceSN);
	if (ret < 0)
	{
		return true;
	}
	else if (ret  > 0)
	{
		return false;
	}
	else
	{
		if (supplyFirst.pos < supplySecond.pos)
		{
			return true;
		}
		else if (supplyFirst.pos > supplySecond.pos)
		{
			return false;
		}
		else
		{
			return false;
		}
	}
}

///
/// @brief 根据起点和结束点来生成对应的表达式
///
/// @param[in]  startRange  范围起点
/// @param[in]  endRange    范围结束点
///
/// @return qt正则表达式
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月16日，新建函数
///
std::shared_ptr<QRegExp> GenRegexpByText(int startRange, int endRange);

///
/// @bref
///		根据设备SN查找设备列表中的设备
///
/// @param[in] srcDevs 设备列表
/// @param[in] deviceSn 指定的SN
///
/// @par History:
/// @li 8276/huchunli, 2023年1月5日，新建函数
///
std::shared_ptr<const tf::DeviceInfo> FindDeviceInfoBySn(\
    std::vector<std::shared_ptr<const tf::DeviceInfo>>& srcDevs, const std::string& deviceSn);

// 组合试剂的位置信息（DeviceName-Position）
QString CombineReagentPos(const std::string& deviceSn, int reagentPos);

///
/// @bref	试剂/耗材是否优先使用的弹框
///
/// @param[in] isReagent true表示试剂,false表示耗材
///
/// @par History:
/// @li 7997/XuXiaoLong，2023年11月24日，新建函数
///
bool IsReagentPriortyTipDlg(const bool isReagent);

///
/// @bref
///		试剂是否屏蔽的弹框
///
/// @param[in] reagentName 试剂名
/// @param[in] isShield 是否是屏蔽，true代表屏蔽，false代表解除屏蔽
///
/// @par History:
/// @li 8276/huchunli, 2023年2月17日，新建函数
///
bool IsReagentShieldTipDlg(const QString& reagentName, bool isShield);

///
/// @brief
///     试剂卸载的统一弹框提示
///
/// @par History:
/// @li 未知，新建函数
/// @li 4170/TangChuXian，2023年11月10日，删除未使用的参数
///
bool IsReagentUnloadTipDlg();


///
/// @brief   生化试剂卸载的判断
///
/// @param[in]  deviceInfo		设备信息
/// @param[in]  spyInfo			耗材信息
/// @param[in]  regGroupInfo	试剂组信息  默认不包含试剂组
///
/// @par History:
/// @li 7997/XuXiaoLong，2023年11月24日，新建函数
///
bool UnloadReagentStat(const tf::DeviceInfo &deviceInfo, const ch::tf::SuppliesInfo &spyInfo, const ch::tf::ReagentGroup &regGroupInfo = {});

///
/// @brief
///     弹出卸载试剂被禁止的提示框
///
/// @param[in]  deviceInfo   设备信息
/// @param[in]  stuRgntInfo  试剂信息
///
/// @return true表示卸载试剂被禁止并弹出了提示框
///
/// @par History:
/// @li 4170/TangChuXian，2023年5月5日，新建函数
///
bool PopUnloadRgntForbiddenTip(const tf::DeviceInfo &deviceInfo, const im::tf::ReagentInfoTable& stuRgntInfo);

///
/// @brief
///     无位置卸载试剂
///
/// @param[in]  deviceInfo  设备信息
///
/// @par History:
/// @li 4170/TangChuXian，2023年11月10日，新建函数
///
bool PopUnloadRgntForbiddenTip(const tf::DeviceInfo &deviceInfo);

///
/// @brief
///     是否使能试剂卸载
///
/// @param[in]  deviceInfo   设备信息
/// @param[in]  stuRgntInfo  试剂信息
///
/// @return true表示允许
///
/// @par History:
/// @li 4170/TangChuXian，2023年9月14日，新建函数
///
bool IsEnableUnloadRgnt(const tf::DeviceInfo &deviceInfo, const im::tf::ReagentInfoTable& stuRgntInfo = im::tf::ReagentInfoTable());

///
/// @brief
///     是否使能装载装置复位
///
/// @param[in]  deviceInfo  设备信息
///
/// @return true表示允许
///
/// @par History:
/// @li 4170/TangChuXian，2024年7月4日，新建函数
///
bool IsEnableRgntLoaderReset(const tf::DeviceInfo &deviceInfo);

///
/// @bref
///     是否存在符合扫描条件的设备
///
/// @par History:
/// @li 4170/TangChuXian, 2023年9月14日，新建函数
///
bool IsExistDeviceCanScan();

///
/// @brief
///     是否存在设备可以磁珠混匀
///
/// @par History:
/// @li 4170/TangChuXian，2023年9月14日，新建函数
///
bool IsExistDeviceCanMix();

///
/// @brief
///     是否存在设备可以装载装置复位
///
/// @par History:
/// @li 4170/TangChuXian，2024年7月4日，新建函数
///
bool IsExistDeviceCanResetLoader();

///
/// @brief
///     弹出卸载稀释液被禁止的提示框
///
/// @param[in]  deviceInfo  设备信息
/// @param[in]  stuDltInfo  稀释液信息
///
/// @return true表示卸载稀释液被禁止并弹出了提示框
///
/// @par History:
/// @li 4170/TangChuXian，2023年5月5日，新建函数
///
bool PopUnloadDltForbiddenTip(const tf::DeviceInfo &deviceInfo, const im::tf::DiluentInfoTable& stuDltInfo);

///
/// @brief
///     是否使能卸载稀释液
///
/// @param[in]  deviceInfo  设备信息
/// @param[in]  stuDltInfo  稀释液
///
/// @par History:
/// @li 4170/TangChuXian，2023年9月14日，新建函数
///
bool IsEnableUnloadDlt(const tf::DeviceInfo &deviceInfo, const im::tf::DiluentInfoTable& stuDltInfo);

///
/// @brief
///     弹出装载试剂被禁止的提示框
///
/// @param[in]  deviceInfo  设备信息
/// @param[in]  iPos        试剂位置
///
/// @return true表示装载试剂被禁止并弹出了提示框
///
/// @par History:
/// @li 4170/TangChuXian，2023年5月6日，新建函数
///
bool PopLoadRgntForbiddenTip(const tf::DeviceInfo &deviceInfo, int& iPos);

///
/// @brief
///     是否使能加载试剂
///
/// @param[in]  deviceInfo  设备信息
///
/// @return true表示使能
///
/// @par History:
/// @li 4170/TangChuXian，2023年9月14日，新建函数
///
bool IsEnableLoadRgnt(const tf::DeviceInfo &deviceInfo);

///
/// @brief
///     弹出磁珠混匀被禁止提示框
///
/// @param[in]  deviceInfo  设备信息
///
/// @return true表示磁珠混匀被禁止并弹出了提示框
///
/// @par History:
/// @li 4170/TangChuXian，2023年5月6日，新建函数
///
bool PopBeadMixForbiddenTip(const tf::DeviceInfo &deviceInfo);

///
/// @brief
///     装载装置复位
///
/// @param[in]  deviceInfo  设备信息
///
/// @return true表示装载装置复位被禁止并弹出了提示框
///
/// @par History:
/// @li 4170/TangChuXian，2024年7月4日，新建函数
///
bool PopRgntLoaderResetForbiddenTip(const tf::DeviceInfo &deviceInfo);

///
/// @brief
///     执行单设备试剂混匀
///
/// @param[in]  strDevSn  设备序列号
///
/// @par History:
/// @li 4170/TangChuXian，2023年8月18日，新建函数
///
void ExecuteSingleDevBeadMix(const std::string& strDevSn);

///
/// @brief
///     弹出试剂扫描被禁止提示框
///
/// @param[in]  deviceInfo  设备信息
///
/// @return true表示试剂扫描被禁止并弹出了提示框
///
/// @par History:
/// @li 4170/TangChuXian，2023年6月12日，新建函数
///
bool PopRgntScanForbiddenTip(const tf::DeviceInfo &deviceInfo);

///
/// @brief 是否允许试剂扫描（只针对免疫单机）
///
/// @param[in]  deviceInfo  
///
/// @return true：允许 false：不允许
///
/// @par History:
/// @li 1556/Chenjianlin，2023年8月2日，新建函数
///
bool EnableReagentScan(const tf::DeviceInfo &deviceInfo);

///
/// @brief 创建文字居中的Item
///
/// @param[in]  text  项目显示文字
///
/// @return QStandardItem*对象
///
/// @par History:
/// @li 8090/YeHuaNing，2023年7月17日，新建函数
///
QStandardItem* CenterAligmentItem(const QString& text);

///
/// @brief 设置文字居中
///
/// @param[in]  item  项目
///
/// @return 修改后的项目
///
/// @par History:
/// @li 8090/YeHuaNing，2023年7月17日，新建函数
///
QStandardItem* SetItemTextAligCenter(QStandardItem* item);


///
/// @brief
///     是否校准过期
///
/// @param[in]  stuRgntInfo  试剂信息
///
/// @return true表示校准过期
///
/// @par History:
/// @li 4170/TangChuXian，2023年6月2日，新建函数
///
bool IsCaliExpired(const ::im::tf::ReagentInfoTable& stuRgntInfo);

///
/// @brief
///     是否是特殊清洗液
///
/// @param[in]  stuDltInfo  稀释液信息
///
/// @return true表示是
///
/// @par History:
/// @li 4170/TangChuXian，2023年12月5日，新建函数
///
bool IsSpecialWashing(const im::tf::DiluentInfoTable& stuDltInfo);

///
/// @brief
///     是否是特殊清洗液
///
/// @param[in]  iDltNum  稀释液编号
///
/// @return true表示是
///
/// @par History:
/// @li 4170/TangChuXian，2023年12月5日，新建函数
///
bool IsSpecialWashing(int iDltNum);

///
/// @brief
///     根据耗材需求信息获取耗材名称
///
/// @param[in]  stuSplReqInfo  耗材需求信息
///
/// @return 耗材名称
///
/// @par History:
/// @li 4170/TangChuXian，2023年11月17日，新建函数
///
QString GetNameBySplReqInfo(const tf::SplRequireInfo& stuSplReqInfo);

// 试剂屏蔽的判断
inline bool ShieldReagentStat(const tf::DeviceInfo &deviceInfo, tf::PlaceStatus::type palceStat)
{
    return (palceStat == tf::PlaceStatus::PLACE_STATUS_LOAD);
}

bool ShieldReagentStatIm(const tf::DeviceInfo &deviceInfo, im::tf::StatusForUI::type uiStat);

// 设备列表中是否只有免疫
bool IsDevicesOnlyIm(const std::vector<std::shared_ptr<const tf::DeviceInfo>>& curDevi);

// 预处理试剂， 批号相同，瓶号substep互同, bottleIdx 不可同时为1，不可同时为0
inline bool IsRelativeReagentIm(const ::im::tf::ReagentInfoTable& reagentA, const ::im::tf::ReagentInfoTable& reagentB)
{
    return (reagentA.bottleIdx ^ reagentB.bottleIdx) && reagentA.reagentLot == reagentB.reagentLot && \
        reagentA.reagentSN == reagentB.subStep;
}

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
void GetTblItemColor(std::shared_ptr<RowCellInfo>& cellInfo, TblItemState enState);

///
/// @brief
///     试剂加载功能是否被屏蔽
///
/// @param[in]  bMask       是否屏蔽
/// @param[in]  deviceSN    设备序列号
///
/// @return true表示执行成功
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月18日，新建函数
///
bool ReagentLoadIsMask(bool& bMask, const std::string& deviceSN);

///
/// @brief
///     设置是否申请更换试剂
///
/// @param[in]  enApplyStatus  申请状态
///
/// @par History:
/// @li 4170/TangChuXian，2024年1月18日，新建函数
/// @li 4170/TangChuXian，2024年5月16日，参数修改enApplyStatus
///
void SetApplyChangeRgnt(const std::string& strDevSn, RgntChangeApplyStatus enApplyStatus);

///
/// @brief
///     获取申请更换试剂状态
///
/// @param[in]  strDevSn  设备序列号
///
/// @return 申请更换试剂状态
///
/// @par History:
/// @li 4170/TangChuXian，2024年5月16日，新建函数
///
RgntChangeApplyStatus GetApplyChangeRgntStatus(const std::string& strDevSn);

///
/// @brief 获取同杯前反应名称
///
/// @param[in]  preAssayCode  同杯前项目编号
///
/// @return 同杯前名称
///
/// @par History:
/// @li 8090/YeHuaNing，2024年3月8日，新建函数
///
QString GetPreAssayName(int preAssayCode);

///
/// @brief
///     获取开瓶有效天数
///
/// @param[in]  stuRgntInfo  试剂信息
///
/// @return 开瓶有效天数
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月12日，新建函数
///
int GetOpenBottleValidDays(const im::tf::ReagentInfoTable& stuRgntInfo);

///
/// @brief
///     获取开瓶有效天数
///
/// @param[in]  stuDltInfo  稀释液信息
///
/// @return 开瓶有效天数
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月12日，新建函数
///
int GetOpenBottleValidDays(const im::tf::DiluentInfoTable& stuDltInfo);

///
/// @brief
///     获取开瓶有效天数
///
/// @param[in]  stuSplInfo  耗材信息
///
/// @return 开瓶有效天数
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月12日，新建函数
///
int GetOpenBottleValidDays(const im::tf::SuppliesInfoTable& stuSplInfo);

///
/// @brief
///     初始化试剂管理器
///
/// @par History:
/// @li 4170/TangChuXian，2024年3月29日，新建函数
///
void InitRgntManager();

///
/// @brief  获取开瓶有效天数
///
/// @param[in]  openTime  开瓶有效期字符串
///
/// @return 开瓶有效期天数
///
/// @par History:
/// @li 7997/XuXiaoLong，2024年5月16日，新建函数
///
int GetOpenRestdays(const std::string& openTime);

///
/// @brief  生化耗材过了失效日期（以yyyy-MM-dd来判断）
///
/// @param[in]  exprityTime  失效日期
///
/// @return true表示失效
///
/// @par History:
/// @li 7997/XuXiaoLong，2024年5月17日，新建函数
///
bool ChSuppliesExpire(const std::string& exprityTime);

///
/// @brief  解析输入的位置字符串
///
/// @param[in]   inputStringPos  字符串位置信息
/// @param[out]  vctPositons  vector位置信息
///
/// @return true表示成功
///
/// @par History:
/// @li 7997/XuXiaoLong，2024年8月14日，新建函数
///
bool ParserInputPositions(const QString& inputStringPos, std::vector<int>& vctPositons);
