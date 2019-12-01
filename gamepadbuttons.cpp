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
#include <QKeySequence>
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
    {QGamepadManager::ButtonA, "[%!GAMEPAD_A]"},
    {QGamepadManager::ButtonB, "[%!GAMEPAD_B]"},
    {QGamepadManager::ButtonX, "[%!GAMEPAD_X]"},
    {QGamepadManager::ButtonY, "[%!GAMEPAD_Y]"},
    {QGamepadManager::ButtonL1, "[%!GAMEPAD_L1]"},
    {QGamepadManager::ButtonL2, "[%!GAMEPAD_L2]"},
    {QGamepadManager::ButtonL3, "[%!GAMEPAD_L3]"},
    {QGamepadManager::ButtonR1, "[%!GAMEPAD_R1]"},
    {QGamepadManager::ButtonR2, "[%!GAMEPAD_R2]"},
    {QGamepadManager::ButtonR3, "[%!GAMEPAD_R3]"},
    {QGamepadManager::ButtonSelect, "[%!GAMEPAD_SEL]"},
    {QGamepadManager::ButtonStart, "[%!GAMEPAD_STR]"},
    {QGamepadManager::ButtonUp, "[%!GAMEPAD_UP]"},
    {QGamepadManager::ButtonDown, "[%!GAMEPAD_DN]"},
    {QGamepadManager::ButtonLeft, "[%!GAMEPAD_LF]"},
    {QGamepadManager::ButtonRight, "[%!GAMEPAD_RG]"},
    {QGamepadManager::ButtonCenter, "[%!GAMEPAD_CN]"},
    {QGamepadManager::ButtonGuide, "[%!GAMEPAD_GD]"}
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
    return buttonToStringMapping.value(button, "[%!GAMEPAD_INV]");
}

QPixmap GamepadButtons::iconForKey(QKeySequence key, QFont font, QPalette pal)
{
    QFontMetrics metrics(font);
    QString sequence = key.toString(QKeySequence::NativeText);
    QStringList chordParts = sequence.split(", ", QString::SkipEmptyParts);
    if (chordParts.count() == 0) {
        QRect textRect;
        textRect.setWidth(metrics.horizontalAdvance(tr("No Shortcut")) + 1);
        textRect.setHeight(metrics.height());
        textRect.moveLeft(0);
        textRect.moveTop(0);

        QPixmap pixmap(textRect.size());
        pixmap.fill(Qt::transparent);

        QPainter painter(&pixmap);
        painter.setPen(pal.color(QPalette::WindowText));
        painter.drawText(textRect, tr("No Shortcut"));

        return pixmap;
    } else {
        int width = 0;
        for (int i = 0; i < chordParts.count(); i++) {
            QStringList keys = chordParts.at(i).split("+", QString::SkipEmptyParts);

            for (QString key : keys) {
                QPixmap icon = getKeyIcon(key, font, pal);
                width += icon.width() + SC_DPI(4);
            }

            if (chordParts.count() > i + 1) {
                width += metrics.horizontalAdvance(",") + 1 + SC_DPI(4);
            }
        }
        width -= SC_DPI(4);

        QPixmap pixmap(width, qMax(SC_DPI(24), metrics.height() + SC_DPI(8)));
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setPen(pal.color(QPalette::WindowText));
        int currentX = 0;
        bool cropTo16 = true;

        for (int i = 0; i < chordParts.count(); i++) {
            QStringList keys = chordParts.at(i).split("+", QString::SkipEmptyParts);

            for (QString key : keys) {
                QPixmap icon = getKeyIcon(key, font, pal);
                if (icon.width() == SC_DPI(16)) {
                    painter.drawPixmap(currentX, 0, icon);
                } else {
                    cropTo16 = false;
                    painter.drawPixmap(currentX, SC_DPI(4), icon);
                }
                currentX += icon.width() + SC_DPI(4);
            }

            if (chordParts.count() > i + 1) {
                QRect textRect;
                textRect.setWidth(metrics.horizontalAdvance(",") + 1);
                textRect.setHeight(metrics.height());
                textRect.moveLeft(currentX);
                textRect.moveTop(pixmap.height() / 2 - textRect.height() / 2);
                painter.drawText(textRect, ",");
                currentX = textRect.right() + SC_DPI(4);
            }
        }

        painter.end();

        if (cropTo16) {
            pixmap = pixmap.copy(0, 0, pixmap.width(), SC_DPI(16));
        }
        return pixmap;
    }
}

QString GamepadButtons::stringForKey(QKeySequence key)
{
    return QStringLiteral("[%!KEYBOARD_%1]").arg(key.toString(QKeySequence::PortableText));
}

int GamepadButtons::measureGamepadString(QFont font, QString string)
{
    QRegularExpression regex("\\[%!.*?\\]");
    QRegularExpressionMatchIterator matches = regex.globalMatch(string);
    QStringList normalStringParts = string.split(regex);

    QStringList stringParts;
    while (!normalStringParts.isEmpty()) {
        stringParts.append(normalStringParts.takeFirst());
        if (matches.hasNext()) stringParts.append(matches.next().captured());
    }

    int width = 0;
    for (QString stringPart : stringParts) {
        if (buttonToStringMapping.values().contains(stringPart)) {
            width += QFontMetrics(font).height();
        } else if (stringPart.startsWith("[%!KEYBOARD_")) {
            QPixmap px = iconForKey(stringPart.mid(12).chopped(1), font, QPalette());
            width += px.width();
        } else {
            width += QFontMetrics(font).horizontalAdvance(stringPart);
        }
    }
    return width;
}

void GamepadButtons::drawGamepadString(QPainter* painter, QString string, QRect boundingRect, QPalette pal)
{
    //Build the regex to split by
//    QStringList regexParts;
//    for (QString button : buttonToStringMapping.values()) {
//        regexParts.append(QRegularExpression::escape(button));
//    }

//    QRegularExpression regex(regexParts.join("|"));

    QRegularExpression regex("\\[%!.*?\\]");
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
        } else if (stringPart.startsWith("[%!KEYBOARD_")) {
            qDebug() << "Draw kbutton" << stringPart;

            QPixmap px = iconForKey(stringPart.mid(12).chopped(1), painter->font(), pal);

            QRect bRect;
            bRect.moveLeft(currentX);
            bRect.setSize(px.size());
            bRect.moveTop(boundingRect.top() + (boundingRect.height() / 2 - px.height() / 2));
            painter->drawPixmap(bRect, px);
            currentX += px.width();
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

QPixmap GamepadButtons::getKeyIcon(QString key, QFont font, QPalette pal)
{
    //Special Cases
    if (key == "Meta") key = "Super";
    if (key == "Print") key = "PrtSc";
    if (key == "Enter") key = "↵";
    if (key == "Return") key = "↵";
    if (key == "Space") key = "⌴";
//    if (key == "Esc") key = "⎋";

    QPixmap squarePx(SC_DPI_T(QSize(16, 16), QSize));
    squarePx.fill(Qt::transparent);

    QPainter sqPainter(&squarePx);
    sqPainter.setRenderHint(QPainter::Antialiasing);
    sqPainter.setPen(Qt::transparent);
    sqPainter.setBrush(pal.color(QPalette::WindowText));
    sqPainter.drawRoundedRect(QRect(QPoint(0, 0), squarePx.size()), 50, 50, Qt::RelativeSize);

    QRect squareIconRect;
    squareIconRect.setWidth(SC_DPI(12));
    squareIconRect.setHeight(SC_DPI(12));
    squareIconRect.moveCenter(SC_DPI_T(QPoint(8, 8), QPoint));

    font.setPixelSize(SC_DPI(12));
    QFontMetrics fontMetrics(font);

    QSize pixmapSize;
    pixmapSize.setHeight(SC_DPI(16));
    pixmapSize.setWidth(qMax(fontMetrics.horizontalAdvance(key) + SC_DPI(6), SC_DPI(16)));

    QPixmap px(pixmapSize);
    px.fill(Qt::transparent);

    QPainter painter(&px);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::transparent);
    painter.setBrush(pal.color(QPalette::WindowText));
    painter.drawRoundedRect(QRect(QPoint(0, 0), px.size()), 4 * theLibsGlobal::getDPIScaling(), 4 * theLibsGlobal::getDPIScaling());

    painter.setFont(font);
    painter.setPen(Qt::black);
    painter.setCompositionMode(QPainter::CompositionMode_DestinationOut);

    QRect textRect;
    textRect.setHeight(fontMetrics.height());
    textRect.setWidth(fontMetrics.horizontalAdvance(key) + 1);
    textRect.moveCenter(QPoint(pixmapSize.width() / 2, pixmapSize.height() / 2));

    painter.drawText(textRect, Qt::AlignCenter, key);

    painter.end();
    return px;
}

