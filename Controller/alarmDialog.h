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
/// @file     alarmDialogui.h
/// @brief    主界面->告警对话框
///
/// @author   7951/LuoXin
/// @date     2022年7月19日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2022年7月19日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "alarmModel.h"
#include "shared/basedlg.h"
#include "src/thrift/gen-cpp/defs_types.h"
#include "src/public/ConfigDefine.h"

struct AlarmRowItem;
class QStandardItemModel;
class QMediaPlayer;
class QCheckBox;
class QMediaPlaylist;
class mainResetDlg;
class alarmShieldDlg;
class QItemSelection;
class QRadioButton;
class QUtilitySortFilterProxyModel;

namespace Ui {
    class QAlarmDialog;
};


class QAlarmDialog : public BaseDlg
{
	Q_OBJECT

public:
	QAlarmDialog(QWidget *parent = Q_NULLPTR);
	~QAlarmDialog();

    ///
    /// @brief	加载程序初始化前产生的报警
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年12月19日，新建函数
    ///
    void LoadAlarmBeforeInit(std::vector<::tf::AlarmDesc>&& ads);

	///
	/// @brief	停止报警音
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年9月27日，新建函数
	///
	void StopAlarmSound();

    ///
    /// @brief	检查是否存在未读的报警信息
    ///
    /// @return 存在返回true
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年3月12日，新建函数
    ///
    bool IsExistUnReadAlarm();

private:
	///
	/// @brief 显示前初始化
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年7月19日，新建函数
	///
	void InitBeforeShow();

    ///
    /// @brief 初始化表格
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年11月16日，新建函数
    ///
    void InitTableview();

    ///
    /// @brief 初始化设备筛选按钮
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年10月24日，新建函数
    ///
    void InitDevRabtn();

	///
	/// @brief
	///     加载报警描述信息到表格
	///
	/// @param[in]  alarmDesc  告警描述信息
	/// @par History:
	/// @li 7951/LuoXin，2022年7月25日，新建函数
	///
	void LoadDataToTabView(const ::tf::AlarmDesc& alarmDesc);

	///
	/// @brief	播放报警音
	///     
	///@param[in]	level	报警等级
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年9月27日，新建函数
	///
	void PlayAlarmSound(::tf::AlarmLevel::type level);

    ///
    /// @brief  更新报警音播放器
    ///     
    ///
    /// @param[in]  playList    播放列表
    /// @param[in]  name        音乐名称
    /// @param[in]  vol         音量
    /// @return 
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年5月23日，新建函数
    ///
    void UpdateMediaPlayer(QMediaPlaylist* playList, const std::string& name, int vol);

	///
	/// @brief 窗口显示事件
	///     
	/// @param[in]  event  事件对象
	///
	/// @par History:
	/// @li 7951/LuoXin，2023年1月12日，新建函数
	///
	void showEvent(QShowEvent *event);

    ///
    /// @brief 加载电子报警按钮状态
    ///     
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年11月16日，新建函数
    ///
    void LoadShowLowerAlarmBtnStatus();

    ///
    /// @brief 保存电子报警按钮状态
    ///     
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年11月16日，新建函数
    ///
    void UpdateLowerAlarmBtnStatus(bool checked);

    ///
    /// @brief	设置设备的消息已读
    ///
    /// @param[in]  name   设备名称
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年12月12日，新建函数
    ///
    void SetDataRead(const QString& name);

    ///
    /// @brief	更新显示的行
    ///
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年12月12日，新建函数
    ///
    void UpdateShowAlarmByDevName();

    ///
    /// @brief	设置屏蔽的报警码
    ///
    /// @param[in]  DevSn       设备序列号
    /// @param[in]  name        子模块名称
    /// @param[in]  mainCode    报警码主码
    /// @param[in]  midCode     报警码中码
    /// @param[in]  subCode     报警码子码
    /// @param[in]  isShield     是否屏蔽
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年12月12日，新建函数
    ///
    void SetShieldRow(const std::string& devSn, const std::string& name, int mainCode, int midCode, int subCode, bool isShield);

    ///
    /// @brief 时间过滤器
    ///
    /// @param[in]  target  目标对象
    /// @param[in]  event   事件对象
    ///
    /// @return 处理则返回true，否则返回false
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年1月30日，新建函数
    ///
    virtual bool eventFilter(QObject* target, QEvent* event) override;

signals:
    void AlarmLevelChanged(int level);

protected Q_SLOTS:

	///
	/// @brief	告警配置信息更新
	///     
	/// @par History:
	/// @li 7951/LuoXin，2022年9月27日，新建函数
	///
	void OnAlarmComfigUpdate();

	///
	/// @brief
	///     表格当前选中行改变的槽函数
	///
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年7月21日，新建函数
	///
	void OnCurrentRowChanged(const QItemSelection &selected, const QItemSelection &deselected);

	///
	/// @brief
	///     按combox的当前文本作为过滤条件显示告警
	///
	/// @param[in]  checked 按钮状态  
	///
	/// @par History:
	/// @li 7951/LuoXin，2023年1月13日，新建函数
	///
	void OnDevChanged(bool checked);

	///
	/// @brief	报警屏蔽
	///
	/// @par History:
	/// @li 7951/LuoXin，2023年1月13日，新建函数
	///
	void OnAlarmShield();

    ///
    /// @brief	解除报警屏蔽
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2023年6月29日，新建函数
    ///
    void OnDisAlarmShield();

    ///
    /// @brief	清除当前所有的报警
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2023年7月19日，新建函数
    ///
    void OnClearAllAlarm();

    ///
    /// @brief	清除所有的被屏蔽的报警码
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2023年9月5日，新建函数
    ///
    void OnClearAllShieldCode();

    ///
    /// @brief
    ///     处理告警描述信息
    ///
    /// @param[in]  alarmDesc  告警描述信息
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 7951/LuoXin，2022年7月19日，新建函数
    ///
    void OnDealFaultUpdate(const tf::AlarmDesc& alarmDesc);

    ///
    /// @brief
    ///     处理告警详情信息更新
    ///
    /// @param[in]  alarmDesc  告警描述信息
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年08月07日，新建函数
    ///
    void OnDealUpdateAlarmDetail(const tf::AlarmDesc& alarmDesc, const bool increaseParams);

    ///
    /// @brief
    ///     当前告警消失
    ///
    /// @param[in]  alarmDesc  告警描述信息
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年12月18日，新建函数
    ///
    void OnDisCurrentAlarm(const tf::AlarmDesc& alarmDesc);

    ///
    /// @bref
    ///		权限变更响应函数
    ///
    /// @par History:
    /// @li 7951/LuoXin, 2024年1月10日，新建函数
    ///
    void OnPermisionChanged();

    ///
    /// @bref
    ///		报警信息已读响应函数
    ///
    /// @par History:
    /// @li 7951/LuoXin, 2024年2月26日，新建函数
    ///
    void OnAlarmReaded(std::vector<tf::AlarmDesc, std::allocator<tf::AlarmDesc>>  alarmDescs);

private:
	Ui::QAlarmDialog*							ui;
	bool										m_bDelFalg;						///< 表格删除标志
	bool										m_bInitMediaPlayerFlag;			///< 播放器初始化标志
	QMediaPlayer*								m_mediaPlayer;				    ///< 音频播放器
    QMediaPlaylist*								m_stopMediaPlaylist;			///< 停机音频播放列表
	QMediaPlaylist*								m_sampleStopMediaPlaylist;	    ///< 加样停音频播放列表
	QMediaPlaylist*								m_attentionMediaPlaylist;		///< 提示音频播放列表
    alarmShieldDlg*                             m_pAlarmShieldDlg;              ///< 报警屏蔽列表弹窗
    AlarmShieldInfo		                        m_asi;			                ///< 屏蔽的报警码
    QRadioButton*                               m_lastRaBtn;                    ///< 上次选中的筛选按钮
    int                                         m_loginUserType;                ///< 当前登陆的用户的类型+
    bool                                        m_existReagentAlarm;            ///< 是否存在试剂/耗材不足的报警
    std::vector<std::shared_ptr<AlarmModel::AlarmRowItem>>	m_alarmData;        ///< 报警数据
};
