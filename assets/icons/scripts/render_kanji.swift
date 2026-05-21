import AppKit
import CoreGraphics
import Foundation

// this is 100% AI generated obviously :/

func renderKanji(text: String, size: CGFloat, outputPath: String) {
    let imageSize = NSSize(width: size, height: size)
    let offscreenRep = NSBitmapImageRep(bitmapDataPlanes: nil,
                                       pixelsWide: Int(size),
                                       pixelsHigh: Int(size),
                                       bitsPerSample: 8,
                                       samplesPerPixel: 4,
                                       hasAlpha: true,
                                       isPlanar: false,
                                       colorSpaceName: .deviceRGB,
                                       bytesPerRow: 0,
                                       bitsPerPixel: 0)!

    let graphicsContext = NSGraphicsContext(bitmapImageRep: offscreenRep)!
    NSGraphicsContext.saveGraphicsState()
    NSGraphicsContext.current = graphicsContext

    // Clear background (ensures transparency)
    NSColor.clear.set()
    NSRect(origin: .zero, size: imageSize).fill(using: .copy)

    // Setup text attributes
    let font = NSFont(name: "Hiragino Sans W6", size: size * 0.8) ?? NSFont.systemFont(ofSize: size * 0.8)
    let style = NSMutableParagraphStyle()
    style.alignment = .center

    let attributes: [NSAttributedString.Key: Any] = [
        .font: font,
        .foregroundColor: NSColor.white,
        .paragraphStyle: style
    ]

    let attributedString = NSAttributedString(string: text, attributes: attributes)
    let stringSize = attributedString.size()

    // Center the text
    let rect = NSRect(x: (size - stringSize.width) / 2,
                      y: (size - stringSize.height) / 2,
                      width: stringSize.width,
                      height: stringSize.height)

    attributedString.draw(in: rect)

    NSGraphicsContext.restoreGraphicsState()

    if let pngData = offscreenRep.representation(using: .png, properties: [:]) {
        try? pngData.write(to: URL(fileURLWithPath: outputPath))
        print("Successfully rendered \(text) to \(outputPath)")
    } else {
        print("Failed to create PNG data")
    }
}

let args = CommandLine.arguments
if args.count < 3 {
    print("Usage: swift render_kanji.swift <text> <output_path>")
} else {
    renderKanji(text: args[1], size: 256, outputPath: args[2])
}
