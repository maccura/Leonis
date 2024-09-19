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
/// @file     maintainopdlg.h
/// @brief    维护执行对话框
///
/// @author   4170/TangChuXian
/// @date     2023年2月17日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2023年2月17日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QList>
#include <QMap>
#include "shared/basedlg.h"
#include "src/thrift/gen-cpp/defs_types.h"

namespace Ui { class MaintainOpDlg; };
class QPushButton;


class MaintainOpDlg : public BaseDlg
{
    Q_OBJECT

	// 参数设置控件的页码
	enum ParamPageNum
	{
		page_Normal = 0,				///< 常规页
		page_ChPipeFill,				///< 比色管路填充
		page_ProbeClean,				///< 针清洗
		page_ReacTankWaterReplace,		///< 反应槽水更换
		page_CuvetteReplace,			///< 比色杯更换
		page_CuvetteBlank,				///< 杯空白测定
		page_PhotometerCheck,			///< 光度计检查
		page_ReacTankClean,				///< 清洗反应槽
		page_IsePipeFill,				///< ISE管路填充
		page_IsePipeClean,				///< ISE管路清洗
		page_IseElecReplace,			///< ISE电极更换
		page_DrainBucket				///< 排空供水桶
	};

public:
    MaintainOpDlg(const QString strMaintainName, QWidget *parent = Q_NULLPTR);
    ~MaintainOpDlg();

protected:
    ///
    /// @brief
    ///     界面显示前初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月17日，新建函数
    ///
    void InitBeforeShow();

    ///
    /// @brief
    ///     界面显示后初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月17日，新建函数
    ///
    void InitAfterShow();

    ///
    /// @brief
    ///     初始化字符串资源
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年1月17日，新建函数
    ///
    void InitStrResource();

    ///
    /// @brief
    ///     初始化信号槽连接
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月17日，新建函数
    ///
    void InitConnect();

    ///
    /// @brief
    ///     初始化子控件
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月17日，新建函数
    ///
    void InitChildCtrl();

    ///
    /// @brief
    ///     窗口显示事件
    ///
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月17日，新建函数
    ///
    void showEvent(QShowEvent *event);

Q_SIGNALS:

protected Q_SLOTS:
 
    ///
    /// @brief
    ///     执行按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月18日，新建函数
    ///
    void OnExcuteBtnClicked();

	///
	/// @brief 半自动维护结束确认
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年3月12日，新建函数
	///
	void OnEndComfirmBtnClicked();

    ///
    /// @brief
    ///     更新维护组按钮显示状态
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月23日，新建函数
    ///
    void UpdateMtGrpBtnStatus();

    ///
    /// @brief
    ///     设备按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年2月23日，新建函数
    ///
    void OnDevBtnClicked();

    ///
    /// @brief
    ///     维护组阶段更新
    ///
    /// @param[in]  strDevSn        设备序列号
    /// @param[in]  lGrpId          维护组ID
    /// @param[in]  enPhaseType     阶段类型
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月30日，新建函数
    ///
    void OnMaintainGrpPhaseUpdate(QString strDevSn, long long lGrpId, tf::MaintainResult::type enPhaseType);

	///
	/// @brief 维护项阶段更新
	///
	/// @param[in]  strDevSn  设备序列号
	/// @param[in]  lGrpId  维护组ID
	/// @param[in]  mier  维护结果
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年3月12日，新建函数
	///
	void OnMaintainItemStageUpdate(QString strDevSn, long long lGrpId, tf::MaintainItemExeResult mier);

    ///
    /// @brief
    ///     更新设备维护组状态
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年3月30日，新建函数
    ///
    void UpdateDevMaintainGrpStatus();

	///
	/// @brief
	///     杯空白测定结果被点击
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年8月2日，新建函数
	///
	void OnCupBlankTestResultClicked();

	///
	/// @brief
	///     光度计检查结果被点击
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年8月2日，新建函数
	///
	void OnPhotoCheckResultClicked();

private:

    ///
    /// @brief 获取被选中的设备信息
	///		如果同一个sn的多个模块同时被选中，说明是整机，则modelIndex为0
    ///
    /// @param[in]  checkedDev  被选中的设备信息
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 8580/GongZhiQiang，2024年9月9日，新建函数
    ///
    void TakeCheckedDevkeyInfo(std::vector<::tf::DevicekeyInfo>& checkedDev);

	///
	/// @brief 通过单机版或联机版调整UI
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年3月13日，新建函数
	///
	void AdjustUIBySingleOrPipeLine();

	///
	/// @brief 垂直移动操作按钮框架
	///
	/// @param[in]  direction  方向，true:向下，false:向上
	/// @param[in]  height  高度
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年3月13日，新建函数
	///
	void VerticalMoveOptionBtnFrame(bool direction, int height);

	///
	/// @brief 根据上次维护初始化参数信息
	///
	/// @param[in]  latestMaintainInfo  上次维护结果
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年3月13日，新建函数
	///
	void InitParamByLatestMaintain(const tf::LatestMaintainInfo& latestMaintainInfo);

	///
	/// @brief
	///     根据维护名称调整界面
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年6月20日，新建函数
	///
	void UpdateUIByMaintainName();

	///
	/// @brief
	///     根据维护名称调整界面
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年6月20日，新建函数
	///
	void AdjustDevBtnByMaintainItem();

	///
	/// @brief
	///     获取维护参数
	///
	/// @param[in]  params  编码后的维护参数
	/// @param[in]  info   提示信息
	///
	/// @return true:获取成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年6月20日，新建函数
	///
	bool GetMaintainSetParam(std::string& params, QString& info);

	///
	/// @brief 从界面设置单项维护的参数
	///
	/// @param[in]  stuMtGrp  执行的维护组
	/// @param[in]  exeParams  维护执行参数
	///
	/// @return true:设置成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年9月10日，新建函数
	///
	bool SetMaintainSingleParamFromUi(tf::MaintainGroup& stuMtGrp, tf::MaintainExeParams& exeParams);

	///
	/// @brief 设置维护组参数
	///
	/// @param[in]  group  维护组
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年11月30日，新建函数
	///
	void InitMaintainGroupParam(const tf::MaintainGroup& group);

	///
	/// @brief 设置维护单项参数默认值
	///
	/// @param[in]  maintainItem  维护单项
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年11月30日，新建函数
	///
	void SetMaintainItemDefaultParam(const tf::MaintainItem&  maintainItem);

	///
	/// @brief	为输入框设置正则表达式
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年6月21日，新建函数
	///
	void SetCtrlsRegExp();

	///
	/// @brief 执行下一阶段维护
	///
	///
	/// @return true : 成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年4月11日，新建函数
	///
	bool NextStageMaintain();

	///
	/// @brief 更新提示信息
	///
	/// @param[in]  mier  维护单项阶段执行结果
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年3月9日，新建函数
	///
	void UpdateNoticeInfo(const QString& sn, const tf::MaintainItemExeResult& mier);
	
	///
	/// @brief 反应杯更换阶段更新
	///
	/// @param[in]  sn  设备序列号
	/// @param[in]  mier  维护单项阶段执行结果
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年4月11日，新建函数
	///
	void UpdateCuvetteReplacePageNotice(const QString& sn, const tf::MaintainItemExeResult& mier);

	///
	/// @brief 反应槽清洗阶段更新
	///
	/// @param[in]  mier  维护单项阶段执行结果
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年4月11日，新建函数
	///
	void UpdateReacTankCleanPageNotice(const tf::MaintainItemExeResult& mier);

	///
	/// @brief ISE管路清洗阶段更新
	///
	/// @param[in]  mier  维护单项阶段执行结果
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年4月11日，新建函数
	///
	void UpdateIsePipeCleanPageNotice(const tf::MaintainItemExeResult& mier);

	///
	/// @brief 电极更换阶段更新
	///
	/// @param[in]  mier  维护单项阶段执行结果
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年4月11日，新建函数
	///
	void UpdateIseElecReplacePageNotice(const tf::MaintainItemExeResult& mier);

	///
	/// @brief 排空供水桶阶段更新
	///
	/// @param[in]  mier  维护单项阶段执行结果
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年4月11日，新建函数
	///
	void UpdateDrainBucketPageNotice(const tf::MaintainItemExeResult& mier);

	///
	/// @brief 执行后初始化显示（半自动维护）
	///
	/// @param[in]  currentMaintainGroup  当前维护组
	///
	/// @return 
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年6月19日，新建函数
	///
	void InitMaintainExecStatus(const tf::MaintainGroup& currentMaintainGroup);

	///
	/// @brief	切换设备按钮状态
	///
	/// @param[in]  devBtn		需要切换的设备按钮
	/// @param[in]  newStatus   新的状态（status）
	///
	/// @return  true:切换成功
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年8月9日，新建函数
	///
	bool SwitchDevBtnStatus(QPushButton* const &devBtn,const QString &newStatus);

    ///
    /// @brief
    ///     选中未维护按钮
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年4月25日，新建函数
    ///
    void SelectUnmaintainBtn();

	///
	/// @brief 获取反应杯更换选择的联排编号
	///
	///
	/// @return 选中的联排编号，没有选中的则为空
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2024年8月7日，新建函数
	///
	std::vector<int> GetCuvetteReplaceCupUnits();

private:
    Ui::MaintainOpDlg                          *ui;                         // ui指针
    bool                                        m_bInit;                    // 是否已经初始化

    QString                                     m_strMaintainName;          // 维护名
    QStringList                                 m_strDevNameList;           // 设备名列表
	bool										m_DevBtnsExclusiveFlag;		// 设备按钮互斥标志
    QList<QPushButton*>                         m_pDevBtnList;              // 设备按钮列表
    QMap<QPushButton*, ::tf::DevicekeyInfo>     m_mapBtnDevInfo;            // 按钮设备信息映射
};
