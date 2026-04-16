import SwiftUI

struct WaveformView: View {
    let peaks: [Float]  // alternating min/max pairs
    let durationMS: Int
    let pixelsPerMS: CGFloat
    let playPositionMS: Int
    let isPlaying: Bool

    var body: some View {
        let totalWidth = CGFloat(durationMS) * pixelsPerMS
        let height: CGFloat = 60

        ZStack(alignment: .leading) {
            // Waveform
            Canvas { context, size in
                guard peaks.count >= 2 else { return }
                let numBuckets = peaks.count / 2
                let centerY = size.height / 2
                let scale = centerY * 0.9

                for i in 0..<numBuckets {
                    let x = (CGFloat(i) / CGFloat(numBuckets)) * size.width
                    let mn = CGFloat(peaks[i * 2])
                    let mx = CGFloat(peaks[i * 2 + 1])

                    let y1 = centerY - mx * scale
                    let y2 = centerY - mn * scale

                    var path = Path()
                    path.move(to: CGPoint(x: x, y: y1))
                    path.addLine(to: CGPoint(x: x, y: y2))
                    context.stroke(path, with: .color(.green.opacity(0.7)), lineWidth: 1)
                }
            }
            .frame(width: totalWidth, height: height)
            .background(Color.black.opacity(0.3))

            // Play position marker
            Rectangle()
                .fill(Color.white)
                .frame(width: 1, height: height)
                .offset(x: CGFloat(playPositionMS) * pixelsPerMS)
        }
        .frame(height: height)
    }
}
