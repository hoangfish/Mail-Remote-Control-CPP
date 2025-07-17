#pragma once

#include <QString>
#include <QPlainTextEdit>

// Con trỏ GUI (cài đặt từ main window)
extern QPlainTextEdit* gui_textEdit;
extern QPlainTextEdit* gui_textEdit_2;

// Cài đặt an toàn với Qt signal-slot
void safe_set_text(QPlainTextEdit* widget, const QString& text);
void safe_append_text(QPlainTextEdit* widget, const QString& text);
