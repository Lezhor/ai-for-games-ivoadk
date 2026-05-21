import base64
import sys
import os
import subprocess
import tempfile

def convert_to_base64_bw(image_path):
    if not os.path.exists(image_path):
        print(f"Error: File '{image_path}' not found.", file=sys.stderr)
        sys.exit(1)

    # Create a temporary file for the B&W PNG
    with tempfile.NamedTemporaryFile(suffix=".png", delete=False) as tmp:
        tmp_path = tmp.name

    try:
        # On macOS (darwin), we can use 'sips' to convert to grayscale.
        # We use -s format png to ensure it's a PNG.
        # We use --matchTo '/System/Library/ColorSync/Profiles/Generic Gray Profile.icc' for B&W
        # Or simpler: sips -s format png --setProperty formatOptions gray

        # Strategy: Use sips to create a grayscale version
        cmd = [
            "sips",
            "-s", "format", "png",
            "--matchTo", "/System/Library/ColorSync/Profiles/Generic Gray Profile.icc",
            image_path,
            "--out", tmp_path
        ]

        result = subprocess.run(cmd, stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
        if result.returncode != 0:
            # Fallback for different macOS versions if profile path differs
            subprocess.run(["sips", "-s", "format", "png", image_path, "--out", tmp_path], check=True, stdout=subprocess.DEVNULL)

        with open(tmp_path, "rb") as image_file:
            encoded_string = base64.b64encode(image_file.read()).decode('utf-8')
            print(encoded_string)

    except Exception as e:
        print(f"Error during conversion: {e}", file=sys.stderr)
        sys.exit(1)
    finally:
        if os.path.exists(tmp_path):
            os.remove(tmp_path)

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Usage: python3 converter.py <path_to_image>", file=sys.stderr)
        sys.exit(1)

    convert_to_base64_bw(sys.argv[1])
