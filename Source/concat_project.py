from pathlib import Path

# Change this if needed
TARGET_DIR = Path(".")  # current directory
OUTPUT_FILE = "current_state.txt"

# FIX: Add .json, .html, and .py to capture the whole codebase
EXTENSIONS = {".cpp", ".h", ".hpp", ".txt", ".json", ".html", ".py"}

def collect_files(root: Path):
    files = []
    for path in root.rglob("*"):
        # FIX: Skip the output file itself so it doesn't read its own data
        if path.name == OUTPUT_FILE:
            continue
            
        if path.is_file() and path.suffix in EXTENSIONS:
            files.append(path)
    return sorted(files)

def write_output(files, output_path: Path):
    with output_path.open("w", encoding="utf-8") as out:
        for file in files:
            try:
                content = file.read_text(encoding="utf-8", errors="ignore")
            except Exception as e:
                content = f"<<ERROR READING FILE: {e}>>"

            out.write("\n")
            out.write("=" * 80 + "\n")
            out.write(f"FILE: {file}\n")
            out.write("=" * 80 + "\n\n")
            
            # FIX: Wrap the file content in Markdown code blocks. 
            # This completely stops the chat interface from hiding 
            # C++ angle brackets or flattening your line breaks!
            out.write("```cpp\n")
            out.write(content)
            
            # Ensure the code block closes on a new line
            if not content.endswith("\n"):
                out.write("\n")
            out.write("```\n\n")

def main():
    files = collect_files(TARGET_DIR)
    output_path = TARGET_DIR / OUTPUT_FILE

    print(f"Found {len(files)} files")
    print(f"Writing to {output_path}")

    write_output(files, output_path)

    print("Done.")

if __name__ == "__main__":
    main()