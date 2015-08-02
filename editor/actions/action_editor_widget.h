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

#ifndef ACTION_EDITOR_WIDGET_H
#define ACTION_EDITOR_WIDGET_H

#include <QLineEdit>

#include "gameobject_editorwidget.h"
#include "action.h"

class Action;
class Object;

class ActionEditorWidget : public GameObjectEditorWidget
{
    Q_OBJECT

    QLineEdit* mNameEdit;
    QList<Object*> mObjects;

public:
    explicit ActionEditorWidget(QWidget *parent = 0);
    virtual void updateData(GameObject*);
    Action* action();
    void setAction(Action*);
    ActionEditorWidget* copy();

signals:

protected:
    Action* mAction;

private slots:
    void onNameEdited(const QString& name);
    void actionDestroyed();

};

#endif // ACTION_PROPERTIES_WIDGET_H
