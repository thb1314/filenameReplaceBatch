#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QFileInfo>
#include <QFileDialog>
#include <QDirIterator>
#include <QRegExp>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <QCursor>
#include <QPoint>
#include <QStandardPaths>

#if _MSC_VER >= 1600
#pragma execution_character_set("utf-8")
#endif
namespace Ui
{
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget* parent = 0);
    ~Widget();
private slots:
    void getDirPathSlot();
    void getFileNamesFromDirSlot();
    void previewFNR_EffectSlot();
    void executeReplaceSlot();
    void fileNameListTableViewMenuSLot(const QPoint& pos);
    void removeSelectionSlot();
private:
    QMenu* fileListPopMenu;
    Ui::Widget* ui;
    QStandardItemModel* filenamelist_data_model;
    bool is_exec_prev;
    void init_table(void);
};

#endif // WIDGET_H
