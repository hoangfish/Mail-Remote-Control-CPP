#include "utils.h"
#include <curl.h>

namespace utils {

bool openBrowser(const std::string& url) {
    QString qUrl = QString::fromStdString(url);
    bool success = QDesktopServices::openUrl(QUrl(qUrl));
    return success;
}

void log(QTextEdit* edit, const QString& message) {
    if (edit) {
        edit->append(message);
    }
}

std::string urlEncode(const std::string& input) {
    CURL* curl = curl_easy_init();
    if (!curl) return input;

    char* encoded = curl_easy_escape(curl, input.c_str(), input.length());
    std::string result = encoded ? encoded : input;
    curl_free(encoded);
    curl_easy_cleanup(curl);
    return result;
}

static size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    size_t total = size * nmemb;
    userp->append((char*)contents, total);
    return total;
}

bool httpGet(const std::string& url, const std::vector<std::string>& headers, std::string& response) {
    CURL* curl = curl_easy_init();
    if (!curl) return false;

    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    struct curl_slist* list = nullptr;
    for (const auto& header : headers) {
        list = curl_slist_append(list, header.c_str());
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

    CURLcode res = curl_easy_perform(curl);
    curl_slist_free_all(list);
    curl_easy_cleanup(curl);

    return res == CURLE_OK;
}

}