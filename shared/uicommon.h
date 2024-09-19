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
/// @file     uicommon.h
/// @brief    UI通用接口
///
/// @author   4170/TangChuXian
/// @date     2020年5月21日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 4170/TangChuXian，2020年5月21日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <sstream>
#include <ostream>

#include <QObject>
#include <QMap>
#include <QString>
#include "shared/uidef.h"
#include "src/thrift/ch/gen-cpp/ch_types.h"

// 前置声明
class QComboBox;
class QCheckBox;
class QPushButton;
class QTimer;
class QTableWidget;
class QTableView; // 表格控件
class QDateEdit;
class QStandardItemModel;
class BackupOrderConfig;

struct BaseSet;
struct DisplaySet;

enum SUCK_VOL_OR_DILLUTE;
enum DILLUTE_FACTOR;
enum QC_STATE;

// 重载"样本量/稀释"流输入运算
std::ostream& operator<<(std::ostream& out, const SUCK_VOL_OR_DILLUTE& val);

// 重载"稀释倍数"流输入运算
std::ostream& operator<<(std::ostream& out, const DILLUTE_FACTOR& val);

// 失控状态
std::ostream& operator<<(std::ostream& out, const QC_STATE& val);

#define ConvertTfEnumToQString(enObj)               UiCommon::Instance()->ConvertTfEnumToString(enObj)                      // 将thrift枚举值转换为QString
#define AddTfEnumItemToComBoBox(pCombo, enObj)      UiCommon::Instance()->AddComboBoxItem(pCombo, enObj)                    // 将thrift枚举值添加到下拉框中

#define SetTblColWidthScale(pTable, vScale)         UiCommon::Instance()->SetTableColWidthScale(pTable, vScale)             // 设置表格列宽比
#define ResizeTblColToContent(pTable)               UiCommon::Instance()->ResizeTblColToContents(pTable)                    // 表格列宽自适应
#define SetTblTextAlign(pTable, ...)                UiCommon::Instance()->SetTableItemAlign(pTable, __VA_ARGS__)            // 设置表格文本对齐方式
#define BindDateEdit(pStartEdit, pEndEdit, iIntervalMonth)UiCommon::Instance()->BindStartAndEndDateEdit(pStartEdit, pEndEdit, iIntervalMonth)     // 绑定起止日期编辑框
#define BindQcDateEdit(pStartEdit, pEndEdit)        UiCommon::Instance()->BindQcStartAndEndDateEdit(pStartEdit, pEndEdit)   // 绑定质控起止日期编辑框
#define AddOptLog(type, record)                     UiCommon::AddOperateLog(type, record)                                   // 添加页面的操作日志
#define devIsRun(dev)                               UiCommon::DeviceIsRunning(dev)                                          // 判断设备是否正在运行

// 添加文本到表格
#define AddTextToTableView(model,row,colunm,text)\
{\
    QStandardItem*si = new QStandardItem(text);\
    si->setData(Qt::AlignCenter, Qt::TextAlignmentRole);\
    model->setItem(row, colunm, si);\
}

class UiCommon : public QObject
{
    Q_OBJECT
        Q_DISABLE_COPY(UiCommon)

public:
    ~UiCommon();

    ///
    /// @brief
    ///     初始化单例对象
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年9月18日，新建函数
    ///
    static void Init();

    ///
    /// @brief 获取单例
    ///     
    /// @return 单实例
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年5月21日，新建函数
    ///
    static UiCommon* Instance();

	///
	/// @brief 根据样本类型和样本号生成样本类型
	///
	/// @param[in]  category  样本类型（急诊，常规，校准等）
	/// @param[in]  seq		  样本号
	///
	/// @return 格式形样本号
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月13日，新建函数
	///
	static std::string CombinSeqName(const std::string& category, const std::string& seq);

	///
	/// @brief 分解格式化的样本号
	///
	/// @param[in]  seqString  格式化样本号
	///
	/// @return 样本号
	///
	/// @par History:
	/// @li 5774/WuHongTao，2022年5月13日，新建函数
	///
	static int UnCombinSeqString(std::string seqString);

    ///
    /// @brief double转QString
    ///
    /// @param[in]  value  待转换的数据
    /// @param[in]  precision  精度
    ///
    /// @return 成功返回true
    ///
    /// @par History:
    /// @li 7951/LuoXin，2024年03月05日，新建函数
    ///
    static QString DoubleToQString(double value, int precision = UI_DOUBLE_DECIMALS);

    ///
    /// @brief  检查设置的试剂量/样本量是否合法
    ///     
    ///
    /// @param[in]  rav  试剂量
    /// @param[in]  sav  样本量
    /// @param[in]  isCail  是否由校准页面调用
    ///
    /// @return 合法返回true
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年8月25日，新建函数
    ///
    static bool CheckReagentVolumeIsValid(const std::vector<ch::tf::ReagentAspirateVol>& rav,
        const std::vector<ch::tf::SampleAspirateVol>& sav, bool isCail);

    ///
    /// @brief  检查参考范围是否合法
    ///     
    /// @param[in]  ar  参考范围
    ///
    /// @return  合法返回true
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年8月30日，新建函数
    ///
    static bool CheckAssayReferenceIsValid(::tf::AssayReference ar);

    ///
    /// @brief  修改了生化通用项目中的参数，提示用户确认
    ///     
    /// @param[in]  gai  项目通用参数
    ///
    /// @return true：用户确认，false：用户取消
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年10月12日，新建函数
    ///
    static bool  PromptUserAfterEditGeneralAssayInfo(const ::ch::tf::GeneralAssayInfo& gai);

    ///
    /// @brief  修改了生化特殊项目中的参数，提示用户确认
    ///     
    /// @param[in]  sai  项目特殊参数
    ///
    /// @return true：用户确认，false：用户取消
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年10月12日，新建函数
    ///
    static bool  PromptUserAfterEditSpecialAssayInfo(const ::ch::tf::SpecialAssayInfo& sai);

	///
	/// @brief 过滤QStringList中的相同项
	///
	/// @param[in]  strList  需要过滤的字符串列表
	///
	/// @return 过滤后的字符串列表
	///
	/// @par History:
	/// @li 8580/GongZhiQiang，2023年9月20日，新建函数
	///
	static QStringList QStringListFiltSameItem(const QStringList& strList);

    ///
    /// @bref
    ///		把备选项目订单设置保存到数据库
    ///
    /// @param[in] bkOrder 备选项目订单
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年7月25日，新建函数
    ///
    static bool SaveBackupOrderConfig(BackupOrderConfig& bkOrder);

    ///
    /// @brief  添加应用页面的操日志
    ///
    /// @param[in]  type  操作类型
    /// @param[in]  record  操作记录
    ///
    /// @return 成功返回true
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年10月30日，新建函数
    ///
    static bool AddOperateLog(::tf::OperationType::type type, const QString& record);

    ///
    /// @brief  设备是否正在运行
    ///
    /// @param[in]  stuTfDevInfo  设备信息
    ///
    /// @return 设备运行中返回true
    ///
    /// @par History:
    /// @li 7951/LuoXin，2023年12月14日，新建函数
    ///
    static bool DeviceIsRunning(const ::tf::DeviceInfo& stuTfDevInfo);

    ///
    /// @bref
    ///		给按钮设置字符串，如果超出按钮宽度，则添加省略号
    ///
    /// @param[in] pBtn 按钮
    /// @param[in] strText 按钮文字
    ///
    /// @par History:
    /// @li 8276/huchunli, 2024年3月26日，新建函数
    ///
    static void SetButtonTextWithEllipsis(QPushButton* pBtn, const QString& strText);

    ///
    /// @bref
    ///		报警码转换为字符串
    ///
    /// @param[in] mainCode 报警码主分段
    /// @param[in] middleCode 报警码中分段
    /// @param[in] subCode 报警码次分段
    ///
    /// @par History:
    /// @li 7951/LuoXin, 2024年7月24日，新建函数
    ///
    static QString AlarmCodeToString(int mainCode, int middleCode, int subCode);

    ///
    /// @brief
    ///     记录字符串资源
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年9月9日，新建函数
    ///
    static void RecordResourceString();

    ///
    /// @brief
    ///     记录维护失败原因字符串
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年9月6日，新建函数
    ///
    static void RecordMatainFailReasonString();

    ///
    /// @brief
    ///     记录维护项名称字符串
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2024年9月9日，新建函数
    ///
    static void RecordMatainItemNameString();

    ///
    /// @brief
    ///     将thrift中的枚举变量转化为字符串
    ///
    /// @param[in]  enVal  thrift枚举值
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年5月21日，新建函数
    ///
    template<typename tfEnumType>
    const QString& ConvertTfEnumToString(tfEnumType enVal)
    {
        // 先获取枚举类型对应的字符串
        std::ostringstream oss;
        oss << enVal;
        std::string strEnVal = oss.str();

        // 查找对应UI字符串
        auto it = m_enNameMap.find(strEnVal);
		if (it != m_enNameMap.end())
		{
			return it.value();
		}

        m_cStrEmpty = ""; // QString::fromStdString(strEnVal);

        // 没有匹配到字符串时返回空，避免界面显示出 -1 这样的错误值，出现错误值是严重BUG
        return m_cStrEmpty;
    }

	///
	/// @brief 维护失败枚举值转字符串信息
	///
	/// @param[in]  iValue  
	///
	/// @return 
	///
	/// @par History:
	/// @li 1556/Chenjianlin，2024年5月22日，新建函数
	///
	const QString ConvertMaintainFailCauseToString(int32_t iValue);

    ///
    /// @brief
    ///     为下拉框添加选项
    ///
    /// @param[in]  pComBo    下拉框
    /// @param[in]  userData  自定义thrift枚举值
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年5月21日，新建函数
    ///
    template<typename tfEnumType>
    void AddComboBoxItem(QComboBox* pComBo, tfEnumType enVal)
    {
        pComBo->addItem(ConvertTfEnumToQString(enVal), enVal);
    }

    ///
    /// @bref
    ///		赋值样本类型下拉框
    ///
    /// @param[in] pComBo 样本类型下拉框控件
    ///
    /// @par History:
    /// @li 8276/huchunli, 2023年7月13日，新建函数
    ///
    void AssignSampleCombox(QComboBox* pComBo);

	///
	/// @brief	QTableView表头插入checkBox
	///     
	/// @param[in]  tableView   表格指针
	/// @param[in]  checkBox    插入到表格的checkBox
	/// @param[in]  column		插入的勾选框所在列
	/// @par History:
	/// @li 7951/LuoXin，2022年10月19日，新建函数
	///
	void AddCheckboxToTableView(QTableView* tableView, QCheckBox* checkBox, int column = 0);

    ///
    /// @brief
    ///     设置表格列宽比
    ///
    /// @param[in]  pTable     表格控件
    /// @param[in]  scaleVec   列宽比列表
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年6月2日，新建函数
    ///
    void SetTableColWidthScale(QTableView* pTable, const QVector<double>& scaleVec);

    ///
    /// @brief
    ///     表格列宽自适应
    ///
    /// @param[in]  pTable  表格
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2022年3月29日，新建函数
    ///
    void ResizeTblColToContents(QTableView* pTable);

    ///
    /// @brief
    ///     设置表格文本对齐方式
    ///
    /// @param[in]  pTable  表格
    /// @param[in]  eAlign  对齐方式
    /// @param[in]  iRow    表格行号，-1代表全表格
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年11月12日，新建函数
    ///
    void SetTableItemAlign(QTableWidget* pTable, int eAlign, int iRow = -1);

    ///
    /// @brief
    ///     设置表格文本对齐方式
    ///
    /// @param[in]  pModel  表格
    /// @param[in]  eAlign  对齐方式
    /// @param[in]  iRow    表格行号，-1代表全表格
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年11月12日，新建函数
    ///
    void SetTableItemAlign(QStandardItemModel* pModel, int eAlign, int iRow = -1);

    ///
    /// @brief
    ///     设置起止日期编辑控件初始范围
    ///
    /// @param[in]  pStartEdit      起始日期编辑框
    /// @param[in]  pEndEdit        终止日期编辑框
    /// @param[in]  iIntervalMonth  间隔月数
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年6月19日，新建函数
    ///
    void BindStartAndEndDateEdit(QDateEdit* pStartEdit, QDateEdit* pEndEdit, int iIntervalMonth);

    ///
    /// @brief
    ///     设置质控起止日期编辑控件初始范围
    ///
    /// @param[in]  pStartEdit  起始日期编辑框
    /// @param[in]  pEndEdit    终止日期编辑框
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年11月17日，新建函数
    ///
    void BindQcStartAndEndDateEdit(QDateEdit* pStartEdit, QDateEdit* pEndEdit);

    ///
    /// @brief
    ///     获取维护项目的名称映射表
    ///
    /// @param[out]  nameList  具体的映射表内容
    ///
    /// @par History:
    /// @li 5774/WuHongTao，2020年9月17日，新建函数
    ///
    void GetMantenanceNameList(std::vector<QString>& nameList);

	///
	/// @brief
	///     检查定性判断数据是否合法
	///
	/// @param[in]  widget 窗口指针
	/// @param[in]  type(SIND时传入子项目名称，其他时候使用默认参数)
	///
	/// @return 是否合法
	///
	/// @par History:
	/// @li 7951/LuoXin，2023年3月17日，新建函数
	///
	bool CheckUserInputQualitativeData(QWidget* widget, QString type = "");

protected:
    UiCommon(QObject *parent = Q_NULLPTR);

    ///
    /// @brief
    ///     初始化thrift枚举名映射
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年5月21日，新建函数
    ///
    void InitEnNameMap();

    ///
    /// @brief
    ///     添加Thrift枚举名称映射
    ///
    ///@param[in]    emMap TF提供的映射map
    ///@param[in]    names 映射的名字列表
    ///
    /// @return     
    ///
    /// @par History:
    /// @li 6950/ChenFei，2022年05月17日，新建函数
    ///
    void AddNameMap(const char* key, const QString name)
    {
        if (key != nullptr && 0 != strcmp(key, ""))
        {
            m_enNameMap[key] = name;
        }
    }

    ///
    /// @brief
    ///     添加Thrift枚举名称映射
    ///
    ///@param[in]    emMap TF提供的映射map
    ///@param[in]    names 映射的名字列表
    ///
    /// @return     
    ///
    /// @par History:
    /// @li 6950/ChenFei，2022年05月17日，新建函数
    ///
    void AddTfEnNameMap(const std::map<int, const char*>& emMap, const QStringList& names)
    {
        int i = 0;
        for (auto& each : emMap)
        {
            AddNameMap(each.second, names.at(i));
            i++;
        }
    }

    ///
    /// @brief
    ///     初始化字典值映射
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年9月11日，新建函数
    ///
    void InitDiValMap();

    ///
    /// @brief
    ///     初始化当前日期更新触发器
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年11月17日，新建函数
    ///
    void InitCurDateUpdateMonitor();

    ///
    /// @brief
    ///     事件过滤器
    ///
    /// @param[in]  obj    目标对象
    /// @param[in]  event  事件
    ///
    /// @return true表示已处理
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年9月18日，新建函数
    ///
    bool eventFilter(QObject *obj, QEvent *event);	

Q_SIGNALS:
    ///
    /// @brief
    ///     增加表格平均列宽
    ///
    /// @param[in]  pTable      表格
    /// @param[in]  iAddWidth   增加宽度
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年8月31日，新建函数
    ///
    void SigTblAddColWidth(QTableView* pTable, int iAddWidth);

protected Q_SLOTS:
    ///
    /// @brief
    ///     日期编辑框被销毁
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年6月19日，新建函数
    ///
    void OnDateEditDistroyed();

    ///
    /// @brief
    ///     起始日期改变
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年6月19日，新建函数
    ///
    void OnStartDateChanged();

    ///
    /// @brief
    ///     终止日期改变
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2020年6月19日，新建函数
    ///
    void OnEndDateChanged();

    ///
    /// @brief
    ///     新的一天开始
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2021年11月17日，新建函数
    ///
    void OnNewDayArrived();

    ///
    /// @brief
    ///     增加平均列宽
    ///
    /// @param[in]  pTable      表格
    /// @param[in]  iAddWidth   增加宽度
    ///
    /// @par History:
    /// @li 4170/TangChuXian，2023年8月31日，新建函数
    ///
    void AddAvgColWidth(QTableView* pTable, int iAddWidth);

private:
    QMap<std::string, QString>          m_enNameMap;            // 枚举名映射
    QMap<std::string, std::string>      m_diValMap;             // 字典值映射
    QString								m_cStrEmpty;            // 空字符串
    QMap<QDateEdit*, QDateEdit*>        m_dateEditMap;          // 起止日期编辑框映射
    QMap<QDateEdit*, QDateEdit*>        m_qcDateEditMap;        // 质控起止日期编辑框映射
    QTimer*                             m_pDateChangeTimer;     // 日期改变监听定时器
	QMap<QString, int>					m_SampleSourceType;		// 样本源字符串与枚举值的映射
};
