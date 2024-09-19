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
/// @file     ImDataAlarmRetestSettingsDlg.h
/// @brief    应用->项目设置->项目范围->数据报警复查
///
/// @author   8276/huchunli
/// @date     2023年8月30日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8276/huchunli，2023年8月30日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QWidget>
#include <basedlg.h>
#include <map>
#include <string>
#include <memory>
#include "src/thrift/im/gen-cpp/im_types.h"

namespace Ui { class ImDataAlarmRetestSettingsDlg; };
class QStandardItemModel;
class QPixmap;
class QLabel;
class CenterComboBox;

struct DataAlarmItem
{
    ///
    /// @bref
    ///		记录一条数据报警描述行
    ///
    /// @param[in] alarmId 报警ID
    /// @param[in] name 报警名
    /// @param[in] decrip 报警描述
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年8月31日，新建函数
    ///
    DataAlarmItem(int alarmId, const std::string& name, const QString& decrip);

    DataAlarmItem();

    int m_alarmId;
    int m_rowIndex;
    std::string m_alarmName;
    QString m_description;

    QLabel* m_beUseCheck;
    CenterComboBox* m_diluRateComb;
};


class ImDataAlarmRetestSettingsDlg : public BaseDlg
{
    Q_OBJECT

public:
    ImDataAlarmRetestSettingsDlg(QWidget *parent = Q_NULLPTR);
    ~ImDataAlarmRetestSettingsDlg();

    ///
    /// @bref
    ///		初始化数据
    ///
    /// @param[in] ver 参数版本
    /// @param[in] assayCode 项目编号
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年10月25日，新建函数
    ///
    bool InitData(const std::string& ver, int assayCode);

protected slots:
    ///
    /// @bref
    ///		点击表格中的一行的槽函数
    ///
    /// @param[in] qIndex 点击位置
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年8月31日，新建函数
    ///
    void OnTableViewClicked(const QModelIndex& qIndex);

    ///
    /// @bref
    ///		确认按钮响应（保存设置并退出）
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年10月26日，新建函数
    ///
    void OnClickedOk();

    ///
    /// @bref
    ///		用于过滤鼠标滚轮事件等
    ///
    virtual bool eventFilter(QObject *obj, QEvent *event) override;

private:

    ///
    /// @bref
    ///		初始化报警项数据
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年8月31日，新建函数
    ///
    void InitAlarmItems();

    ///
    /// @bref
    ///		界面数据报警表格初始化
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年8月31日，新建函数
    ///
    void InitAlarmTable();

    ///
    /// @bref
    ///		创建一个勾选
    ///
    /// @param[in] qIndex 勾选的位置
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年10月26日，新建函数
    ///
    void CreateCheck(const QModelIndex& qIndex);

	///
	/// @brief 添加报警复查的操作日志
	///
	/// @param[in]  alarmCfgNew  新设置的报警复查信息
	/// @param[in]  spImAssayInfo  项目中保存的报警复查信息
	/// @param[in]  strAlarmOptLog  操作日志
	///
	/// @return 
	///
	/// @par History:
	/// @li 1556/Chenjianlin，2024年3月14日，新建函数
	///
	void GetImAlarmAssayOptLog(std::vector<im::tf::autoRerunByAlarm> alarmCfgNew, const std::shared_ptr<im::tf::GeneralAssayInfo>& spImAssayInfo, QString& strAlarmOptLog);

	///
	/// @brief 项目报警信息勾选有是否有变更
	///
	/// @param[in]  alarmCfgNew  界面勾选信息
	/// @param[in]  spImAssayInfo  项目信息
	///
	/// @return 
	///
	/// @par History:
	/// @li 1556/Chenjianlin，2024年3月15日，新建函数
	///
	bool IsChangeAlarmInfo(const std::vector<im::tf::autoRerunByAlarm>& alarmCfgNew,const std::shared_ptr<im::tf::GeneralAssayInfo>& spImAssayInfo);
	
private:
    Ui::ImDataAlarmRetestSettingsDlg *ui;
    QStandardItemModel* m_model;
    QPixmap* m_selectedImg;

    int m_assayCode;
    std::string m_paramVersion;

    std::map<std::string, std::shared_ptr<DataAlarmItem>> m_alarmItem;
};
