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

#ifndef ANALYSISPARAMSINDWIDGET_H
#define ANALYSISPARAMSINDWIDGET_H

#include "model/AssayListModel.h"
#include "src/thrift/ch/gen-cpp/ch_constants.h"
#include <QWidget>


class QLineEdit;

namespace Ui {
class AnalysisParamSindWidget;
}
namespace tf {
	class QualitativeItem;
}

class AnalysisParamSindWidget : public QWidget
{
    Q_OBJECT
        
public:

	///
	/// @brief
	///     加载项目参数
	///
	///@param[in]    item 项目信息
	///
	/// @return  成功返回true   
	///
	/// @par History:
	/// @li 6950/ChenFei，2022年05月23日，新建函数
	///
	bool LoadAnalysisParam(const AssayListModel::StAssayListRowItem& item);

    ///
    /// @brief  加载定性判断
    ///     
    /// @param[in]  assayCode  项目编号
    /// @param[in]  flag       标识
    ///
    /// @return 成功返回true
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年9月8日，新建函数
    ///
    bool LoadQualitative(int assayCode, QString flag);

    ///
    /// @brief  获取定性判断
    ///     
    /// @param[in]  flag   标识
    /// @param[out] qj     定性判断
    ///
    /// @return 成功返回true
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年9月8日，新建函数
    ///
    bool GetQualitative(QString flag, ::tf::QualitativeJudge& qj);

    ///
    /// @brief  检查定性判断
    ///     
    /// @param[out] qis     定性判断列表
    ///
    /// @return 成功返回true
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年9月8日，新建函数
    ///
    bool CheckQualitative(const std::vector<::tf::QualitativeItem> & qis);

    ///
    /// @brief
    ///     保存项目信息
    ///
    ///@param[in]   item 项目信息 
    ///
    /// @return   成功返回true  
    ///
    /// @par History:
    /// @li 6950/ChenFei，2022年05月23日，新建函数
    ///
	bool SaveAnalysisParam(const AssayListModel::StAssayListRowItem& item);

    ///
    /// @brief  保存血清指数子项目的参数
    ///     
    /// @param[in]  assayCode  子项目编号
    /// @param[in]  flag  标识
    ///
    /// @return 成功返回true
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年9月11日，新建函数
    ///
    bool SaveSindSubAsssayParams(int assayCode, QString flag);

    ///
    /// @brief
    ///     当前页面是否有未保存的数据
    ///
    ///@param[in]    item 项目信息
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年4月22日，新建函数
    ///
    bool HasUnSaveChangedData();

protected:
    ///
    /// @brief
    ///     复位界面
    ///
    /// @par History:
    /// @li 6950/ChenFei，2022年05月23日，新建函数
    ///
	void Reset();

	///
	/// @brief
	///     加载分析参数界面系数A~F
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年6月20日，新建函数
	///
	bool LoaCFTToCtrls();

	///
	/// @brief	为输入框设置正则表达式
	///
	/// @return 
	///
	/// @par History:
	/// @li 7951/LuoXin，2022年10月24日，新建函数
	///
	void SetCtrlsRegExp();

    ///
    /// @brief	检查用户输入参数是否合法
    ///
    /// @return 合法返回true
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年7月6日，新建函数
    ///
    bool CheckUserInputParams();

    ///
    /// @brief
    ///     子项目是否有未保存的数据
    ///
    /// @param[in]  assayCode  子项目编号
    /// @param[in]  flag  标识
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年4月22日，新建函数
    ///
    bool SubHasUnSaveChangedData(int assayCode, QString flag);

public:
    explicit AnalysisParamSindWidget(QWidget *parent = nullptr);
    ~AnalysisParamSindWidget();


private:
    Ui::AnalysisParamSindWidget *ui;
	std::vector<QLineEdit *>	m_vecCFTEdits;							// 系数输入框A~F指针
};

#endif // ANALYSISPARAMSINDWIDGET_H
