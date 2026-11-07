#include <catch2/catch_test_macros.hpp>

#include "utils/CloudBackupStore.h"
#include "utils/S3Client.h"

#include <map>
#include <string>
#include <vector>

namespace {

const std::string FOLDER = "xlights/My Show/";

// A bucket held in memory. failOn makes remove() fail for that key.
struct FakeBucket {
    std::map<std::string, std::string> objects;
    std::string failOn;
    std::vector<std::string> removed;

    void AddBackup(const std::string& stamp, bool thumbnail = true) {
        objects[FOLDER + stamp + ".zip"] = std::string(100, 'z');
        objects[FOLDER + stamp + ".manifest.json"] = "manifest " + stamp;
        if (thumbnail) {
            objects[FOLDER + stamp + ".thumbnail.png"] = "png";
        }
    }

    std::vector<S3Object> List() const {
        std::vector<S3Object> out;
        for (const auto& [k, v] : objects) {
            if (k.rfind(FOLDER, 0) == 0) {
                out.push_back({ k, v.size(), "" });
            }
        }
        return out;
    }

    CloudObjectOps Ops() {
        CloudObjectOps ops;
        ops.remove = [this](const std::string& key, std::string& error) {
            if (key == failOn) {
                error = "HTTP 403 AccessDenied";
                return false;
            }
            objects.erase(key);
            removed.push_back(key);
            return true;
        };
        ops.read = [this](const std::string& key, std::string& data, std::string& error) {
            auto it = objects.find(key);
            if (it == objects.end()) {
                error = "Not found: " + key;
                return false;
            }
            data = it->second;
            return true;
        };
        ops.write = [this](const std::string& key, const std::string& data, std::string&) {
            objects[key] = data;
            return true;
        };
        return ops;
    }

    std::string Latest() const {
        auto it = objects.find(FOLDER + CloudBackupStore::LATEST_MANIFEST);
        return it == objects.end() ? "<none>" : it->second;
    }
};

std::vector<std::string> Stamps(const std::vector<CloudBackup>& backups) {
    std::vector<std::string> out;
    for (const auto& b : backups) {
        out.push_back(b.stamp);
    }
    return out;
}

CloudBackup Find(const std::vector<CloudBackup>& backups, const std::string& stamp) {
    for (const auto& b : backups) {
        if (b.stamp == stamp) {
            return b;
        }
    }
    FAIL("no backup " << stamp);
    return {};
}

} // namespace

TEST_CASE("ShowFolderKey ignores stray slashes around the prefix", "[cloud]") {
    CHECK(CloudBackupStore::ShowFolderKey("xlights", "My Show") == "xlights/My Show/");
    CHECK(CloudBackupStore::ShowFolderKey("/xlights/", "My Show") == "xlights/My Show/");
    CHECK(CloudBackupStore::ShowFolderKey("", "My Show") == "My Show/");
}

TEST_CASE("FromListing keeps only complete backups, newest first", "[cloud]") {
    FakeBucket bucket;
    bucket.AddBackup("2026-11-01-100000");
    bucket.AddBackup("2026-11-03-100000", false);
    bucket.AddBackup("2026-11-02-100000");
    bucket.objects[FOLDER + "2026-11-04-100000.zip"] = "upload whose manifest failed";
    bucket.objects[FOLDER + "2026-11-05-100000.manifest.json"] = "archive missing";
    bucket.objects[FOLDER + "latest.manifest.json"] = "manifest 2026-11-03-100000";
    bucket.objects[FOLDER + "sub/2026-11-06-100000.manifest.json"] = "nested";
    bucket.objects[FOLDER + "sub/2026-11-06-100000.zip"] = "nested";

    auto backups = CloudBackupStore::FromListing(bucket.List(), FOLDER);
    CHECK(Stamps(backups) == std::vector<std::string>{ "2026-11-03-100000", "2026-11-02-100000", "2026-11-01-100000" });
    CHECK(backups[0].thumbnailKey.empty());
    CHECK(backups[1].thumbnailKey == FOLDER + "2026-11-02-100000.thumbnail.png");
    CHECK(backups[1].zipKey == FOLDER + "2026-11-02-100000.zip");
    CHECK(backups[1].manifestKey == FOLDER + "2026-11-02-100000.manifest.json");
    CHECK(backups[1].zipSize == 100);
}

TEST_CASE("SelectForPruning keeps the newest backups and the protected one", "[cloud]") {
    FakeBucket bucket;
    for (const char* s : { "2026-11-01-100000", "2026-11-02-100000", "2026-11-03-100000", "2026-11-04-100000" }) {
        bucket.AddBackup(s);
    }
    auto all = CloudBackupStore::FromListing(bucket.List(), FOLDER);

    CHECK(CloudBackupStore::SelectForPruning(all, 0).empty());
    CHECK(CloudBackupStore::SelectForPruning(all, -3).empty());
    CHECK(CloudBackupStore::SelectForPruning(all, 4).empty());
    CHECK(CloudBackupStore::SelectForPruning(all, 10).empty());
    CHECK(Stamps(CloudBackupStore::SelectForPruning(all, 2)) ==
          std::vector<std::string>{ "2026-11-02-100000", "2026-11-01-100000" });
    CHECK(Stamps(CloudBackupStore::SelectForPruning(all, 1, "2026-11-04-100000")) ==
          std::vector<std::string>{ "2026-11-03-100000", "2026-11-02-100000", "2026-11-01-100000" });

    // A clock set back makes the new backup sort older: it is still kept.
    auto pruned = CloudBackupStore::SelectForPruning(all, 2, "2026-11-01-100000");
    CHECK(Stamps(pruned) == std::vector<std::string>{ "2026-11-03-100000", "2026-11-02-100000" });

    // An unknown protected stamp counts for nothing.
    CHECK(Stamps(CloudBackupStore::SelectForPruning(all, 3, "2025-01-01-000000")) ==
          std::vector<std::string>{ "2026-11-01-100000" });
}

TEST_CASE("Deleting an older backup leaves latest alone", "[cloud]") {
    FakeBucket bucket;
    bucket.AddBackup("2026-11-01-100000");
    bucket.AddBackup("2026-11-02-100000");
    bucket.objects[FOLDER + "latest.manifest.json"] = "manifest 2026-11-02-100000";
    auto all = CloudBackupStore::FromListing(bucket.List(), FOLDER);

    std::string error;
    REQUIRE(CloudBackupStore::Delete(bucket.Ops(), FOLDER, all, { Find(all, "2026-11-01-100000") }, error));
    CHECK(bucket.removed == std::vector<std::string>{ FOLDER + "2026-11-01-100000.manifest.json",
                                                      FOLDER + "2026-11-01-100000.zip",
                                                      FOLDER + "2026-11-01-100000.thumbnail.png" });
    CHECK(bucket.Latest() == "manifest 2026-11-02-100000");
    CHECK(Stamps(CloudBackupStore::FromListing(bucket.List(), FOLDER)) == std::vector<std::string>{ "2026-11-02-100000" });
}

TEST_CASE("Deleting the newest backup points latest at the one left", "[cloud]") {
    FakeBucket bucket;
    bucket.AddBackup("2026-11-01-100000");
    bucket.AddBackup("2026-11-02-100000", false);
    bucket.objects[FOLDER + "latest.manifest.json"] = "manifest 2026-11-02-100000";
    auto all = CloudBackupStore::FromListing(bucket.List(), FOLDER);

    std::string error;
    REQUIRE(CloudBackupStore::Delete(bucket.Ops(), FOLDER, all, { Find(all, "2026-11-02-100000") }, error));
    CHECK(bucket.removed.size() == 2); // no thumbnail to remove
    CHECK(bucket.Latest() == "manifest 2026-11-01-100000");
}

TEST_CASE("Deleting the last backup removes latest", "[cloud]") {
    FakeBucket bucket;
    bucket.AddBackup("2026-11-01-100000");
    bucket.objects[FOLDER + "latest.manifest.json"] = "manifest 2026-11-01-100000";
    auto all = CloudBackupStore::FromListing(bucket.List(), FOLDER);

    std::string error;
    REQUIRE(CloudBackupStore::Delete(bucket.Ops(), FOLDER, all, all, error));
    CHECK(bucket.objects.empty());
}

TEST_CASE("Pruning after a backup keeps the new one as latest", "[cloud]") {
    FakeBucket bucket;
    for (const char* s : { "2026-11-01-100000", "2026-11-02-100000", "2026-11-03-100000" }) {
        bucket.AddBackup(s);
    }
    bucket.objects[FOLDER + "latest.manifest.json"] = "manifest 2026-11-03-100000";
    auto all = CloudBackupStore::FromListing(bucket.List(), FOLDER);
    auto old = CloudBackupStore::SelectForPruning(all, 1, "2026-11-03-100000");

    std::string error;
    REQUIRE(CloudBackupStore::Delete(bucket.Ops(), FOLDER, all, old, error));
    CHECK(Stamps(CloudBackupStore::FromListing(bucket.List(), FOLDER)) == std::vector<std::string>{ "2026-11-03-100000" });
    CHECK(bucket.Latest() == "manifest 2026-11-03-100000");
}

TEST_CASE("A failed delete stops and still fixes latest", "[cloud]") {
    FakeBucket bucket;
    bucket.AddBackup("2026-11-01-100000");
    bucket.AddBackup("2026-11-02-100000");
    bucket.AddBackup("2026-11-03-100000");
    bucket.objects[FOLDER + "latest.manifest.json"] = "manifest 2026-11-03-100000";
    auto all = CloudBackupStore::FromListing(bucket.List(), FOLDER);

    SECTION("archive refused after its manifest went") {
        bucket.failOn = FOLDER + "2026-11-03-100000.zip";
        std::string error;
        CHECK_FALSE(CloudBackupStore::Delete(bucket.Ops(), FOLDER, all,
                                             { Find(all, "2026-11-03-100000"), Find(all, "2026-11-01-100000") }, error));
        CHECK(error == "HTTP 403 AccessDenied");
        // The half-deleted backup no longer lists; the next one was not touched.
        CHECK(Stamps(CloudBackupStore::FromListing(bucket.List(), FOLDER)) ==
              std::vector<std::string>{ "2026-11-02-100000", "2026-11-01-100000" });
        CHECK(bucket.Latest() == "manifest 2026-11-02-100000");
    }
    SECTION("manifest refused") {
        bucket.failOn = FOLDER + "2026-11-03-100000.manifest.json";
        std::string error;
        CHECK_FALSE(CloudBackupStore::Delete(bucket.Ops(), FOLDER, all, { Find(all, "2026-11-03-100000") }, error));
        CHECK(bucket.removed.empty());
        CHECK(bucket.Latest() == "manifest 2026-11-03-100000");
    }
}
