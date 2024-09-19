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
/// @file     ImCalibrateSettingWidget.h
/// @brief    项目设置-校准
///
/// @author   1226/ZhangJing
/// @date     2023年2月17日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 1226/ZhangJing，2023年2月17日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#ifndef IMCALIBRATESETTINGWIDGET_H
#define IMCALIBRATESETTINGWIDGET_H

#include <memory>
#include "model/AssayListModel.h"
#include <QWidget>
#include "src/thrift/im/gen-cpp/im_types.h"

namespace Ui {
    class ImCalibrateSettingWidget;
};

namespace im {
    namespace tf {
        class GeneralAssayInfo;
    };
};


class ImCalibrateSettingWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ImCalibrateSettingWidget(QWidget *parent = nullptr);
    ~ImCalibrateSettingWidget();

    ///
    /// @brief
    ///     加载指定项目的分析参数
    ///
    /// @param[in]  item  数据
    ///
    /// @par History:
    /// @li 1226/zhangjing，2023年2月17日，新建函数
    ///
    bool LoadAnalysisParam(const AssayListModel::StAssayListRowItem& item);

    ///
    /// @brief
    ///     保存项目信息
    ///
    ///@param[in]   item 项目信息 
    ///
    /// @return   成功返回true  
    ///
    /// @par History:
    /// @li 1226/zhangjing，2023年2月17日，新建函数
    ///
    bool GetAnalysisParam(const AssayListModel::StAssayListRowItem& item, std::vector<std::shared_ptr<im::tf::GeneralAssayInfo>>& vecImAssayInfo);

    ///
    /// @brief 窗口显示事件
    ///     
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 1226/zhangjing，2023年4月27日，新建函数
    ///
    void showEvent(QShowEvent *event);

    ///
    /// @brief
    ///     复位界面
    ///
    /// @par History:
    /// @li 1226/zhangjing，2023年2月17日，新建函数
    ///
    void Reset();

    ///
    /// @brief
    ///     初始化控件
    ///
    /// @par History:
    /// @li 1226/zhangjing，2023年2月17日，新建函数
    ///
    void InitCtrls();

    ///
    /// @brief 为输入框设置正则表达式
    ///
    /// @par History:
    /// @li 1226/zhangjing，2023年2月17日，新建函数
    ///
    void SetCtrlsRegExp();

	///
	/// @brief 获取更改操作细节信息
	///
	/// @param[in]  strOptLog  返回的日志记录信息
	/// @param[in]  spAssayInfoUi  界面修改的项目信息
	/// @param[in]  vecImAssayInfoUi  界面修改的免疫项目信息
	///
	/// @return 
	///
	/// @par History:
	/// @li 1556/Chenjianlin，2024年3月7日，新建函数
	///
	void GetImAssayOptLog(QString& strOptLog, const std::shared_ptr<tf::GeneralAssayInfo> spAssayInfoUi, const std::vector<std::shared_ptr<im::tf::GeneralAssayInfo>>& vecImAssayInfoUi);


protected Q_SLOTS:

    ///
    /// @brief
    ///     槽函数，设置表格行数
    ///
    /// @par History:
    /// @li 1226/zhangjing，2023年2月17日，新建函数
    ///
    void OnsetTableViewRows(const QString& text);

    ///
    /// @bref
    ///		权限变化响应
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年7月22日，新建函数
    ///
    void OnPermisionChanged();

private:
    Ui::ImCalibrateSettingWidget        *ui;
    AssayListModel::StAssayListRowItem	m_rowItem;				// 当前选中行信息
};

#endif // IMCALIBRATESETTINGWIDGET_H
