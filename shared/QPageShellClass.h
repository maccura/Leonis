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
/// @file     QPageShellClass.h
/// @brief    生化免疫试剂外壳页面
///
/// @author   5774/WuHongTao
/// @date     2022年1月13日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年1月13日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include <QStackedWidget>
#include <map>
#include "src/thrift/gen-cpp/defs_types.h"

class QRadioButton;
class QTabWidget;
class QPushButton;

// 待生成按钮的信息
struct CornerButtonInfo 
{
    CornerButtonInfo(int propNum, const QString& objName, const QString& showText)
        : m_propertyNumber(propNum), m_objectName(objName), m_buttonText(showText)
    {}
    int m_propertyNumber;
    QString m_objectName;
    QString m_buttonText;
};


class QPageShellClass : public QStackedWidget
{
    Q_OBJECT
   
public:
    QPageShellClass(QWidget *parent);
    ~QPageShellClass();

    ///
    /// @brief
    ///     刷新右上角按钮
    ///
    /// @param[in]  index  
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年1月12日，新建函数
    ///
    void RefreshCornerWidget(int index);

    void showEvent(QShowEvent *event);

    ///
    /// @bref
    ///		获取当前页面类型
    ///
    /// @par History:
    /// @li 8276/huchunli, 2022年11月19日，新建函数
    ///
    virtual int CurrentPageType() = 0;

    ///
    /// @brief
    ///     生成右上角按钮控件
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年1月13日，新建函数
    ///
    void SetCornerButtonWidget(const std::vector<CornerButtonInfo>& pageInfos);

    ///
    /// @brief
    ///     设置选择按钮的状态
    ///
    /// @param[in]  status  状态
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月23日，新建函数
    ///
    void HideSelectButton(bool status);

	void SetSelectByDevSn(const std::string &devSn);

signals:
    void ChangePage(int index);


    ///
    /// @brief
    ///     显示提示消息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年1月13日，新建函数
    ///
    void ShowTipMessage(QString msg);

    ///
    /// @brief
    ///     关闭查询筛选提示条
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年3月14日，新建函数
    ///
    void CloseFilterLable();
    void HideFilterLable();

protected:
    ///
    /// @brief
    ///     获取设备信息列表
    ///
    ///
    /// @return 设备信息列表
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年2月18日，新建函数
    ///
  virtual std::vector<std::shared_ptr<const tf::DeviceInfo>> GetDeviceInfos();

  ///
  /// @brief 生成设备按钮
  ///
  ///
  /// @par History:
  /// @li 5774/WuHongTao，2022年9月22日，新建函数
  ///
  virtual void GenDeviceButton();

protected slots :
    ///
    /// @brief 选中生化和免疫的切换
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年9月22日，新建函数
    ///
    virtual void OnSelectButton() = 0;

    ///
    /// @brief 选中不同的功能页
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年9月22日，新建函数
    ///
    virtual void OnSelectPage() = 0;

protected:
    ///
    /// @brief
    ///     根据页面类型，生成不同的页面
    ///
    /// @param[in]  type  页面类型
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年1月21日，新建函数
    ///
    virtual void CreatSubPageByPageType(int type) = 0;

    ///
    /// @brief
    ///     根据按钮信息获取设备信息
    ///
    /// @param[out]  devices  设备信息
    ///
    /// @return true表示成功，反之失败
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年2月21日，新建函数
    ///
    bool GetDevicesFromButton(std::vector<std::shared_ptr<const tf::DeviceInfo>>& devices);

    virtual QPushButton* GetCurrentPageButton(int buttonNum);

protected:
    QTabWidget*                     m_parent;                   ///< 导航页面句柄
    QWidget*                        m_cornerItem;               ///< 切换句柄
    std::vector<QRadioButton*>      m_buttons;                  ///< 按钮列表
	std::vector<QPushButton*>		m_deviceClassifyBtns;		///< 设备项目类型	按钮表	
    std::map<QRadioButton*, std::vector<std::shared_ptr<const tf::DeviceInfo>>> m_DeviceButton;             ///< 按钮对应的设备信息列表
    bool                            m_Isall;                    ///< 是否全部
    bool                            m_IsNeedShow;               ///< 是否需要显示
	bool							m_IsIse;					///< 是否需要显示ise
    bool                            m_showItem;                 ///< 是否显示控件
    bool                            m_deviceTypeShow;           ///< 设备类型显示
};
