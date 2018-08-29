#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget* parent) :
    QWidget(parent),
    ui(new Ui::Widget), filenamelist_data_model(NULL), is_exec_prev(false)
{
    ui->setupUi(this);
    this->connect(ui->ChooseDirPushButton, SIGNAL(clicked()), this, SLOT(getDirPathSlot()));
    this->connect(ui->goThroughDirPushButton, SIGNAL(clicked()), this, SLOT(getFileNamesFromDirSlot()));
    this->connect(ui->PreviewButton, SIGNAL(clicked()), this, SLOT(previewFNR_EffectSlot()));
    this->connect(ui->ExecuteButton, SIGNAL(clicked()), this, SLOT(executeReplaceSlot()));
    setWindowFlags(windowFlags() & ~Qt::WindowMaximizeButtonHint);  // 禁止最大化按钮
    setFixedSize(this->width(), this->height());
    init_table();


}

void Widget::getDirPathSlot()
{
    QString dirName = ui->DirPathLineEdit->text().trimmed();

    QDir dir(dirName);

    if(!dir.exists() || dirName.isEmpty())
    {
        dirName = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
        qDebug() << dirName;
    }

    dirName = QFileDialog::getExistingDirectory(this, "选择文件夹", dirName);
    if(false == dirName)
    {
        // do not anything
    }
    else
    {
        // get the dirname and foreach
        qDebug() << dirName;
        ui->DirPathLineEdit->setText(dirName);

    }
}

void Widget::getFileNamesFromDirSlot()
{
    QString dirName = ui->DirPathLineEdit->text().trimmed();
    QDir dir(dirName);

    if(!dir.exists() || dirName.isEmpty())
    {
        qWarning("Cannot find the directory");
        return;
    }
    dir.setFilter(QDir::Files | QDir::Hidden | QDir::NoSymLinks);
    dir.setSorting(QDir::Name | QDir::Time | QDir::Reversed);
    QFileInfoList filelist;
    if(ui->isOpenSubDirCheckBox->isChecked())
    {
        QDirIterator dir_iterator(dir, QDirIterator::Subdirectories);
        while(dir_iterator.hasNext())
        {
            dir_iterator.next();
            filelist.append(dir_iterator.fileInfo());
        }
    }
    else
    {

        filelist = dir.entryInfoList();

    }
    int size = filelist.size();
    if(ui->isClearPrimaryListCheckBox->isChecked())
        filenamelist_data_model->removeRows(0, filenamelist_data_model->rowCount(QModelIndex()),
                                            QModelIndex());

    int init_row = filenamelist_data_model->rowCount(QModelIndex()) - 1;
    init_row = (init_row < 0) ? 0 : init_row;
    filenamelist_data_model->insertRows(init_row, size, QModelIndex());
    size += init_row;
    qDebug() << init_row << size;
    for(int i = init_row; i < size; ++i)
    {

        QFileInfo fileInfo = filelist.at(i - init_row);
        qDebug() << fileInfo.filePath();
        QStringList filename_list;
        filename_list << fileInfo.fileName() << "" << "" << fileInfo.filePath();
        filenamelist_data_model->setData(filenamelist_data_model->index(i, 0, QModelIndex()), filename_list.value(0));
        filenamelist_data_model->setData(filenamelist_data_model->index(i, 1, QModelIndex()), filename_list.value(1));
        filenamelist_data_model->setData(filenamelist_data_model->index(i, 2, QModelIndex()), filename_list.value(2));
        filenamelist_data_model->setData(filenamelist_data_model->index(i, 3, QModelIndex()), filename_list.value(3));
//        qDebug() << filename_list;
    }



}

void Widget::previewFNR_EffectSlot()
{
    //get the text of searchLineEdit and replaceLineEdit
    QString search_words = ui->SearchKeyWordsLineEdit->text();
    if(search_words.isEmpty())
        return;
    QString replace_words = ui->ReplaceKeyWordsLineEdit->text();
    QStandardItem* filename_item;
    QStandardItem* filepath_item;
    //遍历当前model
    int count = filenamelist_data_model->rowCount();
    for(int i = 0; i < count; i++)
    {

        filename_item = filenamelist_data_model->item(i, 0);
        filepath_item = filenamelist_data_model->item(i, 3);
        if(filename_item && filepath_item)
        {
            QString filename = filename_item->text();
//            QString filepath = filepath_item->text();
//            qDebug() << filename << filepath;
            QString preveiw_replace_string;
            if(ui->isSuportedRegexCheckBox->isChecked())
                preveiw_replace_string = filename.replace(QRegExp(search_words), replace_words);
            else
                preveiw_replace_string = filename.replace(search_words, replace_words);
            filenamelist_data_model->setData(filenamelist_data_model->index(i, 1, QModelIndex()), preveiw_replace_string);
        }
    }
    is_exec_prev = true;

}

void Widget::executeReplaceSlot()
{
    QString search_words = ui->SearchKeyWordsLineEdit->text();
    if(search_words.isEmpty())
        return;
    int ret = QMessageBox::warning(this, "警告",
                                   "文件名资环操作不可逆！\n建议先执行预览，是否继续？",
                                   QMessageBox::Yes | QMessageBox::Cancel,
                                   QMessageBox::Cancel);
    if(ret == QMessageBox::Cancel)
        return;
    int count = filenamelist_data_model->rowCount();
    if(count <= 0)
        return;

    QStandardItem* filename_item;
    QStandardItem* fileprev_item;
    fileprev_item = filenamelist_data_model->item(0, 1);

    if(!is_exec_prev)
    {
        this->previewFNR_EffectSlot();
        is_exec_prev = false;
    }

    fileprev_item = filenamelist_data_model->item(0, 1);

    for(int i = 0; i < count; i++)
    {
        filename_item = filenamelist_data_model->item(i, 0);
        fileprev_item = filenamelist_data_model->item(i, 1);
        if(filename_item && fileprev_item)
        {

            QString filename = filename_item->text();
            QString fileprev = fileprev_item->text();
            if(filename == fileprev)
            {
                filenamelist_data_model->setData(filenamelist_data_model->index(i, 2, QModelIndex()), "无替换字符");
            }
            else
            {
                QStandardItem* filepath_item = filenamelist_data_model->item(i, 3);
                if(filepath_item)
                {

                    QFileInfo file_info(filepath_item->text());
                    if(file_info.isFile())
                    {
                        QDir file_dir = file_info.dir();
                        QString file_replaced_path = file_dir.absolutePath() + "/" + fileprev;
                        qDebug() << file_replaced_path;
                        if(QFile::rename(file_info.absoluteFilePath(), file_replaced_path))
                        {
                            filenamelist_data_model->setData(filenamelist_data_model->index(i, 2, QModelIndex()), "替换成功");
                            filenamelist_data_model->setData(filenamelist_data_model->index(i, 0, QModelIndex()), fileprev);
                            filenamelist_data_model->setData(filenamelist_data_model->index(i, 3, QModelIndex()), file_replaced_path);
                        }
                        else
                        {
                            filenamelist_data_model->setData(filenamelist_data_model->index(i, 2, QModelIndex()), "失败,文件被占用");
                        }

                    }
                    else
                    {
                        filenamelist_data_model->setData(filenamelist_data_model->index(i, 2, QModelIndex()), "路径不存在");
                    }

                }



            }
        }
    }
    if(!is_exec_prev)
    {
        for(int i = 0; i < count; i++)
        {
            filenamelist_data_model->setData(filenamelist_data_model->index(i, 1, QModelIndex()), "");
        }
    }
    is_exec_prev = false;

}

void Widget::fileNameListTableViewMenuSLot(const QPoint& pos)
{
    QModelIndex index = ui->FileNameListTableView->indexAt(pos);

    if(index.isValid())
    {
        fileListPopMenu->exec(QCursor::pos());
        qDebug() << index;
    }
}

void Widget::removeSelectionSlot()
{

    QModelIndexList selected_rows = ui->FileNameListTableView->selectionModel()->selectedRows();
//    ui->FileNameListTableView->model()->removeRows(selected_rows);
//    qDebug() << selected_rows;
    //先排序 这里运用插入排序
    int list_count = selected_rows.count();
    for(int i = 1; i < list_count; i++)
    {
        QModelIndex e = selected_rows[i];
        int j ;//保存元素应该插入的位置
        for(j = i; j > 0 && selected_rows[j - 1].row() > e.row(); j--)
        {
            selected_rows[j] = selected_rows[j - 1];
        }
        selected_rows[j] = e;
    }
    for(int i = list_count - 1; i >= 0; --i)
    {
        ui->FileNameListTableView->model()->removeRow(selected_rows[i].row());
    }

}


void Widget::init_table()
{
    ui->FileNameListTableView->setShowGrid(true);
    ui->FileNameListTableView->setEditTriggers(QTableView::NoEditTriggers);
    ui->FileNameListTableView->setSelectionBehavior(QTableView::SelectRows);
    ui->FileNameListTableView->setSelectionMode(QTableView::ExtendedSelection);
    ui->FileNameListTableView->setGridStyle(Qt::DotLine);
//    ui->FileNameListTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    filenamelist_data_model = new QStandardItemModel(this);
    filenamelist_data_model->setColumnCount(4);
    filenamelist_data_model->setHeaderData(0, Qt::Horizontal, "原文件名");
    filenamelist_data_model->setHeaderData(1, Qt::Horizontal, "预览效果");
    filenamelist_data_model->setHeaderData(2, Qt::Horizontal, "执行状态");
    filenamelist_data_model->setHeaderData(3, Qt::Horizontal, "原文件路径");

//    ui->FileNameListTableView->horizontalHeader()->setSectionsMovable(true);
    ui->FileNameListTableView->setModel(filenamelist_data_model);
    int table_width = ui->FileNameListTableView->width() - 2;
    ui->FileNameListTableView->setColumnWidth(0, table_width / 3);
    ui->FileNameListTableView->setColumnWidth(1, table_width / 3);
    ui->FileNameListTableView->setColumnWidth(2, table_width / 3);
//    ui->FileNameListTableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->FileNameListTableView->setAlternatingRowColors(true);  // alternative colors
    QItemSelectionModel* selectionModel = new QItemSelectionModel(filenamelist_data_model);
    ui->FileNameListTableView->setSelectionModel(selectionModel);
    ui->FileNameListTableView->setColumnHidden(3, true);
    this->connect(selectionModel, SIGNAL(selectionChanged(QItemSelection, QItemSelection)), ui->FileNameListTableView,
                  SLOT(selectionChanged(QItemSelection, QItemSelection)));
    ui->FileNameListTableView->setContextMenuPolicy(Qt::CustomContextMenu);
    QObject::connect(ui->FileNameListTableView, SIGNAL(customContextMenuRequested(const QPoint&)), this, SLOT(fileNameListTableViewMenuSLot(const QPoint&)));

    fileListPopMenu = new QMenu(ui->FileNameListTableView);
    QAction* action_remove_selection = new QAction(fileListPopMenu);
    action_remove_selection->setText("移除选中");
    fileListPopMenu->addAction(action_remove_selection);
    QObject::connect(action_remove_selection, SIGNAL(triggered()), this, SLOT(removeSelectionSlot()));
}

Widget::~Widget()
{
    delete ui;
}
