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

#include "dialogue.h"

#include <QTextCodec>
#include <QRegularExpression>

static const QRegularExpression mEditTextRegExp("^(?<name>[^:]+):( )*((\"(?<text1>.*)\")|('(?<text2>.*)'))$",
                                          QRegularExpression::CaseInsensitiveOption |
                                          QRegularExpression::DotMatchesEverythingOption);

Dialogue::Dialogue(QObject *parent) :
    Action(parent)
{
    init();
}

Dialogue::Dialogue(const QVariantMap & data, QObject *parent):
    Action(data, parent)
{
    init();
    loadInternal(data);
}

void Dialogue::init()
{
    setType(GameObjectMetaType::Dialogue);
    mCharacter = 0;
    mText = "";
    mAppend = false;
    setMouseClickOnFinish(true);
    setTextEditable(true);
    mSound = 0;
    mSoundVolume = 100;
    mTextSpeedEnabled = false;
    mTextSpeed = 50;
}

void Dialogue::loadData(const QVariantMap & data, bool internal)
{
    if (!internal)
        Action::loadData(data, internal);

    if (data.contains("character") && data.value("character").type() == QVariant::String) {
        Scene* scene = this->scene();
        Character* character = 0;
        QString charName = data.value("character").toString();
        if (scene)
            character = qobject_cast<Character*>(scene->object(charName));
        setCharacter(character);

        if (!mCharacter)
            setCharacterName(charName);
    }

    if (data.contains("text") && data.value("text").type() == QVariant::String) {
        setText(data.value("text").toString());
    }

    if (data.contains("append") && data.value("append").type() == QVariant::Bool) {
        setAppend(data.value("append").toBool());
    }

    if (data.contains("sound") && data.value("sound").type() == QVariant::String) {
        setSound(data.value("sound").toString());
    }

    if (data.contains("soundVolume") && data.value("soundVolume").canConvert(QVariant::Int)) {
        setSoundVolume(data.value("soundVolume").toInt());
    }

    //Used when exchanging data between resource and clones
    if (data.contains("textSpeedEnabled") && data.value("textSpeedEnabled").type() == QVariant::Bool) {
        setTextSpeedEnabled(data.value("textSpeedEnabled").toBool());
    }

    if (data.contains("textSpeed") && data.value("textSpeed").canConvert(QVariant::Int)) {
        setTextSpeedEnabled(true);
        setTextSpeed(data.value("textSpeed").toInt());
    }
}

void Dialogue::setCharacter(Character *character)
{
    if (mCharacter == character)
        return;

    removeCharacter();
    mCharacter = character;
    mCharacterName = "";

    if (mCharacter) {
        connect(mCharacter, SIGNAL(destroyed()), this, SLOT(onCharacterDestroyed()));
        connect(mCharacter, SIGNAL(nameChanged(const QString&)), this, SLOT(onCharacterNameChanged(const QString&)));
        mCharacterName = mCharacter->name();
    }

    //update DialogueBox or TextBox
    updateTextBox();
    notify("character", characterName());
}

void Dialogue::setCharacter(const QString& name)
{
    if (!mCharacter || mCharacter->name() != name) {
        Character* character = findCharacter(name);
        setCharacter(character);
    }

    if (mCharacterName == name)
        return;

    if (!mCharacter) {
        mCharacterName = name;
        updateTextBox();
        notify("character", characterName());
    }
}

Character* Dialogue::character()
{
    return mCharacter;
}

void Dialogue::setCharacterName(const QString & name)
{
    if (mCharacterName == name)
        return;

    removeCharacter();
    mCharacterName = name;

    //update dialoguebox if any
    updateTextBox();
    notify("character", characterName());
}

QString Dialogue::characterName() const
{
    //the character's name can change after being assigned to this action
    //so mCharacterName could be outdated.
    if (mCharacter)
        return mCharacter->objectName();
    return mCharacterName;
}

void Dialogue::setText(const QString & text)
{
    mText = text;
    updateTextBox();
    notify("text", mText);
}

QString Dialogue::text()
{
    return mText;
}

QString Dialogue::displayText() const
{
    QString text("");
    if (! characterName().isEmpty())
        text += characterName() + ": ";


    text += '"' + mText + '"';

    return text;
}

QVariantMap Dialogue::toJsonObject(bool internal) const
{
    QVariantMap object = Action::toJsonObject(internal);

    if (mCharacter)
        object.insert("character", mCharacter->objectName());
    else if (! mCharacterName.isEmpty())
        object.insert("character", mCharacterName);

    if (mAppend)
        object.insert("append", mAppend);

    if (mSound) {
        object.insert("sound", mSound->name());
        object.insert("soundVolume", mSoundVolume);
    }

    if (mTextSpeedEnabled) {
        object.insert("textSpeed", mTextSpeed);
    }

    object.insert("text", mText);
    return object;
}

void Dialogue::onCharacterDestroyed()
{
    mCharacter = 0;
}

void Dialogue::onCharacterNameChanged(const QString & name)
{
    emit dataChanged();
}

void Dialogue::updateTextBox()
{
    if (!isActive())
        return;

    Object* object = sceneObject();
    if (! object)
        return;

    QColor textColor, nameColor;
    QString name = mCharacterName;

    if (mCharacter) {
        textColor = mCharacter->textColor();
        nameColor = mCharacter->nameColor();
        name = mCharacter->name();
    }

    DialogueBox* dialogueBox = qobject_cast<DialogueBox*>(object);
    if (dialogueBox) {
        dialogueBox->setSpeakerName(name);
        dialogueBox->setText(mText);
        dialogueBox->setTextColor(textColor);
        dialogueBox->setSpeakerNameColor(nameColor);
    }
    else {
        TextBox* textBox = qobject_cast<TextBox*>(object);
        if (textBox) {
            textBox->setPlaceholderTextColor(textColor);
            textBox->setPlaceholderText(mText);
        }
    }
}

void Dialogue::restoreTextBox()
{
    if (!isActive())
        return;

    Object* object = sceneObject();
    if (! object)
        return;

    DialogueBox* dialogueBox = qobject_cast<DialogueBox*>(object);
    if (dialogueBox) {
       dialogueBox->setText("", "");
    }
    else {
        TextBox* textBox = qobject_cast<TextBox*>(object);
        if (textBox) {
            textBox->setPlaceholderText("");
        }
    }
}

void Dialogue::loadSceneObject()
{
    Action::loadSceneObject();
    updateTextBox();
}

void Dialogue::restoreSceneObject()
{
    Action::restoreSceneObject();
    restoreTextBox();
}

bool Dialogue::append() const
{
    return mAppend;
}

void Dialogue::setAppend(bool append)
{
    mAppend = append;
    notify("append", mAppend);
}

void Dialogue::removeCharacter()
{
    if (mCharacter) {
        mCharacter->disconnect(this);
        mCharacter = 0;
    }
}

QString Dialogue::editText() const
{
    QString name = characterName();
    if (!name.isEmpty())
        return QString("%1: \"%2\"").arg(name).arg(mText);
    return mText;
}

void Dialogue::setEditText(const QString& content)
{
    QString text = content.trimmed();
    QRegularExpression regexp = mEditTextRegExp;
    QRegularExpressionMatch match = regexp.match(text);

    if (match.hasMatch()) {
        text = match.captured("text1");
        if (text.isEmpty())
            text = match.captured("text2");
        setCharacter(match.captured("name"));
        setText(text);
    }
    else {
        setCharacterName("");
        setText(text);
    }
}

Character* Dialogue::findCharacter(const QString & name)
{
    Scene* scene = this->scene();
    if (!scene)
        return 0;

    QList<Object*> objects = scene->objects(GameObjectMetaType::Character);
    foreach (Object* obj, objects) {
        if (obj && obj->name() == name)
            return qobject_cast<Character*>(obj);
    }

    return 0;
}

Sound* Dialogue::sound() const
{
    return mSound;
}

void Dialogue::setSound(Sound * sound)
{
    if (mSound == sound)
        return;

    if (mSound)
        mSound->disconnect(this);

    mSound = sound;
    QString soundName;

    if (mSound) {
        connect(mSound, SIGNAL(destroyed(GameObject*)), this, SLOT(onSoundDestroyed(GameObject*)));
        soundName = mSound->name();
    }

    notify("sound", soundName);
}

void Dialogue::setSound(const QString& soundName)
{
    GameObject* obj = ResourceManager::instance()->object(soundName);
    Sound* sound = qobject_cast<Sound*>(obj);
    setSound(sound);
}

void Dialogue::onSoundDestroyed(GameObject* sound)
{
    mSound = 0;
}

int Dialogue::soundVolume() const
{
    return mSoundVolume;
}

void Dialogue::setSoundVolume(int volume)
{
    if (mSoundVolume == volume)
        return;

    mSoundVolume = volume;
    notify("soundVolume", mSoundVolume);
}

bool Dialogue::textSpeedEnabled() const
{
    return mTextSpeedEnabled;
}

void Dialogue::setTextSpeedEnabled(bool enabled)
{
    if (mTextSpeedEnabled == enabled)
        return;

    mTextSpeedEnabled = enabled;
    notify("textSpeedEnabled", mTextSpeedEnabled);
}

int Dialogue::textSpeed() const
{
    return mTextSpeed;
}

void Dialogue::setTextSpeed(int value)
{
    if (mTextSpeed == value)
        return;

    mTextSpeed = value;
    notify("textSpeed", mTextSpeed);
}
