#include "Mail_reader.h"
#include "utils.h"

using json = nlohmann::json;

// Khởi tạo biến tĩnh
long long MailReader::lastProcessedDate = 0;

// =======================
// Constructor
// =======================
MailReader::MailReader(const std::string &accessToken, const std::string &userEmail)
    : token(accessToken), gmailUser(userEmail)
{
    if (lastProcessedDate == 0)
    {
        lastProcessedDate = std::chrono::duration_cast<std::chrono::milliseconds>(
                                std::chrono::system_clock::now().time_since_epoch())
                                .count() -
                            60000; // Trừ 1 phút để lấy email gần đây
    }
}

// =======================
// Giải mã base64url
// =======================
std::string MailReader::decodeBase64Url(const std::string &input)
{
    try
    {
        if (input.empty())
            return "";
        // Thay thế ký tự base64url thành base64 chuẩn
        std::string modifiedInput = input;
        std::replace(modifiedInput.begin(), modifiedInput.end(), '-', '+');
        std::replace(modifiedInput.begin(), modifiedInput.end(), '_', '/');
        // Thêm padding nếu cần
        while (modifiedInput.length() % 4 != 0)
        {
            modifiedInput += '=';
        }
        QByteArray base64Data = QByteArray::fromBase64(
            QByteArray::fromStdString(modifiedInput),
            QByteArray::Base64UrlEncoding);
        return std::string(base64Data.constData(), base64Data.size());
    }
    catch (const std::exception &e)
    {
        return "";
    }
}

// =======================
// Lấy danh sách ID các message gần đây
// =======================
std::vector<std::string> MailReader::listRecentMessageIds(int maxResults, QTextEdit *logEdit)
{
    std::vector<std::string> result;
    try
    {
        std::string gmail_client = "phphuc2417@clc.fitus.edu.vn";
        std::string query = "from:" + gmailUser + " to:" + gmail_client + " -in:drafts";
        std::string url = "https://gmail.googleapis.com/gmail/v1/users/me/messages?maxResults=" +
                          std::to_string(maxResults) + "&q=" + utils::urlEncode(query);

        std::vector<std::string> headers = {
            "Authorization: Bearer " + token,
            "Content-Type: application/json"};

        std::string response;
        if (!utils::httpGet(url, headers, response))
        {
            utils::log(logEdit, "❌ Không lấy được danh sách mail: " + QString::fromStdString(response));
            return result;
        }

        json data = json::parse(response, nullptr, false);
        if (data.is_discarded() || !data.contains("messages"))
        {
            utils::log(logEdit, "❌ Phản hồi API không chứa messages: " + QString::fromStdString(response));
            return result;
        }

        for (const auto &msg : data["messages"])
        {
            if (msg.contains("id"))
            {
                std::string messageId = msg["id"].get<std::string>();
                std::string detailUrl = "https://gmail.googleapis.com/gmail/v1/users/me/messages/" + messageId + "?format=metadata&metadataHeaders=labelIds";
                std::string detailResponse;
                if (utils::httpGet(detailUrl, headers, detailResponse))
                {
                    json detailJson = json::parse(detailResponse, nullptr, false);
                    if (!detailJson.is_discarded() && detailJson.contains("labelIds"))
                    {
                        bool isDraft = false;
                        for (const auto &label : detailJson["labelIds"])
                        {
                            if (label == "DRAFT")
                            {
                                isDraft = true;
                                break;
                            }
                        }
                        if (!isDraft)
                        {
                            result.push_back(messageId);
                        }
                    }
                }
            }
        }
    }
    catch (const std::exception &e)
    {
        utils::log(logEdit, "❌ Lỗi trong listRecentMessageIds: " + QString(e.what()));
    }
    return result;
}

// =======================
// Lấy nội dung của 1 message và phân tích nó
// =======================
bool MailReader::parseMessage(const std::string &messageId, MailData &data, QTextEdit *logEdit)
{
    try
    {
        std::string url = "https://gmail.googleapis.com/gmail/v1/users/me/messages/" + messageId + "?format=full";
        std::vector<std::string> headers = {
            "Authorization: Bearer " + token,
            "Content-Type: application/json"};

        std::string response;
        if (!utils::httpGet(url, headers, response))
        {
            utils::log(logEdit, QString::fromUtf8("⚠️ Không đọc được nội dung mail ID: ") + QString::fromStdString(messageId));
            return false;
        }

        json msgJson = json::parse(response, nullptr, false);
        data.id = messageId;
        if (msgJson.is_discarded())
        {
            utils::log(logEdit, QString::fromUtf8("⚠️ Lỗi phân tích JSON mail ID: ") + QString::fromStdString(messageId));
            return false;
        }

        if (msgJson.contains("internalDate"))
        {
            try
            {
                data.internalDate = std::stoll(msgJson["internalDate"].get<std::string>());
                if (data.internalDate <= lastProcessedDate)
                {
                    return false; // Bỏ qua email cũ
                }
            }
            catch (const std::exception &e)
            {
                utils::log(logEdit, QString::fromUtf8("⚠️ Lỗi phân tích internalDate: ") + e.what());
                return false;
            }
        }
        else
        {
            return false;
        }

        if (msgJson.contains("payload") && msgJson["payload"].contains("headers"))
        {
            for (const auto &header : msgJson["payload"]["headers"])
            {
                if (header.contains("name") && header.contains("value"))
                {
                    std::string name = header["name"].get<std::string>();
                    std::string value = header["value"].get<std::string>();
                    if (name == "Subject")
                        data.subject = value;
                    else if (name == "From")
                        data.sender = value;
                }
            }
        }

        if (msgJson.contains("threadId"))
        {
            data.threadId = msgJson["threadId"].get<std::string>();
        }

        std::string encoded;
        if (msgJson["payload"].contains("body") && msgJson["payload"]["body"].contains("data"))
        {
            encoded = msgJson["payload"]["body"]["data"].get<std::string>();
        }
        else if (msgJson["payload"].contains("parts"))
        {
            for (const auto &part : msgJson["payload"]["parts"])
            {
                if (part.contains("body") && part["body"].contains("data"))
                {
                    encoded = part["body"]["data"].get<std::string>();
                    break;
                }
            }
        }

        data.body = decodeBase64Url(encoded);
        if (data.body.empty())
        {
            utils::log(logEdit, QString::fromUtf8("⚠️ Không giải mã được nội dung mail ID: ") + QString::fromStdString(messageId));
            return false;
        }

        return true;
    }
    catch (const std::exception &e)
    {
        utils::log(logEdit, QString::fromUtf8("❌ Lỗi trong parseMessage: ") + QString(e.what()));
        return false;
    }
}

// =======================
// Trả về nhiều mail hợp lệ mới nhất
// =======================
std::vector<MailData> MailReader::fetchLatestRequests(int maxResults, QTextEdit *logEdit)
{
    std::vector<MailData> validMails;
    try
    {
        auto ids = listRecentMessageIds(maxResults, logEdit);
        for (const auto &id : ids)
        {
            MailData mail;
            if (parseMessage(id, mail, logEdit))
            {
                if (!mail.subject.empty() && !mail.body.empty() && mail.internalDate > lastProcessedDate)
                {
                    validMails.push_back(mail);
                    lastProcessedDate = mail.internalDate;
                    utils::log(logEdit, "📥 Nhận được email mới từ: " + QString::fromStdString(mail.sender));
                }
            }
        }
    }
    catch (const std::exception &e)
    {
        utils::log(logEdit, "❌ Lỗi trong fetchLatestRequests: " + QString(e.what()));
    }
    return validMails;
}