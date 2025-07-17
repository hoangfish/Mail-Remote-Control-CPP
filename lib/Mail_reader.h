#ifndef MAIL_READER_H
#define MAIL_READER_H

#include <QString>
#include <QTextEdit>
#include <QByteArray>
#include <chrono>
#include <json.hpp>
#include <string>
#include <vector>

// ========== Cấu trúc mail yêu cầu ==========
struct MailData {
    std::string subject;   // IP server
    std::string body;      // Nội dung lệnh từ user
    std::string sender;    // Gmail user đã gửi
    std::string threadId;  // Để trả lời đúng luồng
    long long internalDate; // Thời gian email (ms)
    std::string id;
};

// ========== Class đọc mail từ Gmail API ==========
class MailReader {
public:
    // Khởi tạo với access_token hợp lệ
    explicit MailReader(const std::string& accessToken, const std::string& userEmail);

    // Lấy nhiều mail hợp lệ
    std::vector<MailData> fetchLatestRequests(int maxResults=5, QTextEdit* logEdit = nullptr);

private:
    std::string token;
    std::string gmailUser;
    static long long lastProcessedDate; // Lưu internalDate của email cuối cùng đã xử lý

    // Truy vấn các message gần nhất từ inbox (id mail)
    std::vector<std::string> listRecentMessageIds(int maxResults, QTextEdit* logEdit = nullptr);

    // Lấy chi tiết 1 message → subject, body, sender, threadId, internalDate
    bool parseMessage(const std::string& messageId, MailData& data, QTextEdit* logEdit = nullptr);

    // Giải mã base64url → chuẩn thư của Gmail (chứa nội dung mail raw)
    std::string decodeBase64Url(const std::string& input);
};

#endif // MAIL_READER_H