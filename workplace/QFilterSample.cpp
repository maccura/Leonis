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
#include "QFilterSample.h"
#include "ui_QFilterSample.h"
#include <QCheckBox>
#include <QDateTime>
#include <set>
#include "src/common/StringUtil.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/TimeUtil.h"
#include "src/common/common.h"
#include "QSampleAssayModel.h"
#include <boost/multiprecision/cpp_int.hpp>
#include "boost/algorithm/string.hpp"
#include <algorithm>
#include "src/public/SerializeUtil.hpp"
#include "src/public/DictionaryKeyName.h"
#include "utility/QGroupAssayPositionSettings.h"
#include "thrift/DcsControlProxy.h"
#include "shared/CommonInformationManager.h"
#include "shared/tipdlg.h"
#include "shared/SwitchButton.h"
#include "shared/uidef.h"
#include "shared/datetimefmttool.h"
#include "shared/messagebus.h"
#include "shared/msgiddef.h"
#include "shared/ThriftEnumTransform.h"
#include "manager/DictionaryQueryManager.h"
#include "WorkpageCommon.h"
#include <QKeyEvent>

#define PAGEMAX 5
#define MAXLENGTH 25

// 序列化 ConditionDoing
template<typename condtion, typename coreData, typename conPara>
void to_json(nlohmann::json& j, const ConditionDoing<condtion, coreData, conPara>& cd) {
	j["condition"] = cd.m_condition;
	j["condNumber"] = cd.m_condNumber;
	j["condContent"] = cd.m_condContent;
}

// 反序列化 ConditionDoing
template<typename condtion, typename coreData, typename conPara>
void from_json(const nlohmann::json& j, ConditionDoing<condtion, coreData, conPara>& cd)
{
	j.at("condition").get_to(cd.m_condition);
	j.at("condNumber").get_to(cd.m_condNumber);
	j.at("condContent").get_to(cd.m_condContent);
}

// 序列化&反序列化 FilterConDition
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(FilterConDition,
	oderType,
	status,
	sampleType,
	auditStatus,
	reCheckStatus,
	printStatus,
	alarmStatus,
	dulitionType,
	reagentLots,
	sendLis,
	aiCheck,
	examinationTime,
	sampleRange,
	barcode,
	device,
	medicalCase,
	patientName,
	sampleAssays,
	testItemAssay,
	enable,
	name);

QFilterSample::QFilterSample(QWidget *parent)
	: BaseDlg(parent)
    , m_currentFastPage(boost::none)
    , m_currentSetPage(boost::none)
    , m_preSetPage(boost::none)
    , m_assaySelectDialog(Q_NULLPTR)
{
    ui = new Ui::QFilterSample();
	ui->setupUi(this);
	Init();
}

QFilterSample::~QFilterSample()
{
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
bool QFilterSample::SetCurrentFastCond(boost::optional<int> index)
{
	m_currentSetPage = index;
    if (!m_currentSetPage)
    {
        return true;
    }

    if (index.value() <= 0 || index.value() > PAGEMAX)
    {
        return false;
    }

    // 设置完毕以后通知页面更新筛选条件
    emit finished();
    return true;
}

///
/// @brief 初始化
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月31日，新建函数
///
void QFilterSample::Init()
{
	// 初始化标题
	SetTitleName(tr("筛选"));
	// 初始化显示页面的开始
	ui->tabWidget->setCurrentIndex(0);
	m_currentSetPage = 0;
	// 分配属性
	m_typeButtons.resize(6, TypeButton());
	// 设备类型
	// 非免疫设备需要隐藏
	if (CommonInformationManager::GetInstance()->GetSoftWareType() == CHEMISTRY)
	{
		ui->pushButton_81->hide();
		ui->pushButton_82->hide();
		ui->pushButton_56->hide();
		ui->pushButton_57->hide();
		ui->pushButton_106->hide();
		ui->pushButton_107->hide();
		ui->pushButton_131->hide();
		ui->pushButton_132->hide();
		ui->pushButton_156->hide();
		ui->pushButton_157->hide();

		ui->label_108->hide();
		ui->label_88->hide();
		ui->label_68->hide();
		ui->label_48->hide();
		ui->label_28->hide();

		ui->pushButton_resultPositive->hide();
		ui->pushButton_resultNegtive->hide();
		ui->label_19->hide();
	}

	// 工作页面的设置更新
	REGISTER_HANDLER(MSG_ID_WORK_PAGE_SET_DISPLAY_UPDATE, this, OnPageSetChanged);
	// 初始化所有的lineEdit
	QList<QLineEdit*> lineEditGroup = findChildren<QLineEdit*>(QString(), Qt::FindDirectChildrenOnly);
	std::for_each(lineEditGroup.begin(), lineEditGroup.end(), [](QLineEdit* lineEdit) {lineEdit->setEnabled(false); });
	connect(ui->cancel_btn, &QPushButton::clicked, this, [&]() 
    {
        // 当前筛选页面需要重置一下
        auto index = ui->tabWidget->currentIndex();
        if (index == 0)
        {
            //m_realCond.Reset();
            //InitPageCurrent();
        }

        //this->InitPageCurrent();
        this->InItPageAttribute();
        this->close(); 
    });

	// 项目选择对话框
    connect(ui->assay_select_btn, &QPushButton::clicked, this, [&]()
    {
        this->OnAssaySelect(); 
    });

	// 确定
    connect(ui->ok_btn, &QPushButton::clicked, this, [&]() 
    {
        if (ExecuteFilterCond(false))
        {
			// 执行筛选
			emit finished();
            close();
        }
    });

    // 重置页面
    connect(ui->reste_btn, &QPushButton::clicked, this, [&]()
    {
        auto index = ui->tabWidget->currentIndex();
		// 筛选页面
        if (index == 0)
        {
            m_realCond.Reset();
            InitPageCurrent();
        }
		// 快捷筛选页面（1-6）
        else if(index > 0)
        {
			// 快捷页面
            auto currentIndex = index - 1;
            if (currentIndex < m_filterConditions.size())
            {
                m_filterConditions[currentIndex].Reset();
                m_filterConditions[currentIndex].enable = false;
                InItPageAttribute(true);
            }
        }
    });

	// 保存数据
	{
		connect(ui->save_btn, &QPushButton::clicked, this, [=]()
		{
			std::string xml;
			if (!Encode2Json(xml, m_filterConditions))
			{
				return;
			}

			if (m_conditionXml != xml)
			{
				// 执行保存
				if (!ExecuteFilterCond(true))
				{
					ULOG(LOG_WARN, "Failed to save filter-cond!");
					// 重新初始化页面
					InitPageCurrent();
					InItPageAttribute();
					return;
				}

				// modify bug0011226
				std::shared_ptr<QCustomDialog> spDialog = std::make_shared<QCustomDialog>(Q_NULLPTR, 1000, tr("保存成功"));
				spDialog->exec();
			}
		});
	}

    // 页面切换
    connect(ui->tabWidget, &QTabWidget::currentChanged, this, [&](int index)
    {
        // 从快捷配置切换到当前筛选页面
        if (index == 0)
        {
			ui->save_btn->setEnabled(false);
            std::string xml;
            if (!Encode2Json(xml, m_filterConditions))
            {
				m_currentSetPage = index;
                return;
            }

            if (m_conditionXml != xml)
            {
                std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("是否保存当前设置？"), TipDlgType::TWO_BUTTON));
                if (pTipDlg->exec() == QDialog::Rejected)
                {
                    ULOG(LOG_INFO, "Cancel save!");

                    // 重新初始化页面
                    InitPageCurrent();
                    InItPageAttribute();
					m_currentSetPage = index;
                    return;
                }

                // 执行保存
                if (!ExecuteFilterCond(true))
                {
                    InitPageCurrent();
                    InItPageAttribute();
					m_currentSetPage = index;
                    return;
                }
            }
        }
		// 若是快捷界面之间的切换，若有改动需要提示是否保存
		// m_currentSetPage,保存的是当前页面，若m_currentSetPage不等于0，说明从其他页面切换过来的
        else if(m_currentSetPage != 0)
        {
			ui->save_btn->setEnabled(true);
			std::string xml;
			if (!Encode2Json(xml, m_filterConditions))
			{
				m_currentSetPage = index;
				return;
			}

			if (m_conditionXml != xml)
			{
				std::shared_ptr<TipDlg> pTipDlg(new TipDlg(tr("是否保存当前设置？"), TipDlgType::TWO_BUTTON));
				if (pTipDlg->exec() == QDialog::Rejected)
				{
					ULOG(LOG_INFO, "Cancel save!");
					// 重新初始化页面
					InitPageCurrent();
					InItPageAttribute();
					m_currentSetPage = index;
					return;
				}

				// 执行保存
				if (!ExecuteFilterCond(true))
				{
					ULOG(LOG_INFO, "save failed!");
					// 重新初始化页面
					InitPageCurrent();
					InItPageAttribute();
				}
			}
			else
			{
				InitPageCurrent();
				InItPageAttribute();
			}
        }
		// 恢复筛选页面的情况
		else if (m_currentSetPage == 0)
		{
			ui->save_btn->setEnabled(true);
			m_realCond.Reset();
			InitPageCurrent();
			InItPageAttribute();
		}

		m_currentSetPage = index;
    });

    // 初始化筛选页面的关系映射
    InitPageCurrent();
    InItPageAttribute();
	// 根据显示设置和状态信息来显示按钮
	OnPageSetChanged();
	// 初始化，避免为空的时候
	if (m_conditionXml.empty())
	{
		Encode2Json(m_conditionXml, m_filterConditions);
	}
}

void QFilterSample::InItPageAttribute(bool reset)
{
    // 当不是reset的时候才从新导入
    if (!reset)
    {
        // 快捷方式有个5个
		m_filterConditions.clear();
        m_filterConditions.resize(PAGEMAX);
        // 导入快捷筛选设置
        LoadCond();
    }

    // 订单类型
    std::vector<std::vector<QPushButton*>> orderPageButton;
    {
        // 筛选1
        {
            std::vector<QPushButton*> oderButtons;
            oderButtons.push_back(ui->pushButton_44);
            oderButtons.push_back(ui->pushButton_45);
            oderButtons.push_back(ui->pushButton_46);
            oderButtons.push_back(ui->pushButton_47);
            orderPageButton.push_back(oderButtons);
        }

        // 筛选2
        {
            std::vector<QPushButton*> oderButtons;
            oderButtons.push_back(ui->pushButton_69);
            oderButtons.push_back(ui->pushButton_70);
            oderButtons.push_back(ui->pushButton_71);
            oderButtons.push_back(ui->pushButton_72);
            orderPageButton.push_back(oderButtons);
        }

        {
            std::vector<QPushButton*> oderButtons;
            oderButtons.push_back(ui->pushButton_94);
            oderButtons.push_back(ui->pushButton_95);
            oderButtons.push_back(ui->pushButton_96);
            oderButtons.push_back(ui->pushButton_97);
            orderPageButton.push_back(oderButtons);
        }

        {
            std::vector<QPushButton*> oderButtons;
            oderButtons.push_back(ui->pushButton_119);
            oderButtons.push_back(ui->pushButton_120);
            oderButtons.push_back(ui->pushButton_121);
            oderButtons.push_back(ui->pushButton_122);
            orderPageButton.push_back(oderButtons);
        }

        {
            std::vector<QPushButton*> oderButtons;
            oderButtons.push_back(ui->pushButton_144);
            oderButtons.push_back(ui->pushButton_145);
            oderButtons.push_back(ui->pushButton_146);
            oderButtons.push_back(ui->pushButton_147);
            orderPageButton.push_back(oderButtons);
        }

    }

    int currentIndex = 0;
    for (const auto& oderButtons : orderPageButton)
    {
        int postion = 0;
        for (auto& button : oderButtons)
        {
			disconnect(button);
            m_filterConditions[currentIndex].oderType.m_condition.test(postion) ? button->setChecked(true)
                : button->setChecked(false);

            QVariant pos(postion);
            button->setProperty("postion", pos);
            connect(button, &QPushButton::clicked, this, [=]()
            {
                int pos = button->property("postion").toInt();
                bool status = button->isChecked();
                m_filterConditions[currentIndex].oderType.OnChangeCondition(std::make_pair(pos, status));
            });

            postion++;
        }

		std::vector<std::string> condNames = { QObject::tr("订单类型：").toStdString(), QObject::tr("常规").toStdString(), QObject::tr("急诊").toStdString(),QObject::tr("校准").toStdString(),QObject::tr("质控").toStdString() };
		m_filterConditions[currentIndex].oderType.SetContentName(condNames);
        currentIndex++;
        if (currentIndex >= PAGEMAX)
        {
            break;
        }
    }

    // 状态
    std::vector<std::vector<QPushButton*>> statusPageButton;
    {
        {
            std::vector<QPushButton*> statusButtons;
            statusButtons.push_back(ui->pushButton_62);
            statusButtons.push_back(ui->pushButton_63);
            statusButtons.push_back(ui->pushButton_64);
            //statusButtons.push_back(ui->pushButton_177);
            statusPageButton.push_back(statusButtons);
        }

        {
            std::vector<QPushButton*> statusButtons;
            statusButtons.push_back(ui->pushButton_87);
            statusButtons.push_back(ui->pushButton_88);
            statusButtons.push_back(ui->pushButton_89);
            //statusButtons.push_back(ui->pushButton_175);
            statusPageButton.push_back(statusButtons);
        }

        {
            std::vector<QPushButton*> statusButtons;
            statusButtons.push_back(ui->pushButton_112);
            statusButtons.push_back(ui->pushButton_113);
            statusButtons.push_back(ui->pushButton_114);
            //statusButtons.push_back(ui->pushButton_173);
            statusPageButton.push_back(statusButtons);
        }

        {
            std::vector<QPushButton*> statusButtons;
            statusButtons.push_back(ui->pushButton_137);
            statusButtons.push_back(ui->pushButton_138);
            statusButtons.push_back(ui->pushButton_139);
            //statusButtons.push_back(ui->pushButton_171);
            statusPageButton.push_back(statusButtons);
        }

        {
            std::vector<QPushButton*> statusButtons;
            statusButtons.push_back(ui->pushButton_162);
            statusButtons.push_back(ui->pushButton_163);
            statusButtons.push_back(ui->pushButton_164);
            //statusButtons.push_back(ui->pushButton_169);
            statusPageButton.push_back(statusButtons);
        }
    }

    currentIndex = 0;
    for (const auto& statusButtons : statusPageButton)
    {
        // 与数据结构联动
        int postion = 0;
        for (auto& button : statusButtons)
        {
			disconnect(button);
            m_filterConditions[currentIndex].status.m_condition.test(postion) ? button->setChecked(true)
                : button->setChecked(false);

            QVariant pos(postion);
            button->setProperty("postion", pos);
            connect(button, &QPushButton::clicked, this, [=]()
            {
                int pos = button->property("postion").toInt();
                bool status = button->isChecked();
                m_filterConditions[currentIndex].status.OnChangeCondition(std::make_pair(pos, status));
            });

            postion++;
        }

		std::vector<std::string> condNames = { QObject::tr("状态：").toStdString(), QObject::tr("待测").toStdString(), QObject::tr("检测中").toStdString(),QObject::tr("已完成").toStdString()};
		m_filterConditions[currentIndex].status.SetContentName(condNames);
        currentIndex++;
        if (currentIndex >= PAGEMAX)
        {
            break;
        }
    }

    // 样本类型
    std::vector<std::vector<QPushButton*>> sampleTypePageButton;
    {
        {
            std::vector<QPushButton*> sampleTypeButtons;
            sampleTypeButtons.push_back(ui->pushButton_50);
            sampleTypeButtons.push_back(ui->pushButton_51);
            sampleTypeButtons.push_back(ui->pushButton_52);
            sampleTypeButtons.push_back(ui->pushButton_53);
            sampleTypeButtons.push_back(ui->pushButton_54);
            sampleTypeButtons.push_back(ui->pushButton_55);
            sampleTypePageButton.push_back(sampleTypeButtons);
        }

        {
            std::vector<QPushButton*> sampleTypeButtons;
            sampleTypeButtons.push_back(ui->pushButton_75);
            sampleTypeButtons.push_back(ui->pushButton_76);
            sampleTypeButtons.push_back(ui->pushButton_77);
            sampleTypeButtons.push_back(ui->pushButton_78);
            sampleTypeButtons.push_back(ui->pushButton_79);
            sampleTypeButtons.push_back(ui->pushButton_80);
            sampleTypePageButton.push_back(sampleTypeButtons);
        }

        {
            std::vector<QPushButton*> sampleTypeButtons;
            sampleTypeButtons.push_back(ui->pushButton_100);
            sampleTypeButtons.push_back(ui->pushButton_101);
            sampleTypeButtons.push_back(ui->pushButton_102);
            sampleTypeButtons.push_back(ui->pushButton_103);
            sampleTypeButtons.push_back(ui->pushButton_104);
            sampleTypeButtons.push_back(ui->pushButton_105);
            sampleTypePageButton.push_back(sampleTypeButtons);
        }

        {
            std::vector<QPushButton*> sampleTypeButtons;
            sampleTypeButtons.push_back(ui->pushButton_125);
            sampleTypeButtons.push_back(ui->pushButton_126);
            sampleTypeButtons.push_back(ui->pushButton_127);
            sampleTypeButtons.push_back(ui->pushButton_128);
            sampleTypeButtons.push_back(ui->pushButton_129);
            sampleTypeButtons.push_back(ui->pushButton_130);
            sampleTypePageButton.push_back(sampleTypeButtons);
        }

        {
            std::vector<QPushButton*> sampleTypeButtons;
            sampleTypeButtons.push_back(ui->pushButton_150);
            sampleTypeButtons.push_back(ui->pushButton_151);
            sampleTypeButtons.push_back(ui->pushButton_152);
            sampleTypeButtons.push_back(ui->pushButton_153);
            sampleTypeButtons.push_back(ui->pushButton_154);
            sampleTypeButtons.push_back(ui->pushButton_155);
            sampleTypePageButton.push_back(sampleTypeButtons);
        }
    }
    // 对应的grid
    std::vector<QGridLayout*> vecGrids = {
        ui->gridLayout_smpletype1,
        ui->gridLayout_smpletype2,
        ui->gridLayout_smpletype3,
        ui->gridLayout_smpletype4,
        ui->gridLayout_smpletype5
    };

    currentIndex = 0;
    SOFTWARE_TYPE devType = CommonInformationManager::GetInstance()->GetSoftWareType();
    for (auto& sampleTypeButtons : sampleTypePageButton)
    {
        // 与数据结构联动
        int postion = 0;
        for (auto& button : sampleTypeButtons)
        {
			disconnect(button);
            button->setChecked(m_filterConditions[currentIndex].sampleType.m_condition.test(postion));

            QVariant pos(postion);
            button->setProperty("postion", pos);
            connect(button, &QPushButton::clicked, this, [=](){
                int pos = button->property("postion").toInt();
                bool status = button->isChecked();
                m_filterConditions[currentIndex].sampleType.OnChangeCondition(std::make_pair(pos, status));
            });

            postion++;
        }
        // 有生化的前提下才显示
        bool bShowBmqjy = (devType == CHEMISTRY || devType == CHEMISTRY_AND_IMMUNE);
        WorkpageCommon::SetSampleTypeGridBtnsVisble(sampleTypeButtons, vecGrids[currentIndex], 3, bShowBmqjy);

		std::vector<std::string> condNames = { QObject::tr("样本类型：").toStdString(), QObject::tr("血清/血浆").toStdString(), QObject::tr("全血").toStdString(),QObject::tr("尿液").toStdString()
			,QObject::tr("浆膜腔积液").toStdString() ,QObject::tr("脑脊液").toStdString(), QObject::tr("其他").toStdString() };
		m_filterConditions[currentIndex].sampleType.SetContentName(condNames);
        currentIndex++;
        if (currentIndex >= PAGEMAX)
        {
            break;
        }
    }

    // 复查
    std::vector<std::vector<QPushButton*>> reCheckPageButton;
    {
        {
            std::vector<QPushButton*> buttons;
            buttons.push_back(ui->pushButton_65);
            buttons.push_back(ui->pushButton_66);
            reCheckPageButton.push_back(buttons);
        }

        {
            std::vector<QPushButton*> buttons;
            buttons.push_back(ui->pushButton_90);
            buttons.push_back(ui->pushButton_91);
            reCheckPageButton.push_back(buttons);
        }

        {
            std::vector<QPushButton*> buttons;
            buttons.push_back(ui->pushButton_115);
            buttons.push_back(ui->pushButton_116);
            reCheckPageButton.push_back(buttons);
        }

        {
            std::vector<QPushButton*> buttons;
            buttons.push_back(ui->pushButton_140);
            buttons.push_back(ui->pushButton_141);
            reCheckPageButton.push_back(buttons);
        }

        {
            std::vector<QPushButton*> buttons;
            buttons.push_back(ui->pushButton_165);
            buttons.push_back(ui->pushButton_166);
            reCheckPageButton.push_back(buttons);
        }
    }

    currentIndex = 0;
    for (const auto& reCheckButtons : reCheckPageButton)
    {
        // 与数据结构联动
        int postion = 0;
        for (auto& button : reCheckButtons)
        {
            m_filterConditions[currentIndex].reCheckStatus.m_condition.test(postion) ? button->setChecked(true)
                : button->setChecked(false);

            QVariant pos(postion);
            button->setProperty("postion", pos);
			disconnect(button);
            connect(button, &QPushButton::clicked, this, [=]()
            {
                int pos = button->property("postion").toInt();
                bool status = button->isChecked();
                m_filterConditions[currentIndex].reCheckStatus.OnChangeCondition(std::make_pair(pos, status));
            });

            postion++;
        }

		std::vector<std::string> condNames = { QObject::tr("复查：").toStdString(), QObject::tr("有复查").toStdString(), QObject::tr("无复查").toStdString()};
		m_filterConditions[currentIndex].reCheckStatus.SetContentName(condNames);
        currentIndex++;
        if (currentIndex >= PAGEMAX)
        {
            break;
        }
    }

    // 打印
    std::vector<std::vector<QPushButton*>> printPageButton;
    {
        {
            std::vector<QPushButton*> buttons;
			TypeButton& printType = m_typeButtons[1];
            buttons.push_back(ui->pushButton_83);
            buttons.push_back(ui->pushButton_38);
			printType.printConds.buttons = buttons;
			printType.printConds.title = ui->label_129;
            printPageButton.push_back(buttons);
        }

        {
            std::vector<QPushButton*> buttons;
			TypeButton& printType = m_typeButtons[2];
            buttons.push_back(ui->pushButton_84);
            buttons.push_back(ui->pushButton_39);
			printType.printConds.buttons = buttons;
			printType.printConds.title = ui->label_130;
            printPageButton.push_back(buttons);
        }

        {
            std::vector<QPushButton*> buttons;
			TypeButton& printType = m_typeButtons[3];
            buttons.push_back(ui->pushButton_108);
            buttons.push_back(ui->pushButton_40);
			printType.printConds.buttons = buttons;
			printType.printConds.title = ui->label_131;
            printPageButton.push_back(buttons);
        }

        {
            std::vector<QPushButton*> buttons;
			TypeButton& printType = m_typeButtons[4];
            buttons.push_back(ui->pushButton_134);
            buttons.push_back(ui->pushButton_133);
			printType.printConds.buttons = buttons;
			printType.printConds.title = ui->label_135;
            printPageButton.push_back(buttons);
        }

        {
            std::vector<QPushButton*> buttons;
			TypeButton& printType = m_typeButtons[5];
            buttons.push_back(ui->pushButton_182);
            buttons.push_back(ui->pushButton_181);
			printType.printConds.buttons = buttons;
			printType.printConds.title = ui->label_133;
            printPageButton.push_back(buttons);
        }
    }

    currentIndex = 0;
    for (const auto& printButtons : printPageButton)
    {
        // 与数据结构联动
        int postion = 0;
        for (auto& button : printButtons)
        {
            m_filterConditions[currentIndex].printStatus.m_condition.test(postion) ? button->setChecked(true)
                : button->setChecked(false);

            QVariant pos(postion);
            button->setProperty("postion", pos);
			disconnect(button);
            connect(button, &QPushButton::clicked, this, [=]()
            {
                int pos = button->property("postion").toInt();
                bool status = button->isChecked();
                m_filterConditions[currentIndex].printStatus.OnChangeCondition(std::make_pair(pos, status));
            });

            postion++;
        }

		std::vector<std::string> condNames = { QObject::tr("打印：").toStdString(), QObject::tr("已打印").toStdString(), QObject::tr("未打印").toStdString() };
		m_filterConditions[currentIndex].printStatus.SetContentName(condNames);
        currentIndex++;
        if (currentIndex >= PAGEMAX)
        {
            break;
        }
    }

    // 告警
    std::vector<std::vector<QPushButton*>> alarmPageButton;
    {
        {
            std::vector<QPushButton*> buttons;
            buttons.push_back(ui->pushButton_68);
            buttons.push_back(ui->pushButton_67);
            alarmPageButton.push_back(buttons);
        }

        {
            std::vector<QPushButton*> buttons;
            buttons.push_back(ui->pushButton_93);
            buttons.push_back(ui->pushButton_92);
            alarmPageButton.push_back(buttons);
        }

        {
            std::vector<QPushButton*> buttons;
            buttons.push_back(ui->pushButton_118);
            buttons.push_back(ui->pushButton_117);
            alarmPageButton.push_back(buttons);
        }

        {
            std::vector<QPushButton*> buttons;
            buttons.push_back(ui->pushButton_143);
            buttons.push_back(ui->pushButton_142);
            alarmPageButton.push_back(buttons);
        }

        {
            std::vector<QPushButton*> buttons;
            buttons.push_back(ui->pushButton_168);
            buttons.push_back(ui->pushButton_167);
            alarmPageButton.push_back(buttons);
        }
    }

    currentIndex = 0;
    for (const auto& alarmButtons : alarmPageButton)
    {
        // 与数据结构联动
        int postion = 0;
        for (auto& button : alarmButtons)
        {
            m_filterConditions[currentIndex].alarmStatus.m_condition.test(postion) ? button->setChecked(true)
                : button->setChecked(false);

            QVariant pos(postion);
            button->setProperty("postion", pos);
			disconnect(button);
            connect(button, &QPushButton::clicked, this, [=]()
            {
                int pos = button->property("postion").toInt();
                bool status = button->isChecked();
                m_filterConditions[currentIndex].alarmStatus.OnChangeCondition(std::make_pair(pos, status));
            });

            postion++;
        }

		std::vector<std::string> condNames = { QObject::tr("数据报警：").toStdString(), QObject::tr("有报警").toStdString(), QObject::tr("无报警").toStdString() };
		m_filterConditions[currentIndex].alarmStatus.SetContentName(condNames);
        currentIndex++;
        if (currentIndex >= PAGEMAX)
        {
            break;
        }
    }

	// 阴性阳性
	std::vector<std::vector<QPushButton*>> qualJudgePageButton;
	{
		{
			std::vector<QPushButton*> buttons;
			buttons.push_back(ui->pushButton_56);
			buttons.push_back(ui->pushButton_57);
			qualJudgePageButton.push_back(buttons);
		}

		{
			std::vector<QPushButton*> buttons;
			buttons.push_back(ui->pushButton_82);
			buttons.push_back(ui->pushButton_81);
			qualJudgePageButton.push_back(buttons);
		}

		{
			std::vector<QPushButton*> buttons;
			buttons.push_back(ui->pushButton_107);
			buttons.push_back(ui->pushButton_106);
			qualJudgePageButton.push_back(buttons);
		}

		{
			std::vector<QPushButton*> buttons;
			buttons.push_back(ui->pushButton_131);
			buttons.push_back(ui->pushButton_132);
			qualJudgePageButton.push_back(buttons);
		}

		{
			std::vector<QPushButton*> buttons;
			buttons.push_back(ui->pushButton_157);
			buttons.push_back(ui->pushButton_156);
			qualJudgePageButton.push_back(buttons);
		}
	}

	currentIndex = 0;
	for (const auto& qualJudgeButton : qualJudgePageButton)
	{
		// 与数据结构联动
		int postion = 0;
		for (auto& button : qualJudgeButton)
		{
			m_filterConditions[currentIndex].qualJudge.m_condition.test(postion) ? button->setChecked(true)
				: button->setChecked(false);

			QVariant pos(postion);
			button->setProperty("postion", pos);
			disconnect(button);
			connect(button, &QPushButton::clicked, this, [=]()
			{
				int pos = button->property("postion").toInt();
				bool status = button->isChecked();
				m_filterConditions[currentIndex].qualJudge.OnChangeCondition(std::make_pair(pos, status));
			});

			postion++;
		}

		std::vector<std::string> condNames = { QObject::tr("定性结果：").toStdString(), QObject::tr("阳性").toStdString(), QObject::tr("阴性").toStdString() };
		m_filterConditions[currentIndex].qualJudge.SetContentName(condNames);
		currentIndex++;
		if (currentIndex >= PAGEMAX)
		{
			break;
		}
	}

    // 审核
    std::vector<std::vector<QPushButton*>> auditPageButton;
    {
        {
			TypeButton& auditType = m_typeButtons[1];
            std::vector<QPushButton*> buttons;
            buttons.push_back(ui->pushButton_60);
            buttons.push_back(ui->pushButton_61);
			auditType.auditConds.buttons = buttons;
			auditType.auditConds.title = ui->label_30;
            auditPageButton.push_back(buttons);
        }

        {
			TypeButton& auditType = m_typeButtons[2];
            std::vector<QPushButton*> buttons;
            buttons.push_back(ui->pushButton_85);
            buttons.push_back(ui->pushButton_86);
			auditType.auditConds.buttons = buttons;
			auditType.auditConds.title = ui->label_50;
            auditPageButton.push_back(buttons);
        }

        {
			TypeButton& auditType = m_typeButtons[3];
            std::vector<QPushButton*> buttons;
            buttons.push_back(ui->pushButton_110);
            buttons.push_back(ui->pushButton_111);
			auditType.auditConds.buttons = buttons;
			auditType.auditConds.title = ui->label_70;
            auditPageButton.push_back(buttons);
        }

        {
			TypeButton& auditType = m_typeButtons[4];
            std::vector<QPushButton*> buttons;
            buttons.push_back(ui->pushButton_135);
            buttons.push_back(ui->pushButton_136);
			auditType.auditConds.buttons = buttons;
			auditType.auditConds.title = ui->label_90;
            auditPageButton.push_back(buttons);
        }

        {
			TypeButton& auditType = m_typeButtons[5];
            std::vector<QPushButton*> buttons;
            buttons.push_back(ui->pushButton_160);
            buttons.push_back(ui->pushButton_161);
			auditType.auditConds.buttons = buttons;
			auditType.auditConds.title = ui->label_110;
            auditPageButton.push_back(buttons);
        }
    }

    currentIndex = 0;
    for (const auto& auditButtons : auditPageButton)
    {
        // 与数据结构联动
        int postion = 0;
        for (auto& button : auditButtons)
        {
            m_filterConditions[currentIndex].auditStatus.m_condition.test(postion) ? button->setChecked(true)
                : button->setChecked(false);

            QVariant pos(postion);
            button->setProperty("postion", pos);
			disconnect(button);
            connect(button, &QPushButton::clicked, this, [=]()
            {
                int pos = button->property("postion").toInt();
                bool status = button->isChecked();
                m_filterConditions[currentIndex].auditStatus.OnChangeCondition(std::make_pair(pos, status));
            });

            postion++;
        }

		std::vector<std::string> condNames = { QObject::tr("审核：").toStdString(), QObject::tr("已审核").toStdString(), QObject::tr("未审核").toStdString() };
		m_filterConditions[currentIndex].auditStatus.SetContentName(condNames);
        currentIndex++;
        if (currentIndex >= PAGEMAX)
        {
            break;
        }
    }

    // 稀释
    std::vector<std::vector<QPushButton*>> duliTionPageButton;
    {
        {
            std::vector<QPushButton*> buttons;
            buttons.push_back(ui->pushButton_49);
            buttons.push_back(ui->pushButton_48);
            if (devType == IMMUNE)
            {
                ui->pushButton_173->hide();
            }
			buttons.push_back(ui->pushButton_173);
            buttons.push_back(ui->pushButton_178);
            duliTionPageButton.push_back(buttons);
        }

        {
            std::vector<QPushButton*> buttons;
            buttons.push_back(ui->pushButton_74);
            buttons.push_back(ui->pushButton_73);
            if (devType == IMMUNE)
            {
                ui->pushButton_175->hide();
            }
			buttons.push_back(ui->pushButton_175);
            buttons.push_back(ui->pushButton_176);
            duliTionPageButton.push_back(buttons);
        }

        {
            std::vector<QPushButton*> buttons;
            buttons.push_back(ui->pushButton_99);
            buttons.push_back(ui->pushButton_98);
            if (devType == IMMUNE)
            {
                ui->pushButton_177->hide();
            }
			buttons.push_back(ui->pushButton_177);
            buttons.push_back(ui->pushButton_174);
            duliTionPageButton.push_back(buttons);
        }

        {
            std::vector<QPushButton*> buttons;
            buttons.push_back(ui->pushButton_124);
            buttons.push_back(ui->pushButton_123);
            if (devType == IMMUNE)
            {
                ui->pushButton_169->hide();
            }
			buttons.push_back(ui->pushButton_169);
            buttons.push_back(ui->pushButton_172);
            duliTionPageButton.push_back(buttons);
        }

        {
            std::vector<QPushButton*> buttons;
            buttons.push_back(ui->pushButton_149);
            buttons.push_back(ui->pushButton_148);
            if (devType == IMMUNE)
            {
                ui->pushButton_179->hide();
            }
			buttons.push_back(ui->pushButton_179);
            buttons.push_back(ui->pushButton_170);
            duliTionPageButton.push_back(buttons);
        }
    }

    currentIndex = 0;
    for (const auto& duliButtons : duliTionPageButton)
    {
        // 与数据结构联动
        int postion = 0;
        for (auto& button : duliButtons)
        {
            m_filterConditions[currentIndex].dulitionType.m_condition.test(postion) ? button->setChecked(true)
                : button->setChecked(false);

            QVariant pos(postion);
            button->setProperty("postion", pos);
			disconnect(button);
            connect(button, &QPushButton::clicked, this, [=]()
            {
                int pos = button->property("postion").toInt();
                bool status = button->isChecked();
                m_filterConditions[currentIndex].dulitionType.OnChangeCondition(std::make_pair(pos, status));
            });

            postion++;
        }

		std::vector<std::string> condNames;
        condNames = { QObject::tr("稀释状态：").toStdString(), QObject::tr("无").toStdString(), QObject::tr("机内稀释").toStdString(), QObject::tr("机内增量").toStdString(), QObject::tr("手工稀释").toStdString() };

		m_filterConditions[currentIndex].dulitionType.SetContentName(condNames);
        currentIndex++;
        if (currentIndex >= PAGEMAX)
        {
            break;
        }
    }

    // 发送
    std::vector<std::vector<QPushButton*>> sendPageButton;
    {
        {
            std::vector<QPushButton*> buttons;
            buttons.push_back(ui->pushButton_184);
            buttons.push_back(ui->pushButton_183);
            sendPageButton.push_back(buttons);
        }

        {
            std::vector<QPushButton*> buttons;
            buttons.push_back(ui->pushButton_109);
            buttons.push_back(ui->pushButton_58);
            sendPageButton.push_back(buttons);
        }

        {
            std::vector<QPushButton*> buttons;
            buttons.push_back(ui->pushButton_33);
            buttons.push_back(ui->pushButton_34);
			sendPageButton.push_back(buttons);
        }

        {
            std::vector<QPushButton*> buttons;
            buttons.push_back(ui->pushButton_27);
            buttons.push_back(ui->pushButton_26);
			sendPageButton.push_back(buttons);
        }

        {
            std::vector<QPushButton*> buttons;
            buttons.push_back(ui->pushButton_19);
            buttons.push_back(ui->pushButton_201);
			sendPageButton.push_back(buttons);
        }
    }

    currentIndex = 0;
    for (const auto& sendButtons : sendPageButton)
    {
        // 与数据结构联动
        int postion = 0;
        for (auto& button : sendButtons)
        {
            m_filterConditions[currentIndex].sendLis.m_condition.test(postion) ? button->setChecked(true)
                : button->setChecked(false);

            QVariant pos(postion);
            button->setProperty("postion", pos);
			disconnect(button);
            connect(button, &QPushButton::clicked, this, [=]()
            {
                int pos = button->property("postion").toInt();
                bool status = button->isChecked();
                m_filterConditions[currentIndex].sendLis.OnChangeCondition(std::make_pair(pos, status));
            });

            postion++;
        }

		std::vector<std::string> condNames = { QObject::tr("传输：").toStdString(), QObject::tr("已传输").toStdString(), QObject::tr("未传输").toStdString()};
		m_filterConditions[currentIndex].sendLis.SetContentName(condNames);
        currentIndex++;
        if (currentIndex >= PAGEMAX)
        {
            break;
        }
    }

	// ai识别
	std::vector<std::vector<QPushButton*>> aiPageButton;
	{
		{
			std::vector<QPushButton*> buttons;
			buttons.push_back(ui->pushButton_185);
			buttons.push_back(ui->pushButton_186);
			aiPageButton.push_back(buttons);
			// 标题
			TypeButton& aiType = m_typeButtons[1];
			aiType.aiConds.title = ui->label_143;
			aiType.aiConds.buttons = buttons;
			m_typeButtons.push_back(aiType);
		}

		{
			std::vector<QPushButton*> buttons;
			buttons.push_back(ui->pushButton_158);
			buttons.push_back(ui->pushButton_159);
			aiPageButton.push_back(buttons);

			TypeButton& aiType = m_typeButtons[2];
			aiType.aiConds.title = ui->label_141;
			aiType.aiConds.buttons = buttons;
			m_typeButtons.push_back(aiType);
		}

		{
			std::vector<QPushButton*> buttons;
			buttons.push_back(ui->pushButton_36);
			buttons.push_back(ui->pushButton_41);
			aiPageButton.push_back(buttons);

			TypeButton& aiType = m_typeButtons[3];
			aiType.aiConds.title = ui->label_139;
			aiType.aiConds.buttons = buttons;
			m_typeButtons.push_back(aiType);
		}

		{
			std::vector<QPushButton*> buttons;
			buttons.push_back(ui->pushButton_32);
			buttons.push_back(ui->pushButton_28);
			aiPageButton.push_back(buttons);

			TypeButton& aiType = m_typeButtons[4];
			aiType.aiConds.title = ui->label_137;
			aiType.aiConds.buttons = buttons;
			m_typeButtons.push_back(aiType);
		}

		{
			std::vector<QPushButton*> buttons;
			buttons.push_back(ui->pushButton_23);
			buttons.push_back(ui->pushButton_25);
			aiPageButton.push_back(buttons);

			TypeButton& aiType = m_typeButtons[5];
			aiType.aiConds.title = ui->label_134;
			aiType.aiConds.buttons = buttons;
			m_typeButtons.push_back(aiType);
		}
	}

	currentIndex = 0;
	for (const auto& aiButtons : aiPageButton)
	{
		// 与数据结构联动
		int postion = 0;
		for (auto& button : aiButtons)
		{
			m_filterConditions[currentIndex].aiCheck.m_condition.test(postion) ? button->setChecked(true)
				: button->setChecked(false);

			QVariant pos(postion);
			button->setProperty("postion", pos);
			disconnect(button);
			connect(button, &QPushButton::clicked, this, [=]()
			{
				int pos = button->property("postion").toInt();
				bool aiStatus = button->isChecked();
				m_filterConditions[currentIndex].aiCheck.OnChangeCondition(std::make_pair(pos, aiStatus));
			});

			postion++;
		}

		std::vector<std::string> condNames = { QObject::tr("视觉识别：").toStdString(), QObject::tr("无异常").toStdString(), QObject::tr("有异常").toStdString() };
		m_filterConditions[currentIndex].aiCheck.SetContentName(condNames);
		currentIndex++;
		if (currentIndex >= PAGEMAX)
		{
			break;
		}
	}

    // 检测时间
    std::vector<std::vector<QBaseDateEdit*>> examinationPageButton;
    {
        {
            std::vector<QBaseDateEdit*> examinationEdits;
            examinationEdits.push_back(ui->test_start_time_2);
            examinationEdits.push_back(ui->test_stop_time_2);
            examinationPageButton.push_back(examinationEdits);
        }

        {
            std::vector<QBaseDateEdit*> examinationEdits;
            examinationEdits.push_back(ui->test_start_time_3);
            examinationEdits.push_back(ui->test_stop_time_3);
            examinationPageButton.push_back(examinationEdits);
        }

        {
            std::vector<QBaseDateEdit*> examinationEdits;
            examinationEdits.push_back(ui->test_start_time_4);
            examinationEdits.push_back(ui->test_stop_time_4);
            examinationPageButton.push_back(examinationEdits);
        }

        {
            std::vector<QBaseDateEdit*> examinationEdits;
            examinationEdits.push_back(ui->test_start_time_5);
            examinationEdits.push_back(ui->test_stop_time_5);
            examinationPageButton.push_back(examinationEdits);
        }

        {
            std::vector<QBaseDateEdit*> examinationEdits;
            examinationEdits.push_back(ui->test_start_time_6);
            examinationEdits.push_back(ui->test_stop_time_6);
            examinationPageButton.push_back(examinationEdits);
        }
    }

    currentIndex = 0;
    for (auto& examinationEdits : examinationPageButton)
    {
        if (examinationEdits.size() != 2)
        {
            continue;
        }

        auto time = m_filterConditions[currentIndex].examinationTime.m_condition;
        auto startEdit = examinationEdits[0];
        auto endEdit = examinationEdits[1];

        startEdit->setDateTimeStr(QString::fromStdString(time.first));
        endEdit->setDateTimeStr(QString::fromStdString(time.second));

        QVariant index(currentIndex);
        startEdit->setProperty("index", index);
        connect(startEdit, &QDateEdit::dateChanged, this, [=]()
        {
            int currentIndex = startEdit->property("index").toInt();
            std::string startTime = (startEdit->getDateTimeStr(true)).toStdString();
            auto data = std::make_pair(startTime, m_filterConditions[currentIndex].examinationTime.m_condition.second);
            m_filterConditions[currentIndex].examinationTime.OnChangeCondition(data);
        });

        endEdit->setProperty("index", index);
        connect(endEdit, &QDateEdit::dateChanged, this, [=]()
        {
            int currentIndex = endEdit->property("index").toInt();
            std::string stopTime = (endEdit->getDateTimeStr()).toStdString();
            auto data = std::make_pair(m_filterConditions[currentIndex].examinationTime.m_condition.first, stopTime);
            m_filterConditions[currentIndex].examinationTime.OnChangeCondition(data);
        });

		std::vector<std::string> condNames = { QObject::tr("检测日期：").toStdString()};
		m_filterConditions[currentIndex].examinationTime.SetContentName(condNames);
        currentIndex++;
        if (currentIndex >= PAGEMAX)
        {
            break;
        }
    }

    // 样本号范围
    std::vector<std::vector<QLineEdit*>> sampleRangePage;
    {
        {
            std::vector<QLineEdit*> sampleRangeEdits;
            sampleRangeEdits.push_back(ui->sample_no_edit_4);
            sampleRangeEdits.push_back(ui->sample_no_edit_3);
            sampleRangePage.push_back(sampleRangeEdits);
        }

        {
            std::vector<QLineEdit*> sampleRangeEdits;
            sampleRangeEdits.push_back(ui->sample_no_edit_6);
            sampleRangeEdits.push_back(ui->sample_no_edit_5);
            sampleRangePage.push_back(sampleRangeEdits);
        }

        {
            std::vector<QLineEdit*> sampleRangeEdits;
            sampleRangeEdits.push_back(ui->sample_no_edit_8);
            sampleRangeEdits.push_back(ui->sample_no_edit_7);
            sampleRangePage.push_back(sampleRangeEdits);
        }

        {
            std::vector<QLineEdit*> sampleRangeEdits;
            sampleRangeEdits.push_back(ui->sample_no_edit_10);
            sampleRangeEdits.push_back(ui->sample_no_edit_9);
            sampleRangePage.push_back(sampleRangeEdits);
        }

        {
            std::vector<QLineEdit*> sampleRangeEdits;
            sampleRangeEdits.push_back(ui->sample_no_edit_12);
            sampleRangeEdits.push_back(ui->sample_no_edit_11);
            sampleRangePage.push_back(sampleRangeEdits);
        }
    }

    currentIndex = 0;
    for (auto& edits : sampleRangePage)
    {
        if (edits.size() != 2)
        {
            continue;
        }

        auto range = m_filterConditions[currentIndex].sampleRange.m_condition;
        auto startEdit = edits[0];
        auto endEdit = edits[1];
        // 限制样本号输入范围
        //startEdit->setValidator(new QRegExpValidator(QRegExp(UI_REG_SAMPLE_NUM), this));
		// 不限制中文，特殊字符 bug0020311 modify by wuht
		startEdit->setMaxLength(12);
        startEdit->setText(QString::fromStdString(range.first));
        // 限制样本号输入范围
        //endEdit->setValidator(new QRegExpValidator(QRegExp(UI_REG_SAMPLE_NUM), this));
		// 不限制中文，特殊字符 bug0020311 modify by wuht
		endEdit->setMaxLength(12);
        endEdit->setText(QString::fromStdString(range.second));

        QVariant index(currentIndex);
        startEdit->setProperty("index", index);
        connect(startEdit, &QLineEdit::textChanged, this, [=]()
        {
            int currentIndex = startEdit->property("index").toInt();
            QString strSampleBegin = startEdit->text();
            if (!strSampleBegin.isEmpty())
            {
                std::string strEndSeq = m_filterConditions[currentIndex].sampleRange.m_condition.second;
                m_filterConditions[currentIndex].sampleRange.OnChangeCondition(std::make_pair(strSampleBegin.toStdString(), strEndSeq));
            }
			else
			{
				std::string strEndSeq = m_filterConditions[currentIndex].sampleRange.m_condition.second;
				m_filterConditions[currentIndex].sampleRange.OnChangeCondition(std::make_pair("", strEndSeq));
			}
        });

        endEdit->setProperty("index", index);
        connect(endEdit, &QLineEdit::textChanged, this, [=]()
        {
            int currentIndex = endEdit->property("index").toInt();
            QString strSampleEnd = endEdit->text();
            if (!strSampleEnd.isEmpty())
            {
				// modify bug0012141 by wuht
                std::string strStartSequence = m_filterConditions[currentIndex].sampleRange.m_condition.first;
                m_filterConditions[currentIndex].sampleRange.OnChangeCondition(std::make_pair(strStartSequence, strSampleEnd.toStdString()));
            }
			else
			{
				std::string strStartSequence = m_filterConditions[currentIndex].sampleRange.m_condition.first;
				m_filterConditions[currentIndex].sampleRange.OnChangeCondition(std::make_pair(strStartSequence, ""));
			}
        });

		std::vector<std::string> condNames = { QObject::tr("样本号：").toStdString() };
		m_filterConditions[currentIndex].sampleRange.SetContentName(condNames);
        currentIndex++;
        if (currentIndex >= PAGEMAX)
        {
            break;
        }
    }

    // 样本条码
    std::vector<std::vector<QLineEdit*>> barcodePage;
    {
        {
            std::vector<QLineEdit*> sampleBarCodeEdits;
            sampleBarCodeEdits.push_back(ui->sample_bar_edit_2);
            sampleBarCodeEdits.push_back(ui->sample_bar_edit_17);
            barcodePage.push_back(sampleBarCodeEdits);
        }

        {
            std::vector<QLineEdit*> sampleBarCodeEdits;
            sampleBarCodeEdits.push_back(ui->sample_bar_edit_3);
            sampleBarCodeEdits.push_back(ui->sample_bar_edit_16);
            barcodePage.push_back(sampleBarCodeEdits);
        }

        {
            std::vector<QLineEdit*> sampleBarCodeEdits;
            sampleBarCodeEdits.push_back(ui->sample_bar_edit_4);
            sampleBarCodeEdits.push_back(ui->sample_bar_edit_15);
            barcodePage.push_back(sampleBarCodeEdits);
        }

        {
            std::vector<QLineEdit*> sampleBarCodeEdits;
            sampleBarCodeEdits.push_back(ui->sample_bar_edit_5);
            sampleBarCodeEdits.push_back(ui->sample_bar_edit_14);
            barcodePage.push_back(sampleBarCodeEdits);
        }

        {
            std::vector<QLineEdit*> sampleBarCodeEdits;
            sampleBarCodeEdits.push_back(ui->sample_bar_edit_6);
            sampleBarCodeEdits.push_back(ui->sample_bar_edit_13);
            barcodePage.push_back(sampleBarCodeEdits);
        }
    }

    currentIndex = 0;
    for (auto& edits : barcodePage)
    {
        if (edits.size() != 2)
        {
            continue;
        }

        auto range = m_filterConditions[currentIndex].barcode.m_condition;
        auto startEdit = edits[0];
        auto endEdit = edits[1];

        startEdit->setText(QString::fromStdString(range.first));
        startEdit->setMaxLength(MAXLENGTH);
        startEdit->setValidator(new QRegExpValidator(QRegExp(UI_REG_SAMPLE_BAR_ASCII), this));

        endEdit->setText(QString::fromStdString(range.second));
        endEdit->setMaxLength(MAXLENGTH);
        endEdit->setValidator(new QRegExpValidator(QRegExp(UI_REG_SAMPLE_BAR_ASCII), this));

        QVariant index(currentIndex);
        startEdit->setProperty("index", index);
        connect(startEdit, &QLineEdit::textChanged, this, [=]()
        {
            int currentIndex = startEdit->property("index").toInt();
            std::string startBarCode = startEdit->text().toStdString();
            std::string endBarCode = endEdit->text().toStdString();
            auto data = std::make_pair(startBarCode, endBarCode);
            m_filterConditions[currentIndex].barcode.OnChangeCondition(data);
        });

        endEdit->setProperty("index", index);
        connect(endEdit, &QLineEdit::textChanged, this, [=]()
        {
            int currentIndex = endEdit->property("index").toInt();
            std::string startBarCode = startEdit->text().toStdString();
            std::string endBarCode = endEdit->text().toStdString();
            auto data = std::make_pair(startBarCode, endBarCode);
            m_filterConditions[currentIndex].barcode.OnChangeCondition(data);
        });

		std::vector<std::string> condNames = { QObject::tr("样本条码：").toStdString() };
		m_filterConditions[currentIndex].barcode.SetContentName(condNames);
        currentIndex++;
        if (currentIndex >= PAGEMAX)
        {
            break;
        }
    }

    // 设备模块
    std::vector<std::vector<QComboBox*>> devicePage;
    {
        {
            std::vector<QComboBox*> moduleEdits;
            moduleEdits.push_back(ui->comboBox_3);
            moduleEdits.push_back(ui->comboBox_4);
            devicePage.push_back(moduleEdits);
        }

        {
            std::vector<QComboBox*> moduleEdits;
            moduleEdits.push_back(ui->comboBox_5);
            moduleEdits.push_back(ui->comboBox_6);
            devicePage.push_back(moduleEdits);
        }

        {
            std::vector<QComboBox*> moduleEdits;
            moduleEdits.push_back(ui->comboBox_7);
            moduleEdits.push_back(ui->comboBox_8);
            devicePage.push_back(moduleEdits);
        }

        {
            std::vector<QComboBox*> moduleEdits;
            moduleEdits.push_back(ui->comboBox_9);
            moduleEdits.push_back(ui->comboBox_10);
            devicePage.push_back(moduleEdits);
        }

        {
            std::vector<QComboBox*> moduleEdits;
            moduleEdits.push_back(ui->comboBox_11);
            moduleEdits.push_back(ui->comboBox_12);
            devicePage.push_back(moduleEdits);
        }
    }

    currentIndex = 0;
    for (auto& comboxs : devicePage)
    {
        if (comboxs.size() != 2)
        {
            continue;
        }

        auto deviceSn = m_filterConditions[currentIndex].device.m_condition;
        auto deviceClassy = comboxs[0];
        auto deiviceName = comboxs[1];
		disconnect(deviceClassy,0,0,0);
		disconnect(deiviceName,0,0,0);
        SetDeivceComBoxInfo(deviceClassy, deiviceName);
        connect(deviceClassy, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, [=](int index)
        {
            deiviceName->clear();
            deiviceName->addItem("", "");
            auto devices = deviceClassy->itemData(index).toStringList();
            for (const auto& device : devices)
            {
                auto spDevice = CommonInformationManager::GetInstance()->GetDeviceInfo(device.toStdString());
                if (spDevice == Q_NULLPTR)
                {
                    continue;
                }

				// 当ISE有两个模块的时候需要区分A和B
				if ((spDevice->deviceClassify == tf::AssayClassify::ASSAY_CLASSIFY_ISE) && (spDevice->moduleCount > 1))
				{
					// ISEA
					auto iseNameA = CommonInformationManager::GetDeviceName(device.toStdString(), 1);
					deiviceName->addItem(QString::fromStdString(iseNameA), QString::fromStdString(spDevice->deviceSN) + ",1");

					// ISEB
					auto iseNameB = CommonInformationManager::GetDeviceName(device.toStdString(), 2);
					deiviceName->addItem(QString::fromStdString(iseNameB), QString::fromStdString(spDevice->deviceSN) + ",2");
					continue;
				}

                auto name = QString::fromStdString(spDevice->groupName + spDevice->name);
                deiviceName->addItem(name, QString::fromStdString(spDevice->deviceSN));
            }
            deiviceName->setCurrentIndex(0);
        });

		auto deviceSnParamter = QString::fromStdString(deviceSn).split(",");
		if (!deviceSnParamter.empty())
		{
			deviceSn = deviceSnParamter[0].toStdString();
		}

        deviceClassy->setCurrentIndex(0);
        auto spDevice = CommonInformationManager::GetInstance()->GetDeviceInfo(deviceSn);
        if (spDevice != Q_NULLPTR)
        {
            QString strAssayClassfyName = ThriftEnumTrans::GetAssayClassfiyName(spDevice->deviceClassify);
            if (!strAssayClassfyName.isEmpty())
            {
                int index = deviceClassy->findText(strAssayClassfyName);
                deviceClassy->setCurrentIndex(index);
            }
			auto name = QString::fromStdString(spDevice->groupName + spDevice->name);

			if (deviceSnParamter.size() > 1)
			{
				bool ok = false;
				int moduleIndex = deviceSnParamter[1].toInt(&ok);
				if (!ok)
				{
					moduleIndex = 1;
				}

				name = QString::fromStdString(CommonInformationManager::GetDeviceName(deviceSn, moduleIndex));
			}

			if (!name.isEmpty())
			{
				int index = deiviceName->findText(name);
				deiviceName->setCurrentIndex(index);
			}
        }

        QVariant index(currentIndex);
        deiviceName->setProperty("index", index);
        connect(deiviceName, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), 
            this, [=](int index)
        {
			auto deviceSn = deiviceName->itemData(index).toString().toStdString();
			auto deviceSnRaw = deviceSn;
			auto deviceSnParamter = QString::fromStdString(deviceSn).split(",");
			if (!deviceSnParamter.empty())
			{
				deviceSn = deviceSnParamter[0].toStdString();
			}

			std::string deviceType;
			auto spDevice = CommonInformationManager::GetInstance()->GetDeviceInfo(deviceSn);
			if (spDevice != Q_NULLPTR)
			{
                QString strAssayClassfyName = ThriftEnumTrans::GetAssayClassfiyName(spDevice->deviceClassify);
				if (!strAssayClassfyName.isEmpty())
				{
					deviceType = strAssayClassfyName.toStdString();
				}
			}

			int currentIndex = deiviceName->property("index").toInt();
            m_filterConditions[currentIndex].device.OnChangeCondition(std::make_pair(deviceSnRaw, deviceType));
        });

		std::vector<std::string> condNames = { QObject::tr("模块: ").toStdString() };
		m_filterConditions[currentIndex].device.SetContentName(condNames);
        currentIndex++;
        if (currentIndex >= PAGEMAX)
        {
            break;
        }
    }

    // 病历号
    std::vector<QLineEdit*> medicalCasePage;
    {
        medicalCasePage.push_back(ui->patient_record_edit_2);
        medicalCasePage.push_back(ui->patient_record_edit_3);
        medicalCasePage.push_back(ui->patient_record_edit_4);
        medicalCasePage.push_back(ui->patient_record_edit_5);
        medicalCasePage.push_back(ui->patient_record_edit_6);
    }

    auto f = [&](int db)->boost::optional<tf::PatientInfo>
    {
        ::tf::PatientInfoQueryCond patienQryCond;
        patienQryCond.__set_id(db);
        ::tf::PatientInfoQueryResp patienQryResp;
        // 执行查询条件
        if (!DcsControlProxy::GetInstance()->QueryPatientInfo(patienQryResp, patienQryCond)
            || patienQryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS
            || patienQryResp.lstPatientInfos.empty())
        {
            ULOG(LOG_ERROR, "query patient Info failed!");
            return boost::none;
        }

        return patienQryResp.lstPatientInfos[0];
    };

    currentIndex = 0;
    for (auto& edits : medicalCasePage)
    {
		disconnect(edits);
		edits->setMaxLength(50);
		edits->setText("");
        // 虽然有两个参数，但是都一样
        auto range = m_filterConditions[currentIndex].medicalCase.m_condition;
		auto content = m_filterConditions[currentIndex].medicalCase.m_condContent;
        if (!range.empty())
        {
			if (!content.empty())
			{
				auto contentString = QString::fromStdString(content);
				auto contents = contentString.split(":");
				if (contents.size() > 1)
				{
					QString rawData = contents[1];
					if (!rawData.isEmpty())
					{
						rawData.mid(1);
					}
					edits->setText(rawData);
				}
				else
				{
					edits->setText("");
				}
			}
        }

        QVariant index(currentIndex);
        edits->setProperty("index", index);
        connect(edits, &QLineEdit::textEdited, this, [=]()
        {
            int currentIndex = edits->property("index").toInt();
            std::string medicalCase = edits->text().toStdString();
            auto data = std::make_pair(medicalCase, std::string());
            m_filterConditions[currentIndex].medicalCase.OnChangeCondition(data);
        });

		std::vector<std::string> condNames = { QObject::tr("患者ID: ").toStdString() };
		m_filterConditions[currentIndex].medicalCase.SetContentName(condNames);
        currentIndex++;
        if (currentIndex >= PAGEMAX)
        {
            break;
        }
    }

    // 试剂批号
    std::vector<QLineEdit*> reagentLotPage;
    {
        reagentLotPage.push_back(ui->lineEdit_2);
        reagentLotPage.push_back(ui->lineEdit_4);
        reagentLotPage.push_back(ui->lineEdit_6);
        reagentLotPage.push_back(ui->lineEdit_8);
        reagentLotPage.push_back(ui->lineEdit_10);
    }

    currentIndex = 0;
    for (auto& edits : reagentLotPage)
    {
		disconnect(edits);
		edits->setMaxLength(50);
        edits->setText("");
        // 虽然有两个参数，但是都一样
        std::set<std::string>& setReagentLots = m_filterConditions[currentIndex].reagentLots.m_condition;
		// modify bug0024958 by wuht
		//if (!setReagentLots.empty())
        {
            auto reagents =  boost::algorithm::join(setReagentLots, ";");
            edits->setText(QString::fromStdString(reagents));
        }

        QVariant index(currentIndex);
        edits->setProperty("index", index);
        connect(edits, &QLineEdit::textEdited, this, [=]()
        {
            int currentIndex = edits->property("index").toInt();
            QString strReagentLots = edits->text();
			// modify bug0024958 by wuht
            //if (!strReagentLots.isEmpty())
            {
                m_filterConditions[currentIndex].reagentLots.OnChangeCondition(strReagentLots.toStdString());
            }
        });

		std::vector<std::string> condNames = { QObject::tr("试剂批号: ").toStdString() };
		m_filterConditions[currentIndex].reagentLots.SetContentName(condNames);
        currentIndex++;
        if (currentIndex >= PAGEMAX)
        {
            break;
        }
    }

    // 患者姓名
    std::vector<QLineEdit*> patientNamePage;
    {
        patientNamePage.push_back(ui->patient_name_edit_2);
        patientNamePage.push_back(ui->patient_name_edit_3);
        patientNamePage.push_back(ui->patient_name_edit_4);
        patientNamePage.push_back(ui->patient_name_edit_5);
        patientNamePage.push_back(ui->patient_name_edit_6);
    }

    currentIndex = 0;
    for (auto& edits : patientNamePage)
    {
		disconnect(edits);
		edits->setMaxLength(50);
        edits->setText("");
        // 虽然有两个参数，但是都一样
        auto range = m_filterConditions[currentIndex].patientName.m_condition;
		auto content = m_filterConditions[currentIndex].patientName.m_condContent;
        if (!range.empty())
        {
			if (!content.empty())
			{
				auto contentString = QString::fromStdString(content);
				auto contents = contentString.split(":");
				if (contents.size() > 1)
				{
					QString rawData = contents[1];
					if (!rawData.isEmpty())
					{
						rawData.mid(1);
					}
					edits->setText(rawData);
				}
				else
				{
					edits->setText("");
				}
			}
        }

        QVariant index(currentIndex);
        edits->setProperty("index", index);
        connect(edits, &QLineEdit::textEdited, this, [=]()
        {
            int currentIndex = edits->property("index").toInt();
            std::string patientName = edits->text().toStdString();
            auto data = std::make_pair(std::string(), patientName);
            m_filterConditions[currentIndex].patientName.OnChangeCondition(data);
        });

		std::vector<std::string> condNames = { QObject::tr("患者姓名: ").toStdString() };
		m_filterConditions[currentIndex].patientName.SetContentName(condNames);
        currentIndex++;
        if (currentIndex >= PAGEMAX)
        {
            break;
        }
    }

    // 是否使能
    std::vector<QCheckBox*> enablePage;
    {
        enablePage.push_back(ui->checkBox);
        enablePage.push_back(ui->checkBox_2);
        enablePage.push_back(ui->checkBox_3);
        enablePage.push_back(ui->checkBox_4);
        enablePage.push_back(ui->checkBox_5);
    }

    currentIndex = 0;
    for (auto& checkBox : enablePage)
    {
        // 设置状态
        checkBox->setChecked(m_filterConditions[currentIndex].enable);
        emit enableFilter(currentIndex, checkBox->isChecked());

        QVariant index(currentIndex);
        checkBox->setProperty("index", index);
        connect(checkBox, &QCheckBox::clicked, this, [=]()
        {
            int currentIndex = checkBox->property("index").toInt();
            bool status = checkBox->isChecked();
            // 当没有选择条件的时候，不能启用
			/*bug3106
            if (m_filterConditions[currentIndex].GetCondNumber() == 0 && status)
            {
                TipDlg(tr("提示"), tr("条件不能为空")).exec();
                checkBox->setChecked(false);
                return;
            }*/

            m_filterConditions[currentIndex].enable = status;
            // 使能快捷按钮
            //emit enableFilter(currentIndex, status);
        });

        currentIndex++;
        if (currentIndex >= PAGEMAX)
        {
            break;
        }
    }

    // 快捷名称
    std::vector<QLineEdit*> namePage;
    {
        namePage.push_back(ui->lineEdit_3);
        namePage.push_back(ui->lineEdit_5);
        namePage.push_back(ui->lineEdit_7);
        namePage.push_back(ui->lineEdit_9);
        namePage.push_back(ui->lineEdit_11);
    }

    currentIndex = 0;
    for (auto& nameEdit : namePage)
    {
        // 设置名称
        auto titleName = QString::fromStdString(m_filterConditions[currentIndex].name);
        if (titleName.isEmpty())
        {
            titleName = tr("快捷筛选") + QString::number(currentIndex + 1);
        }

        m_filterConditions[currentIndex].name = titleName.toStdString();
        nameEdit->setMaxLength(10);
        nameEdit->setText(titleName);
        // 是否显示
        if (!m_filterConditions[currentIndex].enable)
        {
            emit changeFilterName(currentIndex, titleName);
        }

        QVariant index(currentIndex);
        nameEdit->setProperty("index", index);
        connect(nameEdit, &QLineEdit::textChanged, this, [=]()
        {
            int currentIndex = nameEdit->property("index").toInt();
            m_filterConditions[currentIndex].name = nameEdit->text().toStdString();
            // emit changeFilterName(currentIndex, nameEdit->text());
        });

        currentIndex++;
        if (currentIndex >= PAGEMAX)
        {
            break;
        }
    }

    // 项目名显示
    std::vector<QPushButton*> assaySelectButtons;
    {
        assaySelectButtons.push_back(ui->assay_select_btn_2);
        assaySelectButtons.push_back(ui->assay_select_btn_3);
        assaySelectButtons.push_back(ui->assay_select_btn_4);
        assaySelectButtons.push_back(ui->assay_select_btn_5);
        assaySelectButtons.push_back(ui->assay_select_btn_6);
    }

    currentIndex = 1;
    for (auto& selectButton : assaySelectButtons)
    {
        QVariant index(currentIndex);
        selectButton->setProperty("index", index);
        connect(selectButton, &QPushButton::clicked, this, [=]()
        {
            int currentIndex = selectButton->property("index").toInt();
            m_currentSetPage = currentIndex;
            OnAssaySelect();
        });

        currentIndex++;
        if (currentIndex > PAGEMAX)
        {
            break;
        }
    }

    // 只管显示
    std::vector<QTextEdit*> assayNameShows;
    {
        assayNameShows.push_back(ui->assay_edit_2);
        assayNameShows.push_back(ui->assay_edit_3);
        assayNameShows.push_back(ui->assay_edit_4);
        assayNameShows.push_back(ui->assay_edit_5);
        assayNameShows.push_back(ui->assay_edit_6);
    }

    currentIndex = 0;
    for (auto& edits : assayNameShows)
    {
        // 按项目展示和按样本展示一样
        QStringList assays;
        auto assayCodes = m_filterConditions[currentIndex].sampleAssays.m_condition;
        for (auto assayCode : assayCodes)
        {
            std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(assayCode);
			// 判空处理
			if (spAssayInfo == Q_NULLPTR)
			{
				continue;
			}

            assays.append(QString::fromStdString(spAssayInfo->assayName));
        }

        edits->setReadOnly(true);
        edits->setText(assays.join(tr("、")));

		std::vector<std::string> condNames = { QObject::tr("项目名：").toStdString() };
		m_filterConditions[currentIndex].sampleAssays.SetContentName(condNames);
        currentIndex++;
        if (currentIndex >= PAGEMAX)
        {
            break;
        }
    }
}

void QFilterSample::SetDeivceComBoxInfo(QComboBox* classyComBox, QComboBox* nameComBox)
{
    if (classyComBox == Q_NULLPTR || nameComBox == Q_NULLPTR)
    {
        return;
    }
    classyComBox->clear();
    nameComBox->clear();

    // 添加空白行
    classyComBox->addItem("", "");
    nameComBox->addItem("", "");

    std::map<QString, QStringList> mapsDevices;
    // 获取设备信息
    auto deviceMap = CommonInformationManager::GetInstance()->GetDeviceMaps();
    for (auto& device : deviceMap)
    {
        if (device.second == Q_NULLPTR)
        {
            continue;
        }

        // 其他类型不显示
        if (device.second->deviceClassify == tf::AssayClassify::type::ASSAY_CLASSIFY_OTHER)
        {
            continue;
        }

        QString classyName = ThriftEnumTrans::GetAssayClassfiyName(device.second->deviceClassify, tr("未知"));
        if (mapsDevices.count(classyName) > 0)
        {
            mapsDevices[classyName].push_back(QString::fromStdString(device.second->deviceSN));
        }
        else
        {
            QStringList deviceSns(QString::fromStdString(device.second->deviceSN));
            mapsDevices[classyName] = deviceSns;
        }
    }

    // 设置combox
    for (auto& deviceInfo : mapsDevices)
    {
        classyComBox->addItem(deviceInfo.first, deviceInfo.second);
    }
}

void QFilterSample::showEvent(QShowEvent* event)
{
	// 每次显示都重新初始化当前页面

	if (m_currentSetPage)
	{
		auto page = m_currentSetPage.value();
		if (page >= 0 && page <= ui->tabWidget->count())
		{
			ui->tabWidget->setCurrentIndex(page);
		}

		// 保证筛选页面进入的时候save按钮灰色(0013240)
		if (page == 0)
		{
			ui->save_btn->setEnabled(false);
		}

	}
    else
    {
        // 用以修正：在点击确定退出筛选设置框后，主界面勾选掉筛选，
        // 再次进入时m_currentSetPage被设置成了无效，而当前选中没改变,导致无法保存的问题。
        int curIdx = ui->tabWidget->currentIndex();
        if (curIdx >= 0)
        {
            m_currentSetPage = curIdx;
        }

		// 保证筛选页面进入的时候save按钮灰色(0013240)
		if (curIdx == 0)
		{
			ui->save_btn->setEnabled(false);
		}
    }

	//InitPageCurrent();
    BaseDlg::showEvent(event);
}

///
/// @bref
///		设置‘项目名’编辑框的内容
///
/// @param[in] strAssays 项目字符串
///
/// @par History:
/// @li 8276/huchunli, 2023年9月4日，新建函数
///
void QFilterSample::SetAssayEdit(QTextEdit* curEdit, const QString& strAssays)
{
    if (curEdit == Q_NULLPTR)
    {
        return;
    }

    curEdit->setText(strAssays);
    curEdit->setToolTip(strAssays);

    // 设置空时，对项目选择框进行清空处理
    if (m_assaySelectDialog != nullptr && strAssays.isEmpty())
    {
        m_assaySelectDialog->ClearSelectedAssays();
    }
}

///
/// @bref
///		获取当前页面的项目编辑框中的项目
///
/// @par History:
/// @li 8276/huchunli, 2023年11月15日，新建函数
///
void QFilterSample::TakeCurrentPageAssays(QStringList& lstAssay)
{
    static std::vector<QTextEdit*> assayNameShows = {
		ui->assay_edit,
        ui->assay_edit_2,
        ui->assay_edit_3,
        ui->assay_edit_4,
        ui->assay_edit_5,
        ui->assay_edit_6
    };

    QString strAssays;
    if (!m_currentSetPage)
    {
        strAssays = ui->assay_edit->toPlainText();
    }
    else if ((m_currentSetPage.value() < assayNameShows.size()) && m_currentSetPage.value() >= 0)
    {
        strAssays = assayNameShows[m_currentSetPage.value()]->toPlainText();
    }

    lstAssay = strAssays.split("、");
}

///
/// @brief 执行当前筛选
///
///
/// @return true:成功
///
/// @par History:
/// @li 5774/WuHongTao，2023年11月24日，新建函数
///
bool QFilterSample::ExecuteFilterCond(bool isChanged)
{
	// 当前页面的范围不能超出
	auto CheckPageIndexRange = [&](boost::optional<int> pageIndex)->bool {
		// 获取之前的页面参数，若没有则无法保存，直接返回
		if (!pageIndex)
		{
			return false;
		}

		// 越界
		if (pageIndex.value() < 1 || pageIndex.value() > m_filterConditions.size())
		{
			return false;
		}

		return true;
	};

	// 保存参数设置
	auto saveDataFunction = [&]()->bool {
		std::string xml;
		if (!Encode2Json(xml, m_filterConditions))
		{
            ULOG(LOG_WARN, "Failed to encode2json.");
			return false;
		}

		// 更新条件,修改数据字典信息
		m_conditionXml = xml;
		tf::DictionaryInfo di;
		di.__set_keyName(DKN_UI_WORKPAGE_FILTER_COND);
		di.__set_value(xml);
		if (!DcsControlProxy::GetInstance()->ModifyDictionaryInfo(di))
		{
            ULOG(LOG_WARN, "Failed to modify dictionary info, KeyName:%s.", di.keyName.c_str());
			return false;
		}

		if (CheckPageIndexRange(m_currentSetPage))
		{
			auto index = m_currentSetPage.value() - 1;
			emit enableFilter(index, m_filterConditions[index].enable);
			// 快捷按钮的名称
			emit changeFilterName(index, QString::fromStdString(m_filterConditions[index].name));
		}

		return true;
	};

	auto index = ui->tabWidget->currentIndex();
	// 当前页面是筛选页面,用户点击确定，执行筛选动作
	if (index == 0 && !isChanged)
	{
		// 1:检查筛选参数是否正确
		if (CheckCond(m_realCond, false))
		{
			// 执行筛选消息
			return true;
		}
		// 若参数检查不过，则需要恢复之前的参数设置
		else
		{
			//modify bug0011899 by wuht
			m_realCond.Reset();
			InitPageCurrent();
		}

		// 改变页面参数
		// m_currentSetPage = index;
	}
	// isChanged代表页面切换，当页面切换的时候需要保存之前页面的设置参数
	// 从一个快捷页面切换到另外一个快捷页面，需要保存之前的快捷页面的参数
	else if (isChanged)
	{
		// 获取之前的页面参数，若没有则无法保存，直接返回
		if (!CheckPageIndexRange(m_currentSetPage))
		{
            ULOG(LOG_WARN, "Invalid current-set-page index range.");
			return false;
		}

        // 1:检查筛选参数是否正确(检查快捷页面参数是否符合要求)
        FilterConDition& cond = m_filterConditions[m_currentSetPage.value() - 1];
		if (CheckCond(cond, true))
		{
			return saveDataFunction();
		}
		// 若参数检查不过，则需要恢复之前的参数设置
		else
		{
			//modify bug0011899 by wuht
			cond.Reset();
			InItPageAttribute();
		}

		// 改变页面参数
		// m_currentSetPage = index;
	}
	// 快捷页面点击保存
	else if (index != 0 && !isChanged)
	{
		// 越界
		if (!CheckPageIndexRange(index))
        {
            ULOG(LOG_WARN, "Invalid current-set-page index range.");
			return false;
		}

        // 1:检查筛选参数是否正确(检查快捷页面参数是否符合要求)
        const FilterConDition& cond = m_filterConditions[index - 1];
		if (CheckCond(cond, true))
		{
			return saveDataFunction();
		}
		// 若参数检查不过，则需要恢复之前的参数设置
		else
		{
			InItPageAttribute();
		}
	}

	return false;
}

///
/// @brief 检查条件是否符合要求
///
/// @param[in]  condition  条件
/// @param[in]  isSelect  是否是筛选（非快捷筛选）
///
/// @return true:符合要求
///
/// @par History:
/// @li 5774/WuHongTao，2023年11月24日，新建函数
///
bool QFilterSample::CheckCond(const FilterConDition& condition, bool isFastCond)
{
	bool ret = false;
	// 当前的条件不允许为空
	if (condition.GetCondNumber().first == 0)
	{
		TipDlg(tr("提示"), tr("必须至少选择一个筛选条件")).exec();
		return ret;
	}

	// 快捷筛选需要特殊处理的
	if (isFastCond)
	{
		// 筛选名称检测
		if (condition.name.empty())
		{
			TipDlg(tr("提示"), tr("快捷筛选名称不能为空，请重新设置！")).exec();
			return ret;
		}

		// 任何情况下不能存在两个相同的筛选名称
		std::set<std::string> names;
		for (auto& condition : m_filterConditions)
		{
			if (names.count(condition.name))
			{
				TipDlg(tr("提示"), tr("名字有重复，不能保存")).exec();
				return ret;
			}

			names.insert(condition.name);
		}
	}

	auto startSeq = condition.sampleRange.m_condition.first;
	auto endSeq = condition.sampleRange.m_condition.second;
	if (!startSeq.empty() && !endSeq.empty())
	{
		if (stringutil::IsPureDigit(startSeq) && stringutil::IsPureDigit(endSeq)
			&& (boost::multiprecision::cpp_int(endSeq) < boost::multiprecision::cpp_int(startSeq)))
		{
			TipDlg(tr("提示"), tr("开始样本号大于结束样本号")).exec();
			return ret;
		}
	}

	// 条码的限制
	{
		auto startBar = condition.barcode.m_condition.first;
		auto endBar = condition.barcode.m_condition.second;
		// 若起始条码都不为空
		if (!startBar.empty() && !endBar.empty())
		{
			if (stringutil::IsPureDigit(startBar) && stringutil::IsPureDigit(endBar)
				&& (boost::multiprecision::cpp_int(endBar) < boost::multiprecision::cpp_int(startBar)))
			{
				TipDlg(tr("提示"), tr("开始条码大于结束条码")).exec();
				return ret;
			}
		}
	}

	auto startTime = condition.examinationTime.m_condition.first;
	auto endTime = condition.examinationTime.m_condition.second;
	if (startTime.empty() ^ endTime.empty())
	{
		TipDlg(tr("提示"), tr("必须同时设置开始和结束时间")).exec();
		return ret;
	}

	if (startTime > endTime)
	{
		TipDlg(tr("提示"), tr("检测日期结束时间早于开始时间，请重新设置！")).exec();
		return ret;
	}

	return true;
}

void QFilterSample::InitPageCurrent()
{
    // 订单类型
    {
        std::vector<QPushButton*> oderButtons;
        oderButtons.push_back(ui->pushButton_ordertypenormal);
        oderButtons.push_back(ui->pushButton_ordertypeurg);
        oderButtons.push_back(ui->pushButton_ordertypecali);
        oderButtons.push_back(ui->pushButton_ordertypeqc);


        // 与数据结构联动
        int postion = 0;
        for (auto& button : oderButtons)
        {
            m_realCond.oderType.m_condition.test(postion) ? button->setChecked(true)
                : button->setChecked(false);

            QVariant pos(postion);
            button->setProperty("postion", pos);
            connect(button, &QPushButton::clicked, this, [=]()
            {
                int pos = button->property("postion").toInt();
                bool status = button->isChecked();
                m_realCond.oderType.OnChangeCondition(std::make_pair(pos, status));
            });

            postion++;
        }

		std::vector<std::string> condNames = { QObject::tr("订单类型：").toStdString(), QObject::tr("常规").toStdString(), QObject::tr("急诊").toStdString(),QObject::tr("校准").toStdString(),QObject::tr("质控").toStdString() };
		m_realCond.oderType.SetContentName(condNames);
    }

    // 状态
    {
        std::vector<QPushButton*> buttons;
        buttons.push_back(ui->pushButton_stutaWait);
        buttons.push_back(ui->pushButton_stutaTesting);
        buttons.push_back(ui->pushButton_stutaFinish);
        //buttons.push_back(ui->pushButton_179);

        // 与数据结构联动
        int postion = 0;
        for (auto& button : buttons)
        {
            m_realCond.status.m_condition.test(postion) ? button->setChecked(true)
                : button->setChecked(false);

            QVariant pos(postion);
            button->setProperty("postion", pos);
            connect(button, &QPushButton::clicked, this, [=]()
            {
                int pos = button->property("postion").toInt();
                bool status = button->isChecked();
                m_realCond.status.OnChangeCondition(std::make_pair(pos, status));
            });

            postion++;
        }

		std::vector<std::string> condNames = { QObject::tr("状态：").toStdString(), QObject::tr("待测").toStdString(), QObject::tr("检测中").toStdString(),QObject::tr("已完成").toStdString() };
		m_realCond.status.SetContentName(condNames);
    }

	SOFTWARE_TYPE devType = CommonInformationManager::GetInstance()->GetSoftWareType();
    // 样本类型
    {
        std::vector<QPushButton*> buttons = {
            ui->pushButton_sampleXQ,
            ui->pushButton_sampleQX,
            ui->pushButton_sampleNY,
            ui->pushButton_sampleBMQJY,
            ui->pushButton_sampleNJY,
            ui->pushButton_sampleQT
        };

        // 与数据结构联动
        int postion = 0;
        for (auto& button : buttons)
        {
            button->setChecked(m_realCond.sampleType.m_condition.test(postion));

            QVariant pos(postion);
            button->setProperty("postion", pos);
            connect(button, &QPushButton::clicked, this, [=](){
                int pos = button->property("postion").toInt();
                bool status = button->isChecked();
                m_realCond.sampleType.OnChangeCondition(std::make_pair(pos, status));
            });

            postion++;
        }

        // 免疫，设置包膜腔积液的显示与隐藏（有生化的前提下才显示 包膜腔积液
        bool bShowBmqjy = (devType == CHEMISTRY || devType == CHEMISTRY_AND_IMMUNE);
        WorkpageCommon::SetSampleTypeGridBtnsVisble(buttons, ui->gridLayout, 3, bShowBmqjy);

		std::vector<std::string> condNames = { QObject::tr("样本类型：").toStdString(), QObject::tr("血清/血浆").toStdString(), QObject::tr("全血").toStdString(),QObject::tr("尿液").toStdString()
			,QObject::tr("浆膜腔积液").toStdString() ,QObject::tr("脑脊液").toStdString(), QObject::tr("其他").toStdString() };
		m_realCond.sampleType.SetContentName(condNames);
    }

    // 复查
    {
        std::vector<QPushButton*> buttons;
        buttons.push_back(ui->pushButton_retestHave);
        buttons.push_back(ui->pushButton_retestNo);


        // 与数据结构联动
        int postion = 0;
        for (auto& button : buttons)
        {
            m_realCond.reCheckStatus.m_condition.test(postion) ? button->setChecked(true)
                : button->setChecked(false);

            QVariant pos(postion);
            button->setProperty("postion", pos);
            connect(button, &QPushButton::clicked, this, [=]()
            {
                int pos = button->property("postion").toInt();
                bool status = button->isChecked();
                m_realCond.reCheckStatus.OnChangeCondition(std::make_pair(pos, status));
            });

            postion++;
        }

		std::vector<std::string> condNames = { QObject::tr("复查：").toStdString(), QObject::tr("有复查").toStdString(), QObject::tr("无复查").toStdString() };
		m_realCond.reCheckStatus.SetContentName(condNames);
    }

    // 打印
    {
        std::vector<QPushButton*> buttons;
		TypeButton& printType = m_typeButtons[0];
        buttons.push_back(ui->pushButton_printAlready);
        buttons.push_back(ui->pushButton_printNot);
		printType.printConds.buttons = buttons;
		printType.printConds.title = ui->label_128;

        // 与数据结构联动
        int postion = 0;
        for (auto& button : buttons)
        {
            m_realCond.printStatus.m_condition.test(postion) ? button->setChecked(true)
                : button->setChecked(false);

            QVariant pos(postion);
            button->setProperty("postion", pos);
            connect(button, &QPushButton::clicked, this, [=]()
            {
                int pos = button->property("postion").toInt();
                bool status = button->isChecked();
                m_realCond.printStatus.OnChangeCondition(std::make_pair(pos, status));
            });

            postion++;
        }

		std::vector<std::string> condNames = { QObject::tr("打印：").toStdString(), QObject::tr("已打印").toStdString(), QObject::tr("未打印").toStdString() };
		m_realCond.printStatus.SetContentName(condNames);
    }

    // 告警
    {
        std::vector<QPushButton*> buttons;
        buttons.push_back(ui->pushButton_alarmHave);
        buttons.push_back(ui->pushButton_alarmNo);


        // 与数据结构联动
        int postion = 0;
        for (auto& button : buttons)
        {
            m_realCond.alarmStatus.m_condition.test(postion) ? button->setChecked(true)
                : button->setChecked(false);

            QVariant pos(postion);
            button->setProperty("postion", pos);
            connect(button, &QPushButton::clicked, this, [=]()
            {
                int pos = button->property("postion").toInt();
                bool status = button->isChecked();
                m_realCond.alarmStatus.OnChangeCondition(std::make_pair(pos, status));
            });

            postion++;
        }

		std::vector<std::string> condNames = { QObject::tr("数据报警：").toStdString(), QObject::tr("有报警").toStdString(), QObject::tr("无报警").toStdString() };
		m_realCond.alarmStatus.SetContentName(condNames);
    }

	// 阳性阴性
	{
		std::vector<QPushButton*> buttons;
		buttons.push_back(ui->pushButton_resultPositive);
		buttons.push_back(ui->pushButton_resultNegtive);


		// 与数据结构联动
		int postion = 0;
		for (auto& button : buttons)
		{
			m_realCond.qualJudge.m_condition.test(postion) ? button->setChecked(true)
				: button->setChecked(false);

			QVariant pos(postion);
			button->setProperty("postion", pos);
			connect(button, &QPushButton::clicked, this, [=]()
			{
				int pos = button->property("postion").toInt();
				bool status = button->isChecked();
				m_realCond.qualJudge.OnChangeCondition(std::make_pair(pos, status));
			});

			postion++;
		}

		std::vector<std::string> condNames = { QObject::tr("定性结果：").toStdString(), QObject::tr("阳性").toStdString(), QObject::tr("阴性").toStdString() };
		m_realCond.qualJudge.SetContentName(condNames);
	}

    // 审核
    {
        std::vector<QPushButton*> buttons;
		TypeButton& auditType = m_typeButtons[0];
        buttons.push_back(ui->pushButton_verifAlready);
        buttons.push_back(ui->pushButton_verifNot);
		auditType.auditConds.buttons = buttons;
		auditType.auditConds.title = ui->label_13;

        // 与数据结构联动
        int postion = 0;
        for (auto& button : buttons)
        {
            m_realCond.auditStatus.m_condition.test(postion) ? button->setChecked(true)
                : button->setChecked(false);

            QVariant pos(postion);
            button->setProperty("postion", pos);
            connect(button, &QPushButton::clicked, this, [=]()
            {
                int pos = button->property("postion").toInt();
                bool status = button->isChecked();
                m_realCond.auditStatus.OnChangeCondition(std::make_pair(pos, status));
            });

            postion++;
        }

		std::vector<std::string> condNames = { QObject::tr("审核：").toStdString(), QObject::tr("已审核").toStdString(), QObject::tr("未审核").toStdString() };
		m_realCond.auditStatus.SetContentName(condNames);
    }

    // 稀释
    {
        std::vector<QPushButton*> buttons;
        buttons.push_back(ui->pushButton_diluNo);
        buttons.push_back(ui->pushButton_diluInner);
        // 仅仅是免疫的情况下隐藏
        if (devType == IMMUNE)
        {
            ui->pushButton_171->hide();
        }
		buttons.push_back(ui->pushButton_171);
        buttons.push_back(ui->pushButton_diluManual);


        // 与数据结构联动
        int postion = 0;
        for (auto& button : buttons)
        {
            m_realCond.dulitionType.m_condition.test(postion) ? button->setChecked(true)
                : button->setChecked(false);

            QVariant pos(postion);
            button->setProperty("postion", pos);
            connect(button, &QPushButton::clicked, this, [=]()
            {
                int pos = button->property("postion").toInt();
                bool status = button->isChecked();
                m_realCond.dulitionType.OnChangeCondition(std::make_pair(pos, status));
            });
            postion++;
        }

		std::vector<std::string> condNames;
		condNames = { QObject::tr("稀释状态：").toStdString(), QObject::tr("无").toStdString(), QObject::tr("机内稀释").toStdString(), QObject::tr("机内增量").toStdString(), QObject::tr("手工稀释").toStdString() };

		m_realCond.dulitionType.SetContentName(condNames);
    }

    // 发送
    {
        std::vector<QPushButton*> buttons;
        buttons.push_back(ui->pushButton_transAlready);
        buttons.push_back(ui->pushButton_transNot);


        // 与数据结构联动
        int postion = 0;
        for (auto& button : buttons)
        {
            m_realCond.sendLis.m_condition.test(postion) ? button->setChecked(true)
                : button->setChecked(false);

            QVariant pos(postion);
            button->setProperty("postion", pos);
            connect(button, &QPushButton::clicked, this, [=]()
            {
                int pos = button->property("postion").toInt();
                bool status = button->isChecked();
                m_realCond.sendLis.OnChangeCondition(std::make_pair(pos, status));
            });

            postion++;
        }

		std::vector<std::string> condNames = { QObject::tr("传输：").toStdString(), QObject::tr("已传输").toStdString(), QObject::tr("未传输").toStdString() };
		m_realCond.sendLis.SetContentName(condNames);
    }

	// ai识别
	{
		TypeButton& aiType = m_typeButtons[0];
		std::vector<QPushButton*> buttons;
		buttons.push_back(ui->pushButton_aiExistErr);
		buttons.push_back(ui->pushButton_aiNoErr);

		aiType.aiConds.title = ui->label_127;
		// 与数据结构联动
		int postion = 0;
		for (auto& button : buttons)
		{
			// modifybug0011224 by wuht
			aiType.aiConds.buttons.push_back(button);
			m_realCond.aiCheck.m_condition.test(postion) ? button->setChecked(true)
				: button->setChecked(false);

			QVariant pos(postion);
			button->setProperty("postion", pos);
			connect(button, &QPushButton::clicked, this, [=]()
			{
				int pos = button->property("postion").toInt();
				bool status = button->isChecked();
				m_realCond.aiCheck.OnChangeCondition(std::make_pair(pos, status));
			});

			postion++;
		}

		std::vector<std::string> condNames = { QObject::tr("视觉识别：").toStdString(), QObject::tr("有异常").toStdString(), QObject::tr("无异常").toStdString() };
		m_realCond.aiCheck.SetContentName(condNames);
	}

    auto time = m_realCond.examinationTime.m_condition;
    connect(ui->test_start_time, &QDateEdit::dateChanged, this, [=]()
    {
        auto time = m_realCond.examinationTime.m_condition;
        std::string startTime = (ui->test_start_time->getDateTimeStr(true)).toStdString();
        m_realCond.examinationTime.OnChangeCondition(std::make_pair(startTime, time.second));
    });

    connect(ui->test_stop_time, &QDateEdit::dateChanged, this, [=]()
    {
        auto time = m_realCond.examinationTime.m_condition;
        std::string stopTime = (ui->test_stop_time->getDateTimeStr()).toStdString();
        m_realCond.examinationTime.OnChangeCondition(std::make_pair(time.first, stopTime));
    });

	std::vector<std::string> condNames = { QObject::tr("检测日期：").toStdString() };
	m_realCond.examinationTime.SetContentName(condNames);

	//bug 0020291 by ldx add 20230915-modify by wuhongtao 20231010
	//按照最新需求不需要显示当前时间
	/*
	if (time.first.empty())
	{
		ui->test_start_time->setDateTime(QDateTime::currentDateTime());
	}
	else
	{
		ui->test_start_time->setDateTimeStr(QString::fromStdString(time.first));
	}
	emit ui->test_start_time->dateChanged(QDate());

	if (time.second.empty())
	{
		ui->test_stop_time->setDateTime(QDateTime::currentDateTime());
	}
	else
	{
		ui->test_stop_time->setDateTimeStr(QString::fromStdString(time.second));
	}
	emit ui->test_stop_time->dateChanged(QDate());
	*/

	ui->test_start_time->setDateTimeStr(QString::fromStdString(time.first));
	ui->test_stop_time->setDateTimeStr(QString::fromStdString(time.second));
	emit ui->test_start_time->dateChanged(QDate());
	emit ui->test_stop_time->dateChanged(QDate());

    // 样本号
    auto range = m_realCond.sampleRange.m_condition;
    ui->sample_no_edit->setText(QString::fromStdString(range.first));
	// 不限制中文，特殊字符 bug0020311 modify by wuht
	ui->sample_no_edit->setMaxLength(12);
    //ui->sample_no_edit->setValidator(new QRegExpValidator(QRegExp(UI_REG_SAMPLE_NUM), this));
    ui->sample_no_edit_end->setText(QString::fromStdString(range.second));
	// 不限制中文，特殊字符 bug0020311 modify by wuht
	ui->sample_no_edit_end->setMaxLength(12);
    //ui->sample_no_edit_end->setValidator(new QRegExpValidator(QRegExp(UI_REG_SAMPLE_NUM), this));

    connect(ui->sample_no_edit, &QLineEdit::textChanged, this, [=](){ 
		QString strSampleStart = ui->sample_no_edit->text();
		std::string strEndSequence = m_realCond.sampleRange.m_condition.second;
		m_realCond.sampleRange.OnChangeCondition(std::make_pair(strSampleStart.toStdString(), strEndSequence));
    });

    connect(ui->sample_no_edit_end, &QLineEdit::textChanged, this, [=]() {
		QString strSampleEnd = ui->sample_no_edit_end->text();
		std::string strStartSequence = m_realCond.sampleRange.m_condition.first;
		m_realCond.sampleRange.OnChangeCondition(std::make_pair(strStartSequence, strSampleEnd.toStdString()));
    });

	condNames = { QObject::tr("样本号：").toStdString() };
	m_realCond.sampleRange.SetContentName(condNames);

    // 样本条码
    auto barcode = m_realCond.barcode.m_condition;
    ui->sample_bar_edit->setText(QString::fromStdString(barcode.first));
    ui->sample_bar_edit->setValidator(new QRegExpValidator(QRegExp(UI_REG_SAMPLE_BAR_ASCII), this)); // 条码限制
    connect(ui->sample_bar_edit, &QLineEdit::textChanged, this, [=]() {
		QString strSampleBarStart = ui->sample_bar_edit->text();
		std::string strEndBarcode = m_realCond.barcode.m_condition.second;
		m_realCond.barcode.OnChangeCondition(std::make_pair(strSampleBarStart.toStdString(), strEndBarcode));
    });
    ui->sample_barend_edit->setText(QString::fromStdString(barcode.second)); 
    ui->sample_barend_edit->setValidator(new QRegExpValidator(QRegExp(UI_REG_SAMPLE_BAR_ASCII), this));// 条码限制
    connect(ui->sample_barend_edit, &QLineEdit::textChanged, this, [=]() {
		QString strSampleBarEnd = ui->sample_barend_edit->text();
		std::string strStartBarcode = m_realCond.barcode.m_condition.first;
		m_realCond.barcode.OnChangeCondition(std::make_pair(strStartBarcode, strSampleBarEnd.toStdString()));
    });

	condNames = { QObject::tr("样本条码：").toStdString() };
	m_realCond.barcode.SetContentName(condNames);

    auto deviceSn = m_realCond.device.m_condition;
    auto deviceClassy = ui->comboBox;
    auto deiviceName = ui->comboBox_mudulename;
    SetDeivceComBoxInfo(deviceClassy, deiviceName);

    connect(deviceClassy, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        this, [=](int index)
    {
        deiviceName->clear();
        deiviceName->addItem("", "");
        auto devices = deviceClassy->itemData(index).toStringList();
        for (const auto& device : devices)
        {
            auto spDevice = CommonInformationManager::GetInstance()->GetDeviceInfo(device.toStdString());
            if (spDevice == Q_NULLPTR)
            {
                continue;
            }

			// 当ISE有两个模块的时候需要区分A和B
			if ((spDevice->deviceClassify == tf::AssayClassify::ASSAY_CLASSIFY_ISE) && (spDevice->moduleCount > 1))
			{
				// ISEA
				auto iseNameA = CommonInformationManager::GetDeviceName(device.toStdString(), 1);
				deiviceName->addItem(QString::fromStdString(iseNameA), QString::fromStdString(spDevice->deviceSN) + ",1");

				// ISEB
				auto iseNameB = CommonInformationManager::GetDeviceName(device.toStdString(), 2);
				deiviceName->addItem(QString::fromStdString(iseNameB), QString::fromStdString(spDevice->deviceSN) + ",2");
				continue;
			}

            auto name = QString::fromStdString(spDevice->groupName + spDevice->name);
            deiviceName->addItem(name, QString::fromStdString(spDevice->deviceSN));
        }

        deiviceName->setCurrentIndex(0);

        //若为免疫单机版默认选中第一行 by ldx 20230902 bug	0020295
        if (CommonInformationManager::GetInstance()->GetImisSingle())
        {
            deiviceName->setCurrentIndex(1);
        }
    });

    deviceClassy->setCurrentIndex(0);

    //若为免疫单机版默认选中第一行 by ldx 20230902 bug	0020295
    if (CommonInformationManager::GetInstance()->GetImisSingle())
    {
        deviceClassy->setCurrentIndex(1);
    }

    auto spDevice = CommonInformationManager::GetInstance()->GetDeviceInfo(deviceSn);
    if (spDevice != Q_NULLPTR)
    {
        QString strAssayClassfyName = ThriftEnumTrans::GetAssayClassfiyName(spDevice->deviceClassify);
        if (!strAssayClassfyName.isEmpty())
        {
            int index = deviceClassy->findText(strAssayClassfyName);
            deviceClassy->setCurrentIndex(index);
        }

		auto name = QString::fromStdString(spDevice->groupName + spDevice->name);
		if (!name.isEmpty())
		{
			int index = deiviceName->findText(name);
			deiviceName->setCurrentIndex(index);
		}
    }

    connect(deiviceName, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
        this, [=](int index)
    {
		auto deviceSn = deiviceName->itemData(index).toString().toStdString();
		std::string deviceType;
		auto spDevice = CommonInformationManager::GetInstance()->GetDeviceInfo(deviceSn);
		if (spDevice != Q_NULLPTR)
		{
            QString strAssayClassfyName = ThriftEnumTrans::GetAssayClassfiyName(spDevice->deviceClassify);
			if (!strAssayClassfyName.isEmpty())
			{
				deviceType = strAssayClassfyName.toStdString();
			}
		}

        m_realCond.device.OnChangeCondition(std::make_pair(deviceSn, deviceType));
    });

	condNames = { QObject::tr("模块：").toStdString() };
	m_realCond.device.SetContentName(condNames);

	condNames = { QObject::tr("项目名：").toStdString() };
	m_realCond.sampleAssays.SetContentName(condNames);
    auto assayCodes = m_realCond.sampleAssays.m_condition;
    QStringList assays;
    for (auto assayCode : assayCodes)
    {
        std::shared_ptr<tf::GeneralAssayInfo> spAssayInfo = CommonInformationManager::GetInstance()->GetAssayInfo(assayCode);
		// 判空处理
		if (spAssayInfo == Q_NULLPTR)
		{
			continue;
		}

		assays.append(QString::fromStdString(spAssayInfo->assayName));
    }
    SetAssayEdit(ui->assay_edit, assays.join(tr("、")));
	// modify by wuht for bug3448
	ui->assay_edit->setReadOnly(true);

    auto f = [&](int db)->boost::optional<tf::PatientInfo>
    {
        ::tf::PatientInfoQueryCond patienQryCond;
        patienQryCond.__set_id(db);
        ::tf::PatientInfoQueryResp patienQryResp;
        // 执行查询条件
        if (!DcsControlProxy::GetInstance()->QueryPatientInfo(patienQryResp, patienQryCond)
            || patienQryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS
            || patienQryResp.lstPatientInfos.empty())
        {
            ULOG(LOG_ERROR, "query patient Info failed!");
            return boost::none;
        }

        return patienQryResp.lstPatientInfos[0];
    };

    // 病历号-患者姓名
    auto medical = m_realCond.medicalCase.m_condition;
	auto condContent = m_realCond.medicalCase.m_condContent;
	auto condContentName = m_realCond.patientName.m_condContent;
	ui->patient_record_edit->setMaxLength(10);
    ui->patient_record_edit->setText("");
	ui->patient_name_edit->setMaxLength(50);
    ui->patient_name_edit->setText("");
    if (!medical.empty())
    {
		if (!condContent.empty())
		{
			auto contentString = QString::fromStdString(condContent);
			auto contents = contentString.split("：");
            ui->patient_record_edit->setText(contents.size() > 1 ? contents[1] : "");
		}

		if (!condContentName.empty())
		{
			auto contentString = QString::fromStdString(condContentName);
            auto contents = contentString.split("：");
            ui->patient_name_edit->setText(contents.size() > 1 ? contents[1] : "");
		}
    }

	condNames = { QObject::tr("患者ID：").toStdString() };
	m_realCond.medicalCase.SetContentName(condNames);
    connect(ui->patient_record_edit, &QLineEdit::textChanged, this, [=]()
    {
        auto patientId = ui->patient_record_edit->text().toStdString();
        auto patientname = ui->patient_name_edit->text().toStdString();
        auto data = std::make_pair(patientId, patientname);
        m_realCond.medicalCase.OnChangeCondition(data);
    });

	condNames = { QObject::tr("患者姓名：").toStdString() };
	m_realCond.patientName.SetContentName(condNames);
    connect(ui->patient_name_edit, &QLineEdit::textChanged, this, [=]()
    {
        auto patientId = ui->patient_record_edit->text().toStdString();
        auto patientname = ui->patient_name_edit->text().toStdString();
        auto data = std::make_pair(patientId, patientname);
        m_realCond.patientName.OnChangeCondition(data);
    });

    // 试剂批号
	ui->lineEditReagentLot->setMaxLength(50);
    ui->lineEditReagentLot->setText("");
    std::set<std::string> setReagentLots = m_realCond.reagentLots.m_condition;
	// modify bug0024958 by wuht
    //if (!setReagentLots.empty())
    {
        auto reagents = boost::algorithm::join(setReagentLots, ";");
        ui->lineEditReagentLot->setText(QString::fromStdString(reagents));
    }

	condNames = { QObject::tr("试剂批号：").toStdString() };
	m_realCond.reagentLots.SetContentName(condNames);
    connect(ui->lineEditReagentLot, &QLineEdit::textChanged, this, [=]()
    {
        QString strReagentLots = ui->lineEditReagentLot->text();
		// modify bug0024958 by wuht
        //if (!strReagentLots.isEmpty())
        {
            m_realCond.reagentLots.OnChangeCondition(strReagentLots.toStdString());
        }
    });
}

///
/// @brief 导入快捷筛选设置
///
///
/// @return 
///
/// @par History:
/// @li 5774/WuHongTao，2023年3月10日，新建函数
///
void QFilterSample::LoadCond()
{
    // 构造查询条件
    ::tf::DictionaryInfoQueryCond   qryCond;
    ::tf::DictionaryInfoQueryResp   qryResp;
    qryCond.__set_keyName(DKN_UI_WORKPAGE_FILTER_COND);
    //m_conditionXml.clear();

    // 查询
    if (!DcsControlProxy::GetInstance()->QueryDictionaryInfo(qryResp, qryCond)
        || qryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS
        || qryResp.lstDictionaryInfos.empty())
    {
        ULOG(LOG_ERROR, "QueryDictionaryInfo Failed");
        return;
    }

    if (qryResp.lstDictionaryInfos[0].value.empty())
    {
        ULOG(LOG_INFO, "value is empty");
        return;
    }

    if (!DecodeJson(m_filterConditions, qryResp.lstDictionaryInfos[0].value))
    {
        ULOG(LOG_ERROR, "DecodeJson WorkPageFilterCond Failed");
        return;
    }

    // 保存条件xml
    m_conditionXml = qryResp.lstDictionaryInfos[0].value;
}

void QFilterSample::Reset()
{
    // 快捷筛选
    for (auto& cond : m_filterConditions)
    {
        cond.Reset();
    }

    m_realCond.Reset();

	// 清除项目信息中的项目
	if (m_assaySelectDialog != Q_NULLPTR)
	{
		m_assaySelectDialog->ClearSelectedAssays();
	}
}

///
/// @brief 项目选择对应的槽函数
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月31日，新建函数
///
void QFilterSample::OnAssaySelect()
{
    if (m_assaySelectDialog != nullptr)
    {
        delete m_assaySelectDialog;
        m_assaySelectDialog = nullptr;
    }

    m_assaySelectDialog = new QGroupAssayPositionSettings(Q_NULLPTR, DlgTypeAssaySelect);
    connect(m_assaySelectDialog, SIGNAL(selected()), this, SLOT(OnSelectAssay()));

    QStringList lstAssay;
    TakeCurrentPageAssays(lstAssay);
    m_assaySelectDialog->UpdateButtonCheckStat(lstAssay);

	m_assaySelectDialog->show();
}

///
/// @brief 处理项目选择信息
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年6月1日，新建函数
///
void QFilterSample::OnSelectAssay()
{
	if (m_assaySelectDialog == nullptr || !m_currentSetPage)
	{
		return;
	}

    // 从选择框中获取选择按钮的项目
    QStringList lstAssayNames;
    std::set<int> setAssayCodes;
    m_assaySelectDialog->GetSelectedAssayNames(lstAssayNames, setAssayCodes);

	// 依次显示控件; 中文逗号分隔符
	QString assays = lstAssayNames.join(tr("、"));
    if (m_currentSetPage == 0)
    {
        SetAssayEdit(ui->assay_edit, assays); // m_currentSetPage ? "" : assays
        // 按样本排序的条件
        m_realCond.sampleAssays.OnChangeCondition(setAssayCodes);
        // 按项目排序的条件
        m_realCond.testItemAssay.OnChangeCondition(setAssayCodes);
        return;
    }

    static std::vector<QTextEdit*> assayNameShows = {
        ui->assay_edit_2,
        ui->assay_edit_3,
        ui->assay_edit_4,
        ui->assay_edit_5,
        ui->assay_edit_6
    };

	int index = m_currentSetPage.value() - 1;
    if ((index < assayNameShows.size()) && index >= 0)
    {
        SetAssayEdit(assayNameShows[index], assays);
        // 按样本排序的条件
        m_filterConditions[index].sampleAssays.OnChangeCondition(setAssayCodes);
        // 按项目排序的条件
        m_filterConditions[index].testItemAssay.OnChangeCondition(setAssayCodes);
    }
}

///
/// @brief 保存筛选参数
///
///
/// @par History:
/// @li 5774/WuHongTao，2022年5月31日，新建函数
///
bool QFilterSample::OnSaveParameter(bool isPageChaned)
{
    bool ret = false;
    auto index = ui->tabWidget->currentIndex();
    // 检查名字是否有重复(仅在快捷筛选的时候做)
    if (index != 0 || isPageChaned)
    {
        auto judgeIndex = (m_preSetPage && isPageChaned)? m_preSetPage: m_currentSetPage;
        if (judgeIndex
            && judgeIndex.value() < m_filterConditions.size()
            && judgeIndex.value() >= 0)
        {
            auto condition = m_filterConditions[judgeIndex.value()];
            // 当前的快捷条件不允许为空
            if (condition.GetCondNumber().first == 0)
            {
                TipDlg(tr("提示"), tr("必须至少选择一个筛选条件")).exec();
                return ret;
            }

            // 筛选名称检测
            if (condition.name.empty())
            {
                TipDlg(tr("提示"), tr("快捷筛选名称不能为空，请重新设置！")).exec();
                return ret;
            }

            emit enableFilter(judgeIndex.value(), m_filterConditions[judgeIndex.value()].enable);
        }

        //auto currentIndex = 0;
        std::set<std::string> names;
        for (auto& condition : m_filterConditions)
        {
            if (names.count(condition.name))
            {
                TipDlg(tr("提示"), tr("名字有重复，不能保存")).exec();
                // 恢复修改之前的
                InItPageAttribute();
                return ret;
            }
            names.insert(condition.name);

            // 样本序号的限制
            {
                auto startSeq = condition.sampleRange.m_condition.first;
                auto endSeq = condition.sampleRange.m_condition.second;
				if (!startSeq.empty() && !endSeq.empty())
				{
					if (stringutil::IsPureDigit(startSeq) && stringutil::IsPureDigit(endSeq)
						&& (boost::multiprecision::cpp_int(endSeq) < boost::multiprecision::cpp_int(startSeq)))
					{
						TipDlg(tr("提示"), tr("开始样本号大于结束样本号")).exec();
						InItPageAttribute();
						return ret;
					}
				}
            }

            // 条码的限制
            {
                auto startBar = condition.barcode.m_condition.first;
                auto endBar = condition.barcode.m_condition.second;
                // 若起始条码都不为空
                if (!startBar.empty() && !endBar.empty())
                {                  
                    if (stringutil::IsPureDigit(startBar) && stringutil::IsPureDigit(endBar)
                        && (boost::multiprecision::cpp_int(endBar) < boost::multiprecision::cpp_int(startBar)))
                    {
                        TipDlg(tr("提示"), tr("开始条码大于结束条码")).exec();
                        InItPageAttribute();
                        return ret;
                    }
                }

            }

            // 样本检测时间
            {
                auto startTime = condition.examinationTime.m_condition.first;
                auto endTime = condition.examinationTime.m_condition.second;
                if (startTime.empty() ^ endTime.empty())
                {
                    TipDlg(tr("提示"), tr("必须同时设置开始和结束时间")).exec();
                    InItPageAttribute();
                    return ret;
                }

                if (startTime > endTime)
                {
                    TipDlg(tr("提示"), tr("检测日期结束时间早于开始时间，请重新设置！")).exec();
                    InItPageAttribute();
                    return ret;
                }
            }
        }
    }

    if (index == 0 && !isPageChaned)
    {
        // 当前的条件不允许为空
        if (m_realCond.GetCondNumber().first == 0)
        {
            TipDlg(tr("提示"), tr("必须至少选择一个筛选条件")).exec();
            return ret;
        }

        auto startSeq = m_realCond.sampleRange.m_condition.first;
        auto endSeq = m_realCond.sampleRange.m_condition.second;
		if (!startSeq.empty() && !endSeq.empty())
		{
			if (stringutil::IsPureDigit(startSeq) && stringutil::IsPureDigit(endSeq)
				&& (boost::multiprecision::cpp_int(endSeq) < boost::multiprecision::cpp_int(startSeq)))
			{
				TipDlg(tr("提示"), tr("开始样本号大于结束样本号")).exec();
				InItPageAttribute();
				return ret;
			}
		}

        // 条码的限制
        {
            auto startBar = m_realCond.barcode.m_condition.first;
            auto endBar = m_realCond.barcode.m_condition.second;
            // 若起始条码都不为空
            if (!startBar.empty() && !endBar.empty())
            {
                if (stringutil::IsPureDigit(startBar) && stringutil::IsPureDigit(endBar)
                    && (boost::multiprecision::cpp_int(endBar) < boost::multiprecision::cpp_int(startBar)))
                {
                    TipDlg(tr("提示"), tr("开始条码大于结束条码")).exec();
                    InitPageCurrent();
                    return ret;
                }
            }
        }

        auto startTime = m_realCond.examinationTime.m_condition.first;
        auto endTime = m_realCond.examinationTime.m_condition.second;
        if (startTime.empty() ^ endTime.empty())
        {
            TipDlg(tr("提示"), tr("必须同时设置开始和结束时间")).exec();
            InitPageCurrent();
            return ret;
        }

        if (startTime > endTime)
        {
            TipDlg(tr("提示"), tr("检测日期结束时间早于开始时间，请重新设置！")).exec();
            InitPageCurrent();
            return ret;
		}

		m_currentFastPage = boost::none;
		emit finished();
	}
    else
    {
        auto indexInner = index - 1;
        if (indexInner < PAGEMAX && indexInner >= 0)
        {
            // 刚好更同一快捷筛选页面
            if (m_currentFastPage && m_currentFastPage.value() == indexInner)
            {
                emit finished();
            }
//             else
//             {
//                 m_currentFastPage = indexInner;
//             }

            // 快捷按钮是否显示
            emit enableFilter(indexInner, m_filterConditions[indexInner].enable);
            // 快捷按钮的名称
            emit changeFilterName(indexInner, QString::fromStdString(m_filterConditions[indexInner].name));

        }
        else
        {
            m_currentFastPage = boost::none;
        }
    }

    std::string xml;
    if (!Encode2Json(xml, m_filterConditions))
    {
        return ret;
    }

    // 更新条件
    m_conditionXml = xml;

    tf::DictionaryInfo di;
    di.__set_keyName(DKN_UI_WORKPAGE_FILTER_COND);
    di.__set_value(xml);

    // 修改数据字典信息
    if (!DcsControlProxy::GetInstance()->ModifyDictionaryInfo(di))
    {
        return ret;
    }

    return true;
}

void QFilterSample::OnPageSetChanged()
{
    if (!DictionaryQueryManager::GetPageset(m_workSet))
    {
        ULOG(LOG_ERROR, "Failed to get workpage set.");
        return;
    }

	bool isAi = false;
	// 只有在联机模式下才有AI
	if (DictionaryQueryManager::GetInstance()->GetPipeLine())
	{
		isAi = m_workSet.aiRecognition;
	}

	auto fsetButton = [](bool isVisable, typeItem& buttons)
	{
		if (isVisable)
		{
			if (buttons.title != Q_NULLPTR)
			{
				buttons.title->show();
			}

			for (auto& button : buttons.buttons)
			{
				button->show();
			}
		}
		else
		{
			if (buttons.title != Q_NULLPTR)
			{
				buttons.title->hide();
			}

			for (auto& button : buttons.buttons)
			{
				button->hide();
			}
		}
	};

	for (auto& typeData : m_typeButtons)
	{
		fsetButton(isAi, typeData.aiConds);
		fsetButton(m_workSet.print, typeData.printConds);
		fsetButton(m_workSet.audit, typeData.auditConds);
	}
}

void QFilterSample::AssignRealSampleRange()
{
    QString strSampleBegin = ui->sample_no_edit->text();
    QString strSampleEnd = ui->sample_no_edit_end->text();
    if (strSampleBegin.isEmpty() && strSampleEnd.isEmpty())
    {
        return;
    }
    m_realCond.sampleRange.OnChangeCondition(std::make_pair(strSampleBegin.toStdString(), strSampleEnd.toStdString()));
}

void QFilterSample::AssignRealBarRange()
{
    QString strBarBegin = ui->sample_bar_edit->text();
    QString strBarEnd = ui->sample_barend_edit->text();
    if (strBarBegin.isEmpty() && strBarEnd.isEmpty())
    {
        return;
    }
    m_realCond.barcode.OnChangeCondition(std::make_pair(strBarBegin.toStdString(), strBarEnd.toStdString()));
}


void QFilterSample::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter)
    {
        return;
    }
}

bool OrderCondition::Judge(int postion)
{
    if (!m_coreData->__isset.sampleType || !m_coreData->__isset.stat)
    {
        return false;
    }

    switch (postion)
    {
        // 常规订单
        case 0:
        {
            if (m_condition.test(postion)
                && (m_coreData->sampleType == ::tf::SampleType::SAMPLE_TYPE_PATIENT)
                && (!m_coreData->stat))
            {
                return true;
            }

            return false;
        }
        break;
        // 急诊订单
        case 1:
        {
            if (m_condition.test(postion)
                && (m_coreData->sampleType == ::tf::SampleType::SAMPLE_TYPE_PATIENT)
                && (m_coreData->stat))
            {
                return true;
            }

            return false;
        }
        break;
        // 校准订单
        case 2:
        {
            if (m_condition.test(postion)
                && (m_coreData->sampleType == ::tf::SampleType::SAMPLE_TYPE_CALIBRATOR))
            {
                return true;
            }

            return false;
        }
        break;
        // 质控订单
        case 3:
        {
            if (m_condition.test(postion)
                && (m_coreData->sampleType == ::tf::SampleType::SAMPLE_TYPE_QC))
            {
                return true;
            }

            return false;
        }
        break;

        default:
            return false;
            break;
    }
}

bool StatusCondition::Judge(int postion)
{
    auto spSampleInfo = m_coreData.first;
    auto spTestItemInfo = m_coreData.second;

    // 样本类型的状态
    if (spSampleInfo != Q_NULLPTR)
    {
        switch (postion)
        {
            // 待测
            case 0:
            {
                if (m_condition.test(postion)
                    && spSampleInfo->__isset.status
                    && (spSampleInfo->status == ::tf::SampleStatus::SAMPLE_STATUS_PENDING))
                {
                    return true;
                }

                return false;
            }
            break;
            // 检测中
            case 1:
            {
                if (m_condition.test(postion)
                    && spSampleInfo->__isset.status
                    && (spSampleInfo->status == ::tf::SampleStatus::SAMPLE_STATUS_TESTING))
                {
                    return true;
                }

                return false;
            }
            break;
            // 已完成
            case 2:
            {
                if (m_condition.test(postion)
                    && spSampleInfo->__isset.status
                    && (spSampleInfo->status == ::tf::SampleStatus::SAMPLE_STATUS_TESTED))
                {
                    return true;
                }

                return false;
            }
            break;
            // 已传输
//             case 3:
//             {
//                 if (m_condition.test(postion)
//                     && (spSampleInfo->__isset.uploaded && spSampleInfo->uploaded))
//                 {
//                     return true;
//                 }
// 
//                 return false;
//             }
//             break;

            default:
                return false;
                break;
        }
    }
    else if (spTestItemInfo != Q_NULLPTR)
    {
        switch (postion)
        {
            // 待测
            case 0:
            {
                if (m_condition.test(postion)
                    && spTestItemInfo->__isset.status
                    && (spTestItemInfo->status == ::tf::TestItemStatus::TEST_ITEM_STATUS_PENDING))
                {
                    return true;
                }

                return false;
            }
            break;
            // 检测中
            case 1:
            {
                if (m_condition.test(postion)
                    && spTestItemInfo->__isset.status
                    && (spTestItemInfo->status == ::tf::TestItemStatus::TEST_ITEM_STATUS_TESTING))
                {
                    return true;
                }

                return false;
            }
            break;
            // 已完成
            case 2:
            {
                if (m_condition.test(postion)
                    && spTestItemInfo->__isset.status
                    && (spTestItemInfo->status == ::tf::TestItemStatus::TEST_ITEM_STATUS_TESTED))
                {
                    return true;
                }

                return false;
            }
            break;
            // 已传输
//             case 3:
//             {
//                 // 找不到对应的样本
//                 auto spSample = DataPrivate::Instance().GetSampleInfoByDb(spTestItemInfo->sampleInfoId);
//                 if (spSample == Q_NULLPTR)
//                 {
//                     return false;
//                 }
// 
//                 if (m_condition.test(postion)
//                     && (spSample->__isset.uploaded && spSample->uploaded))
//                 {
//                     return true;
//                 }
// 
//                 return false;
//             }
//             break;

            default:
                return false;
                break;
        }
    }

    return false;
}

bool QualJudgeCondition::Judge(int postion)
{
	auto spSampleInfo = m_coreData.first;
	auto spTestItemInfo = m_coreData.second;

	auto IsNegative = [](const std::shared_ptr<::tf::TestItem>& spTestItem)->bool
	{
		if (spTestItem == nullptr)
		{
			return false;
		}

		if (!spTestItem->__isset.firstQualitativeJudge)
		{
			return false;
		}

		if (spTestItem->firstQualitativeJudge == tf::QualJudge::type::Negative)
		{
			return true;
		}

		if (!spTestItem->__isset.retestQualitativeJudge)
		{
			return false;
		}

		if (spTestItem->retestQualitativeJudge == tf::QualJudge::type::Negative)
		{
			return true;
		}

		return false;
	};

	auto IsPositive = [](const std::shared_ptr<::tf::TestItem>& spTestItem)->bool
	{
		if (spTestItem == nullptr)
		{
			return false;
		}

		if (!spTestItem->__isset.firstQualitativeJudge)
		{
			return false;
		}

		if (spTestItem->firstQualitativeJudge == tf::QualJudge::type::Positive)
		{
			return true;
		}

		if (!spTestItem->__isset.retestQualitativeJudge)
		{
			return false;
		}

		if (spTestItem->retestQualitativeJudge == tf::QualJudge::type::Positive)
		{
			return true;
		}

		return false;
	};

	if (spSampleInfo)
	{
		bool isPositive = false;
		bool isNegative = false;
		for (auto& testItem : DataPrivate::Instance().GetSampleTestItems(spSampleInfo->id))
		{
			if (IsPositive(testItem))
			{
				isPositive = true;
			}

			if (IsNegative(testItem))
			{
				isNegative = true;
			}
		}

		switch (postion)
		{
			// 阴性
			case 1:
			{
				if (m_condition.test(postion) && isNegative)
				{
					return true;
				}

				return false;
			}
			break;
			// 阳性
			case 0:
			{
				if (m_condition.test(postion) && isPositive)
				{
					return true;
				}

				return false;
			}
			break;
			default:
				return false;
				break;
		}
	}
	else if (spTestItemInfo)
	{
		switch (postion)
		{
			// 阴性
			case 1:
			{
				if (m_condition.test(postion) && IsNegative(spTestItemInfo))
				{
					return true;
				}

				return false;
			}
			break;
			// 阳性
			case 0:
			{
				if (m_condition.test(postion) && IsPositive(spTestItemInfo))
				{
					return true;
				}

				return false;
			}
			break;
			default:
				return false;
				break;
		}
	}

	return false;
}

bool AICondition::Judge(int postion)
{
	switch (postion)
	{
		// 已发送
		case 0:
		{
			if (m_condition.test(postion))
			{
				return true;
			}

			return false;
		}
		break;
		// 未发送
		case 1:
		{
			if (m_condition.test(postion))
			{
				return true;
			}

			return false;
		}
		break;
		default:
			return false;
			break;
	}
}

bool SampleTypeCondition::Judge(int postion)
{
    if (!m_coreData->__isset.sampleSourceType)
    {
        return false;
    }

    switch (postion)
    {
        // 血清/血浆
        case 0:
        {
            if (m_condition.test(postion)
                && (m_coreData->sampleSourceType == tf::SampleSourceType::type::SAMPLE_SOURCE_TYPE_XQXJ))
            {
                return true;
            }

            return false;
        }
        break;
        // 全血
        case 1:
        {
            if (m_condition.test(postion)
                && (m_coreData->sampleSourceType == tf::SampleSourceType::type::SAMPLE_SOURCE_TYPE_QX))
            {
                return true;
            }

            return false;
        }
        break;
        // 尿液
        case 2:
        {
            if (m_condition.test(postion)
                && (m_coreData->sampleSourceType == tf::SampleSourceType::type::SAMPLE_SOURCE_TYPE_NY))
            {
                return true;
            }

            return false;
        }
        break;
        // 浆膜腔积液
        case 3:
        {
            if (m_condition.test(postion)
                && (m_coreData->sampleSourceType == tf::SampleSourceType::type::SAMPLE_SOURCE_TYPE_JMQJY))
            {
                return true;
            }

            return false;
        }
        break;
        // 脑脊液
        case 4:
        {
            if (m_condition.test(postion)
                && (m_coreData->sampleSourceType == tf::SampleSourceType::type::SAMPLE_SOURCE_TYPE_NJY))
            {
                return true;
            }

            return false;
        }
        break;
        // 其他
        case 5:
        {
            if (m_condition.test(postion)
                && (m_coreData->sampleSourceType == tf::SampleSourceType::type::SAMPLE_SOURCE_TYPE_OTHER))
            {
                return true;
            }

            return false;
        }
        break;

        default:
            return false;
            break;
    }
}

bool AuditCondition::Judge(int postion)
{
    if (!m_coreData->__isset.audit)
    {
        return false;
    }

    switch (postion)
    {
        // 审核
        case 0:
        {
            if (m_condition.test(postion) && m_coreData->audit)
            {
                return true;
            }

            return false;
        }
        break;
        // 未审核
        case 1:
        {
            if (m_condition.test(postion) && !m_coreData->audit)
            {
                return true;
            }

            return false;
        }
        break;
        default:
            return false;
            break;
    }
}

bool LISCondition::Judge(int postion)
{
    if (!m_coreData->__isset.uploaded)
    {
        return false;
    }

    switch (postion)
    {
        // 已发送
        case 0:
        {
            if (m_condition.test(postion) && m_coreData->uploaded)
            {
                return true;
            }

            return false;
        }
        break;
        // 未发送
        case 1:
        {
            if (m_condition.test(postion) && !m_coreData->uploaded)
            {
                return true;
            }

            return false;
        }
        break;
        default:
            return false;
            break;
    }
}

bool ReCheckCondition::Judge(int postion)
{
    auto spSampleInfo = m_coreData.first;
    auto spTestItemInfo = m_coreData.second;

	auto isHaveRecheck = [&](const std::shared_ptr<tf::TestItem>& spTestItemInfo)->bool
	{
		if ((spTestItemInfo->__isset.rerun && spTestItemInfo->rerun)
			|| (spTestItemInfo->__isset.lastTestResultKey && spTestItemInfo->lastTestResultKey.assayTestResultId > 0))
		{
			return true;
		}

		return false;
	};

    if (Q_NULLPTR != spSampleInfo)
    {
        bool hasRecheck = false;
        for (auto& testItem : DataPrivate::Instance().GetSampleTestItems(spSampleInfo->id))
        {
			// 复查(设置了rerun或者有了复查Key)
			hasRecheck = isHaveRecheck(testItem);
			if (hasRecheck)
			{
				break;
			}

			/*
			if ((testItem->__isset.rerun && testItem->rerun)
				|| (testItem->__isset.lastTestResultKey && testItem->lastTestResultKey.__isset.assayTestResultId))
			{
				hasRecheck = true;
				break;
			}*/
        }

        switch (postion)
        {
            // 有复查
            case 0:
            {
                if (m_condition.test(postion)
                    && hasRecheck)
                {
                    return true;
                }

                return false;
            }
            break;
            // 无复查
            case 1:
            {
                if (m_condition.test(postion) 
                    && !hasRecheck)
                {
                    return true;
                }

                return false;
            }
            break;
            default:
                return false;
                break;
        }
    }
    // 项目的情况（查看复查key有没有被设置）
    else if (Q_NULLPTR != spTestItemInfo)
    {
        switch (postion)
        {
            // 有复查
            case 0:
            {
                if (m_condition.test(postion)
					// modify bug0012341 by wuht
                    && isHaveRecheck(spTestItemInfo))
                {
                    return true;
                }

                return false;
            }
            break;
            // 无复查
            case 1:
            {
                if (m_condition.test(postion)
					// modify bug0012341 by wuht
                    && !isHaveRecheck(spTestItemInfo))
                {
                    return true;
                }

                return false;
            }
            break;
            default:
                return false;
                break;
        }
    }

    return false;
}

bool AlarmDoCondition::Judge(int postion)
{
    auto spSampleInfo = m_coreData.first;
    auto spTestItemInfo = m_coreData.second;

    if (Q_NULLPTR != spSampleInfo)
    {
        bool hasAlarm = false;
        for (auto& testItem : DataPrivate::Instance().GetSampleTestItems(spSampleInfo->id))
        {
            if (testItem->__isset.resultStatusCodes
                && !testItem->resultStatusCodes.empty())
            {
                hasAlarm = true;
                break;
            }

            if (testItem->__isset.retestResultStatusCodes
                && !testItem->retestResultStatusCodes.empty())
            {
                hasAlarm = true;
                break;
            }
        }

        switch (postion)
        {
            // 有报警
            case 0:
            {
                if (m_condition.test(postion) && hasAlarm)
                {
                    return true;
                }

                return false;
            }
            break;
            // 无报警
            case 1:
            {
                if (m_condition.test(postion) && !hasAlarm)
                {
                    return true;
                }

                return false;
            }
            break;
            default:
                return false;
                break;
        }
    }
    else if (Q_NULLPTR != spTestItemInfo)
    {
        bool hasAlarm = false;
        if (spTestItemInfo->__isset.resultStatusCodes
            && !spTestItemInfo->resultStatusCodes.empty())
        {
            hasAlarm = true;
        }

        if (spTestItemInfo->__isset.retestResultStatusCodes
            && !spTestItemInfo->retestResultStatusCodes.empty())
        {
            hasAlarm = true;
        }

        switch (postion)
        {
            // 有报警
            case 0:
            {
                if (m_condition.test(postion) && hasAlarm)
                {
                    return true;
                }

                return false;
            }
            break;
            // 无报警
            case 1:
            {
                if (m_condition.test(postion) && !hasAlarm)
                {
                    return true;
                }

                return false;
            }
            break;
            default:
                return false;
                break;
        }
    }

    return false;
}

bool PrintCondition::Judge(int postion)
{
    switch (postion)
    {
        // 已打印
        case 0:
        {
            if (m_condition.test(postion) && m_coreData->printed)
            {
                return true;
            }

            return false;
        }
        break;
        // 未打印
        case 1:
        {
            if (m_condition.test(postion) && !m_coreData->printed)
            {
                return true;
            }

            return false;
        }
        break;
        default:
            return false;
            break;
    }
}

bool ExaminationCondition::Judge(std::pair<std::string, std::string>& parameter)
{
	auto startTime = TimeStringToPosixTime(parameter.first);
	// 条件时间不能被解析
	if (startTime.is_not_a_date_time())
	{
		return false;
	}

	boost::posix_time::ptime endTime;
	if (parameter.first == parameter.second)
	{
		boost::posix_time::time_duration duration(23,59,59,999999);
		endTime = startTime + duration;
	}
	else
	{
		endTime = TimeStringToPosixTime(parameter.second);
	}

	// 条件时间不能被解析
	if (endTime.is_not_a_date_time())
	{
		return false;
	}

    auto spSampleInfo = m_coreData.first;
    auto spTestItemInfo = m_coreData.second;

    std::vector<std::string> testTimes;
    if (Q_NULLPTR != spSampleInfo)
    {
        if (!spSampleInfo->__isset.endTestTime)
        {
            return false;
        }
        testTimes.push_back(spSampleInfo->endTestTime);

        // 若有复查完成时间，则显示复查完成时间
        if (spSampleInfo->__isset.endRetestTime)
        {
            testTimes.push_back(spSampleInfo->endRetestTime);
        }
    }
    else if (Q_NULLPTR != spTestItemInfo)
    {
        if (!spTestItemInfo->__isset.endTime)
        {
            return false;
        }
        testTimes.push_back(spTestItemInfo->endTime);

        // 若有复查完成时间，则显示复查完成时间
        if (spTestItemInfo->__isset.retestEndTime)
        {
            testTimes.push_back(spTestItemInfo->retestEndTime);
        }
    }


    for (auto& testTime : testTimes)
    {
		// 将时间转化为统一标准
		auto timeNow = TimeStringToPosixTime(testTime);
		if (timeNow.is_not_a_date_time())
		{
			return false;
		}

        if (timeNow >= startTime && timeNow <= endTime)
        {
            return true;
        }
    }

    return false;
}

bool SampleNumberCondition::Judge(std::pair<std::string, std::string>& parameter)
{
	// 获取样本参数
	auto spSampleInfo = m_coreData.first;

	// 只有一个样本号的时候，直接判断是否包含
	if (parameter.first.empty())
	{
		return (spSampleInfo->seqNo.find(parameter.second) != std::string::npos) ? true : false;
	}

	if (parameter.second.empty())
	{
		return (spSampleInfo->seqNo.find(parameter.first) != std::string::npos) ? true : false;
	}

	bool isDigital = true;
	// 当有两个序号的时候判断是否全是数字，不是则判断是否包含
	if (!stringutil::IsPureDigit(parameter.first))
	{
		isDigital = false;
	}

	if (!stringutil::IsPureDigit(parameter.second))
	{
		isDigital = false;
	}

	if (isDigital)
	{
		// 若序号不是数字，直接返回
		if (!stringutil::IsPureDigit(spSampleInfo->seqNo))
		{
			return false;
		}

        boost::multiprecision::cpp_int mySeqNo = boost::multiprecision::cpp_int(spSampleInfo->seqNo);
        boost::multiprecision::cpp_int minSeqNo = boost::multiprecision::cpp_int(parameter.first);
        boost::multiprecision::cpp_int maxSeqNo = boost::multiprecision::cpp_int(parameter.second);

		if (mySeqNo < minSeqNo)
		{
			return false;
		}

		if (mySeqNo > maxSeqNo)
		{
			return false;
		}

		return true;
	}
	else
	{
		if (spSampleInfo->seqNo.find(parameter.second) != std::string::npos)
		{
			return true;
		}

		if (spSampleInfo->seqNo.find(parameter.first) != std::string::npos)
		{
			return true;
		}

		return false;
	}
}

bool SampleBarCodeCondition::Judge(std::pair<std::string, std::string>& parameter)
{
    // 获取样本参数
    auto spSampleInfo = m_coreData.first;

    // 只有一个条码的时候，直接判断是否包含
    if (parameter.first.empty())
    {
        return (spSampleInfo->barcode.find(parameter.second) != std::string::npos) ? true : false;
    }

    if (parameter.second.empty())
    {
        return (spSampleInfo->barcode.find(parameter.first) != std::string::npos) ? true : false;
    }

    bool isDigital = true;
    // 当有两个条码的时候判断是否全是数字，不是则判断是否包含
    if (!stringutil::IsPureDigit(parameter.first))
    {
        isDigital = false;
    }

    if (!stringutil::IsPureDigit(parameter.second))
    {
        isDigital = false;
    }

    if (isDigital)
    {
        // 若条码不是数字，直接返回(非严格审查，可以允许barCode前面有0)
        if (!stringutil::IsPureDigit(spSampleInfo->barcode,false))
        {
            return false;
        }

		// 去掉barCode最前面的0
		auto realBarCode = spSampleInfo->barcode;
		boost::trim_left_if(realBarCode, boost::is_any_of("0"));

        boost::multiprecision::cpp_int myBarCode = boost::multiprecision::cpp_int(realBarCode);
        boost::multiprecision::cpp_int minBarCode = boost::multiprecision::cpp_int(parameter.first);
        boost::multiprecision::cpp_int maxBarCode = boost::multiprecision::cpp_int(parameter.second);

        if (myBarCode < minBarCode)
        {
            return false;
        }

        if (myBarCode > maxBarCode)
        {
            return false;
        }

        return true;
    }
    else
    {
        if (spSampleInfo->barcode.find(parameter.second) != std::string::npos)
        {
            return true;
        }

        if (spSampleInfo->barcode.find(parameter.first) != std::string::npos)
        {
            return true;
        }

        return false;
    }
}

void DeviceCondition::OnChangeCondition(const std::pair<std::string, std::string>& parameter)
{
	m_condition = parameter.first;
	if (parameter.first.empty())
	{
		m_condNumber = 0;
	}
	else
	{
		m_condNumber = 1;
	}

	auto tmpParameter = QString::fromStdString(parameter.first).split(",");
	if (tmpParameter.empty())
	{
		QString deviceName;
		auto spDevice = CommonInformationManager::GetInstance()->GetDeviceInfo(parameter.first);
		if (spDevice != Q_NULLPTR)
		{
			deviceName = QString::fromStdString(spDevice->groupName + spDevice->name);
		}

		const std::string name = deviceName.toStdString();
		// 获取条件信息
		m_condContent = GenCondContent(std::make_pair(parameter.second, name));
	}
	else
	{
		auto deviceSn = tmpParameter[0];
		std::string name;
		if (tmpParameter.size() > 1)
		{
			bool ok = false;
			int moduleIndex = tmpParameter[1].toInt(&ok);
			if (!ok)
			{
				moduleIndex = 1;
			}
			name = CommonInformationManager::GetDeviceName(deviceSn.toStdString(), moduleIndex);
		}
		else
		{
			name = CommonInformationManager::GetDeviceName(deviceSn.toStdString(), 1);
		}

		// 获取条件信息
		m_condContent = GenCondContent(std::make_pair(parameter.second, name));
	}
}

bool DeviceCondition::Judge(std::string& parameter)
{
    if (parameter.empty())
    {
        return true;
    }

	auto tmpParameter = QString::fromStdString(parameter).split(",");
	if (tmpParameter.empty())
	{
		// 参数保存的是deviceSn;
		if (m_coreData->__isset.deviceSN
			&& m_coreData->deviceSN == parameter)
		{
			return true;
		}
	}
	else
	{
		auto deviceSn = tmpParameter[0].toStdString();
		if (tmpParameter.size() > 1)
		{
			bool ok = false;
			int moduleIndex = tmpParameter[1].toInt(&ok);
			if (!ok)
			{
				moduleIndex = -1;
			}

			if (m_coreData->__isset.deviceSN
				&& m_coreData->deviceSN == deviceSn
				&& m_coreData->moduleIndex == moduleIndex)
			{
				return true;
			}
		}
		else
		{
			// 参数保存的是deviceSn;
			if (m_coreData->__isset.deviceSN
				&& m_coreData->deviceSN == deviceSn)
			{
				return true;
			}
		}
	}



    return false;
}

bool ReagentLotCondition::Judge()
{
    // 判断是否包含(此方法适用于多个试剂批号精确匹配)
    auto fContain = [&](std::set<std::string>& condition, std::set<std::string>& sourceData)->bool
    {
        // 不能全包含，直接错误
        if (condition.size() > sourceData.size())
        {
            return false;
        }

        // 求交集
        std::set<std::string> intersection;
        std::set_intersection(sourceData.begin(), sourceData.end(),
            condition.begin(), condition.end()
            , std::inserter(intersection, intersection.begin()));

        // 若交集等于条件的大小，说明被全包含
        if (intersection.size() == condition.size())
        {
            return true;
        }

        return false;
    };

	// 方案2模糊匹配
	auto fMatch = [&](std::set<std::string>& condition, std::set<std::string>& sourceData)->bool
	{
		if (condition.empty())
		{
			return false;
		}

		// 获取条件
        std::string cond = *(condition.begin());
		if (cond.empty())
		{
			return false;
		}

		// 只要归属的试剂列表中包含条件中的字符即可（模糊匹配）
		for (const auto& dataLot : sourceData)
		{
			// 只要有一个包含即可
			if (dataLot.find(cond) != std::string::npos)
			{
				return true;
			}
		}

		return false;
	};

    auto spSampleInfo = m_coreData.first;
    auto spTestItemInfo = m_coreData.second;
    if (Q_NULLPTR != spSampleInfo)
    {
        // 提取样本已使用的试剂批号
        std::set<std::string> usedReagentLots;
        for (auto& testItem : DataPrivate::Instance().GetSampleTestItems(spSampleInfo->id))
        {
            std::set<std::string> innerReagentLots;
            boost::split(innerReagentLots, testItem->reagentlots, boost::is_any_of(";"));
			usedReagentLots.insert(innerReagentLots.begin(), innerReagentLots.end());
		}

        return fMatch(m_condition, usedReagentLots);
    }
    else if (Q_NULLPTR != spTestItemInfo)
    {
        std::set<std::string> innerReagentLots;
        boost::split(innerReagentLots, spTestItemInfo->reagentlots, boost::is_any_of(";"));

        return fMatch(m_condition, innerReagentLots);
    }

    return false;
}

bool ReagentLotCondition::IsPass()
{
    // 若都没有数据，则返回
    if (m_coreData.first == Q_NULLPTR && m_coreData.second == Q_NULLPTR)
    {
        return false;
    }

    if (m_condition.empty())
    {
        return true;
    }

    return Judge();
}

void ReagentLotCondition::OnChangeCondition(const std::string& parameter)
{
	m_condition.clear();
	if (parameter.empty())
	{
		m_condNumber = 0;
		const std::string other = "";
		m_condContent = GenCondContent(std::make_pair(parameter, other));
		return;
	}

    // 提取试剂批号列表
    try
    {
        boost::split(m_condition, parameter, boost::is_any_of(";"));
		m_condNumber = 1;
		// 获取条件信息
		const std::string other = "";
		m_condContent = GenCondContent(std::make_pair(parameter, other));
    }
    catch (std::exception& e)
    {
        ULOG(LOG_INFO, "Reagent condition decode Error!");
    }
}

bool PatientCondition::Judge()
{
    // 如果为空，则表示全部都通过
    if (m_condition.empty())
    {
        return true;
    }

    // 是否包含病人信息
    if (m_condition.count(m_coreData->patientInfoId) > 0)
    {
        return true;
    }

    return false;
}

bool PatientCondition::IsPass()
{
    if (m_coreData == Q_NULLPTR)
    {
        return false;
    }

    if (m_condition.empty())
    {
        return true;
    }

    return Judge();
}

void PatientCondition::OnChangeCondition(const std::pair<std::string, std::string>& parameter)
{
    m_condNumber = 0;
    // 病里和姓名都是空则返回，清空条件
    if (parameter.first.empty() && parameter.second.empty())
    {
		m_condContent.clear();
        m_condition.clear();
        return;
    }

    m_condition.clear();
    // 查询病人信息
    ::tf::PatientInfoQueryCond patienQryCond;

    // 患者id
    if (!parameter.first.empty())
    {
        m_condNumber++;
		// 获取条件信息
		const std::string patientId = parameter.first;
		const std::string other = "";
		m_condContent = GenCondContent(std::make_pair(patientId, other));
		::tf::StringMatchCond matchCond;
		matchCond.__set_value(parameter.first);
		matchCond.__set_like(true);
        patienQryCond.__set_medicalRecordNo(matchCond);
    }

    // 患者姓名
    if (!parameter.second.empty())
    {
        m_condNumber++;
		// 获取条件信息
		const std::string patientIName = parameter.second;
		const std::string other = "";
		m_condContent = GenCondContent(std::make_pair(patientIName, other));
		::tf::StringMatchCond matchCond;
		matchCond.__set_value(parameter.second);
		matchCond.__set_like(true);
		patienQryCond.__set_name(matchCond);
    }

    ::tf::PatientInfoQueryResp patienQryResp;
    // 执行查询条件
    if (!DcsControlProxy::GetInstance()->QueryPatientInfo(patienQryResp, patienQryCond) 
        || patienQryResp.result != ::tf::ThriftResult::THRIFT_RESULT_SUCCESS)
    {
        ULOG(LOG_ERROR, "query patient Info failed!");
        return;
    }

    // 空的时候给一个永远查不到的值
    if (patienQryResp.lstPatientInfos.empty())
    {
        m_condition.insert(-1);
    }

    // 依次写入病人信息的db
    for (const auto& db : patienQryResp.lstPatientInfos)
    {
        m_condition.insert(db.id);
    }

    // 条件改变
    m_isChanged = true;
}

bool DulitionCondition::IsDulition(const TestItemPointer& testItem, bool isIncrease)
{
	tf::SuckVolType::type jugeType = tf::SuckVolType::type::SUCK_VOL_TYPE_DEC;
	// 判断是否机内增量
	if (isIncrease)
	{
		jugeType = tf::SuckVolType::type::SUCK_VOL_TYPE_INC;
	}

	if (testItem->__isset.suckVolType
		&& (testItem->suckVolType == jugeType)
		&& testItem->__isset.dilutionFactor
		&& (testItem->dilutionFactor == 1))
	{
		return true;
	}

	// 倍数稀释
	else if (testItem->__isset.dilutionFactor
		&& testItem->dilutionFactor > 1
		&& !isIncrease)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool DulitionCondition::Judge(int postion)
{
    switch (postion)
    {
        // 未稀释
        case 0:
		{
			// modify bug0011200 by wuht
			// 对于生化来说，常量稀释就是未稀释
			if (m_condition.test(0))
			{
				// 是否机外稀释
				bool isPreDultion = (m_coreData->__isset.preDilutionFactor && m_coreData->preDilutionFactor > 1) ? true : false;
				// 未稀释（常量）
				if (!isPreDultion
					&& m_coreData->__isset.suckVolType
					&& m_coreData->suckVolType == tf::SuckVolType::type::SUCK_VOL_TYPE_STD
					&& m_coreData->__isset.dilutionFactor
					&& (m_coreData->dilutionFactor == 1))
				{
					return true;
				}
			}

			return false;
		}
		break;
		// 机外稀释（手工稀释）
        case 3:
        {
			// modify bug0011200 by wuht
            // 是否机外稀释
            bool isPreDultion = (m_coreData->__isset.preDilutionFactor && m_coreData->preDilutionFactor > 1)? true: false;
            if (m_condition.test(3))
            {
				return isPreDultion;
            }

            return false;
        }
        break;
        // 机内稀释
        case 1:
        {
            if (m_condition.test(postion)
                && IsDulition(m_coreData, false))
            {
                return true;
            }

            return false;
        }
		// 机内增量
		case 2:
		{
			if (m_condition.test(postion)
				&& IsDulition(m_coreData, true))
			{
				return true;
			}

			return false;
		}
        break;
        default:
            return false;
            break;
    }
}

void AssayCondition::OnChangeCondition(const std::set<int>& parameter)
{
	m_condition = parameter;
	m_condNumber = 1;
	if (parameter.empty())
	{
		m_condNumber = 0;
	}

	QStringList assayCodeNames;
	for (auto assayCode : m_condition)
	{
		// 计算项目，bug0011157 modify by wuht
		bool isCalc = CommonInformationManager::GetInstance()->IsCalcAssay(assayCode);
		if (isCalc)
		{
			auto spAssay = CommonInformationManager::GetInstance()->GetCalcAssayInfo(assayCode);
			if (spAssay == Q_NULLPTR)
			{
				continue;
			}

			assayCodeNames.push_back(QString::fromStdString(spAssay->name));
		}
		// 普通项目
		else
		{
			auto spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(assayCode);
			if (spAssay == Q_NULLPTR)
			{
				continue;
			}

			assayCodeNames.push_back(QString::fromStdString(spAssay->assayName));
		}
	}

	// modify bug0011232 by wuht
	const std::string condFirst = assayCodeNames.join("/").toStdString();
	const std::string other = "";
	m_condContent = GenCondContent(std::make_pair(condFirst, other));
}

bool AssayCondition::Judge(std::set<int>& parameter)
{
    if (parameter.empty())
    {
        return true;
    }

    // 获取样本对应的项目类型的列表（项目编号列表）
    auto smapleAssayCodes = DataPrivate::Instance().GetSampleAssayCodes(m_coreData->id);
    // 若要求的项目类型数目大于样本项目类型数据，则肯定不能全包含直接返回false
    if (m_condition.size() > smapleAssayCodes.size())
    {
        return false;
    }

    // 求交集
    std::set<int> intersection;
    std::set_intersection(smapleAssayCodes.begin(), smapleAssayCodes.end(),
    m_condition.begin(), m_condition.end()
    , std::inserter(intersection, intersection.begin()));

    // 若交集等于条件的大小，说明被全包含
    if (intersection.size() == m_condition.size())
    {
        return true;
    }

    return false;
}

bool AssayItemCondition::Judge(std::set<int>& parameter)
{
    if (parameter.empty())
    {
        return true;
    }

    if (m_condition.find(m_coreData->assayCode) != m_condition.end())
    {
        return true;
    }

    return false;
}

void AssayItemCondition::OnChangeCondition(const std::set<int>& parameter)
{
	m_condition = parameter;
	m_condNumber = 1;
	if (parameter.empty())
	{
		m_condNumber = 0;
	}

	QStringList assayCodeNames;
	for (auto assayCode : m_condition)
	{
		// 计算项目，bug0011157 modify by wuht
		bool isCalc = CommonInformationManager::GetInstance()->IsCalcAssay(assayCode);
		if (isCalc)
		{
			auto spAssay = CommonInformationManager::GetInstance()->GetCalcAssayInfo(assayCode);
			if (spAssay == Q_NULLPTR)
			{
				continue;
			}

			assayCodeNames.push_back(QString::fromStdString(spAssay->name));
		}
		// 普通项目
		else
		{
			auto spAssay = CommonInformationManager::GetInstance()->GetAssayInfo(assayCode);
			if (spAssay == Q_NULLPTR)
			{
				continue;
			}

			assayCodeNames.push_back(QString::fromStdString(spAssay->assayName));
		}
	}

	const std::string condFirst = assayCodeNames.join(";").toStdString();
	const std::string other = "";
	m_condContent = GenCondContent(std::make_pair(condFirst, other));
}

bool FilterConDition::IsPass(SamplePointer& spSample)
{
    oderType.LoadCoreData(spSample);
    if (!oderType.IsPass())
    {
        return false;
    }

    sampleType.LoadCoreData(spSample);
    if (!sampleType.IsPass())
    {
        return false;
    }

    auditStatus.LoadCoreData(spSample);
    if (!auditStatus.IsPass())
    {
        return false;
    }

	// 是否已传输
	sendLis.LoadCoreData(spSample);
	if (!sendLis.IsPass())
	{
		return false;
	}

	// ai识别
	aiCheck.LoadCoreData(spSample);
	if (!aiCheck.IsPass())
	{
		return false;
	}

    // 样本的状态
    auto data = std::make_pair(spSample, Q_NULLPTR);
    status.LoadCoreData(data);
    if (!status.IsPass())
    {
        return false;
    }

	// 免疫才有定性
	if (CommonInformationManager::GetInstance()->GetSoftWareType() == IMMUNE)
	{	
		qualJudge.LoadCoreData(data);
		if (!qualJudge.IsPass())
		{
			return false;
		}
	}

    reCheckStatus.LoadCoreData(data);
    if (!reCheckStatus.IsPass())
    {
        return false;
    }

    reagentLots.LoadCoreData(data);
    if (!reagentLots.IsPass())
    {
        return false;
    }

    alarmStatus.LoadCoreData(data);
    if (!alarmStatus.IsPass())
    {
        return false;
    }

    examinationTime.LoadCoreData(data);
    if (!examinationTime.IsPass())
    {
        return false;
    }

    sampleRange.LoadCoreData(data);
    if (!sampleRange.IsPass())
    {
        return false;
    }

    barcode.LoadCoreData(data);
    if (!barcode.IsPass())
    {
        return false;
    }

    medicalCase.LoadCoreData(spSample);
    if (!medicalCase.IsPass())
    {
        return false;
    }

    patientName.LoadCoreData(spSample);
    if (!patientName.IsPass())
    {
        return false;
    }

    printStatus.LoadCoreData(spSample);
    if (!printStatus.IsPass())
    {
        return false;
    }

    sampleAssays.LoadCoreData(spSample);
    if (!sampleAssays.IsPass())
    {
        return false;
    }

    // 项目处理(样本中的)
    auto fPass = [&](TestItemPointer& testItem)->bool
    {
        testItemAssay.LoadCoreData(testItem);
        if (!testItemAssay.IsPass())
        {
            return false;
        }
		// 当有项目的时候需要进一步判断是否与试剂编号匹配
		else
		{
			auto data = std::make_pair(Q_NULLPTR, testItem);
			reagentLots.LoadCoreData(data);
			if (!reagentLots.IsPass())
			{
				return false;
			}
		}

        dulitionType.LoadCoreData(testItem);
        if (!dulitionType.IsPass())
        {
            return false;
        }

        device.LoadCoreData(testItem);
        if (!device.IsPass())
        {
            return false;
        }

        return true;
    };

    // 对样本里面的每个项目进行判断，是否符合筛选要求
	auto testItems = DataPrivate::Instance().GetSampleTestItems(spSample->id);
    for ( auto& testItem : testItems)
    {
        // 只要其中有一个满足状态
        if (fPass(testItem))
        {
            return true;
        }
    }

	// 空的时候特殊处理
	if (testItems.size() == 0)
	{
		return true;
	}

    return false;
}

bool FilterConDition::IsPass(TestItemPointer& spTestItem)
{
    testItemAssay.LoadCoreData(spTestItem);
    if (!testItemAssay.IsPass())
    {
        return false;
    }

    dulitionType.LoadCoreData(spTestItem);
    if (!dulitionType.IsPass())
    {
        return false;
    }

    device.LoadCoreData(spTestItem);
    if (!device.IsPass())
    {
        return false;
    }

    auto spSample = DataPrivate::Instance().GetSampleInfoByDb(spTestItem->sampleInfoId);
    if (spSample == Q_NULLPTR)
    {
        return false;
    }

    oderType.LoadCoreData(spSample);
    if (!oderType.IsPass())
    {
        return false;
    }

    sampleType.LoadCoreData(spSample);
    if (!sampleType.IsPass())
    {
        return false;
    }

    auditStatus.LoadCoreData(spSample);
    if (!auditStatus.IsPass())
    {
        return false;
    }

	// 是否已传输
	sendLis.LoadCoreData(spSample);
	if (!sendLis.IsPass())
	{
		return false;
	}

	// ai识别
	aiCheck.LoadCoreData(spSample);
	if (!aiCheck.IsPass())
	{
		return false;
	}

    // 样本的状态
    auto data = std::make_pair(Q_NULLPTR, spTestItem);
    status.LoadCoreData(data);
    if (!status.IsPass())
    {
        return false;
    }

	if (CommonInformationManager::GetInstance()->GetSoftWareType() == IMMUNE)
	{
		qualJudge.LoadCoreData(data);
		if (!qualJudge.IsPass())
		{
			return false;
		}
	}

    reCheckStatus.LoadCoreData(data);
    if (!reCheckStatus.IsPass())
    {
        return false;
    }

    reagentLots.LoadCoreData(data);
    if (!reagentLots.IsPass())
    {
        return false;
    }

    alarmStatus.LoadCoreData(data);
    if (!alarmStatus.IsPass())
    {
        return false;
    }

    examinationTime.LoadCoreData(data);
    if (!examinationTime.IsPass())
    {
        return false;
    }

    // 样本的状态
    auto dataSample = std::make_pair(spSample, Q_NULLPTR);
    sampleRange.LoadCoreData(dataSample);
    if (!sampleRange.IsPass())
    {
        return false;
    }

    barcode.LoadCoreData(dataSample);
    if (!barcode.IsPass())
    {
        return false;
    }

    medicalCase.LoadCoreData(spSample);
    if (!medicalCase.IsPass())
    {
        return false;
    }

    patientName.LoadCoreData(spSample);
    if (!patientName.IsPass())
    {
        return false;
    }

    printStatus.LoadCoreData(spSample);
    if (!printStatus.IsPass())
    {
        return false;
    }

    return true;
}

///
/// @brief 获取条件数目
///
///
/// @return 条件数目
///
/// @par History:
/// @li 5774/WuHongTao，2023年3月15日，新建函数
///
std::pair<int, std::string> FilterConDition::GetCondNumber(bool isShowBySample) const
{
    int condNumber = 0;
	std::string condCondtent;

	// 检测日期
	condNumber = examinationTime.m_condNumber + condNumber;
	condCondtent += examinationTime.m_condContent;
	// 样本号
	condNumber = sampleRange.m_condNumber + condNumber;
	condCondtent += sampleRange.m_condContent;
	// 样本条码
	condNumber = barcode.m_condNumber + condNumber;
	condCondtent += barcode.m_condContent;
	// 模块
	condNumber = device.m_condNumber + condNumber;
	condCondtent += device.m_condContent;
	// 患者ID
	condNumber = medicalCase.m_condNumber + condNumber;
	condCondtent += medicalCase.m_condContent;
	// 患者姓名
	condNumber = patientName.m_condNumber + condNumber;
	condCondtent += patientName.m_condContent;
	// 试剂批号
	condNumber = reagentLots.m_condNumber + condNumber;
	condCondtent += reagentLots.m_condContent;
	// 订单类型
    condNumber = oderType.m_condNumber + condNumber;
	condCondtent += oderType.m_condContent;
	// 样本类型
	condNumber = sampleType.m_condNumber + condNumber;
	condCondtent += sampleType.m_condContent;
	// 稀释状态
	condNumber = dulitionType.m_condNumber + condNumber;
	condCondtent += dulitionType.m_condContent;
	// 状态
    condNumber = status.m_condNumber + condNumber;
	condCondtent += status.m_condContent;
	// 传输
	condNumber = sendLis.m_condNumber + condNumber;
	condCondtent += sendLis.m_condContent;
	// AI识别
	condNumber = aiCheck.m_condNumber + condNumber;
	condCondtent += aiCheck.m_condContent;
	// 复查
	condNumber = reCheckStatus.m_condNumber + condNumber;
	condCondtent += reCheckStatus.m_condContent;
	// 审核
    condNumber = auditStatus.m_condNumber + condNumber;
	condCondtent += auditStatus.m_condContent;

	// 免疫才有定性
	if ((CommonInformationManager::GetInstance()->GetSoftWareType() & IMMUNE) != 0)
	{
		// 定性
		condNumber = qualJudge.m_condNumber + condNumber;
		condCondtent += qualJudge.m_condContent;
	}

	// 打印
	condNumber = printStatus.m_condNumber + condNumber;
	condCondtent += printStatus.m_condContent;
	// 数据报警
    condNumber = alarmStatus.m_condNumber + condNumber;
	condCondtent += alarmStatus.m_condContent;
	// 定性结果
	// 项目名
	if (isShowBySample)
	{
		condNumber = sampleAssays.m_condNumber + condNumber;
		condCondtent += sampleAssays.m_condContent;
	}
	else
	{
		condNumber = testItemAssay.m_condNumber + condNumber;
		condCondtent += testItemAssay.m_condContent;
	}

    return std::make_pair(condNumber, condCondtent);
}

