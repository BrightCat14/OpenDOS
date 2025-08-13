# OpenDOS 
![Build](https://github.com/BrightCat14/OpenDOS/actions/workflows/build.yml/badge.svg)
![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)
![Status: WIP](https://img.shields.io/badge/status-active-blue)
---
A minimal test operating system for learning and experimentation with low-level programming.

## Features

- Basic kernel with assembly/C components
- ATA driver
- Keyboard input handling
- Basic I/O operations
- String utilities

## Current Limitations

⚠️ **Early Development Stage**  
This is a test/experimental OS. Many core features are missing:
- No command shell/CLI implemented yet
- Limited hardware support
- No filesystem or persistent storage

## Building

Requirements:
- NASM (assembler)
- GCC
- LD

```bash
./build.sh
./run.sh  # (or use your preferred emulator)
```

## Community

* 💬 [GitHub Discussions](https://github.com/BrightCat14/OpenDOS/discussions) — ask questions, suggest ideas, and discuss development.
* 📢 [Telegram Group](https://t.me/opendosc) — announcements and chatting about all.

## License

[MIT](LICENSE.md)

---

**Note**: This is an educational project, not suitable for production use. Contributions welcome!
