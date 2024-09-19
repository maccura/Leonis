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
/// @file     itemresultdetaildlg.h
/// @brief    结果详情对话框
///
/// @author   4170/TangChuXian
/// @date     2022年10月13日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2022年10月13日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once
#include "shared/basedlg.h"
#include "src/thrift/gen-cpp/defs_types.h"
#include "src/thrift/im/gen-cpp/im_types.h"
#include "src/thrift/ch/gen-cpp/ch_types.h"
#include "src/thrift/ise/gen-cpp/ise_types.h"

namespace Ui { class QHistoryItemResultDetailDlg; };

class QHistoryAIIdentifyDlg;
class QPushButton;
class QLabel;
class QTabWidget;
struct HistoryBaseDataItem;

// tab翻页窗口
class TabPage : public QWidget
{
	Q_OBJECT
public:
	///
	/// @brief 构造函数
	///
	/// @param[in]  parent  
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年7月26日，新建函数
	///
	TabPage(QWidget* parent = nullptr);

	///
	/// @brief 设置总页数
	///
	/// @param[in]  iTotalPage  总页数
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年7月26日，新建函数
	///
	void SetTotalPage(int iTotalPage);

	///
	/// @brief 设置当前页数
	///
	/// @param[in]  iCurPage  当前页数
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年7月26日，新建函数
	///
	void SetCurPage(int iCurPage);
signals:
	///
	/// @brief 向前翻页按钮点击信号
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年7月26日，新建函数
	///
	void SignalPreButtonClicked();
	///
	/// @brief 向后翻页按钮点击信号
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年7月26日，新建函数
	///
	void SignalNextButtonClicked();

private:
	QPushButton*					m_preButton;				///< 向前按钮
	QPushButton*					m_nextButton;				///< 向后按钮
	QLabel*							m_totalPageLabel;			///< 总页数标签
	QLabel*							m_curPageLabel;				///< 当前页标签
	int								m_iTotalPage;				///< 总页数
	int								m_iCurPage;					///< 当前页数
};

class QHistoryItemResultDetailDlg : public BaseDlg
{
    Q_OBJECT

public:
    QHistoryItemResultDetailDlg(QWidget *parent = Q_NULLPTR);
    ~QHistoryItemResultDetailDlg();

    ///
    /// @brief
    ///     清空内容
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年10月26日，新建函数
    ///
    void ClearContents();

	///
	/// @brief 更新结果详情
	///   
	/// @param[in]  data
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年1月6日，新建函数
	///
	void Update(const std::tuple<int64_t, std::vector<std::shared_ptr<HistoryBaseDataItem>>, bool>& data);

	///
	/// @brief 设置上一条数据按钮是否可用
	///
	/// @param[in]  bPreEnable  
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年1月9日，新建函数
	///
	void SetPreBtnEnable(bool bPreEnable);

	///
	/// @brief 设置下一条数据按钮是否可用
	///
	/// @param[in]  bNextEnable  
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年1月9日，新建函数
	///
	void SetNextBtnEnable(bool bNextEnable);

protected:
    ///
    /// @brief
    ///     清空选中项信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年10月20日，新建函数
    ///
    void ClearSelItemInfo();

    ///
    /// @brief
    ///     清空病人信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年10月20日，新建函数
    ///
    void ClearPatientInfo();

protected:
    ///
    /// @brief
    ///     显示前初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年10月13日，新建函数
    ///
    void InitBeforeShow();

    ///
    /// @brief
    ///     显示之后初始化
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年10月13日，新建函数
    ///
    void InitAfterShow();

    ///
    /// @brief
    ///     初始化子控件
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年10月13日，新建函数
    ///
    void InitChildCtrl();

    ///
    /// @brief
    ///     初始化连接
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年10月13日，新建函数
    ///
    void InitConnect();

    ///
    /// @brief
    ///     重写显示事件
    ///
    /// @param[in]  event  
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 1226/zhangjing，2024年9月2日，新建函数
    ///
    virtual void showEvent(QShowEvent* event) override;

Q_SIGNALS:
    ///
    /// @brief
    ///     上一条按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年10月13日，新建函数
    ///
    void SigPrevBtnClicked();

    ///
    /// @brief
    ///     下一条按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年10月13日，新建函数
    ///
    void SigNextBtnClicked();

    ///
    /// @brief 当前数据被修改
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年11月19日，新建函数
    ///
    void dataChanged();

	///
	/// @brief 复查结果发生改变
	///
	/// @param[in]  vol  元组对象，0-复查信号值，1-复查状态，2-复查结果对应结果表ID
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年7月4日，新建函数
	///
	void SigRetestResultChanged(std::tuple<std::string,std::string,int64_t> vol);

protected Q_SLOTS:
    ///
    /// @brief
    ///     删除按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年10月13日，新建函数
    ///
    void OnDelBtnClicked();

    ///
    /// @brief
    ///     保存按钮被点击
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年10月13日，新建函数
    ///
    void OnSaveBtnClicked();

    ///
    /// @brief 恢复图例
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年12月6日，新建函数
    ///
    void OnResetCurve();

    ///
    /// @brief
    ///     项目结果表当前选中项改变
    ///
    /// @param[in]  curIndex 当前选中项索引
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年10月20日，新建函数
    ///
    void OnItemTblCurIndexChanged(const QModelIndex& curIndex);

	///
	/// @brief 设置为复查结果
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年7月3日，新建函数
	///
	void OnSetRetestResult();

	///
	/// @brief AI识别结果
	///
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年7月24日，新建函数
	///
	void OnAIIdentify();

 private:

	///
	/// @brief 初始化病人信息
	///
	/// @param[in]  patientInfo  
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年1月4日，新建函数
	///
	void InitPatientInfo(const ::tf::PatientInfo& patientInfo);

	///
	/// @brief 显示免疫项目结果
	///
	/// @param[in]  result  免疫项目结果
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年2月3日，新建函数
	///
	void ShowIMResult(const std::vector<im::tf::AssayTestResult>& result, const std::shared_ptr<HistoryBaseDataItem>& historyItem, QTabWidget* pImTabWgt, int64_t retestId);

	///
	/// @brief 显示生化项目结果
	///
	/// @param[in]  result  生化项目结果
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年2月3日，新建函数
	///
	void ShowCHResult(const std::vector<ch::tf::AssayTestResult>& result, const std::shared_ptr<HistoryBaseDataItem>& historyItem, QTabWidget* pChTabWgt, int64_t retestId);

	///
	/// @brief 显示ISE项目结果
	///
	/// @param[in]  result  ISE项目结果
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2023年2月3日，新建函数
	///
	void ShowISEResult(const std::vector<ise::tf::AssayTestResult>& result, const std::shared_ptr<HistoryBaseDataItem>& historyItem, QTabWidget* pIseTabWgt, int64_t retestId);

	///
	/// @brief 根据是否是病人样本类型初始化结果表
	///
	/// @param[in]  bIsPatient  
	///
	/// @return 
	///
	/// @par History:
	/// @li 7702/WangZhongXin，2024年2月1日，新建函数
	///
	void InitAssayResTable(bool bIsPatient);

    inline QString TranseConc(double dConc)
    {
        std::string resultStr = std::to_string(dConc);
        // 浮点数转换为字符串后末尾可能会有多余的0，需要去除
        resultStr.erase(resultStr.find_last_not_of('0') + 1, std::string::npos);
        resultStr.erase(resultStr.find_last_not_of('.') + 1, std::string::npos);

        return QString::fromStdString(resultStr);
    }

    ///
    /// @brief  设置lable显示
    ///
    /// @param[in]  lab  lab控件
    /// @param[in]  labTitle  lab控件(标题)
    /// @param[in]  strItemName  字符串
    ///
    /// @return 
    ///
    /// @par History:
    /// @li 1226/zhangjing，2024年9月2日，新建函数
    ///
    void SetItemName(QLabel *lab, QLabel *labTitle, const QString& strItemName);

    ///
    /// @brief 转换字符串
    ///
    /// @param[in]  src  源字符串
    /// @param[in]  dst  目的格式
    /// @param[in]  width  宽度
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2024年1月30日，新建函数
    ///
    void CutTipstring(const QString& src, QString& dst, int width);

 private:
    Ui::QHistoryItemResultDetailDlg *ui;                   // UI对象指针

	std::tuple<int64_t, std::vector<std::shared_ptr<HistoryBaseDataItem>>, bool>	m_mHistoryDetailData;

	QHistoryAIIdentifyDlg*			m_AIIdentifyDlg;
    ::tf::PatientInfo               m_patientInfo;

	bool							m_bInit;		///< 是否初始化病人信息
};
