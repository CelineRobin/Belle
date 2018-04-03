/* Copyright (C) 2012-2014 Carlos Pais
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ACTIONS_VIEW_H
#define ACTIONS_VIEW_H

#include <QListView>
#include <QStyledItemDelegate>

#include "action.h"
#include "actions_model.h"

class ActionsModel;

class ActionsViewDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    explicit ActionsViewDelegate(QObject* parent = 0);
    virtual void paint (QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index ) const;
    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual QWidget * createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual void setEditorData(QWidget *editor, const QModelIndex &index) const;
    virtual void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    virtual bool eventFilter(QObject *watched, QEvent *event);

private:
    bool mTextEditCursorAtEnd;
    int mTextEditCursorBlockPos;

    bool editorFlag(QWidget*, const char*) const;
};

class ActionsView : public QListView
{
    Q_OBJECT
public:
    explicit ActionsView(QWidget *parent = 0);
    void appendAction(Action*);
    void selectAction(Action*);
    void setCurrentAction(Action*);
    void selectActions(const QList<Action*>&);
    void addActionItem(Action*);
    bool eventFilter(QObject *watched, QEvent *event);

protected:
    virtual void dropEvent(QDropEvent *);
    QList<Action*> selectedActions() const;
    virtual void selectionChanged(const QItemSelection &, const QItemSelection &);
    bool canPaste() const;
    virtual bool edit(const QModelIndex &index, EditTrigger trigger, QEvent *event);

private:
    void pasteActionsAt(int, const QList<Action*>&, bool, bool select=true);

signals:
    void actionCopied(const QList<Action*>&);
    void actionCut(const QList<Action*>&);
    void actionPasted(const QList<Action*>&);
    void currentActionClicked(Action*);
    void currentActionChanged(Action*);

private slots:
    void onContextMenuRequested(const QPoint&);
    void onDeleteAction();
    void onItemClicked(const QModelIndex&);
    void onCopyAction();
    void onCutAction();
    void onPasteAction();
    void onEditorClosed(QWidget*, QAbstractItemDelegate::EndEditHint);
    void onWriteAction();
    void onModifyActionText();

private:
    QAction* mDeleteAction;
    QAction* mCopyAction;
    QAction* mCutAction;
    QAction* mPasteAction;
    ActionsModel* mActionsModel;
    Action* mWriteAction;
    bool mEditTextMode;
};

#endif // ACTIONS_VIEW_H
