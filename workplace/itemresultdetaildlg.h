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
#include "boost/optional.hpp"

class QAssayResultView;
class QTabWidget;
namespace Ui { class QHistoryItemResultDetailDlg; };

class ItemResultDetailDlg : public BaseDlg
{
    Q_OBJECT

public:
    ItemResultDetailDlg(QWidget *parent = Q_NULLPTR);
    ~ItemResultDetailDlg();

    ///
    /// @brief 设置需要显示的样本信息和项目信息
    ///
    /// @param[in]  sampId      样本信息的主键
    /// @param[in]  testItems   项目信息的主键列表
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年11月18日，新建函数
    ///
    void SetSampleId(int64_t sampId, std::vector<int64_t>& testItems, bool isFirst, bool isLast);

    ///
    /// @brief
    ///     获取样本数据库主键
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年10月26日，新建函数
    ///
    inline std::int64_t  GetSampleId() { return m_lSampId; };

    ///
    /// @brief
    ///     清空内容
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年10月26日，新建函数
    ///
    void ClearContents();

protected:
    ///
    /// @brief 按键事件
    ///
    /// @param[in]  event  事件
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年7月3日，新建函数
    ///
    void keyPressEvent(QKeyEvent* event);

    ///
    /// @brief
    ///     清空选中生化项目项目详情信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年10月20日，新建函数
    ///
    void ClearSelChItemDetail();

    ///
    /// @brief
    ///     清空选中生化项目项目详情信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年10月20日，新建函数
    ///
    void ClearSelImItemDetail();

    ///
    /// @brief
    ///     清空病人信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年10月20日，新建函数
    ///
    void ClearPatientInfo();

	///
	/// @brief 事件过滤
	///
	/// @param[in]  target  控件
	/// @param[in]  event   事件
	///
	/// @return true:成功
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年9月19日，新建函数
	///
	bool eventFilter(QObject* target, QEvent* event);

	///
	/// @brief 根据要求筛选出需要显示的结果信息
	///1：若当前没有已经测试完成的结果，要求显示信息
	///2：当前已经有测试完成的结果，但是复查结果还未出，不能显示未完成的结果信息
	///3：当前已经有测试完成的结果信息，但是结果无法计算，不允许显示当前信息
	///4：当前已经有测试完成的结果信息，但是结果无法计算，并且只有一个结果信息，依然要求显示
	/// @param[in]  rawAssayResultVec  原始结果信息
	///
	/// @return 筛选后的结果信息
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年4月25日，新建函数
	///
	template<typename T>
	std::vector<T> GennerateAssayResult(const std::vector<T>& rawAssayResultVec)
	{
		std::vector<T> dstResult;
		// 获取显示要的页面数目（bug0011480)
		// 清除页面 removeTab方法不能删除页面
		for (const auto& assayResult : rawAssayResultVec)
		{
			if (!assayResult.__isset.endTime)
			{
				continue;
			}

			// 若结果无法计算则不显示
			// bug0011863
			if (dstResult.empty() && assayResult.resultStatusCodes.find(u8"Calc.?") != std::string::npos)
			{
				continue;
			}

			dstResult.push_back(assayResult);
		}

		// 若经过筛选（结果无法计算的项目的不显示，导致没有结果可以显示），则显示最后一个
		if (dstResult.empty() && !rawAssayResultVec.empty())
		{
			// 获取最后一个项目
			int size = rawAssayResultVec.size();
			auto assayResult = rawAssayResultVec[size-1];
			// 同时要求也是有时间的
			if (assayResult.__isset.endTime)
			{
				dstResult.push_back(assayResult);
			}
		}

		if (dstResult.empty())
		{
			T tmpResult;
			tmpResult.__set_id(-1);
			tmpResult.__set_endTime("");
			dstResult.push_back(tmpResult);
		}

		return std::move(dstResult);
	};

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
    ///     根据结果关键信息获取项目结果(可考虑使用通用项目结果类型，使UI与设备类型隔离)
    ///
    /// @param[in]  stuResultKey  结果关键信息
    /// @param[out] pVarRlt       项目测试结果信息
    ///
    /// @return 项目结果
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年10月19日，新建函数
    ///
    QString GetItemResult(const tf::TestResultKeyInfo& stuResultKey, QVariant* pVarRlt = Q_NULLPTR);

    ///
    /// @brief
    ///     更新选中生化项目详情信息
    ///
    /// @param[in]  curIndex        选中项索引
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年10月21日，新建函数
    ///
    void UpdateSelChItemDetailInfo(const QModelIndex& curIndex);

    ///
    /// @brief
    ///     更新选中免疫项目详情信息
    ///
    /// @param[in]  curIndex        选中项索引
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年10月21日，新建函数
    ///
    void UpdateSelImItemDetailInfo(const QModelIndex& curIndex);

    ///
    /// @brief
    ///     更新选中项目详情信息
    ///
    /// @param[in]  curIndex        选中项索引
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年10月21日，新建函数
    ///
    void UpdateSelItemDetailInfo(const QModelIndex& curIndex);

    ///
    /// @brief
    ///     重写显示事件
    ///
    /// @param[in]  event  事件
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年10月13日，新建函数
    ///
    virtual void showEvent(QShowEvent* event) override;

    ///
    /// @brief 显示病人信息
    ///
    /// @param[in]  sample  样本信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年11月18日，新建函数
    ///
    void DisplayPatientInfo(const tf::SampleInfo& sample);

    ///
    /// @brief 显示项目信息
    ///
    /// @param[in]  testItemInfo  项目信息
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年11月18日，新建函数
    ///
    void DisplayTestItem(const tf::TestItem& testItemInfo, int iRow);

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
    /// @bref
    ///		AI识别结果按钮点击槽函数
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年7月28日，新建函数
    ///
    void OnAIIdentify();

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
    /// @brief
    ///     设置哪个结果为当前复查结果
    ///
    void OnSetCurrentRetestResult();

    ///
    /// @brief
    ///     更新样本信息
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年10月26日，新建函数
    ///
    void UpdateSampleInfo();

    ///
    /// @bref
    ///		把详细结果显示到UI
    ///
    /// @param[in] resultItems 结果集
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年5月10日，新建函数
    ///
    void ShowTestResultItem(const std::vector<im::tf::AssayTestResult>& resultItems, const tf::TestItem& stuTestItem, bool bVolEditEnable);

private:
	Ui::QHistoryItemResultDetailDlg *ui;                   // UI对象指针
    bool                     m_bInit;				// 是否初始化
	bool					 m_delete;				// 是否允许删除
	bool					 m_reset;				// 是否允许复查
	bool					 m_fromBlank;		    // 是否从空白区域到数据区域
	bool				     m_modifyData;			//< 是否修改过数据
    QAssayResultView*        m_actionDiglog;        // 曲线句柄
    QTabWidget*              m_pChTabWidget;        // 生化当前详情句柄

    // 样本信息
    std::int64_t             m_lSampId;            // 样本数据库主键
    std::vector<int64_t>     m_testItems;          // 项目列表
    tf::SampleInfo           m_sampleInfo;         // 当前样本
	boost::optional<int>	 m_curRow;			   // 当前项目
	boost::optional<int>	 m_curColumn;		   // 第几次复查结果
    std::int64_t             m_lItemId;            // 当前选中的itemid
};
