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
/// @file     QAssayTabWidgetContainer.cpp
/// @brief    测试项目选择页面容器
///
/// @author   5774/WuHongTao
/// @date     2022年5月18日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月18日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include <QWidget>
#include <boost/optional.hpp>
#include "src/thrift/gen-cpp/defs_types.h"
#include "shared/QAssaySelectButton.h"


#define ASSAY_ROW_AMOUNT_TOTAL      7   /// 一页的总行数

class QRadioButton;
class QAssayTabWidgets;
namespace Ui { class QAssayTabWidgetContainer; };

// 页面返回信息类型
typedef std::pair<tf::AssayClassify::type, int> AssayPageRetInfo;

class QAssayTabWidgetContainer : public QWidget
{
    Q_OBJECT

public:
    QAssayTabWidgetContainer(QWidget *parent = Q_NULLPTR);
    ~QAssayTabWidgetContainer();

    struct AssayPageKeyInfo
    {
        QString			  buttonName;
        tf::AssayClassify::type devType;
        QRadioButton*	  devButton;
        QAssayTabWidgets* assayTable;
        AssayType pageType;
    };

    ///
    ///  @brief 初始化
    ///
    ///
    ///
    ///  @return	
    ///
    ///  @par History: 
    ///  @li 7656/zhang.changjiang，2023年6月30日，新建函数
    ///
    void Init();

    ///
    /// @brief 设置当前样本类型（血清，血浆，尿，全血等等 ）
    ///
    /// @param[in]  currentType  样本类型
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年7月20日，新建函数
    ///
    inline void SetCurrentSampleType(int currentType)
    {
        m_currentSampleType = currentType;
    };

    ///
    /// @brief 获取修改列表
    ///
    ///
    /// @return 获取修改列表
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年6月6日，新建函数
    ///
    inline std::map<int, AssayPageRetInfo>&& GetPostionChangeList()
    {
        return std::move(m_assayPostionMap);
    }

    ///
    /// @brief 设置单项目位置信息
    ///
    /// @param[in]  assayCode  编码
    ///
    /// @return true表示设置成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年6月2日，新建函数
    ///
    bool SetAssayCardInfo(int assayCode);

    void ClearSingalAssayCardInfo();
    void ClearAllAssayCardInfo();
    void ClearAllBtnSelectStat();

    ///
    /// @bref
    ///		删除指定的项目卡片信息
    ///
    /// @param[in] assayName 项目名
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年5月10日，新建函数
    ///
    void DeleteAssayAttriCard(const QString& assayName);

    ///
    /// @brief 设置稀释方式
    ///
    /// @param[in]  absortSize  稀释方式
    /// @param[in]  dulitions   稀释倍数
    ///
    /// @return true设置成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年9月14日，新建函数
    ///
    bool SetDilution(SampleSize absortSize, int dulitions);

    ///
    /// @brief 设置组合项目位置信息
    ///
    /// @param[in]  profileId  组合项目id码
    ///
    /// @return true表示设置成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年6月28日，新建函数
    ///
    bool SetProfileCardInfo(int profileId);

    ///
    /// @brief 获取所有页面的已选择的测试项目
    ///
    /// @param[in]  testItems  单一测试项目
    /// @param[in]  profiles   组合项目
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月18日，新建函数
    ///
    bool GetSampleTestItemInfo(std::vector<::tf::TestItem>& testItems, std::vector<int64_t>& profiles);

    ///
    /// @brief 更新测试项目信息
    ///
    /// @param[in]  testItem  测试项目详细信息
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月19日，新建函数
    ///
    bool UpdateTestItemStatus(tf::TestItem& testItem);

    ///
    /// @brief 更新机外稀释量
    ///
    /// @param[in]  preDilutionFactor  机外稀释
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年12月8日，新建函数
    ///
    void UpdatePreDilutionFactor(int32_t preDilutionFactor) { m_preDilutionFactor = preDilutionFactor; };

    ///
    /// @brief 返回手工稀释的数目
    ///
    ///
    /// @return 手工稀释数目
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年12月8日，新建函数
    ///
    int32_t GetPreDilutionFactor() { return m_storeDilution; }

    ///
    /// @brief 更新组合项目信息
    ///
    /// @param[in]  assayCode  组合项目编号
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月19日，新建函数
    ///
    bool UpdateAssayProfileStatus(int assayCode);

    ///
    /// @brief 刷新所有的项目状态
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月19日，新建函数
    ///
    void RefreshAssayCards();
    void RefreshAssayCardsWithNoClear();

    ///
    /// @brief 完全更新项目卡
    ///
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年6月7日，新建函数
    ///
    void RefreshAssayComplete();

    ///
    /// @brief 更新项目的稀释信息
    ///
    /// @param[in]  testItem  
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月19日，新建函数
    ///
    bool UpdateDulition(tf::TestItem& testItem);

    ///
    /// @brief 设置选中的项目
    ///
    /// @param[in]  assayCodes  项目编号列表
    /// @param[in]  type  页面类型
    ///
    /// @return true:设置成功
    ///
    /// @par History: 5774/WuHongTao，2023年9月15日，新建函数
    /// @li 7951/LuoXin，2023年9月18日，修改函数
    ///
    bool SetSelectAssay(const std::vector<int>& assayCodes, int type);

    ///
    /// @brief 更新项目测试次数
    ///
    /// @param[in]  assayCode  项目编号
    /// @param[in]  number	   次数
    ///
    /// @return true表示成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月24日，新建函数
    ///
    bool UpdateTestItemDbList(int assayCode, int number);

    ///
    /// @brief 初始化项目类型（生\免）选择控件
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月18日，新建函数
    ///
    void StartContainer(AssayType type);

    ///
    /// @brief 设置审核与否
    ///
    /// @param[in]  audit  审核与否
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年11月30日，新建函数
    ///
    void SetAudit(bool audit) { m_audit = audit; };
    tf::AssayClassify::type GetCurrentAssayType() { return m_currentDevType; };

    ///
    /// @bref
    ///		重新设定单项目行数
    ///
    /// @param[in] rowAmount 单项目行数
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年6月13日，新建函数
    ///
    void ResetSingleAssayRow(int rowAmount);

    ///
    /// @bref
    ///		获取单项目行数
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年6月14日，新建函数
    ///
    int  GetSingleAssayRowAmount() { return m_singleAssayRowAmount; }

signals:
    ///
    /// @brief 点击（稀释倍数，批量输入，保存）按钮的信号
    ///
    /// @param[in]  index  根据数字判断是什么信号
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月19日，新建函数
    ///
    void buttonPress(int index);
private:

    ///
    ///  @brief 清除所有选择
    ///
    ///  @par History: 
    /// @li 8276/huchunli, 2023年6月13日，新建函数
    ///
    void OnClearAllBtn();

    ///
    /// @bref
    ///		设置单项目行数
    ///
    /// @param[in] rowValue 行数
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年6月13日，新建函数
    ///
    void OnRowAdjustChanged(int rowValue);

    ///
    ///  @brief 减号“-”按钮
    ///
    ///
    ///
    ///  @return	
    ///
    ///  @par History: 
    ///  @li 7656/zhang.changjiang，2023年6月30日，新建函数
    ///
    void OnSubBtnClicked();

    ///
    ///  @brief 加号“+”按钮
    ///
    ///
    ///
    ///  @return	
    ///
    ///  @par History: 
    ///  @li 7656/zhang.changjiang，2023年6月30日，新建函数
    ///
    void OnAddBtnClicked();

    ///
    /// @bref
    ///		定时器函数
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年9月28日，新建函数
    ///
    void OnTimerFunc();

protected:

    ///
    /// @brief 判断样本类型能否对项目进行反应
    ///
    /// @param[in]  assayCode  项目编号
    ///
    /// @return true表示当前样本能测试这个项目
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年7月20日，新建函数
    ///
    bool ffbidenAssay(int assayCode);

    ///
    /// @brief 清除控件的暂时属性
    ///
    /// @param[in]  assayTable		控件容器
    /// @param[in]  indexPostion    控件位置
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月24日，新建函数
    ///
    void ClearTempAttribute(QAssayTabWidgets* assayTable, int indexPostion);

    ///
    /// @brief 获取免疫项目的默认稀释倍数
    ///
    /// @param[in]  assayCode  免疫项目的项目code
    ///
    /// @return 稀释倍数
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年1月5日，新建函数
    ///
    int GetDefaultDuliTon(int assayCode);

    ///
    /// @brief 根据编号清除暂存参数
    ///
    /// @param[in]  assayTable  页面句柄
    /// @param[in]  assaycode   项目编号
    ///
    /// @return true代表成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月24日，新建函数
    ///
    bool ClearTempAttributeByAssayCode(QAssayTabWidgets* assayTable, int assaycode);

    ///
    /// @brief 更新对应页面的稀释倍数状态
    ///
    /// @param[in]  assayTable  项目页面
    /// @param[in]  testItem  项目
    ///
    /// @return true，更新成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月19日，新建函数
    ///
    bool UpdateDulition(QAssayTabWidgets* assayTable, tf::TestItem& testItem);

    ///
    /// @brief 初始化项目选择控件
    ///
    /// @param[in]  cornerWidgets  角落控件
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月18日，新建函数
    ///
    void InitAssayTabWidget(AssayType type, tf::AssayClassify::type devType, QAssayTabWidgets* assayTable);

    ///
    /// @bref
    ///		获取所有的按钮信息，用于设置按钮的内容
    ///
    /// @param[out] btnInfo 返回的所有的按钮信息
    /// @param[in] devType 项目类型
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年11月14日，新建函数
    ///
    void TakeAssayBtnInfor(AssayButtonDatas& btnInfo, tf::AssayClassify::type devType);

    ///
    /// @brief 更新测试分类的框表
    ///
    /// @param[in]  devType  测试项目分类
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月18日，新建函数
    ///
    void RefreshPageAssayCards(tf::AssayClassify::type devType, QAssayTabWidgets* assayTable);

    ///
    /// @brief 获取单项目卡片显示列表
    ///
    /// @param[in]  assayType  项目类型
    /// @param[in]  pageIndex  页面索引
    ///
    /// @return 返回项目卡片的列表
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年6月27日，新建函数
    ///
    boost::optional<AssayButtons> GetSingleAssayCardList(tf::AssayClassify::type& assayType, int pageIndex);

    ///
    /// @brief 获取组合项目显示列表
    ///
    /// @param[in]  assayType  项目类型
    /// @param[in]  pageIndex  页面索引
    ///
    /// @return 返回组合项目卡片的列表
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年6月27日，新建函数
    ///
    boost::optional<AssayButtons> GetCombAssayCardList(const tf::AssayClassify::type& assayType, int pageIndex);

    ///
    /// @brief 更新项目的状态
    ///
    /// @param[in]  testItem  测试项目信息
    ///
    /// @return true，更新成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月11日，新建函数
    ///
    bool UpdateTestItemStatus(QAssayTabWidgets* assayTable, tf::TestItem& testItem);

    ///
    /// @brief 刷新样本组合项目的值
    ///
    /// @param[in]  assayCode  组合项目的编号
    ///
    /// @return true，更新成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月17日，新建函数
    ///
    bool UpdateAssayProfileStatus(QAssayTabWidgets* assayTable, int assayCode);

    ///
    /// @brief 根据用户选择获取测试项目信息
    ///
    /// @param[out]  testItems  项目信息
    /// @param[out]  profiles   组合项目信息
    ///
    /// @return true:表示获取成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月12日，新建函数
    ///
    bool GetSampleTestItemInfo(QAssayTabWidgets* assayTable, std::vector<::tf::TestItem>& testItems, std::vector<int64_t>& profiles);

    ///
    /// @brief 设置图例状态
    ///
    /// @param[in]  status  状态
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年6月1日，新建函数
    ///
    void SetLengendStatus(bool status);

	///
	/// @brief 设置按钮属性
	///
	/// @param[in]  attribute  按钮属性
	/// @param[in]  testItem   测试项目信息
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年11月7日，新建函数
	///
	void SetAttributeFromTestitem(AssayButtonData& attribute, const tf::TestItem& testItem);

    ///
    /// @brief 稀释策略是否规范（1：不能手工稀释和自动稀释混合，2：不能多次手工稀释）
    ///
    ///
    /// @return true,表示pass
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年12月8日，新建函数
    ///
    bool IsDiluTionPolicy(AssayButtonData& data);

    ///
    /// @brief 检测免疫项目稀释倍数设置是否超过要求的范围
    ///
    /// @param[in]  data  项目选择按钮属性
    ///
    /// @return true：通过， false：不通过
    ///
    /// @par History:
    /// @li 1556/Chenjianlin，2023年9月14日，新建函数
    ///
    bool IsImDiluTionPolicy(const AssayButtonData& data);

    private slots:
    ///
    /// @brief 处理对应的小空间选择卡
    ///
    /// @param[in]  indexPostion  对应位置的按钮被选中
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月11日，新建函数
    ///
    void OnAssaySelected(int indexPostion);

    ///
    /// @brief 切换页面
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2022年5月18日，新建函数
    ///
    void OnChangeTabIndex();

    ///
    /// @bref
    ///		处理按钮选择动作
    ///
    /// @param[in] assayTable 目标 页
    /// @param[in] indexPostion 目标按钮
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年10月30日，新建函数(来自对OnAssaySelected函数的提取
    ///
    void ProcSelectButton(QAssayTabWidgets* assayTable, int indexPostion);

private:
    Ui::QAssayTabWidgetContainer*	ui;
    std::map<tf::AssayClassify::type, AssayPageKeyInfo> m_pageKeyinfo;
    tf::AssayClassify::type			m_currentDevType;			///< 当前设备的类型
    int								m_currentIndex;				///< 当前选中的按钮位置
    std::map<int, AssayPageRetInfo>	m_assayPostionMap;			///< 项目位置修改列表
    int                             m_singleAssayRowAmount;     ///< 单项目在tb中的行数

    std::pair<SampleSize, int>      m_duliton;                  ///< 稀释方式
    bool                            m_audit;                    ///< 是否审核
    int                             m_preDilutionFactor;        ///< 机外系数数
    int                             m_requestAssays;            ///< 请求项目数目
    int                             m_storeDilution;            ///< 保存的机外稀释
    int                             m_currentSampleType;        ///< 当前样本类型
    AssayType						m_currentTableType;			///< 当前的选择框类型
    std::shared_ptr<QTimer>         m_timer;                    ///< 定时器
};
