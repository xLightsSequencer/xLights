# Phase H — App Store readiness

1. **Icon set** (1024x1024 + all iPad sizes), launch screen.
2. **Privacy manifest** (`PrivacyInfo.xcprivacy`).
3. **`Info.plist`** — `NSLocalNetworkUsageDescription` +
   `NSBonjourServices` for controller discovery (relevant once output
   re-enters scope).
4. **TestFlight** build, external beta group, feedback iteration.
5. **Submission.**

Open question: existing Apple Developer team? Is the iPad app a
separate App Store record or shipped as a universal app alongside
macOS? Impacts bundle id and entitlements.
