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
/// @file     QHistoryFilterSample.h
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
#include "QHistoryFilterSample.h"
#include "ui_QHistoryFilterSample.h"
#include <QGridLayout>
#include "shared/CommonInformationManager.h"
#include "shared/CReadOnlyDelegate.h"
#include "shared/SwitchButton.h"
#include "shared/tipdlg.h"
#include "thrift/DcsControlProxy.h"
#include "QHistoryFilterSampleImp.h"
#include "src/public/DictionaryKeyName.h"
#include "src/common/Mlog/mlog.h"

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <QKeyEvent>


#define FAST_FILTER_COUNT (5)	// 快捷查询界面个数

// 组装逗号分隔的字符串
std::string ComposeStr(const std::vector<std::string>& vecStr)
{
    std::string s;
    for (const std::string& d : vecStr)
        s += d + ",";

    if (!s.empty())
        s.pop_back();

    return s;
};

template<typename T>
std::string ComposeInt(const std::vector<T>& vecInt)
{
    std::string s;
    for (T d : vecInt)
        s += std::to_string(d) + ",";

    if (!s.empty())
        s.pop_back();

    return s;
}

template<typename T>
std::vector<T> ToIntVec(const std::string& strInput)
{
    std::vector<T> vecInt;

    std::vector<std::string> val;
    boost::split(val, strInput, boost::is_any_of(","));
    for (const std::string& d : val)
    {
        if (d.empty())
            continue;

        vecInt.push_back((T)std::stoi(d));
    }

    return vecInt;
}


// 历史数据查询条件转换xml
bool HistoryFilterToXML(std::vector<::tf::HistoryBaseDataQueryCond>& vecQryCond, std::string &xml)
{
    xml.clear();

    try
    {
        boost::property_tree::ptree pt;
        pt.put("root", "");
        for (const auto& qryCond : vecQryCond)
        {
            auto& filter = pt.add("root.historyFilter", "");
            filter.put("<xmlattr>.used", qryCond.isUsed);
            filter.put("<xmlattr>.index", qryCond.filterIndex);
            filter.put("<xmlattr>.name", qryCond.filterName);
            filter.put("<xmlattr>.startTime", qryCond.startTime);
            filter.put("<xmlattr>.endtime", qryCond.endtime);

            filter.put("<xmlattr>.seqNoList", ComposeStr(qryCond.seqNoList));
            filter.put("<xmlattr>.sampleBarcode", ComposeStr(qryCond.sampleBarcodeList));
            filter.put("<xmlattr>.moduleList", ComposeStr(qryCond.moduleList));

            filter.put("<xmlattr>.caseNo", qryCond.caseNo);
            filter.put("<xmlattr>.patientName", qryCond.patientName);
            filter.put("<xmlattr>.reagentLot", qryCond.reagentLot);

            filter.put("<xmlattr>.assayCodeList", ComposeInt(qryCond.assayCodeList));
            filter.put("<xmlattr>.sampleTypeList", ComposeInt(qryCond.sampleTypeList));
            filter.put("<xmlattr>.sampleSourceTypeList", ComposeInt(qryCond.sampleSourceTypeList));
            filter.put("<xmlattr>.retestList", ComposeInt(qryCond.retestList));
            filter.put("<xmlattr>.warnList", ComposeInt(qryCond.warnList));
            filter.put("<xmlattr>.checkList", ComposeInt(qryCond.checkList));
            filter.put("<xmlattr>.diluteStatuList", ComposeInt(qryCond.diluteStatuList));
            filter.put("<xmlattr>.qualitativeResultList", ComposeInt(qryCond.qualitativeResultList));
            filter.put("<xmlattr>.sendLISList", ComposeInt(qryCond.sendLISList));
            filter.put("<xmlattr>.printList", ComposeInt(qryCond.printList));
        }
        std::ostringstream os;
        boost::property_tree::write_xml(os, pt, boost::property_tree::xml_writer_make_settings<std::string>(' ', 4));
        xml = os.str();
    }
    catch (const std::exception&)
    {
        return false;
    }
    return true;
}
bool HistoryXMLToFilter(std::vector<::tf::HistoryBaseDataQueryCond>& vecQryCond, const std::string &xml)
{
	if (xml.empty())
	{
		return false;
	}

    try
    {
        boost::property_tree::ptree pt;
        std::istringstream is(xml);
        boost::property_tree::read_xml(is, pt);
        auto root = pt.get_child("root");
        BOOST_FOREACH(auto& item, root)
        {
            if (item.first == "historyFilter")
            {
                const auto &filter = item.second;
                ::tf::HistoryBaseDataQueryCond qryCond;
                qryCond.__set_isUsed(filter.get<bool>("<xmlattr>.used"));
                qryCond.__set_filterIndex(filter.get<int>("<xmlattr>.index"));
                qryCond.__set_filterName(filter.get<std::string>("<xmlattr>.name"));
                qryCond.__set_startTime(filter.get<std::string>("<xmlattr>.startTime"));
                qryCond.__set_reTestStartTime(qryCond.startTime);
                qryCond.__set_endtime(filter.get<std::string>("<xmlattr>.endtime"));
                qryCond.__set_reTestEndtime(qryCond.endtime);
                {
                    std::vector<std::string> val;
                    auto input = filter.get<std::string>("<xmlattr>.seqNoList");
                    boost::split(val, input, boost::is_any_of(","));
                    if (!(val.empty() || (val[0].empty())))
                    {
                        qryCond.__set_seqNoList(val);
                    }
                }
                {
                    std::vector<std::string> val;
                    auto input = filter.get<std::string>("<xmlattr>.sampleBarcode");
                    boost::split(val, input, boost::is_any_of(","));
                    if (!(val.empty() || (val[0].empty())))
                    {
                        qryCond.__set_sampleBarcodeList(val);
                    }
                }
                std::string str;
                {
                    std::vector<std::string> val;
                    auto input = filter.get<std::string>("<xmlattr>.moduleList");
                    boost::split(val, input, boost::is_any_of(","));
                    if (!(val.empty() || (val[0].empty())))
                    {
                        qryCond.__set_moduleList(val);
                    }
                }
                str = filter.get<std::string>("<xmlattr>.caseNo");
                if (!str.empty())
                {
                    qryCond.__set_caseNo(str);
                }
                str = filter.get<std::string>("<xmlattr>.patientName");
                if (!str.empty())
                {
                    qryCond.__set_patientName(str);
                }
				str = filter.get<std::string>("<xmlattr>.reagentLot");
				if (!str.empty())
				{
					qryCond.__set_reagentLot(str);
				}
                {
                    std::string input = filter.get<std::string>("<xmlattr>.assayCodeList");
                    std::vector<int> val_int = ToIntVec<int>(input);
                    if (!val_int.empty())
                    {
                        qryCond.__set_assayCodeList(val_int);
                    }
                }
                {
                    auto input = filter.get<std::string>("<xmlattr>.sampleTypeList");
                    std::vector<::tf::HisSampleType::type> val_int = ToIntVec<::tf::HisSampleType::type>(input);
                    if (!val_int.empty())
                    {
                        qryCond.__set_sampleTypeList(val_int);
                    }
                }
                {
                    auto input = filter.get<std::string>("<xmlattr>.sampleSourceTypeList");
                    std::vector<::tf::SampleSourceType::type> val_int = ToIntVec<::tf::SampleSourceType::type>(input);
                    if (!val_int.empty())
                    {
                        qryCond.__set_sampleSourceTypeList(val_int);
                    }
                }
                {
                    auto input = filter.get<std::string>("<xmlattr>.retestList");
                    std::vector<int> val_int = ToIntVec<int>(input);
                    if (!val_int.empty())
                    {
                        qryCond.__set_retestList(val_int);
                    }
                }
                {
                    auto input = filter.get<std::string>("<xmlattr>.warnList");
                    std::vector<int> val_int = ToIntVec<int>(input);
                    if (!val_int.empty())
                    {
                        qryCond.__set_warnList(val_int);
                    }
                }
                {
                    auto input = filter.get<std::string>("<xmlattr>.checkList");
                    std::vector<int> val_int = ToIntVec<int>(input);
                    if (!val_int.empty())
                    {
                        qryCond.__set_checkList(val_int);
                    }
                }
                {
                    auto input = filter.get<std::string>("<xmlattr>.diluteStatuList");
                    std::vector<::tf::HisDataDiluteStatu::type> val_int = ToIntVec<::tf::HisDataDiluteStatu::type>(input);
                    if (!val_int.empty())
                    {
                        qryCond.__set_diluteStatuList(val_int);
                    }
                }
                {
                    auto input = filter.get<std::string>("<xmlattr>.qualitativeResultList");
                    std::vector<int> val_int = ToIntVec<int>(input);
                    if (!val_int.empty())
                    {
                        qryCond.__set_qualitativeResultList(val_int);
                    }
                }
                {
                    auto input = filter.get<std::string>("<xmlattr>.sendLISList");
                    std::vector<int> val_int = ToIntVec<int>(input);
                    if (!val_int.empty())
                    {
                        qryCond.__set_sendLISList(val_int);
                    }
                }
                {
                    auto input = filter.get<std::string>("<xmlattr>.printList");
                    std::vector<int> val_int = ToIntVec<int>(input);
                    if (!val_int.empty())
                    {
                        qryCond.__set_printList(val_int);
                    }
                }
                vecQryCond.push_back(qryCond);
            }
        }
    }
    catch (const std::exception& e)
    {
        ULOG(LOG_WARN, "Failed to parase history filter consition xml, %s", e.what());
        return false;
    }
    return true;
}


QHistoryFilterSample::QHistoryFilterSample(QWidget *parent)
    : BaseDlg(parent)
    , m_filterImp(nullptr)
    , m_fastFilterTab(nullptr)
{
    ui = new Ui::QHistoryFilterSample();
    ui->setupUi(this);

    m_prePageIdx = 0;
    Init();
}

QHistoryFilterSample::~QHistoryFilterSample()
{
}

///
/// @brief 初始化
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月31日，新建函数
///
void QHistoryFilterSample::Init()
{
    m_totalPages.clear();
    qRegisterMetaType<::tf::HistoryBaseDataQueryCond>("::tf::HistoryBaseDataQueryCond");
    // 初始化标题
    SetTitleName(tr("筛选"));

    if (m_fastFilterTab == nullptr)
    {
        m_fastFilterTab = new CustomTabWidget();
    }

    auto layout = new QGridLayout();
    layout->setContentsMargins(11, 10, 0, 0);

    // 设置样式表属性
    m_fastFilterTab->setProperty("tabs", QVariant(QApplication::translate("QHistoryFilterSampleImp", "west", Q_NULLPTR)));
    m_fastFilterTab->setTabPosition(QTabWidget::West);
    layout->addWidget(m_fastFilterTab);

    // 添加筛选页签
    if (m_filterImp == nullptr)
    {
        m_filterImp = new QHistoryFilterSampleImp(this, 0);
        m_totalPages.push_back(m_filterImp);
        m_fastFilterTab->addTab(m_filterImp, QString(tr("筛选")));
    }
    for (int i = 1; i <= FAST_FILTER_COUNT; ++i)
    {
        auto fastfilter = new QHistoryFilterSampleImp(this, i);
        m_totalPages.push_back(fastfilter);
        m_fastFilterTab->addTab(fastfilter, QString(tr("快捷筛选设置%1")).arg(i));
		// 连接快捷筛选保存时的槽函数
		connect(fastfilter, &QHistoryFilterSampleImp::SignalSaveFastFilter, this,&QHistoryFilterSample::OnSaveFastFilter);
    }
    ui->widget1->setLayout(layout);

    connect(ui->cancel_btn, &QPushButton::clicked, this, [&]() {
		m_filterImp->SetQueryConditon(m_lastFilterCond);
        this->close(); });
    // 确定
    connect(ui->ok_btn, &QPushButton::clicked, this, &QHistoryFilterSample::OnSaveParameter);
	// 保存
	connect(ui->save_btn, &QPushButton::clicked, this, &QHistoryFilterSample::OnSaveFastFilter);
    // 重置
    connect(ui->reste_btn, &QPushButton::clicked, this, &QHistoryFilterSample::OnReset);
    // 切换页签
    connect(m_fastFilterTab, &CustomTabWidget::currentChanged, this, &QHistoryFilterSample::OnCurrentChanged);

    InitCond();
}

/// @brief 
///
/// @param[in]  qryCond  
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年3月14日，新建函数
///
bool QHistoryFilterSample::GetQueryConditon(::tf::HistoryBaseDataQueryCond& qryCond)
{
	auto fastfilter = qobject_cast<QHistoryFilterSampleImp*>(m_fastFilterTab->currentWidget());
	if (fastfilter == nullptr)
	{
		return false;
	}
	fastfilter->GetQueryConditon(qryCond);

    return true;
}

///
/// @brief 获取快捷筛选条件
///
/// @param[in]  vecQryCond  快捷筛选条件
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年4月14日，新建函数
///
bool QHistoryFilterSample::GetFastFilterQueryConditon(std::vector<::tf::HistoryBaseDataQueryCond>& vecQryCond)
{
    vecQryCond = m_vecFastFilterCond;
    return true;
}

///
/// @brief 更新工作页面的按钮状态
///
///
/// @par History:
/// @li 7702/WangZhongXin，2023年9月5日，新建函数
///
void QHistoryFilterSample::UpdateButtonStatus(const PageSet& workSet)
{
    if (m_filterImp != nullptr)
    {
        m_filterImp->UpdateButtonStatus(workSet);
    }

    if (m_fastFilterTab == nullptr)
    {
        return;
    }
    for (int index = 0; index < m_fastFilterTab->count(); ++index)
    {
        auto widget = qobject_cast<QHistoryFilterSampleImp*>(m_fastFilterTab->widget(index));
        if (widget == nullptr)
        {
            return;
        }
        widget->UpdateButtonStatus(workSet);
    }
}

///
/// @brief 保存筛选参数
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月31日，新建函数
///
void QHistoryFilterSample::OnSaveParameter()
{
    if (m_filterImp->isVisible())
    {
        if (!m_filterImp->VerifyParameter())
        {
            return;
        }
		else
		{
			m_filterImp->GetQueryConditon(m_lastFilterCond);
		}
    }
    else
    {
		if (!OnSaveFastFilter())
		{
			return;
		}
    }

	emit finished();

    close();
}

bool QHistoryFilterSample::SavePageParameter(QHistoryFilterSampleImp* targetPage, ::tf::HistoryBaseDataQueryCond& qryCond)
{
    if (!targetPage->VerifyParameter())
    {
        ULOG(LOG_WARN, "Failed to verifyparameter.");
        return false;
    }

    // 验证筛选页名称的重复性
    QString strCurName = targetPage->GetFilterName();
    for (const QHistoryFilterSampleImp* tmpPage : m_totalPages)
    {
        if (targetPage == tmpPage)
        {
            continue;
        }
        if (strCurName == tmpPage->GetFilterName())
        {
            TipDlg(tr("提示"), tr("名字有重复，不能保存")).exec();
            return false;
        }
    }

    int index = targetPage->GetCurrentIndex();
    targetPage->GetQueryConditon(qryCond);
    // wuhongtao modify for bug2757
    //auto txt = qryCond.filterName.empty() ? QString(tr("快捷筛选%1")).arg(index + 1) : QString::fromUtf8(qryCond.filterName.c_str());
    //m_fastFilterTab->tabBar()->setTabText(index, txt);
    qryCond.__set_filterIndex(index);

    auto it = std::find_if(m_vecFastFilterCond.begin(), m_vecFastFilterCond.end(), [&](::tf::HistoryBaseDataQueryCond& v) {
        return (v.filterIndex == index);
    });
    if (it == m_vecFastFilterCond.end())
    {
        m_vecFastFilterCond.push_back(qryCond);
    }
    else
    {
        *it = qryCond;
    }

    // 把快捷查询条件转换为xml存放在数据字典
    std::string xml;
    HistoryFilterToXML(m_vecFastFilterCond, xml);
    tf::DictionaryInfo di;
    di.__set_keyName(DKN_UI_HISTORY_FILTER_COND);
    di.__set_value(xml);
    // 修改数据字典信息
    if (!DcsControlProxy::GetInstance()->ModifyDictionaryInfo(di))
    {
        ULOG(LOG_WARN, "Failed to modify historyfiltercond dictionaryinfo.");
        return false;
    }
    // 刷新界面缓存的cond
    targetPage->SetQueryConditon(qryCond);

    return true;
}


///
/// @brief 保存快捷筛选条件
///
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2024年1月5日，新建函数
///
bool QHistoryFilterSample::OnSaveFastFilter()
{
    auto fastfilter = qobject_cast<QHistoryFilterSampleImp*>(m_fastFilterTab->currentWidget());
    if (fastfilter == nullptr)
    {
	    return false;
    }

    ::tf::HistoryBaseDataQueryCond qryCond;
    if (!SavePageParameter(fastfilter, qryCond))
    {
        ULOG(LOG_WARN, "Failed to save pageparamter.");
        return false;
    }

    emit fastFilterFinished(qryCond);

	// 0027302: [工作] 历史数据界面设置快捷筛选条件确定后，软件多余提示“保存成功”
	// 1.无提示“保存成功”，与数据浏览界面处理一致 modify by Chenjianlin 20240509
    // std::shared_ptr<QCustomDialog> spDialog = std::make_shared<QCustomDialog>(Q_NULLPTR, 1000, tr("保存成功"));
	// spDialog->exec();

	return true;
}

///
/// @brief 重置按钮槽函数
///
///
/// @return 
///
/// @par History:
/// @li 7702/WangZhongXin，2023年3月14日，新建函数
///
void QHistoryFilterSample::OnReset()
{
    // 筛选界面显示时才重置
    if (m_filterImp->isVisible())
    {
        m_filterImp->OnReset();
    }
    // 快捷筛选界面重置
    else if (m_fastFilterTab->isVisible())
    {
        auto filter = qobject_cast<QHistoryFilterSampleImp*>(m_fastFilterTab->currentWidget());
        if (filter == nullptr)
        {
            return;
        }

        filter->OnReset();
    }
}

void QHistoryFilterSample::OnCurrentChanged(int index)
{
    if (m_prePageIdx < 0 || m_prePageIdx >= m_totalPages.size())
    {
        return;
    }

	if (index == 0)
	{
		ui->save_btn->setEnabled(false);
	}
	else
	{
		ui->save_btn->setEnabled(true);
	}

	// 只有当页面从快捷页面跳转到筛选页面或者其他页面才需要询问，从筛选页面跳转到快捷页面不需要询问
	if (m_prePageIdx != 0)
	{

		if (m_totalPages[m_prePageIdx]->IsWannaSaveChanged())
		{
			::tf::HistoryBaseDataQueryCond qryCond;
			if (!SavePageParameter(m_totalPages[m_prePageIdx], qryCond))
			{
				ULOG(LOG_WARN, "Failed to save pageparameter.");
				// 此处联动处理不完善，暂时注释掉，没有成功也切换到选择的页面
#if 0
			// 没有保存成功则回退到 待保存的页面
				m_fastFilterTab->tabBar()->setCurrentIndex(m_prePageIdx);
				return;
#endif
			}
            emit fastFilterFinished(qryCond);
		}

	}

	m_totalPages[m_prePageIdx]->ReloadCondition();
	m_prePageIdx = index;
}

void QHistoryFilterSample::InitCond()
{
    // 初始化快捷筛选
	// 索引0为常规筛选
	for (int index = 1; index < m_fastFilterTab->count(); ++index)
	{
		auto widget = qobject_cast<QHistoryFilterSampleImp*>(m_fastFilterTab->widget(index));
		if (widget == nullptr)
		{
			return;
		}
		widget->OnReset();
	}

    // 构造查询条件
    ::tf::DictionaryInfoQueryCond   qryCond;
    ::tf::DictionaryInfoQueryResp   qryResp;
    qryCond.__set_keyName(DKN_UI_HISTORY_FILTER_COND);

    // 查询
    if (!DcsControlProxy::GetInstance()->QueryDictionaryInfo(qryResp, qryCond)
        || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS
        || qryResp.lstDictionaryInfos.empty())
    {
        return;
    }
    std::vector<::tf::HistoryBaseDataQueryCond> historyCond;
    auto bRet = HistoryXMLToFilter(historyCond, qryResp.lstDictionaryInfos[0].value);
    if (!bRet)
    {
        return;
    }
    m_vecFastFilterCond = historyCond;
    for (const ::tf::HistoryBaseDataQueryCond& cond : historyCond)
    {
        if (m_fastFilterTab->count() <= cond.filterIndex)
        {
            continue;
        }
        auto filter = qobject_cast<QHistoryFilterSampleImp*>(m_fastFilterTab->widget(cond.filterIndex));
        if (filter == nullptr)
        {
            continue;
        }

        // wuhongtao modify for bug2757
        //cond.filterName.empty() ? 0 : m_fastFilterTab->tabBar()->setTabText(cond.filterIndex, QString::fromStdString(cond.filterName));

        filter->SetQueryConditon(cond);
    }
    emit fastFilterFinished(::tf::HistoryBaseDataQueryCond());
}

void QHistoryFilterSample::showEvent(QShowEvent *event)
{
    InitCond();

	int index = m_fastFilterTab->currentIndex();
	if (index == 0)
	{
		ui->save_btn->setEnabled(false);
	}
	else
	{
		ui->save_btn->setEnabled(true);
	}

    // 显示时把筛选界面设置为当前窗口
    //ui->tabWidget->setCurrentWidget(m_filterImp);

    BaseDlg::showEvent(event);
}


void QHistoryFilterSample::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
    {
        return;
    }
}