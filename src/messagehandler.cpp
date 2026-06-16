/*
 * Copyright (c) KylinSoft Co., Ltd. 2025.All rights reserved.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "messagehandler.h"
#include <QCommandLineParser>
#include <QCoreApplication>
#include <QtGlobal>

MessageHandler::MessageHandler(QObject *parent) : QObject(parent) {
    registerCommand("loglevel", [this](const QStringList &args) {
        return handleLogLevel(args);
    });
}

void MessageHandler::registerCommand(
    const QString &command,
    std::function<QString(const QStringList &)> handler) {
    commandHandlers[command] = handler;
}

void MessageHandler::processMessage(const QString &rawMessage,
                                    ResultCallback callback) {
    QStringList parts = rawMessage.split(' ', QString::SkipEmptyParts);
    if (parts.isEmpty()) {
        QString error = "Empty message received";
        KVKBD_WARN(error.toStdString());
        if (callback)
            callback(error);
        return;
    }

    QString command = parts.first().toLower();
    QStringList args = parts.mid(1);

    if (commandHandlers.contains(command)) {
        try {
            QString result = commandHandlers[command](args);
            if (callback) {
                callback(result);
            }
        } catch (const std::exception &e) {
            QString error = QString("Error processing command '%1': %2")
                                .arg(command)
                                .arg(e.what());
            KVKBD_ERROR(error.toStdString());
            if (callback)
                callback(error);
        }
    } else {
        QString error = QString("Unknown command: %1").arg(command);
        KVKBD_WARN(error.toStdString());
        if (callback)
            callback(error);
    }
}

QString MessageHandler::handleLogLevel(const QStringList &args) const {
    if (args.isEmpty()) {
        return "log-level command requires a level argument";
    }

    QString level = args.first().toLower();
    QString result;

    if (level == "debug") {
        spdlog::set_level(spdlog::level::debug);
        result = "Log level set to DEBUG";
    } else if (level == "info") {
        spdlog::set_level(spdlog::level::info);
        result = "Log level set to INFO";
    } else if (level == "warn") {
        spdlog::set_level(spdlog::level::warn);
        result = "Log level set to WARN";
    } else if (level == "error") {
        spdlog::set_level(spdlog::level::err);
        result = "Log level set to ERROR";
    } else {
        result = QString("Invalid log level: %1").arg(level);
    }
    return result;
}
