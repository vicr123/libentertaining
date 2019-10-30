/****************************************
 *
 *   INSERT-PROJECT-NAME-HERE - INSERT-GENERIC-NAME-HERE
 *   Copyright (C) 2019 Victor Tran
 *
 *   This program is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * *************************************/
#include "gamepadbuttons.h"
#include <QIcon>
#include <QSvgRenderer>
#include <QPainter>
#include <QRegularExpression>
#include <QDebug>
#include <the-libs_global.h>

QIcon iconForButtonHelper(QString iconName, QColor tint) {
    QIcon icon;

    QSvgRenderer renderer(QStringLiteral(":/libentertaining/icons/%1.svg").arg(iconName));
    QList<QSize> sizes = {{16, 16}, {32, 32}, {48, 48}, {64, 64}, {128, 128}, {256, 256}};
    for (QSize size : sizes) {
        QImage px(size, QImage::Format_ARGB32);
        px.fill(Qt::transparent);
        QPainter painter(&px);
        renderer.render(&painter, QRect(QPoint(0, 0), size));
        painter.end();

        theLibsGlobal::tintImage(px, tint);
        icon.addPixmap(QPixmap::fromImage(px));
    }

    return icon;
}

const QMap<QGamepadManager::GamepadButton, QString> buttonToStringMapping = {
    {QGamepadManager::ButtonA, "[GAMEPAD_A]"},
    {QGamepadManager::ButtonB, "[GAMEPAD_B]"},
    {QGamepadManager::ButtonX, "[GAMEPAD_X]"},
    {QGamepadManager::ButtonY, "[GAMEPAD_Y]"},
    {QGamepadManager::ButtonL1, "[GAMEPAD_L1]"},
    {QGamepadManager::ButtonL2, "[GAMEPAD_L2]"},
    {QGamepadManager::ButtonL3, "[GAMEPAD_L3]"},
    {QGamepadManager::ButtonR1, "[GAMEPAD_R1]"},
    {QGamepadManager::ButtonR2, "[GAMEPAD_R2]"},
    {QGamepadManager::ButtonR3, "[GAMEPAD_R3]"},
    {QGamepadManager::ButtonSelect, "[GAMEPAD_SEL]"},
    {QGamepadManager::ButtonStart, "[GAMEPAD_STR]"},
    {QGamepadManager::ButtonUp, "[GAMEPAD_UP]"},
    {QGamepadManager::ButtonDown, "[GAMEPAD_DN]"},
    {QGamepadManager::ButtonLeft, "[GAMEPAD_LF]"},
    {QGamepadManager::ButtonRight, "[GAMEPAD_RG]"},
    {QGamepadManager::ButtonCenter, "[GAMEPAD_CN]"},
    {QGamepadManager::ButtonGuide, "[GAMEPAD_GD]"}
};

QIcon GamepadButtons::iconForButton(QGamepadManager::GamepadButton button, QColor tint)
{

    const QMap<QGamepadManager::GamepadButton, QString> buttonToIconMapping = {
        {QGamepadManager::ButtonA, "btnA"},
        {QGamepadManager::ButtonB, "btnB"},
        {QGamepadManager::ButtonX, "btnX"},
        {QGamepadManager::ButtonY, "btnY"},
//        {QGamepadManager::ButtonA, "btnPSX"},
//        {QGamepadManager::ButtonB, "btnPSC"},
//        {QGamepadManager::ButtonX, "btnPST"},
//        {QGamepadManager::ButtonY, "btnPSS"},
        {QGamepadManager::ButtonL1, "btnL1"},
        {QGamepadManager::ButtonL2, "btnL2"},
        {QGamepadManager::ButtonL3, "btnL3"},
        {QGamepadManager::ButtonR1, "btnR1"},
        {QGamepadManager::ButtonR2, "btnR2"},
        {QGamepadManager::ButtonR3, "btnL3"},
        {QGamepadManager::ButtonSelect, "btnSEL"},
        {QGamepadManager::ButtonStart, "btnSTR"},
        {QGamepadManager::ButtonUp, "btnDU"},
        {QGamepadManager::ButtonDown, "btnDD"},
        {QGamepadManager::ButtonLeft, "btnDL"},
        {QGamepadManager::ButtonRight, "btnDR"},
        {QGamepadManager::ButtonCenter, "btn"},
        {QGamepadManager::ButtonGuide, "btnGDE"}
    };

    return iconForButtonHelper(buttonToIconMapping.value(button, "btn"), tint);
}

QString GamepadButtons::stringForButton(QGamepadManager::GamepadButton button)
{
    return buttonToStringMapping.value(button, "[GAMEPAD_INV]");
}

int GamepadButtons::measureGamepadString(QFontMetrics fm, QString string)
{
    const int btnWidth = fm.height();
    int width = 0;
    for (QString button : buttonToStringMapping.values()) {
        int count = string.count(button);
        width += count * btnWidth;
        string = string.remove(button);
    }
    return width + fm.horizontalAdvance(string);
}

void GamepadButtons::drawGamepadString(QPainter* painter, QString string, QRect boundingRect)
{
    //Build the regex to split by
    QStringList regexParts;
    for (QString button : buttonToStringMapping.values()) {
        regexParts.append(QRegularExpression::escape(button));
    }

    QRegularExpression regex(regexParts.join("|"));
    QRegularExpressionMatchIterator matches = regex.globalMatch(string);
    QStringList normalStringParts = string.split(regex);

    QStringList stringParts;
    while (!normalStringParts.isEmpty()) {
        stringParts.append(normalStringParts.takeFirst());
        if (matches.hasNext()) stringParts.append(matches.next().captured());
    }

    int currentX = boundingRect.left();
    for (QString stringPart : stringParts) {
        if (buttonToStringMapping.values().contains(stringPart)) {
            qDebug() << "Draw button" << stringPart;

            QSize pxSize(painter->fontMetrics().height(), painter->fontMetrics().height());
            QPixmap px = iconForButton(buttonToStringMapping.key(stringPart), painter->pen().color()).pixmap(pxSize);

            QRect bRect;
            bRect.moveLeft(currentX);
            bRect.setSize(pxSize);
            bRect.moveTop(boundingRect.top() + (boundingRect.height() / 2 - pxSize.height() / 2));
            painter->drawPixmap(bRect, px);
            currentX += pxSize.width();
        } else {
            qDebug() << "Draw text" << stringPart;
            QRect bRect;
            bRect.moveLeft(currentX);
            bRect.moveTop(boundingRect.top());
            bRect.setWidth(painter->fontMetrics().horizontalAdvance(stringPart));
            bRect.setHeight(painter->fontMetrics().height());
            painter->drawText(bRect, Qt::AlignLeft | Qt::AlignCenter, stringPart);

            currentX += bRect.width();
        }
    }
}

GamepadButtons::GamepadButtons(QObject *parent) : QObject(parent)
{

}

