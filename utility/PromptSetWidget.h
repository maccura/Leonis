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
/// @file     PromptSetWidget.h
/// @brief    应用--系统--提示设置
///
/// @author   7951/LuoXin
/// @date     2023年5月22日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2023年5月22日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QMediaPlayer>
#include <QWidget>

class AlarmMusicDlg;
class QColorHeaderModel;
class QStandardItemModel;
class QUtilitySortFilterProxyModel;

namespace Ui {
    class PromptSetWidget;
}

namespace tf { class DisableAlarmCode; };

class PromptSetWidget : public QWidget
{
    Q_OBJECT

public:
    PromptSetWidget(QWidget *parent = Q_NULLPTR);
    ~PromptSetWidget();

    ///
    /// @brief	当前界面是否有未保存的数据
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2024年03月14日，新建函数
    ///
    bool isExistChangedData();

protected:

    ///
    /// @brief 事件过滤器（处理tabbar的点击事件，当前页面存在未保存的数据，切换时提示用户）
    ///
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年3月13日，新建函数
    /// 
    bool eventFilter(QObject *obj, QEvent *event) override;

    private slots:
    ///
    /// @brief 保存设置参数
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年7月1日，新建函数
    ///
    void OnSaveParameter();

    ///
    /// @brief  音乐管理按钮被点击
    ///     
    /// @return 
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年5月24日，新建函数
    ///
    void OnMusicManageBtnClicked();

    ///
    /// @brief	试听音频
    ///
    /// @par History:
    /// @li 7951/LuoXin，2022年9月26日，新建函数
    ///
    void OnSoundClicked();

    ///
    /// @brief	音量调节槽函数
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年10月09日，新建函数
    ///
    void OnValueChanged(int value);

    ///
    /// @brief	切换tab页
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年10月11日，新建函数
    ///
    void OnTabWidgetChangedPage(int index);

    ///
    /// @brief	更新控件的显示和编辑权限
    ///    
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年3月29日，新建函数
    ///
    void UpdateCtrlsEnabled();

private:
    ///
    /// @brief 初始化报警设置对话框
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年6月30日，新建函数
    ///
    void Init();

    ///
    /// @brief 初始化报警提示
    ///
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年5月24日，新建函数
    ///
    void InitAlarmView();

    ///
    /// @brief 报警提示刷新表格显示
    ///
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年5月24日，新建函数
    ///
    void UpdateAlarmView();

    ///
    /// @brief 初始化结果状态
    ///
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年5月24日，新建函数
    ///
    void InitResultView();

    ///
    /// @brief 通过设备的型号获取其分类
    ///
    /// @return 设备分类(产品线)
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年5月31日，新建函数
    ///
    QString GetDevClassifyByDevType(int devType);

    ///
    /// @brief	刷新报警音乐文件列表
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2022年9月26日，新建函数
    ///
    void RefreshAlarmMusicFiles();

    ///
    /// @brief	加载结果提示设置
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2023年5月30日，新建函数
    ///
    void LoadResultPromptConfig();

    ///
    /// @brief	加载报警提示设置
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2023年5月30日，新建函数
    ///
    void LoadAlarmPromptConfig();

    ///
    /// @brief	加载报警提示音配置信息
    ///
    /// @par History:
    /// @li 7951/LuoXin，2022年9月23日，新建函数
    ///
    void LoadAlarmMusicConfig();

    ///
    /// @brief	加载结果状态配置信息到表格
    ///     
    /// @par History:
    ///  @li 7951/LuoXin，2023年10月13日，新建函数
    ///
    void LoadResultStatusConfig();

    ///
    /// @brief	加载生化结果状态配置信息到表格
    ///     
    /// @par History:
    ///  @li 7951/LuoXin，2023年10月13日，新建函数
    ///
    void LoadChResultStatusConfig();

    ///
    /// @brief	加载免疫结果状态配置信息到表格
    ///     
    /// @par History:
    ///  @li 7656/zhang.changjiang，2023年4月19日，新建函数
    ///
    void LoadImResultStatusConfig();

    ///
    /// @brief	是否存在未保存的结果提示信息
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2024年3月13日，新建函数
    ///
    bool ExistUnSaveResultPromptConfig();

    ///
    /// @brief	保存结果提示设置
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2023年5月30日，新建函数
    ///
    void SaveResultPromptConfig();

    ///
    /// @brief	是否存在未保存的报警提示信息
    ///    
    /// @param[out]  deleteCodes   删除的项目编号
    /// @param[out]  deleteCodes   增加的项目编号
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年3月13日，新建函数
    ///
    void GetModifyAlarmPromptConfig(std::vector<::tf::DisableAlarmCode>& deleteCodes,
        std::vector<::tf::DisableAlarmCode>& addCodes);

    ///
    /// @brief	保存报警提示设置
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2023年5月30日，新建函数
    ///
    void SaveAlarmPromptConfig();

    ///
    /// @brief	是否存在未保存的报警提示音乐信息
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2024年3月13日，新建函数
    ///
    bool ExistUnSaveAlarmMusicConfig();

    ///
    /// @brief	保存报警提示音配置信息
    ///
    /// @par History:
    /// @li 7951/LuoXin，2022年9月23日，新建函数
    ///
    void SaveAlarmMusicConfig();

    ///
    /// @brief	是否存在未保存的结果状态配置信息
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2024年3月13日，新建函数
    ///
    bool ExistUnSaveResultStatusConfig();

    ///
    /// @brief	保存结果状态配置信息
    ///
    /// @par History:
    ///  @li 7656/zhang.changjiang，2023年4月19日，新建函数
    ///
    void SaveResultStatusConfig();

    ///
    /// @brief	保存结果状态配置信息
    ///
    /// @par History:
    ///  @li 7951/LuoXin，2023年10月16日，新建函数
    ///
    void SaveChResultStatusConfig();

    ///
    /// @brief	保存结果状态配置信息
    ///
    /// @par History:
    ///  @li 7951/LuoXin，2023年10月16日，新建函数
    ///
    void SaveImResultStatusConfig();

    ///
    /// @brief	是否存在未保存的校准提示配置信息
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2024年3月13日，新建函数
    ///
    bool ExistUnSaveCaliLineExpire();

    ///
    /// @brief	保存校准提示配置信息
    ///
    /// @par History:
    ///  @li 7951/LuoXin，2024年2月4日，新建函数
    ///
    void SaveCaliLineExpire();

    ///
    /// @brief 窗口显示事件
    ///     
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年5月24日，新建函数
    ///
    void showEvent(QShowEvent *event);
private:
    Ui::PromptSetWidget*		ui;
    QMediaPlayer							m_player;			    // 音频播放器
    QStringList								m_listAlarmMusic;	    // 报警音乐名称列表
    QStandardItemModel*						m_pResultStatusModel;   // 结果状态配置表model
    QColorHeaderModel* 						m_pAlarmCodeModel;      // 报警提示配置表model
    QUtilitySortFilterProxyModel*           m_pAlarmCodeSortModel;  // 报警提示表格排序的model
    AlarmMusicDlg*                          m_pAlarmMusicDlg;       // 音乐管理弹窗
    bool                                    m_bInit;                // 是否初始化
    std::vector<::tf::DisableAlarmCode>     m_vecDisableAlarmCode;  // 禁用的报警码
    QUtilitySortFilterProxyModel*           m_pReagentSortModel;    // 试剂提示表格排序的model
    QUtilitySortFilterProxyModel*           m_pSupplySortModel;     // 耗材提示表格排序的model
    QUtilitySortFilterProxyModel*           m_pWasteSortModel;      // 废液提示表格排序的model
    QUtilitySortFilterProxyModel*           m_pDataAlarmSortModel;  // 数据报警表格排序的model
};
