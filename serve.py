import http.server
import socketserver
import shutil
import os
import sys

# --- Configuration ---
PORT = 8000
DIRECTORY = "build_wasm/bin/cocos2dx-re2048"

class MyHTTPRequestHandler(http.server.SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        # Set the directory to serve from
        super().__init__(*args, directory=DIRECTORY, **kwargs)

    def end_headers(self):
        # Required for Axmol WASM multi-threading
        self.send_header("Cross-Origin-Embedder-Policy", "require-corp")
        self.send_header("Cross-Origin-Opener-Policy", "same-origin")
        super().end_headers()

def copy_file(src, dst_filename):
    """Copies a file into the designated serving directory."""
    try:
        # Construct full destination path
        dst_path = os.path.join(DIRECTORY, dst_filename)
        shutil.copy2(src, dst_path)
        print(f"Successfully copied: {src} -> {dst_path}")
    except Exception as e:
        print(f"Error copying file: {e}")
        sys.exit(1)

def main():
    # Ensure the directory exists before serving
    cwd = os.getcwd()
    full = os.path.join(cwd, DIRECTORY)
    if not os.path.exists(os.path.join(cwd, DIRECTORY)):
        print(f"Error: Directory '{DIRECTORY}' not found.")
        sys.exit(1)

    copy_file("misc/index.html", "")

    with socketserver.TCPServer(("", PORT), MyHTTPRequestHandler) as httpd:
        print(f"Serving Axmol game from: {DIRECTORY}")
        print(f"URL: http://localhost:{PORT}")
        try:
            httpd.serve_forever()
        except KeyboardInterrupt:
            print("\nServer stopped.")

if __name__ == "__main__":
    main()