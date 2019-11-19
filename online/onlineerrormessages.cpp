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
#include "onlineerrormessages.h"

#include <QApplication>
#include <QMap>

struct OnlineErrorMessagesPrivate {
    QMap<QString, const char*> errorMessages = {
        {"fields.missing", QT_TRANSLATE_NOOP("OnlineErrorMessagesPrivate", "Some fields are missing.")},
        {"username.taken", QT_TRANSLATE_NOOP("OnlineErrorMessagesPrivate", "That username has already been taken. Set a different username.")},
        {"username.tooLong", QT_TRANSLATE_NOOP("OnlineErrorMessagesPrivate", "That username is too long. Keep it at 32 characters or under.")},
        {"username.bad", QT_TRANSLATE_NOOP("OnlineErrorMessagesPrivate", "That username is invalid. Stick to alphanumeric characters and spaces.")},
        {"email.taken", QT_TRANSLATE_NOOP("OnlineErrorMessagesPrivate", "That email address has already been taken.")},
        {"email.bad", QT_TRANSLATE_NOOP("OnlineErrorMessagesPrivate", "That email address is invalid.")},
        {"authentication.incorrect", QT_TRANSLATE_NOOP("OnlineErrorMessagesPrivate", "Check your username and password and try again.")},
        {"otp.incorrect", QT_TRANSLATE_NOOP("OnlineErrorMessagesPrivate", "Check your Two Factor Authentication code and try again.")},
        {"otp.unavailable", QT_TRANSLATE_NOOP("OnlineErrorMessagesPrivate", "Two Factor Authentication is unavailable at this time.")},
        {"otp.alreadyEnabled", QT_TRANSLATE_NOOP("OnlineErrorMessagesPrivate", "Two Factor Authentication is already enabled for your account.")},
        {"otp.alreadyDisabled", QT_TRANSLATE_NOOP("OnlineErrorMessagesPrivate", "Two Factor Authentication is already disabled for your account.")},
        {"otp.invalidToken", QT_TRANSLATE_NOOP("OnlineErrorMessagesPrivate", "Check your Two Factor Authentication code and try again.")},
        {"authentication.invalid", QT_TRANSLATE_NOOP("OnlineErrorMessagesPrivate", "Your credentials are incorrect. If you are logged in, you'll need to log out and log in again.")},
        {"verification.invalid", QT_TRANSLATE_NOOP("OnlineErrorMessagesPrivate", "Your verification code is incorrect, or the verification code has expired.\n\n"
                                                                                 "If it's been more than a day since you received the verification email, you'll need to resend the verification email to get a new code.")}
    };

    Q_DECLARE_TR_FUNCTIONS(OnlineErrorMessagePrivate)
};

OnlineErrorMessagesPrivate* OnlineErrorMessages::d = new OnlineErrorMessagesPrivate();

OnlineErrorMessages::OnlineErrorMessages()
{

}

QString OnlineErrorMessages::messageForCode(QString code, QString defaultMessage)
{
    if (d->errorMessages.contains(code)) return OnlineErrorMessagesPrivate::tr(d->errorMessages.value(code));
    return defaultMessage;
}
