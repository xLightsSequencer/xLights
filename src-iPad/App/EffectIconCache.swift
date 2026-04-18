import UIKit

/// Per-effect icon cache keyed by (effect name, bucket size). Pulls the
/// raw BGRA bytes from the ObjC bridge (which parses the desktop's
/// compiled-in XPM data) and wraps them in a `CGImage`. Images are
/// cached in process memory — there are only ~55 effects and at most
/// five bucket sizes, so the cache is bounded.
@MainActor
final class EffectIconCache {
    static let shared = EffectIconCache()

    private struct Key: Hashable {
        let name: String
        let bucket: Int
    }
    private var cache: [Key: CGImage] = [:]
    // Names we've already tried and failed for (no icon data). Skip
    // re-probing on every frame draw — a first-frame cache miss happens
    // once per session at most.
    private var missing: Set<Key> = []

    /// Buckets mirror the desktop's XPM size list. `desiredPx` is the
    /// pixel width we want on screen (already multiplied by
    /// `UIScreen.main.scale`), and we round up so sampling stays crisp.
    static func bucket(forDesiredPx desiredPx: CGFloat) -> Int {
        let px = Int(desiredPx.rounded(.up))
        if px <= 16 { return 16 }
        if px <= 24 { return 24 }
        if px <= 32 { return 32 }
        if px <= 48 { return 48 }
        return 64
    }

    func image(for effectName: String,
               bucket: Int,
               document: XLSequenceDocument) -> CGImage? {
        let key = Key(name: effectName, bucket: bucket)
        if let img = cache[key] { return img }
        if missing.contains(key) { return nil }

        var w: Int32 = 0
        guard let data = document.iconBGRA(forEffectNamed: effectName,
                                            desiredSize: Int32(bucket),
                                            outputSize: &w),
              w > 0 else {
            missing.insert(key)
            return nil
        }
        let width = Int(w)
        let height = Int(w) // xLights icons are square

        guard let img = makeCGImage(bgraData: data,
                                    width: width,
                                    height: height) else {
            missing.insert(key)
            return nil
        }
        cache[key] = img
        return img
    }

    private func makeCGImage(bgraData: Data, width: Int, height: Int) -> CGImage? {
        let bytesPerRow = width * 4
        let colorSpace = CGColorSpaceCreateDeviceRGB()
        let bitmapInfo = CGBitmapInfo(rawValue:
            CGImageAlphaInfo.premultipliedFirst.rawValue
            | CGBitmapInfo.byteOrder32Little.rawValue)

        guard let cfData = bgraData.withUnsafeBytes({ buf -> CFData? in
            guard let p = buf.baseAddress else { return nil }
            return CFDataCreate(nil,
                                p.assumingMemoryBound(to: UInt8.self),
                                bgraData.count)
        }) else { return nil }

        guard let provider = CGDataProvider(data: cfData) else { return nil }

        return CGImage(width: width,
                       height: height,
                       bitsPerComponent: 8,
                       bitsPerPixel: 32,
                       bytesPerRow: bytesPerRow,
                       space: colorSpace,
                       bitmapInfo: bitmapInfo,
                       provider: provider,
                       decode: nil,
                       shouldInterpolate: true,
                       intent: .defaultIntent)
    }
}
