import zlib
import struct

def save_png(buf, width, height, file_path):
    """Saves a grayscale buffer as a PNG file without external libraries."""
    def make_chunk(type, data):
        return struct.pack(">I", len(data)) + type + data + struct.pack(">I", zlib.crc32(type + data) & 0xffffffff)

    # PNG Header
    header = b"\x89PNG\r\n\x1a\n"
    
    # IHDR chunk: width, height, bit depth=8, color type=0 (grayscale), compression=0, filter=0, interlace=0
    ihdr = make_chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 0, 0, 0, 0))
    
    # IDAT chunk: compressed image data
    rows = []
    for y in range(height):
        rows.append(b"\x00") # Filter type 0
        rows.append(bytes(buf[y*width : (y+1)*width]))
    idat = make_chunk(b"IDAT", zlib.compress(b"".join(rows)))
    
    # IEND chunk
    iend = make_chunk(b"IEND", b"")
    
    with open(file_path, "wb") as f:
        f.write(header + ihdr + idat + iend)

def generate_love_kanji():
    # 16x16 bitmask for Kanji '愛' (Love)
    # This is a simplified representation
    mask = [
        0b0000111111110000,
        0b0000000100000000,
        0b0111111111111110,
        0b0100010000100010,
        0b0100011111100010,
        0b0100010001000010,
        0b0111111111111110,
        0b0000011001100000,
        0b0011111111111100,
        0b0010000000000100,
        0b0011111111111100,
        0b0010100110010100,
        0b0100101001010010,
        0b0100010000100010,
        0b1000110000110001,
        0b0000000000000000,
    ]
    
    width, height = 256, 256
    buf = [255] * (width * height)
    
    # Upscale 16x16 to 256x256
    scale = 256 // 16
    padding = 20 # Padding around the character
    inner_scale = (256 - 2 * padding) // 16
    
    for y in range(16):
        for x in range(16):
            if (mask[y] >> (15 - x)) & 1:
                # Draw a block
                for py in range(inner_scale):
                    for px in range(inner_scale):
                        iy = padding + y * inner_scale + py
                        ix = padding + x * inner_scale + px
                        if iy < height and ix < width:
                            buf[iy * width + ix] = 0
                            
    save_png(buf, width, height, "assets/icons/love.png")
    print("Generated assets/icons/love.png")

if __name__ == "__main__":
    generate_love_kanji()
