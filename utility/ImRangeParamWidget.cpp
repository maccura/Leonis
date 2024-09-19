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
/// @file     ImRangeParamWidget.cpp
/// @brief    项目范围
///
/// @author   1226/ZhangJing
/// @date     2023年2月14日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 1226/ZhangJing，2023年2月14日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "ImRangeParamWidget.h"
#include "ui_ImRangeParamWidget.h"
#include "shared/tipdlg.h"
#include "shared/uidef.h"
#include "shared/msgiddef.h"
#include "shared/messagebus.h"
#include "shared/uicommon.h"
#include "shared/CommonInformationManager.h"
#include "shared/ThriftEnumTransform.h"

#include "thrift/im/ImLogicControlProxy.h"
#include "manager/UserInfoManager.h"
#include "manager/OperationLogManager.h"
#include "manager/DictionaryQueryManager.h"

#include "RangParaSetWidget.h"
#include "UtilityCommonFunctions.h"
#include "ImDataAlarmRetestSettingsDlg.h"
#include "src/thrift/gen-cpp/defs_types.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/common.h"


#define IM_REF_TABLE_DEFAULT_ROW_CNT                   (16)      // 参考区间表默认行数
#define IM_REF_TABLE_DEFAULT_COLUMN_CNT                (12)      // 参考区间表默认列数

///
/// @brief 范围判断
///
#define OutOfRangeAutoRecheck(dValue) ((dValue)<0 || (dValue)>999999)

///
/// @brief double的最小值判断
///
#define IsMinDouble(dValue) ((dValue) == DBL_MIN || (dValue) == std::numeric_limits<double>::lowest())

Q_DECLARE_METATYPE(::tf::AssayReferenceItem);

// 当发生错误时，弹出消息框后退出当前函数
#define OnErr(bErr, msg) \
if ((bErr))\
{\
	TipDlg(msg).exec();\
	return false;\
}

///
/// @brief
///     构造函数
///
/// @par History:
/// @li 5774/WuHongTao，2021年7月19日，新建函数
///
ImRangeParamWidget::ImRangeParamWidget(QWidget *parent)
	: QWidget(parent),
    ui(new Ui::ImRangeParamWidget()),
    m_bInit(false),
    m_optRow(-1),
    m_RangeParaSetWidget(new CRangParaSetWidget(RPD_Im, this))
{
    ui->setupUi(this);
    InitCtrErrorDescription();
    
    // 默认不选中参考区间
    ui->AccDiaGrpBox->setChecked(false);

    // 初始化参考区间表
    InitRefRangeTable();

    // 初始化倍率下拉
    QStringList valueList = { "" };
    valueList.append(utilcomm::GetDuliRatioListIm());
    ui->comboBoxRetestDuliRate1->addItems(valueList);
    ui->comboBoxRetestDuliRate2->addItems(valueList);

    // 自动复查范围框数字校验
    ui->lineEditAutoRetestRange1Lower->setValidator(new QDoubleValidator(this));
    ui->lineEditAutoRetestRange1Upper->setValidator(new QDoubleValidator(this));
    ui->lineEditAutoRetestRange2Lower->setValidator(new QDoubleValidator(this));
    ui->lineEditAutoRetestRange2Upper->setValidator(new QDoubleValidator(this));

    AssignDeviceComb(ui->comboBoxModuelDspAuto);
    AssignDeviceComb(ui->comboBoxModuelDspManual);

    // 注册当前用户权限更新处理函数
    SEG_REGIST_PERMISSION(this, UpdateCtrlStatu);
}

///
/// @bref
///		初始化控件输入值错误描述提示
///
/// @par History:
/// @li 8276/huchunli, 2023年8月21日，新建函数
///
void ImRangeParamWidget::InitCtrErrorDescription()
{
    m_ctrlErrorDescription.clear();

    m_ctrlErrorDescription[UV_AUTORETEST_VALUE_LOWER] = tr("值错误：自动复查范围1设置范围为[0,999999]。");
    m_ctrlErrorDescription[UV_AUTORETEST_VALUE_UPPER] = tr("值错误：自动复查范围1设置范围为[0,999999]。");
    m_ctrlErrorDescription[UV_AUTORETEST_RANGE] = tr("值错误：自动复查范围1低值大于等于高值。");
    m_ctrlErrorDescription[UV_AUTORETEST_DILU_RADIO] = tr("值错误：自动复查1稀释倍数。");
    m_ctrlErrorDescription[UV_AUTORETEST_LEAK_PARAM] = tr("填写的自动复查范围不完整。");
    m_ctrlErrorDescription[UV_AUTORETEST_DILU_RADIO_LESS] = tr("值错误：自动复查1稀释倍数小于默认稀释倍数。");
    m_ctrlErrorDescription[UV_AUTORETEST_DILU_RADIO_MORE] = tr("值错误：自动复查1稀释倍数大于最大稀释倍数。");
    m_ctrlErrorDescription[UV_AUTORETEST_VALUE2_LOWER] = tr("值错误：自动复查范围2设置范围为[0,999999]。");
    m_ctrlErrorDescription[UV_AUTORETEST_VALUE2_UPPER] = tr("值错误：自动复查范围2设置范围为[0,999999]。");
    m_ctrlErrorDescription[UV_AUTORETEST_RANGE2] = tr("值错误：自动复查范围2低值大于等于高值。");
    m_ctrlErrorDescription[UV_AUTORETEST_DILU_RADIO2] = tr("值错误：自动复查2稀释倍数。");
    m_ctrlErrorDescription[UV_AUTORETEST_DILU_RADIO2_LESS] = tr("值错误：自动复查2稀释倍数小于默认稀释倍数。");
    m_ctrlErrorDescription[UV_AUTORESTES_DILU_RADIO2_MORE] = tr("值错误：自动复查2稀释倍数大于最大稀释倍数。");
    m_ctrlErrorDescription[UV_AUTORESTES_RANGE_COVER] = tr("值错误：自动复查范围1不能与自动复查范围2重叠。");
}

///
/// @brief
///     析构函数
///
/// @par History:
/// @li 5774/WuHongTao，2021年7月19日，新建函数
///
ImRangeParamWidget::~ImRangeParamWidget()
{
}

///
/// @brief
///     界面显示后初始化
///
/// @par History:
/// @li 5774/WuHongTao，2021年7月19日，新建函数
///
void ImRangeParamWidget::InitAfterShow()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 参考区间添加按钮被点击
    connect(ui->RefAddBtn, SIGNAL(clicked()), this, SLOT(OnRefAddBtnClicked()));

    // 参考区间修改按钮被点击
    connect(ui->RefModBtn, SIGNAL(clicked()), this, SLOT(OnRefModBtnClicked()));

    // 参考区间删除按钮被点击
    connect(ui->RefDelBtn, SIGNAL(clicked()), this, SLOT(OnRefDelBtnClicked()));

    // 参考区间保存按钮被点击
    connect(m_RangeParaSetWidget->getSaveBtn(), SIGNAL(clicked()), this, SLOT(OnRefSaveBtnClicked()));

    // 数据报警复查的按钮
    connect(ui->pushButtonAlarmRetestSet, SIGNAL(clicked()), this, SLOT(OnAlarmRetestSettings()));
}

///
/// @brief
///     初始化参考区间表
///
/// @par History:
/// @li 5774/WuHongTao，2021年7月19日，新建函数
///
void ImRangeParamWidget::InitRefRangeTable()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 设置表格默认行数，列数
    ui->RefRangeTable->setColumnCount(IM_REF_TABLE_DEFAULT_COLUMN_CNT);

    // 设置表头
    QStringList strSelSampHeaderList;
    strSelSampHeaderList << tr("序号") << tr("是否缺省")<< tr("样本类型") << tr("低值") << tr("高值") 
        << tr("参考值范围") << tr("危急低值") << tr("危急高值") << tr("性别") << tr("年龄下限") 
        << tr("年龄上限") << tr("单位");
    ui->RefRangeTable->setHorizontalHeaderLabels(strSelSampHeaderList);
    ui->RefRangeTable->verticalHeader()->setVisible(false);

    // 设置表格选中模式为行选中，不可编辑，0024957:从不可多选修改为可选择多行
    ui->RefRangeTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->RefRangeTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->RefRangeTable->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 设置表格列宽比
    ResizeTblColToContent(ui->RefRangeTable);
}

///
/// @brief
///     加载数据到控件
///
/// @param[in]  assayCode  项目通道号
/// @par History:
/// @li 1226/zhangjing，2023年2月15日，新建函数
///
void ImRangeParamWidget::LoadRangeTbCtrlsData(const std::shared_ptr<im::tf::GeneralAssayInfo>& imAssay)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    if (imAssay == nullptr)
    {
        ULOG(LOG_WARN, "Null im assayinfo.");
        return;
    }

    // 更新参考区间表
    std::shared_ptr<CommonInformationManager> insMgr = CommonInformationManager::GetInstance();
    ui->RefRangeTable->setRowCount(0);
    for each (tf::AssayReferenceItem stuReference in imAssay->assayRefInfo.Items)
    {
        // 获取当前单位与主单位的转化倍率
        double factor = 1;;
        if (!insMgr->GetUnitFactor(m_rowItem.assayCode, factor))
        {
            ULOG(LOG_ERROR, "GetUnitFactor By AssayCode[%d] Failed !", m_rowItem.assayCode);
            return;
        }
        ui->RefRangeTable->setRowCount(ui->RefRangeTable->rowCount() + 1);
        if (!IsMinDouble(stuReference.fLowerRefRang))
        {
            stuReference.__set_fLowerRefRang(stuReference.fLowerRefRang * factor);
        }
        if (stuReference.fUpperRefRang != DBL_MAX)
        {
            stuReference.__set_fUpperRefRang(stuReference.fUpperRefRang * factor);
        }
        if (!IsMinDouble(stuReference.fLowerCrisisRang))
        {
            stuReference.__set_fLowerCrisisRang(stuReference.fLowerCrisisRang * factor);
        }
        if (stuReference.fUpperCrisisRang != DBL_MAX)
        {
            stuReference.__set_fUpperCrisisRang(stuReference.fUpperCrisisRang * factor);
        }
        ShowReferrence(ui->RefRangeTable->rowCount() - 1, stuReference);
    }

    SetTblTextAlign(ui->RefRangeTable, Qt::AlignCenter, -1);
}

///
/// @bref
///		加载自动复查的数据到界面
///
/// @param[in] imAssay 免疫项目
///
/// @par History:
/// @li 8276/huchunli, 2023年8月31日，新建函数
///
void ImRangeParamWidget::LoadRetestSettings(std::shared_ptr<im::tf::GeneralAssayInfo>& imAssay)
{
    if (imAssay == nullptr)
    {
        return;
    }
    std::shared_ptr<CommonInformationManager> comMgr = CommonInformationManager::GetInstance();

    // 清空原有自动复查界面
    ui->comboBoxRetestDuliRate1->setCurrentIndex(-1);
    ui->lineEditAutoRetestRange1Lower->setText("");
    ui->lineEditAutoRetestRange1Upper->setText("");
    ui->comboBoxRetestDuliRate2->setCurrentIndex(-1);
    ui->lineEditAutoRetestRange2Lower->setText("");
    ui->lineEditAutoRetestRange2Upper->setText("");

    // 获取当前单位与主单位的转化倍率
    double factor = 0;
    if (!comMgr->GetUnitFactor(imAssay->assayCode, factor))
    {
        ULOG(LOG_ERROR, "GetUnitFactor By AssayCode[%d] Failed !", imAssay->assayCode);
        return;
    }
    std::shared_ptr<::tf::GeneralAssayInfo> pGeneralAssay = comMgr->GetAssayInfo(imAssay->assayCode);
    if (pGeneralAssay == nullptr)
    {
        ULOG(LOG_ERROR, "Null generalAssayInfo.");
        return;
    }

    // 自动复查
    ui->groupBoxRetest->setChecked(imAssay->autoRerunActive);
    if (imAssay->rerunSet.ranges.size() >= 1)
    {
        im::tf::autoRerunByRange &rng = imAssay->rerunSet.ranges[0];
        QString strLower1 = IsMinDouble(rng.range.lower) ? "" : QString::number(rng.range.lower * factor, 'f', pGeneralAssay->decimalPlace);
        ui->lineEditAutoRetestRange1Lower->setText(strLower1);
        QString strUpper1 = rng.range.upper == DBL_MAX ? "" : QString::number(rng.range.upper * factor, 'f', pGeneralAssay->decimalPlace);
        ui->lineEditAutoRetestRange1Upper->setText(strUpper1);

        QString strRate1 = (rng.ratio > 0) ? (QString::number(rng.ratio)) : (
            imAssay->reactInfo.dilutionRatio > 0 ? QString::number(imAssay->reactInfo.dilutionRatio) : "");
        ui->comboBoxRetestDuliRate1->setCurrentText(strRate1);
    }
    else
    {
        ui->comboBoxRetestDuliRate1->setCurrentText(imAssay->reactInfo.dilutionRatio > 0 ? QString::number(imAssay->reactInfo.dilutionRatio) : "");
    }

    if (imAssay->rerunSet.ranges.size() >= 2)
    {
        im::tf::autoRerunByRange &rng = imAssay->rerunSet.ranges[1];
        QString strLower2 = IsMinDouble(rng.range.lower) ? "" : QString::number(rng.range.lower * factor, 'f', pGeneralAssay->decimalPlace);
        ui->lineEditAutoRetestRange2Lower->setText(strLower2);
        QString strUpper2 = rng.range.upper == DBL_MAX ? "" : QString::number(rng.range.upper * factor, 'f', pGeneralAssay->decimalPlace);
        ui->lineEditAutoRetestRange2Upper->setText(strUpper2);

        QString strRate2 = (rng.ratio > 0) ? QString::number(rng.ratio) : (
            imAssay->reactInfo.dilutionRatio > 0 ? QString::number(imAssay->reactInfo.dilutionRatio) : "");
        ui->comboBoxRetestDuliRate2->setCurrentText(strRate2);
    }
    else
    {
        ui->comboBoxRetestDuliRate2->setCurrentText(imAssay->reactInfo.dilutionRatio > 0 ? QString::number(imAssay->reactInfo.dilutionRatio) : "");
    }

    // 自动复查模块
    if (imAssay->rerunSet.__isset.autoRerunDspType)
    {
        std::string stdSn = imAssay->rerunSet.autoRerunDspType.rtModules.size() > 0 
			?  imAssay->rerunSet.autoRerunDspType.rtModules[0].sn : "";
        std::string strDevName = comMgr->GetDeviceName(stdSn);
        QString strName = GetComDeviceShowText(imAssay->rerunSet.autoRerunDspType.rtDispatchType, strDevName);
        ui->comboBoxModuelDspAuto->setCurrentText(strName);
    }
    else
    {
        ui->comboBoxModuelDspAuto->setCurrentIndex(0);
    }
}

///
/// @brief
///     初始化范围界面数据
///
/// @param[in]  db  生化数据库主键
///
/// @par History:
/// @li 5774/WuHongTao，2021年7月20日，新建函数
///
void ImRangeParamWidget::LoadRangeParameter(const AssayListModel::StAssayListRowItem &item)
{
	ULOG(LOG_INFO, "%s(%d)", __FUNCTION__, item.assayCode);

    ui->RefRangeTable->clearContents();
	if (item.model != ::tf::DeviceType::DEVICE_TYPE_I6000)
	{
        ULOG(LOG_WARN, "Invalid device type.");
		return;
	}

	// 缓存当前选中行信息
	m_rowItem = item;

    // 查询通用项目信息
    auto imAssay = CommonInformationManager::GetInstance()->GetImmuneAssayInfo(item.assayCode, (::tf::DeviceType::DEVICE_TYPE_I6000), item.version.toStdString());
    if (imAssay == nullptr)
    {
        ULOG(LOG_ERROR, "Failed to get immune assayinfo!");
        return;
    }

    // 范围表格
    LoadRangeTbCtrlsData(imAssay);
	
    // 加载自动复查
    LoadRetestSettings(imAssay);

    UpdateCtrlStatu();
}

///
/// @brief
///     保存范围配置参数
///
/// @param[out]  imAssay  免疫通用项目列表
/// @param[in]  factor  单位倍率
///
/// @par History:
/// @li 1226/zhangjing，2023年2月15日，新建函数
///
void ImRangeParamWidget::TakeRangeTbCtrlsData(im::tf::GeneralAssayInfo& imAssay, double factor)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 参考区间
    ::tf::AssayReference referrence;
    int row = 0;
    while (row < ui->RefRangeTable->rowCount())
    {
        ::tf::AssayReferenceItem item;
        if (GetReferrenceByRow(row, item))
        {
            // 单位倍率转换
            if (!IsMinDouble(item.fLowerRefRang))
            {
                item.__set_fLowerRefRang(item.fLowerRefRang / factor);
            }
            if (item.fUpperRefRang != DBL_MAX)
            {
                item.__set_fUpperRefRang(item.fUpperRefRang / factor);
            }
            if (!IsMinDouble(item.fLowerCrisisRang))
            {
                item.__set_fLowerCrisisRang(item.fLowerCrisisRang / factor);
            }
            if (item.fUpperCrisisRang != DBL_MAX)
            {
                item.__set_fUpperCrisisRang(item.fUpperCrisisRang / factor);
            }

            referrence.Items.push_back(item);
        }

        row++;
    }

    imAssay.__set_refActive(true);

    // 处理小数位数变化导致的精度差异问题
    std::shared_ptr<::tf::GeneralAssayInfo> pGeneralAssay = CommonInformationManager::GetInstance()->GetAssayInfo(m_rowItem.assayCode);
    if (pGeneralAssay == nullptr)
    {
        ULOG(LOG_ERROR, "Null generalAssayInfo.");
        return;
    }
    int deci = pGeneralAssay->decimalPlace;
    if (imAssay.assayRefInfo.Items.size() == referrence.Items.size())
    {    
        // 判断变化是否是由于精度改变产生
        auto funcDoubleEque = [](int deci, double da, double db) -> bool {
            QString nmb1 = QString::number(da, 'f', deci + 2);
            QString nmb2 = QString::number(db, 'f', deci + 2);
            return (nmb1.mid(0, nmb1.indexOf('.') + 1 + deci) == nmb2.mid(0, nmb2.indexOf('.') + 1 + deci));
        };
        for (int i=0; i<referrence.Items.size(); ++i)
        {
            ::tf::AssayReferenceItem &oldItem = imAssay.assayRefInfo.Items[i];
            ::tf::AssayReferenceItem &newItem = referrence.Items[i];
            if (funcDoubleEque(deci, oldItem.fLowerRefRang, newItem.fLowerRefRang))
            {
                newItem.__set_fLowerRefRang(oldItem.fLowerRefRang);
            }
            if (funcDoubleEque(deci, oldItem.fLowerCrisisRang, newItem.fLowerCrisisRang))
            {
                newItem.__set_fLowerCrisisRang(oldItem.fLowerCrisisRang);
            }
            if (funcDoubleEque(deci, oldItem.fUpperRefRang, newItem.fUpperRefRang))
            {
                newItem.__set_fUpperRefRang(oldItem.fUpperRefRang);
            }
            if (funcDoubleEque(deci, oldItem.fUpperCrisisRang, newItem.fUpperCrisisRang))
            {
                newItem.__set_fUpperCrisisRang(oldItem.fUpperCrisisRang);
            }
        }
    }
    imAssay.__set_assayRefInfo(referrence);
}

///
/// @bref
///		获取自动复查的设置
///
/// @param[in] imAssay 免疫项目
/// @param[in] factor 单位倍率
///
/// @par History:
/// @li 8276/huchunli, 2023年8月31日，新建函数
///
void ImRangeParamWidget::TakeRetestSettinggs(im::tf::GeneralAssayInfo& imAssay, double factor)
{
    std::shared_ptr<::tf::GeneralAssayInfo> pGeneralAssay = CommonInformationManager::GetInstance()->GetAssayInfo(m_rowItem.assayCode);
    if (pGeneralAssay == nullptr)
    {
        ULOG(LOG_ERROR, "Null generalAssayInfo.");
        return;
    }
    int deci = pGeneralAssay->decimalPlace;

    // 判断变化是否是由于精度改变产生
    auto funcDoubleEque = [](int deci, double da, double db) -> bool {
        QString nmb1 = QString::number(da, 'f', deci + 2);
        QString nmb2 = QString::number(db, 'f', deci + 2);
        return (nmb1.mid(0, nmb1.indexOf('.') + 1 + deci) == nmb2.mid(0, nmb2.indexOf('.') + 1 + deci));
    };

    // 自动复查
    bool bRerunActive = ui->groupBoxRetest->isChecked();
    imAssay.__set_autoRerunActive(bRerunActive);

    if (bRerunActive)
    {
        std::vector<im::tf::autoRerunByRange> autoReRanges = imAssay.rerunSet.ranges;

        // 第一组
        if (autoReRanges.size() < 1)
        {
            ::tf::DoubleRange range1;
            im::tf::autoRerunByRange  rs1;
            range1.__set_lower(std::numeric_limits<double>::lowest());
            range1.__set_upper(DBL_MAX);
            rs1.__set_range(range1);
            autoReRanges.push_back(std::move(rs1));
        }
        im::tf::autoRerunByRange &rs1 = autoReRanges[0];
        rs1.__set_ratio(ui->comboBoxRetestDuliRate1->currentText().toInt());
        QString strLower1 = ui->lineEditAutoRetestRange1Lower->text();
        double curLower1 = strLower1.isEmpty() ? std::numeric_limits<double>::lowest() : strLower1.toDouble() / factor;
        if (IsMinDouble(rs1.range.lower) || !funcDoubleEque(pGeneralAssay->decimalPlace, rs1.range.lower, curLower1))
        {
            rs1.range.__set_lower(curLower1);
        }
        QString strUpper1 = ui->lineEditAutoRetestRange1Upper->text();
        double curUpper1 = strUpper1.isEmpty()? DBL_MAX : strUpper1.toDouble() / factor;
        if (!funcDoubleEque(pGeneralAssay->decimalPlace, rs1.range.upper, curUpper1))
        {
            rs1.range.__set_upper(curUpper1);
        }

        // 第二组
        if (autoReRanges.size() < 2)
        {
            im::tf::autoRerunByRange  rs2;
            ::tf::DoubleRange range2;
            range2.__set_lower(std::numeric_limits<double>::lowest());
            range2.__set_upper(DBL_MAX);
            rs2.__set_range(range2);
            autoReRanges.push_back(std::move(rs2));
        }
        im::tf::autoRerunByRange &rs2 = autoReRanges[1];
        rs2.__set_ratio(ui->comboBoxRetestDuliRate2->currentText().toInt());
        QString strLower2 = ui->lineEditAutoRetestRange2Lower->text();
        double curLower2 = strLower2.isEmpty() ? std::numeric_limits<double>::lowest() : strLower2.toDouble() / factor;
        if (IsMinDouble(rs2.range.lower) || !funcDoubleEque(pGeneralAssay->decimalPlace, rs2.range.lower, curLower2))
        {
            rs2.range.__set_lower(curLower2);
        }
        QString strUpper2 = ui->lineEditAutoRetestRange2Upper->text();
        double curUpper2 = strUpper2.isEmpty() ? DBL_MAX : strUpper2.toDouble() / factor;
        if (!funcDoubleEque(pGeneralAssay->decimalPlace, rs2.range.upper, curUpper2))
        {
            rs2.range.__set_upper(curUpper2);
        }

        imAssay.rerunSet.__set_ranges(autoReRanges);
        if (!imAssay.__isset.rerunSet)
        {
            imAssay.__set_rerunSet(imAssay.rerunSet);
        }

        // 复查模块
        tf::RerunDispatchModule autoModule = imAssay.rerunSet.autoRerunDspType;
        QVariant selData = ui->comboBoxModuelDspAuto->currentData();
        if (!selData.isNull() && selData.isValid())
        {
            autoModule.__set_rtDispatchType((tf::RerunDispatchType::type)selData.toInt());
            if (autoModule.rtDispatchType == tf::RerunDispatchType::type::RtModuleAppoint)
            {
                QString curtext = ui->comboBoxModuelDspAuto->currentText();
                int firstSla = curtext.indexOf('-');
                if (firstSla != -1)
                {
                    std::vector<::tf::DevicekeyInfo> vecModules;
                    std::string strName = curtext.mid(firstSla + 1).toStdString();
                    QStringList lstSn = CommonInformationManager::GetInstance()->GetDeviceSnByName(strName);
                    for (const QString& snItem : lstSn)
                    {
						::tf::DevicekeyInfo mki;
						mki.__set_sn(snItem.toStdString());
						vecModules.push_back(mki);
                    }
                    autoModule.__set_rtModules(vecModules);
                }
            }
        }        
        imAssay.rerunSet.__set_autoRerunDspType(autoModule);
    }
}

///
/// @brief
///     获取范围数据
///     
///
/// @param[in]  db  生化数据库主键
///
/// @par History:
/// @li 5774/WuHongTao，2021年7月20日，新建函数
///
bool ImRangeParamWidget::GetRangeParameter(const AssayListModel::StAssayListRowItem& item, \
    std::vector<std::shared_ptr<::im::tf::GeneralAssayInfo>>& vecImAssayInfo)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

    std::shared_ptr<CommonInformationManager> comMgr = CommonInformationManager::GetInstance();

    // 获取当前单位与主单位的转化倍率
    double factor = 1;
    if (!comMgr->GetUnitFactor(item.assayCode, factor))
    {
        ULOG(LOG_ERROR, "GetUnitFactor By AssayCode[%d] Failed !", item.assayCode);
        return false;
    }

    // 特定需求：当前页面设置对所有版本生效
    for (std::shared_ptr<im::tf::GeneralAssayInfo>& imAssay : vecImAssayInfo)
    {
        // 检测自动复查的界面参数合法性
        UiCtrlValidError errStat = CheckCtrlValueAutoRetest(imAssay);
        if (errStat != UV_NO_ERROR)
        {
            std::map<UiCtrlValidError, QString>::iterator it = m_ctrlErrorDescription.find(errStat);
            if (it == m_ctrlErrorDescription.end())
            {
                ULOG(LOG_ERROR, "Unknown ctrl error value description.");
                return false;
            }
            TipDlg(it->second).exec();
            return false;
        }

        // 获取范围配置参数
        TakeRangeTbCtrlsData(*imAssay, factor);

        // 获取自动复查设置
        TakeRetestSettinggs(*imAssay, factor);
    }

	return true;
}

///
/// @brief
///     窗口显示事件
///
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 5774/WuHongTao，2021年7月20日，新建函数
///
void ImRangeParamWidget::showEvent(QShowEvent *event)
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    // 让基类处理事件
    QWidget::showEvent(event);

    // 第一次显示时初始化
    if (!m_bInit)
    {
        m_bInit = true;
        InitAfterShow();
    }
}


bool ImRangeParamWidget::GetReferrenceByRow(int row, ::tf::AssayReferenceItem& referrence)
{
    QTableWidgetItem* tbItem = ui->RefRangeTable->item(row, 0);
    if (tbItem == nullptr || tbItem->text().isEmpty())
    {
        ULOG(LOG_WARN, "Invalid table item.");
        return false;
    }

    referrence.__set_bGenderRelevent(true);
    referrence.__set_bAgeRelevant(true);

	// 是否缺省
	auto item = ui->RefRangeTable->item(row, IM_REF_TABLE_DEFAULT_COLUMN);
    referrence.__set_bAutoDefault(item->text() == tr(STR_AUTO_DEFAULT_Y));

	// 样本类型
	item = ui->RefRangeTable->item(row, IM_REF_TABLE_SAMP_SOURCE);
    referrence.__set_sampleSourceType(ThriftEnumTrans::TransSampleType(item->text()));

	// 低值
	auto tempLowText = ui->RefRangeTable->item(row, IM_REF_TABLE_LOW_COLUMN)->text();
	referrence.__set_fLowerRefRang(tempLowText.isEmpty() ? std::numeric_limits<double>::lowest() : tempLowText.toDouble());
	
	// 高值
	auto tempHigeText = ui->RefRangeTable->item(row, IM_REF_TABLE_UP_COLUMN)->text();
	referrence.__set_fUpperRefRang(tempHigeText.isEmpty() ? DBL_MAX : tempHigeText.toDouble());

	// 参考区间
	auto tempText = ui->RefRangeTable->item(row, IM_REF_TABLE_REF_COLUMN)->text();
	QString temp = tempLowText + "-" + tempHigeText;
	if (temp != tempText)
	{
        referrence.__set_strRefRange(tempText.toStdString());
	}	

	// 危急区间
	tempText = ui->RefRangeTable->item(row, IM_REF_TABLE_F_AGE_COLUMN_LOW)->text();
	referrence.__set_fLowerCrisisRang(tempText.isEmpty() ? std::numeric_limits<double>::lowest() : tempText.toDouble());
    tempText = ui->RefRangeTable->item(row, IM_REF_TABLE_F_AGE_COLUMN_UPPER)->text();
	referrence.__set_fUpperCrisisRang(tempText.isEmpty() ? DBL_MAX : tempText.toDouble());

	// 性别
	item = ui->RefRangeTable->item(row, IM_REF_TABLE_GENDER_COLUMN);
    int iGender = ThriftEnumTrans::TransGender(item->text());
    if (iGender != -1)
    {
        referrence.__set_enGender((tf::Gender::type)iGender);
    }

    // 年龄上限
    tempText = ui->RefRangeTable->item(row, IM_REF_TABLE_AGE_UP_COLUMN)->text();
    referrence.__set_iUpperAge(tempText.isEmpty() ? INT_MAX : tempText.toInt());

	// 年龄下限
	tempText = ui->RefRangeTable->item(row, IM_REF_TABLE_AGE_LOW_COLUMN)->text();
    referrence.__set_iLowerAge(tempText.isEmpty() ? INT_MIN : tempText.toInt());
	
	// 年龄单位
	item = ui->RefRangeTable->item(row, IM_REF_TABLE_AGEUNIT_COLUMN);
    int iAgeUnit = ThriftEnumTrans::TransAgeUnit(item->text());
    if (iAgeUnit != -1)
    {
        referrence.__set_enAgeUnit((tf::AgeUnit::type)iAgeUnit);
    }

	return true;
}

///
/// @brief	显示参考区间
///     
/// @param[in]  row   行号
/// @param[in]  item  参考区间
///
/// @par History:
/// @li 7951/LuoXin，2022年12月29日，新建函数
///
void ImRangeParamWidget::ShowReferrence(int row, const ::tf::AssayReferenceItem& item)
{
    int iPriv = UI_RESULT_PRECISION; // 默认保留2位
    std::shared_ptr<tf::GeneralAssayInfo> pAss = CommonInformationManager::GetInstance()->GetAssayInfo(m_rowItem.assayCode);
    if (pAss != nullptr)
    {
        iPriv = pAss->decimalPlace;
    }
	// 将参考区间参数添加到表格中
	// 序号
	ui->RefRangeTable->setItem(row, IM_REF_TABLE_NUM_COLUMN,
		new QTableWidgetItem(QString::number(row + 1)));
	// 是否缺省
	ui->RefRangeTable->setItem(row, IM_REF_TABLE_DEFAULT_COLUMN,
		new QTableWidgetItem(item.bAutoDefault ? STR_AUTO_DEFAULT_Y : STR_AUTO_DEFAULT_N));
	// 样本类型

	if (item.sampleSourceType >= 0)
	{
		ui->RefRangeTable->setItem(row, IM_REF_TABLE_SAMP_SOURCE,
			new QTableWidgetItem(ConvertTfEnumToQString((tf::SampleSourceType::type)item.sampleSourceType)));
	}
	else
	{
		ui->RefRangeTable->setItem(row, IM_REF_TABLE_SAMP_SOURCE,new QTableWidgetItem(""));
	}
	
	// 低值
	QString strLow = IsMinDouble(item.fLowerRefRang) ? "" : QString::number(item.fLowerRefRang, 'f', iPriv);
	ui->RefRangeTable->setItem(row, IM_REF_TABLE_LOW_COLUMN, new QTableWidgetItem(strLow));

	// 高值
	QString strUp = item.fUpperRefRang == DBL_MAX ? "" : QString::number(item.fUpperRefRang, 'f', iPriv);
	ui->RefRangeTable->setItem(row, IM_REF_TABLE_UP_COLUMN, new QTableWidgetItem(strUp));

	// 参考范围
	QString temp = strLow + "-" + strUp;
	if (!item.strRefRange.empty())
	{
		temp = QString::fromStdString(item.strRefRange);
	}
	else if (strLow.isEmpty() || strUp.isEmpty())
	{
		temp = "";
	}
	ui->RefRangeTable->setItem(row, IM_REF_TABLE_REF_COLUMN, new QTableWidgetItem(temp));

	// 危急范围
    temp = IsMinDouble(item.fLowerCrisisRang) ? "" : QString::number(item.fLowerCrisisRang, 'f', iPriv);
    ui->RefRangeTable->setItem(row, IM_REF_TABLE_F_AGE_COLUMN_LOW, new QTableWidgetItem(temp));
    temp = item.fUpperCrisisRang == DBL_MAX ? "" : QString::number(item.fUpperCrisisRang, 'f', iPriv);
    ui->RefRangeTable->setItem(row, IM_REF_TABLE_F_AGE_COLUMN_UPPER, new QTableWidgetItem(temp));

	// 性别
	ui->RefRangeTable->setItem(row, IM_REF_TABLE_GENDER_COLUMN,
		new QTableWidgetItem(ConvertTfEnumToQString(item.enGender)));
    // 年龄上限
    ui->RefRangeTable->setItem(row, IM_REF_TABLE_AGE_UP_COLUMN, new QTableWidgetItem(
        item.iUpperAge == INT_MAX ? "" : QString::number(item.iUpperAge)));
	// 年龄下限
	ui->RefRangeTable->setItem(row, IM_REF_TABLE_AGE_LOW_COLUMN, new QTableWidgetItem(
		item.iLowerAge == INT_MIN ? "" : QString::number(item.iLowerAge)));
	// 年龄单位
	ui->RefRangeTable->setItem(row, IM_REF_TABLE_AGEUNIT_COLUMN,
		new QTableWidgetItem(ConvertTfEnumToQString(item.enAgeUnit)));

    ui->RefRangeTable->item(row, 0)->setData(Qt::UserRole, QVariant::fromValue<::tf::AssayReferenceItem>(item));
}

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
void ImRangeParamWidget::GetImAssayOptLog(QString& strOptLog, const std::shared_ptr<tf::GeneralAssayInfo> spAssayInfoUi, const std::vector<std::shared_ptr<im::tf::GeneralAssayInfo>>& vecImAssayInfoUi)
{
	auto MakeStr = [](QString& strOptLog, const QString strName, const QString strValue)
	{
		strOptLog += " ";
		strOptLog += strName;
		strOptLog += ":";
		strOptLog += strValue;
	};
	// 参数检查
	if (nullptr == spAssayInfoUi)
	{
		ULOG(LOG_WARN, "The input param is nullptr.");
		return;
	}
	// 从CommMgr中获取免疫项目信息
	std::vector<std::shared_ptr<im::tf::GeneralAssayInfo>> vecImAssayInfo;
	std::shared_ptr<CommonInformationManager> &assayMgr = CommonInformationManager::GetInstance();
	assayMgr->GetImmuneAssayInfos(vecImAssayInfo, spAssayInfoUi->assayCode, ::tf::DeviceType::DEVICE_TYPE_I6000);
	auto spAssayInfo = assayMgr->GetAssayInfo(spAssayInfoUi->assayCode);
	if (spAssayInfo == nullptr)
	{
		ULOG(LOG_WARN, "Get Assay parameter infor error, by assayCode:%d.", spAssayInfoUi->assayCode);
		return;
	}
	double dFactor = 1;
	for (auto& atUnit : spAssayInfoUi->units)
	{
		if (atUnit.isCurrent)
		{
			dFactor = atUnit.factor;
			break;
		}
	}
	QString strOpen(tr("打开")), strClose(tr("关闭")), strCheck(tr("勾选")), strUncheck(tr("取消勾选")), strEmpty(tr("空"));
	bool autoRerunActive = false, ranges1 = false, ratio1 = false, ranges2 = false, ratio2 = false;
	for (auto& atValUi : vecImAssayInfoUi)
	{
		for (auto& atVal : vecImAssayInfo)
		{
			if (atValUi->version != atVal->version)
			{
				continue;
			}
			// 如果修改了自动复查开关
			if (atValUi->autoRerunActive != atVal->autoRerunActive && !autoRerunActive)
			{
				MakeStr(strOptLog, ui->groupBoxRetest->title(), atValUi->autoRerunActive ? strOpen : strClose);
				autoRerunActive = true;
			}
			// 复查范围检查
			if (!atValUi->autoRerunActive)
			{
				continue;
			}
			// 如果之前没有元素
			if (atVal->rerunSet.ranges.size() <= 0)
			{
				// 设置了复查范围1
				if (atValUi->rerunSet.ranges.size() >= 1 && !ranges1)
				{
					QString strVal;
					double dLower = atValUi->rerunSet.ranges[0].range.lower*dFactor;
					double dUpper = atValUi->rerunSet.ranges[0].range.upper*dFactor;
					if (dLower >= 0)
					{
						strVal = QString::number(dLower, 'f', spAssayInfo->decimalPlace);
						strVal += "-";
						strVal += QString::number(dUpper, 'f', spAssayInfo->decimalPlace);						
					}
					else
					{
						strVal = strEmpty;
					}
					MakeStr(strOptLog, ui->label_5->text(), strVal);
					ranges1 = true;
				}
				// 如果修改了自动复查范围1的稀释倍数
				if (atValUi->rerunSet.ranges.size() >= 1 && !ratio1)
				{
					int16_t iRatio = atValUi->rerunSet.ranges[0].ratio;
					MakeStr(strOptLog, ui->label_5->text() + ui->label_7->text(), iRatio>0 ? QString::number(iRatio) : strEmpty);
					ratio1 = true;
				}
			}
			else
			{
				if (atValUi->rerunSet.ranges.size() >= 1 && !ranges1)
				{
					// 如果修改了自动复查范围1
					if ((!IsEqual(atValUi->rerunSet.ranges[0].range.lower, atVal->rerunSet.ranges[0].range.lower)
						|| !IsEqual(atValUi->rerunSet.ranges[0].range.upper, atVal->rerunSet.ranges[0].range.upper)) && !ranges1)
					{
						QString strVal;
						double dLower = atValUi->rerunSet.ranges[0].range.lower*dFactor;
						double dUpper = atValUi->rerunSet.ranges[0].range.upper*dFactor;
						if (dLower >= 0)
						{
							strVal = QString::number(dLower, 'f', spAssayInfo->decimalPlace);
							strVal += "-";
							strVal += QString::number(dUpper, 'f', spAssayInfo->decimalPlace);
						}
						else
						{
							strVal = strEmpty;
						}
						MakeStr(strOptLog, ui->label_5->text(), strVal);
						ranges1 = true;
					}
				}
				if (atValUi->rerunSet.ranges.size() >= 1 && !ratio1)
				{
					// 如果修改了自动复查范围1的稀释倍数
					if (atValUi->rerunSet.ranges[0].ratio != atVal->rerunSet.ranges[0].ratio)
					{
						int16_t iRatio = atValUi->rerunSet.ranges[0].ratio;
						MakeStr(strOptLog, ui->label_5->text() + ui->label_7->text(), iRatio > 0 ? QString::number(iRatio) : strEmpty);
						ratio1 = true;
					}
				}
				// 如果之前没有范围2元素
				if (atVal->rerunSet.ranges.size() <= 1)
				{
					// 设置了复查范围2
					if (atValUi->rerunSet.ranges.size() >= 2 && !ranges2)
					{
						QString strVal;
						double dLower = atValUi->rerunSet.ranges[1].range.lower*dFactor;
						double dUpper = atValUi->rerunSet.ranges[1].range.upper*dFactor;
						if (dLower >= 0)
						{
							strVal = QString::number(dLower, 'f', spAssayInfo->decimalPlace);
							strVal += "-";
							strVal += QString::number(dUpper, 'f', spAssayInfo->decimalPlace);
						}
						else
						{
							strVal = strEmpty;
						}
						MakeStr(strOptLog, ui->label_8->text(), strVal);
						ranges2 = true;
					}
					// 如果修改了自动复查范围1的稀释倍数
					if (atValUi->rerunSet.ranges.size() >= 2 && !ratio2)
					{
						int16_t iRatio = atValUi->rerunSet.ranges[1].ratio;
						MakeStr(strOptLog, ui->label_8->text() + ui->label_9->text(), iRatio > 0 ? QString::number(iRatio) : strEmpty);
						ratio2 = true;
					}
				}
				else
				{
					if (atValUi->rerunSet.ranges.size() >= 2 && !ranges2)
					{
						// 如果修改了自动复查范围2
						if ((!IsEqual(atValUi->rerunSet.ranges[1].range.lower, atVal->rerunSet.ranges[1].range.lower)
							|| !IsEqual(atValUi->rerunSet.ranges[1].range.upper, atVal->rerunSet.ranges[1].range.upper)) && !ranges2)
						{
							QString strVal;
							double dLower = atValUi->rerunSet.ranges[1].range.lower*dFactor;
							double dUpper = atValUi->rerunSet.ranges[1].range.upper*dFactor;
							if (dLower >= 0)
							{
								strVal = QString::number(dLower, 'f', spAssayInfo->decimalPlace);
								strVal += "-";
								strVal += QString::number(dUpper, 'f', spAssayInfo->decimalPlace);
							}
							else
							{
								strVal = strEmpty;
							}
							MakeStr(strOptLog, ui->label_8->text(), strVal);
							ranges2 = true;
						}
					}
					if (atValUi->rerunSet.ranges.size() >= 2 && !ratio2)
					{
						// 如果修改了自动复查范围2的稀释倍数
						if (atValUi->rerunSet.ranges[1].ratio != atVal->rerunSet.ranges[1].ratio)
						{
							int16_t iRatio = atValUi->rerunSet.ranges[1].ratio;
							MakeStr(strOptLog, ui->label_8->text() + ui->label_9->text(), iRatio > 0 ? QString::number(iRatio) : strEmpty);
							ratio2 = true;
						}
					}
				}
			}
		}
	}
}

///
/// @brief
///     参考区间添加按钮被点击
///
/// @par History:
/// @li 5774/WuHongTao，2021年7月19日，新建函数
///
void ImRangeParamWidget::OnRefAddBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);

    if (CommonInformationManager::GetInstance()->IsExistDeviceRuning())
    {
        TipDlg(tr("仪器运行中禁止修改计算项目参数")).exec();
        return;
    }

	m_optRow = -1;
    m_RangeParaSetWidget->ShowDlg(Et_Add);
}

///
/// @brief
///     参考区间修改按钮被点击
///
///
/// @par History:
/// @li 5774/WuHongTao，2021年7月19日，新建函数
///
void ImRangeParamWidget::OnRefModBtnClicked()
{
    ULOG(LOG_INFO, "%s()", __FUNCTION__);
	
	m_optRow = ui->RefRangeTable->currentRow();

    ::tf::AssayReferenceItem loadRangData;
    if (!GetReferrenceByRow(m_optRow, loadRangData))
    {
        ULOG(LOG_ERROR, "Failed to get referrence by row:%d.", m_optRow);
        return;
    }

    m_RangeParaSetWidget->ShowDlg(Et_Modify);
	m_RangeParaSetWidget->loadRangParaSetWidget(loadRangData);
}

///
/// @bref
///		保存设置信息，
///
/// @par History:
/// @li 8276/huchunli, 2023年12月4日，新建函数
///
void ImRangeParamWidget::SaveRangeInfo()
{
    std::vector<std::shared_ptr<im::tf::GeneralAssayInfo>> vecImAssayInfo;
    CommonInformationManager::GetInstance()->GetImmuneAssayInfos(vecImAssayInfo, m_rowItem.assayCode, ::tf::DeviceType::DEVICE_TYPE_I6000);
    std::vector<std::shared_ptr<im::tf::GeneralAssayInfo>> tmpImAssayInfo;
    for (const std::shared_ptr<::im::tf::GeneralAssayInfo>& imPs : vecImAssayInfo)
    {
        tmpImAssayInfo.push_back(std::make_shared<::im::tf::GeneralAssayInfo>(*imPs));
    }
    if (GetRangeParameter(m_rowItem, tmpImAssayInfo))
    {
        // 保存免疫项目信息
        int idx = 0;
        for (const auto& imAssay : tmpImAssayInfo)
        {
            if (!im::LogicControlProxy::ModifyGeneralAssayInfo(*imAssay))
            {
                ULOG(LOG_WARN, "Failed to save imAssayinfo settings.");
                continue;
            }
            if (idx < vecImAssayInfo.size())
            {
                *(vecImAssayInfo[idx]) = *imAssay;
                idx++;
            }
        }
    }
}

void ImRangeParamWidget::AssignDeviceComb(QComboBox* pComb)
{
    if (pComb == Q_NULLPTR)
    {
        return;
    }
    auto devMap = CommonInformationManager::GetInstance()->GetDeviceMaps();
    pComb->clear();
    pComb->addItem(tr("原模块"), tf::RerunDispatchType::type::RtModuleOriginal);
    if (DictionaryQueryManager::GetInstance()->GetPipeLine())
    {
        pComb->addItem(tr("非原模块"), tf::RerunDispatchType::type::RtModuleNotOriginal);
        pComb->addItem(tr("随机模块"), tf::RerunDispatchType::type::RtModuleRandom);

        for (const auto& item : devMap)
        {
            if (item.second->deviceType != tf::DeviceType::type::DEVICE_TYPE_I6000)
            {
                continue;
            }
            QString strModule = tr("指定模块") + "-" + QString::fromStdString(item.second->name);
            pComb->addItem(strModule, tf::RerunDispatchType::type::RtModuleAppoint);
        }
    }
}

QString ImRangeParamWidget::GetComDeviceShowText(::tf::RerunDispatchType::type curType, const std::string& strDevName)
{
    QString strName;
    switch (curType)
    {
    case tf::RerunDispatchType::RtModuleOriginal:
        strName = tr("原模块");
        break;
    case tf::RerunDispatchType::RtModuleNotOriginal:
        strName = tr("非原模块");
        break;
    case tf::RerunDispatchType::RtModuleRandom:
        strName = tr("随机模块");
        break;
    case tf::RerunDispatchType::RtModuleAppoint:
        strName = tr("指定模块") + "-" + QString::fromStdString(strDevName);
        break;
    default:
        break;
    }
    return strName;
}

void ImRangeParamWidget::UpdateCtrlStatu()
{
    // 未登录时直接返回
    SEG_NO_LOGIN_RETURN;

    bool existRow = ui->RefRangeTable->rowCount() > 0;
    std::shared_ptr<UserInfoManager> userPms = UserInfoManager::GetInstance();

    // 自动复查
    userPms->IsPermissionShow(PSM_IM_ASSAYSET_AUTO_RERUNSET) ? ui->groupBoxRetest->show() : ui->groupBoxRetest->hide();
    ui->groupBoxRetest->setEnabled(userPms->IsPermisson(PSM_IM_ASSAYSET_AUTO_RERUNSET));
    // 新增参考范围
    ui->RefAddBtn->setEnabled(userPms->IsPermisson(PSM_IM_ASSAYSET_ADD_REF_RANGE));
    userPms->IsPermissionShow(PSM_IM_ASSAYSET_ADD_REF_RANGE) ? ui->RefAddBtn->show() : ui->RefAddBtn->hide();
    // 修改参考范围
    ui->RefModBtn->setEnabled(existRow ? userPms->IsPermisson(PSM_IM_ASSAYSET_MODLFY_REF_RANGE) : false);
    userPms->IsPermissionShow(PSM_IM_ASSAYSET_MODLFY_REF_RANGE) ? ui->RefModBtn->show() : ui->RefModBtn->hide();
    // 删除参考范围
    ui->RefDelBtn->setEnabled(existRow ? userPms->IsPermisson(PSM_IM_ASSAYSET_DEL_REF_RANGE) : false);
    userPms->IsPermissionShow(PSM_IM_ASSAYSET_DEL_REF_RANGE) ? ui->RefDelBtn->show() : ui->RefDelBtn->hide();
}

///
/// @brief
///     参考区间删除按钮被点击
///
/// @par History:
/// @li 5774/WuHongTao，2021年7月19日，新建函数
///
void ImRangeParamWidget::OnRefDelBtnClicked()
{
    // 移除选中行
    QModelIndexList sRows = ui->RefRangeTable->selectionModel()->selectedRows();
    std::set<int> deletedRow;
    for (int i = ((int)sRows.size()) - 1; i >= 0; --i)
    {
        int dRow = sRows[i].row();
        if (deletedRow.find(dRow) != deletedRow.end())
        {
            continue;
        }
        deletedRow.insert(dRow);
        ui->RefRangeTable->removeRow(dRow);
    }

    // 重置行序号
    int optCurrentRow = 0;
    while (optCurrentRow < ui->RefRangeTable->rowCount())
    {
        QTableWidgetItem* curItem = ui->RefRangeTable->item(optCurrentRow, 1);
        if (curItem == Q_NULLPTR)
        {
            break;
        }
        if (!curItem->text().isEmpty())
        {
            ui->RefRangeTable->setItem(optCurrentRow, 0, new QTableWidgetItem(QString::number(optCurrentRow + 1))); // 修改序号
        }
        optCurrentRow++;
    }

    // bug230928
    SaveRangeInfo();
    if (deletedRow.size() > 0)
    {
        int newSelect = *(deletedRow.begin());
        ui->RefRangeTable->selectRow(newSelect > 0 ? (newSelect - 1) : 0);
    }

    SetTblTextAlign(ui->RefRangeTable, Qt::AlignCenter, -1); 
    UpdateCtrlStatu();

	// 添加操作日志 add by chenjianlin 20231214
	COperationLogManager::GetInstance()->AddRangeParamOperationLog(tf::OperationType::type::DEL, m_rowItem.name.toStdString());
}

///
/// @brief
///     新增/编辑弹窗保存按钮被点击
///
///
/// @return 
///
/// @par History:
/// @li 7951/LuoXin，2022年6月14日，新建函数
///
void ImRangeParamWidget::OnRefSaveBtnClicked()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	tf::AssayReferenceItem setRangData;
	if (!m_RangeParaSetWidget->getSetParaData(setRangData))
	{
        ULOG(LOG_ERROR, "Failed to get range settings from setting dlg.");
		return;
    }

    // 非缺省值必须填写样本类型
    if (!setRangData.bAutoDefault && setRangData.sampleSourceType == -1)
    {
        TipDlg(tr("非缺省值，需要填写样本类型！")).exec();
        return;
    }

    // 判断是否重复添加
    std::map<int, int> rowIdxMap;
    ::tf::AssayReference rangeItems;
    GetUiReferenceItem(rangeItems, rowIdxMap);
    if (m_RangeParaSetWidget->GetEditType() == Et_Modify)
    {
        std::map<int, int>::iterator dtIter = rowIdxMap.find(m_optRow);
        if (dtIter != rowIdxMap.end() && dtIter->second < rangeItems.Items.size())
        {
            rangeItems.Items[dtIter->second] = setRangData;
        }
    }
    else
    {
        rangeItems.Items.push_back(setRangData);
    }
    if (!UiCommon::CheckAssayReferenceIsValid(rangeItems))
    {
        ULOG(LOG_ERROR, "Exist invalid range item.");
        return;
    }

	int currentOptRow = m_optRow;
	if (currentOptRow == -1)
	{
		currentOptRow = ui->RefRangeTable->rowCount();
        ui->RefRangeTable->setRowCount(currentOptRow + 1);
	} 

	ShowReferrence(currentOptRow, setRangData);
    // bug230928
    SaveRangeInfo();
	// 添加操作日志 add by chenjianlin 20231214
	COperationLogManager::GetInstance()->AddRangeParamOperationLog(
        m_RangeParaSetWidget->GetEditType() == Et_Modify ? tf::OperationType::type::MOD : tf::OperationType::type::ADD, m_rowItem.name.toStdString());

	// 启用修改、删除按钮
    UpdateCtrlStatu();

	// 选中新增行
	ui->RefRangeTable->selectRow(currentOptRow);
    SetTblTextAlign(ui->RefRangeTable, Qt::AlignCenter, -1);

	m_optRow = currentOptRow;
	m_RangeParaSetWidget->close();
}

///
/// @bref
///		数据报警复查设置按钮
///
/// @par History:
/// @li 8276/huchunli, 2023年8月30日，新建函数
///
void ImRangeParamWidget::OnAlarmRetestSettings()
{
    ImDataAlarmRetestSettingsDlg retestAlarmDlg;
    retestAlarmDlg.InitData(m_rowItem.version.toStdString(), m_rowItem.assayCode);
    retestAlarmDlg.exec();
}

///
/// @bref
///		获取界面所有行设置(tbView表的第0列为缓存::tf::AssayReferenceItem的列，缓存id为Qt::UserRole)
///
/// @param[out] rangeItems 界面所有设置项
///
/// @par History:
/// @li 8276/huchunli, 2023年8月3日，新建函数
///
void ImRangeParamWidget::GetUiReferenceItem(::tf::AssayReference& rangeItems, std::map<int, int>& rowIdxMap)
{
    std::vector<::tf::AssayReferenceItem> subItems;
    int rowCnt = ui->RefRangeTable->rowCount();
    int dtIdx = 0;
    for (int i = 0; i < rowCnt; ++i)
    {
        // 空行认为是不重复的
        QTableWidgetItem* tbWidgetItem = ui->RefRangeTable->item(i, 0);
        if (tbWidgetItem == nullptr)
        {
            break;
        }
        if (tbWidgetItem->text().isEmpty())
        {
            break;
        }

        QVariant itemCache = tbWidgetItem->data(Qt::UserRole);
        if (itemCache.isNull() || !itemCache.isValid())
        {
            continue;
        }
        ::tf::AssayReferenceItem curItem = itemCache.value<::tf::AssayReferenceItem>();
        subItems.push_back(curItem);
        rowIdxMap.insert(std::pair<int, int>(i, dtIdx));
        dtIdx++;
    }

    rangeItems.__set_Items(subItems);
}

///
/// @bref
///		检测界面自动复查设置的合法性
///
/// @param[in] imAssayInfo 免疫项目
///
/// @par History:
/// @li 8276/huchunli, 2023年8月31日，新建函数
///
ImRangeParamWidget::UiCtrlValidError ImRangeParamWidget::CheckCtrlValueAutoRetest(const std::shared_ptr<im::tf::GeneralAssayInfo>& imAssayInfo)
{
    if (imAssayInfo == nullptr)
    {
        ULOG(LOG_WARN, "Null im assayinfo.");
        return UV_NO_ERROR;
    }

    bool bOk = false;
    bool bRerunActive = ui->groupBoxRetest->isChecked();
    if (bRerunActive)
    {
        QString strLower1 = ui->lineEditAutoRetestRange1Lower->text();
        QString strUpper1 = ui->lineEditAutoRetestRange1Upper->text();
        QString strRate1 = ui->comboBoxRetestDuliRate1->currentText();
        if (strLower1.isEmpty() ^ strUpper1.isEmpty())
        {
            return UV_AUTORETEST_LEAK_PARAM;
        }
        double reLower1 = 0;
        double reUpper1 = 0;
        if (!strLower1.isEmpty() && !strUpper1.isEmpty())
        {
            // 自动复查范围1低值
            reLower1 = strLower1.toDouble(&bOk);
            if (!bOk || OutOfRangeAutoRecheck(reLower1))
            {
                return UV_AUTORETEST_VALUE_LOWER;
            }
            // 自动复查范围1高值
            reUpper1 = strUpper1.toDouble(&bOk);
            if (!bOk || OutOfRangeAutoRecheck(reUpper1))
            {
                return UV_AUTORETEST_VALUE_UPPER;
            }
            if (reLower1 >= reUpper1)
            {
                return UV_AUTORETEST_RANGE;
            }

            // 自动复查1稀释倍数
            int iDiluRadio1 = ui->comboBoxRetestDuliRate1->currentText().toInt(&bOk);
            if (!bOk)
            {
                return UV_AUTORETEST_DILU_RADIO;
            }

            if (iDiluRadio1 < imAssayInfo->reactInfo.dilutionRatio)
            {
                return UV_AUTORETEST_DILU_RADIO_LESS;
            }
            if (iDiluRadio1 > imAssayInfo->reactInfo.maxDilutionRatio)
            {
                return UV_AUTORETEST_DILU_RADIO_MORE;
            }
        }

        QString strLower2 = ui->lineEditAutoRetestRange2Lower->text();
        QString strUpper2 = ui->lineEditAutoRetestRange2Upper->text();
        QString strRate2 = ui->comboBoxRetestDuliRate2->currentText();
        if (strLower2.isEmpty() ^ strUpper2.isEmpty())
        {
            return UV_AUTORETEST_LEAK_PARAM;
        }
        double reLower2 = 0;
        double reUpper2 = 0;
        if (!strLower2.isEmpty() && !strUpper2.isEmpty())
        {
            // 自动复查范围2低值
            reLower2 = strLower2.toDouble(&bOk);
            if (!bOk || OutOfRangeAutoRecheck(reLower2))
            {
                return UV_AUTORETEST_VALUE2_LOWER;
            }
            // 自动复查范围2高值
            reUpper2 = strUpper2.toDouble(&bOk);
            if (!bOk || OutOfRangeAutoRecheck(reUpper2))
            {
                return UV_AUTORETEST_VALUE2_UPPER;
            }
            if (reLower2 >= reUpper2)
            {
                return UV_AUTORETEST_RANGE2;
            }

            // 自动复查2稀释倍数
            int iDiluRadio2 = ui->comboBoxRetestDuliRate2->currentText().toInt(&bOk);
            if (!bOk)
            {
                return UV_AUTORETEST_DILU_RADIO2;
            }

            if (iDiluRadio2 < imAssayInfo->reactInfo.dilutionRatio)
            {
                return UV_AUTORETEST_DILU_RADIO2_LESS;
            }
            if (iDiluRadio2 > imAssayInfo->reactInfo.maxDilutionRatio)
            {
                return UV_AUTORESTES_DILU_RADIO2_MORE;
            }

            // 判断重叠
            if ((reLower1 <= reLower2 && reUpper1 >= reLower2) || (reLower1 >= reLower2 && reLower1 <= reUpper2))
            {
                return UV_AUTORESTES_RANGE_COVER;
            }
        }
    }

    return UV_NO_ERROR;
}
