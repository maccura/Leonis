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
/// @file     QCaliBratePostionEdit.cpp
/// @brief    位置编辑
///
/// @author   5774/WuHongTao
/// @date     2022年3月11日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月11日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QPostionEdit.h"
#include "ui_QPostionEdit.h"
#include <QRegExpValidator>
#include <QPushButton>

#include "shared/ReagentCommon.h"
#include "shared/tipdlg.h"
#include "src/common/defs.h"
#include "src/common/Mlog/mlog.h"
#include "src/public/DictionaryDecode.h"
#include "manager/DictionaryQueryManager.h"
#include "thrift/ch/ChLogicControlProxy.h"
#include "thrift/im/ImLogicControlProxy.h"
#include "thrift/DcsControlProxy.h"

QPostionEdit::QPostionEdit(QWidget *parent)
    : BaseDlg(parent),
    m_Level(-1)
{
    ui = new Ui::QPostionEdit();
    ui->setupUi(this);
    InitSignalConnect();
}

QPostionEdit::~QPostionEdit()
{
}

///
/// @brief 设置校准品的列表
///
/// @param[in]  docs   校准品列表
/// @param[in]  level  水平
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年8月23日，新建函数
///
bool QPostionEdit::SetCaliDoc(const QList<ch::tf::CaliDoc>& docs, int level)
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);

    m_module = MODEL::CALI;
    m_Level = level;
    int docIndex = level - 1;
    m_mapPoses.clear();
    for (int idx = level - 1; idx < docs.size(); idx++)
    {
        auto doc = docs[idx];
        if (!doc.rack.empty())
        {
            std::string strPos = doc.rack + "-" + std::to_string(doc.pos);
            m_mapPoses[strPos] = strPos;
        }
    }
    // 参数检查
    if (docs.size() > 0 && docIndex > -1 && docIndex < docs.size())
    {
        InitUiData(docs[docIndex].tubeType, docs[docIndex].rack, docs[docIndex].pos);
        m_CaliDocs = docs;
    }
    else
    {
        // 全部清空
        InitUiData(::tf::TubeType::TUBE_TYPE_INVALID, "", -1);
    }
    StartInit(m_module);
    return true;
}

///
/// @brief 获取校准文档列表
///
///
/// @return 校准文档列表
///
/// @par History:
/// @li 5774/WuHongTao，2022年8月23日，新建函数
///
boost::optional<QList<ch::tf::CaliDoc>> QPostionEdit::GetCaliDoc()
{
    if (m_module == MODEL::CALI)
    {
        return m_CaliDocs;
    }

    return boost::none;
}

///
/// @brief 设置质控品的列表和水平
///
/// @param[in]  docs   质控品列表
/// @param[in]  level  水平
///
/// @return 质控品列表
///
/// @par History:
/// @li 5774/WuHongTao，2022年8月23日，新建函数
///
bool QPostionEdit::SetQcDoc(const QList<::tf::QcDoc>& docs, int level)
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);

    m_module = MODEL::QC;
    m_Level = level;
    m_mapPoses.clear();

    if (docs.size() > 0)
    {
        auto doc = docs[0];
        if (!doc.rack.empty())
        {
            std::string strPos = doc.rack + "-" + std::to_string(doc.pos);
            m_mapPoses[strPos] = strPos;
        }
        InitUiData(docs[0].tubeType, docs[0].rack, docs[0].pos);
        m_QcDocs = docs;
    }
    StartInit(m_module);
    return true;
}

///
/// @brief 获取质控品的列表
///
///
/// @return 质控品列表
///
/// @par History:
/// @li 5774/WuHongTao，2022年8月23日，新建函数
///
boost::optional<QList<::tf::QcDoc>> QPostionEdit::GetQcDoc()
{
    if (m_module == MODEL::QC)
    {
        return m_QcDocs;
    }

    return boost::none;
}

///
/// @brief 设置免疫校准品的列表
///
/// @param[in]  docs   校准品列表
/// @param[in]  level  水平
///
/// @return true表示成功
///
/// @par History:
/// @li 1226/zhangjing，2022年12月13日，新建函数
///
bool QPostionEdit::SetIMCaliDoc(const QList<im::tf::CaliDoc>& docs, int level)
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);
    if (docs.size() <= 0 || level > docs.size())
    {
        ULOG(LOG_ERROR, "docs size err or level err.");
        return false;
    }

    m_module = MODEL::IMCALI;
    m_Level = level;
    m_mapPoses.clear();
    for (int idx = level - 1; idx < docs.size(); idx++)
    {
        const im::tf::CaliDoc& doc = docs[idx];
        if (!doc.rack.empty())
        {
            std::string strPos = doc.rack + "-" + std::to_string(doc.pos);
            m_mapPoses[strPos] = strPos;
        }
    }
    InitUiData(docs[m_Level - 1].tubeType, docs[m_Level - 1].rack, docs[m_Level - 1].pos);
    m_IMCaliDocs = docs;
    StartInit(m_module);
    return true;
}

///
/// @brief 获取免疫校准文档列表
///
/// @return 校准文档列表
///
/// @par History:
/// @li 1226/zhangjing，2022年12月13日，新建函数
///
boost::optional<QList<im::tf::CaliDoc>> QPostionEdit::GetIMCaliDoc()
{
    if (m_module == MODEL::IMCALI)
    {
        return m_IMCaliDocs;
    }

    return boost::none;
}

///
/// @brief 根据模式启动对应的对话框
///
/// @param[in]  module  模式（校准或者质控）
///
/// @return true表示成功
///
/// @par History:
/// @li 5774/WuHongTao，2022年8月23日，新建函数
///
bool QPostionEdit::StartInit(MODEL module)
{
    // 限制架号范围
    std::shared_ptr<RackNumRange> rackSharedPointer = DictionaryQueryManager::GetInstance()->GetRackRange();
    if (rackSharedPointer)
    {
        std::shared_ptr<QRegExp> regRxpSp;
        // 校准
        if (module == MODEL::CALI || module == MODEL::IMCALI)
        {
            regRxpSp = GenRegexpByText(rackSharedPointer->caliLowerLmt, rackSharedPointer->caliUpperLmt);
        }
        // 质控
        else
        {
            regRxpSp = GenRegexpByText(rackSharedPointer->qcLowerLmt, rackSharedPointer->qcUpperLmt);
        }

        QValidator *validatorRack = nullptr;
        if (regRxpSp == nullptr)
        {
            // 默认的规则
            QRegExp regxRack("^[0-9]*[1-9][0-9]*$");
            validatorRack = new QRegExpValidator(regxRack, ui->rack_edit);
        }
        else
        {
            validatorRack = new QRegExpValidator(*regRxpSp, ui->rack_edit);
        }

        ui->rack_edit->setValidator(validatorRack);
    }

    // 设置对话框的名称
    SetTitleName(tr("位置"));
    return true;
}

///
/// @brief  根据架号更新按钮是否可用
///
/// @param[in]  rack  架号
///
/// @return 
///
/// @par History:
/// @li 1226/zhangjing，2023年9月7日，新建函数
///
void QPostionEdit::ResetPosAvable(const std::string& rack)
{
    // 已占用位置灰化
    for (size_t iPos = 1; iPos <= 5; iPos++)
    {
        m_pushButtons[iPos - 1]->setAutoExclusive(false);
        m_pushButtons[iPos - 1]->setChecked(false);
        m_pushButtons[iPos - 1]->setDisabled(false);

        if (rack.empty())
        {
            m_pushButtons[iPos - 1]->setAutoExclusive(true);
            continue;
        }
        std::string strPos = rack + "-" + std::to_string(iPos);
        if (m_mapPoses.find(strPos) != m_mapPoses.end())
        {
            m_pushButtons[iPos - 1]->setAutoExclusive(true);
            continue;
        }
        if (!IsPositonBeUsing(rack, iPos, false))
        {
            m_pushButtons[iPos - 1]->setAutoExclusive(true);
            continue;
        }
        m_pushButtons[iPos - 1]->setDisabled(true);
        m_pushButtons[iPos - 1]->setAutoExclusive(true);
    }
}

void QPostionEdit::InitUiData(tf::TubeType::type tubetype, const std::string& rackNo, int pos)
{
    // 设置架号
    ui->rack_edit->setText(QString::fromStdString(rackNo));
    ui->rack_edit->setFocus();

    // 无架号则清空不可用
    if (rackNo.empty())
    {
        ui->clr_Button->setEnabled(false);
    }
    else
    {
        ui->clr_Button->setEnabled(true);
        ui->rack_edit->selectAll();
    }
    ResetPosAvable(rackNo);

    // 位置 [1,5]
    if (pos > 0 && pos <= 5)
    {
        m_pushButtons[pos - 1]->setChecked(true);
    }
    else
    {
        foreach(auto btn, m_pushButtons)
        {
            if (btn->isChecked())
            {
                btn->setAutoExclusive(false);
                btn->setChecked(false);
                btn->setAutoExclusive(true);
            }

        }
    }

    // 设置试管类型
    switch (tubetype)
    {
    case tf::TubeType::TUBE_TYPE_NORMAL:
        ui->normal_Button->setChecked(true);
        break;
    case tf::TubeType::TUBE_TYPE_MICRO:
        ui->micro_Button->setChecked(true);
        break;
    case tf::TubeType::TUBE_TYPE_ORIGIN:
        ui->ori_Button->setChecked(true);
        break;
    case tf::TubeType::TUBE_TYPE_STORE:
        ui->store_Button->setChecked(true);
        break;
    default:
        ui->normal_Button->setAutoExclusive(false);
        ui->micro_Button->setAutoExclusive(false);
        ui->ori_Button->setAutoExclusive(false);
        ui->store_Button->setAutoExclusive(false);

        ui->normal_Button->setChecked(false);
        ui->store_Button->setChecked(true); // 修改问题:校准质控全默认选择"样本贮存管"
        ui->ori_Button->setChecked(false);
        ui->micro_Button->setChecked(false);

        ui->normal_Button->setAutoExclusive(true);
        ui->micro_Button->setAutoExclusive(true);
        ui->ori_Button->setAutoExclusive(true);
        ui->store_Button->setAutoExclusive(true);
        break;
    }
}

///
/// @brief 
///
///     初始化信号连接
///
/// @return 
///
/// @par History:
/// @li 8090/YeHuaNing，2022年8月30日，新建函数
///
void QPostionEdit::InitSignalConnect()
{
    // 添加按钮list
    m_pushButtons.push_back(ui->one_Button);
    m_pushButtons.push_back(ui->two_Button);
    m_pushButtons.push_back(ui->three_Button);
    m_pushButtons.push_back(ui->four_Button);
    m_pushButtons.push_back(ui->five_Button);

    // 确认按钮
    connect(ui->ok_Button, SIGNAL(clicked()), this, SLOT(OnSavePostion()));
    // 关闭按钮
    connect(ui->close_Button, SIGNAL(clicked()), this, SLOT(close()));
    // 清空按钮
    connect(ui->clr_Button, &QPushButton::clicked, this, &QPostionEdit::OnClearPosition);
    // 架号编辑框变化
    connect(ui->rack_edit, SIGNAL(textChanged(const QString)), this, SLOT(OnRackChanged(QString)));
}

///
/// @brief 
///
///     点击清空按钮
///
/// @return 
///
/// @par History:
/// @li 8090/YeHuaNing，2022年8月30日，新建函数
///
void QPostionEdit::OnClearPosition()
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);

    //清除架号
    InitUiData(tf::TubeType::TUBE_TYPE_INVALID, "", 0);

    if (m_module == MODEL::CALI)
    {
        // 依次修改后面水平的校准品的位置了类型
        for (int i = 0; i < m_CaliDocs.size(); i++)
        {
            // 初始化为无效的位置和架号
            m_CaliDocs[i].__set_rack("");
            m_CaliDocs[i].__set_pos(0);
            // 初始化为无效样本杯
            m_CaliDocs[i].__set_tubeType(tf::TubeType::TUBE_TYPE_INVALID);
        }
        emit ModifyCaliPostion();
    }
    else if (m_module == MODEL::IMCALI)
    {
        // 依次修改后面水平的校准品的位置了类型
        for (int i = 0; i < m_IMCaliDocs.size(); i++)
        {
            // 初始化为无效的位置和架号
            m_IMCaliDocs[i].__set_rack("");
            m_IMCaliDocs[i].__set_pos(0);
            // 初始化为“样本贮存管” bug22031
            m_IMCaliDocs[i].__set_tubeType(tf::TubeType::TUBE_TYPE_STORE);
        }
        emit ModifyIMCaliPostion();
    }
    else
    {
        // 依次修改后面水平质控品的位置了类型
        for (int i = 0; i < m_QcDocs.size(); i++)
        {
            // 初始化为无效的位置和架号
            m_QcDocs[i].__set_rack("");
            m_QcDocs[i].__set_pos(0);
            // 初始化为无效样本杯
            m_QcDocs[i].__set_tubeType(tf::TubeType::TUBE_TYPE_INVALID);
        }
        emit ModifyQcPostion();
    }

    close();
}

bool QPostionEdit::CheckPositionInvalid(tf::TubeType::type tubetype, const QString& rackNo, int pos)
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);

    bool exit = ((m_module == MODEL::CALI && m_CaliDocs.empty()) ||
        (m_module == MODEL::IMCALI && m_IMCaliDocs.empty()) ||
        (m_module == MODEL::QC && m_QcDocs.empty()));

    if (pos < 0 || m_Level < 0 || tubetype <= tf::TubeType::TUBE_TYPE_INVALID || rackNo.isEmpty() || exit)
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("保存失败：未设置架号、位置号或者样本管类型。")));
        pTipDlg->exec();
        ULOG(LOG_ERROR, "Invalid pos or level or tubetype...");
        return false;
    }

    // 判断架构是否超出范围
    auto  spRack = DictionaryQueryManager::GetInstance()->GetRackRange();
    if (spRack == nullptr)
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("从配置库中获取架号范围配置失败。")));
        pTipDlg->exec();
        ULOG(LOG_ERROR, "Failed to get rack range.");
        return false;
    }
    bool isCali = (m_module == MODEL::CALI || m_module == MODEL::IMCALI);
    int lowLimit = isCali ? spRack->caliLowerLmt : spRack->qcLowerLmt;
    int uppuerLimit = isCali ? spRack->caliUpperLmt : spRack->qcUpperLmt;

    int rackint = rackNo.toInt();
    if (rackint < lowLimit || rackint > uppuerLimit)
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("保存失败：架号不在允许范围的范围。")));
        pTipDlg->exec();
        ULOG(LOG_ERROR, "rack is not in range, rack:%d", rackint);
        return false;
    }

    // 判断是否自动计算后会超过校准架范围
    int endNum = pos + m_CaliDocs.size() - m_Level;
    // 计算跨越了几个架子
    int overRackNum = (endNum / MAX_SLOT_NUM) + ((endNum % MAX_SLOT_NUM) > 0) + -1;

    if (overRackNum + rackint > uppuerLimit)
    {
        std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("保存失败：架号自动分配后，已超过允许范围的最大范围。")));
        pTipDlg->exec();
        ULOG(LOG_ERROR, "Cali endNum over the upperlimit");
        return false;
    }

    return true;
}

bool QPostionEdit::UpdateCaliCh(tf::TubeType::type tubetype, const QString& rackNo, int pos)
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);

    // 依次修改后面水平的校准品的位置了类型
    int rackTmp = rackNo.toInt();
    for (int i = m_Level - 1; i < m_CaliDocs.size(); i++)
    {
        if (pos > MAX_SLOT_NUM)
        {
            pos = 1;
            rackTmp = rackTmp + 1;
        }

        // 先判断当前位置架号位置和样本杯是否被修改（没有被修改就不用查位置重复了）
        if (!((m_CaliDocs[i].__isset.rack && m_CaliDocs[i].__isset.pos)
            && m_CaliDocs[i].rack == std::to_string(rackTmp)
            && m_CaliDocs[i].pos == pos))
        {
            // 存在重复的位置
            if (IsPositonBeUsing(std::to_string(rackTmp), pos))
            {
                ULOG(LOG_ERROR, "Cali ch rack and pos repeated.");
                return false;
            }
        }

        // 修改位置
        m_CaliDocs[i].__set_rack(std::to_string(rackTmp));
        m_CaliDocs[i].__set_pos(pos);
        // 修改杯类型
        m_CaliDocs[i].__set_tubeType(tubetype);
        pos++;
    }

    return true;
}

bool QPostionEdit::UpdateCaliIm(tf::TubeType::type tubetype, const QString& rackNo, int pos)
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);

    // 依次修改后面水平的校准品的位置了类型
    int rackTmp = rackNo.toInt();
    for (int i = m_Level - 1; i < m_IMCaliDocs.size(); i++)
    {
        if (pos > MAX_SLOT_NUM)
        {
            pos = 1;
            rackTmp = rackTmp + 1;
        }

        // 先判断当前位置架号位置和样本杯是否被修改（没有被修改就不用查位置重复了）
        if (!((m_IMCaliDocs[i].__isset.rack && m_IMCaliDocs[i].__isset.pos)
            && m_IMCaliDocs[i].rack == std::to_string(rackTmp)
            && m_IMCaliDocs[i].pos == pos))
        {
            // bug19502 后期连续赋值如果失败只是中断赋值，不取消前面的位置设置
            bool first = (i == m_Level - 1);
            // 存在重复的位置
            if (IsPositonBeUsing(std::to_string(rackTmp), pos, first))
            {
                ULOG(LOG_ERROR, "Cali im rack and pos repeated.");
                if (first)
                {
                    return false;
                }
                else
                {
                    break;
                }
            }
            // BUG19500 需求允许校准品反着放
        }

        // 修改位置
        m_IMCaliDocs[i].__set_rack(std::to_string(rackTmp));
        m_IMCaliDocs[i].__set_pos(pos);
        // 修改杯类型
        m_IMCaliDocs[i].__set_tubeType(tubetype);
        pos++;
    }

    return true;
}

bool QPostionEdit::UpdateQc(tf::TubeType::type tubetype, const QString& rackNo, int pos)
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);

    // 依次修改后面水平的校准品的位置了类型
    int rackTmp = rackNo.toInt();
    for (int i = 0; i < m_QcDocs.size(); i++)
    {
        if (pos > MAX_SLOT_NUM)
        {
            pos = 1;
            rackTmp = rackTmp + 1;
        }

        // 先判断当前位置架号位置和样本杯是否被修改（没有被修改就不用查位置重复了）
        if (!((m_QcDocs[i].__isset.rack && m_QcDocs[i].__isset.pos)
            && m_QcDocs[i].rack == std::to_string(rackTmp)
            && m_QcDocs[i].pos == pos))
        {
            // 存在重复的位置
            if (IsPositonBeUsing(std::to_string(rackTmp), pos))
            {
                ULOG(LOG_ERROR, "Qc rack and pos repeated.");
                return false;
            }
        }

        // 修改位置
        m_QcDocs[i].__set_rack(std::to_string(rackTmp));
        m_QcDocs[i].__set_pos(pos);
        // 修改杯类型
        m_QcDocs[i].__set_tubeType(tubetype);
        pos++;
    }

    return true;
}

///
/// @brief
///     确认按钮按下
///
///
/// @return 
///
/// @par History:
/// @li 5774/WuHongTao，2022年3月11日，新建函数
///
void QPostionEdit::OnSavePostion()
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);

    // 架号
    QString rackTmp = ui->rack_edit->text();
    // 位置
    int posTmp = -1;
    for (int i = 0; i < m_pushButtons.size(); ++i)
    {
        if (m_pushButtons[i]->isChecked())
        {
            posTmp = i + 1;
            break;
        }
    }
    // 获取界面样本管类型
    tf::TubeType::type currType = (ui->ori_Button->isChecked() ? tf::TubeType::TUBE_TYPE_ORIGIN : (\
        ui->normal_Button->isChecked() ? tf::TubeType::TUBE_TYPE_NORMAL : (\
            ui->micro_Button->isChecked() ? tf::TubeType::TUBE_TYPE_MICRO : (\
                ui->store_Button->isChecked() ? tf::TubeType::TUBE_TYPE_STORE : tf::TubeType::TUBE_TYPE_INVALID))));

    // 判断位置合法性
    if (!CheckPositionInvalid(currType, rackTmp, posTmp))
    {
        ULOG(LOG_ERROR, "Invalid input range.tubeType:%d, upLimit:%d, lowLimit:%d", (int)currType, rackTmp, posTmp);
        return;
    }

    if (m_module == MODEL::CALI && UpdateCaliCh(currType, rackTmp, posTmp))
    {
        emit ModifyCaliPostion();
        close();
    }
    else if (m_module == MODEL::IMCALI && UpdateCaliIm(currType, rackTmp, posTmp))
    {
        emit ModifyIMCaliPostion();
        close();
    }
    else if (m_module == MODEL::QC && UpdateQc(currType, rackTmp, posTmp))
    {
        emit ModifyQcPostion();
        close();
    }
}

///
/// @brief  位置是否被占用
///
/// @param[in]  strRack  架号
/// @param[in]  iPos   位置号
/// @param[in]  fl     是否是第一水平（直接报警）
///
/// @return 
///
/// @par History:
/// @li 1226/zhangjing，2022年12月13日，新建函数
///
bool QPostionEdit::IsPositonBeUsing(const std::string& strRack, int iPos, bool fl)
{
    ULOG(LOG_INFO, "%s", __FUNCTION__);

    bool bExist = false;
    if (m_module == MODEL::CALI || m_module == MODEL::IMCALI)
    {
        // 查询当前架号有没有被占用
        ::ch::tf::CaliDocQueryResp ResultDocs;
        ::ch::tf::CaliDocQueryCond QueryCondition;
        QueryCondition.__set_rack(strRack);
        QueryCondition.__set_pos(iPos);

        // 通讯异常和架号重复，直接返回
        if (!ch::LogicControlProxy::QueryCaliDoc(ResultDocs, QueryCondition)
            || ResultDocs.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("查询生化校准数据库异常")));
            pTipDlg->exec();
            bExist = true;
            ULOG(LOG_ERROR, "Failed to query ch cali doc.");
            return bExist;
        }
        else if (!ResultDocs.lstCaliDocs.empty())
        {
            int64_t rackInUseGroupId = ResultDocs.lstCaliDocs[0].caliDocGroupId;         //架位被占用的校准品组ID
            int64_t curGroupId = m_CaliDocs.empty() ? 0 : m_CaliDocs[0].caliDocGroupId;  //当前校准品组ID
            int level = ResultDocs.lstCaliDocs[0].level;                                 //架位被占用的校准文档水平

            bExist = (rackInUseGroupId != curGroupId /*文档不是当前文档，架位被占用*/
                 /*是当前文档，但是被起始水平之前的水平占用了（因为我们是从起始水平向后设置架位）
                 例如：当前从水平3开始设置架位，但是水平1或2占了架位*/
                || m_Level > level); 

            if (bExist)
            {
                if (fl)
                {
                    std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("架号/位置已被占用")));
                    pTipDlg->exec();
                }

                ULOG(LOG_ERROR, "ch cali position already exist.");
                return bExist;
            }
        }

        ::im::tf::CaliDocQueryResp ImResultDocs;
        ::im::tf::CaliDocQueryCond ImQueryCondition;
        ImQueryCondition.__set_rack(strRack);
        ImQueryCondition.__set_pos(iPos);

        // 通讯异常和架号重复，直接返回
        if (!im::LogicControlProxy::QueryCaliDoc(ImResultDocs, ImQueryCondition)
            || ImResultDocs.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("查询免疫校准数据库异常")));
            pTipDlg->exec();
            bExist = true;
            ULOG(LOG_ERROR, "Failed to query im cali doc.");
            return bExist;
        }
        else if (!ImResultDocs.lstCaliDocs.empty())
        {
            // 默认为true，设为false则不报警，bug19504
            if (fl)
            {
                std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("架号/位置已被占用")));
                pTipDlg->exec();
            }
            bExist = true;
            ULOG(LOG_ERROR, "Im cali position already exist.");
            return bExist;
        }
    }
    // 质控判断
    else
    {
        tf::QcDocQueryCond qcCond;
        qcCond.__set_rack(strRack);
        qcCond.__set_pos(iPos);

        tf::QcDocQueryResp resp;
        if (!DcsControlProxy::GetInstance()->QueryQcDoc(resp, qcCond) || \
            resp.result != tf::ThriftResult::THRIFT_RESULT_SUCCESS)
        {
            std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("质控品查询失败")));
            pTipDlg->exec();
            ULOG(LOG_ERROR, "Failed to query qcDoc.");
            return true;
        }
        if (!resp.lstQcDocs.empty())
        {
            if (fl)
            {
                std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("质控的架号/位置已被占用")));
                pTipDlg->exec();
            }
            bExist = true;
            ULOG(LOG_ERROR, "Qc position already exist.");
            return bExist;
        }
    }
    return bExist;
}

///
/// @brief  架号变化
///
/// @param[in]  rack  新架号
///
/// @par History:
/// @li 1226/zhangjing，2023年9月7日，新建函数
///
void QPostionEdit::OnRackChanged(QString rack)
{
    if (!rack.isEmpty() && rack.length() < 4)
    {
        return;
    }
    ResetPosAvable(rack.toStdString());
}
