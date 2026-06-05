/***************************************************************
 * This source files comes from the xLights project
 * https://www.xlights.org
 * https://github.com/xLightsSequencer/xLights
 * See the github commit history for a record of contributing
 * developers.
 * Copyright claimed based on commit dates recorded in Github
 * License: https://github.com/xLightsSequencer/xLights/blob/master/License.txt
 **************************************************************/

#import "XLiPadServiceSettingsStore.h"

#import <Foundation/Foundation.h>
#import <Security/Security.h>

#include <spdlog/spdlog.h>

namespace {

NSString* toNSString(std::string_view sv) {
    return [[NSString alloc] initWithBytes:sv.data()
                                     length:(NSUInteger)sv.size()
                                   encoding:NSUTF8StringEncoding];
}

std::string toStdString(NSString* s) {
    if (s == nil) return {};
    const char* utf8 = [s UTF8String];
    return utf8 ? std::string(utf8) : std::string();
}

// Build the base attribute dictionary for a Keychain query/add.
// We use kSecAttrService = key, kSecAttrAccount = "default" so a single
// service has one stored secret. Items are device-only and accessible
// after first unlock (matches AfterFirstUnlock semantics — appropriate
// for a sequencer that may run sACN output as a background task).
NSMutableDictionary* keychainAttrs(std::string_view serviceKey) {
    NSMutableDictionary* attrs = [NSMutableDictionary dictionary];
    attrs[(__bridge id)kSecClass]      = (__bridge id)kSecClassGenericPassword;
    attrs[(__bridge id)kSecAttrService] = toNSString(serviceKey);
    attrs[(__bridge id)kSecAttrAccount] = @"default";
    return attrs;
}

} // namespace

XLiPadServiceSettingsStore::XLiPadServiceSettingsStore() = default;

std::string XLiPadServiceSettingsStore::getString(std::string_view key, const std::string& defaultValue) const {
    @autoreleasepool {
        NSUserDefaults* d = [NSUserDefaults standardUserDefaults];
        NSString* nsKey = toNSString(key);
        NSString* val = [d stringForKey:nsKey];
        if (val == nil) return defaultValue;
        return toStdString(val);
    }
}

int XLiPadServiceSettingsStore::getInt(std::string_view key, int defaultValue) const {
    @autoreleasepool {
        NSUserDefaults* d = [NSUserDefaults standardUserDefaults];
        NSString* nsKey = toNSString(key);
        // objectForKey distinguishes "not set" from "set to 0"; integerForKey
        // would silently treat a missing entry as 0.
        if ([d objectForKey:nsKey] == nil) return defaultValue;
        return (int)[d integerForKey:nsKey];
    }
}

bool XLiPadServiceSettingsStore::getBool(std::string_view key, bool defaultValue) const {
    @autoreleasepool {
        NSUserDefaults* d = [NSUserDefaults standardUserDefaults];
        NSString* nsKey = toNSString(key);
        if ([d objectForKey:nsKey] == nil) return defaultValue;
        return [d boolForKey:nsKey];
    }
}

void XLiPadServiceSettingsStore::setString(std::string_view key, const std::string& value) {
    @autoreleasepool {
        NSUserDefaults* d = [NSUserDefaults standardUserDefaults];
        [d setObject:toNSString(value) forKey:toNSString(key)];
    }
}

void XLiPadServiceSettingsStore::setInt(std::string_view key, int value) {
    @autoreleasepool {
        NSUserDefaults* d = [NSUserDefaults standardUserDefaults];
        [d setInteger:value forKey:toNSString(key)];
    }
}

void XLiPadServiceSettingsStore::setBool(std::string_view key, bool value) {
    @autoreleasepool {
        NSUserDefaults* d = [NSUserDefaults standardUserDefaults];
        [d setBool:value forKey:toNSString(key)];
    }
}

std::string XLiPadServiceSettingsStore::getSecret(std::string_view serviceName) const {
    @autoreleasepool {
        NSMutableDictionary* query = keychainAttrs(serviceName);
        query[(__bridge id)kSecReturnData] = @YES;
        query[(__bridge id)kSecMatchLimit] = (__bridge id)kSecMatchLimitOne;

        CFTypeRef result = NULL;
        OSStatus status = SecItemCopyMatching((__bridge CFDictionaryRef)query, &result);
        if (status == errSecItemNotFound) {
            return {};
        }
        if (status != errSecSuccess || result == NULL) {
            spdlog::warn("Keychain read failed for AI secret '{}': OSStatus {}",
                         std::string(serviceName), (int)status);
            if (result) CFRelease(result);
            return {};
        }

        NSData* data = (__bridge_transfer NSData*)result;
        if (data.length == 0) return {};

        return std::string(static_cast<const char*>(data.bytes), data.length);
    }
}

void XLiPadServiceSettingsStore::setSecret(std::string_view serviceName, const std::string& token) {
    @autoreleasepool {
        NSMutableDictionary* query = keychainAttrs(serviceName);

        if (token.empty()) {
            // Empty token = clear the entry. Treat "not found" as success.
            OSStatus status = SecItemDelete((__bridge CFDictionaryRef)query);
            if (status != errSecSuccess && status != errSecItemNotFound) {
                spdlog::warn("Keychain delete failed for AI secret '{}': OSStatus {}",
                             std::string(serviceName), (int)status);
            }
            return;
        }

        NSData* tokenData = [NSData dataWithBytes:token.data() length:token.size()];

        // Try update first; on errSecItemNotFound, fall back to add. This
        // avoids a delete/add race that could lose the old value on
        // failure mid-write.
        NSDictionary* update = @{ (__bridge id)kSecValueData : tokenData };
        OSStatus status = SecItemUpdate((__bridge CFDictionaryRef)query,
                                        (__bridge CFDictionaryRef)update);

        if (status == errSecItemNotFound) {
            NSMutableDictionary* add = [query mutableCopy];
            add[(__bridge id)kSecValueData] = tokenData;
            add[(__bridge id)kSecAttrAccessible] = (__bridge id)kSecAttrAccessibleAfterFirstUnlock;
            status = SecItemAdd((__bridge CFDictionaryRef)add, NULL);
        }

        if (status != errSecSuccess) {
            spdlog::warn("Keychain write failed for AI secret '{}': OSStatus {}",
                         std::string(serviceName), (int)status);
        }
    }
}
