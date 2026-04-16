import SwiftUI

struct EffectBlockView: View {
    let name: String
    let width: CGFloat
    let height: CGFloat
    let color: Color
    let isSelected: Bool

    var body: some View {
        ZStack(alignment: .leading) {
            RoundedRectangle(cornerRadius: 2)
                .fill(color)
                .frame(width: max(width, 1), height: height)

            // Selection border
            if isSelected {
                RoundedRectangle(cornerRadius: 2)
                    .stroke(Color.white, lineWidth: 2)
                    .frame(width: max(width, 1), height: height)
            }

            // Effect name label
            if width > 40 {
                Text(name)
                    .font(.system(size: 8))
                    .foregroundStyle(.white)
                    .lineLimit(1)
                    .padding(.horizontal, 2)
            }
        }
        .frame(width: max(width, 1), height: height)
        .contentShape(Rectangle())
    }
}
