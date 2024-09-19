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
/// @file     ImAnalysisparamwidget.h
/// @brief    项目设置-分析
///
/// @author   1226/ZhangJing
/// @date     2023年2月16日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 1226/ZhangJing，2023年2月16日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once

#include "model/AssayListModel.h"
#include <QWidget>
#include <QMap>
#include <memory>
#include "src/thrift/im/gen-cpp/im_types.h"
#include <functional>

class QLineEdit;
namespace Ui { class ImAnalysisParamWidget; };

namespace im
{
    namespace tf
    {
        class GeneralAssayInfo;
    }
};

class ImAnalysisParamWidget : public QWidget
{
    Q_OBJECT

public:
    ImAnalysisParamWidget(QWidget *parent = Q_NULLPTR);
    ~ImAnalysisParamWidget();

    enum UiCtrlValidError {
        // 基础信息
        UV_NO_ERROR,
        UV_BASE_BRIF_NAME_EMPTY, // 简称不能为空
        UV_BASE_BRIF_NAME_TOOLENGTH, // 简称过长
        UV_BASE_BRIF_NAME_EXIST,    // 简称已经存在
        UV_BASE_DECIMAL_VALUE_ERROR, // 值错误：小数位
        UV_BASE_VALUE_LINE_LOWER,   // 值错误：线性范围的低值设置范围[0,999999].
        UV_BASE_VALUE_LINE_UPPER,   // 值错误：线性范围的高值设置范围[0,999999].
        UV_BASE_LINE_RANGE,         // 范围错误：线性范围低值不能大于高值.

        // 定量项目阴阳性阈值
        UV_PN_SYMBO_POS,            // 值错误：阳值符号设置
        UV_PN_VALUE_POS,            // 值错误：阳值设置范围[0,999999].
        UV_PN_SYMBO_NEG,            // 值错误：阴值符号设置
        UV_PN_VALUE_NEG,            // 值错误：阴值设置范围[0,999999].
        UV_PN_REPEAT_RANGE,         // 值错误：阳值和阴值存在重复区间.
        // 自定义提醒
        UV_CUSTOM_SYMBO1,            // 值错误：自定义提醒1符号设置
        UV_CUSTOM_RANGE1,            // 值错误：自定义提醒1值设置范围[0,999999].
        UV_CUSTOM_SYMBO2,            // 值错误：自定义提醒2符号设置
        UV_CUSTOM_RANGE2,            // 值错误：自定义提醒2值设置范围[0,999999].
        UV_CUSTOM_RANGE3,            // 值错误：自定义提醒3值设置范围[0,999999].

        // 自动复查
        UV_AUTORETEST_VALUE_LOWER,  // 值错误：自动复查范围1低值设置范围[0,999999].
        UV_AUTORETEST_VALUE_UPPER,  // 值错误：自动复查范围1高值设置范围[0,999999].
        UV_AUTORETEST_VALUE2_LOWER,  // 值错误：自动复查范围2低值设置范围[0,999999].
        UV_AUTORETEST_VALUE2_UPPER,  // 值错误：自动复查范围2高值设置范围[0,999999].

        UV_DILU_DEFAULT_OVER_MAX,   // 值错误：默认稀释倍数不能大于最大稀释倍数.

        // 其它错误，不需要弹框提示的错误
        UV_ERROR_NO_UNITFACTOR,     // 找不到单位倍率
        UV_ERROR_NO_IMGENERALASSAY, // 找不到免疫项目信息
    };

    // 回调设置参数版本, 当切换版本时改变current param 参数入参版本
    std::function<void(const QString& strVer)> m_funSetVersion;
    void BindSetVersionCallBack(std::function<void(const QString& strVer)> funSetVersion)
    {
        m_funSetVersion = funSetVersion;
    }

    ///
    /// @brief
    ///     加载指定项目的分析参数
    ///
    /// @param[in]  item  数据
    ///
    /// @par History:
    /// @li 1226/zhangjing，2023年2月16日，新建函数
    ///
    void LoadAnalysisParam(const AssayListModel::StAssayListRowItem& item);

    ///
    /// @brief
    ///     保存参数到数据库
    ///
    ///@param[in]   被保存项目的定位信息
    ///
    /// @return     成功返回true
    ///
    /// @par History:
    /// @li 1226/zhangjing，2023年2月16日，新建函数
    ///
    bool GetAnalysisParam(const AssayListModel::StAssayListRowItem& item, std::shared_ptr<tf::GeneralAssayInfo>& pGen, \
        std::vector<std::shared_ptr<::im::tf::GeneralAssayInfo>>& vecImAssayInfo, std::vector<std::shared_ptr<tf::QcDoc>>& changedQc);

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

protected:
    ///
    /// @brief 界面显示后初始化
    ///
    /// @par History:
    /// @li 1226/zhangjing，2023年2月16日，新建函数
    ///
    void InitAfterShow();

    ///
    /// @brief
    ///     初始化子控件
    ///
    /// @par History:
    /// @li 1226/zhangjing，2023年2月16日，新建函数
    ///
    void InitChildCtrl();

    ///
    /// @brief	输入框样式设置
    ///
    /// @par History:
    /// @li 1226/zhangjing，2023年2月16日，新建函数
    ///
    void SetCtrlsRegExp();

    ///
    /// @brief 窗口显示事件
    ///     
    /// @param[in]  event  事件对象
    ///
    /// @par History:
    /// @li 1226/zhangjing，2023年2月16日，新建函数
    ///
    void showEvent(QShowEvent *event);

    ///
    /// @brief
    ///     复位控件
    ///
    /// @par History:
    /// @li 1226/zhangjing，2023年2月16日，新建函数
    ///
    void Reset();


    protected Q_SLOTS:

    ///
    /// @brief  当前版本切换
    ///
    /// @param[in]  text  新版本号
    ///
    /// @par History:
    /// @li 1226/zhangjing，2023年2月16日，新建函数
    ///
    void OnAssayVerChanged(const QString &text);

    ///
    /// @brief  单位切换
    ///
    /// @param[in]  text  切换的新单位
    ///
    /// @par History:
    /// @li 1226/zhangjing，2023年3月28日，新建函数
    ///
    bool OnUnitChanged(const QString &text);

    ///
    /// @bref
    ///		精度下拉框变化
    ///
    /// @param[in] text 变化后的内容
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年11月14日，新建函数
    ///
    bool OnDecimalChanged(const QString& text);

    ///
    /// @bref
    ///		权限变化响应
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年9月22日，新建函数
    ///
    void OnPermisionChanged();

    ///
    /// @bref
    ///		响应单位删除
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年12月6日，新建函数
    ///
    void UpdateUnitCombbox();

private:

    ///
    /// @bref
    ///		获取项目对应精度
    ///
    /// @param[in] iAssayCode 项目编号
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年8月18日，新建函数
    ///
    int GetPrecion(int iAssayCode);

    ///
    /// @bref
    ///		检测界面上所有控件的值的合法性
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年8月19日，新建函数
    ///
    UiCtrlValidError CheckCtrlValue(int assayCode, ::tf::DeviceType::type devType, const std::string& ver);

    ///
    /// @bref
    ///		检查界面的基础信息
    ///
    /// @param[in] assayCode 项目编号
    /// @param[in] factor 单位倍率
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年8月21日，新建函数
    ///
    UiCtrlValidError CheckCtrlValueBaseParameter(int assayCode, double factor);

    ///
    /// @bref
    ///		检查界面的阴阳判定控件值
    ///
    /// @param[in] factor 单位倍率
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年8月21日，新建函数
    ///
    UiCtrlValidError CheckCtrlValuePosNeg(double factor);

    ///
    /// @bref
    ///		检查切换单位时，各个界面值的合法性
    ///
    /// @param[in] changedFactor 切换的单位系数
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年8月21日，新建函数
    ///
    UiCtrlValidError CheckCtrlValueUnitChanged(const std::shared_ptr<im::tf::GeneralAssayInfo>& pImAssayInfo, double factor);

    ///
    /// @bref
    ///		初始化控件输入值错误描述提示
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年8月21日，新建函数
    ///
    void InitCtrErrorDescription();

    ///
    /// @bref
    ///		获取免疫项目信息，并对版本下拉框赋值
    ///
    /// @param[in] iAssayCode 指定的项目编码
    /// @param[in] devType 指定的设备类型
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年8月23日，新建函数
    ///
    std::shared_ptr<im::tf::GeneralAssayInfo> GetImAssayInfoAndInitVerComb(int iAssayCode, ::tf::DeviceType::type devType, const QString& strVer);

    ///
    /// @bref
    ///	    赋值单位系数相关的编辑控件
    ///
    /// @param[in] pImAssayInfo 免疫项目信息
    /// @param[in] factor 单位系数
    /// @param[in] iPrecision 显示精度
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年8月23日，新建函数
    ///
    void AssignFactorAboutEdit(const std::shared_ptr<im::tf::GeneralAssayInfo>& pImAssayInfo, double factor, int iPrecision);

    ///
    /// @bref
    ///		保存项目的基础参数
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年8月23日，新建函数
    ///
    bool GetGeneralAssayInfo(std::shared_ptr<tf::GeneralAssayInfo>& spAssayInfo, std::vector<std::shared_ptr<tf::QcDoc>>& changedQc);

    ///
    /// @bref
    ///		更新界面显示的参数
    ///
    /// @param[in] strUnit 单位
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年11月14日，新建函数
    ///
    bool UpdateParameteShow(const QString& strUnit);

private:
    Ui::ImAnalysisParamWidget		*ui;                        /// UI指针
    bool							m_bInit;                    /// 是否已经初始化
    bool                            m_bInitVerCom;              /// 是否正在初始化下拉菜单
    bool							m_bSampleTypeFlag;          /// 是否设置样本类型(默认false，备用信息)
    // 设置友元类
    friend class					AssayConfigWidget;          /// 将仪器设置窗口设置为友元类
    AssayListModel::StAssayListRowItem	m_rowItem;				// 当前选中行信息
    QString                         m_curUnit;                  // 当前使用单位

    std::map<UiCtrlValidError, QString> m_ctrlErrorDescription; // 控件合法性检查的错误描述
};
