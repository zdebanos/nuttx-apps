"""Implementation of command line application."""

import argparse
import socket
import io


def sender(name: str, address: str) -> None:
    port = 1234
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    s.connect((address, port))

    maxwrite = 256
    with open(name, mode="rb") as f:
        f.seek(0, io.SEEK_END)
        size = f.tell()
        f.seek(0, io.SEEK_SET)
        transfers = size / maxwrite

        i = 0
        while data := f.read(maxwrite):
            s.sendall(data)
            data = s.recv(4)
            print(f"{((i * 100) / transfers):.2f} %")
            if data != b"NXOS":
                break
            i += 1

    s.close()


def parse_args() -> argparse.Namespace:
    """Parse passed arguments and return result."""
    parser = argparse.ArgumentParser(description="NuttX firmware TCP flasher")
    parser.add_argument(
        "--name",
        default="nuttx.nximg",
        help="Firmware update name.",
    )
    parser.add_argument(
        "--host",
        default="192.168.1.55",
        help="Host address",
    )
    return parser.parse_args()


def main() -> None:
    args = parse_args()

    sender(args.name, args.host)


if __name__ == "__main__":
    main()
