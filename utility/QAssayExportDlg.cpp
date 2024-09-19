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
/// @file     QAssayExportDlg.cpp
/// @brief    项目导出，用户不可见，此功能内部使用
///
/// @author   8580/GongZhiQiang
/// @date     2023年8月10日
/// @version  0.1
///
/// @par Copyright(c):
///     2015 迈克医疗电子有限公司，All rights reserved.
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月10日，新建文件
///
///////////////////////////////////////////////////////////////////////////
#include "QAssayExportDlg.h"
#include "ui_QAssayExportDlg.h"
#include <QStandardItemModel>
#include <QCheckBox>
#include <QFileDialog>
#include <QTextStream>

#include "shared/uicommon.h"
#include "shared/tipdlg.h"
#include "shared/CommonInformationManager.h"
#include "src/common/Mlog/mlog.h"
#include "src/common/RapidjsonUtil/RapidjsonUtil.h"
#include "manager/UserInfoManager.h"
#include "thrift/ch/ChLogicControlProxy.h"

#define PARAM_TABLE_INDEX_ROLE                   (Qt::UserRole + 1)                     // 信息查询索引

QAssayExportDlg::QAssayExportDlg(QWidget *parent)
	: BaseDlg(parent),
	m_bInit(false)
{
    ui = new Ui::QAssayExportDlg();
	ui->setupUi(this);
	
	InitBeforeShow();
}

QAssayExportDlg::~QAssayExportDlg()
{

}

///
/// @brief 加载项目信息的数据
///
/// @param[in]  assayData  项目信息数据
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年2月24日，新建函数
///
void QAssayExportDlg::LoadAssayData(const std::vector<AssayListModel::StAssayListRowItem>& assayData)
{
	// 清空
	m_chAssaySpMap.clear();

	// 根据项目编号获取全部信息
	int mapIndex = 0;
	auto comManager = CommonInformationManager::GetInstance();
	for (auto assayIndex : assayData)
	{
		// 只支持生化
		if (assayIndex.assayClassify != tf::AssayClassify::ASSAY_CLASSIFY_CHEMISTRY)
		{
			continue;
		}

		// 过滤掉血清指数
		if (assayIndex.assayCode == ch::tf::g_ch_constants.ASSAY_CODE_SIND || 
			assayIndex.assayCode == ch::tf::g_ch_constants.ASSAY_CODE_L || 
			assayIndex.assayCode == ch::tf::g_ch_constants.ASSAY_CODE_H || 
			assayIndex.assayCode == ch::tf::g_ch_constants.ASSAY_CODE_I)
		{
			continue;
		}

		// 获取通用项目信息
		auto gAssayInfo = comManager->GetAssayInfo(assayIndex.assayCode);
		if (gAssayInfo == nullptr)
		{
			continue;
		}

		
		// 获取全部生化通用项目信息
		auto chGAInfos = comManager->GetAllChemistryAssayInfo(assayIndex.assayCode, static_cast<::tf::DeviceType::type>(assayIndex.model));
		for (auto spChGAI : chGAInfos)
		{
			// 判空
			if (spChGAI == nullptr)
			{
				continue;
			}

			// 获取当前版本全部生化特殊项目信息（所有样本）,必须指定版本
			auto chSAInfos = comManager->GetChemistrySpecialAssayInfo(assayIndex.assayCode, static_cast<::tf::DeviceType::type>(assayIndex.model), -1, spChGAI->version);
			for (auto spChSAI : chSAInfos)
			{
				// 判空
				if (spChSAI == nullptr)
				{
					continue;
				}

				// 添加进缓存
				ChAssayInfoSpStu t_ChAssayInfoSpStu;
				t_ChAssayInfoSpStu.spGA = gAssayInfo;
				t_ChAssayInfoSpStu.spChGA = spChGAI;
				t_ChAssayInfoSpStu.spChSA = spChSAI;
				m_chAssaySpMap.insert(mapIndex, t_ChAssayInfoSpStu);
				
				// 添加进表格 
				QString assayName = QString::fromStdString(gAssayInfo->assayName);
				QString sampleType = ConvertTfEnumToQString(static_cast<::tf::SampleSourceType::type>(spChSAI->sampleSourceType));
				QString version = QString::fromStdString(spChGAI->version);
				AddRowToTable(mapIndex, assayName, sampleType, version);

				mapIndex++;
			}
		}
	}
}

///
/// @brief
///     界面显示前初始化
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月10日，新建函数
///
void QAssayExportDlg::InitBeforeShow()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	SetTitleName(tr(u8"项目导出"));

	// 失能路径编辑框
	ui->edit_path->setEnabled(false);

	// 初始化表格
	m_assayTableModel = new QStandardItemModel();
	ui->tableView_assay->setModel(m_assayTableModel);

	QStringList headerTitle = { tr(""), tr("项目名称"), tr("样本类型"), tr("参数版本") };
	m_assayTableModel->setHorizontalHeaderLabels(headerTitle);

	ui->tableView_assay->verticalHeader()->setDefaultSectionSize(45);
	
	QCheckBox* chkBox = new QCheckBox(ui->tableView_assay);
	chkBox->setMinimumSize(20, 20);
	UiCommon::Instance()->AddCheckboxToTableView(ui->tableView_assay, chkBox);

	ui->tableView_assay->verticalHeader()->hide();
	ui->tableView_assay->setColumnWidth(COLUNM_ASSAY_CHECKBOX, 43);
	ui->tableView_assay->setColumnWidth(COLUNM_ASSAY_NAME, 200);
	ui->tableView_assay->setColumnWidth(COLUNM_ASSAY_SAMPLETYPE, 200);
	ui->tableView_assay->setColumnWidth(COLUNM_ASSAY_NAME, 200);

	// 单选，不可编辑
	ui->tableView_assay->setSelectionBehavior(QAbstractItemView::SelectRows);
	ui->tableView_assay->setSelectionMode(QAbstractItemView::SingleSelection);
	ui->tableView_assay->setEditTriggers(QAbstractItemView::NoEditTriggers);
	ui->tableView_assay->horizontalHeader()->setStretchLastSection(true);
}

///
/// @brief
///     界面显示后初始化
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月10日，新建函数
///
void QAssayExportDlg::InitAfterShow()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 初始化信号槽连接
	InitConnect();
}

///
/// @brief 初始化导出数据，并进行编码
///
///
/// @return true：成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年2月24日，新建函数
///
bool QAssayExportDlg::InitExportData()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
	// 遍历所有选中的项目，并添加进缓存，组装成目标数据结构
	QVector<int> indexVec;
	for (int rowIndex = 0; rowIndex < m_assayTableModel->rowCount(); rowIndex++)
	{
		// 获取是否选中
		QWidget* wid = qobject_cast<QWidget*>(ui->tableView_assay->indexWidget(m_assayTableModel->index(rowIndex, COLUNM_ASSAY_CHECKBOX)));
		QCheckBox* box = wid->findChild<QCheckBox*>();
		if (!box->isChecked())
		{
			continue;
		}

		// 获取索引
		auto infoIndex = m_assayTableModel->item(rowIndex, COLUNM_ASSAY_CHECKBOX)->data(PARAM_TABLE_INDEX_ROLE);
		if (infoIndex.isValid())
		{
			indexVec.push_back(infoIndex.toInt());
		}
	}

	// 勾选判断
	if (indexVec.empty())
	{
		TipDlg(tr("未勾选任何数据！")).exec();
		return false;
	}

	// 编码
	if (!EncodeParams(indexVec))
	{
		TipDlg(tr("编码失败！")).exec();
		return false;
	}

	return true;
}

///
/// @brief 编码参数
///
/// @param[in]  indexVec	界面勾选的数据<数据索引，表格行索引>
///
/// @return true：编码成功
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年2月24日，新建函数
///
bool QAssayExportDlg::EncodeParams(const QVector<int>& indexVec)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 生成JSON信息(项目名称，项目通道号，样本类型，版本号，设备类型，编码后的项目信息)
	auto EncodeToJson = [&](const std::string& assayName, const int& assayCode, 
		const int& sampleType, const std::string& version, const int& deviceType,
        const ::ch::tf::ChAssayInfoList& chTemp, bool isEncryption)->std::string{

        // 调用接口解析
        std::string jsonParam;
        std::string assayInfo = "";
        if (!ch::LogicControlProxy::EncodeChAssayInfo(assayInfo, chTemp, isEncryption) || assayInfo.empty())
        {
            ULOG(LOG_ERROR, "EncodeChAssayInfo Failed! assayCode=%d ", chTemp.gai.assayCode);
             return jsonParam;
        }

		// 根节点
		Document doc;
		doc.SetObject();
		Document::AllocatorType& allocator = doc.GetAllocator();

		doc.AddMember("assayName", { assayName.c_str(), allocator }, allocator);
		doc.AddMember("assayCode",assayCode, allocator);
		doc.AddMember("sampleType", sampleType, allocator);
		doc.AddMember("version", { version.c_str(), allocator }, allocator);
		doc.AddMember("deviceType", deviceType, allocator);
		doc.AddMember("assayInfo", { assayInfo.c_str(), allocator }, allocator);

		jsonParam = RapidjsonUtil::Rapidjson2String(doc);

		return jsonParam;
	};

	// 清空数据
	m_assayInfoJsonVec.clear();
    m_assayInfoEncryptionJsonVec.clear();

	// 遍历生成
	for (auto index : indexVec)
	{
		// 生化中查找
		if (m_chAssaySpMap.contains(index))
		{
			// 数据为空
			if (m_chAssaySpMap[index].IsInvalid())
			{
				continue;
			}
			
			// 根据解析接口的数据结构放入
			::ch::tf::ChAssayInfoList chTemp;
			chTemp.__set_gai(*m_chAssaySpMap[index].spGA);

			::ch::tf::ChAssayInfo chGAssay;
			chGAssay.__set_cgai(*m_chAssaySpMap[index].spChGA);
			chGAssay.__set_csais({ *m_chAssaySpMap[index].spChSA});

			chTemp.__set_cgais({ chGAssay });

			// 生成加密JSON
			std::string encryptionjsonStr = EncodeToJson(m_chAssaySpMap[index].spGA->assayName,
				m_chAssaySpMap[index].spGA->assayCode, m_chAssaySpMap[index].spChSA->sampleSourceType,
				m_chAssaySpMap[index].spChSA->version, m_chAssaySpMap[index].spChSA->deviceType,
                chTemp, true);

            m_assayInfoEncryptionJsonVec.push_back(QByteArray::fromStdString(std::move(encryptionjsonStr)));

            // 生成不加密JSON
            std::string jsonStr = EncodeToJson(m_chAssaySpMap[index].spGA->assayName,
                m_chAssaySpMap[index].spGA->assayCode, m_chAssaySpMap[index].spChSA->sampleSourceType,
                m_chAssaySpMap[index].spChSA->version, m_chAssaySpMap[index].spChSA->deviceType,
                chTemp, false);

            m_assayInfoJsonVec.push_back(QByteArray::fromStdString(std::move(jsonStr)));
		}
	}

	return true;
}

///
/// @brief
///     初始化信号槽连接
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月10日，新建函数
///
void QAssayExportDlg::InitConnect()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);
	// 退出
	connect(ui->btn_cancel, &QPushButton::clicked, this, [&] {this->close(); });

	// 导出
	connect(ui->btn_ensure, &QPushButton::clicked, this, &QAssayExportDlg::OnExportEnsureClicked);

	// 文件位置
	connect(ui->btn_exPathSel, &QPushButton::clicked, this, &QAssayExportDlg::OnExportPathBtnClicked);
}

///
/// @brief 往表格中添加一行数据
///
/// @param[in]  index		 查询索引
/// @param[in]  assayName    项目名称
/// @param[in]  sampleType   样本类型
/// @param[in]  version      参数版本
///
/// @return 
///
/// @par History:
/// @li 8580/GongZhiQiang，2024年2月24日，新建函数
///
void QAssayExportDlg::AddRowToTable(const int& index, const QString& assayName, const QString& sampleType, const QString& version)
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	// 模型指针判断
	if (m_assayTableModel == nullptr)
	{
		return;
	}

	int rowIndex = m_assayTableModel->rowCount();

	AddTextToTableView(m_assayTableModel, rowIndex, QAssayExportDlg::COLUNM_ASSAY_NAME, assayName);
	AddTextToTableView(m_assayTableModel, rowIndex, QAssayExportDlg::COLUNM_ASSAY_SAMPLETYPE, sampleType);
	AddTextToTableView(m_assayTableModel, rowIndex, QAssayExportDlg::COLUNM_ASSAY_VERSION, version);

	// 勾选框
	QWidget* wid = new QWidget(ui->tableView_assay);
	wid->setMinimumSize(44, 44);
	QCheckBox* box = new QCheckBox(wid);
	box->setMinimumSize(20, 20);
	box->move(6, 2);
	ui->tableView_assay->setIndexWidget(m_assayTableModel->index(rowIndex, QAssayExportDlg::COLUNM_ASSAY_CHECKBOX), wid);
	QStandardItem *item = new QStandardItem();
	m_assayTableModel->setItem(rowIndex, QAssayExportDlg::COLUNM_ASSAY_CHECKBOX, item);

	// 设置映射索引
	item->setData(index, PARAM_TABLE_INDEX_ROLE);

	connect(box, &QCheckBox::stateChanged, this, [item](int state) {

		// 参数检查
		if (item == nullptr)
		{
			return;
		}
		item->setData((state == Qt::Checked ? true : false), UI_ITEM_ROLE_SEQ_CHECKBOX_SORT);
	});
	
}

///
/// @brief
///     窗口显示事件
///
/// @param[in]  event  事件对象
///
/// @par History:
/// @li 4170/TangChuXian，2021年6月22日，新建函数
///
void QAssayExportDlg::showEvent(QShowEvent *event)
{
	// 让基类处理事件
	BaseDlg::showEvent(event);

	// 第一次显示时初始化
	if (!m_bInit)
	{
		m_bInit = true;
		InitAfterShow();
	}
}

///
/// @brief
///     导出文件位置选择
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月10日，新建函数
///
void QAssayExportDlg::OnExportPathBtnClicked()
{
	ULOG(LOG_INFO, "%s()", __FUNCTION__);

	QString filePtah = QFileDialog::getExistingDirectory(this, tr(u8"导出文件位置"), ("./"));
	ui->edit_path->setText(filePtah);

}

///
/// @brief
///     确认按钮选择
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月10日，新建函数
///
void QAssayExportDlg::OnExportEnsureClicked()
{
	ULOG(LOG_INFO, __FUNCTION__);

	// 检查路径是否设置
	if (ui->edit_path->text().isEmpty())
	{
		TipDlg(tr("导出路径为空！")).exec();
		return;
	}

	// 初始化生化导出数据
	if (!InitExportData())
	{
		return;
	}

	// 输出文件
	QString path = ui->edit_path->text();
	if (!ExportAssayParamsToFile(path))
	{
		TipDlg(tr("导出失败！")).exec();
		ULOG(LOG_ERROR, "ExportAssayParamsToFile Failed !");
		return;
	}

	// 导出成功
	TipDlg(tr("导出成功！")).exec();
	this->close();
}

///
/// @brief
///     导出参数结构体
///
/// @param[in]  filePath   文件路径
///
/// @return 成功返回true
///
/// @par History:
/// @li 8580/GongZhiQiang，2023年8月10日，新建函数
///
bool QAssayExportDlg::ExportAssayParamsToFile(const QString& filePath)
{
	ULOG(LOG_INFO, __FUNCTION__);

	// 路径判断
	if (filePath.isEmpty())
	{
		return false;
	}
    QString curDT = QDateTime::currentDateTime().toString("yyyyMMddhhmmss");

    // 输出加密文件
	QString encryptionFileDir = filePath + "/" + curDT + ".maccura";
	QFile encryptionFfile(encryptionFileDir);

	if (!encryptionFfile.open(QIODevice::WriteOnly))
	{
		ULOG(LOG_ERROR, "encryption file open Failed !");
		return false;
	}

	QTextStream encryptionOut(&encryptionFfile);
	encryptionOut.setCodec("UTF-8");
	for (const auto& json : m_assayInfoEncryptionJsonVec)
	{
        encryptionOut << json;
        encryptionOut << '\n';
	}

    encryptionFfile.close();

    // 输出不加密文件
    QString fileDir = filePath + "/" + curDT + ".json";
    QFile file(fileDir);

    if (!file.open(QIODevice::WriteOnly))
    {
        ULOG(LOG_ERROR, "file open Failed !");
        return false;
    }

    QTextStream out(&file);
	out.setCodec("UTF-8");
    for (const auto& json : m_assayInfoJsonVec)
    {
        out << json;
        out << '\n';
    }

    file.close();

	return true;
}
