#pragma once

/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

// Minimal client for S3-compatible object storage (AWS S3, Cloudflare R2,
// MinIO, Backblaze B2, ...). Requests are signed with AWS SigV4 by libcurl
// itself (CURLOPT_AWS_SIGV4). Every call is blocking and uses its own curl
// handle, so it is meant to run on a worker thread — it never goes through
// CurlManager::INSTANCE, whose transfers are pumped by the UI thread.

#include <cstdint>
#include <filesystem>
#include <functional>
#include <map>
#include <string>
#include <vector>

struct S3Config {
    std::string endpoint;            // e.g. https://<account_id>.r2.cloudflarestorage.com
    std::string region = "auto";     // "auto" for R2
    std::string bucket;
    std::string accessKeyId;
    std::string secretAccessKey;
    bool pathStyle = true;           // https://endpoint/bucket/key rather than https://bucket.endpoint/key
    bool verifySSL = true;

    static std::string R2Endpoint(const std::string& accountId);
};

// Return false to cancel.
using S3Progress = std::function<bool(uint64_t doneBytes, uint64_t totalBytes)>;

struct S3Object {
    std::string key;
    uint64_t size = 0;
    std::string lastModified; // ISO 8601, as returned by the server
};

class S3Client {
public:
    explicit S3Client(S3Config config);

    // Writes then deletes a small object under keyPrefix, which checks the
    // endpoint, the credentials and write access to the bucket in one go.
    bool TestConnection(const std::string& keyPrefix, std::string& error);

    bool PutObject(const std::string& key, const std::string& data, const std::string& contentType, std::string& error);
    bool DeleteObject(const std::string& key, std::string& error);

    // Single PUT for small files, multipart upload otherwise. sha256Hex, when
    // given, receives the SHA-256 of the whole file computed while sending.
    bool UploadFile(const std::string& key, const std::filesystem::path& file, const std::string& contentType,
                    const S3Progress& progress, std::string& error, std::string* sha256Hex = nullptr,
                    bool* cancelled = nullptr);

    // ListObjectsV2, following continuation tokens. With a delimiter, keys
    // below the next delimiter are folded into commonPrefixes.
    bool ListObjects(const std::string& prefix, const std::string& delimiter, std::vector<S3Object>& objects,
                     std::vector<std::string>& commonPrefixes, std::string& error);

    // Small objects only: the whole body is kept in memory.
    bool ReadObject(const std::string& key, std::string& data, std::string& error);

    // Streams the object to file. On failure or cancel the partial file is removed.
    bool DownloadFile(const std::string& key, const std::filesystem::path& file, const S3Progress& progress,
                      std::string& error, std::string* sha256Hex = nullptr, bool* cancelled = nullptr);

    void SetPartSize(uint64_t bytes) { _partSize = bytes; }

    // RFC 3986 encoding as SigV4 expects it; '/' is kept when keepSlash is set.
    static std::string UriEncode(const std::string& s, bool keepSlash);

private:
    struct Response {
        long status = 0;
        std::string body;
        std::map<std::string, std::string> headers; // lower-case names
        std::string curlError;
        bool aborted = false;
        bool ok() const { return curlError.empty() && status >= 200 && status < 300; }
    };

    struct RequestOptions {
        const char* body = nullptr;
        size_t bodyLen = 0;
        std::string contentType;
        // Called with the bytes transferred so far; return false to abort.
        std::function<bool(uint64_t)> progress;
        bool downloadProgress = false;
        // Receives a 2xx response body instead of Response::body; return false to abort.
        std::function<bool(const char*, size_t)> sink;
    };

    std::string ObjectUrl(const std::string& key) const;
    Response Send(const std::string& method, const std::string& key, const std::string& query, const RequestOptions& opts) const;
    Response Request(const std::string& method, const std::string& key, const std::string& query,
                     const char* body, size_t bodyLen, const std::string& contentType,
                     const std::function<bool(uint64_t)>& uploadProgress = nullptr) const;
    static std::string Describe(const Response& r);

    bool UploadMultipart(const std::string& key, const std::filesystem::path& file, uint64_t size,
                         const std::string& contentType, const S3Progress& progress, std::string& error,
                         std::string* sha256Hex, bool* cancelled);

    S3Config _cfg;
    uint64_t _partSize = 16ull * 1024 * 1024;
};
