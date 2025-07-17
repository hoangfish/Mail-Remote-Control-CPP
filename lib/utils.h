#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QTextEdit>
#include <QDesktopServices>
#include <QUrl>
#include <string>
#include <vector>

namespace utils {
    // Mở trình duyệt
    bool openBrowser(const std::string& url);

    // Log message
    void log(QTextEdit* edit, const QString& message);

    // URL encode
    std::string urlEncode(const std::string& input);

    // Gửi yêu cầu GET
    bool httpGet(const std::string& url, const std::vector<std::string>& headers, std::string& response);
}

#endif // UTILS_H