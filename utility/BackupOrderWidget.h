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
/// @file     BackupOrderWidget.h
/// @brief    应用--检测--备用订单
///
/// @author   7951/LuoXin
/// @date     2022年9月20日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 7951/LuoXin，2022年9月20日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include <QLabel>
#include <set>

class BackupOrderAddAssayDlg;
class CustomLab;
class QPushButton;
namespace Ui {
    class BackupOrderWidget;
};

// 自定义标签
class CustomLab : public QLabel
{
	Q_OBJECT

public:
	CustomLab(int assayCode,QString assayName,QWidget *parent = Q_NULLPTR);

	///
	/// @brief	获取项目编号
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年12月22日，新建函数
	///
	int GetAssayCode() { return m_assayCode; };

	///
	/// @brief	设置按钮可点击区域大小
	///     
	///
	/// @param[in]  size  可点击区域大小
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年12月22日，新建函数
	///
	void setBtnSize(int size);
private:

	///
	/// @brief	初始化
	///
	/// @param[in]  assayName  项目名称
	///
	/// @par History:
	/// @li 7951/LuoXin，2023年1月9日，新建函数
	///
	void Init(QString assayName);

signals:
	void checked();

private:	
	QPushButton*		m_btn;
	QLabel*				m_label;
	int					m_assayCode;			// 项目编号
};

class BackupOrderWidget : public QWidget
{
	Q_OBJECT

public:
	BackupOrderWidget(QWidget *parent = Q_NULLPTR);
	~BackupOrderWidget();

	///
	/// @brief	加载数据到控件
	///     
	/// @return 
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年9月20日，新建函数
	///
	void LoadDataToCtrls();

protected:
	///
	/// @brief 窗口显示事件
	///     
	/// @param[in]  event  事件对象
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年9月20日，2020年4月27日，新建函数
	///
	void showEvent(QShowEvent *event);

private:
	///
	/// @brief	初始化控件
	///     
	/// @par History:
	/// @li 7951/LuoXin，2022年9月20日，新建函数
	///
	void InitCtrls();

	///
	/// @brief	添加按钮到窗口中
	///     
	/// @param[in]  text	显示的文字
	/// @param[in]  code	项目编号
	/// @param[in]  widget  窗口指针
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年12月22日，新建函数
	///
	void AddLabToWidget(const QString& text, int code, QWidget* widget);

	///
	/// @brief	更新所有的按钮的位置
	///     
	/// @param[in]  widget		窗口指针
	///
	/// @return 
	///
	/// @par History:
	/// @li 7951/LuoXin，2023年1月15日，新建函数
	///
	void UpDateAllLabPos(QWidget* widget);

	///
	/// @brief	显示配置的项目信息
	///     
	/// @param[in]  assayCodes  常规项目
    /// @param[in]  sampleSource  样本源类型
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年9月22日，新建函数
	///
	void ShowConfigAssay(std::vector<int32_t>& assayCodes, int sampleSource);

	///
	/// @brief	删除标签
	///  
	/// @param[in]  delLab	待删除的标签
	/// 
	/// @par History:
	/// @li 7951/LuoXin，2022年12月23日，新建函数
	///
	void DeleteLab(CustomLab* delLab);

protected Q_SLOTS:
	///
	/// @brief	确定按钮被点击
	///     
	/// @par History:
	/// @li 7951/LuoXin，2022年9月20日，新建函数
	///
	void OnSaveBtnClicked();

	///
	/// @brief	加载添加的项目到界面
	///     
	/// @par History:
	/// @li 7951/LuoXin，2022年9月21日，新建函数
	///
	void OnSaveAddAssay();

    ///
    /// @brief	删除项目
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2024年3月7日，新建函数
    ///
    void OnDeleteAssay();

    ///
    /// @bref
    ///		响应项目信息发生变更
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年7月25日，新建函数
    ///
    void OnProcAssayInfoUpdate();

    ///
    /// @brief	相应项目添加按钮被点击
    ///     
    /// @par History:
    /// @li 7951/LuoXin，2023年9月18日，新建函数
    ///
    void OnAddAssayBtnClicked();

    ///
    /// @brief	更新控件的显示和编辑权限
    ///    
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年3月27日，新建函数
    ///
    void UpdateCtrlsEnabled();

private:
	Ui::BackupOrderWidget*				    ui;
	BackupOrderAddAssayDlg*					m_BackupOrderAddAssayDlg;	// 添加项目弹窗
	int										m_currentSampleSourceType;	// 当前操作的样本源类型
	bool									m_bInit;					// 是否初始化
    std::map<int, std::set<int64_t>>	    m_sampleSrcType2Profiles;   // 组合项目信息
};
