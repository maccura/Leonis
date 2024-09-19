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
/// @file     QCalibrationFactorEdit.cpp
/// @brief    校准系数编辑
///
/// @author   8580/GongZhiQiang
/// @date     2023年1月6日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年1月6日，新建文件
///
///////////////////////////////////////////////////////////////////////////

#include "QCalibrationFactorEdit.h"
#include "ui_QCalibrationFactorEdit.h"

#include "shared/CommonInformationManager.h"
#include "shared/ReagentCommon.h"
#include "shared/QComDelegate.h"
#include "shared/tipdlg.h"
#include "src/common/TimeUtil.h"
#include "src/common/Mlog/mlog.h"
#include "thrift/ch/c1005/C1005LogicControlProxy.h"
#include "CaliBrateCommom.h"


QCalibrationFactorEdit::QCalibrationFactorEdit(QWidget *parent)
	: BaseDlg(parent)
{
    ui = new Ui::QCalibrationFactorEdit();
	SetTitleName(tr("编辑校准结果"));
	ui->setupUi(this);

	connect(ui->pushButton_close, &QPushButton::clicked, this, [this] { close(); });
	connect(ui->save_button, &QPushButton::clicked, this, &QCalibrationFactorEdit::saveCalibrationFactor);

	// K限制输入范围
	ChDoubleValidator* validatorK = new ChDoubleValidator();
	validatorK->setNotation(QDoubleValidator::StandardNotation);
	ui->lineEdit_K->setValidator(validatorK);

	// Slabs限制输入范围
    ChDoubleValidator* validatorS1abs = new ChDoubleValidator();
	validatorS1abs->setNotation(QDoubleValidator::StandardNotation);
	ui->lineEdit_S1abs->setValidator(validatorS1abs);
	
}

QCalibrationFactorEdit::~QCalibrationFactorEdit()
{
}


///
/// @brief 初始化校准系数编辑对话框
///
/// @param[in]  rowIndex  当前操作校准申请的索引
/// @param[in]  reagent   试剂详细信息
///
/// @return true:初始化成功 false：初始化不成功
/// @par History:
/// @li 8580/GongZhiQiang，2023年1月6日，新建函数
///
bool QCalibrationFactorEdit::initDialog(int rowIndex, ch::tf::ReagentGroup& reagent)
{
	// 查找设备类型
	auto chDeviceInfo = CommonInformationManager::GetInstance()->GetDeviceInfo(reagent.deviceSN);
	if (chDeviceInfo == nullptr)
	{
		ULOG(LOG_ERROR, "%s():GetDeviceInfo() Failed!", __FUNCTION__);
		return false;
	}

	// 查找生化通用信息
	auto chGAI = CommonInformationManager::GetInstance()->GetChemistryAssayInfo(reagent.assayCode, chDeviceInfo->deviceType);
	if (chGAI == nullptr)
	{
		ULOG(LOG_ERROR, "%s():GetChemistryAssayInfo() Failed!", __FUNCTION__);
		return false;
	}

	// 判断校准方法是否为线性
	if (!checkCurveIsLine(chGAI->calibrationType))
	{
		std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("编辑校准结果"), tr("校准曲线为非线性")));
		pTipDlg->exec();
		return false;
	}

	// 存储当前项目通用信息和试剂信息
	currentGAI = chGAI;
	currentReagent = reagent;
	currentRow = rowIndex;

	// 初始化界面信息
	if (!initUI())
	{
		ULOG(LOG_ERROR, "%s():initUI() Failed!", __FUNCTION__);
		return false;
	}

	return true;
}



///
/// @brief 初始化界面信息
///                  
/// @return true：初始化成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年1月6日，新建函数
///
bool QCalibrationFactorEdit::initUI()
{
	// 模块名称
	ui->label_modelName->setText(QString::fromStdString(CommonInformationManager::GetDeviceName(currentReagent.deviceSN)));

	// 项目名称 
	std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(currentReagent.assayCode);
	if (spAssayInfo == nullptr)
	{
		ULOG(LOG_ERROR, "%s():GetAssayInfo() Failed!", __FUNCTION__);
		return false;
	}
	ui->label_projectName->setText(QString::fromStdString(spAssayInfo->assayName));
    // 输入值初始化
    ui->lineEdit_K->clear();
    ui->lineEdit_S1abs->clear();
	
	// 初始化输入值
	// 如果是编辑曲线，需要初始化输入值，直接更改k，b值；
	// 如果不是初始化曲线，则直接清空，增加一条编辑曲线。
    auto currentCurve = getCurrentCurve(currentReagent);
    if (currentCurve == nullptr) {
        ULOG(LOG_ERROR, "%s():getCurrentCurve() Failed!", __FUNCTION__);
        return true;
    }

	//if (currentCurve->isEditCurve)
    {
        // 获取曲线句柄
        auto cureveHandel = CaliBrateCommom::GetCurve(*currentCurve);
        if (cureveHandel == nullptr)
        {
            ULOG(LOG_ERROR, "%s():getCurrentCurve() Failed!", __FUNCTION__);
            return true;
        }

        /**************需要添加对曲线的直接判断，判断其是否是编辑的曲线**************/

		std::map<std::string, double> parameters = cureveHandel->GetModelParameters();
		// 设置K值
		if (parameters.find("k") != parameters.end()) {
            ChDoubleValidator* pValiK = static_cast<ChDoubleValidator*>(const_cast<QValidator *>(ui->lineEdit_K->validator()));
            QString K = std::move(QString::number(parameters["k"]));
            if (pValiK)
                K = std::move(pValiK->GetValidNumber(K, parameters["k"]));

			ui->lineEdit_K->setText(K);
		}
		// 设置S1abs值
		if (parameters.find("b") != parameters.end()) {
            ChDoubleValidator* pValiS1abs = static_cast<ChDoubleValidator*>(const_cast<QValidator *>(ui->lineEdit_S1abs->validator()));
            QString S1abs = std::move(QString::number(parameters["b"]));
            if (pValiS1abs)
                S1abs = std::move(pValiS1abs->GetValidNumber(S1abs, parameters["b"]));

			ui->lineEdit_S1abs->setText(S1abs);
		}
	}

	return true;
}


///
/// @brief 检查曲线，如果校准方法为非线性，则不能进行校准编辑
///                  
///
/// @param[in]  caliType  校准方法类型
///
/// @return true：线性  false：非线性
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年1月6日，新建函数
///
bool QCalibrationFactorEdit::checkCurveIsLine(const ch::tf::CalibrationType::type& caliType)
{
	switch (caliType)
	{
	case ch::CALIBRATION_TYPE::CALIBRATION_TYPE_ONE_PONIT_LINEAR:
		ui->label_caliMethod->setText(QObject::tr("一点线性法"));
		break;
	case ch::CALIBRATION_TYPE::CALIBRATION_TYPE_TWO_POINTS_LINEAR:
		ui->label_caliMethod->setText(QObject::tr("两点线性法"));
		break;
	case ch::CALIBRATION_TYPE::CALIBRATION_TYPE_MULTI_POINTS_LINEAR:
		ui->label_caliMethod->setText(QObject::tr("多点线性法"));
		break;
	default:
		return false;
		break;
	}

	return true;
}


///
/// @brief 分析方法转换（ch::tf::AnalysisMethod的转换成ch::ANALYSIS_METHOD）
///                  
/// @return true：转换成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年1月6日，新建函数
///
bool QCalibrationFactorEdit::analysisMethodExchange(const ch::tf::AnalysisMethod::type& chTfMethod, ch::ANALYSIS_METHOD& chMethod)
{
	switch (chTfMethod)
	{
	case ch::tf::AnalysisMethod::ANALYSIS_METHOD_RATE_A:
		chMethod = ch::ANALYSIS_METHOD::ANALYSIS_METHOD_RATE_A;
		break;

	case ch::tf::AnalysisMethod::ANALYSIS_METHOD_1POINT:
		chMethod = ch::ANALYSIS_METHOD::ANALYSIS_METHOD_ONE_POINT;
		break;

	case ch::tf::AnalysisMethod::ANALYSIS_METHOD_2POINT_END:
		chMethod = ch::ANALYSIS_METHOD::ANALYSIS_METHOD_TWO_POINTS;
		break;

	case ch::tf::AnalysisMethod::ANALYSIS_METHOD_2POINT_RATE:
		chMethod = ch::ANALYSIS_METHOD::ANALYSIS_METHOD_TWO_POINTS_RATE;
		break;

    case ch::tf::AnalysisMethod::ANALYSIS_METHOD_RATE_A_SAMPLE_BLANK:
        chMethod = ch::ANALYSIS_METHOD::ANALYSIS_METHOD_RATE_A_SAMPLE_BLANK;
        break;

	default:
		return false;
		break;
	}

	return true;
}

///
/// @brief 校准类型转换（ch::tf::CalibrationType的转换成ch::CALIBRATION_TYPE）
///                  
/// @return true：转换成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年1月6日，新建函数
///
bool QCalibrationFactorEdit::calibrationTypeExchange(const ch::tf::CalibrationType::type& chTfType, ch::CALIBRATION_TYPE& chType)
{
	switch (chTfType)
	{
	case ch::tf::CalibrationType::CALIBRATION_TYPE_LINEAR_ONE_POINT:
		chType = ch::CALIBRATION_TYPE::CALIBRATION_TYPE_ONE_PONIT_LINEAR;
		break;
	case ch::tf::CalibrationType::CALIBRATION_TYPE_LINEAR_TWO_POINTS:
		chType = ch::CALIBRATION_TYPE::CALIBRATION_TYPE_TWO_POINTS_LINEAR;
		break;
	case ch::tf::CalibrationType::CALIBRATION_TYPE_LINEAR_MULTI_POINTS:
		chType = ch::CALIBRATION_TYPE::CALIBRATION_TYPE_MULTI_POINTS_LINEAR;
		break;
	default:
		return false;
		break;
	}

	return true;
}


///
/// @brief 增加一条编辑曲线，创建成功则返回曲线ID
///                  
///
/// @param[in]  value_K       设置的k值
/// @param[in]  value_S1labs  设置的b值
/// @param[in]  caliCurveId   新增曲线的ID
///
/// @return true：增加成功  false：增加失败
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年1月6日，新建函数
///
bool QCalibrationFactorEdit::addNewCurve(const double& value_K, const double& value_S1labs, int &caliCurveId)
{
	// 分析方法转换
	ch::ANALYSIS_METHOD aMethod;
	if (!analysisMethodExchange(currentGAI->analysisMethod, aMethod)) {
		ULOG(LOG_ERROR, "%s():analysisMethodExchange() Failed!", __FUNCTION__);
		return false;
	}

	// 校准类型转换
	ch::CALIBRATION_TYPE cType;
	if (!calibrationTypeExchange(currentGAI->calibrationType, cType)) {
		ULOG(LOG_ERROR, "%s():calibrationTypeExchange() Failed!", __FUNCTION__);
		return false;
	}

    // 查询优先级最高的生化特殊项目配置参数
    std::shared_ptr<ch::tf::SpecialAssayInfo> spSAI = CommonInformationManager::GetInstance()->GetHighestPrioritySpecialAssay(
        currentGAI->assayCode, currentGAI->deviceType, currentGAI->version);
    if (nullptr == spSAI)
    {
        ULOG(LOG_ERROR, "Failed to execute GetHighestPrioritySpecialAssay()");
        return false;
    }

    // 检查样本吸取信息列表
    if (spSAI->sampleAspirateVols.empty())
    {
        ULOG(LOG_ERROR, "sampleAspirateVols is empty");
        return false;
    }
	
	// 创建校准曲线对象
    ch::chAlgParams algParam(aMethod, cType);
	std::shared_ptr<::ch::ICalibrateCurve> spCaliCurve = ch::CreateCaliCurve(algParam);
	if (spCaliCurve == nullptr)
	{
		ULOG(LOG_ERROR, "%s():CreateCaliCurve() Failed!", __FUNCTION__);
		return false;
	}

    // 生成校准曲线核心参数
    std::string json;
    try
    {
        ch::CalibrateInitWithoutCalibrations ciwc;
        // 是否将前一点纳入吸光度计算
        ciwc.analyPoint.takePrePoint = currentGAI->analyPoints.takePrePoint;
        // m
        if (currentGAI->analyPoints.m >= 1)
        {
            ciwc.analyPoint.m = currentGAI->analyPoints.m;
        }

        // n
        if (currentGAI->analyPoints.n >= 1)
        {
            ciwc.analyPoint.n = currentGAI->analyPoints.n;
        }

        // p
        if (currentGAI->analyPoints.p >= 1)
        {
            ciwc.analyPoint.p = currentGAI->analyPoints.p;
        }

        // q
        if (currentGAI->analyPoints.q >= 1)
        {
            ciwc.analyPoint.q = currentGAI->analyPoints.q;
        }

        // 样本基准吸取信息
        ciwc.standardAspirate.m_originalSample  = spSAI->sampleAspirateVols[0].originalSample;
        if (spSAI->sampleAspirateVols[0].__isset.sample4Test)
        {
            ciwc.standardAspirate.m_sample4Test = spSAI->sampleAspirateVols[0].sample4Test;
        }
        
        if (spSAI->sampleAspirateVols[0].__isset.diluent)
        {
            ciwc.standardAspirate.m_diluent = spSAI->sampleAspirateVols[0].diluent;
        }

        // 样本基准吸取信息
        for (const ::ch::tf::ReagentAspirateVol& rav: currentGAI->reagentAspirateVols)
        {
            // 忽略吸取量为0的试剂阶段
            if (rav.reagent == 0)
            {
                break;
            }

            int iReagentIdx = GetReagentStageIdx(rav.stage);
            ciwc.reagentAspirate[iReagentIdx] = (rav.reagent + rav.diluent);
        }

        // 模型参数
        ciwc.modelParameters["k"] = value_K;
        ciwc.modelParameters["b"] = value_S1labs;

        // 无校准品初始化校准曲线
        json = spCaliCurve->Init(ciwc);
    }
    catch (std::exception& ex)
    {
        ULOG(LOG_ERROR, "exception: %s", ex.what());
        return false;
    }
    
	// 设置编辑的曲线信息
	::ch::tf::CaliCurve newCurve;
	newCurve.__set_assayCode(currentReagent.assayCode);
	newCurve.__set_deviceSN(currentReagent.deviceSN);
	newCurve.__set_version("1");
	newCurve.__set_suppliesCode(currentReagent.suppliesCode);
	newCurve.__set_calibratorLot("");
	newCurve.__set_reagentKeyInfos(currentReagent.reagentKeyInfos);
	newCurve.__set_lotCurve(ch::tf::CurveUsage::CURVE_USAGE_NONE);
	newCurve.__set_assayCurve(ch::tf::CurveUsage::CURVE_USAGE_NONE);
	newCurve.__set_caliTime(PosixTimeToTimeString(boost::posix_time::second_clock::local_time()));
	newCurve.__set_curveData(json);
	newCurve.__set_isEditCurve(true);

	// 获取耗材信息,获取试剂批号
	auto supplyInfo = GetSupplyInfoByReagent(currentReagent);
	if (supplyInfo == nullptr)
	{
		ULOG(LOG_ERROR, "%s():GetSupplyInfoByReagent() Failed!", __FUNCTION__);
		return false;
	}
	newCurve.__set_reagentLot(supplyInfo->lot);
	

	::tf::ResultLong _return;
	ch::c1005::LogicControlProxy::AddCaliCurve(_return, newCurve);
	if (_return.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
	{
		ULOG(LOG_ERROR, "%s():AddCaliCurve() Failed!", __FUNCTION__);
		return false;
	}

	// 返回主键ID
	caliCurveId = _return.value;	
	return true;
}

///
/// @brief 修改旧曲线（暂时不用此函数）
///                  
///
/// @param[in]  value_K       设置的k值
/// @param[in]  value_S1labs  设置的b值
///
/// @return true：修改成功  false：修改失败
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年1月6日，新建函数
///
bool QCalibrationFactorEdit::modifyOldCurve(const double& value_K, const double& value_S1labs)
{
	// 获取曲线句柄
	auto cureveHandel = getCurveHandel(currentReagent);
	if (cureveHandel == nullptr)
	{
		ULOG(LOG_ERROR, "%s():getCurrentCurve() Failed!", __FUNCTION__);
		return false;
	}

	std::map<std::string, double> parameters = cureveHandel->GetModelParameters();

	// 设置K值
	if (parameters.find("k") != parameters.end()) {
		parameters["k"] = value_K;
	}
	// 设置Slabs值
	if (parameters.find("b") != parameters.end()) {
		parameters["b"] = value_S1labs;
	}

	auto json = cureveHandel->SetModelParametersTemporary(parameters);

	return true;

}

///
/// @brief 执行保存
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年1月6日，新建函数
///
void QCalibrationFactorEdit::saveCalibrationFactor()
{
	// 参数检查
	double value_K = ui->lineEdit_K->text().toDouble();
	double value_S1abs = ui->lineEdit_S1abs->text().toDouble();
	
	// 检查K值是否为0
	if (value_K == 0) {
		std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("编辑校准结果"), tr("K不能为0，请重新输入")));
		pTipDlg->exec();
		return;
	}

	// 创建一条校准曲线，并将其添加进数据库
    int curveId = -1;
    if (!addNewCurve(value_K, value_S1abs, curveId)) {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("编辑校准结果"), tr("校准曲线添加失败")));
        pTipDlg->exec();
        ULOG(LOG_ERROR, "%s():addNewCurve() Failed!", __FUNCTION__);
    }
    /*else
    {
        emit sigCalibrationFactorEdited(currentRow, curveId);
    }*/

    // 使用新的对象来修改，只上传修改的值，防止有些状态已经变化被冲掉
    ch::tf::ReagentGroup newGroupInfo;
    newGroupInfo.__set_id(currentReagent.id);
    // 校准状态设置为编辑
    newGroupInfo.__set_caliStatus(tf::CaliStatus::CALI_STATUS_EDIT);
    // 设置曲线ID
    newGroupInfo.__set_caliCurveId(curveId);
	// 设置曲线来源为编辑
	newGroupInfo.__set_caliCurveSource(tf::CurveSource::CURVE_SOURCE_EDIT);
    // 修改数据
    if (!ch::c1005::LogicControlProxy::ModifyReagentGroup(newGroupInfo))
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("编辑校准结果"), tr("校准曲线添加失败")));
        pTipDlg->exec();
        ULOG(LOG_ERROR, "%s():ModifyReagentGroup() Failed, reagentId is %lld!", __FUNCTION__, currentReagent.id);
    }

    currentReagent.__set_caliCurveId(curveId);
	close();
}

int QCalibrationFactorEdit::GetReagentStageIdx(::ch::tf::ReagentStage::type enType)
{
    if (enType == ::ch::tf::ReagentStage::REAGENT_STAGE_R1)
    {
        return 1;
    }
    else if (enType == ::ch::tf::ReagentStage::REAGENT_STAGE_R2)
    {
        return 2;
    }
    else if (enType == ::ch::tf::ReagentStage::REAGENT_STAGE_R3)
    {
        return 3;
    }
    else
    {
        return 4;
    }
}

///
/// @brief 获取试剂的当前校准曲线
///
/// @param[in]  reagent  试剂信息
///
/// @return 校准曲线
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年1月6日，新建函数
///
std::shared_ptr<ch::tf::CaliCurve> QCalibrationFactorEdit::getCurrentCurve(const ch::tf::ReagentGroup& reagent)
{
	::ch::tf::CaliCurveQueryResp CurveRsp;
	::ch::tf::CaliCurveQueryCond CurveQuery;
	CurveQuery.__set_id(reagent.caliCurveId);

	// 查询曲线数据
	ch::c1005::LogicControlProxy::QueryCaliCurve(CurveRsp, CurveQuery);
	if (CurveRsp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS || CurveRsp.lstCaliCurves.empty())
	{
		return nullptr;
	}

	return make_shared<ch::tf::CaliCurve>(CurveRsp.lstCaliCurves[0]);
}


///
/// @brief 获取当前试剂曲线句柄
///
/// @param[in]  reagent  试剂信息
///
/// @return 曲线句柄
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年1月6日，新建函数
///
std::shared_ptr<ch::ICalibrateCurve> QCalibrationFactorEdit::getCurveHandel(const ch::tf::ReagentGroup& reagent)
{

	// 获取校准曲线
	auto currentCurve = getCurrentCurve(reagent);
	if (currentCurve == nullptr) {
		ULOG(LOG_ERROR, "%s():getCurrentCurve() Failed!", __FUNCTION__);
		return nullptr;
	}

	// 获取曲线句柄
	auto cureveHandel = CaliBrateCommom::GetCurve(*currentCurve);
	if (cureveHandel == nullptr)
	{
		ULOG(LOG_ERROR, "%s():getCurrentCurve() Failed!", __FUNCTION__);
		return nullptr;
	}

	return cureveHandel;
}
