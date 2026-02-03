import sys
import os
from ami_utils import Parser, AmiBridge

def main():
    if len(sys.argv) < 2:
        print("Usage: python training.py <book_file.txt>")
        return

    book_path = sys.argv[1]
    if not os.path.exists(book_path):
        print(f"Error: File not found: {book_path}")
        return

    # 1. Distill the book into an intelligence stream
    print(f"Reading and distilling: {book_path}...")
    stream = Parser.batch_process_file(book_path)
    
    stream_file = "brain_stream.txt"
    with open(stream_file, "w", encoding="utf-8") as f:
        f.write("\n".join(stream))

    # 2. Run the C++ Brain in training mode
    bridge = AmiBridge(os.path.join("build", "ami_brain_v1.exe"))
    output = bridge.run_training(stream_file)
    print(output)
    print("\nTraining complete. Knowledge saved to brain_data.ami")

if __name__ == "__main__":
    main()
