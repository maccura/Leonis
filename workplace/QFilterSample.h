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
/// @file     QFilterSample.h
/// @brief    筛选对话框
///
/// @author   5774/WuHongTao
/// @date     2022年5月31日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月31日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#pragma once
#include <set>
#include <string>
#include <vector>
#include <bitset>
#include <xutility>
#include <memory>
#include <QString>
#include <QObject>
#include <QTextEdit>
#include <boost/optional.hpp>
#include <nlohmann/json.hpp>
#include "shared/basedlg.h"
#include "shared/datetimefmttool.h"
#include "src/public/StdSerialize.h"
#include "src/public/ConfigSerialize.h"
#include "src/thrift/ch/c1005/gen-cpp/c1005_ui_control_constants.h"
#include "src/thrift/gen-cpp/defs_types.h"

class QGroupAssayPositionSettings;
namespace Ui {
    class QFilterSample;
};
class QComboBox;
class QLineEdit;
class QGridLayout;

template<typename condtion, typename coreData, typename conPara>
struct ConditionDoing
{
    virtual void Reset() = 0;
    // 是否通过条件选择
    virtual bool IsPass() = 0;
    // 设置条件
    void LoadCondition(condtion& con) { m_condition = con; };
    condtion GetCondition() { return m_condition; };
    // 设置数据
    void LoadCoreData(coreData data) { m_coreData = data; };
    coreData GetCoreData() { return m_coreData; };
    // 条件变更响应函数
    virtual void OnChangeCondition(const conPara& parameter) = 0;
	void SetContentName(const std::vector<std::string>& condNames) { m_condNames = condNames; };

    condtion m_condition;
    coreData m_coreData;
    int      m_condNumber = 0;
	std::string	 m_condContent;
    bool     m_isChanged = false;

protected:
	void cutTipstring(const QString& src, QString& dst)
	{
		QFont font("Source Han Sans CN Regular");
		font.setPixelSize(16);
		QFontMetrics fm(font);
		int currentWidth = 0;
		for (auto& data : src)
		{
			int charWidth = fm.width(data);
			if ((currentWidth + charWidth) > 370)
			{
				dst += "\n";
				currentWidth = 0;
			}

			currentWidth += charWidth;
			dst += data;
		}

		if (dst.endsWith("\n"))
		{
			dst.chop(1);
		}
	}

	// 默认处理方式
	template<typename condtion>
	std::string GenCondContent(const condtion& cond)
	{
		return "";
	};

	// 特化一std::bitset<N>
	template<size_t N>
	std::string GenCondContent(const std::bitset<N>& cond)
	{
		if (m_condNumber == 0)
		{
			return "";
		}

		QString condContent;
		QString templateCondition = ("%1");
		if (m_condNames.empty())
		{
			return templateCondition.arg(QObject::tr("未知条件：")).toStdString();
		}

		// 标题
		condContent = QString::fromStdString(m_condNames[0]);
		for (int i = 0; i < cond.size(); i++)
		{
			if (!cond[i])
			{
				continue;
			}

			QString tmpContent;
			// 找不到对应的条件名称，异常情况,直接使用空
			if (m_condNames.size() <= (i + 1))
			{
				tmpContent = templateCondition.arg(QObject::tr("未知"));
			}
			else
			{
				tmpContent = templateCondition.arg(QString::fromStdString(m_condNames[i + 1]));
			}

			condContent += tmpContent;
			condContent += "/";
		}

		// 删除最后一个“/”
		condContent.chop(1);
		QString returnContent;
		cutTipstring(condContent, returnContent);
		// 添加回车符号
		returnContent += "\n";
		return returnContent.toStdString();
	};

	// 特化二std::pair<std::string, std::string>
	std::string GenCondContent(const std::pair<std::string, std::string>& cond)
	{
		if (m_condNumber == 0)
		{
			return "";
		}

		QString condContent;
		QString templateCondition = ("%1");
		if (m_condNames.empty())
		{
			return templateCondition.arg(QObject::tr("未知条件：\n")).toStdString();
		}

		// 标题
		condContent = QString::fromStdString(m_condNames[0]);
		// 起
		if (!cond.first.empty())
		{
			condContent += ToCfgFmtDate(QString::fromStdString(cond.first));
		}

		// 结束
		if (!cond.second.empty())
		{
			if (!cond.first.empty())
			{
				condContent += "-";
			}

			condContent += ToCfgFmtDate(QString::fromStdString(cond.second));
		}

		QString returnContent;
		cutTipstring(condContent, returnContent);
		// 添加回车符号
		returnContent += "\n";
		return returnContent.toStdString();
	};

	std::vector<std::string> m_condNames;
};


// 只需要样本信息即可判断的情况
using TestItemPointer = std::shared_ptr<tf::TestItem>;
using SamplePointer = std::shared_ptr<tf::SampleInfo>;
template<typename condtion>
struct SampleCondition :public ConditionDoing<condtion, SamplePointer, std::pair<int, bool>>
{
	void Reset() { m_condition.reset(); m_condNumber = 0; m_condContent.clear(); };

    virtual bool IsPass() override
    {
        if (m_coreData == Q_NULLPTR)
        {
            return false;
        }

        // 测试要求，全部不选，则所有都行
        if (m_condition.none())
        {
            return true;
        }

        for (int postion = 0; postion <= m_condition.size(); postion++)
        {
            if (Judge(postion))
            {
                return true;
            }
            else
            {
                continue;
            }
        }

        return false;
    }

    virtual void OnChangeCondition(const std::pair<int, bool>& parameter) override
    {
        if (parameter.first >= m_condition.size())
        {
            return;
        }

        parameter.second ? m_condition.set(parameter.first) : m_condition.reset(parameter.first);
        // 条件数目
        m_condNumber = m_condition.count();
		// 获取条件信息
		m_condContent = GenCondContent(m_condition);
        // 条件改变
        m_isChanged = true;
    }

    // 每个类的业务函数
    virtual bool Judge(int pos) = 0;
};

template<typename condtion>
struct SampleTestItemCondition :public ConditionDoing<condtion, std::pair<SamplePointer, TestItemPointer>, std::pair<int, bool>>
{
    void Reset() { m_condition.reset(); m_condNumber = 0; m_condContent.clear();};

    virtual bool IsPass() override
    {
        // 若都没有数据，则返回
        if (m_coreData.first == Q_NULLPTR && m_coreData.second == Q_NULLPTR)
        {
            return false;
        }

        // 测试要求，全部不选，则所有都行
        if (m_condition.none())
        {
            return true;
        }

        for (int postion = 0; postion <= m_condition.size(); postion++)
        {
            if (Judge(postion))
            {
                return true;
            }
            else
            {
                continue;
            }
        }

        return false;
    }

    virtual void OnChangeCondition(const std::pair<int, bool>& parameter) override
    {
        if (parameter.first >= m_condition.size())
        {
            return;
        }

        parameter.second ? m_condition.set(parameter.first) : m_condition.reset(parameter.first);
        // 条件数目
        m_condNumber = m_condition.count();
		// 获取条件信息
		m_condContent = GenCondContent(m_condition);
        // 条件改变
        m_isChanged = true;
    }

    // 每个类的业务函数
    virtual bool Judge(int pos) = 0;
};


// 只需要样本信息即可判断的情况(比较参数是std::pair<string, string>)
template<typename condtion>
struct SampleTestItemStringCondition :public ConditionDoing<condtion, std::pair<SamplePointer, TestItemPointer>, std::pair<std::string, std::string>>
{
	virtual void Reset() 
	{
		m_condition = std::make_pair("", ""); 
		m_condNumber = 0; 
		m_condContent.clear();
	};

    virtual bool IsPass() override
    {
        // 若都没有数据，则返回
        if (m_coreData.first == Q_NULLPTR && m_coreData.second == Q_NULLPTR)
        {
            return false;
        }

        if (m_condition.first.empty() && m_condition.second.empty())
        {
            return true;
        }

        return Judge(m_condition);
    }

    // 直接使用string的默认比较函数
    virtual void OnChangeCondition(const std::pair<std::string, std::string>& parameter) override
    {
        if (parameter.first.empty() && parameter.second.empty())
        {
            m_condNumber = 0;
        }
		else
		{
			m_condNumber = 1;
		}

        m_condition = parameter;

		// 获取条件信息
		m_condContent = GenCondContent(m_condition);

        // 条件改变
        m_isChanged = true;
    }

    // 每个类的业务函数
    virtual bool Judge(std::pair<std::string, std::string>& parameter) = 0;
};

// 项目的条件
using TestItemPointer = std::shared_ptr<tf::TestItem>;
template<typename condtion>
struct ItemCondition :public ConditionDoing<condtion, TestItemPointer, std::pair<int, bool>>
{
	void Reset() {m_condition.reset(); m_condNumber = 0; m_condContent.clear();};

    virtual bool IsPass() override
    {
        if (m_coreData == Q_NULLPTR)
        {
            return false;
        }

        // 测试要求，全部不选，则所有都行
        if (m_condition.none())
        {
            return true;
        }

        for (int postion = 0; postion <= m_condition.size(); postion++)
        {
            if (Judge(postion))
            {
                return true;
            }
            else
            {
                continue;
            }
        }

        return false;
    }

    virtual void OnChangeCondition(const std::pair<int, bool>& parameter) override
    {
        if (parameter.first >= m_condition.size())
        {
            return;
        }

        parameter.second ? m_condition.set(parameter.first) : m_condition.reset(parameter.first);
        // 条件数目
        m_condNumber = m_condition.count();
		// 获取条件信息
		m_condContent = GenCondContent(m_condition);
        // 条件改变
        m_isChanged = true;
    }

    // 每个类的业务函数
    virtual bool Judge(int pos) = 0;
};

// 订单类型是否符合需求的类
class OrderCondition :public SampleCondition<std::bitset<4>>
{
    virtual bool Judge(int postion) override;
};

// 样本状态的类
class StatusCondition :public SampleTestItemCondition<std::bitset<3>>
{
    virtual bool Judge(int postion) override;
};

// 定性判断
class QualJudgeCondition :public SampleTestItemCondition<std::bitset<2>>
{
	virtual bool Judge(int pos) override;
};

// AI识别
class AICondition :public SampleCondition<std::bitset<2>>
{
	virtual bool Judge(int pos) override;
};

// 样本类型的类
class SampleTypeCondition :public SampleCondition<std::bitset<6>>
{
    virtual bool Judge(int postion) override;
};

// 样本是否复查的类
class ReCheckCondition :public SampleTestItemCondition<std::bitset<2>>
{
    virtual bool Judge(int postion) override;
};

// 样本是否报警的类
class AlarmDoCondition :public SampleTestItemCondition<std::bitset<2>>
{
    virtual bool Judge(int postion) override;
};

// 样本是否审核的类
class AuditCondition :public SampleCondition<std::bitset<2>>
{
    virtual bool Judge(int postion) override;
};

// 样本是否发送LIS
class LISCondition :public SampleCondition<std::bitset<2>>
{
    virtual bool Judge(int postion) override;
};

// 样本是否打印
class PrintCondition :public SampleCondition<std::bitset<2>>
{
    virtual bool Judge(int postion) override;
};

// 样本检测日期
class ExaminationCondition :public SampleTestItemStringCondition<std::pair<std::string, std::string>>
{
    virtual bool Judge(std::pair<std::string, std::string>& parameter) override;
};

// 样本号范围
class SampleNumberCondition :public SampleTestItemStringCondition<std::pair<std::string, std::string>>
{
    virtual bool Judge(std::pair<std::string, std::string>& parameter) override;
};

// 样本条码
class SampleBarCodeCondition :public SampleTestItemStringCondition<std::pair<std::string, std::string>>
{
    virtual bool Judge(std::pair<std::string, std::string>& parameter) override;
};

// 试剂批号
class ReagentLotCondition :public ConditionDoing<std::set<std::string>, std::pair<SamplePointer, TestItemPointer>, std::string>
{

public:
	virtual void Reset() {m_condition.clear(); m_condNumber = 0; m_condContent.clear();};
    virtual bool IsPass() override;
    // 解析条件字符串（"lot;lot1;lot2;lot3..."）解析为（std::set<string>）
    virtual void OnChangeCondition(const std::string& parameter) override;
private:
    virtual bool Judge();
};

// 病人信息(条件是std::vector<int64_t>,包含的是病人信息的样本的db)
class PatientCondition :public ConditionDoing<std::set<int>, SamplePointer, std::pair<std::string, std::string>>
{

public:
	virtual void Reset() {m_condition.clear(); m_condNumber = 0; m_condContent.clear();};
    virtual bool IsPass() override;
    // 此处需要重写病人信息的生成（first:病历号， second:患者姓名，此处需要查询数据库来获取病人信息）
    virtual void OnChangeCondition(const std::pair<std::string, std::string>& parameter) override;
private:
    virtual bool Judge();
};

// 稀释状态
class DulitionCondition :public ItemCondition<std::bitset<4>>
{
	bool IsDulition(const TestItemPointer& testItem, bool isIncrease);
    virtual bool Judge(int pos) override;
};

// 模块信息
struct DeviceCondition :public ConditionDoing<std::string, TestItemPointer, std::pair<std::string, std::string>>
{
	virtual void Reset() {
		m_condition.clear(); m_condNumber = 0; m_condContent.clear();
	};

    virtual bool IsPass() override
    {
        if (m_coreData == Q_NULLPTR)
        {
            return false;
        }

        if (m_condition.empty())
        {
            return true;
        }

        return Judge(m_condition);
    }

	virtual void OnChangeCondition(const std::pair<std::string, std::string>& parameter) override;
    // 每个类的业务函数
    virtual bool Judge(std::string& parameter);
};

struct AssayCondition :public ConditionDoing<std::set<int>, SamplePointer, std::set<int>>
{
	virtual void Reset() {
		m_condition.clear(); m_condNumber = 0; m_condContent.clear();
	};

    virtual bool IsPass() override
    {
        if (m_coreData == Q_NULLPTR)
        {
            return false;
        }

        if (m_condition.empty())
        {
            return true;
        }

        return Judge(m_condition);
    }

	virtual void OnChangeCondition(const std::set<int>& parameter) override;
    // 每个类的业务函数
    virtual bool Judge(std::set<int>& parameter);
};

struct AssayItemCondition :public ConditionDoing<std::set<int>, TestItemPointer, std::set<int>>
{
	virtual void Reset() {
		m_condition.clear(); m_condNumber = 0; m_condContent.clear();
	};

    virtual bool IsPass() override
    {
        if (m_coreData == Q_NULLPTR)
        {
            return false;
        }

        if (m_condition.empty())
        {
            return true;
        }

        return Judge(m_condition);
    }

	virtual void OnChangeCondition(const std::set<int>& parameter) override;
    // 每个类的业务函数
    virtual bool Judge(std::set<int>& parameter);
};

// 筛选条件
class FilterConDition
{
public:
    OrderCondition      oderType;
    StatusCondition     status;
    SampleTypeCondition sampleType;
    AuditCondition      auditStatus;
    ReCheckCondition    reCheckStatus;
    AlarmDoCondition    alarmStatus;
    DulitionCondition   dulitionType;
    PrintCondition      printStatus;
    ReagentLotCondition reagentLots;
    std::bitset<2>      resultStatus;
    LISCondition        sendLis;
	AICondition			aiCheck;

    ExaminationCondition examinationTime;
    SampleNumberCondition sampleRange;
    SampleBarCodeCondition barcode;
    DeviceCondition     device;
    PatientCondition    medicalCase;
    PatientCondition    patientName;
    AssayCondition      sampleAssays;
    AssayItemCondition  testItemAssay;
	QualJudgeCondition  qualJudge;
    bool                enable;
    std::string              name;

    void Reset()
    {
        oderType.Reset();
        status.Reset();
        sampleType.Reset();
        auditStatus.Reset();
        reCheckStatus.Reset();
        alarmStatus.Reset();
        printStatus.Reset();
        dulitionType.Reset();
        reagentLots.Reset();
        sendLis.Reset();
		aiCheck.Reset();
        examinationTime.Reset();
        sampleRange.Reset();
        barcode.Reset();
        device.Reset();
        medicalCase.Reset();
        patientName.Reset();
        sampleAssays.Reset();
        testItemAssay.Reset();
		qualJudge.Reset();
    }

    // 按样本显示
    bool IsPass(SamplePointer& spSample);

    // 按项目显示
    bool IsPass(TestItemPointer& spTestItem);

    ///
    /// @brief 获取条件数目
    ///
    ///
    /// @return 条件数目
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年3月15日，新建函数
    ///
    std::pair<int, std::string> GetCondNumber(bool isShowBySample = true) const;
};

class QFilterSample : public BaseDlg
{
	Q_OBJECT
	struct typeItem
	{
		QLabel*						title;
		std::vector<QPushButton*>	buttons;
	};

	struct TypeButton
	{
		typeItem aiConds;
		typeItem recheckConds;
		typeItem auditConds;
		typeItem printConds;
		typeItem qualitativeConds;
	};

public:
	QFilterSample(QWidget *parent = Q_NULLPTR);
	~QFilterSample();

	boost::optional<int> GetCurrentPage() { return m_currentSetPage; };

    ///
    /// @brief 返回当前的筛选条件
    ///
    ///
    /// @return 筛选条件
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年3月14日，新建函数
    ///
    FilterConDition& GetFilterCond() 
    { 
        m_realCond.enable = true;
		if (m_currentSetPage)
		{
			auto index = m_currentSetPage.value();
			if (index <= 0 || index > m_filterConditions.size())
			{
				return m_realCond;
			}

			return m_filterConditions[index - 1];
		}

        return m_realCond; 
    };

    std::vector<std::pair<bool, std::string>> GetButtonInfo()
    {
        std::vector<std::pair<bool, std::string>> valueVector;
        for (auto condition : m_filterConditions)
        {
            valueVector.push_back(std::make_pair(condition.enable, condition.name));
        }

        return std::move(valueVector);
    }

    ///
    /// @brief 设置当前快捷筛选页面
    ///
    /// @param[in]  index  页面编号
    ///
    /// @return 设置是否成功
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2023年3月14日，新建函数
    ///
    bool SetCurrentFastCond(boost::optional<int> index);

signals:
	void finished();
    void enableFilter(int,bool);
    void changeFilterName(int,QString);

private:
	///
	/// @brief 初始化
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月31日，新建函数
	///
	void Init();
    void InItPageAttribute(bool reset = false);
    void InitPageCurrent();
    void LoadCond();
    void Reset();
    void SetDeivceComBoxInfo(QComboBox* classyComBox, QComboBox* nameComBox);
	virtual void showEvent(QShowEvent* event) override;

    ///
    /// @bref
    ///		设置‘项目名’编辑框的内容
    ///
    /// @param[in] strAssays 项目字符串
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年9月4日，新建函数
    ///
    void SetAssayEdit(QTextEdit* curEdit, const QString& strAssays);

    ///
    /// @bref
    ///		获取当前页面的项目编辑框中的项目
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年11月15日，新建函数
    ///
    void TakeCurrentPageAssays(QStringList& lstAssay);

	///
	/// @brief 保存当前筛选
	///
	///
	/// @return true:成功
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年11月24日，新建函数
	///
	bool ExecuteFilterCond(bool isChanged);

	///
	/// @brief 检查条件是否符合要求
	///
	/// @param[in]  condition    条件
	/// @param[in]  isFastCond   是否是快捷筛选（非快捷筛选）
	///
	/// @return true:符合要求
	///
	/// @par History:
	/// @li 5774/WuHongTao，2023年11月24日，新建函数
	///
	bool CheckCond(const FilterConDition& condition, bool isFastCond);

private slots:
	///
	/// @brief 项目选择对应的槽函数
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月31日，新建函数
	///
	void OnAssaySelect();

	///
	/// @brief 处理项目选择信息
	///
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年6月1日，新建函数
	///
	void OnSelectAssay();

	///
	/// @brief 保存筛选参数
	///
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月31日，新建函数
	///
	bool OnSaveParameter(bool isPageChaned = true);

	///
	/// @brief 显示页面设置对筛选功能的影响（更新显示设定页面的选项）
	///
	///
	/// @par History:
	/// @li 5774/WuHongTao，2024年1月4日，新建函数
	///
	void OnPageSetChanged();

    void AssignRealSampleRange();
    void AssignRealBarRange();

protected:
    void keyPressEvent(QKeyEvent* event);

private:
	Ui::QFilterSample*				ui;
	QStringList						m_attributeNames;		///< 属性名称
    QGroupAssayPositionSettings*	m_assaySelectDialog;	///< 项目选择对话框
    FilterConDition                 m_realCond;             ///< 筛选页面
    boost::optional<int>            m_currentFastPage;      ///< 当前快捷页面
    std::vector<FilterConDition>    m_filterConditions;     ///< 过滤条件
    boost::optional<int>            m_currentSetPage;       ///< 当前正在编辑页面
    boost::optional<int>            m_preSetPage;           ///< 前一编辑页面
    std::string                     m_conditionXml;         ///< 条件xml
	PageSet                         m_workSet;              ///< 工作页面设置
	std::vector<TypeButton>			m_typeButtons;			///< 类型按钮
};
